; OFFSET: original release/snapshot catalogue and sequential installation queue.
section '.verdata' data readable writeable
versionCount dd 0
versionCatalogReady dd 0
versionQueueCount dd 0
versionQueueIndex dd -1
versionQueueDone dd 0
versionQueueFailed dd 0
vManifestMirror du 'https://bmclapi2.bangbang93.com/mc/game/version_manifest_v2.json',0
vManifestOfficial du 'https://piston-meta.mojang.com/mc/game/version_manifest_v2.json',0
vKeyVersions db 'versions',0
vKeyId db 'id',0
vKeyType db 'type',0
vKeyDate db 'releaseTime',0
vRelease db 'release',0
vSnapshot db 'snapshot',0
vStatusFetch db '正在获取版本列表…',0
vStatusReady db '版本列表已更新。',0
vStatusError db '版本列表下载失败，请重试。',0
vStatusCancel db '已取消。',0
vStatusQueueFull db '下载队列已满。',0
vStatusQueueInvalid db '版本或加载器无效。',0
vStatusQueueRunning db '下载进行中。',0
vFmtInstall db '安装 %u/%u · %s',0
vFmtDone db '完成 %u · 失败 %u',0
; Uninitialized arrays are virtual storage, not part of the small executable.
versionRecords rb VC_SIZE*VC_CAPACITY
vCatalogScratch rb VC_SIZE*VC_CAPACITY
versionQueue rb VQ_SIZE*VQ_CAPACITY

section '.vercode' code readable executable
    nop

; Safe ASCII Windows basename, 1..127 bytes. Reject reserved device stems.
proc VersionsSafeId uses ebx esi edi,source
    local stem[128]:BYTE
    mov esi,[source]
    test esi,esi
    jz .bad
    lea edi,[stem]
    xor ecx,ecx
    xor ebx,ebx
.scan:
    mov al,[esi+ecx]
    test al,al
    jz .idEnd
    cmp ecx,127
    jae .bad
    cmp al,'.'
    je .dot
    cmp al,'-'
    je .character
    cmp al,'_'
    je .character
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .character
    cmp al,'A'
    jb .bad
    cmp al,'Z'
    jbe .character
    cmp al,'a'
    jb .bad
    cmp al,'z'
    ja .bad
    jmp .character
.dot:
    test ecx,ecx
    jz .bad
    cmp byte [esi+ecx+1],'.'
    je .bad
    cmp byte [esi+ecx+1],0
    je .bad
    mov ebx,1
.character:
    test ebx,ebx
    jnz .next
    cmp al,'A'
    jb .lowered
    cmp al,'Z'
    ja .lowered
    add al,32
.lowered:
    stosb
.next:
    inc ecx
    jmp .scan
.idEnd:
    test ecx,ecx
    jz .bad
    mov byte [edi],0
    lea esi,[stem]
    sub edi,esi
    cmp edi,3
    je .three
    cmp edi,4
    jne .good
    mov al,[esi+3]
    cmp al,'1'
    jb .good
    cmp al,'9'
    ja .good
    cmp word [esi],'co'
    jne .lpt
    cmp byte [esi+2],'m'
    je .bad
.lpt:
    cmp word [esi],'lp'
    jne .good
    cmp byte [esi+2],'t'
    je .bad
    jmp .good
.three:
    cmp word [esi],'co'
    jne .prn
    cmp byte [esi+2],'n'
    je .bad
.prn:
    cmp word [esi],'pr'
    jne .aux
    cmp byte [esi+2],'n'
    je .bad
.aux:
    cmp word [esi],'au'
    jne .nul
    cmp byte [esi+2],'x'
    je .bad
.nul:
    cmp word [esi],'nu'
    jne .good
    cmp byte [esi+2],'l'
    je .bad
.good:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; Parse into private staging storage, and only publish a complete useful list.
; On error/cancellation an existing successful catalogue remains available.
proc VersionsParse uses ebx esi edi,text
    locals
        context dd ?
        list dd ?
        count dd ?
        item dd ?
        index dd ?
        retained dd ?
        length dd ?
        record rb VC_SIZE
        typeName rb 32
    endl
    mov [retained],0
    mov [context],0
    cmp [engineCancel],0
    jne .bad
    stdcall JsonParse,[text]
    test eax,eax
    jz .bad
    mov [context],eax
    stdcall JsonRoot,eax
    stdcall JsonGet,[context],eax,vKeyVersions
    mov [list],eax
    stdcall JsonCount,[context],eax
    mov [count],eax
    test eax,eax
    jz .bad
    stdcall JsonAt,[context],[list],0
    test eax,eax
    jz .bad
    mov [item],eax
    mov [index],0
.loop:
    cmp [engineCancel],0
    jne .bad
    mov eax,[index]
    cmp eax,[count]
    jae .publish
    cmp [retained],VC_CAPACITY
    jae .publish
    mov ebx,[item]
    test ebx,ebx
    jz .bad
    stdcall JsonGet,[context],ebx,vKeyType
    stdcall JsonString,[context],eax,addr typeName,32
    cmp eax,32
    jae .next
    mov [length],eax
    invoke lstrlenA,addr typeName
    cmp eax,[length]
    jne .next
    invoke lstrcmpA,addr typeName,vRelease
    test eax,eax
    jz .release
    invoke lstrcmpA,addr typeName,vSnapshot
    test eax,eax
    jnz .next
    mov edx,1
    jmp .record
.release:
    xor edx,edx
.record:
    push edx
    lea edi,[record]
    xor eax,eax
    mov ecx,VC_SIZE/4
    rep stosd
    pop edx
    mov dword [record+VC_TYPE],edx
    stdcall JsonGet,[context],ebx,vKeyId
    stdcall JsonString,[context],eax,addr record,128
    cmp eax,128
    jae .next
    test eax,eax
    jz .next
    mov [length],eax
    invoke lstrlenA,addr record
    cmp eax,[length]
    jne .next
    stdcall VersionsSafeId,addr record
    test eax,eax
    jz .next
    stdcall JsonGet,[context],ebx,vKeyDate
    lea edx,[record+VC_DATE]
    stdcall JsonString,[context],eax,edx,32
    cmp eax,32
    jae .next
    mov [length],eax
    lea edx,[record+VC_DATE]
    invoke lstrlenA,edx
    cmp eax,[length]
    jne .next
    imul eax,[retained],VC_SIZE
    lea edi,[vCatalogScratch+eax]
    lea esi,[record]
    mov ecx,VC_SIZE/4
    rep movsd
    inc [retained]
.next:
    stdcall JsonNext,[context],[item]
    mov [item],eax
    inc [index]
    jmp .loop
.publish:
    cmp [retained],0
    je .bad
    cmp [engineCancel],0
    jne .bad
    imul ecx,[retained],VC_SIZE/4
    mov esi,vCatalogScratch
    mov edi,versionRecords
    rep movsd
    mov eax,[retained]
    mov [versionCount],eax
    mov [versionCatalogReady],1
    stdcall JsonFree,[context]
    mov eax,1
    ret
.bad:
    cmp [context],0
    je .failed
    stdcall JsonFree,[context]
.failed:
    xor eax,eax
    ret
endp

proc VersionsFetch uses ebx esi edi
    local stage:DWORD, response:DWORD, result:DWORD
    mov [stage],0
    mov [result],0
    stdcall SetStatusA,vStatusFetch
.request:
    cmp [engineCancel],0
    jne .cancel
    mov eax,vManifestMirror
    cmp [stage],0
    je .transport
    mov eax,vManifestOfficial
.transport:
    stdcall HttpRequest,eax,0,0,0
    mov [response],eax
    test eax,eax
    jz .next
    cmp [engineCancel],0
    jne .freeCancelled
    cmp [httpStatus],200
    jne .freeNext
    stdcall VersionsParse,[response]
    mov [result],eax
.freeNext:
    stdcall FreeMem,[response]
    cmp [result],0
    jne .success
.next:
    inc [stage]
    cmp [stage],2
    jb .request
    stdcall SetStatusA,vStatusError
    xor eax,eax
    ret
.freeCancelled:
    stdcall FreeMem,[response]
.cancel:
    stdcall SetStatusA,vStatusCancel
    xor eax,eax
    ret
.success:
    stdcall SetStatusA,vStatusReady
    mov eax,1
    ret
endp

proc VersionQueueCounts uses esi
    mov [versionQueueDone],0
    mov [versionQueueFailed],0
    xor ecx,ecx
    mov esi,versionQueue
.loop:
    cmp ecx,[versionQueueCount]
    jae .done
    cmp dword [esi+VQ_STATE],2
    jne .failed
    inc [versionQueueDone]
.failed:
    cmp dword [esi+VQ_STATE],3
    jne .next
    inc [versionQueueFailed]
.next:
    inc ecx
    add esi,VQ_SIZE
    jmp .loop
.done:
    ret
endp

proc VersionQueueAdd uses ebx esi edi,baseIdA,loaderKind,loaderVersionA
    cmp [versionQueueIndex],-1
    jne .running
    cmp [loaderKind],4
    ja .invalid
    stdcall VersionsSafeId,[baseIdA]
    test eax,eax
    jz .invalid
    cmp [loaderKind],0
    je .duplicates
    stdcall VersionsSafeId,[loaderVersionA]
    test eax,eax
    jz .invalid
.duplicates:
    xor esi,esi
    mov ebx,versionQueue
.scan:
    cmp esi,[versionQueueCount]
    jae .append
    mov eax,[loaderKind]
    cmp eax,[ebx+VQ_LOADER]
    jne .next
    invoke lstrcmpA,ebx,[baseIdA]
    test eax,eax
    jnz .next
    cmp [loaderKind],0
    je .duplicate
    lea eax,[ebx+VQ_VERSION]
    invoke lstrcmpA,eax,[loaderVersionA]
    test eax,eax
    jz .duplicate
.next:
    add ebx,VQ_SIZE
    inc esi
    jmp .scan
.duplicate:
    ; Adding an already completed/failed item reuses its queue row for repair.
    cmp dword [ebx+VQ_STATE],1
    je .running
    mov dword [ebx+VQ_STATE],0
    stdcall VersionQueueCounts
    mov eax,1
    ret
.append:
    cmp [versionQueueCount],VQ_CAPACITY
    jae .full
    mov edi,ebx
    xor eax,eax
    mov ecx,VQ_SIZE/4
    rep stosd
    invoke lstrcpyA,ebx,[baseIdA]
    mov eax,[loaderKind]
    mov [ebx+VQ_LOADER],eax
    test eax,eax
    jz .plain
    lea eax,[ebx+VQ_VERSION]
    invoke lstrcpyA,eax,[loaderVersionA]
    jmp .added
.plain:
    lea eax,[ebx+VQ_TARGET]
    invoke lstrcpyA,eax,[baseIdA]
.added:
    inc [versionQueueCount]
    mov eax,1
    ret
.full:
    stdcall SetStatusA,vStatusQueueFull
    xor eax,eax
    ret
.invalid:
    stdcall SetStatusA,vStatusQueueInvalid
    xor eax,eax
    ret
.running:
    stdcall SetStatusA,vStatusQueueRunning
    xor eax,eax
    ret
endp

proc VersionQueueRemove uses ebx esi edi,index
    cmp [versionQueueIndex],-1
    jne .bad
    mov eax,[index]
    cmp eax,[versionQueueCount]
    jae .bad
    imul edi,eax,VQ_SIZE
    add edi,versionQueue
    cmp dword [edi+VQ_STATE],1
    je .bad
    lea esi,[edi+VQ_SIZE]
    mov ecx,[versionQueueCount]
    sub ecx,eax
    dec ecx
    imul ecx,VQ_SIZE
    rep movsb
    dec [versionQueueCount]
    mov eax,[versionQueueCount]
    imul edi,eax,VQ_SIZE
    add edi,versionQueue
    xor eax,eax
    mov ecx,VQ_SIZE/4
    rep stosd
    stdcall VersionQueueCounts
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc VersionQueueClear uses edi
    cmp [versionQueueIndex],-1
    jne .bad
    mov edi,versionQueue
    mov ecx,[versionQueueCount]
.check:
    test ecx,ecx
    jz .clear
    cmp dword [edi+VQ_STATE],1
    je .bad
    add edi,VQ_SIZE
    dec ecx
    jmp .check
.clear:
    mov edi,versionQueue
    imul ecx,[versionQueueCount],VQ_SIZE/4
    xor eax,eax
    rep stosd
    mov [versionQueueCount],0
    mov [versionQueueDone],0
    mov [versionQueueFailed],0
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; The GUI disables queue changes while this worker runs. Only pending rows run.
; One completion message belongs to this worker, never to each base install.
proc VersionQueueRun uses ebx esi edi,param
    locals
        savedVersion rb 128
        progress rb 512
        index dd ?
        result dd ?
    endl
    invoke lstrcpyA,addr savedVersion,selectedVersion
    mov [index],0
    mov [result],1
    stdcall VersionQueueCounts
.loop:
    cmp [engineCancel],0
    jne .cancelIdle
    mov eax,[index]
    cmp eax,[versionQueueCount]
    jae .finished
    imul ebx,eax,VQ_SIZE
    add ebx,versionQueue
    cmp dword [ebx+VQ_STATE],0
    jne .next
    mov [versionQueueIndex],eax
    mov dword [ebx+VQ_STATE],1
    inc eax
if defined OFFSET_I18N
    cinvoke wsprintfA,addr progress,[trVFmtInstall],eax,[versionQueueCount],ebx
else
    cinvoke wsprintfA,addr progress,vFmtInstall,eax,[versionQueueCount],ebx
end if
    stdcall SetStatusA,addr progress
    invoke lstrcpyA,selectedVersion,ebx
    stdcall EInstallCore
    mov edi,eax
    stdcall ECleanup
    ; Rebuild the queue pointer after the installer boundary. It must not rely
    ; on a metadata token or a register retained by a nested installer helper.
    imul ebx,[index],VQ_SIZE
    add ebx,versionQueue
    cmp [engineCancel],0
    jne .cancelCurrent
    test edi,edi
    jz .failed
    cmp dword [ebx+VQ_LOADER],0
    je .complete
    lea eax,[ebx+VQ_VERSION]
    lea edx,[ebx+VQ_TARGET]
    stdcall LoaderInstall,ebx,dword [ebx+VQ_LOADER],eax,edx
    cmp [engineCancel],0
    jne .cancelCurrent
    test eax,eax
    jz .failed
    lea eax,[ebx+VQ_TARGET]
    stdcall ProfileNormalize,eax,ebx
    cmp [engineCancel],0
    jne .cancelCurrent
    test eax,eax
    jz .failed
    ; Native classifiers belong to the selected profile's offset-natives path.
    ; Normalization reuses the vanilla JAR, but still needs a child extraction.
    lea eax,[ebx+VQ_TARGET]
    invoke lstrcpyA,selectedVersion,eax
    stdcall EValidateVersion
    test eax,eax
    jz .childInvalid
    stdcall EBuildPaths
    stdcall ELoadMetadata
    test eax,eax
    jz .childFailed
    stdcall EInstallLibraries
    mov edi,eax
    jmp .childCleanup
.childInvalid:
    stdcall SetStatusA,vStatusQueueInvalid
.childFailed:
    xor edi,edi
.childCleanup:
    stdcall ECleanup
    cmp [engineCancel],0
    jne .cancelCurrent
    test edi,edi
    jz .failed
.complete:
    mov dword [ebx+VQ_STATE],2
    jmp .next
.failed:
    mov dword [ebx+VQ_STATE],3
    mov [result],0
.next:
    inc [index]
    jmp .loop
.cancelCurrent:
    mov dword [ebx+VQ_STATE],4
.cancelIdle:
    mov [result],0
    stdcall SetStatusA,vStatusCancel
.finished:
    invoke lstrcpyA,selectedVersion,addr savedVersion
    mov [versionQueueIndex],-1
    stdcall VersionQueueCounts
    cmp [engineCancel],0
    jne .notify
if defined OFFSET_I18N
    cinvoke wsprintfA,addr progress,[trVFmtDone],[versionQueueDone],[versionQueueFailed]
else
    cinvoke wsprintfA,addr progress,vFmtDone,[versionQueueDone],[versionQueueFailed]
end if
    stdcall SetStatusA,addr progress
    cmp [versionQueueFailed],0
    je .notify
    mov [result],0
.notify:
    invoke PostMessageW,[hWnd],WM_APP+2,0,0
    mov eax,[result]
    ret
endp
