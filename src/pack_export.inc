; Original game-content export. Source profiles/settings are only read.
PE_MAX = 4093
PE_NAME = 0
PE_PATH = 512
PE_BYTES = 4608
PE_ENTRY = 4612

section '.pexpd' data readable writeable
peReady dd 0
peFileCount dd 0
peTotalBytes dd 0
peFormat dd 1
peIncludeSaves dd 0
peIncludeMods dd 1
peIncludeConfig dd 1
peIncludeResources dd 1
peLoaderKind dd 0
peManifestBytes dd 0
peCfgBytes dd 0
peMetaBytes dd 0
peArchiveBytes dd 0
peNameA db 'Exported pack',0,242 dup(0)
peVersionLabelA db '1.0.0',0,122 dup(0)
peSlashW du '\',0
peSlashA db '/',0
peFindSuffix du '\*',0
peTempFormat du '.offset-export-%08X.tmp',0
peMrExt du '.mrpack',0
peZipExt du '.zip',0
peEmptyW dw 0
peEmptyA db 0
peNewline db 13,10,0
peMore db '...',13,10,0
peMsgScan db '正在整理导出内容…',0
peMsgSource db '源版本无效，或包含不安全路径、过大文件。',0
peMsgMetadata db '整合包名称、版本号或作者无效。',0
peMsgDestination db '请选择源实例以外的新文件，扩展名须匹配导出格式。',0
peMsgWrite db '正在生成整合包…',0
peMsgFailed db '整合包导出失败，源实例未更改。',0
peMsgDone db '整合包已导出。',0
peMsgCancel db '整合包导出已取消。',0
peDirMods du 'mods',0
peDirConfig du 'config',0
peDirDefault du 'defaultconfigs',0
peDirKube du 'kubejs',0
peDirScripts du 'scripts',0
peDirResources du 'resources',0
peDirResourcepacks du 'resourcepacks',0
peDirShaderpacks du 'shaderpacks',0
peDirDatapacks du 'datapacks',0
peDirSaves du 'saves',0
peFileOptions du 'options.txt',0
peFileOptionsOf du 'optionsof.txt',0
peFileOptionsShaders du 'optionsshaders.txt',0
peFileServers du 'servers.dat',0
peAllowDirs dd peDirMods,peDirConfig,peDirDefault,peDirKube,peDirScripts,peDirResources,peDirResourcepacks,peDirShaderpacks,peDirDatapacks,peDirSaves
peAllowFiles dd peFileOptions,peFileOptionsOf,peFileOptionsShaders
peSkipLogs du 'logs',0
peSkipCrash du 'crash-reports',0
peSkipCache du 'cache',0
peSkipCaches du 'caches',0
peSkipDotCache du '.cache',0
peSkipVersions du 'versions',0
peSkipAssets du 'assets',0
peSkipLibraries du 'libraries',0
peSkipScreenshots du 'screenshots',0
peSkipDownloads du 'downloads',0
peSkipFabric du '.fabric',0
peSkipForge du '.forge',0
peSkipQuilt du '.quilt',0
peSkipAccounts du 'accounts.json',0
peSkipLauncherAccounts du 'launcher_accounts.json',0
peSkipTokens du 'tokens.json',0
peSkipAuth du 'auth.json',0
peSkipCredentials du 'credentials.json',0
peSkipSession du 'session.json',0
peSkipCf du 'offset.cfkey',0
peSkipSingleToken du 'token.json',0
peSkipTokenText du 'token.txt',0
peSkipAccess du 'accesstoken.json',0
peSkipRefresh du 'refreshtoken.json',0
peSkipEnv du '.env',0
peSkipUserCache du 'usercache.json',0
peSkipNames dd peSkipLogs,peSkipCrash,peSkipCache,peSkipCaches,peSkipDotCache,peSkipVersions,peSkipAssets,peSkipLibraries,peSkipScreenshots,peSkipDownloads,peSkipFabric,peSkipForge,peSkipQuilt,peSkipAccounts,peSkipLauncherAccounts,peSkipTokens,peSkipAuth,peSkipCredentials,peSkipSession,peSkipCf
 dd peSkipSingleToken,peSkipTokenText,peSkipAccess,peSkipRefresh,peSkipEnv,peSkipUserCache
peSkipNamesEnd:
peKeyLibraries db 'libraries',0
peKeyName db 'name',0
peKeyId db 'id',0
pePrefixes dd pePrefixFabric,pePrefixForge,pePrefixNeo,pePrefixQuilt
pePrefixFabric db 'net.fabricmc:fabric-loader:',0
pePrefixForge db 'net.minecraftforge:forge:',0
pePrefixNeo db 'net.neoforged:neoforge:',0
pePrefixQuilt db 'org.quiltmc:quilt-loader:',0
peMrName db 'modrinth.index.json',0
peCfName db 'manifest.json',0
peMmcName db 'mmc-pack.json',0
peCfgName db 'instance.cfg',0
peMetaName db 'offset-export.json',0
peOverridePrefix db 'overrides/',0
peMmcPrefix db '.minecraft/',0
peMrStart db '{"formatVersion":1,"game":"minecraft","name":',0
peMrVersion db ',"versionId":',0
peMrDeps db ',"files":[],"dependencies":{"minecraft":"',0
peCfStart db '{"manifestType":"minecraftModpack","manifestVersion":1,"name":',0
peCfVersion db ',"version":',0
peCfAuthor db ',"author":',0
peCfMc db ',"minecraft":{"version":"',0
peCfLoaders db '","modLoaders":[',0
peCfLoaderStart db '{"id":"',0
peCfLoaderEnd db '","primary":true}',0
peCfEnd db ']},"files":[],"overrides":"overrides"}',0
peMmcStart db '{"formatVersion":1,"components":[{"uid":"net.minecraft","version":"',0
peMmcLoaderStart db ',{"uid":"',0
peMmcLoaderVersion db '","version":"',0
peMmcComponentEnd db '"}',0
peMmcEnd db ']}',0
peCfgStart db '[General]',10,'InstanceType=OneSix',10,'name=',0
peMetaStart db '{"formatVersion":1,"generator":"OFFSET","name":',0
peMetaVersion db ',"version":',0
peMetaAuthor db ',"author":',0
peMetaEnd db '}',0
peMrLoaderKeys dd peMrFabric,peMrForge,peMrNeo,peMrQuilt
peMrFabric db '","fabric-loader":"',0
peMrForge db '","forge":"',0
peMrNeo db '","neoforge":"',0
peMrQuilt db '","quilt-loader":"',0
peCfLoaderPrefixes dd peCfFabric,peCfForge,peCfNeo,peCfQuilt
peCfFabric db 'fabric-',0
peCfForge db 'forge-',0
peCfNeo db 'neoforge-',0
peCfQuilt db 'quilt-',0
peMmcUids dd peMmcFabric,peMmcForge,peMmcNeo,peMmcQuilt
peMmcFabric db 'net.fabricmc.fabric-loader',0
peMmcForge db 'net.minecraftforge',0
peMmcNeo db 'net.neoforged',0
peMmcQuilt db 'org.quiltmc.quilt-loader',0
peCloseDeps db '"}}',0
peQuoteA db '"',0
peBraceA db '}',0
peCommaA db ',',0
peJsonSuffix du '.json',0
peAuthorA rb 256
peVersionA rb 128
peBaseA rb 128
peLoaderA rb 128
peRootW rw 1024
peDataDirW rw 2048
peDestW rw 2048
peTempW rw 2048
peArchiveNameA rb 512
peSummaryA rb 32768
peProfile rb VS_SIZE
peRecords rb PE_MAX*PE_ENTRY
pePathW rw 2048
pePath2W rw 2048
peRelativeW rw 512
peComponentW rw 512
peTextA rb 4096
peText2A rb 2048
peManifestA rb 8192
peCfgA rb 2048
peMetaA rb 2048
peQuotedName rb 1024
peQuotedVersion rb 512
peQuotedAuthor rb 1024


section '.pexpc' code readable executable
 nop

; True only for bounded well-formed UTF-8 display text, never controls.
proc PeValidText uses esi,text,capacity,emptyAllowed
 mov esi,[text]
 xor eax,eax
.length:
 cmp eax,[capacity]
 jae .bad
 cmp byte [esi+eax],0
 je .lengthDone
 inc eax
 jmp .length
.lengthDone:
 test eax,eax
 jnz .utf
 mov eax,[emptyAllowed]
 ret
.utf:
 invoke MultiByteToWideChar,65001,8,[text],-1,peComponentW,512
 test eax,eax
 jz .bad
 mov esi,[text]
.scan:
 lodsb
 test al,al
 jz .good
 cmp al,32
 jb .bad
 cmp al,127
 je .bad
 jmp .scan
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Return 1 to omit a sensitive/cache component, at every tree depth.
proc PeSkipName uses ebx esi edi,nameW
 mov esi,[nameW]
 cmp word [esi],'.'
 jne .names
 add esi,2
 mov edi,.offset
 mov ecx,6
.prefix:
 lodsw
 or ax,20h
 cmp al,[edi]
 jne .names
 inc edi
 loop .prefix
 jmp .skip
.offset db 'offset',0
.names:
 xor ebx,ebx
.loop:
 cmp ebx,(peSkipNamesEnd-peSkipNames)/4
 jae .suffix
 invoke lstrcmpiW,[nameW],dword [peSkipNames+ebx*4]
 test eax,eax
 jz .skip
 inc ebx
 jmp .loop
.suffix:
 invoke lstrlenW,[nameW]
 mov esi,[nameW]
 cmp eax,4
 jb .keep
 lea edi,[esi+eax*2-8]
 invoke lstrcmpiW,edi,.log
 test eax,eax
 jz .skip
 invoke lstrlenW,[nameW]
 cmp eax,7
 jb .crashName
 mov esi,[nameW]
 lea edi,[esi+eax*2-14]
 invoke lstrcmpiW,edi,.logGz
 test eax,eax
 jz .skip
.crashName:
 mov esi,[nameW]
 cmp word [esi],'h'
 jne .keep
 cmp word [esi+2],'s'
 jne .keep
 cmp word [esi+4],'_'
 je .skip
.keep:
 xor eax,eax
 ret
.skip:
 mov eax,1
 ret
.log du '.log',0
.logGz du '.log.gz',0
endp

proc PeLoadProfile uses ebx esi edi
 locals
  text dd ?
  ctx dd ?
  root dd ?
  list dd ?
  count dd ?
  index dd ?
  result dd ?
 endl
 mov [text],0
 mov [ctx],0
 mov [result],0
 stdcall VsRead,peRootW,peVersionA,peProfile,1
 test eax,eax
 jz .done
 invoke lstrcpyW,peDataDirW,peProfile+VS_DATA
 invoke lstrcpyA,peBaseA,peProfile+VS_BASE
 mov eax,dword [peProfile+VS_LOADER]
 mov [peLoaderKind],eax
 mov byte [peLoaderA],0
 test eax,eax
 jz .good
 cmp eax,4
 ja .done
 invoke lstrcpyW,pePathW,peProfile+VS_PROFILE
 invoke lstrcatW,pePathW,peSlashW
 invoke MultiByteToWideChar,65001,8,peVersionA,-1,peComponentW,512
 invoke lstrcatW,pePathW,peComponentW
 invoke lstrcatW,pePathW,peJsonSuffix
 stdcall EReadFile,pePathW
 test eax,eax
 jz .done
 mov [text],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [ctx],eax
 stdcall JsonRoot,eax
 mov [root],eax
 stdcall JsonGet,[ctx],eax,peKeyId
 stdcall PkString,[ctx],eax,peTextA,4096
 test eax,eax
 jz .done
 invoke lstrcmpA,peTextA,peVersionA
 test eax,eax
 jnz .done
 stdcall JsonGet,[ctx],[root],peKeyLibraries
 mov [list],eax
 stdcall JsonCount,[ctx],eax
 cmp eax,4096
 ja .done
 mov [count],eax
 mov [index],0
.library:
 mov eax,[index]
 cmp eax,[count]
 jae .found
 stdcall JsonAt,[ctx],[list],eax
 stdcall JsonGet,[ctx],eax,peKeyName
 stdcall PkString,[ctx],eax,peTextA,4096
 test eax,eax
 jz .next
 mov eax,[peLoaderKind]
 dec eax
 mov edi,[pePrefixes+eax*4]
 stdcall CfPrefix,peTextA,edi
 test eax,eax
 jz .next
 invoke lstrlenA,edi
 lea esi,[peTextA+eax]
 mov edi,peText2A
 xor ecx,ecx
.version:
 lodsb
 test al,al
 jz .terminate
 cmp al,':'
 je .terminate
 cmp ecx,127
 jae .done
 stosb
 inc ecx
 jmp .version
.terminate:
 mov byte [edi],0
 mov esi,peText2A
 cmp [peLoaderKind],2
 jne .validate
 stdcall CfPrefix,esi,peBaseA
 test eax,eax
 jz .validate
 invoke lstrlenA,peBaseA
 cmp byte [esi+eax],'-'
 jne .validate
 lea esi,[esi+eax+1]
.validate:
 stdcall LoaderSafeId,esi
 test eax,eax
 jz .done
 invoke lstrlenA,esi
 cmp eax,127
 ja .done
 cmp byte [peLoaderA],0
 je .copy
 invoke lstrcmpA,peLoaderA,esi
 test eax,eax
 jnz .done
 jmp .next
.copy:
 invoke lstrcpyA,peLoaderA,esi
.next:
 inc [index]
 jmp .library
.found:
 cmp byte [peLoaderA],0
 je .done
.good:
 mov [result],1
.done:
 cmp [ctx],0
 je .free
 stdcall JsonFree,[ctx]
.free:
 stdcall FreeMem,[text]
 mov eax,[result]
 ret
endp

; The allowlist is applied only at the data root. Sensitive/cache names are
; omitted recursively. Reparse entries in included content fail closed.
proc PeWalk uses ebx esi edi,relativeW,depth
 locals
  find WIN32_FIND_DATAW
  handle dd ?
  path rw 2048
  child rw 512
  archiveA rb 512
  result dd ?
  index dd ?
 endl
 mov [result],0
 mov [handle],-1
 cmp [depth],40
 ja .done
 cmp [engineCancel],0
 jne .done
 cmp [modCancel],0
 jne .done
 invoke lstrcpyW,addr path,peDataDirW
 invoke lstrlenW,addr path
 mov ebx,eax
 invoke lstrlenW,[relativeW]
 add eax,ebx
 cmp eax,1950
 ja .done
 mov esi,[relativeW]
 cmp word [esi],0
 je .wildcard
 invoke lstrcatW,addr path,peSlashW
 invoke lstrcatW,addr path,[relativeW]
.wildcard:
 stdcall VsDirectoryChain,addr path,0
 test eax,eax
 jz .done
 invoke lstrcatW,addr path,peFindSuffix
 invoke FindFirstFileW,addr path,addr find
 cmp eax,-1
 jne .found
 invoke GetLastError
 cmp eax,2
 je .empty
 cmp eax,18
 je .empty
 jmp .done
.found:
 mov [handle],eax
.entry:
 cmp [engineCancel],0
 jne .done
 cmp [modCancel],0
 jne .done
 cmp word [find.cFileName],'.'
 jne .skipNames
 cmp word [find.cFileName+2],0
 je .next
 cmp word [find.cFileName+2],'.'
 jne .skipNames
 cmp word [find.cFileName+4],0
 je .next
.skipNames:
 stdcall PeSkipName,addr find.cFileName
 test eax,eax
 jnz .next
 cmp [depth],0
 jne .included
 test [find.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY
 jz .rootFile
 mov [index],0
.dirAllow:
 mov eax,[index]
 cmp eax,10
 jae .next
 invoke lstrcmpiW,addr find.cFileName,dword [peAllowDirs+eax*4]
 test eax,eax
 jnz .dirNext
 mov eax,[index]
 cmp eax,9
 je .saveFlag
 test eax,eax
 jz .modFlag
 cmp eax,5
 jae .resourceFlag
 cmp [peIncludeConfig],1
 jne .next
 jmp .included
.modFlag:
 cmp [peIncludeMods],1
 jne .next
 jmp .included
.resourceFlag:
 cmp [peIncludeResources],1
 jne .next
 jmp .included
.saveFlag:
 cmp [peIncludeSaves],1
 jne .next
 jmp .included
.dirNext:
 inc [index]
 jmp .dirAllow
.rootFile:
 cmp [peIncludeConfig],1
 jne .next
 mov [index],0
.fileAllow:
 mov eax,[index]
 cmp eax,3
 jae .next
 invoke lstrcmpiW,addr find.cFileName,dword [peAllowFiles+eax*4]
 test eax,eax
 jz .included
 inc [index]
 jmp .fileAllow
.included:
 test [find.dwFileAttributes],400h
 jnz .done
 invoke lstrcpyW,addr child,[relativeW]
 cmp word [child],0
 je .childName
 invoke lstrcatW,addr child,peSlashW
.childName:
 invoke lstrlenW,addr child
 mov ebx,eax
 invoke lstrlenW,addr find.cFileName
 add eax,ebx
 cmp eax,500
 ja .done
 invoke lstrcatW,addr child,addr find.cFileName
 test [find.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY
 jz .file
 mov eax,[depth]
 inc eax
 stdcall PeWalk,addr child,eax
 test eax,eax
 jz .done
 jmp .next
.file:
 cmp [find.nFileSizeHigh],0
 jne .done
 cmp [find.nFileSizeLow],134217728
 ja .done
 mov eax,[find.nFileSizeLow]
 add eax,[peTotalBytes]
 jc .done
 cmp eax,2147483647
 ja .done
 mov [peTotalBytes],eax
 cmp [peFileCount],PE_MAX
 jae .done
 invoke WideCharToMultiByte,65001,0,addr child,-1,addr archiveA,501,0,0
 test eax,eax
 jz .done
 lea esi,[archiveA]
.slashes:
 lodsb
 test al,al
 jz .safe
 cmp al,92
 jne .slashes
 mov byte [esi-1],'/'
 jmp .slashes
.safe:
 stdcall PkPayloadSafe,addr archiveA
 test eax,eax
 jz .done
 mov eax,[peFileCount]
 imul ebx,eax,PE_ENTRY
 add ebx,peRecords
 invoke lstrcpyA,ebx,addr archiveA
 lea edi,[ebx+PE_PATH]
 invoke lstrcpyW,edi,peDataDirW
 invoke lstrcatW,edi,peSlashW
 invoke lstrcatW,edi,addr child
 mov eax,[find.nFileSizeLow]
 mov [ebx+PE_BYTES],eax
 mov [index],0
.unique:
 mov eax,[index]
 cmp eax,[peFileCount]
 jae .summary
 imul eax,PE_ENTRY
 lea esi,[peRecords+eax+PE_PATH]
 lea edi,[ebx+PE_PATH]
 invoke lstrcmpiW,esi,edi
 test eax,eax
 jz .done
 inc [index]
 jmp .unique
.summary:
 cmp [peFileCount],60
 jae .count
 stdcall ModAppend,peSummaryA,32768,addr archiveA
 stdcall ModAppend,peSummaryA,32768,peNewline
.count:
 inc [peFileCount]
.next:
 invoke FindNextFileW,[handle],addr find
 test eax,eax
 jnz .entry
 invoke GetLastError
 cmp eax,18
 jne .done
.empty:
 mov [result],1
.done:
 cmp [handle],-1
 je .return
 invoke FindClose,[handle]
.return:
 mov eax,[result]
 ret
endp

proc PackExportInspect uses ebx,rootW,idA,includeSaves
 mov [peReady],0
 mov [peFileCount],0
 mov [peTotalBytes],0
 mov byte [peSummaryA],0
 cmp [includeSaves],1
 ja .bad
 invoke lstrlenW,[rootW]
 cmp eax,1024
 jae .bad
 invoke lstrlenA,[idA]
 cmp eax,128
 jae .bad
 invoke lstrcpyW,peRootW,[rootW]
 invoke lstrcpyA,peVersionA,[idA]
 mov eax,[includeSaves]
 mov [peIncludeSaves],eax
 stdcall SetStatusA,peMsgScan
 stdcall PeLoadProfile
 test eax,eax
 jz .bad
 stdcall PeWalk,peEmptyW,0
 test eax,eax
 jz .bad
 cmp [peFileCount],60
 jbe .ready
 stdcall ModAppend,peSummaryA,32768,peMore
.ready:
 mov [peReady],1
 mov eax,1
 ret
.bad:
 stdcall SetStatusA,peMsgSource
 cmp [engineCancel],0
 jne .cancel
 cmp [modCancel],0
 je .return
.cancel:
 stdcall SetStatusA,peMsgCancel
.return:
 xor eax,eax
 ret
endp

proc PeQuote uses ebx esi edi,source,dest,capacity
 mov esi,[source]
 mov edi,[dest]
 mov ebx,[capacity]
 cmp ebx,3
 jb .bad
 mov byte [edi],'"'
 inc edi
 sub ebx,2
.loop:
 lodsb
 test al,al
 jz .close
 cmp al,'"'
 je .escape
 cmp al,92
 jne .copy
.escape:
 cmp ebx,2
 jbe .bad
 mov byte [edi],92
 inc edi
 dec ebx
.copy:
 cmp ebx,1
 jbe .bad
 stosb
 dec ebx
 jmp .loop
.close:
 mov byte [edi],'"'
 mov byte [edi+1],0
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Metadata is deliberately independent of platform distribution IDs. Export
; contains local game payload and never invents downloaded-file provenance.
proc PeMakeManifest uses ebx esi edi
 stdcall PeValidText,peNameA,256,0
 test eax,eax
 jz .bad
 stdcall PeValidText,peVersionLabelA,128,0
 test eax,eax
 jz .bad
 stdcall PeValidText,peAuthorA,256,1
 test eax,eax
 jz .bad
 cmp [peFormat],1
 jb .bad
 cmp [peFormat],3
 ja .bad
 stdcall PeQuote,peNameA,peQuotedName,1024
 test eax,eax
 jz .bad
 stdcall PeQuote,peVersionLabelA,peQuotedVersion,512
 test eax,eax
 jz .bad
 stdcall PeQuote,peAuthorA,peQuotedAuthor,1024
 test eax,eax
 jz .bad
 invoke lstrcpyA,peMetaA,peMetaStart
 stdcall ModAppend,peMetaA,2048,peQuotedName
 stdcall ModAppend,peMetaA,2048,peMetaVersion
 stdcall ModAppend,peMetaA,2048,peQuotedVersion
 stdcall ModAppend,peMetaA,2048,peMetaAuthor
 stdcall ModAppend,peMetaA,2048,peQuotedAuthor
 stdcall ModAppend,peMetaA,2048,peMetaEnd
 test eax,eax
 jz .bad
 invoke lstrlenA,peMetaA
 mov [peMetaBytes],eax
 cmp [peFormat],2
 je .curseforge
 cmp [peFormat],3
 je .prism
 invoke lstrcpyA,peManifestA,peMrStart
 stdcall ModAppend,peManifestA,8192,peQuotedName
 stdcall ModAppend,peManifestA,8192,peMrVersion
 stdcall ModAppend,peManifestA,8192,peQuotedVersion
 stdcall ModAppend,peManifestA,8192,peMrDeps
 stdcall ModAppend,peManifestA,8192,peBaseA
 mov eax,[peLoaderKind]
 test eax,eax
 jz .mrEnd
 dec eax
 stdcall ModAppend,peManifestA,8192,dword [peMrLoaderKeys+eax*4]
 stdcall ModAppend,peManifestA,8192,peLoaderA
.mrEnd:
 stdcall ModAppend,peManifestA,8192,peCloseDeps
 jmp .finish
.curseforge:
 invoke lstrcpyA,peManifestA,peCfStart
 stdcall ModAppend,peManifestA,8192,peQuotedName
 stdcall ModAppend,peManifestA,8192,peCfVersion
 stdcall ModAppend,peManifestA,8192,peQuotedVersion
 stdcall ModAppend,peManifestA,8192,peCfAuthor
 stdcall ModAppend,peManifestA,8192,peQuotedAuthor
 stdcall ModAppend,peManifestA,8192,peCfMc
 stdcall ModAppend,peManifestA,8192,peBaseA
 stdcall ModAppend,peManifestA,8192,peCfLoaders
 mov ebx,[peLoaderKind]
 test ebx,ebx
 jz .cfEnd
 stdcall ModAppend,peManifestA,8192,peCfLoaderStart
 dec ebx
 stdcall ModAppend,peManifestA,8192,dword [peCfLoaderPrefixes+ebx*4]
 stdcall ModAppend,peManifestA,8192,peLoaderA
 stdcall ModAppend,peManifestA,8192,peCfLoaderEnd
.cfEnd:
 stdcall ModAppend,peManifestA,8192,peCfEnd
 jmp .finish
.prism:
 ; Qt INI uses backslash escapes and @-typed values. Plain display names
 ; without those delimiters are emitted unchanged and round-trip in OFFSET.
 mov esi,peNameA
 cmp byte [esi],'@'
 je .bad
 cmp byte [esi],' '
 je .bad
.cfgSafe:
 lodsb
 test al,al
 jz .cfgEnd
 cmp al,92
 je .bad
 cmp al,'"'
 je .bad
 jmp .cfgSafe
.cfgEnd:
 cmp byte [esi-2],' '
 je .bad
 invoke lstrcpyA,peCfgA,peCfgStart
 stdcall ModAppend,peCfgA,2048,peNameA
 stdcall ModAppend,peCfgA,2048,peNewline
 invoke lstrlenA,peCfgA
 mov [peCfgBytes],eax
 invoke lstrcpyA,peManifestA,peMmcStart
 stdcall ModAppend,peManifestA,8192,peBaseA
 stdcall ModAppend,peManifestA,8192,peMmcComponentEnd
 mov ebx,[peLoaderKind]
 test ebx,ebx
 jz .mmcEnd
 stdcall ModAppend,peManifestA,8192,peMmcLoaderStart
 dec ebx
 stdcall ModAppend,peManifestA,8192,dword [peMmcUids+ebx*4]
 stdcall ModAppend,peManifestA,8192,peMmcLoaderVersion
 stdcall ModAppend,peManifestA,8192,peLoaderA
 stdcall ModAppend,peManifestA,8192,peMmcComponentEnd
.mmcEnd:
 stdcall ModAppend,peManifestA,8192,peMmcEnd
.finish:
 test eax,eax
 jz .bad
 invoke lstrlenA,peManifestA
 mov [peManifestBytes],eax
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Case-insensitive component boundary comparison for canonical Windows paths.
proc PeUnderRoot uses ebx esi edi,pathW,rootW
 locals
  prefix rw 2048
 endl
 invoke lstrlenW,[rootW]
 mov ebx,eax
 invoke lstrlenW,[pathW]
 cmp eax,ebx
 jb .no
 mov esi,[pathW]
 lea edi,[prefix]
 mov ecx,ebx
 rep movsw
 mov word [edi],0
 invoke lstrcmpiW,addr prefix,[rootW]
 test eax,eax
 jnz .no
 mov edi,[rootW]
 cmp word [edi+ebx*2-2],92
 je .yes
 mov esi,[pathW]
 cmp word [esi+ebx*2],0
 je .yes
 cmp word [esi+ebx*2],92
 je .yes
.no:
 xor eax,eax
 ret
.yes:
 mov eax,1
 ret
endp

proc PeDestination uses ebx esi edi
 invoke lstrlenW,peDestW
 test eax,eax
 jz .bad
 cmp eax,1900
 jae .bad
 invoke GetFullPathNameW,peDestW,2048,pePathW,0
 test eax,eax
 jz .bad
 cmp eax,1900
 jae .bad
 invoke lstrcpyW,peDestW,pePathW
 stdcall PeUnderRoot,peDestW,peProfile+VS_ROOT
 test eax,eax
 jnz .bad
 stdcall PeUnderRoot,peDestW,peDataDirW
 test eax,eax
 jnz .bad
 invoke GetFileAttributesW,peDestW
 cmp eax,-1
 jne .bad
 invoke lstrlenW,peDestW
 mov ebx,eax
 mov esi,peZipExt
 cmp [peFormat],1
 jne .extension
 mov esi,peMrExt
.extension:
 invoke lstrlenW,esi
 cmp ebx,eax
 jbe .bad
 sub ebx,eax
 lea edi,[peDestW+ebx*2]
 invoke lstrcmpiW,edi,esi
 test eax,eax
 jnz .bad
 invoke lstrcpyW,pePathW,peDestW
 invoke lstrlenW,pePathW
 lea edi,[pePathW+eax*2]
.parent:
 sub edi,2
 cmp edi,pePathW
 jbe .bad
 cmp word [edi],92
 jne .parent
 lea esi,[edi+2]
 invoke lstrlenW,esi
 stdcall PkComponentSafe,esi,eax
 test eax,eax
 jz .bad
 cmp edi,pePathW+4
 jne .split
 cmp word [pePathW+2],':'
 jne .split
 add edi,2
.split:
 mov word [edi],0
 stdcall VsDirectoryChain,pePathW,0
 ret
.bad:
 xor eax,eax
 ret
endp

proc PeSizeAdd,nameA,bytes
 invoke lstrlenA,[nameA]
 shl eax,1
 add eax,76
 add eax,[bytes]
 jc .bad
 add eax,[peArchiveBytes]
 jc .bad
 cmp eax,536870912
 ja .bad
 mov [peArchiveBytes],eax
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PeEstimate uses ebx esi edi
 local index:DWORD
 mov [peArchiveBytes],22
 mov eax,peMrName
 cmp [peFormat],1
 je .main
 mov eax,peCfName
 cmp [peFormat],2
 je .main
 mov eax,peMmcName
.main:
 stdcall PeSizeAdd,eax,[peManifestBytes]
 test eax,eax
 jz .bad
 stdcall PeSizeAdd,peMetaName,[peMetaBytes]
 test eax,eax
 jz .bad
 cmp [peFormat],3
 jne .files
 stdcall PeSizeAdd,peCfgName,[peCfgBytes]
 test eax,eax
 jz .bad
.files:
 mov [index],0
.file:
 mov eax,[index]
 cmp eax,[peFileCount]
 jae .good
 imul ebx,eax,PE_ENTRY
 add ebx,peRecords
 mov eax,peOverridePrefix
 cmp [peFormat],3
 jne .prefix
 mov eax,peMmcPrefix
.prefix:
 invoke lstrcpyA,peArchiveNameA,eax
 stdcall ModAppend,peArchiveNameA,512,ebx
 test eax,eax
 jz .bad
 stdcall PeSizeAdd,peArchiveNameA,dword [ebx+PE_BYTES]
 test eax,eax
 jz .bad
 inc [index]
 jmp .file
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PackExportBuild uses ebx esi edi
 locals
  root rw 1024
  id rb 128
  saves dd ?
  result dd ?
  index dd ?
  owned dd ?
 endl
 mov [result],0
 mov [owned],0
 invoke lstrcpyW,addr root,peRootW
 invoke lstrcpyA,addr id,peVersionA
 mov eax,[peIncludeSaves]
 mov [saves],eax
 stdcall PackExportInspect,addr root,addr id,[saves]
 test eax,eax
 jz .done
 stdcall PeMakeManifest
 test eax,eax
 jz .metadata
 stdcall PeEstimate
 test eax,eax
 jz .failed
 stdcall PeDestination
 test eax,eax
 jz .destination
 invoke lstrcpyW,peTempW,peDestW
 invoke GetTickCount
 cinvoke wsprintfW,pePathW,peTempFormat,eax
 invoke lstrcatW,peTempW,pePathW
 stdcall SetStatusA,peMsgWrite
 stdcall ExZipBegin,peTempW
 mov ebx,eax
 mov eax,[exZipOwned]
 mov [owned],eax
 test ebx,ebx
 jz .failed
 mov eax,peMrName
 cmp [peFormat],1
 je .manifest
 mov eax,peCfName
 cmp [peFormat],2
 je .manifest
 mov eax,peMmcName
.manifest:
 stdcall ExZipAddMemory,eax,peManifestA,[peManifestBytes]
 test eax,eax
 jz .failed
 cmp [peFormat],3
 jne .meta
 stdcall ExZipAddMemory,peCfgName,peCfgA,[peCfgBytes]
 test eax,eax
 jz .failed
.meta:
 stdcall ExZipAddMemory,peMetaName,peMetaA,[peMetaBytes]
 test eax,eax
 jz .failed
 mov [index],0
.file:
 mov eax,[index]
 cmp eax,[peFileCount]
 jae .finish
 cmp [engineCancel],0
 jne .failed
 cmp [modCancel],0
 jne .failed
 imul ebx,eax,PE_ENTRY
 add ebx,peRecords
 mov eax,peOverridePrefix
 cmp [peFormat],3
 jne .prefix
 mov eax,peMmcPrefix
.prefix:
 invoke lstrcpyA,peArchiveNameA,eax
 stdcall ModAppend,peArchiveNameA,512,ebx
 test eax,eax
 jz .failed
 lea edi,[ebx+PE_PATH]
 ; Source ancestry is rechecked immediately before the writer pins the file.
 invoke lstrcpyW,pePathW,edi
 invoke lstrlenW,pePathW
 lea esi,[pePathW+eax*2]
.parent:
 sub esi,2
 cmp word [esi],92
 jne .parent
 mov word [esi],0
 stdcall VsDirectoryChain,pePathW,0
 test eax,eax
 jz .failed
 stdcall ExZipAddFile,peArchiveNameA,edi
 test eax,eax
 jz .failed
 inc [index]
 jmp .file
.finish:
 stdcall ExZipFinish
 test eax,eax
 jz .failed
 ; Validate our complete ZIP and manifest using the real import preflight
 ; before publishing anything at the requested path.
 stdcall PackInspect,peTempW
 test eax,eax
 jz .failed
 cmp [engineCancel],0
 jne .failed
 cmp [modCancel],0
 jne .failed
 invoke MoveFileExW,peTempW,peDestW,0
 test eax,eax
 jz .destination
 mov [owned],0
 mov [result],1
 stdcall SetStatusA,peMsgDone
 jmp .done
.metadata:
 stdcall SetStatusA,peMsgMetadata
 jmp .done
.destination:
 stdcall SetStatusA,peMsgDestination
 jmp .done
.failed:
 stdcall SetStatusA,peMsgFailed
.done:
 stdcall ExZipAbort
 cmp [owned],0
 je .status
 invoke DeleteFileW,peTempW
.status:
 cmp [engineCancel],0
 jne .cancel
 cmp [modCancel],0
 je .return
.cancel:
 stdcall SetStatusA,peMsgCancel
.return:
 mov eax,[result]
 ret
endp

include 'pack_export_zip.inc'
