; Synchronize mod filters once when the selected installed profile changes.
; Runs on the idle UI thread; transient buffers are never shared with a worker.
section '.mpdata' data readable writeable
mpVersions du '\versions\',0
mpSlash du '\',0
mpJson du '.json',0
mpKeyId db 'id',0
mpKeyNormalized db 'offsetNormalized',0
mpKeyBase db 'offsetBase',0
mpKeyInherits db 'inheritsFrom',0
mpKeyLibraries db 'libraries',0
mpKeyName db 'name',0
mpFabricCoordinate db 'net.fabricmc:fabric-loader:',0
mpForgeCoordinate db 'net.minecraftforge:forge:',0
mpNeoCoordinate db 'net.neoforged:neoforge:',0
mpQuiltCoordinate db 'org.quiltmc:quilt-loader:',0
mpFabric db 'fabric',0
mpForge db 'forge',0
mpNeo db 'neoforge',0
mpQuilt db 'quilt',0
mpLoaderNames dd mpFabric,mpForge,mpNeo,mpQuilt
mpLastProfile rb 128
mpLastGameDir rw 1024
mpPath rw 2048
mpWideId rw 128
mpBase rb 64
mpId rb 128
mpLibrary rb 512

section '.mpcode' code readable executable
    nop

; JSON string with a real terminator, no truncation or decoded embedded NUL.
proc MpString uses ebx,ctx,token,destination,capacity
    stdcall JsonString,[ctx],[token],[destination],[capacity]
    test eax,eax
    jz .bad
    cmp eax,[capacity]
    jae .bad
    mov ebx,eax
    invoke lstrlenA,[destination]
    cmp eax,ebx
    jne .bad
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc MpId uses esi,source,capacity,numeric
    stdcall VersionsSafeId,[source]
    test eax,eax
    jz .bad
    invoke lstrlenA,[source]
    cmp eax,[capacity]
    jae .bad
    cmp [numeric],0
    je .good
    mov esi,[source]
    cmp byte [esi],'0'
    jb .bad
    cmp byte [esi],'9'
    ja .bad
.good:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; Maven names are ASCII coordinates. Never accept a truncated prefix match.
proc MpCoordinate uses esi,source
    mov esi,[source]
.next:
    lodsb
    test al,al
    jz .good
    cmp al,'A'
    jb .lower
    cmp al,'Z'
    jbe .next
.lower:
    cmp al,'a'
    jb .digit
    cmp al,'z'
    jbe .next
.digit:
    cmp al,'0'
    jb .punctuation
    cmp al,'9'
    jbe .next
.punctuation:
    cmp al,'.'
    je .next
    cmp al,'-'
    je .next
    cmp al,'_'
    je .next
    cmp al,':'
    je .next
    cmp al,'+'
    je .next
    cmp al,'@'
    je .next
    xor eax,eax
    ret
.good:
    mov eax,1
    ret
endp

proc MpPrefix uses esi edi,source,prefix
    mov esi,[source]
    mov edi,[prefix]
.next:
    mov al,[edi]
    test al,al
    jz .version
    cmp al,[esi]
    jne .bad
    inc esi
    inc edi
    jmp .next
.version:
    cmp byte [esi],0
    je .bad
    cmp byte [esi],':'
    je .bad
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; Prefer platform loaders over compatibility libraries in a mixed classpath.
proc MpFindLoader uses ebx esi edi,ctx,root
    local list:DWORD, item:DWORD, count:DWORD, index:DWORD, flags:DWORD
    mov [flags],0
    stdcall JsonGet,[ctx],[root],mpKeyLibraries
    mov [list],eax
    stdcall JsonCount,[ctx],eax
    mov [count],eax
    cmp eax,4096
    ja .unknown
    test eax,eax
    jz .unknown
    stdcall JsonAt,[ctx],[list],0
    test eax,eax
    jz .unknown
    mov [item],eax
    mov [index],0
.next:
    mov eax,[index]
    cmp eax,[count]
    jae .choose
    stdcall JsonGet,[ctx],[item],mpKeyName
    stdcall MpString,[ctx],eax,mpLibrary,512
    test eax,eax
    jz .advance
    stdcall MpCoordinate,mpLibrary
    test eax,eax
    jz .advance
    stdcall MpPrefix,mpLibrary,mpNeoCoordinate
    test eax,eax
    jnz .neo
    stdcall MpPrefix,mpLibrary,mpForgeCoordinate
    test eax,eax
    jz .quilt
    or [flags],2
    jmp .advance
.quilt:
    stdcall MpPrefix,mpLibrary,mpQuiltCoordinate
    test eax,eax
    jz .fabric
    or [flags],8
    jmp .advance
.fabric:
    stdcall MpPrefix,mpLibrary,mpFabricCoordinate
    test eax,eax
    jz .advance
    or [flags],1
.advance:
    stdcall JsonNext,[ctx],[item]
    mov [item],eax
    inc [index]
    jmp .next
.choose:
    test [flags],2
    jnz .forge
    test [flags],8
    jnz .quiltResult
    test [flags],1
    jz .unknown
    xor eax,eax
    ret
.forge:
    mov eax,1
    ret
.neo:
    mov eax,2
    ret
.quiltResult:
    mov eax,3
    ret
.unknown:
    or eax,-1
    ret
endp

proc ModSyncSelectedProfile uses ebx esi edi
    local response:DWORD, context:DWORD, root:DWORD, kind:DWORD, apply:DWORD
    cmp [busy],0
    jne .done
    stdcall MpId,selectedVersion,128,0
    test eax,eax
    jz .done
    invoke lstrlenW,gameDir
    test eax,eax
    jz .done
    cmp eax,1024
    jae .done
    invoke lstrcmpA,selectedVersion,mpLastProfile
    test eax,eax
    jnz .changed
    ; The same profile name in another game root is a different installation.
    invoke lstrcmpiW,gameDir,mpLastGameDir
    test eax,eax
    jz .done
.changed:
    invoke lstrcpyA,mpLastProfile,selectedVersion
    invoke lstrcpyW,mpLastGameDir,gameDir
    mov [response],0
    mov [context],0
    mov [kind],-1
    mov [apply],0
    invoke lstrcpyW,mpPath,gameDir
    invoke lstrcatW,mpPath,mpVersions
    invoke MultiByteToWideChar,65001,8,selectedVersion,-1,mpWideId,128
    test eax,eax
    jz .cleanup
    invoke lstrcatW,mpPath,mpWideId
    invoke lstrcatW,mpPath,mpSlash
    invoke lstrcatW,mpPath,mpWideId
    invoke lstrcatW,mpPath,mpJson
    stdcall EReadFile,mpPath
    test eax,eax
    jz .cleanup
    mov [response],eax
    stdcall JsonParse,eax
    test eax,eax
    jz .cleanup
    mov [context],eax
    stdcall JsonRoot,eax
    mov [root],eax
    stdcall JsonGet,[context],eax,mpKeyId
    stdcall MpString,[context],eax,mpId,128
    test eax,eax
    jz .cleanup
    invoke lstrcmpA,mpId,selectedVersion
    test eax,eax
    jnz .cleanup
    stdcall JsonGet,[context],[root],mpKeyInherits
    test eax,eax
    jnz .cleanup
    stdcall JsonGet,[context],[root],mpKeyNormalized
    test eax,eax
    jz .vanilla
    stdcall JsonBool,[context],eax
    cmp eax,1
    jne .cleanup
    stdcall JsonGet,[context],[root],mpKeyBase
    stdcall MpString,[context],eax,mpBase,64
    test eax,eax
    jz .cleanup
    stdcall MpId,mpBase,64,1
    test eax,eax
    jz .cleanup
    stdcall MpFindLoader,[context],[root]
    mov [kind],eax
    jmp .accepted
.vanilla:
    stdcall MpId,selectedVersion,64,1
    test eax,eax
    jz .cleanup
    invoke lstrcpyA,mpBase,selectedVersion
.accepted:
    mov [apply],1
.cleanup:
    cmp [context],0
    je .freeResponse
    stdcall JsonFree,[context]
.freeResponse:
    stdcall FreeMem,[response]
    cmp [apply],0
    je .done
    invoke lstrcpyA,modGameVersion,mpBase
    invoke SetWindowTextA,[hModGame],modGameVersion
    mov ebx,[kind]
    cmp ebx,3
    ja .clear
    invoke lstrcpyA,modLoader,dword [mpLoaderNames+ebx*4]
    invoke SendMessageW,[hModLoader],CB_SETCURSEL,ebx,0
.clear:
    stdcall ClearModResults
.done:
    ret
endp
