; OFFSET 0.2 - native shared mod model and local file management.
section '.moddata' data readable writeable
modProvider dd 0
modCancel dd 0
modProjectCount dd 0
modFileCount dd 0
modLocalCount dd 0
modSelectedProject dd -1
modSelectedFile dd -1
modSelectedLocal dd -1
modOffset dd 0
modTotal dd 0
modGameVersion db '1.21.1',0,57 dup(0)
modLoader db 'fabric',0,9 dup(0)
mlSerial dd 0
modContextActive dd 0
mlModsSuffix du '\mods',0
mlScanSuffix du '\mods\*',0
mlSlash du '\',0
mlJar du '.jar',0
mlDisabled du '.disabled',0
mlJarDisabled du '.jar.disabled',0
mlTrashSuffix du '\mods\.offset-trash',0
mlCacheSuffix du '\.offset-mod-cache\',0
mlKeySuffix du '\offset.cfkey',0
mlKeyTempSuffix du '.new',0
mlTrashFormat du '%08X-%u',0
mlHex db '0123456789ABCDEF',0
mlScheme db 'https://',0
mlMrHost db 'cdn.modrinth.com',0
mlCfHost db 'forgecdn.net',0
mlStatusScan db '本地模组列表已刷新。',0
mlStatusLimit db '本地模组超过 512 个，仅显示前 512 个。',0
mlStatusToggle db '模组启用状态已更新。',0
mlStatusTrash db '模组已移入 mods/.offset-trash，可从该文件夹恢复。',0
mlErrFile db '未选择有效模组，或文件名称无效。',0
mlErrMove db '文件操作失败：目标已存在、文件被占用或目录不可写。原文件已保留。',0
mlErrDownload db '模组下载或完整性校验失败，请检查网络后重试。',0
mlErrBlocked db '该文件没有可用的官方下载地址或 SHA-1，请打开项目官网获取。',0
mlErrCollision db '同名模组已存在且内容不同；请先禁用或移入回收区。未覆盖原文件。',0
mlErrCancel db '模组操作已取消。',0
mlErrZip db '下载内容不是有效的 JAR/ZIP 文件，未放入 mods。',0
mlStatusDownload db '正在下载并校验模组…',0
mlStatusInstalled db '模组已下载到 mods。请确认已安装对应加载器和必需依赖。',0
mlStatusDependencies db '模组已下载；此文件声明了必需依赖，请根据详情另行安装。',0
mlStatusSame db '相同内容的模组已经存在，无需重复下载。',0
mlStatusKey db 'CurseForge API Key 已在本机加密保存。',0
mlStatusKeyRemoved db '已清除本机保存的 CurseForge API Key。',0
mlErrKey db 'API Key 无效：不能包含控制字符，且长度不能超过 255 字节。',0
mlErrKeySave db 'API Key 加密保存失败，请检查启动器目录的写入权限。',0
; Large buffers are virtual storage only.
modQuery rb 512
curseApiKey rb 256
modProjects rb MP_SIZE*MOD_MAX_PROJECTS
modFiles rb MF_SIZE*MOD_MAX_FILES
modLocals rb ML_SIZE*MOD_MAX_LOCAL
modContextRootW rw 2048

section '.modcode' code readable executable

proc ModsInit
    mov [modSelectedProject],-1
    mov [modSelectedFile],-1
    mov [modSelectedLocal],-1
    ret
endp

; An explicit data root is scoped only around a version-page operation.
; Ordinary mod-center operations resolve the current profile without changing
; gameDir (the shared client/library cache root) or selectedVersion.
proc ModsSetDataRoot uses esi,rootW
    cmp [rootW],0
    je .clear
    mov esi,[rootW]
    cmp word [esi],0
    je .clear
    mov [modContextActive],-1
    invoke lstrlenW,esi
    cmp eax,3
    jb .bad
    cmp eax,1700
    ja .bad
    invoke lstrcpyW,modContextRootW,esi
    mov [modContextActive],1
    mov eax,1
    ret
.clear:
    mov [modContextActive],0
    mov word [modContextRootW],0
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModsDataRoot uses ebx esi,destination,capacity
    cmp [capacity],1
    jb .bad
    cmp [modContextActive],0
    je .current
    cmp [modContextActive],1
    jne .bad
    mov esi,modContextRootW
    jmp .copy
.current:
if defined OFFSET_VERSION_SETTINGS
    stdcall VsResolveData,gameDir,selectedVersion,[destination],[capacity]
    test eax,eax
    jz .bad
    mov esi,[destination]
    jmp .validate
else
    mov esi,gameDir
end if
.copy:
    invoke lstrlenW,esi
    cmp eax,[capacity]
    jae .bad
    invoke lstrcpyW,[destination],esi
    mov esi,[destination]
.validate:
    invoke lstrlenW,esi
    cmp eax,3
    jb .bad
    cmp eax,1700
    ja .bad
    mov eax,1
    ret
.bad:
    cmp [capacity],0
    je .return
    mov eax,[destination]
    mov word [eax],0
.return:
    xor eax,eax
    ret
endp

proc ModAppend uses ebx esi edi,destination,capacity,source
    mov edi,[destination]
    mov ebx,[capacity]
    xor ecx,ecx
.find:
    cmp ecx,ebx
    jae .bad
    cmp byte [edi+ecx],0
    je .length
    inc ecx
    jmp .find
.length:
    mov esi,[source]
    xor edx,edx
.measure:
    mov eax,ecx
    add eax,edx
    cmp eax,ebx
    jae .bad
    cmp byte [esi+edx],0
    je .copy
    inc edx
    jmp .measure
.copy:
    add edi,ecx
    mov ecx,edx
    inc ecx
    rep movsb
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModUrlEncode uses ebx esi edi,source,destination,capacity
    mov esi,[source]
    mov edi,[destination]
    xor ebx,ebx
    cmp [capacity],0
    je .bad
.loop:
    movzx eax,byte [esi]
    test al,al
    jz .done
    cmp al,'a'
    jb .upper
    cmp al,'z'
    jbe .plain
.upper:
    cmp al,'A'
    jb .digit
    cmp al,'Z'
    jbe .plain
.digit:
    cmp al,'0'
    jb .symbols
    cmp al,'9'
    jbe .plain
.symbols:
    cmp al,'-'
    je .plain
    cmp al,'_'
    je .plain
    cmp al,'.'
    je .plain
    cmp al,'~'
    je .plain
    lea edx,[ebx+3]
    cmp edx,[capacity]
    jae .bad
    mov byte [edi+ebx],'%'
    mov edx,eax
    shr edx,4
    mov dl,[mlHex+edx]
    mov [edi+ebx+1],dl
    and eax,15
    mov al,[mlHex+eax]
    mov [edi+ebx+2],al
    add ebx,3
    inc esi
    jmp .loop
.plain:
    lea edx,[ebx+1]
    cmp edx,[capacity]
    jae .bad
    mov [edi+ebx],al
    inc ebx
    inc esi
    jmp .loop
.done:
    mov byte [edi+ebx],0
    mov eax,ebx
    ret
.bad:
    cmp [capacity],0
    je .failed
    mov byte [edi],0
.failed:
    mov eax,-1
    ret
endp

proc ModIsSha1 uses esi,source
    mov esi,[source]
    xor ecx,ecx
.loop:
    mov al,[esi+ecx]
    cmp ecx,40
    je .complete
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .next
    or al,32
    cmp al,'a'
    jb .bad
    cmp al,'f'
    ja .bad
.next:
    inc ecx
    jmp .loop
.complete:
    test al,al
    jnz .bad
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModNumber uses esi,source
    mov esi,[source]
    xor eax,eax
    xor ecx,ecx
.loop:
    movzx edx,byte [esi+ecx]
    test dl,dl
    jz .done
    sub edx,'0'
    cmp edx,9
    ja .bad
    cmp eax,214748364
    ja .bad
    imul eax,10
    add eax,edx
    test eax,eax
    js .bad
    inc ecx
    cmp ecx,10
    ja .bad
    jmp .loop
.done:
    test ecx,ecx
    jz .bad
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModSafeFilename uses ebx esi edi,source
    locals
        wide rw 280
    endl
    mov esi,[source]
    xor ecx,ecx
.measure:
    cmp ecx,240
    jae .bad
    cmp byte [esi+ecx],0
    je .convert
    inc ecx
    jmp .measure
.convert:
    cmp ecx,5
    jb .bad
    invoke MultiByteToWideChar,65001,8,[source],-1,addr wide,280
    test eax,eax
    jz .bad
    lea esi,[wide]
    cmp word [esi],'.'
    je .bad
    cmp word [esi],' '
    je .bad
    xor ebx,ebx
.chars:
    movzx eax,word [esi+ebx*2]
    test eax,eax
    jz .suffix
    cmp eax,32
    jb .bad
    cmp eax,127
    je .bad
    cmp eax,'\'
    je .bad
    cmp eax,'/'
    je .bad
    cmp eax,':'
    je .bad
    cmp eax,'*'
    je .bad
    cmp eax,'?'
    je .bad
    cmp eax,'"'
    je .bad
    cmp eax,'<'
    je .bad
    cmp eax,'>'
    je .bad
    cmp eax,'|'
    je .bad
    cmp eax,'.'
    jne .next
    cmp word [esi+ebx*2+2],'.'
    je .bad
    test ebx,ebx
    jz .bad
    cmp word [esi+ebx*2-2],' '
    je .bad
.next:
    inc ebx
    jmp .chars
.suffix:
    cmp ebx,5
    jb .bad
    lea eax,[esi+ebx*2-8]
    invoke lstrcmpiW,eax,mlJar
    test eax,eax
    jnz .bad
    ; Device names stay reserved even if another extension follows them.
    xor ecx,ecx
    xor edi,edi
.base:
    movzx eax,word [esi+ecx*2]
    cmp eax,'.'
    je .checkBase
    cmp ecx,8
    jae .okay
    cmp eax,127
    ja .okay
    or al,32
    cmp ecx,4
    jae .moreBase
    push ecx
    shl ecx,3
    shl eax,cl
    or edi,eax
    pop ecx
.moreBase:
    inc ecx
    jmp .base
.checkBase:
    cmp ecx,3
    jne .four
    cmp edi,'con'
    je .bad
    cmp edi,'prn'
    je .bad
    cmp edi,'aux'
    je .bad
    cmp edi,'nul'
    je .bad
.four:
    cmp ecx,4
    jne .okay
    mov eax,edi
    and eax,00FFFFFFh
    cmp eax,'com'
    je .deviceDigit
    cmp eax,'lpt'
    jne .okay
.deviceDigit:
    shr edi,24
    cmp edi,'1'
    jb .okay
    cmp edi,'9'
    jbe .bad
.okay:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModTrustedUrl uses ebx esi edi,url,provider
    locals
        host rb 256
    endl
    mov esi,[url]
    mov edi,mlScheme
    mov ecx,8
    repe cmpsb
    jne .bad
    lea edi,[host]
    xor ebx,ebx
.host:
    lodsb
    test al,al
    jz .bad
    cmp al,'/'
    je .hostEnd
    cmp ebx,253
    jae .bad
    cmp al,'A'
    jb .valid
    cmp al,'Z'
    ja .valid
    or al,32
.valid:
    cmp al,'a'
    jb .numbers
    cmp al,'z'
    jbe .save
.numbers:
    cmp al,'0'
    jb .symbols
    cmp al,'9'
    jbe .save
.symbols:
    cmp al,'.'
    je .save
    cmp al,'-'
    jne .bad
.save:
    stosb
    inc ebx
    jmp .host
.hostEnd:
    mov byte [edi],0
    cmp [provider],0
    jne .curse
    invoke lstrcmpA,addr host,mlMrHost
    test eax,eax
    jnz .bad
    jmp .path
.curse:
    cmp [provider],1
    jne .bad
    cmp ebx,12
    jb .bad
    lea eax,[host+ebx-12]
    invoke lstrcmpA,eax,mlCfHost
    test eax,eax
    jnz .bad
    cmp ebx,12
    je .path
    cmp byte [host+ebx-13],'.'
    jne .bad
.path:
    ; No spaces, controls, backslashes, credentials or fragments in paths.
    xor ecx,ecx
.pathLoop:
    lodsb
    test al,al
    jz .okay
    cmp ecx,1900
    jae .bad
    cmp al,33
    jb .bad
    cmp al,127
    jae .bad
    cmp al,'\'
    je .bad
    cmp al,'#'
    je .bad
    inc ecx
    jmp .pathLoop
.okay:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; returns 1 .jar / 2 .jar.disabled / 0 other.
proc ModLocalKind uses esi,nameW
    invoke lstrlenW,[nameW]
    mov esi,[nameW]
    cmp eax,5
    jb .bad
    push eax
    lea eax,[esi+eax*2-8]
    invoke lstrcmpiW,eax,mlJar
    test eax,eax
    pop eax
    jz .enabled
    cmp eax,14
    jb .bad
    lea eax,[esi+eax*2-26]
    invoke lstrcmpiW,eax,mlJarDisabled
    test eax,eax
    jnz .bad
    mov eax,2
    ret
.enabled:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModsScan uses ebx esi edi
    locals
        find WIN32_FIND_DATAW
        path rw 2048
        handle dd ?
    endl
    mov [modLocalCount],0
    mov [modSelectedLocal],-1
    mov edi,modLocals
    xor eax,eax
    mov ecx,ML_SIZE*MOD_MAX_LOCAL/4
    rep stosd
    stdcall ModsDataRoot,addr path,2048
    test eax,eax
    jz .bad
    invoke lstrcatW,addr path,mlScanSuffix
    invoke FindFirstFileW,addr path,addr find
    cmp eax,-1
    je .empty
    mov [handle],eax
.loop:
    test [find.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY or 400h
    jnz .next
    stdcall ModLocalKind,addr find.cFileName
    test eax,eax
    jz .next
    mov ebx,eax
    mov eax,[modLocalCount]
    cmp eax,MOD_MAX_LOCAL
    jae .limit
    imul eax,ML_SIZE
    lea edi,[modLocals+eax]
    dec ebx
    mov [edi+ML_DISABLED],ebx
    invoke lstrcpyW,edi,addr find.cFileName
    inc [modLocalCount]
.next:
    invoke FindNextFileW,[handle],addr find
    test eax,eax
    jnz .loop
    invoke FindClose,[handle]
.empty:
    mov eax,[modLocalCount]
    ret
.limit:
    invoke FindClose,[handle]
    stdcall SetStatusA,mlStatusLimit
    mov eax,[modLocalCount]
    ret
.bad:
    stdcall SetStatusA,mlErrFile
    xor eax,eax
    ret
endp

; Build a selected exact local path after revalidating its basename.
proc ModLocalPath uses ebx esi,index,destination
    cmp [index],0
    jl .bad
    mov eax,[index]
    cmp eax,[modLocalCount]
    jae .bad
    imul eax,ML_SIZE
    lea ebx,[modLocals+eax]
    stdcall ModLocalKind,ebx
    test eax,eax
    jz .bad
    mov esi,ebx
.safe:
    lodsw
    test ax,ax
    jz .make
    cmp ax,'\'
    je .bad
    cmp ax,'/'
    je .bad
    cmp ax,':'
    je .bad
    cmp ax,32
    jb .bad
    jmp .safe
.make:
    stdcall ModsDataRoot,[destination],2048
    test eax,eax
    jz .bad
    invoke lstrcatW,[destination],mlModsSuffix
    invoke lstrcatW,[destination],mlSlash
    invoke lstrcatW,[destination],ebx
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModsToggle uses ebx esi edi,index
    locals
        source rw 2048
        target rw 2048
    endl
    stdcall ModLocalPath,[index],addr source
    test eax,eax
    jz .invalid
    invoke lstrcpyW,addr target,addr source
    mov eax,[index]
    imul eax,ML_SIZE
    cmp [modLocals+eax+ML_DISABLED],0
    jne .enable
    invoke lstrcatW,addr target,mlDisabled
    jmp .move
.enable:
    invoke lstrlenW,addr target
    mov word [target+eax*2-18],0
.move:
    invoke MoveFileExW,addr source,addr target,8
    test eax,eax
    jz .bad
    stdcall SetStatusA,mlStatusToggle
    mov eax,1
    ret
.invalid:
    stdcall SetStatusA,mlErrFile
    xor eax,eax
    ret
.bad:
    stdcall SetStatusA,mlErrMove
    xor eax,eax
    ret
endp

proc ModsTrash uses ebx esi edi,index
    locals
        source rw 2048
        target rw 2048
        prefix rw 64
        tries dd ?
    endl
    stdcall ModLocalPath,[index],addr source
    test eax,eax
    jz .invalid
    stdcall ModsDataRoot,addr target,2048
    test eax,eax
    jz .invalid
    invoke lstrcatW,addr target,mlTrashSuffix
    stdcall EnsureDirectory,addr target
    test eax,eax
    jz .bad
    mov [tries],8
.attempt:
    stdcall ModsDataRoot,addr target,2048
    test eax,eax
    jz .invalid
    invoke lstrcatW,addr target,mlTrashSuffix
    invoke lstrcatW,addr target,mlSlash
    inc [mlSerial]
    invoke GetTickCount
    cinvoke wsprintfW,addr prefix,mlTrashFormat,eax,[mlSerial]
    invoke lstrcatW,addr target,addr prefix
    ; Keep the original full basename, even when it is near the filesystem
    ; filename limit. A unique subdirectory avoids lengthening the basename.
    stdcall EnsureDirectory,addr target
    test eax,eax
    jz .bad
    invoke lstrcatW,addr target,mlSlash
    mov eax,[index]
    imul eax,ML_SIZE
    add eax,modLocals
    invoke lstrcatW,addr target,eax
    invoke MoveFileExW,addr source,addr target,8
    test eax,eax
    jnz .okay
    invoke GetLastError
    cmp eax,183
    je .retry
    cmp eax,80
    jne .bad
.retry:
    dec [tries]
    jnz .attempt
    jmp .bad
.okay:
    stdcall SetStatusA,mlStatusTrash
    mov eax,1
    ret
.invalid:
    stdcall SetStatusA,mlErrFile
    xor eax,eax
    ret
.bad:
    stdcall SetStatusA,mlErrMove
    xor eax,eax
    ret
endp

proc ModsInstall uses ebx esi edi,index
    locals
        cached rw 2048
        target rw 2048
        nameW rw 280
        handle dd ?
        got dd ?
        signature dd ?
    endl
    cmp [modCancel],0
    jne .cancel
    mov eax,[index]
    cmp eax,[modFileCount]
    jae .invalid
    imul eax,MF_SIZE
    lea ebx,[modFiles+eax]
    cmp dword [ebx+MF_BLOCKED],0
    jne .blocked
    lea eax,[ebx+MF_NAME]
    stdcall ModSafeFilename,eax
    test eax,eax
    jz .invalid
    lea eax,[ebx+MF_SHA1]
    stdcall ModIsSha1,eax
    test eax,eax
    jz .blocked
    lea eax,[ebx+MF_URL]
    stdcall ModTrustedUrl,eax,[modProvider]
    test eax,eax
    jz .blocked
    lea eax,[ebx+MF_NAME]
    invoke MultiByteToWideChar,65001,8,eax,-1,addr nameW,280
    test eax,eax
    jz .invalid
    stdcall ModsDataRoot,addr target,2048
    test eax,eax
    jz .invalid
    invoke lstrcatW,addr target,mlModsSuffix
    stdcall EnsureDirectory,addr target
    test eax,eax
    jz .moveBad
    invoke lstrcatW,addr target,mlSlash
    invoke lstrcatW,addr target,addr nameW
    invoke GetFileAttributesW,addr target
    cmp eax,-1
    je .download
    lea eax,[ebx+MF_SHA1]
    stdcall VerifySha1,addr target,eax
    test eax,eax
    jnz .same
    stdcall SetStatusA,mlErrCollision
    xor eax,eax
    ret
.download:
    stdcall ModsDataRoot,addr cached,2048
    test eax,eax
    jz .invalid
    invoke lstrcatW,addr cached,mlCacheSuffix
    lea eax,[ebx+MF_SHA1]
    invoke MultiByteToWideChar,65001,0,eax,-1,addr nameW,280
    invoke lstrcatW,addr cached,addr nameW
    invoke lstrcatW,addr cached,mlJar
    stdcall SetStatusA,mlStatusDownload
    lea eax,[ebx+MF_URL]
    lea ecx,[ebx+MF_SHA1]
    stdcall HttpDownload,eax,addr cached,ecx
    test eax,eax
    jz .downloadBad
    cmp [modCancel],0
    jne .cancel
    ; Recheck checksum even when a transport fixture supplies a file directly.
    lea eax,[ebx+MF_SHA1]
    stdcall VerifySha1,addr cached,eax
    test eax,eax
    jz .downloadBad
    invoke CreateFileW,addr cached,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .downloadBad
    mov [handle],eax
    mov [signature],0
    mov [got],0
    invoke ReadFile,eax,addr signature,4,addr got,0
    mov esi,eax
    invoke CloseHandle,[handle]
    test esi,esi
    jz .zipBad
    cmp [got],4
    jne .zipBad
    cmp [signature],04034B50h
    je .place
    cmp [signature],06054B50h
    jne .zipBad
.place:
    ; Never replace an existing file, including a file created during download.
    invoke MoveFileExW,addr cached,addr target,8
    test eax,eax
    jz .moveBad
    mov eax,mlStatusInstalled
    cmp dword [ebx+MF_REQUIRED],0
    je .status
    mov eax,mlStatusDependencies
.status:
    stdcall SetStatusA,eax
    mov eax,1
    ret
.same:
    stdcall SetStatusA,mlStatusSame
    mov eax,1
    ret
.invalid:
    stdcall SetStatusA,mlErrFile
    xor eax,eax
    ret
.blocked:
    stdcall SetStatusA,mlErrBlocked
    xor eax,eax
    ret
.cancel:
    stdcall SetStatusA,mlErrCancel
    xor eax,eax
    ret
.downloadBad:
    stdcall SetStatusA,mlErrDownload
    xor eax,eax
    ret
.zipBad:
    stdcall SetStatusA,mlErrZip
    xor eax,eax
    ret
.moveBad:
    stdcall SetStatusA,mlErrMove
    xor eax,eax
    ret
endp

; Key validation is shared with the masked editor and storage. No controls.
proc ModKeyValid uses esi
    mov esi,curseApiKey
    xor ecx,ecx
.loop:
    cmp ecx,256
    jae .bad
    mov al,[esi+ecx]
    test al,al
    jz .okay
    cmp al,33
    jb .bad
    cmp al,127
    jae .bad
    inc ecx
    jmp .loop
.okay:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModKeyPath uses esi edi,destination
    invoke GetModuleFileNameW,0,[destination],2048
    test eax,eax
    jz .bad
    cmp eax,1900
    jae .bad
    mov esi,[destination]
    lea edi,[esi+eax*2]
.back:
    cmp edi,esi
    jbe .bad
    sub edi,2
    cmp word [edi],'\'
    jne .back
    mov word [edi],0
    invoke lstrcatW,[destination],mlKeySuffix
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ModsLoadKey uses ebx esi edi
    locals
        path rw 2048
        blobIn rd 2
        blobOut rd 2
        buffer rb 4096
        got dd ?
        handle dd ?
        okay dd ?
    endl
    mov [okay],0
    mov dword [blobOut],0
    mov dword [blobOut+4],0
    mov edi,curseApiKey
    xor eax,eax
    mov ecx,64
    rep stosd
    stdcall ModKeyPath,addr path
    test eax,eax
    jz .finish
    invoke CreateFileW,addr path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .finish
    mov [handle],eax
    invoke GetFileSize,eax,0
    cmp eax,16
    jb .close
    cmp eax,4096
    ja .close
    mov [blobIn],eax
    lea ecx,[buffer]
    mov [blobIn+4],ecx
    invoke ReadFile,[handle],addr buffer,eax,addr got,0
    test eax,eax
    jz .close
    mov eax,[got]
    cmp eax,[blobIn]
    jne .close
    invoke CryptUnprotectData,addr blobIn,0,0,0,0,1,addr blobOut
    test eax,eax
    jz .close
    mov ecx,[blobOut]
    test ecx,ecx
    jz .release
    cmp ecx,255
    ja .release
    mov esi,[blobOut+4]
    mov edi,curseApiKey
    rep movsb
    mov byte [edi],0
    stdcall ModKeyValid
    mov [okay],eax
.release:
    mov edi,[blobOut+4]
    mov ecx,[blobOut]
    xor eax,eax
    rep stosb
    invoke LocalFree,[blobOut+4]
.close:
    invoke CloseHandle,[handle]
.finish:
    cmp [okay],0
    jne .done
    mov edi,curseApiKey
    xor eax,eax
    mov ecx,64
    rep stosd
.done:
    mov eax,[okay]
    ret
endp

proc ModsSaveKey uses ebx esi edi
    locals
        path rw 2048
        temporary rw 2048
        blobIn rd 2
        blobOut rd 2
        written dd ?
        handle dd ?
        okay dd ?
    endl
    mov [okay],0
    mov dword [blobOut],0
    mov dword [blobOut+4],0
    stdcall ModKeyValid
    test eax,eax
    jz .invalid
    stdcall ModKeyPath,addr path
    test eax,eax
    jz .failed
    cmp byte [curseApiKey],0
    jne .encrypt
    invoke GetFileAttributesW,addr path
    cmp eax,-1
    je .removed
    invoke DeleteFileW,addr path
    test eax,eax
    jz .failed
.removed:
    stdcall SetStatusA,mlStatusKeyRemoved
    mov eax,1
    ret
.encrypt:
    invoke lstrlenA,curseApiKey
    mov [blobIn],eax
    mov dword [blobIn+4],curseApiKey
    invoke CryptProtectData,addr blobIn,0,0,0,0,1,addr blobOut
    test eax,eax
    jz .failed
    invoke lstrcpyW,addr temporary,addr path
    invoke lstrcatW,addr temporary,mlKeyTempSuffix
    invoke CreateFileW,addr temporary,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .release
    mov [handle],eax
    invoke WriteFile,eax,[blobOut+4],[blobOut],addr written,0
    mov ebx,eax
    invoke CloseHandle,[handle]
    test ebx,ebx
    jz .removeTemp
    mov eax,[written]
    cmp eax,[blobOut]
    jne .removeTemp
    invoke MoveFileExW,addr temporary,addr path,9
    mov [okay],eax
.removeTemp:
    cmp [okay],0
    jne .release
    invoke DeleteFileW,addr temporary
.release:
    mov edi,[blobOut+4]
    mov ecx,[blobOut]
    xor eax,eax
    rep stosb
    invoke LocalFree,[blobOut+4]
    cmp [okay],0
    je .failed
    stdcall SetStatusA,mlStatusKey
    mov eax,1
    ret
.invalid:
    stdcall SetStatusA,mlErrKey
    xor eax,eax
    ret
.failed:
    stdcall SetStatusA,mlErrKeySave
    xor eax,eax
    ret
endp
