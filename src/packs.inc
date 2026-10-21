; Original OFFSET modpack installer. No shell/script runtime.
; Installs into a private sibling stage, atomically publishes a new instance.
PK_ZMAX = 4096
PK_ZSIZE = 1032
PK_FMAX = 4096
PK_FPATH = 0
PK_FURL = 512
PK_FHASH = 2560
PK_FPROJECT = 2604
PK_FID = 2608
PK_FTOKEN = 2612
PK_FSIZE = 2616

section '.packd' data readable writeable
pkZipCount dd 0
pkFileCount dd 0
pkLoaderKind dd 0
pkFormat dd 0
pkContext dd 0
pkText dd 0
pkZip dd 0
pkZipBytes dd 0
pkManifest dd 0
pkZipTotal dd 0
packLastError dd 0
pkKeyFormat db 'formatVersion',0
pkKeyGame db 'game',0
pkKeyName db 'name',0
pkKeyFiles db 'files',0
pkKeyPath db 'path',0
pkKeyHash db 'hashes',0
pkKeySha db 'sha1',0
pkKeyDownloads db 'downloads',0
pkKeyDeps db 'dependencies',0
pkKeyMinecraft db 'minecraft',0
pkKeyEnv db 'env',0
pkKeyClient db 'client',0
pkKeyLoaderFabric db 'fabric-loader',0
pkKeyLoaderForge db 'forge',0
pkKeyLoaderNeo db 'neoforge',0
pkKeyLoaderQuilt db 'quilt-loader',0
pkKeyManifestType db 'manifestType',0
pkKeyManifestVersion db 'manifestVersion',0
pkKeyVersion db 'version',0
pkKeyModLoaders db 'modLoaders',0
pkKeyId db 'id',0
pkKeyOverrides db 'overrides',0
pkKeyRequired db 'required',0
pkKeyProject db 'projectID',0
pkKeyFile db 'fileID',0
pkGame db 'minecraft',0
pkModpack db 'minecraftModpack',0
pkRequired db 'required',0
pkOptional db 'optional',0
pkUnsupported db 'unsupported',0
pkMrManifest db 'modrinth.index.json',0
pkCfManifest db 'manifest.json',0
pkDefaultOverrides db 'overrides',0
pkClientOverrides db 'client-overrides',0
pkForgePrefix db 'forge-',0
pkNeoPrefix db 'neoforge-',0
pkFabricPrefix db 'fabric-',0
pkQuiltPrefix db 'quilt-',0
pkCfFileFmt db 'https://api.curseforge.com/v1/mods/%u/files/%u',0
pkCfProjectFmt db 'https://api.curseforge.com/v1/mods/%u',0
pkCfModsPrefix db 'mods/',0
pkCfResourcePrefix db 'resourcepacks/',0
pkCfShaderPrefix db 'shaderpacks/',0
pkHostModrinth db 'https://cdn.modrinth.com/',0
pkHostGithub db 'https://github.com/',0
pkHostRaw db 'https://raw.githubusercontent.com/',0
pkHostGitlab db 'https://gitlab.com/',0
pkEmpty db 0
pkSlash du '\',0
pkStageSuffix du '.offset-stage',0
pkOwnerSuffix du '\.offset-pack-stage',0
pkExtractSuffix du '\.offset-archive',0
pkLogSuffix du '\.offset-pack-extract.log',0
pkMarkerSuffix du '\.offset-pack.json',0
pkStar du '\*',0
pkTarName du '\tar.exe',0
pkTarNative du '\Sysnative\tar.exe',0
pkTarX db '-xf',0
pkTarC db '-C',0
pkTarNoOwner db '--no-same-owner',0
pkMarkerPrefix db '{"complete":true,"name":',0
pkMarkerMiddle db ',"version":"',0
pkMarkerEnd db '"}',0
pkOwnedVersions du 'versions',0
pkOwnedLibraries du 'libraries',0
pkOwnedAssets du 'assets',0
pkOwnedProfiles du 'launcher_profiles.json',0
pkOwnedStore du 'launcher_profiles_microsoft_store.json',0
pkDeviceCon du 'CON',0
pkDevicePrn du 'PRN',0
pkDeviceAux du 'AUX',0
pkDeviceNul du 'NUL',0
pkDeviceIn du 'CONIN$',0
pkDeviceOut du 'CONOUT$',0
pkMsgStart db '正在安装整合包…',0
pkMsgArchive db '整合包压缩文件无效、路径不安全或超过支持范围。',0
pkMsgManifest db '整合包清单无效，或包含暂不支持的依赖。',0
pkMsgDestination db '实例目录已存在，或临时目录不属于本整合包。',0
pkMsgFiles db '正在下载整合包文件…',0
pkMsgBlocked db '整合包包含无法通过官方接口下载的必需文件，请查看来源官网。',0
pkMsgNetwork db '整合包文件下载或校验失败，临时缓存已保留。',0
pkMsgOverride db '整合包配置复制失败，临时缓存已保留。',0
pkMsgDone db '整合包安装完成，可选择并启动。',0
pkMsgCancel db '整合包安装已取消，临时缓存已保留。',0
packInstalledVersion rb 128
packInstalledName rb 256
pkNameRaw rb 2048
pkBase rb 128
pkLoader rb 128
pkOverrides rb 512
pkStage rw 1024
pkDestination rw 1024
pkArchive rw 1024
pkExtract rw 1024
pkPath rw 2048
pkSource rw 2048
pkWide rw 512
pkComponent rw 128
pkValue rb 4096
pkValue2 rb 2048
pkArchiveHash rb 41
pkMarker rb 4096
pkZipRecords rb PK_ZMAX*PK_ZSIZE
pkFiles rb PK_FMAX*PK_FSIZE

section '.packc' code readable executable
 nop

; Strict bounded JSON string; rejects decoded NUL aliases.
proc PkString uses ebx,ctx,token,dest,capacity
 stdcall JsonString,[ctx],[token],[dest],[capacity]
 test eax,eax
 jz .bad
 cmp eax,[capacity]
 jae .bad
 mov ebx,eax
 invoke lstrlenA,[dest]
 cmp eax,ebx
 jne .bad
 mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

; One Windows path component, including DOS-device and trailing-dot checks.
proc PkComponentSafe uses ebx esi edi,source,length
 mov ecx,[length]
 test ecx,ecx
 jz .bad
 cmp ecx,120
 ja .bad
 mov esi,[source]
 lea edi,[pkComponent]
 xor ebx,ebx
.copy:
 lodsw
 cmp ax,32
 jb .bad
 cmp ax,127
 je .bad
 cmp ax,':'
 je .bad
 cmp ax,92
 je .bad
 cmp ax,'*'
 je .bad
 cmp ax,'?'
 je .bad
 cmp ax,'"'
 je .bad
 cmp ax,'<'
 je .bad
 cmp ax,'>'
 je .bad
 cmp ax,'|'
 je .bad
 cmp ax,'.'
 jne .stem
 test ebx,ebx
 jnz .stem
 mov ebx,1
 mov word [edi],0
.stem:
 test ebx,ebx
 jnz .next
 stosw
.next:
 dec ecx
 jnz .copy
 cmp word [esi-2],'.'
 je .bad
 cmp word [esi-2],' '
 je .bad
 test ebx,ebx
 jnz .devices
 mov word [edi],0
.devices:
 invoke lstrcmpiW,pkComponent,pkDeviceCon
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkDevicePrn
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkDeviceAux
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkDeviceNul
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkDeviceIn
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkDeviceOut
 test eax,eax
 jz .bad
 invoke lstrlenW,pkComponent
 cmp eax,4
 jne .good
 mov ax,[pkComponent+6]
 cmp ax,'1'
 jb .super
 cmp ax,'9'
 jbe .port
.super:
 cmp ax,00B9h
 je .port
 cmp ax,00B2h
 je .port
 cmp ax,00B3h
 jne .good
.port:
 mov ax,[pkComponent]
 or ax,20h
 cmp ax,'c'
 jne .lpt
 mov ax,[pkComponent+2]
 or ax,20h
 cmp ax,'o'
 jne .good
 mov ax,[pkComponent+4]
 or ax,20h
 cmp ax,'m'
 je .bad
.lpt:
 mov ax,[pkComponent]
 or ax,20h
 cmp ax,'l'
 jne .good
 mov ax,[pkComponent+2]
 or ax,20h
 cmp ax,'p'
 jne .good
 mov ax,[pkComponent+4]
 or ax,20h
 cmp ax,'t'
 je .bad
.good: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

; UTF-8 relative paths, no slash aliases, empty/dot components or NT devices.
; A single final slash is permitted only for archive pkDirectoryArg records.
proc PkSafePath uses ebx esi edi,path,pkDirectoryArg
 invoke lstrlenA,[path]
 cmp eax,511
 ja .bad
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,[path],-1,pkWide,512
 test eax,eax
 jz .bad
 mov esi,pkWide
 mov edi,esi
 xor ebx,ebx
.scan:
 lodsw
 test ax,ax
 jz .last
 cmp ax,'/'
 je .component
 inc ebx
 jmp .scan
.component:
 stdcall PkComponentSafe,edi,ebx
 test eax,eax
 jz .bad
 mov edi,esi
 xor ebx,ebx
 jmp .scan
.last:
 test ebx,ebx
 jnz .checkLast
 cmp [pkDirectoryArg],1
 jne .bad
 cmp edi,pkWide
 je .bad
 mov eax,1
 ret
.checkLast:
 stdcall PkComponentSafe,edi,ebx
 ret
.bad: xor eax,eax
 ret
endp

; Payload cannot supply launcher-owned versions, caches or configuration.
proc PkPayloadSafe uses esi edi,path
 stdcall PkSafePath,[path],0
 test eax,eax
 jz .bad
 mov esi,pkWide
 mov edi,pkComponent
.root:
 lodsw
 cmp ax,'/'
 je .pkEnd
 test ax,ax
 jz .pkEnd
 stosw
 jmp .root
.pkEnd:
 mov word [edi],0
 invoke lstrcmpiW,pkComponent,pkOwnedVersions
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkOwnedLibraries
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkOwnedAssets
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkOwnedProfiles
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkComponent,pkOwnedStore
 test eax,eax
 jz .bad
 ; Reserve every .offset-* root, regardless of case.
 cmp word [pkComponent],'.'
 jne .good
 mov esi,pkComponent+2
 mov edi,.offset
 mov ecx,7
.prefix:
 lodsw
 or ax,20h
 cmp al,[edi]
 jne .good
 inc edi
 loop .prefix
 jmp .bad
.offset db 'offset-',0
.good: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

; Validate the complete ZIP structure before giving it to tar. Central/local
; entries must be contiguous and agree, so no unlisted local entries survive.
; ZIP64, encryption, links, SFX, multi-disk and non-deflate codecs are rejected.
; Reject alternate name/link/ZIP64 metadata rather than let the extractor
; interpret another path than the one validated in the normal ZIP headers.
proc PkExtraSafe uses ebx esi edi,extra,length
 mov esi,[extra]
 mov edi,esi
 add edi,[length]
 jc .bad
.loop:
 cmp esi,edi
 je .good
 lea eax,[esi+4]
 cmp eax,edi
 ja .bad
 movzx ebx,word [esi+2]
 add eax,ebx
 cmp eax,edi
 ja .bad
 mov dx,[esi]
 cmp dx,5455h                 ; Extended timestamp, no names or links.
 je .accepted
 cmp dx,7875h                 ; Unix UID/GID; tar --no-same-owner ignores it.
 je .accepted
 cmp dx,000Ah                 ; NTFS timestamp container only.
 jne .bad
 cmp ebx,32
 jne .bad
 cmp word [esi+8],1
 jne .bad
 cmp word [esi+10],24
 jne .bad
.accepted:
 mov esi,eax
 jmp .loop
.good: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkValidateZip uses ebx esi edi,buffer,size
 locals
  endPos dd ?
  cdPos dd ?
  cdEnd dd ?
  cdCursor dd ?
  localCursor dd ?
  count dd ?
  index dd ?
  flags dd ?
  method dd ?
  compressed dd ?
  expanded dd ?
  crc dd ?
  nameLength dd ?
  pkDirectoryArg dd ?
  localHeader dd ?
  previous dd ?
  centralHeader dd ?
 endl
 mov [pkZipCount],0
 mov [pkZipTotal],0
 mov [pkManifest],0
 mov eax,[size]
 cmp eax,22
 jb .bad
 cmp eax,536870912
 ja .bad
 sub eax,22
 mov esi,[buffer]
 add esi,eax
 mov edi,65536
.findEnd:
 cmp dword [esi],06054B50h
 je .endFound
 cmp esi,[buffer]
 jbe .bad
 dec esi
 dec edi
 jnz .findEnd
 jmp .bad
.endFound:
 mov [endPos],esi
 movzx eax,word [esi+20]
 add eax,esi
 add eax,22
 sub eax,[buffer]
 cmp eax,[size]
 jne .bad
 cmp dword [esi+4],0
 jne .bad
 movzx eax,word [esi+10]
 cmp ax,[esi+8]
 jne .bad
 test eax,eax
 jz .bad
 cmp eax,PK_ZMAX
 ja .bad
 mov [count],eax
 mov eax,[esi+16]
 add eax,[buffer]
 jc .bad
 mov [cdPos],eax
 mov [cdCursor],eax
 add eax,[esi+12]
 jc .bad
 cmp eax,[endPos]
 jne .bad
 mov [cdEnd],eax
 mov [localCursor],0
 mov [index],0
.entry:
 mov eax,[index]
 cmp eax,[count]
 jae .finished
 mov esi,[cdCursor]
 mov [centralHeader],esi
 lea eax,[esi+46]
 cmp eax,[cdEnd]
 ja .bad
 cmp dword [esi],02014B50h
 jne .bad
 cmp word [esi+34],0
 jne .bad
 movzx eax,word [esi+8]
 mov [flags],eax
 and eax,0FFFFF7F1h
 jnz .bad
 movzx eax,word [esi+10]
 mov [method],eax
 test eax,eax
 jz .codec
 cmp eax,8
 jne .bad
.codec:
 mov eax,[esi+16]
 mov [crc],eax
 mov eax,[esi+20]
 mov [compressed],eax
 mov eax,[esi+24]
 cmp eax,134217728
 ja .bad
 mov [expanded],eax
 add [pkZipTotal],eax
 jc .bad
 cmp [pkZipTotal],2147483647
 ja .bad
 movzx ecx,word [esi+28]
 test ecx,ecx
 jz .bad
 cmp ecx,511
 ja .bad
 mov [nameLength],ecx
 movzx eax,word [esi+30]
 add eax,ecx
 movzx edx,word [esi+32]
 add eax,edx
 add eax,46
 add eax,esi
 jc .bad
 cmp eax,[cdEnd]
 ja .bad
 mov [cdCursor],eax
 lea eax,[esi+46]
 add eax,[nameLength]
 movzx edx,word [esi+30]
 stdcall PkExtraSafe,eax,edx
 test eax,eax
 jz .bad
 mov esi,[centralHeader]
 mov eax,[esi+38]
 test eax,400h
 jnz .bad
 shr eax,16
 and eax,0F000h
 cmp eax,0A000h
 je .bad
 test eax,eax
 jz .attributes
 cmp eax,8000h
 je .attributes
 cmp eax,4000h
 jne .bad
.attributes:
 mov eax,[esi+42]
 cmp eax,[localCursor]
 jne .bad
 add eax,[buffer]
 jc .bad
 mov ebx,eax
 mov [localHeader],eax
 add eax,30
 cmp eax,[cdPos]
 ja .bad
 cmp dword [ebx],04034B50h
 jne .bad
 mov ax,[ebx+6]
 cmp ax,word [flags]
 jne .bad
 mov ax,[ebx+8]
 cmp ax,word [method]
 jne .bad
 mov ax,[ebx+26]
 cmp ax,word [nameLength]
 jne .bad
 movzx eax,word [ebx+28]
 add eax,[nameLength]
 add eax,30
 add eax,ebx
 jc .bad
 mov edi,eax
 add eax,[compressed]
 jc .bad
 cmp eax,[cdPos]
 ja .bad
 mov [localCursor],eax
 lea eax,[ebx+30]
 add eax,[nameLength]
 movzx edx,word [ebx+28]
 stdcall PkExtraSafe,eax,edx
 test eax,eax
 jz .bad
 mov esi,[centralHeader]
 lea edi,[ebx+30]
 lea esi,[esi+46]
 mov ecx,[nameLength]
 repe cmpsb
 jne .bad
 mov esi,[cdCursor]
 ; Copy the central name from the matching local header, including no NULs.
 mov esi,[localHeader]
 add esi,30
 mov edi,pkValue
 mov ecx,[nameLength]
.name:
 lodsb
 test al,al
 jz .bad
 cmp al,128
 jb .nameCopy
 test [flags],800h
 jz .bad
.nameCopy:
 stosb
 loop .name
 mov byte [edi],0
 mov [pkDirectoryArg],0
 cmp byte [edi-1],'/'
 jne .path
 mov [pkDirectoryArg],1
 cmp [expanded],0
 jne .bad
.path:
 stdcall PkSafePath,pkValue,[pkDirectoryArg]
 test eax,eax
 jz .bad
 ; Canonical wide names make case-aliased duplicates explicit on Windows.
 mov [previous],0
.duplicates:
 mov eax,[previous]
 cmp eax,[index]
 jae .publish
 imul eax,PK_ZSIZE
 add eax,pkZipRecords
 invoke lstrcmpiW,eax,pkWide
 test eax,eax
 jz .bad
 inc [previous]
 jmp .duplicates
.publish:
 mov eax,[index]
 imul eax,PK_ZSIZE
 add eax,pkZipRecords
 invoke lstrcpyW,eax,pkWide
 mov ebx,[index]
 imul ebx,PK_ZSIZE
 add ebx,pkZipRecords
 mov eax,[pkDirectoryArg]
 mov [ebx+1024],eax
 mov eax,[expanded]
 mov [ebx+1028],eax
.localSizes:
 mov ebx,[localHeader]
 test [flags],8
 jnz .descriptor
 mov eax,[crc]
 cmp eax,[ebx+14]
 jne .bad
 mov eax,[compressed]
 cmp eax,[ebx+18]
 jne .bad
 mov eax,[expanded]
 cmp eax,[ebx+22]
 jne .bad
 jmp .cursor
.descriptor:
 ; Descriptor archives must not give the streaming extractor conflicting
 ; nonzero sizes/CRC in the local header. Stored+descriptor has no bounded
 ; compressed stream terminator and is deliberately outside this format set.
 cmp [method],8
 jne .bad
 mov eax,[ebx+14]
 test eax,eax
 jz .descriptorCompressed
 cmp eax,[crc]
 jne .bad
.descriptorCompressed:
 mov eax,[ebx+18]
 test eax,eax
 jz .descriptorExpanded
 cmp eax,[compressed]
 jne .bad
.descriptorExpanded:
 mov eax,[ebx+22]
 test eax,eax
 jz .descriptorStart
 cmp eax,[expanded]
 jne .bad
.descriptorStart:
 mov esi,[localCursor]
 lea eax,[esi+12]
 cmp eax,[cdPos]
 ja .bad
 cmp dword [esi],08074B50h
 jne .descriptorValues
 add esi,4
 lea eax,[esi+12]
 cmp eax,[cdPos]
 ja .bad
.descriptorValues:
 mov eax,[crc]
 cmp eax,[esi]
 jne .bad
 mov eax,[compressed]
 cmp eax,[esi+4]
 jne .bad
 mov eax,[expanded]
 cmp eax,[esi+8]
 jne .bad
 add esi,12
 mov [localCursor],esi
.cursor:
 mov eax,[localCursor]
 sub eax,[buffer]
 mov [localCursor],eax
 inc [index]
 jmp .entry
.finished:
 mov eax,[localCursor]
 add eax,[buffer]
 cmp eax,[cdPos]
 jne .bad
 mov eax,[cdCursor]
 cmp eax,[cdEnd]
 jne .bad
 mov eax,[count]
 mov [pkZipCount],eax
 stdcall PkDetectArchive
 ret
.bad:
 mov [pkZipCount],0
 xor eax,eax
 ret
endp

proc PkReadZip uses ebx esi,path
 local file:DWORD,size:DWORD,high:DWORD,mapping:DWORD,result:DWORD
 mov [result],0
 invoke CreateFileW,[path],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .done
 mov [file],eax
 invoke GetFileSize,eax,addr high
 cmp [high],0
 jne .close
 cmp eax,536870912
 ja .close
 cmp eax,22
 jb .close
 mov [size],eax
 invoke CreateFileMappingW,[file],0,2,0,0,0
 test eax,eax
 jz .close
 mov [mapping],eax
 invoke MapViewOfFile,eax,4,0,0,0
 test eax,eax
 jz .mapping
 mov esi,eax
 mov [pkZip],eax
 mov eax,[size]
 mov [pkZipBytes],eax
 stdcall PkValidateZip,esi,eax
 mov [result],eax
.free:
 invoke UnmapViewOfFile,[pkZip]
 mov [pkZip],0
.mapping:
 invoke CloseHandle,[mapping]
.close:
 invoke CloseHandle,[file]
.done:
 mov eax,[result]
 ret
endp

; Check an existing private stage recursively before writing anything into it.
; Reject all reparse points; this also checks parents used for continuation.
proc PkTreeSafe uses ebx esi edi,pkDirectoryArg,depth
 locals
  find WIN32_FIND_DATAW
  handle dd ?
  mask rw 2048
  result dd ?
 endl
 mov [result],0
 cmp [depth],48
 ja .done
 invoke GetFileAttributesW,[pkDirectoryArg]
 cmp eax,-1
 je .done
 test eax,400h
 jnz .done
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .done
 invoke lstrlenW,[pkDirectoryArg]
 cmp eax,1500
 ja .done
 invoke lstrcpyW,addr mask,[pkDirectoryArg]
 invoke lstrcatW,addr mask,pkStar
 invoke FindFirstFileW,addr mask,addr find
 cmp eax,-1
 je .empty
 mov [handle],eax
.entry:
 cmp word [find.cFileName],'.'
 jne .check
 cmp word [find.cFileName+2],0
 je .next
 cmp word [find.cFileName+2],'.'
 jne .check
 cmp word [find.cFileName+4],0
 je .next
.check:
 test [find.dwFileAttributes],400h
 jnz .close
 test [find.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY
 jz .next
 invoke lstrcpyW,addr mask,[pkDirectoryArg]
 invoke lstrcatW,addr mask,pkSlash
 invoke lstrcatW,addr mask,addr find.cFileName
 mov eax,[depth]
 inc eax
 stdcall PkTreeSafe,addr mask,eax
 test eax,eax
 jz .close
.next:
 invoke FindNextFileW,[handle],addr find
 test eax,eax
 jnz .entry
 invoke GetLastError
 cmp eax,18
 jne .close
 mov [result],1
.close:
 invoke FindClose,[handle]
 jmp .done
.empty:
 invoke GetLastError
 cmp eax,2
 jne .done
 mov [result],1
.done:
 mov eax,[result]
 ret
endp

; Resolve target to a full path; existing final instances are never replaced.
; Resume only a stage carrying the exact archive SHA1 owner marker.
proc PkAncestorsSafe uses ebx esi edi,path
 local copy rw 1024
 invoke lstrlenW,[path]
 cmp eax,1024
 jae .bad
 invoke lstrcpyW,addr copy,[path]
 lea esi,[copy]
 cmp word [esi+2],':'
 jne .bad
 cmp word [esi+4],92
 jne .bad
 lea edi,[esi+6]
.loop:
 mov ax,[edi]
 test ax,ax
 jz .last
 cmp ax,92
 jne .next
 mov word [edi],0
 invoke GetFileAttributesW,esi
 mov word [edi],92
 cmp eax,-1
 je .bad
 test eax,400h
 jnz .bad
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .bad
.next:
 add edi,2
 jmp .loop
.last:
 invoke GetFileAttributesW,esi
 cmp eax,-1
 je .bad
 test eax,400h
 jnz .bad
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .bad
 mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkPrepareStage uses ebx esi edi,destination
 local parent rw 1024
 local ownerFile:DWORD
 invoke GetFullPathNameW,[destination],1024,pkDestination,0
 test eax,eax
 jz .bad
 cmp eax,850
 ja .bad
 invoke GetFileAttributesW,pkDestination
 cmp eax,-1
 jne .bad
 invoke lstrcpyW,pkStage,pkDestination
 invoke lstrcatW,pkStage,pkStageSuffix
 invoke lstrcpyW,pkExtract,pkStage
 invoke lstrcatW,pkExtract,pkExtractSuffix
 ; Validate the existing parent chain before creating the owned stage.
 invoke lstrcpyW,addr parent,pkDestination
 lea esi,[parent]
 invoke lstrlenW,esi
 lea edi,[esi+eax*2]
.parent:
 cmp edi,esi
 jbe .bad
 sub edi,2
 cmp word [edi],92
 jne .parent
 mov word [edi],0
 stdcall PkAncestorsSafe,addr parent
 test eax,eax
 jz .bad
 invoke GetFileAttributesW,pkStage
 cmp eax,-1
 jne .resume
 invoke CreateDirectoryW,pkStage,0
 test eax,eax
 jz .bad
 invoke lstrcpyW,pkPath,pkStage
 invoke lstrcatW,pkPath,pkOwnerSuffix
 stdcall PkWriteNew,pkPath,pkArchiveHash
 test eax,eax
 jz .bad
 jmp .extract
.resume:
 stdcall PkTreeSafe,pkStage,0
 test eax,eax
 jz .bad
 invoke lstrcpyW,pkPath,pkStage
 invoke lstrcatW,pkPath,pkOwnerSuffix
 invoke CreateFileW,pkPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [ownerFile],eax
 invoke GetFileSize,eax,0
 mov ebx,eax
 invoke CloseHandle,[ownerFile]
 cmp ebx,40
 jne .bad
 stdcall EReadFile,pkPath
 test eax,eax
 jz .bad
 mov esi,eax
 invoke lstrcmpA,esi,pkArchiveHash
 mov ebx,eax
 stdcall FreeMem,esi
 test ebx,ebx
 jnz .bad
.extract:
 stdcall EnsureDirectory,pkExtract
 ret
.bad: xor eax,eax
 ret
endp

proc PkWriteNew uses ebx,path,text
 local file:DWORD,written:DWORD,size:DWORD
 invoke lstrlenA,[text]
 mov [size],eax
 invoke CreateFileW,[path],GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [file],eax
 invoke WriteFile,eax,[text],[size],addr written,0
 mov ebx,eax
 invoke CloseHandle,[file]
 test ebx,ebx
 jz .bad
 mov eax,[written]
 cmp eax,[size]
 jne .bad
 mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkExtractArchive
 invoke GetWindowsDirectoryW,eTarPath,990
 invoke lstrcatW,eTarPath,pkTarNative
 invoke GetFileAttributesW,eTarPath
 cmp eax,-1
 jne .tar
 invoke GetSystemDirectoryW,eTarPath,990
 invoke lstrcatW,eTarPath,pkTarName
 invoke GetFileAttributesW,eTarPath
 cmp eax,-1
 je .bad
.tar:
 stdcall LdCommandStart,eTarPath
 test eax,eax
 jz .bad
 stdcall EAppendCommand,pkTarX
 stdcall LdCommandWide,pkArchive
 test eax,eax
 jz .bad
 stdcall EAppendCommand,pkTarC
 stdcall LdCommandWide,pkExtract
 test eax,eax
 jz .bad
 stdcall EAppendCommand,pkTarNoOwner
 invoke lstrcpyW,pkPath,pkStage
 invoke lstrcatW,pkPath,pkLogSuffix
 stdcall LdRun,eTarPath,pkStage,pkPath
 test eax,eax
 jz .bad
 stdcall PkTreeSafe,pkExtract,0
 ret
.bad: xor eax,eax
 ret
endp

proc PkTrustedUrl,url
 stdcall CfPrefix,[url],pkHostModrinth
 test eax,eax
 jnz .good
 stdcall CfPrefix,[url],pkHostGithub
 test eax,eax
 jnz .good
 stdcall CfPrefix,[url],pkHostRaw
 test eax,eax
 jnz .good
 stdcall CfPrefix,[url],pkHostGitlab
.good: ret
endp

proc PkFileUnique uses ebx esi,index
 mov ebx,[index]
 imul ebx,PK_FSIZE
 add ebx,pkFiles
 stdcall PkPayloadSafe,ebx
 test eax,eax
 jz .bad
 xor esi,esi
.loop:
 cmp esi,[index]
 jae .good
 imul eax,esi,PK_FSIZE
 add eax,pkFiles
 invoke MultiByteToWideChar,65001,8,eax,-1,pkSource,2048
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,ebx,-1,pkPath,2048
 test eax,eax
 jz .bad
 invoke lstrcmpiW,pkSource,pkPath
 test eax,eax
 jz .bad
 inc esi
 jmp .loop
.good: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkReadDependencies uses ebx esi edi,root
 local deps:DWORD,token:DWORD,index:DWORD,kind:DWORD,total:DWORD
 stdcall JsonGet,[pkContext],[root],pkKeyDeps
 mov [deps],eax
 stdcall JsonCount,[pkContext],eax
 test eax,eax
 jz .bad
 cmp eax,2
 ja .bad
 mov [total],eax
 stdcall JsonGet,[pkContext],[deps],pkKeyMinecraft
 stdcall PkString,[pkContext],eax,pkBase,128
 test eax,eax
 jz .bad
 stdcall LoaderSafeId,pkBase
 test eax,eax
 jz .bad
 mov [index],0
 mov [pkLoaderKind],0
 mov byte [pkLoader],0
.loop:
 mov eax,[index]
 cmp eax,4
 jae .good
 mov eax,[.keys+eax*4]
 stdcall JsonGet,[pkContext],[deps],eax
 test eax,eax
 jz .next
 cmp [pkLoaderKind],0
 jne .bad
 mov [token],eax
 stdcall PkString,[pkContext],eax,pkLoader,128
 test eax,eax
 jz .bad
 stdcall LoaderSafeId,pkLoader
 test eax,eax
 jz .bad
 mov eax,[index]
 inc eax
 mov [pkLoaderKind],eax
.next:
 inc [index]
 jmp .loop
.keys dd pkKeyLoaderFabric,pkKeyLoaderForge,pkKeyLoaderNeo,pkKeyLoaderQuilt
.good:
 cmp [total],2
 jne .accepted
 cmp [pkLoaderKind],0
 je .bad
.accepted: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkReadMr uses ebx esi edi,root
 local list:DWORD,count:DWORD,index:DWORD,item:DWORD,env:DWORD,urls:DWORD,urlIndex:DWORD
 stdcall JsonGet,[pkContext],[root],pkKeyFormat
 stdcall JsonInt,[pkContext],eax
 cmp eax,1
 jne .bad
 stdcall JsonGet,[pkContext],[root],pkKeyGame
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .bad
 invoke lstrcmpA,pkValue,pkGame
 test eax,eax
 jnz .bad
 stdcall PkReadDependencies,[root]
 test eax,eax
 jz .bad
 invoke lstrcpyA,pkOverrides,pkDefaultOverrides
 stdcall JsonGet,[pkContext],[root],pkKeyFiles
 mov [list],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[pkContext],eax
 cmp eax,PK_FMAX
 ja .bad
 mov [count],eax
 mov [index],0
 mov [pkFileCount],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .good
 stdcall JsonAt,[pkContext],[list],eax
 mov [item],eax
 stdcall JsonGet,[pkContext],eax,pkKeyEnv
 test eax,eax
 jz .file
 stdcall JsonGet,[pkContext],eax,pkKeyClient
 test eax,eax
 jz .bad
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .bad
 invoke lstrcmpA,pkValue,pkUnsupported
 test eax,eax
 jz .next
 invoke lstrcmpA,pkValue,pkRequired
 test eax,eax
 jz .file
 invoke lstrcmpA,pkValue,pkOptional
 test eax,eax
 jnz .bad
.file:
 mov ebx,[pkFileCount]
 imul ebx,PK_FSIZE
 add ebx,pkFiles
 stdcall JsonGet,[pkContext],[item],pkKeyPath
 stdcall PkString,[pkContext],eax,ebx,512
 test eax,eax
 jz .bad
 stdcall PkFileUnique,[pkFileCount]
 test eax,eax
 jz .bad
 stdcall JsonGet,[pkContext],[item],pkKeyHash
 stdcall JsonGet,[pkContext],eax,pkKeySha
 lea edi,[ebx+PK_FHASH]
 stdcall PkString,[pkContext],eax,edi,41
 test eax,eax
 jz .bad
 stdcall ModIsSha1,edi
 test eax,eax
 jz .bad
 stdcall JsonGet,[pkContext],[item],pkKeyDownloads
 mov [urls],eax
 mov [ebx+PK_FTOKEN],eax
 mov [urlIndex],0
.urls:
 cmp [urlIndex],16
 jae .bad
 stdcall JsonAt,[pkContext],[urls],[urlIndex]
 test eax,eax
 jz .bad
 lea edi,[ebx+PK_FURL]
 stdcall PkString,[pkContext],eax,edi,2048
 test eax,eax
 jz .nextUrl
 stdcall PkTrustedUrl,edi
 test eax,eax
 jnz .accepted
.nextUrl:
 inc [urlIndex]
 jmp .urls
.accepted:
 inc [pkFileCount]
.next:
 inc [index]
 jmp .loop
.good: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkReadCf uses ebx esi edi,root
 local mc:DWORD,list:DWORD,count:DWORD,index:DWORD,item:DWORD
 stdcall JsonGet,[pkContext],[root],pkKeyManifestType
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .bad
 invoke lstrcmpA,pkValue,pkModpack
 test eax,eax
 jnz .bad
 stdcall JsonGet,[pkContext],[root],pkKeyManifestVersion
 stdcall JsonInt,[pkContext],eax
 cmp eax,1
 jne .bad
 stdcall JsonGet,[pkContext],[root],pkKeyOverrides
 stdcall PkString,[pkContext],eax,pkOverrides,512
 test eax,eax
 jz .bad
 stdcall PkSafePath,pkOverrides,0
 test eax,eax
 jz .bad
 stdcall JsonGet,[pkContext],[root],pkKeyMinecraft
 mov [mc],eax
 stdcall JsonGet,[pkContext],eax,pkKeyVersion
 stdcall PkString,[pkContext],eax,pkBase,128
 test eax,eax
 jz .bad
 stdcall LoaderSafeId,pkBase
 test eax,eax
 jz .bad
 mov [pkLoaderKind],0
 mov byte [pkLoader],0
 stdcall JsonGet,[pkContext],[mc],pkKeyModLoaders
 mov [list],eax
 stdcall JsonCount,[pkContext],eax
 cmp eax,1
 ja .bad
 test eax,eax
 jz .files
 stdcall JsonAt,[pkContext],[list],0
 stdcall JsonGet,[pkContext],eax,pkKeyId
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .bad
 mov esi,pkValue
 mov edi,pkForgePrefix
 mov ebx,2
 stdcall CfPrefix,esi,edi
 test eax,eax
 jnz .loader
 mov edi,pkNeoPrefix
 mov ebx,3
 stdcall CfPrefix,esi,edi
 test eax,eax
 jnz .loader
 mov edi,pkFabricPrefix
 mov ebx,1
 stdcall CfPrefix,esi,edi
 test eax,eax
 jnz .loader
 mov edi,pkQuiltPrefix
 mov ebx,4
 stdcall CfPrefix,esi,edi
 test eax,eax
 jz .bad
.loader:
 invoke lstrlenA,edi
 add esi,eax
 invoke lstrlenA,esi
 cmp eax,127
 ja .bad
 invoke lstrcpyA,pkLoader,esi
 stdcall LoaderSafeId,pkLoader
 test eax,eax
 jz .bad
 mov [pkLoaderKind],ebx
.files:
 stdcall JsonGet,[pkContext],[root],pkKeyFiles
 mov [list],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[pkContext],eax
 cmp eax,PK_FMAX
 ja .bad
 mov [count],eax
 mov [index],0
 mov [pkFileCount],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .good
 stdcall JsonAt,[pkContext],[list],eax
 mov [item],eax
 stdcall JsonGet,[pkContext],eax,pkKeyRequired
 stdcall JsonBool,[pkContext],eax
 cmp eax,-1
 je .bad
 test eax,eax
 jz .next
 mov ebx,[pkFileCount]
 imul ebx,PK_FSIZE
 add ebx,pkFiles
 stdcall JsonGet,[pkContext],[item],pkKeyProject
 stdcall JsonInt,[pkContext],eax
 test eax,eax
 jle .bad
 mov [ebx+PK_FPROJECT],eax
 stdcall JsonGet,[pkContext],[item],pkKeyFile
 stdcall JsonInt,[pkContext],eax
 test eax,eax
 jle .bad
 mov [ebx+PK_FID],eax
 inc [pkFileCount]
.next:
 inc [index]
 jmp .loop
.good: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkCloseManifest
 cmp [pkContext],0
 je .text
 stdcall JsonFree,[pkContext]
 mov [pkContext],0
.text:
 stdcall FreeMem,[pkText]
 mov [pkText],0
 ret
endp

proc PkReadManifest uses ebx
 local result:DWORD
 mov [result],0
 invoke lstrcpyW,pkPath,pkExtract
 invoke lstrcatW,pkPath,pkSlash
 invoke lstrcatW,pkPath,pkManifestPathW
 stdcall EReadFile,pkPath
 test eax,eax
 jz .done
 mov [pkText],eax
 stdcall PkParseManifestText
 mov [result],eax
.done:
 mov eax,[result]
 ret
endp

proc PkParseManifestText uses ebx
 local result:DWORD
 mov [result],0
 mov [pkEmbeddedCount],0
 stdcall JsonParse,[pkText]
 test eax,eax
 jz .done
 mov [pkContext],eax
 stdcall JsonRoot,eax
 mov ebx,eax
 cmp [pkManifest],3
 je .mmc
 stdcall JsonGet,[pkContext],ebx,pkKeyName
 mov edx,eax
 stdcall JsonRaw,[pkContext],edx,pkNameRaw,2048
 test eax,eax
 jz .done
 cmp eax,2048
 jae .done
 stdcall JsonGet,[pkContext],ebx,pkKeyName
 stdcall PkString,[pkContext],eax,packInstalledName,256
 test eax,eax
 jz .done
 mov eax,[pkManifest]
 mov [pkFormat],eax
 cmp eax,1
 jne .cf
 stdcall PkReadMr,ebx
 jmp .parsed
.cf:
 stdcall PkReadCf,ebx
 jmp .parsed
.mmc:
 mov [pkFormat],3
 stdcall PkReadMmc,ebx
.parsed:
 test eax,eax
 jz .done
 stdcall PkValidatePayload,pkOverrides
 test eax,eax
 jz .done
 cmp [pkFormat],1
 jne .accepted
 stdcall PkValidatePayload,pkClientOverrides
.accepted:
 mov [result],eax
.done:
 cmp [result],0
 jne .keepContext
 stdcall PkCloseManifest
.keepContext:
 mov eax,[result]
 ret
endp

; CF manifest file IDs resolve through the official authenticated API. Never
; guess a blocked download URL or forward the key to a CDN/redirect.
proc PkResolveCf uses ebx esi edi,index
 local item:DWORD,hashes:DWORD,count:DWORD,hashIndex:DWORD,found:DWORD,result:DWORD,folder:DWORD
 mov [result],0
 mov ebx,[index]
 imul ebx,PK_FSIZE
 add ebx,pkFiles
 cinvoke wsprintfA,cfUrl,pkCfProjectFmt,dword [ebx+PK_FPROJECT]
 stdcall CfRequest
 test eax,eax
 jz .done
 stdcall JsonGet,[cfContext],[cfRoot],cfKData
 mov [item],eax
 stdcall JsonGet,[cfContext],eax,cfKId
 stdcall JsonInt,[cfContext],eax
 cmp eax,[ebx+PK_FPROJECT]
 jne .done
 stdcall JsonGet,[cfContext],[item],cfKGame
 stdcall JsonInt,[cfContext],eax
 cmp eax,432
 jne .done
 stdcall JsonGet,[cfContext],[item],cfKAvailable
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 jne .done
 stdcall JsonGet,[cfContext],[item],cfKDistribution
 stdcall JsonBool,[cfContext],eax
 cmp eax,0
 je .done
 stdcall JsonGet,[cfContext],[item],cfKClass
 stdcall JsonInt,[cfContext],eax
 mov [folder],pkCfModsPrefix
 cmp eax,6
 je .projectAccepted
 mov [folder],pkCfResourcePrefix
 cmp eax,12
 je .projectAccepted
 mov [folder],pkCfShaderPrefix
 cmp eax,6552
 jne .done
.projectAccepted:
 stdcall CfCleanup
 cinvoke wsprintfA,cfUrl,pkCfFileFmt,dword [ebx+PK_FPROJECT],dword [ebx+PK_FID]
 stdcall CfRequest
 test eax,eax
 jz .done
 stdcall JsonGet,[cfContext],[cfRoot],cfKData
 mov [item],eax
 stdcall JsonGet,[cfContext],eax,cfKId
 stdcall JsonInt,[cfContext],eax
 cmp eax,[ebx+PK_FID]
 jne .done
 stdcall JsonGet,[cfContext],[item],cfKMod
 stdcall JsonInt,[cfContext],eax
 cmp eax,[ebx+PK_FPROJECT]
 jne .done
 stdcall JsonGet,[cfContext],[item],cfKAvailable
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 jne .done
 stdcall JsonGet,[cfContext],[item],cfKServer
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 je .done
 stdcall JsonGet,[cfContext],[item],cfKEarly
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 je .done
 stdcall JsonGet,[cfContext],[item],cfKDownload
 lea edi,[ebx+PK_FURL]
 stdcall PkString,[cfContext],eax,edi,2048
 test eax,eax
 jz .done
 stdcall ModTrustedUrl,edi,1
 test eax,eax
 jz .done
 stdcall JsonGet,[cfContext],[item],cfKFileName
 stdcall PkString,[cfContext],eax,pkValue,4096
 test eax,eax
 jz .done
 cmp [folder],pkCfModsPrefix
 jne .zipName
 stdcall ModSafeFilename,pkValue
 test eax,eax
 jz .done
 jmp .filenameAccepted
.zipName:
 stdcall PkSafePath,pkValue,0
 test eax,eax
 jz .done
 mov esi,pkValue
.basename:
 lodsb
 test al,al
 jz .extension
 cmp al,'/'
 je .done
 jmp .basename
.extension:
 invoke lstrlenA,pkValue
 cmp eax,5
 jb .done
 cmp eax,255
 ja .done
 lea esi,[pkValue+eax-4]
 cmp byte [esi],'.'
 jne .done
 mov al,[esi+1]
 or al,20h
 cmp al,'z'
 jne .done
 mov al,[esi+2]
 or al,20h
 cmp al,'i'
 jne .done
 mov al,[esi+3]
 or al,20h
 cmp al,'p'
 jne .done
.filenameAccepted:
 invoke lstrcpyA,ebx,[folder]
 stdcall ModAppend,ebx,512,pkValue
 test eax,eax
 jz .done
 stdcall PkFileUnique,[index]
 test eax,eax
 jz .done
 stdcall JsonGet,[cfContext],[item],cfKHashes
 mov [hashes],eax
 stdcall JsonCount,[cfContext],eax
 mov [count],eax
 mov [hashIndex],0
 mov [found],0
.hash:
 mov eax,[hashIndex]
 cmp eax,[count]
 jae .hashDone
 stdcall JsonAt,[cfContext],[hashes],eax
 mov esi,eax
 stdcall JsonGet,[cfContext],eax,cfKAlgo
 stdcall JsonInt,[cfContext],eax
 cmp eax,1
 jne .nextHash
 stdcall JsonGet,[cfContext],esi,cfKValue
 lea edi,[ebx+PK_FHASH]
 stdcall PkString,[cfContext],eax,edi,41
 test eax,eax
 jz .done
 stdcall ModIsSha1,edi
 test eax,eax
 jz .done
 mov [found],1
.nextHash:
 inc [hashIndex]
 jmp .hash
.hashDone:
 cmp [found],1
 jne .done
 mov [result],1
.done:
 stdcall CfCleanup
 mov eax,[result]
 ret
endp

proc PkDownloadFiles uses ebx esi edi
 local urlIndex:DWORD,downloaded:DWORD,urls:DWORD
 xor esi,esi
 stdcall SetStatusA,pkMsgFiles
.resolve:
 cmp esi,[pkFileCount]
 jae .downloadStart
 cmp [engineCancel],0
 jne .bad
 cmp [modCancel],0
 jne .bad
 cmp [pkFormat],2
 jne .nextResolve
 stdcall PkResolveCf,esi
 test eax,eax
 jz .blocked
.nextResolve:
 inc esi
 jmp .resolve
.downloadStart:
 xor esi,esi
.download:
 cmp esi,[pkFileCount]
 jae .good
 cmp [engineCancel],0
 jne .bad
 cmp [modCancel],0
 jne .bad
 imul ebx,esi,PK_FSIZE
 add ebx,pkFiles
 stdcall PkPayloadSafe,ebx
 test eax,eax
 jz .bad
 invoke lstrcpyW,pkPath,pkStage
 invoke lstrcatW,pkPath,pkSlash
 invoke lstrcatW,pkPath,pkWide
 lea eax,[ebx+PK_FURL]
 lea edx,[ebx+PK_FHASH]
 stdcall HttpDownload,eax,pkPath,edx
 test eax,eax
 jnz .fileDone
 cmp [pkFormat],1
 jne .network
 mov eax,[ebx+PK_FTOKEN]
 mov [urls],eax
 mov [urlIndex],0
.fallback:
 cmp [engineCancel],0
 jne .bad
 cmp [modCancel],0
 jne .bad
 cmp [urlIndex],16
 jae .network
 stdcall JsonAt,[pkContext],[urls],[urlIndex]
 test eax,eax
 jz .network
 inc [urlIndex]
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .fallback
 stdcall PkTrustedUrl,pkValue
 test eax,eax
 jz .fallback
 lea eax,[ebx+PK_FURL]
 invoke lstrcmpA,eax,pkValue
 test eax,eax
 jz .fallback
 lea eax,[ebx+PK_FHASH]
 stdcall HttpDownload,pkValue,pkPath,eax
 test eax,eax
 jz .fallback
.fileDone:
 inc esi
 jmp .download
.good: mov eax,1
 ret
.blocked:
 stdcall SetStatusA,pkMsgBlocked
 jmp .bad
.network:
 stdcall SetStatusA,pkMsgNetwork
.bad: xor eax,eax
 ret
endp

; Copy only explicitly indexed override files. Common overrides first, then
; client-overrides; pkDirectoryArg symlinks were refused before extraction.
proc PkCopyOverrides uses ebx esi edi,prefix
 local widePrefix rw 512
 local prefixLength:DWORD,index:DWORD
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,[prefix]
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,pkValue,-1,addr widePrefix,512
 test eax,eax
 jz .bad
 dec eax
 mov [prefixLength],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[pkZipCount]
 jae .good
 imul ebx,eax,PK_ZSIZE
 add ebx,pkZipRecords
 cmp dword [ebx+1024],0
 jne .next
 mov esi,ebx
 lea edi,[widePrefix]
 mov ecx,[prefixLength]
 repe cmpsw
 jne .next
 cmp word [esi],'/'
 jne .next
 add esi,2
 invoke WideCharToMultiByte,65001,0,esi,-1,pkValue,4096,0,0
 test eax,eax
 jz .bad
 stdcall PkPayloadSafe,pkValue
 test eax,eax
 jz .bad
 invoke lstrcpyW,pkPath,pkStage
 invoke lstrcatW,pkPath,pkSlash
 invoke lstrcatW,pkPath,esi
 ; Create only the payload file's parent, not the filename itself.
 invoke lstrcpyW,pkSource,pkPath
 invoke lstrlenW,pkSource
 lea edi,[pkSource+eax*2]
.parent:
 sub edi,2
 cmp word [edi],'/'
 je .split
 cmp word [edi],92
 jne .parent
.split:
 mov word [edi],0
 stdcall EnsureDirectory,pkSource
 test eax,eax
 jz .bad
 invoke lstrcpyW,pkSource,pkExtract
 invoke lstrcatW,pkSource,pkSlash
 invoke lstrcatW,pkSource,ebx
 cmp [engineCancel],0
 jne .bad
 invoke CopyFileW,pkSource,pkPath,0
 test eax,eax
 jz .bad
.next:
 inc [index]
 jmp .loop
.good: mov eax,1
 ret
.bad: xor eax,eax
 ret
endp

proc PkInstallGame uses ebx esi edi
 invoke lstrcpyW,gameDir,pkStage
 invoke lstrcpyA,selectedVersion,pkBase
 stdcall EInstallCore
 mov ebx,eax
 stdcall ECleanup
 test ebx,ebx
 jz .bad
 cmp [pkLoaderKind],0
 je .plain
 stdcall LoaderInstall,pkBase,[pkLoaderKind],pkLoader,packInstalledVersion
 test eax,eax
 jz .bad
 stdcall ProfileNormalize,packInstalledVersion,pkBase
 test eax,eax
 jz .bad
 invoke lstrcpyA,selectedVersion,packInstalledVersion
 stdcall EValidateVersion
 test eax,eax
 jz .bad
 stdcall EBuildPaths
 stdcall ELoadMetadata
 test eax,eax
 jz .bad
 stdcall EInstallLibraries
 mov ebx,eax
 stdcall ECleanup
 mov eax,ebx
 ret
.plain:
 invoke lstrcpyA,packInstalledVersion,pkBase
 mov eax,1
 ret
.bad:
 stdcall ECleanup
 xor eax,eax
 ret
endp

proc PkPublish uses ebx
 invoke lstrcpyA,pkMarker,pkMarkerPrefix
 stdcall ModAppend,pkMarker,4096,pkNameRaw
 test eax,eax
 jz .bad
 stdcall ModAppend,pkMarker,4096,pkMarkerMiddle
 stdcall ModAppend,pkMarker,4096,packInstalledVersion
 stdcall ModAppend,pkMarker,4096,pkMarkerEnd
 test eax,eax
 jz .bad
 invoke lstrcpyW,pkPath,pkStage
 invoke lstrcatW,pkPath,pkMarkerSuffix
 ; This file is module-owned and may already exist after a failed final rename.
 stdcall LdWriteFile,pkPath,pkMarker
 test eax,eax
 jz .bad
 mov ebx,20
.rename:
 cmp [engineCancel],0
 jne .bad
 invoke MoveFileExW,pkStage,pkDestination,0
 test eax,eax
 jnz .ok
 invoke GetLastError
 mov [packLastError],eax
 cmp eax,5
 je .retry
 cmp eax,32
 jne .bad
.retry:
 dec ebx
 jz .bad
 invoke Sleep,100
 jmp .rename
.bad:
 xor eax,eax
.ok:
 ret
endp

proc PackInstall,archivePathW,destinationW
 stdcall PackInstallChecked,[archivePathW],[destinationW],0
 ret
endp
proc PackInstallChecked uses ebx esi edi,archivePathW,destinationW,expectedSha1A
 locals
  savedGame rw 1024
  savedVersion rb 128
  result dd ?
  archiveLock dd ?
 endl
 mov [result],0
 mov [archiveLock],-1
 mov [packLastError],0
 mov [pkContext],0
 mov [pkText],0
 mov byte [packInstalledVersion],0
 mov byte [packInstalledName],0
 invoke lstrcpyW,addr savedGame,gameDir
 invoke lstrcpyA,addr savedVersion,selectedVersion
 stdcall SetStatusA,pkMsgStart
 invoke GetFullPathNameW,[archivePathW],1024,pkArchive,0
 test eax,eax
 jz .archive
 cmp eax,1024
 jae .archive
 ; Hold a sharing lock throughout validation/extraction to prevent an archive
 ; being replaced between the independent ZIP scan and the tar invocation.
 invoke CreateFileW,pkArchive,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .archive
 mov [archiveLock],eax
 stdcall PkReadZip,pkArchive
 test eax,eax
 jz .archive
 stdcall Sha1File,pkArchive,pkArchiveHash
 test eax,eax
 jz .archive
 cmp [expectedSha1A],0
 je .checked
 invoke lstrcmpA,pkArchiveHash,[expectedSha1A]
 test eax,eax
 jnz .changed
.checked:
 cmp [engineCancel],0
 jne .done
 stdcall PkPrepareStage,[destinationW]
 test eax,eax
 jz .destination
 stdcall PkExtractArchive
 test eax,eax
 jz .archive
 stdcall PkReadManifest
 test eax,eax
 jz .manifest
 stdcall PkDownloadFiles
 test eax,eax
 jz .done
 stdcall PkInstallGame
 test eax,eax
 jz .done
 stdcall PkCopyOverrides,pkOverrides
 test eax,eax
 jz .override
 cmp [pkFormat],1
 jne .publish
 stdcall PkCopyOverrides,pkClientOverrides
 test eax,eax
 jz .override
.publish:
 cmp [engineCancel],0
 jne .done
 stdcall PkPublish
 test eax,eax
 jz .destination
 mov [result],1
 stdcall SetStatusA,pkMsgDone
 jmp .done
.archive:
 stdcall SetStatusA,pkMsgArchive
 jmp .done
.destination:
 stdcall SetStatusA,pkMsgDestination
 jmp .done
.manifest:
 stdcall SetStatusA,pkMsgManifest
 jmp .done
.changed:
 stdcall SetStatusA,pkMsgChanged
 jmp .done
.override:
 stdcall SetStatusA,pkMsgOverride
.done:
 stdcall PkCloseManifest
 cmp [archiveLock],-1
 je .restore
 invoke CloseHandle,[archiveLock]
.restore:
 invoke lstrcpyW,gameDir,addr savedGame
 invoke lstrcpyA,selectedVersion,addr savedVersion
 cmp [engineCancel],0
 je .return
 stdcall SetStatusA,pkMsgCancel
 mov [result],0
.return:
 mov eax,[result]
 ret
endp

include 'pack_import.inc'
