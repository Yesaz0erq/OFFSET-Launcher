; OFFSET: original Minecraft source routing. URL layouts follow the public
; Mojang/BMCLAPI endpoints used by PCL's mirror-first configuration.
; Authentication and mod downloads do not pass through this module.
section '.mcsdata' data readable writeable
mcsMeta db 'https://piston-meta.mojang.com/',0
mcsData db 'https://piston-data.mojang.com/',0
mcsLauncher db 'https://launcher.mojang.com/',0
mcsLauncherMeta db 'https://launchermeta.mojang.com/',0
mcsLibraries db 'https://libraries.minecraft.net/',0
mcsResources db 'https://resources.download.minecraft.net/',0
mcsMirror db 'https://bmclapi2.bangbang93.com/',0
mcsMaven db 'https://bmclapi2.bangbang93.com/maven/',0
mcsLibraryMirror db 'https://bmclapi2.bangbang93.com/libraries/',0
mcsAssetMirror db 'https://bmclapi2.bangbang93.com/assets/',0
mcsMetaHosts dd mcsMeta,mcsData,mcsLauncher,mcsLauncherMeta,0
mcsLibraryHosts dd mcsLibraries,mcsMeta,mcsData,0
mcsAssetHosts dd mcsResources,0
mcsManifestMirror du 'https://bmclapi2.bangbang93.com/mc/game/version_manifest_v2.json',0
mcsManifestOfficial du 'https://piston-meta.mojang.com/mc/game/version_manifest_v2.json',0
mcsKeyVersions db 'versions',0
mcsKeyId db 'id',0
mcsKeyUrl db 'url',0
mcsKeySha db 'sha1',0
mcsHttps db 'https://',0

section '.mcscode' code readable executable
; Installer and launch-only harnesses may omit all routing procedures.
    nop
; Match an ASCII scheme/host prefix including its final slash. Return the
; original path pointer, preserving path case and escapes, or 0 on no match.
proc McSourcePrefix uses esi edi,source,prefix
    mov esi,[source]
    mov edi,[prefix]
.loop:
    mov al,[edi]
    test al,al
    jz .match
    mov dl,[esi]
    cmp dl,'A'
    jb .compare
    cmp dl,'Z'
    ja .compare
    add dl,'a'-'A'
.compare:
    cmp dl,al
    jne .no
    inc esi
    inc edi
    jmp .loop
.match:
    mov eax,esi
    ret
.no:
    xor eax,eax
    ret
endp

; kind: 0 metadata/client/index, 1 library, 2 resource object.
; variant: 0 primary mirror, 1 alternate library mirror only.
; Input/output aliasing is rejected without modifying the input.
proc McMirrorUrl uses ebx esi edi,urlA,destA,capacity,kind,variant
    locals
        base dd ?
        tail dd ?
        baseLength dd ?
        tailLength dd ?
    endl
    cmp [destA],0
    je .no
    mov eax,[urlA]
    cmp eax,[destA]
    je .no
    cmp [capacity],0
    je .no
    mov edi,[destA]
    mov byte [edi],0
    test eax,eax
    jz .no
    mov esi,eax
    xor ecx,ecx
.bounded:
    mov al,[esi+ecx]
    test al,al
    jz .select
    cmp ecx,4095
    jae .no
    cmp al,32
    jb .no
    inc ecx
    jmp .bounded
.select:
    test ecx,ecx
    jz .no
    cmp [kind],1
    je .library
    cmp [variant],0
    jne .no
    cmp [kind],0
    je .metadata
    cmp [kind],2
    jne .no
    mov ebx,mcsAssetHosts
    mov [base],mcsAssetMirror
    jmp .host
.metadata:
    mov ebx,mcsMetaHosts
    mov [base],mcsMirror
    jmp .host
.library:
    cmp [variant],1
    ja .no
    mov ebx,mcsLibraryHosts
    mov [base],mcsMaven
    cmp [variant],0
    je .host
    mov [base],mcsLibraryMirror
.host:
    mov esi,[ebx]
    test esi,esi
    jz .no
    stdcall McSourcePrefix,[urlA],esi
    test eax,eax
    jnz .matched
    add ebx,4
    jmp .host
.matched:
    mov [tail],eax
    invoke lstrlenA,eax
    mov [tailLength],eax
    invoke lstrlenA,[base]
    mov [baseLength],eax
    add eax,[tailLength]
    jc .no
    cmp eax,[capacity]
    jae .no
    mov esi,[base]
    mov edi,[destA]
    mov ecx,[baseLength]
    rep movsb
    mov esi,[tail]
    mov ecx,[tailLength]
    inc ecx
    rep movsb
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

; Each invocation owns its URL buffer, including concurrent resource workers.
; HttpDownload handles verified cache reuse, hashes and atomic file replacement.
proc McDownload uses ebx esi edi,urlA,pathW,expectedSha1A,kind
    local mirror[4096]:BYTE
    cmp [engineCancel],0
    jne .cancel
    stdcall McMirrorUrl,[urlA],addr mirror,4096,[kind],0
    test eax,eax
    jz .official
    cmp [engineCancel],0
    jne .cancel
    stdcall HttpDownload,addr mirror,[pathW],[expectedSha1A]
    cmp [engineCancel],0
    jne .cancel
    test eax,eax
    jnz .done
    cmp [kind],1
    jne .official
    stdcall McMirrorUrl,[urlA],addr mirror,4096,1,1
    test eax,eax
    jz .official
    cmp [engineCancel],0
    jne .cancel
    stdcall HttpDownload,addr mirror,[pathW],[expectedSha1A]
    cmp [engineCancel],0
    jne .cancel
    test eax,eax
    jnz .done
.official:
    cmp [engineCancel],0
    jne .cancel
    stdcall HttpDownload,[urlA],[pathW],[expectedSha1A]
    cmp [engineCancel],0
    jne .cancel
.done:
    ret
.cancel:
    xor eax,eax
    ret
endp

; A mirror must contain the requested version with usable URL/hash fields.
; JsonAt also validates the private token's array type, without depending on
; the JSON token layout here.
proc McManifestHasVersion uses ebx esi edi,ctx,list,versionA
    locals
        count dd ?
        idx dd ?
        item dd ?
        fieldLength dd ?
        id rb 128
        value rb 4096
    endl
    stdcall JsonCount,[ctx],[list]
    mov [count],eax
    mov [idx],0
.loop:
    mov eax,[idx]
    cmp eax,[count]
    jae .no
    cmp [engineCancel],0
    jne .no
    stdcall JsonAt,[ctx],[list],eax
    test eax,eax
    jz .no
    mov [item],eax
    stdcall JsonGet,[ctx],eax,mcsKeyId
    stdcall JsonString,[ctx],eax,addr id,128
    test eax,eax
    jz .next
    cmp eax,128
    jae .next
    mov [fieldLength],eax
    invoke lstrlenA,addr id
    cmp eax,[fieldLength]
    jne .next
    invoke lstrcmpA,addr id,[versionA]
    test eax,eax
    jnz .next
    stdcall JsonGet,[ctx],[item],mcsKeyUrl
    stdcall JsonString,[ctx],eax,addr value,4096
    cmp eax,8
    jbe .no
    cmp eax,4096
    jae .no
    mov [fieldLength],eax
    invoke lstrlenA,addr value
    cmp eax,[fieldLength]
    jne .no
    stdcall McSourcePrefix,addr value,mcsHttps
    test eax,eax
    jz .no
    stdcall JsonGet,[ctx],[item],mcsKeySha
    stdcall JsonString,[ctx],eax,addr value,4096
    cmp eax,40
    jne .no
    lea esi,[value]
    mov ecx,40
.hash:
    lodsb
    cmp al,'0'
    jb .no
    cmp al,'9'
    jbe .hex
    or al,20h
    cmp al,'a'
    jb .no
    cmp al,'f'
    ja .no
.hex:
    loop .hash
    mov eax,1
    ret
.next:
    inc [idx]
    jmp .loop
.no:
    xor eax,eax
    ret
endp

; Return an owned UTF-8 manifest response or 0. Keep v2 to retain SHA-1 values.
; A stale/invalid mirror falls through to Mojang. A valid official list may
; omit the requested ID so the engine can preserve its unknown-version error.
proc McManifestRequest uses ebx esi edi,versionA
    locals
        stage dd ?
        response dd ?
        context dd ?
        list dd ?
    endl
    mov [stage],0
    mov [response],0
    mov [context],0
    cmp [versionA],0
    je .failed
.request:
    cmp [engineCancel],0
    jne .failed
    mov eax,mcsManifestMirror
    cmp [stage],0
    je .transport
    mov eax,mcsManifestOfficial
.transport:
    stdcall HttpRequest,eax,0,0,0
    mov [response],eax
    test eax,eax
    jz .next
    cmp [engineCancel],0
    jne .failed
    cmp [httpStatus],200
    jne .next
    stdcall JsonParse,[response]
    mov [context],eax
    test eax,eax
    jz .next
    stdcall JsonRoot,eax
    stdcall JsonGet,[context],eax,mcsKeyVersions
    mov [list],eax
    test eax,eax
    jz .next
    stdcall JsonAt,[context],eax,0
    test eax,eax
    jz .next
    cmp [stage],1
    je .accepted
    stdcall McManifestHasVersion,[context],[list],[versionA]
    test eax,eax
    jz .next
.accepted:
    cmp [engineCancel],0
    jne .failed
    stdcall JsonFree,[context]
    mov eax,[response]
    ret
.next:
    cmp [context],0
    je .freeResponse
    stdcall JsonFree,[context]
    mov [context],0
.freeResponse:
    stdcall FreeMem,[response]
    mov [response],0
    cmp [engineCancel],0
    jne .failed
    inc [stage]
    cmp [stage],2
    jb .request
.failed:
    cmp [context],0
    je .freeFailed
    stdcall JsonFree,[context]
.freeFailed:
    stdcall FreeMem,[response]
    xor eax,eax
    ret
endp
