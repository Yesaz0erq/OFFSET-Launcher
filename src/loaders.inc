; Original native loader discovery and official installer integration.
section '.ldrdata' data readable writeable
loaderVersionCount dd 0
ldFabricList db 'https://meta.fabricmc.net/v2/versions/loader/%s',0
ldQuiltList db 'https://meta.quiltmc.org/v3/versions/loader/%s',0
ldFabricProfile db 'https://meta.fabricmc.net/v2/versions/loader/%s/%s/profile/json',0
ldQuiltProfile db 'https://meta.quiltmc.org/v3/versions/loader/%s/%s/profile/json',0
ldForgeList db 'https://files.minecraftforge.net/net/minecraftforge/forge/maven-metadata.json',0
ldNeoList db 'https://maven.neoforged.net/api/maven/versions/releases/net/neoforged/neoforge',0
ldForgeJar db 'https://maven.minecraftforge.net/net/minecraftforge/forge/%s/forge-%s-installer.jar',0
ldForgeMirror db 'https://bmclapi2.bangbang93.com/maven/net/minecraftforge/forge/%s/forge-%s-installer.jar',0
ldNeoJar db 'https://maven.neoforged.net/releases/net/neoforged/neoforge/%s/neoforge-%s-installer.jar',0
ldShaExt db '.sha1',0
ldKeyVersions db 'versions',0
ldKeyLoader db 'loader',0
ldKeyVersion db 'version',0
ldKeyStable db 'stable',0
ldKeyId db 'id',0
ldKeyParent db 'inheritsFrom',0
ldListStatus db '正在获取加载器版本…',0
ldInstallStatus db '正在安装加载器…',0
ldFailure db '加载器安装失败，请检查安装日志与 Java 版本。',0
ldCancelled db '安装已取消，缓存已保留。',0
ldInvalid db '加载器版本或游戏版本无效。',0
ldNetwork db '加载器列表获取失败。',0
ldSlash du '\',0
ldVersions du '\versions\',0
ldJson du '.json',0
ldBackup du '\offset-before-install.json',0
ldNew du '.offset-new',0
ldCacheFormat du '\.offset-install\%u-%s-%s',0
ldInstaller du '\installer.jar',0
ldVersionJson du '\version.json',0
ldInstallerLog du '\installer.log',0
ldExtractLog du '\extract.log',0
ldProfiles du '\launcher_profiles.json',0
ldStoreProfiles du '\launcher_profiles_microsoft_store.json',0
ldProfileStub db '{"profiles":{}}',0
ldNul du 'NUL',0
ldJavaJar db '-jar',0
ldInstallArg db '--installClient',0
ldTarExtract db '-xf',0
ldTarC db '-C',0
ldVersionEntry db 'version.json',0
ldForgeFull db '%s-%s',0
ldForgeId db '%s-forge-%s',0
ldNeoId db 'neoforge-%s',0
loaderVersions rb VL_SIZE*VL_CAPACITY

section '.ldrcode' code readable executable
    nop

; A URL/path component, with no traversal, control characters or truncation.
proc LoaderSafeId uses esi,value
    mov esi,[value]
    test esi,esi
    jz .bad
    xor ecx,ecx
.loop:
    mov al,[esi+ecx]
    test al,al
    jz .idEnd
    cmp ecx,120
    jae .bad
    cmp al,'0'
    jb .punct
    cmp al,'9'
    jbe .next
    cmp al,'A'
    jb .punct
    cmp al,'Z'
    jbe .next
    cmp al,'a'
    jb .punct
    cmp al,'z'
    jbe .next
.punct:
    cmp al,'-'
    je .next
    cmp al,'_'
    je .next
    cmp al,'+'
    je .next
    cmp al,'.'
    jne .bad
    test ecx,ecx
    jz .bad
    cmp byte [esi+ecx-1],'.'
    je .bad
.next:
    inc ecx
    jmp .loop
.idEnd:
    test ecx,ecx
    jz .bad
    cmp byte [esi+ecx-1],'.'
    je .bad
    cmp ecx,3
    jb .idGood
    movzx eax,word [esi]
    or eax,2020h
    movzx edx,byte [esi+2]
    or edx,20h
    shl edx,16
    or eax,edx
    cmp eax,'con'
    je .reserved3
    cmp eax,'prn'
    je .reserved3
    cmp eax,'aux'
    je .reserved3
    cmp eax,'nul'
    je .reserved3
    cmp eax,'com'
    je .device4
    cmp eax,'lpt'
    jne .idGood
.device4:
    cmp ecx,4
    jb .idGood
    mov al,[esi+3]
    cmp al,'1'
    jb .idGood
    cmp al,'9'
    ja .idGood
    mov al,[esi+4]
    jmp .deviceEnd
.reserved3:
    mov al,[esi+3]
.deviceEnd:
    test al,al
    jz .bad
    cmp al,'.'
    je .bad
.idGood:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; Natural numerical ordering without integer overflow. 1 = left newer.
proc LdCompare uses ebx esi edi,left,right
    locals
        aStart dd ?
        bStart dd ?
        aEnd dd ?
        bEnd dd ?
    endl
    mov esi,[left]
    mov edi,[right]
.loop:
    mov al,[esi]
    mov dl,[edi]
    test al,al
    jz .leftEnd
    test dl,dl
    jz .rightEnd
    cmp al,'0'
    jb .chars
    cmp al,'9'
    ja .chars
    cmp dl,'0'
    jb .chars
    cmp dl,'9'
    ja .chars
    mov [aStart],esi
    mov [bStart],edi
.aDigits:
    mov al,[esi]
    cmp al,'0'
    jb .aDone
    cmp al,'9'
    ja .aDone
    inc esi
    jmp .aDigits
.aDone:
    mov [aEnd],esi
.bDigits:
    mov al,[edi]
    cmp al,'0'
    jb .bDone
    cmp al,'9'
    ja .bDone
    inc edi
    jmp .bDigits
.bDone:
    mov [bEnd],edi
    mov esi,[aStart]
    mov edi,[bStart]
.aZeros:
    cmp esi,[aEnd]
    jae .bZeros
    cmp byte [esi],'0'
    jne .bZeros
    inc esi
    jmp .aZeros
.bZeros:
    cmp edi,[bEnd]
    jae .lengths
    cmp byte [edi],'0'
    jne .lengths
    inc edi
    jmp .bZeros
.lengths:
    mov eax,[aEnd]
    sub eax,esi
    mov edx,[bEnd]
    sub edx,edi
    cmp eax,edx
    ja .newer
    jb .older
    mov ecx,eax
.number:
    test ecx,ecx
    jz .numbersEqual
    mov al,[esi]
    cmp al,[edi]
    ja .newer
    jb .older
    inc esi
    inc edi
    dec ecx
    jmp .number
.numbersEqual:
    mov esi,[aEnd]
    mov edi,[bEnd]
    jmp .loop
.chars:
    cmp al,dl
    ja .newer
    jb .older
    inc esi
    inc edi
    jmp .loop
.leftEnd:
    test dl,dl
    jz .equal
    cmp dl,'-'
    je .newer
    jmp .older
.rightEnd:
    cmp al,'-'
    je .older
.newer:
    mov eax,1
    ret
.older:
    mov eax,-1
    ret
.equal:
    xor eax,eax
    ret
endp

proc LdStable uses esi,version
    mov esi,[version]
.loop:
    lodsb
    cmp al,'-'
    je .no
    test al,al
    jnz .loop
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

; The 26.x repository also contains snapshot coordinates sharing a release
; prefix. Only numeric stable coordinates are proven compatible here.
proc LdModernNeoStable uses esi,base,version
    mov esi,[base]
    cmp word [esi],'26'
    jne .ok
    cmp byte [esi+2],'.'
    jne .ok
    mov esi,[version]
.loop:
    lodsb
    test al,al
    jz .ok
    cmp al,'.'
    je .loop
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .loop
.bad:
    xor eax,eax
    ret
.ok:
    mov eax,1
    ret
endp

proc LdAddVersion uses ebx esi edi,version,stable
    local index:DWORD
    stdcall LoaderSafeId,[version]
    test eax,eax
    jz .done
    xor ebx,ebx
.find:
    cmp ebx,[loaderVersionCount]
    jae .insert
    imul esi,ebx,VL_SIZE
    add esi,loaderVersions
    stdcall LdCompare,[version],esi
    test eax,eax
    jz .done
    jg .insert
    inc ebx
    jmp .find
.insert:
    cmp ebx,VL_CAPACITY
    jae .done
    mov [index],ebx
    mov eax,[loaderVersionCount]
    cmp eax,VL_CAPACITY
    jb .grow
    dec eax
    jmp .shift
.grow:
    inc [loaderVersionCount]
.shift:
    cmp eax,[index]
    jbe .store
    imul edi,eax,VL_SIZE
    add edi,loaderVersions
    lea esi,[edi-VL_SIZE]
    mov ecx,VL_SIZE/4
    rep movsd
    dec eax
    jmp .shift
.store:
    imul edi,[index],VL_SIZE
    add edi,loaderVersions
    invoke lstrcpyA,edi,[version]
    mov eax,[stable]
    mov [edi+VL_STABLE],eax
.done:
    ret
endp

; Proven NeoForge schemes: MC1.21.1 ->21.1.; MC26.3 ->26.3.0.;
; MC26.1.2 ->26.1.2. (verified against official installer.minecraft fields).
; Snapshots, pre-releases and unknown schemes are intentionally incompatible.
proc LdNeoPrefix uses ebx esi edi,base,dest
    mov edi,[dest]
    mov byte [edi],0
    mov esi,[base]
    cmp word [esi],'26'
    jne .legacy
    cmp byte [esi+2],'.'
    jne .bad
    mov word [edi],'26'
    mov byte [edi+2],'.'
    add edi,3
    add esi,3
    xor ecx,ecx
.modernMinor:
    mov al,[esi]
    cmp al,'0'
    jb .modernMinorEnd
    cmp al,'9'
    ja .modernMinorEnd
    cmp ecx,3
    jae .bad
    stosb
    inc esi
    inc ecx
    jmp .modernMinor
.modernMinorEnd:
    test ecx,ecx
    jz .bad
    cmp al,0
    jne .modernPatchStart
    mov word [edi],'.0'
    add edi,2
    jmp .finish
.modernPatchStart:
    cmp al,'.'
    jne .bad
    stosb
    inc esi
    xor ecx,ecx
.modernPatch:
    mov al,[esi]
    test al,al
    jz .modernEnd
    cmp al,'0'
    jb .bad
    cmp al,'9'
    ja .bad
    cmp ecx,3
    jae .bad
    stosb
    inc esi
    inc ecx
    jmp .modernPatch
.modernEnd:
    test ecx,ecx
    jz .bad
    jmp .finish
.legacy:
    cmp word [esi],'1.'
    jne .bad
    add esi,2
    xor ebx,ebx
    xor ecx,ecx
.minor:
    movzx eax,byte [esi]
    sub eax,'0'
    cmp eax,9
    ja .minorEnd
    cmp ecx,3
    jae .bad
    imul ebx,10
    add ebx,eax
    mov al,[esi]
    stosb
    inc esi
    inc ecx
    jmp .minor
.minorEnd:
    test ecx,ecx
    jz .bad
    cmp ebx,20
    jb .bad
    mov byte [edi],'.'
    inc edi
    xor edx,edx
    cmp byte [esi],0
    je .zero
    cmp byte [esi],'.'
    jne .bad
    inc esi
    xor ecx,ecx
.patch:
    movzx eax,byte [esi]
    sub eax,'0'
    cmp eax,9
    ja .patchEnd
    cmp ecx,3
    jae .bad
    imul edx,10
    add edx,eax
    mov al,[esi]
    stosb
    inc esi
    inc ecx
    jmp .patch
.patchEnd:
    test ecx,ecx
    jz .bad
    cmp byte [esi],0
    jne .bad
    jmp .compatible
.zero:
    mov byte [edi],'0'
    inc edi
.compatible:
    cmp ebx,20
    jne .finish
    cmp edx,2
    jb .bad
.finish:
    mov word [edi],'.'
    mov eax,1
    ret
.bad:
    mov edi,[dest]
    mov byte [edi],0
    xor eax,eax
    ret
endp

proc LdRequestText uses ebx,urlA
    local wide[4096]:WORD
    cmp [engineCancel],0
    jne .bad
    invoke MultiByteToWideChar,65001,8,[urlA],-1,addr wide,4096
    test eax,eax
    jz .bad
    stdcall HttpRequest,addr wide,0,0,0
    test eax,eax
    jz .bad
    mov ebx,eax
    cmp [httpStatus],200
    jne .free
    cmp [engineCancel],0
    jne .free
    mov eax,ebx
    ret
.free:
    stdcall FreeMem,ebx
.bad:
    xor eax,eax
    ret
endp

proc LdReadJson uses ebx,urlA,textOut
    mov ebx,[textOut]
    mov dword [ebx],0
    stdcall LdRequestText,[urlA]
    test eax,eax
    jz .done
    mov [ebx],eax
    stdcall JsonParse,eax
    test eax,eax
    jnz .done
    stdcall FreeMem,dword [ebx]
    mov dword [ebx],0
    xor eax,eax
.done:
    ret
endp

proc LdIsArray,ctx,token
    local raw[4]:BYTE
    stdcall JsonRaw,[ctx],[token],addr raw,4
    cmp eax,2
    jb .no
    cmp byte [raw],'['
    jne .no
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

proc LdString uses ebx,ctx,token,dest,capacity
    stdcall JsonString,[ctx],[token],[dest],[capacity]
    test eax,eax
    jz .bad
    cmp eax,[capacity]
    jae .bad
    mov ebx,eax
    invoke lstrlenA,[dest]
    cmp eax,ebx
    jne .bad
    ret
.bad:
    mov eax,[dest]
    mov byte [eax],0
    xor eax,eax
    ret
endp

proc LoaderFetchVersions uses ebx esi edi,baseIdA,kind
    locals
        response dd ?
        context dd ?
        list dd ?
        count dd ?
        index dd ?
        stable dd ?
        prefixLength dd ?
        result dd ?
        url rb 4096
        version rb 128
        prefix rb 128
    endl
    mov [loaderVersionCount],0
    mov [response],0
    mov [context],0
    mov [result],0
    stdcall LoaderSafeId,[baseIdA]
    test eax,eax
    jz .done
    cmp [engineCancel],0
    jne .done
    mov eax,[kind]
    cmp eax,1
    je .fabric
    cmp eax,2
    je .forge
    cmp eax,3
    je .neo
    cmp eax,4
    jne .done
    cinvoke wsprintfA,addr url,ldQuiltList,[baseIdA]
    jmp .request
.fabric:
    cinvoke wsprintfA,addr url,ldFabricList,[baseIdA]
    jmp .request
.forge:
    invoke lstrcpyA,addr url,ldForgeList
    jmp .request
.neo:
    stdcall LdNeoPrefix,[baseIdA],addr prefix
    test eax,eax
    jz .empty
    invoke lstrlenA,addr prefix
    mov [prefixLength],eax
    invoke lstrcpyA,addr url,ldNeoList
.request:
    stdcall SetStatusA,ldListStatus
    stdcall LdReadJson,addr url,addr response
    mov [context],eax
    test eax,eax
    jnz .root
    cmp [kind],1
    je .notFound
    cmp [kind],4
    jne .done
.notFound:
    cmp [httpStatus],404
    je .empty
    jmp .done
.root:
    stdcall JsonRoot,eax
    cmp [kind],2
    jne .neoList
    stdcall JsonGet,[context],eax,[baseIdA]
    test eax,eax
    jz .empty
    jmp .array
.neoList:
    cmp [kind],3
    jne .array
    stdcall JsonGet,[context],eax,ldKeyVersions
.array:
    mov [list],eax
    stdcall LdIsArray,[context],eax
    test eax,eax
    jz .done
    stdcall JsonCount,[context],[list]
    mov [count],eax
    mov [index],0
.loop:
    cmp [engineCancel],0
    jne .done
    mov eax,[index]
    cmp eax,[count]
    jae .empty
    stdcall JsonAt,[context],[list],eax
    mov ebx,eax
    cmp [kind],2
    je .forgeVersion
    cmp [kind],3
    je .neoVersion
    stdcall JsonGet,[context],ebx,ldKeyLoader
    mov ebx,eax
    stdcall JsonGet,[context],ebx,ldKeyStable
    stdcall JsonBool,[context],eax
    mov [stable],eax
    stdcall JsonGet,[context],ebx,ldKeyVersion
    stdcall LdString,[context],eax,addr version,128
    test eax,eax
    jz .next
    cmp eax,128
    jae .next
    cmp [stable],-1
    jne .add
    stdcall LdStable,addr version
    mov [stable],eax
    jmp .add
.forgeVersion:
    stdcall LdString,[context],ebx,addr version,128
    cmp eax,128
    jae .next
    mov esi,[baseIdA]
    lea edi,[version]
.base:
    lodsb
    test al,al
    jz .baseEnd
    cmp al,[edi]
    jne .next
    inc edi
    jmp .base
.baseEnd:
    cmp byte [edi],'-'
    jne .next
    inc edi
    invoke lstrcpyA,addr version,edi
    jmp .derivedStable
.neoVersion:
    stdcall LdString,[context],ebx,addr version,128
    cmp eax,128
    jae .next
    lea esi,[prefix]
    lea edi,[version]
    mov ecx,[prefixLength]
    repe cmpsb
    jne .next
    stdcall LdModernNeoStable,[baseIdA],addr version
    test eax,eax
    jz .next
.derivedStable:
    stdcall LdStable,addr version
    mov [stable],eax
.add:
    stdcall LdAddVersion,addr version,[stable]
.next:
    inc [index]
    jmp .loop
.empty:
    mov [result],1
.done:
    cmp [context],0
    je .response
    stdcall JsonFree,[context]
.response:
    stdcall FreeMem,[response]
    cmp [result],0
    jne .return
    mov [loaderVersionCount],0
    cmp [engineCancel],0
    jne .return
    stdcall SetStatusA,ldNetwork
.return:
    mov eax,[result]
    ret
endp

proc LdValidateProfile uses ebx,ctx,base,target
    locals
        id rb 128
        parent rb 128
    endl
    stdcall JsonRoot,[ctx]
    mov ebx,eax
    stdcall JsonGet,[ctx],ebx,ldKeyParent
    stdcall LdString,[ctx],eax,addr parent,128
    test eax,eax
    jz .bad
    cmp eax,128
    jae .bad
    invoke lstrcmpA,addr parent,[base]
    test eax,eax
    jnz .bad
    stdcall JsonGet,[ctx],ebx,ldKeyId
    stdcall LdString,[ctx],eax,addr id,128
    test eax,eax
    jz .bad
    cmp eax,128
    jae .bad
    stdcall LoaderSafeId,addr id
    test eax,eax
    jz .bad
    invoke lstrcmpA,addr id,[base]
    test eax,eax
    jz .bad
    invoke lstrcpyA,[target],addr id
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc LdProfilePaths uses ebx,id,installDir,path
    local wide[128]:WORD
    invoke MultiByteToWideChar,65001,8,[id],-1,addr wide,128
    test eax,eax
    jz .done
    invoke lstrlenW,gameDir
    cmp eax,1400
    ja .bad
    invoke lstrcpyW,[installDir],gameDir
    invoke lstrcatW,[installDir],ldVersions
    invoke lstrcatW,[installDir],addr wide
    invoke lstrcpyW,[path],[installDir]
    invoke lstrcatW,[path],ldSlash
    invoke lstrcatW,[path],addr wide
    invoke lstrcatW,[path],ldJson
    mov eax,1
.done:
    ret
.bad:
    xor eax,eax
    ret
endp

proc LdBackupProfile,path,installDir
    local backup[2048]:WORD
    invoke GetFileAttributesW,[path]
    cmp eax,-1
    je .ok
    invoke lstrcpyW,addr backup,[installDir]
    invoke lstrcatW,addr backup,ldBackup
    invoke GetFileAttributesW,addr backup
    cmp eax,-1
    jne .ok
    invoke CopyFileW,[path],addr backup,1
    ret
.ok:
    mov eax,1
    ret
endp

proc LdWriteFile uses ebx esi,path,text
    local file:DWORD,written:DWORD,length:DWORD
    invoke lstrlenA,[text]
    mov [length],eax
    invoke CreateFileW,[path],GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .bad
    mov [file],eax
    invoke WriteFile,eax,[text],[length],addr written,0
    mov ebx,eax
    invoke CloseHandle,[file]
    test ebx,ebx
    jz .bad
    mov eax,[written]
    cmp eax,[length]
    jne .bad
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc LdPublishProfile uses ebx,path,installDir,text
    local temp[2048]:WORD
    stdcall EnsureDirectory,[installDir]
    test eax,eax
    jz .bad
    stdcall LdBackupProfile,[path],[installDir]
    test eax,eax
    jz .bad
    invoke lstrcpyW,addr temp,[path]
    invoke lstrcatW,addr temp,ldNew
    stdcall LdWriteFile,addr temp,[text]
    test eax,eax
    jz .remove
    cmp [engineCancel],0
    jne .remove
    invoke MoveFileExW,addr temp,[path],9
    test eax,eax
    jnz .done
.remove:
    invoke DeleteFileW,addr temp
.bad:
    xor eax,eax
.done:
    ret
endp

; Engine's existing command encoder handles quotes and trailing backslashes.
proc LdCommandStart,pathW
    mov [eCLen],0
    mov byte [eCommand],0
    stdcall LdCommandWide,[pathW]
    ret
endp
proc LdCommandWide,pathW
    invoke WideCharToMultiByte,65001,0,[pathW],-1,eScratch,32768,0,0
    test eax,eax
    jz .done
    stdcall EAppendCommand,eScratch
.done:
    ret
endp

; Only the child process created here is terminated when cancellation is set.
proc LdRun uses ebx esi edi,application,cwd,log
    locals
        startup STARTUPINFO
        process PROCESS_INFORMATION
        security dd 3 dup(?)
        output dd ?
        input dd ?
        exitCode dd ?
        result dd ?
    endl
    mov [result],0
    mov [output],-1
    mov [input],-1
    cmp [engineCancel],0
    jne .done
    invoke MultiByteToWideChar,65001,8,eCommand,-1,eCmd,32768
    test eax,eax
    jz .done
    mov [security],12
    mov [security+4],0
    mov [security+8],1
    invoke CreateFileW,[log],GENERIC_WRITE,FILE_SHARE_READ+FILE_SHARE_WRITE,addr security,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .done
    mov [output],eax
    invoke CreateFileW,ldNul,GENERIC_READ,FILE_SHARE_READ+FILE_SHARE_WRITE,addr security,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .done
    mov [input],eax
    lea edi,[startup]
    xor eax,eax
    mov ecx,sizeof.STARTUPINFO/4
    rep stosd
    mov [startup.cb],sizeof.STARTUPINFO
    mov [startup.dwFlags],101h
    mov eax,[output]
    mov [startup.hStdOutput],eax
    mov [startup.hStdError],eax
    mov eax,[input]
    mov [startup.hStdInput],eax
    invoke CreateProcessW,[application],eCmd,0,0,1,08000000h,0,[cwd],addr startup,addr process
    test eax,eax
    jz .done
    invoke CloseHandle,[process.hThread]
.wait:
    invoke WaitForSingleObject,[process.hProcess],100
    test eax,eax
    jz .exited
    cmp eax,258
    jne .kill
    cmp [engineCancel],0
    je .wait
.kill:
    invoke TerminateProcess,[process.hProcess],1
    invoke WaitForSingleObject,[process.hProcess],3000
    jmp .close
.exited:
    invoke GetExitCodeProcess,[process.hProcess],addr exitCode
    test eax,eax
    jz .close
    cmp [exitCode],0
    jne .close
    cmp [engineCancel],0
    jne .close
    mov [result],1
.close:
    invoke CloseHandle,[process.hProcess]
.done:
    cmp [output],-1
    je .input
    invoke CloseHandle,[output]
.input:
    cmp [input],-1
    je .return
    invoke CloseHandle,[input]
.return:
    mov eax,[result]
    ret
endp

proc LdSha1 uses esi edi,text,dest
    mov esi,[text]
    mov edi,[dest]
    mov ecx,40
.hex:
    lodsb
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .store
    or al,20h
    cmp al,'a'
    jb .bad
    cmp al,'f'
    ja .bad
.store:
    stosb
    loop .hex
    mov byte [edi],0
.rest:
    lodsb
    test al,al
    jz .ok
    cmp al,' '
    je .rest
    cmp al,9
    je .rest
    cmp al,13
    je .rest
    cmp al,10
    je .rest
.bad:
    xor eax,eax
    ret
.ok:
    mov eax,1
    ret
endp

proc LoaderInstall uses ebx esi edi,baseIdA,kind,versionA,targetIdA
    locals
        response dd ?
        context dd ?
        result dd ?
        written dd ?
        target rb 128
        expected rb 128
        fullVersion rb 256
        url rb 4096
        mirror rb 4096
        shaUrl rb 4096
        sha rb 64
        wideBase rw 128
        wideVersion rw 128
        cache rw 2048
        installer rw 2048
        extracted rw 2048
        installDir rw 2048
        path rw 2048
        log rw 2048
        profiles rw 2048
    endl
    mov [response],0
    mov [context],0
    mov [result],0
    mov edi,[targetIdA]
    mov byte [edi],0
    stdcall LoaderSafeId,[baseIdA]
    test eax,eax
    jz .done
    stdcall LoaderSafeId,[versionA]
    test eax,eax
    jz .done
    cmp [engineCancel],0
    jne .done
    stdcall SetStatusA,ldInstallStatus
    mov eax,[kind]
    cmp eax,1
    je .fabric
    cmp eax,4
    je .quilt
    cmp eax,2
    je .forge
    cmp eax,3
    jne .done
    invoke lstrlenA,[versionA]
    cmp eax,118
    ja .done
    stdcall LdNeoPrefix,[baseIdA],addr fullVersion
    test eax,eax
    jz .done
    invoke lstrlenA,addr fullVersion
    mov ecx,eax
    lea esi,[fullVersion]
    mov edi,[versionA]
    repe cmpsb
    jne .done
    stdcall LdModernNeoStable,[baseIdA],[versionA]
    test eax,eax
    jz .done
    cinvoke wsprintfA,addr url,ldNeoJar,[versionA],[versionA]
    cinvoke wsprintfA,addr expected,ldNeoId,[versionA]
    mov byte [mirror],0
    jmp .installer
.forge:
    invoke lstrlenA,[baseIdA]
    mov ebx,eax
    invoke lstrlenA,[versionA]
    add eax,ebx
    cmp eax,119
    ja .done
    cinvoke wsprintfA,addr fullVersion,ldForgeFull,[baseIdA],[versionA]
    cinvoke wsprintfA,addr url,ldForgeJar,addr fullVersion,addr fullVersion
    cinvoke wsprintfA,addr mirror,ldForgeMirror,addr fullVersion,addr fullVersion
    cinvoke wsprintfA,addr expected,ldForgeId,[baseIdA],[versionA]
    jmp .installer
.fabric:
    cinvoke wsprintfA,addr url,ldFabricProfile,[baseIdA],[versionA]
    jmp .profile
.quilt:
    cinvoke wsprintfA,addr url,ldQuiltProfile,[baseIdA],[versionA]
.profile:
    stdcall LdReadJson,addr url,addr response
    mov [context],eax
    test eax,eax
    jz .done
    stdcall LdValidateProfile,eax,[baseIdA],addr target
    test eax,eax
    jz .done
    stdcall LdProfilePaths,addr target,addr installDir,addr path
    test eax,eax
    jz .done
    stdcall LdPublishProfile,addr path,addr installDir,[response]
    test eax,eax
    jz .done
    jmp .success
.installer:
    stdcall LoaderSafeId,addr expected
    test eax,eax
    jz .done
    invoke lstrlenW,gameDir
    cmp eax,1200
    ja .done
    invoke MultiByteToWideChar,65001,8,[baseIdA],-1,addr wideBase,128
    invoke MultiByteToWideChar,65001,8,[versionA],-1,addr wideVersion,128
    invoke lstrcpyW,addr cache,gameDir
    invoke lstrlenW,addr cache
    lea edi,[cache+eax*2]
    cinvoke wsprintfW,edi,ldCacheFormat,[kind],addr wideBase,addr wideVersion
    stdcall EnsureDirectory,addr cache
    test eax,eax
    jz .done
    invoke lstrcpyW,addr installer,addr cache
    invoke lstrcatW,addr installer,ldInstaller
    invoke lstrcpyW,addr extracted,addr cache
    invoke lstrcatW,addr extracted,ldVersionJson
    invoke lstrcpyA,addr shaUrl,addr url
    invoke lstrcatA,addr shaUrl,ldShaExt
    stdcall LdRequestText,addr shaUrl
    test eax,eax
    jz .done
    mov [response],eax
    stdcall LdSha1,eax,addr sha
    test eax,eax
    jz .done
    stdcall FreeMem,[response]
    mov [response],0
    cmp byte [mirror],0
    je .officialJar
    stdcall HttpDownload,addr mirror,addr installer,addr sha
    test eax,eax
    jnz .extract
.officialJar:
    cmp [engineCancel],0
    jne .done
    stdcall HttpDownload,addr url,addr installer,addr sha
    test eax,eax
    jz .done
.extract:
    invoke GetWindowsDirectoryW,eTarPath,990
    invoke lstrcatW,eTarPath,eTarNative
    invoke GetFileAttributesW,eTarPath
    cmp eax,-1
    jne .tar
    invoke GetSystemDirectoryW,eTarPath,990
    invoke lstrcatW,eTarPath,eTarName
.tar:
    stdcall LdCommandStart,eTarPath
    test eax,eax
    jz .done
    stdcall EAppendCommand,ldTarExtract
    stdcall LdCommandWide,addr installer
    stdcall EAppendCommand,ldTarC
    stdcall LdCommandWide,addr cache
    stdcall EAppendCommand,ldVersionEntry
    test eax,eax
    jz .done
    invoke lstrcpyW,addr log,addr cache
    invoke lstrcatW,addr log,ldExtractLog
    stdcall LdRun,eTarPath,addr cache,addr log
    test eax,eax
    jz .done
    stdcall EReadFile,addr extracted
    test eax,eax
    jz .done
    mov [response],eax
    stdcall JsonParse,eax
    mov [context],eax
    test eax,eax
    jz .done
    stdcall LdValidateProfile,eax,[baseIdA],addr target
    test eax,eax
    jz .done
    invoke lstrcmpA,addr target,addr expected
    test eax,eax
    jnz .done
    stdcall LdProfilePaths,addr target,addr installDir,addr path
    test eax,eax
    jz .done
    stdcall EnsureDirectory,addr installDir
    stdcall LdBackupProfile,addr path,addr installDir
    test eax,eax
    jz .done
    invoke lstrcpyW,addr profiles,gameDir
    invoke lstrcatW,addr profiles,ldProfiles
    invoke GetFileAttributesW,addr profiles
    cmp eax,-1
    jne .java
    invoke lstrcpyW,addr log,gameDir
    invoke lstrcatW,addr log,ldStoreProfiles
    invoke GetFileAttributesW,addr log
    cmp eax,-1
    jne .java
    ; CREATE_NEW never overwrites an existing launcher's configuration.
    invoke CreateFileW,addr profiles,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .done
    mov ebx,eax
    invoke lstrlenA,ldProfileStub
    mov esi,eax
    invoke WriteFile,ebx,ldProfileStub,esi,addr written,0
    mov edi,eax
    invoke CloseHandle,ebx
    test edi,edi
    jz .done
    cmp [written],esi
    jne .done
    mov [result],0
.java:
    stdcall LdCommandStart,javaPath
    test eax,eax
    jz .done
    stdcall EAppendCommand,ldJavaJar
    stdcall LdCommandWide,addr installer
    stdcall EAppendCommand,ldInstallArg
    stdcall LdCommandWide,gameDir
    test eax,eax
    jz .done
    invoke lstrcpyW,addr log,addr cache
    invoke lstrcatW,addr log,ldInstallerLog
    stdcall LdRun,javaPath,addr cache,addr log
    test eax,eax
    jz .done
    stdcall JsonFree,[context]
    mov [context],0
    stdcall FreeMem,[response]
    mov [response],0
    stdcall EReadFile,addr path
    test eax,eax
    jz .done
    mov [response],eax
    stdcall JsonParse,eax
    mov [context],eax
    test eax,eax
    jz .done
    stdcall LdValidateProfile,eax,[baseIdA],addr target
    test eax,eax
    jz .done
    invoke lstrcmpA,addr target,addr expected
    test eax,eax
    jnz .done
.success:
    cmp [engineCancel],0
    jne .done
    invoke lstrcpyA,[targetIdA],addr target
    mov [result],1
.done:
    cmp [context],0
    je .free
    stdcall JsonFree,[context]
.free:
    stdcall FreeMem,[response]
    cmp [result],1
    je .return
    mov [result],0
    cmp [engineCancel],0
    jne .cancel
    stdcall SetStatusA,ldFailure
    jmp .return
.cancel:
    stdcall SetStatusA,ldCancelled
.return:
    mov eax,[result]
    ret
endp
