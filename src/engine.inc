; OFFSET Launcher - original native Minecraft engine (FASM x86).
; All paths are UTF-16. Network metadata and identities are UTF-8.
section '.engdata' data readable writeable
engineCancel dd 0
eRuntimeIsolated dd 0
eMeta dd 0
eMetaText dd 0
eRoot dd 0
eIndex dd 0
eIndexText dd 0
eIndexRoot dd 0
eHeap dd 0
eCount dd 0
eItem dd 0
eLibs dd 0
eTok dd 0
eNatives dd 0
eFile dd 0
eResult dd 0
eJLen dd 0
eCLen dd 0
eOutLen dd 0
eArgFile dd 0
ePhase dd 0
eAssetCount dd 0
eAssetJobCount dd 0
eAssetNext dd 0
eAssetDone dd 0
eAssetError dd 0
eAssetThreads dd 8 dup(0)
eDigestLen dd 16
eProvider dd 0
eHashHandle dd 0
eVersions du '\versions\',0
eMinecraft du '\.minecraft',0
eLibraries du '\libraries\',0
eAssets du '\assets',0
eObjects du '\assets\objects\',0
eIndexes du '\assets\indexes\',0
eNativesSuffix du '\offset-natives',0
eArgsSuffix du '\offset-jvm.args',0
eJsonExt du '.json',0
eJarExt du '.jar',0
eGlob du '*',0
eSlash du '\',0
eJavaEnv du 'JAVA_HOME',0
eJavaBin du '\bin\javaw.exe',0
eJavaName du 'javaw.exe',0
eTarName du '\tar.exe',0
eTarNative du '\Sysnative\tar.exe',0
eQuoteW du '"',0
eTarSwitch du '" -xf "',0
eTarDest du '" -C "',0
eTarExclude du '" --exclude META-INF',0
eKeyVersions db 'versions',0
eKeyId db 'id',0
eKeyUrl db 'url',0
eKeySha db 'sha1',0
eKeyDownloads db 'downloads',0
eKeyClient db 'client',0
eKeyLibraries db 'libraries',0
eKeyArtifact db 'artifact',0
eKeyPath db 'path',0
eKeyRules db 'rules',0
eKeyAction db 'action',0
eKeyOs db 'os',0
eKeyName db 'name',0
eKeyArch db 'arch',0
eKeyFeatures db 'features',0
eKeyVersion db 'version',0
eKeyInherits db 'inheritsFrom',0
eKeyNormalized db 'offsetNormalized',0
eKeyArguments db 'arguments',0
eKeyJvm db 'jvm',0
eKeyGame db 'game',0
eKeyValue db 'value',0
eKeyMain db 'mainClass',0
eKeyType db 'type',0
eKeyAssets db 'assetIndex',0
eKeyObjects db 'objects',0
eKeyHash db 'hash',0
eKeyNatives db 'natives',0
eKeyWindows db 'windows',0
eKeyClassifiers db 'classifiers',0
eWindows db 'windows',0
eArch db 'x86_64',0
eAllow db 'allow',0
eEmpty db 0
eOfflinePrefix db 'OfflinePlayer:',0
eHex db '0123456789abcdef'
eAssetBase db 'https://resources.download.minecraft.net/',0
eFmtProgress db 'Assets: %u / %u verified',0
eFmtRam db '-Xmx%um',0
if defined OFFSET_LAUNCH_LOG
; JDK 19+ standard properties, plus the OpenJDK 9-18 standard-stream aliases.
; Limit this choice to captured streams; preserve file.encoding and native paths.
eLogStdout db '-Dstdout.encoding=UTF-8',0
eLogStderr db '-Dstderr.encoding=UTF-8',0
eLogLegacyStdout db '-Dsun.stdout.encoding=UTF-8',0
eLogLegacyStderr db '-Dsun.stderr.encoding=UTF-8',0
end if
eCpFlag db '-cp',0
eNativeArg db '-Djava.library.path=${natives_directory}',0
eSlashA db '\',0
eSemi db ';',0
eNewline db 13,10,0
eLauncher db 'OFFSET',0
eLauncherVersion db '0.7.0',0
eUserType db 'msa',0
eOfflineToken db '0',0
eUserProps db '{}',0
eZeroUuid db '00000000-0000-0000-0000-000000000000',0
eDefaultType db 'release',0,24 dup(0)
eReleaseType db 'release',0
eRuntimeParent db '../../../',0
eArgPrefix db '" @"',0
eQuoteA db '"',0
eSpaceA db ' ',0
eStatusScan db 'Checking Minecraft metadata...',0
eStatusManifest db 'Fetching Minecraft version list...',0
eStatusInstall db 'Verifying client, libraries and assets...',0
eStatusLaunch db 'Starting Minecraft...',0
eStatusRunning db 'Minecraft is running. Close the game to return.',0
eStatusDone db 'Minecraft has exited. Ready to launch again.',0
eStatusInstalled db 'Installation verified. Ready to launch.',0
eErrVersion db 'Enter a valid official Minecraft version ID (for example 1.21.1).',0
eErrUnknown db 'Version was not found in the official Mojang manifest.',0
eErrNetwork db 'Download failed or SHA-1 verification failed. Check your connection and retry.',0
eErrMetadata db 'Invalid Minecraft metadata. Installation stopped safely.',0
eErrInherited db 'This version inherits another profile. This build supports official vanilla versions only.',0
eErrOld db 'This version uses legacy launch metadata. Choose vanilla Minecraft 1.13 or newer.',0
eErrPath db 'A metadata path is invalid or too long.',0
eErrJava db 'Java was not found. Set a full path to a suitable 64-bit javaw.exe.',0
eErrName db 'Offline names must contain 3-16 ASCII letters, digits or underscores.',0
eErrAuth db 'Sign in to Microsoft before launching in online mode.',0
eErrStart db 'Windows could not start Java. Check Java path, game folder and memory setting.',0
eErrArgs db 'Launch arguments exceed the supported bound or contain an unknown placeholder.',0
eErrTar db 'Native extraction failed. Windows tar.exe is required for this Minecraft version.',0
eErrExit db 'Minecraft exited with an error. Check the game logs/latest.log and Java version.',0
eErrCancel db 'Operation cancelled. Verified downloaded files are kept.',0
; Placeholder table. Pointer pairs terminated by zero; values resolved at launch.
ePlaceholders dd ePhName,eName,ePhVersion,selectedVersion,ePhGame,eGameUtf,ePhAssets,eAssetsUtf,ePhIndex,eIndexId,ePhUuid,eUuid,ePhToken,authToken,ePhType,eUserType,ePhProps,eUserProps,ePhNative,eNativeUtf,ePhLauncher,eLauncher,ePhLauncherVer,eLauncherVersion,ePhClass,eClasspath,ePhLib,eLibUtf,ePhSep,eSemi,ePhClient,clientId,ePhXuid,authXuid,ePhVerType,eDefaultType,0,0
ePhName db 'auth_player_name}',0
ePhVersion db 'version_name}',0
ePhGame db 'game_directory}',0
ePhAssets db 'assets_root}',0
ePhIndex db 'assets_index_name}',0
ePhUuid db 'auth_uuid}',0
ePhToken db 'auth_access_token}',0
eSecretPh db '${auth_access_token}',0
ePhType db 'user_type}',0
ePhProps db 'user_properties}',0
ePhNative db 'natives_directory}',0
ePhLauncher db 'launcher_name}',0
ePhLauncherVer db 'launcher_version}',0
ePhClass db 'classpath}',0
ePhLib db 'library_directory}',0
ePhSep db 'classpath_separator}',0
ePhClient db 'clientid}',0
ePhXuid db 'auth_xuid}',0
ePhVerType db 'version_type}',0

eInfo STARTUPINFO
eProcess PROCESS_INFORMATION
eFind WIN32_FIND_DATAW
ePath rw 2048
ePath2 rw 2048
eMetaPath rw 2048
eClientPath rw 2048
eNativePath rw 2048
eArgPath rw 2048
eRuntimeDirW rw 2048
eTarPath rw 1024
eWide rw 8192
eCmd rw 32768
eVersionW rw 128
eUrl rb 4096
eHash rb 64
eRel rb 4096
eValue rb 65536
eExpanded rb 1200000
eGameUtf rb 8192
eNativeUtf rb 8192
eLibUtf rb 8192
eAssetsUtf rb 8192
eIndexId rb 128
eUuid rb 64
eName rb 64
eIdentity rb 128
eDigest rb 16
eClasspath rb 1048576
eArgs rb 2097152
eCommand rb 32768
eScratch rb 32768
eBuf rb 4096
eMsg rb 512

eAssetJobs rd 65536
eAssetLocks rd 256

section '.engcode' code readable executable
proc EngineInit uses ebx esi edi
 invoke GetProcessHeap
 mov [eHeap],eax
 cmp word [gameDir],0
 jne .java
 invoke GetModuleFileNameW,0,gameDir,1024
 mov esi,gameDir
 mov edi,esi
 .walk:
 mov ax,[esi]
 test ax,ax
 jz .cut
 cmp ax,'\'
 jne @f
 mov edi,esi
 @@: add esi,2
 jmp .walk
 .cut: mov word [edi],0
 invoke lstrcatW,gameDir,eMinecraft
 .java:
 cmp word [javaPath],0
 jne .done
 invoke GetEnvironmentVariableW,eJavaEnv,javaPath,1000
 test eax,eax
 jz .search
 cmp eax,990
 jae .search
 invoke lstrcatW,javaPath,eJavaBin
 invoke GetFileAttributesW,javaPath
 cmp eax,-1
 jne .done
 .search:
 mov word [javaPath],0
 invoke SearchPathW,0,eJavaName,0,1024,javaPath,0
 .done:
 stdcall EnsureDirectory,gameDir
 ret
endp

proc EngineScanVersions uses ebx esi edi, combo
 local handle:DWORD, count:DWORD
 mov [count],0
 invoke SendMessageA,[combo],CB_RESETCONTENT,0,0
 invoke lstrcpyW,ePath,gameDir
 invoke lstrcatW,ePath,eVersions
 invoke lstrcatW,ePath,eGlob
 invoke FindFirstFileW,ePath,eFind
 cmp eax,-1
 je .done
 mov [handle],eax
 .next:
 test [eFind.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY
 jz .advance
 cmp word [eFind.cFileName],'.'
 je .advance
 invoke WideCharToMultiByte,65001,0,eFind.cFileName,-1,eValue,128,0,0
 test eax,eax
 jz .advance
 invoke lstrcpyW,ePath2,gameDir
 invoke lstrcatW,ePath2,eVersions
 invoke lstrcatW,ePath2,eFind.cFileName
 invoke lstrcatW,ePath2,eSlash
 invoke lstrcatW,ePath2,eFind.cFileName
 invoke lstrcatW,ePath2,eJsonExt
 invoke GetFileAttributesW,ePath2
 cmp eax,-1
 je .advance
 stdcall EStandaloneProfile,ePath2
 test eax,eax
 jz .advance
 invoke lstrcpyW,ePath2,gameDir
 invoke lstrcatW,ePath2,eVersions
 invoke lstrcatW,ePath2,eFind.cFileName
 invoke lstrcatW,ePath2,eSlash
 invoke lstrcatW,ePath2,eFind.cFileName
 invoke lstrcatW,ePath2,eJarExt
 invoke GetFileAttributesW,ePath2
 cmp eax,-1
 je .advance
 invoke SendMessageA,[combo],CB_ADDSTRING,0,eValue
 inc [count]
 .advance:
 invoke FindNextFileW,[handle],eFind
 test eax,eax
 jnz .next
 invoke FindClose,[handle]
 .done:
 mov eax,[count]
 ret
endp

; Unfinished installer profiles remain hidden until inheritance is normalized.
proc EStandaloneProfile uses ebx esi,path
 local text:DWORD,context:DWORD,root:DWORD,result:DWORD
 mov [result],0
 mov [context],0
 stdcall EReadFile,[path]
 mov [text],eax
 test eax,eax
 jz .done
 stdcall JsonParse,eax
 mov [context],eax
 test eax,eax
 jz .done
 stdcall JsonRoot,eax
 mov [root],eax
 stdcall JsonGet,[context],eax,eKeyInherits
 test eax,eax
 jnz .done
 stdcall JsonGet,[context],[root],eKeyArguments
 test eax,eax
 jz .done
 stdcall JsonGet,[context],[root],eKeyMain
 test eax,eax
 jz .done
 mov [result],1
.done:
 stdcall JsonFree,[context]
 stdcall FreeMem,[text]
 mov eax,[result]
 ret
endp

; Version IDs are local path components. Accept only official ID characters.
proc EValidateVersion uses esi
 mov esi,selectedVersion
 xor ecx,ecx
 .next:
 mov al,[esi+ecx]
 test al,al
 jz .vend
 cmp ecx,120
 jae .bad
 cmp al,'.'
 je .dot
 cmp al,'-'
 je .good
 cmp al,'_'
 je .good
 cmp al,'0'
 jb .bad
 cmp al,'9'
 jbe .good
 cmp al,'A'
 jb .bad
 cmp al,'Z'
 jbe .good
 cmp al,'a'
 jb .bad
 cmp al,'z'
 ja .bad
 .good: inc ecx
 jmp .next
 .dot:
 cmp ecx,0
 je .bad
 cmp byte [esi+ecx+1],'.'
 je .bad
 jmp .good
 .vend:
 test ecx,ecx
 jz .bad
 invoke MultiByteToWideChar,65001,0,selectedVersion,-1,eVersionW,128
 ret
 .bad: xor eax,eax
 ret
endp

proc EBuildPaths
 invoke lstrcpyW,eMetaPath,gameDir
 invoke lstrcatW,eMetaPath,eVersions
 invoke lstrcatW,eMetaPath,eVersionW
 invoke lstrcatW,eMetaPath,eSlash
 invoke lstrcatW,eMetaPath,eVersionW
 invoke lstrcatW,eMetaPath,eJsonExt
 invoke lstrcpyW,eClientPath,gameDir
 invoke lstrcatW,eClientPath,eVersions
 invoke lstrcatW,eClientPath,eVersionW
 invoke lstrcatW,eClientPath,eSlash
 invoke lstrcatW,eClientPath,eVersionW
 invoke lstrcatW,eClientPath,eJarExt
 invoke lstrcpyW,eNativePath,gameDir
 invoke lstrcatW,eNativePath,eVersions
 invoke lstrcatW,eNativePath,eVersionW
 invoke lstrcatW,eNativePath,eNativesSuffix
 invoke lstrcpyW,eArgPath,gameDir
 invoke lstrcatW,eArgPath,eVersions
 invoke lstrcatW,eArgPath,eVersionW
 invoke lstrcatW,eArgPath,eArgsSuffix
 ret
endp

proc EReadFile uses ebx esi, path
 local h:DWORD, size:DWORD, got:DWORD
 invoke CreateFileW,[path],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [h],eax
 invoke GetFileSize,eax,0
 cmp eax,33554430
 ja .closebad
 mov [size],eax
 inc eax
 invoke HeapAlloc,[eHeap],HEAP_ZERO_MEMORY,eax
 test eax,eax
 jz .closebad
 mov esi,eax
 invoke ReadFile,[h],esi,[size],addr got,0
 mov ebx,eax
 invoke CloseHandle,[h]
 test ebx,ebx
 jz .freebad
 mov eax,[got]
 cmp eax,[size]
 jne .freebad
 mov eax,esi
 ret
 .freebad: stdcall FreeMem,esi
 jmp .bad
 .closebad: invoke CloseHandle,[h]
 .bad: xor eax,eax
 ret
endp

proc ELoadMetadata
 cmp [eMeta],0
 je @f
 stdcall JsonFree,[eMeta]
 mov [eMeta],0
 @@:
 cmp [eMetaText],0
 je @f
 stdcall FreeMem,[eMetaText]
 mov [eMetaText],0
 @@:
 stdcall EReadFile,eMetaPath
 test eax,eax
 jz .missing
 mov [eMetaText],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .bad
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall JsonGet,[eMeta],eax,eKeyInherits
 test eax,eax
 jnz .inherited
 stdcall JsonGet,[eMeta],[eRoot],eKeyArguments
 test eax,eax
 jz .old
 mov eax,1
 ret
 .inherited: stdcall SetStatusA,eErrInherited
 xor eax,eax
 ret
 .old: stdcall SetStatusA,eErrOld
 jmp .fail
 .missing: stdcall SetStatusA,eErrMissing
 jmp .fail
 .bad: stdcall SetStatusA,eErrMetadata
 .fail: xor eax,eax
 ret
endp

; Returns 1 if a Mojang rule list permits Windows x64, with features false.
proc EAllowed uses ebx esi edi, ctx, object
 local rules:DWORD, count:DWORD, idx:DWORD, allowed:DWORD, rule:DWORD
 stdcall JsonGet,[ctx],[object],eKeyRules
 test eax,eax
 jz .yes
 mov [rules],eax
 stdcall JsonCount,[ctx],eax
 mov [count],eax
 mov [idx],0
 mov [allowed],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .done
 stdcall JsonAt,[ctx],[rules],eax
 mov [rule],eax
 stdcall JsonGet,[ctx],eax,eKeyFeatures
 test eax,eax
 jnz .advance
 stdcall JsonGet,[ctx],[rule],eKeyOs
 test eax,eax
 jz .match
 mov ebx,eax
 stdcall JsonGet,[ctx],ebx,eKeyName
 test eax,eax
 jz .arch
 stdcall JsonString,[ctx],eax,eBuf,128
 invoke lstrcmpA,eBuf,eWindows
 test eax,eax
 jnz .advance
 .arch:
 stdcall JsonGet,[ctx],ebx,eKeyArch
 test eax,eax
 jz .version
 stdcall JsonString,[ctx],eax,eBuf,128
 invoke lstrcmpA,eBuf,eArch
 test eax,eax
 jnz .advance
 .version:
 ; A version-regex condition cannot safely be guessed; do not match it.
 stdcall JsonGet,[ctx],ebx,eKeyVersion
 test eax,eax
 jnz .advance
 .match:
 stdcall JsonGet,[ctx],[rule],eKeyAction
 stdcall JsonString,[ctx],eax,eBuf,128
 invoke lstrcmpA,eBuf,eAllow
 test eax,eax
 setz al
 movzx eax,al
 mov [allowed],eax
 .advance: inc [idx]
 jmp .loop
 .done: mov eax,[allowed]
 ret
 .yes: mov eax,1
 ret
endp

; Check remote relative path and compose gameDir\libraries\path.
proc EArtifactPath uses esi edi
 mov esi,eRel
 xor ecx,ecx
 .check:
 mov al,[esi+ecx]
 test al,al
 jz .make
 cmp ecx,1000
 jae .bad
 cmp al,32
 jb .bad
 cmp al,':'
 je .bad
 cmp al,'\'
 je .bad
 cmp al,'/'
 jne .dot
 test ecx,ecx
 jz .bad
 jmp .next
 .dot:
 cmp al,'.'
 jne .next
 cmp byte [esi+ecx+1],'.'
 je .bad
 .next: inc ecx
 jmp .check
 .make:
 test ecx,ecx
 jz .bad
 invoke lstrcpyW,ePath,gameDir
 invoke lstrcatW,ePath,eLibraries
 invoke lstrlenW,ePath
 lea edi,[ePath+eax*2]
 invoke MultiByteToWideChar,65001,8,eRel,-1,edi,1024
 test eax,eax
 jz .bad
 .slashes:
 mov ax,[edi]
 test ax,ax
 jz .okay
 cmp ax,'/'
 jne @f
 mov word [edi],'\'
 @@: add edi,2
 jmp .slashes
 .okay: mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

proc EGetArtifact uses ebx, ctx, artifact
 test [artifact],-1
 jz .bad
 stdcall JsonGet,[ctx],[artifact],eKeyPath
 stdcall JsonString,[ctx],eax,eRel,4096
 cmp eax,4096
 jae .bad
 stdcall EArtifactPath
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],[artifact],eKeyUrl
 stdcall JsonString,[ctx],eax,eUrl,4096
 cmp eax,4096
 jae .bad
 stdcall JsonGet,[ctx],[artifact],eKeySha
 stdcall JsonString,[ctx],eax,eHash,64
 cmp eax,40
 jne .bad
 stdcall McDownload,eUrl,ePath,eHash,1
 ret
 .bad: xor eax,eax
 ret
endp

proc EExtractNative uses ebx esi edi
 invoke GetWindowsDirectoryW,eTarPath,990
 invoke lstrcatW,eTarPath,eTarNative
 invoke GetFileAttributesW,eTarPath
 cmp eax,-1
 jne .found
 invoke GetSystemDirectoryW,eTarPath,990
 invoke lstrcatW,eTarPath,eTarName
 invoke GetFileAttributesW,eTarPath
 cmp eax,-1
 je .bad
 .found:
 stdcall EnsureDirectory,eNativePath
 invoke lstrcpyW,eCmd,eQuoteW
 invoke lstrcatW,eCmd,eTarPath
 invoke lstrcatW,eCmd,eTarSwitch
 invoke lstrcatW,eCmd,ePath
 invoke lstrcatW,eCmd,eTarDest
 invoke lstrcatW,eCmd,eNativePath
 invoke lstrcatW,eCmd,eTarExclude
 mov edi,eInfo
 xor eax,eax
 mov ecx,sizeof.STARTUPINFO/4
 rep stosd
 mov [eInfo.cb],sizeof.STARTUPINFO
 invoke CreateProcessW,eTarPath,eCmd,0,0,0,08000000h,0,gameDir,eInfo,eProcess
 test eax,eax
 jz .bad
 invoke CloseHandle,[eProcess.hThread]
 invoke WaitForSingleObject,[eProcess.hProcess],-1
 invoke GetExitCodeProcess,[eProcess.hProcess],eResult
 invoke CloseHandle,[eProcess.hProcess]
 cmp [eResult],0
 jne .bad
 mov eax,1
 ret
 .bad: stdcall SetStatusA,eErrTar
 xor eax,eax
 ret
endp

; Install/update metadata with SHA1 links, using mirror then official sources.
proc EFetchMetadata uses ebx esi edi
 local textptr:DWORD, ctx:DWORD, root:DWORD, list:DWORD, idx:DWORD, count:DWORD, item:DWORD, result:DWORD
 mov [result],0
 mov [textptr],0
 mov [ctx],0
 stdcall SetStatusA,eStatusManifest
 stdcall McManifestRequest,selectedVersion
 test eax,eax
 jz .done
 mov [textptr],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [ctx],eax
 stdcall JsonRoot,eax
 mov [root],eax
 stdcall JsonGet,[ctx],eax,eKeyVersions
 mov [list],eax
 stdcall JsonCount,[ctx],eax
 mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .unknown
 cmp [engineCancel],0
 jne .done
 stdcall JsonAt,[ctx],[list],eax
 mov [item],eax
 stdcall JsonGet,[ctx],eax,eKeyId
 stdcall JsonString,[ctx],eax,eValue,256
 invoke lstrcmpA,eValue,selectedVersion
 test eax,eax
 jz .found
 inc [idx]
 jmp .loop
 .found:
 stdcall JsonGet,[ctx],[item],eKeyUrl
 stdcall JsonString,[ctx],eax,eUrl,4096
 cmp eax,4096
 jae .done
 stdcall JsonGet,[ctx],[item],eKeySha
 stdcall JsonString,[ctx],eax,eHash,64
 cmp eax,40
 jne .done
 stdcall McDownload,eUrl,eMetaPath,eHash,0
 mov [result],eax
 jmp .done
 .unknown: stdcall SetStatusA,eErrUnknown
 .done:
 cmp [ctx],0
 je @f
 stdcall JsonFree,[ctx]
 @@:
 cmp [textptr],0
 je @f
 stdcall FreeMem,[textptr]
 @@:
 mov eax,[result]
 ret
endp

proc EInstallLibraries uses ebx esi edi
 local list:DWORD, count:DWORD, idx:DWORD, lib:DWORD, downloads:DWORD, native:DWORD
 stdcall JsonGet,[eMeta],[eRoot],eKeyLibraries
 test eax,eax
 jz .bad
 mov [list],eax
 stdcall JsonCount,[eMeta],eax
 mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .okay
 cmp [engineCancel],0
 jne .bad
 stdcall JsonAt,[eMeta],[list],eax
 mov [lib],eax
 stdcall EAllowed,[eMeta],eax
 test eax,eax
 jz .next
 stdcall JsonGet,[eMeta],[lib],eKeyDownloads
 mov [downloads],eax
 stdcall JsonGet,[eMeta],eax,eKeyArtifact
 test eax,eax
 jz .native
 stdcall EGetArtifact,[eMeta],eax
 test eax,eax
 jz .bad
 .native:
 stdcall JsonGet,[eMeta],[lib],eKeyNatives
 test eax,eax
 jz .next
 stdcall JsonGet,[eMeta],eax,eKeyWindows
 test eax,eax
 jz .next
 stdcall JsonString,[eMeta],eax,eValue,512
 ; Official legacy LWJGL classifiers may use ${arch}; replace with 64.
 mov esi,eValue
 mov edi,eBuf
 .copy:
 mov al,[esi]
 test al,al
 jz .copied
 cmp dword [esi],'${ar'
 jne .char
 cmp dword [esi+4],'ch}'
 jne .char
 mov word [edi],'64'
 add edi,2
 add esi,7
 jmp .copy
 .char: stosb
 inc esi
 jmp .copy
 .copied: mov byte [edi],0
 stdcall JsonGet,[eMeta],[downloads],eKeyClassifiers
 stdcall JsonGet,[eMeta],eax,eBuf
 test eax,eax
 jz .bad
 stdcall EGetArtifact,[eMeta],eax
 test eax,eax
 jz .bad
 stdcall EExtractNative
 test eax,eax
 jz .bad
 .next: inc [idx]
 jmp .loop
 .okay: mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

proc EInstallAssets uses ebx esi edi
 local obj:DWORD, idx:DWORD, count:DWORD, val:DWORD, threads:DWORD
 stdcall JsonGet,[eMeta],[eRoot],eKeyAssets
 test eax,eax
 jz .bad
 mov ebx,eax
 stdcall JsonGet,[eMeta],ebx,eKeyId
 stdcall JsonString,[eMeta],eax,eIndexId,128
 cmp eax,120
 jae .bad
 test eax,eax
 jz .bad
 mov esi,eIndexId
 .safe:
 lodsb
 test al,al
 jz .idokay
 cmp al,'/'
 je .bad
 cmp al,'\'
 je .bad
 cmp al,':'
 je .bad
 cmp al,32
 jb .bad
 jmp .safe
 .idokay:
 invoke lstrcpyW,ePath,gameDir
 invoke lstrcatW,ePath,eIndexes
 invoke MultiByteToWideChar,65001,0,eIndexId,-1,eWide,256
 invoke lstrcatW,ePath,eWide
 invoke lstrcatW,ePath,eJsonExt
 stdcall JsonGet,[eMeta],ebx,eKeyUrl
 stdcall JsonString,[eMeta],eax,eUrl,4096
 stdcall JsonGet,[eMeta],ebx,eKeySha
 stdcall JsonString,[eMeta],eax,eHash,64
 cmp eax,40
 jne .bad
 stdcall McDownload,eUrl,ePath,eHash,0
 test eax,eax
 jz .bad
 stdcall EReadFile,ePath
 test eax,eax
 jz .bad
 mov [eIndexText],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .bad
 mov [eIndex],eax
 stdcall JsonRoot,eax
 stdcall JsonGet,[eIndex],eax,eKeyObjects
 test eax,eax
 jz .bad
 mov [obj],eax
 stdcall JsonCount,[eIndex],eax
 cmp eax,65536
 ja .bad
 mov [count],eax
 mov [eAssetCount],eax
 mov [eAssetJobCount],eax
 mov [idx],0
 stdcall JsonValueAt,[eIndex],[obj],0
 mov [val],eax
 .jobs:
 mov eax,[idx]
 cmp eax,[count]
 jae .spawn
 mov edx,[val]
 test edx,edx
 jz .bad
 mov [eAssetJobs+eax*4],edx
 stdcall JsonNext,[eIndex],edx
 stdcall JsonNext,[eIndex],eax
 mov [val],eax
 inc [idx]
 jmp .jobs
 .spawn:
 mov [eAssetNext],0
 mov [eAssetDone],0
 mov [eAssetError],0
 mov edi,eAssetLocks
 xor eax,eax
 mov ecx,256
 rep stosd
 mov [threads],0
 .spawnloop:
 mov eax,[threads]
 cmp eax,8
 jae .join
 invoke CreateThread,0,0,EAssetWorker,eax,0,0
 test eax,eax
 jz .join
 mov ecx,[threads]
 mov [eAssetThreads+ecx*4],eax
 inc [threads]
 jmp .spawnloop
 .join:
 cmp [threads],0
 je .bad
 mov [idx],0
 .joinloop:
 mov eax,[idx]
 cmp eax,[threads]
 jae .joined
 mov ebx,[eAssetThreads+eax*4]
 invoke WaitForSingleObject,ebx,-1
 invoke CloseHandle,ebx
 inc [idx]
 jmp .joinloop
 .joined:
 cmp [engineCancel],0
 jne .bad
 cmp [eAssetError],0
 jne .bad
 mov eax,[eAssetDone]
 cmp eax,[count]
 jne .bad
if defined OFFSET_I18N
 cinvoke wsprintfA,eMsg,[trEFmtProgress],eax,[count]
else
 cinvoke wsprintfA,eMsg,eFmtProgress,eax,[count]
end if
 stdcall SetStatusA,eMsg
 mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

; Bounded concurrent downloads. Each worker owns all transient HTTP/path data.
; Prefix locks serialize duplicate hashes that would share one .part filename.
proc EAssetWorker uses ebx esi edi, worker
 locals
  assetHash rb 64
  assetUrl rb 512
  assetPath rw 2048
  shortName rb 4
  wideName rw 64
  progress rb 512
  tries dd ?
 endl
 .work:
 cmp [engineCancel],0
 jne .done
 cmp [eAssetError],0
 jne .done
 mov eax,1
 lock xadd [eAssetNext],eax
 cmp eax,[eAssetJobCount]
 jae .done
 mov eax,[eAssetJobs+eax*4]
 stdcall JsonGet,[eIndex],eax,eKeyHash
 stdcall JsonString,[eIndex],eax,addr assetHash,64
 cmp eax,40
 jne .bad
 lea esi,[assetHash]
 mov ecx,40
 .hex:
 lodsb
 cmp al,'0'
 jb .bad
 cmp al,'9'
 jbe .hexnext
 cmp al,'a'
 jb .bad
 cmp al,'f'
 ja .bad
 .hexnext: loop .hex
 movzx ebx,byte [assetHash]
 cmp bl,'9'
 jbe .firstdigit
 sub ebx,'a'-10
 jmp .firsthex
 .firstdigit: sub ebx,'0'
 .firsthex:
 shl ebx,4
 movzx eax,byte [assetHash+1]
 cmp al,'9'
 jbe .seconddigit
 sub eax,'a'-10
 jmp .secondhex
 .seconddigit: sub eax,'0'
 .secondhex:
 or ebx,eax
 .lock:
 cmp [engineCancel],0
 jne .done
 cmp [eAssetError],0
 jne .done
 mov eax,1
 xchg eax,[eAssetLocks+ebx*4]
 test eax,eax
 jz .locked
 invoke Sleep,5
 jmp .lock
 .locked:
 invoke lstrcpyA,addr assetUrl,eAssetBase
 mov ax,word [assetHash]
 mov word [shortName],ax
 mov byte [shortName+2],0
 invoke lstrcatA,addr assetUrl,addr shortName
 mov byte [shortName],'/'
 mov byte [shortName+1],0
 invoke lstrcatA,addr assetUrl,addr shortName
 invoke lstrcatA,addr assetUrl,addr assetHash
 invoke lstrcpyW,addr assetPath,gameDir
 invoke lstrcatW,addr assetPath,eObjects
 mov ax,word [assetHash]
 mov word [shortName],ax
 mov byte [shortName+2],0
 invoke MultiByteToWideChar,65001,0,addr shortName,-1,addr wideName,64
 invoke lstrcatW,addr assetPath,addr wideName
 invoke lstrcatW,addr assetPath,eSlash
 invoke MultiByteToWideChar,65001,0,addr assetHash,-1,addr wideName,64
 invoke lstrcatW,addr assetPath,addr wideName
 mov [tries],3
 .download:
 cmp [engineCancel],0
 jne .unlockdone
 stdcall McDownload,addr assetUrl,addr assetPath,addr assetHash,2
 test eax,eax
 jnz .downloaded
 dec [tries]
 jz .unlockbad
 invoke Sleep,250
 jmp .download
 .downloaded:
 mov dword [eAssetLocks+ebx*4],0
 mov eax,1
 lock xadd [eAssetDone],eax
 inc eax
 cmp [worker],0
 jne .work
 test eax,15
 jnz .work
if defined OFFSET_I18N
 cinvoke wsprintfA,addr progress,[trEFmtProgress],eax,[eAssetCount]
else
 cinvoke wsprintfA,addr progress,eFmtProgress,eax,[eAssetCount]
end if
 stdcall SetStatusA,addr progress
 jmp .work
 .unlockbad:
 mov dword [eAssetLocks+ebx*4],0
 .bad:
 mov [eAssetError],1
 .done:
 xor eax,eax
 ret
 .unlockdone:
 mov dword [eAssetLocks+ebx*4],0
 jmp .done
endp

proc ECleanup uses edi
 cmp [eIndex],0
 je @f
 stdcall JsonFree,[eIndex]
 mov [eIndex],0
 @@:
 cmp [eIndexText],0
 je @f
 stdcall FreeMem,[eIndexText]
 mov [eIndexText],0
 @@:
 cmp [eMeta],0
 je @f
 stdcall JsonFree,[eMeta]
 mov [eMeta],0
 @@:
 cmp [eMetaText],0
 je @f
 stdcall FreeMem,[eMetaText]
 mov [eMetaText],0
 @@:
 ; Session-bearing temporary arguments are cleared on every exit path.
 mov edi,eCommand
 xor eax,eax
 mov ecx,32768/4
 rep stosd
 mov edi,eCmd
 mov ecx,65536/4
 rep stosd
 mov edi,eExpanded
 mov ecx,1200000/4
 rep stosd
 ret
endp

proc EInstallCore uses ebx esi edi
 stdcall EValidateVersion
 test eax,eax
 jz .version
 stdcall EBuildPaths
 stdcall EnsureDirectory,gameDir
 invoke GetFileAttributesW,eMetaPath
 cmp eax,-1
 je .fetch
 stdcall ELoadMetadata
 test eax,eax
 jz .fetch
 stdcall JsonGet,[eMeta],[eRoot],eKeyNormalized
 stdcall JsonBool,[eMeta],eax
 cmp eax,1
 je .metadataReady
.fetch:
 stdcall EFetchMetadata
 test eax,eax
 jz .network
 stdcall ELoadMetadata
 test eax,eax
 jz .bad
.metadataReady:
 stdcall SetStatusA,eStatusInstall
 stdcall JsonGet,[eMeta],[eRoot],eKeyDownloads
 stdcall JsonGet,[eMeta],eax,eKeyClient
 test eax,eax
 jz .metadata
 mov ebx,eax
 stdcall JsonGet,[eMeta],ebx,eKeyUrl
 stdcall JsonString,[eMeta],eax,eUrl,4096
 stdcall JsonGet,[eMeta],ebx,eKeySha
 stdcall JsonString,[eMeta],eax,eHash,64
 cmp eax,40
 jne .metadata
 stdcall McDownload,eUrl,eClientPath,eHash,0
 test eax,eax
 jz .network
 stdcall EInstallLibraries
 test eax,eax
 jz .network
 stdcall EInstallAssets
 test eax,eax
 jz .network
 mov eax,1
 ret
 .version: stdcall SetStatusA,eErrVersion
 jmp .bad
 .metadata: stdcall SetStatusA,eErrMetadata
 jmp .bad
 .network:
 cmp [engineCancel],0
 jne .cancel
 stdcall SetStatusA,eErrNetwork
 jmp .bad
 .cancel: stdcall SetStatusA,eErrCancel
 .bad: xor eax,eax
 ret
endp

proc EngineInstall uses ebx esi edi, param
 stdcall EInstallCore
 mov [eResult],eax
 test eax,eax
 jz @f
 stdcall SetStatusA,eStatusInstalled
 @@:
 stdcall ECleanup
 invoke PostMessageW,[hWnd],WM_APP+2,0,0
 mov eax,[eResult]
 ret
endp

; Generate the Java-compatible RFC4122 v3 offline UUID.
proc EIdentity uses ebx esi edi
 cmp [onlineMode],0
 jne .online
 mov esi,playerName
 xor ecx,ecx
 .validate:
 mov al,[esi+ecx]
 test al,al
 jz .length
 cmp ecx,16
 jae .bad
 cmp al,'_'
 je .next
 cmp al,'0'
 jb .bad
 cmp al,'9'
 jbe .next
 cmp al,'A'
 jb .bad
 cmp al,'Z'
 jbe .next
 cmp al,'a'
 jb .bad
 cmp al,'z'
 ja .bad
 .next: inc ecx
 jmp .validate
 .length:
 cmp ecx,3
 jb .bad
 invoke lstrcpyA,eName,playerName
 invoke lstrcpyA,eIdentity,eOfflinePrefix
 invoke lstrcatA,eIdentity,playerName
 invoke CryptAcquireContextW,eProvider,0,0,1,0F0000000h
 test eax,eax
 jz .bad
 invoke CryptCreateHash,[eProvider],08003h,0,0,eHashHandle
 test eax,eax
 jz .release
 invoke lstrlenA,eIdentity
 invoke CryptHashData,[eHashHandle],eIdentity,eax,0
 test eax,eax
 jz .destroy
 mov [eDigestLen],16
 invoke CryptGetHashParam,[eHashHandle],2,eDigest,eDigestLen,0
 mov ebx,eax
 invoke CryptDestroyHash,[eHashHandle]
 invoke CryptReleaseContext,[eProvider],0
 test ebx,ebx
 jz .bad
 and byte [eDigest+6],0Fh
 or byte [eDigest+6],30h
 and byte [eDigest+8],3Fh
 or byte [eDigest+8],80h
 mov esi,eDigest
 mov edi,eUuid
 mov ecx,16
 .encode:
 lodsb
 mov dl,al
 shr al,4
 movzx eax,al
 mov al,[eHex+eax]
 stosb
 movzx eax,dl
 and eax,15
 mov al,[eHex+eax]
 stosb
 loop .encode
 mov byte [edi],0
 mov dword [ePlaceholders+6*8+4],eOfflineToken
 mov dword [ePlaceholders+7*8+4],eOfflineType
 mov eax,1
 ret
 .destroy: invoke CryptDestroyHash,[eHashHandle]
 .release: invoke CryptReleaseContext,[eProvider],0
 .bad: stdcall SetStatusA,eErrName
 xor eax,eax
 ret
 .online:
 cmp [authReady],1
 jne .authbad
 cmp byte [authName],0
 je .authbad
 cmp byte [authUuid],0
 je .authbad
 cmp byte [authToken],0
 je .authbad
 invoke lstrcpyA,eName,authName
 invoke lstrcpyA,eUuid,authUuid
 mov dword [ePlaceholders+6*8+4],authToken
 mov dword [ePlaceholders+7*8+4],eUserType
 mov eax,1
 ret
 .authbad: stdcall SetStatusA,eErrAuth
 xor eax,eax
 ret
endp

; Expand only recognized metadata placeholders, never execute metadata as shell.
proc EExpand uses ebx esi edi, source
 local key:DWORD, val:DWORD, left:DWORD
 mov esi,[source]
 mov edi,eExpanded
 mov [left],1200000-1
 .next:
 lodsb
 test al,al
 jz .done
 cmp al,'$'
 jne .char
 cmp byte [esi],'{'
 jne .char
 inc esi
 mov ebx,ePlaceholders
 .table:
 cmp dword [ebx],0
 je .bad
 mov edx,[ebx]
 push esi
 .compare:
 mov al,[edx]
 test al,al
 jz .matched
 cmp al,[esi]
 jne .nomatch
 inc esi
 inc edx
 jmp .compare
 .nomatch:
 pop esi
 add ebx,8
 jmp .table
 .matched:
 add esp,4
 mov edx,[ebx+4]
 .value:
 mov al,[edx]
 test al,al
 jz .next
 inc edx
 cmp [left],0
 je .bad
 stosb
 dec [left]
 jmp .value
 .char:
 cmp [left],0
 je .bad
 stosb
 dec [left]
 jmp .next
 .done:
 mov byte [edi],0
 mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

; Quoted UTF8 Java argfile token. Backslashes are escaped as required by JDK.
proc EAppendJava uses esi edi, source
 mov esi,[source]
 mov edi,eArgs
 add edi,[eJLen]
 cmp edi,eArgs+2097140
 jae .bad
 mov al,'"'
 stosb
 .loop:
 lodsb
 test al,al
 jz .done
 cmp edi,eArgs+2097140
 jae .bad
 cmp al,'\'
 je .escape
 cmp al,'"'
 je .escape
 cmp al,13
 je .bad
 cmp al,10
 je .bad
 stosb
 jmp .loop
 .escape: mov byte [edi],'\'
 inc edi
 stosb
 jmp .loop
 .done:
 mov al,'"'
 stosb
 mov al,10
 stosb
 mov byte [edi],0
 sub edi,eArgs
 mov [eJLen],edi
 mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

; Windows CreateProcess argument quoting, including trailing backslashes.
proc EAppendCommand uses ebx esi edi, source
 local slashes:DWORD
 mov esi,[source]
 mov edi,eCommand
 add edi,[eCLen]
 cmp edi,eCommand+32760
 jae .bad
 cmp [eCLen],0
 je @f
 mov al,' '
 stosb
 @@:
 mov al,'"'
 stosb
 mov [slashes],0
 .loop:
 lodsb
 cmp al,'\'
 jne .other
 inc [slashes]
 jmp .loop
 .other:
 test al,al
 jz .finish
 cmp al,'"'
 je .quote
 cmp al,13
 je .bad
 cmp al,10
 je .bad
 mov ebx,eax
 mov ecx,[slashes]
 cmp ecx,10000
 ja .bad
 add ecx,edi
 cmp ecx,eCommand+32760
 jae .bad
 mov ecx,[slashes]
 mov al,'\'
 rep stosb
 mov eax,ebx
 stosb
 mov [slashes],0
 cmp edi,eCommand+32760
 jae .bad
 jmp .loop
 .quote:
 mov ecx,[slashes]
 shl ecx,1
 inc ecx
 add ecx,edi
 cmp ecx,eCommand+32760
 jae .bad
 mov ecx,[slashes]
 shl ecx,1
 inc ecx
 mov al,'\'
 rep stosb
 mov al,'"'
 stosb
 mov [slashes],0
 jmp .loop
 .finish:
 mov ecx,[slashes]
 shl ecx,1
 add ecx,edi
 cmp ecx,eCommand+32760
 jae .bad
 mov ecx,[slashes]
 shl ecx,1
 mov al,'\'
 rep stosb
 mov al,'"'
 stosb
 mov byte [edi],0
 sub edi,eCommand
 mov [eCLen],edi
 mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

; Compare an ASCII path prefix, preserving Windows case/separator semantics.
; Return the first byte after a complete prefix or 0. Callers include directory
; boundaries in literal prefixes, so similarly named paths cannot match.
proc EModulePrefix uses esi edi,source,prefix
 mov esi,[source]
 mov edi,[prefix]
.loop:
 mov al,[edi]
 test al,al
 jz .match
 mov dl,[esi]
 cmp al,92
 jne .sourceSlash
 mov al,'/'
.sourceSlash:
 cmp dl,92
 jne .lowerPrefix
 mov dl,'/'
.lowerPrefix:
 cmp al,'A'
 jb .lowerSource
 cmp al,'Z'
 ja .lowerSource
 add al,32
.lowerSource:
 cmp dl,'A'
 jb .compare
 cmp dl,'Z'
 ja .compare
 add dl,32
.compare:
 cmp al,dl
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

; Match one module-path file against a relative Maven artifact. The canonical
; library placeholder expands to the relative libraries directory, retaining
; the launcher's Unicode-safe argument-file behavior.
proc EModuleItemMatch uses ebx esi edi,item,artifact
 jmp .moduleCode
.libraryPlaceholder db '${library_directory}',0
.gamePlaceholder db '${game_directory}',0
.libraryLiteral db 'libraries/',0
.moduleCode:
 stdcall EModulePrefix,[item],addr .libraryPlaceholder
 test eax,eax
 jnz .trim
 stdcall EModulePrefix,[item],addr .gamePlaceholder
 test eax,eax
 jz .literal
 mov esi,eax
 cmp byte [esi],'/'
 je .gameSlash
 cmp byte [esi],92
 jne .no
.gameSlash:
 inc esi
 jmp .library
.literal:
 mov esi,[item]
 cmp byte [esi],'.'
 jne .library
 cmp byte [esi+1],'/'
 je .relative
 cmp byte [esi+1],92
 jne .library
.relative:
 add esi,2
.library:
 stdcall EModulePrefix,esi,addr .libraryLiteral
 test eax,eax
 jz .no
.trim:
 mov esi,eax
.trimSlashes:
 cmp byte [esi],'/'
 je .oneSlash
 cmp byte [esi],92
 jne .artifact
.oneSlash:
 inc esi
 jmp .trimSlashes
.artifact:
 stdcall EModulePrefix,esi,[artifact]
 test eax,eax
 jz .no
 cmp byte [eax],0
 jne .no
 mov eax,1
 ret
.no:
 xor eax,eax
 ret
endp

; A path-list may use a literal Windows semicolon or the metadata separator.
; Compare complete entries only, never substrings of a longer artifact path.
proc EModulePathMatch uses ebx esi edi,paths,artifact
 locals
  item rb 4096
  length dd ?
  overflow dd ?
  tail dd ?
  last dd ?
 endl
 jmp .moduleCode
.separator db '${classpath_separator}',0
.moduleCode:
 mov esi,[paths]
.newItem:
 mov [length],0
 mov [overflow],0
 mov [last],0
.read:
 mov al,[esi]
 test al,al
 jz .lastItem
 cmp al,';'
 je .semicolon
 cmp al,'$'
 jne .copy
 stdcall EModulePrefix,esi,addr .separator
 test eax,eax
 jz .copy
 mov [tail],eax
 jmp .itemEnd
.semicolon:
 lea eax,[esi+1]
 mov [tail],eax
 jmp .itemEnd
.lastItem:
 mov [tail],esi
 mov [last],1
.itemEnd:
 mov ecx,[length]
 mov byte [item+ecx],0
 cmp [overflow],0
 jne .advance
 stdcall EModuleItemMatch,addr item,[artifact]
 test eax,eax
 jnz .yes
.advance:
 cmp [last],1
 je .no
 mov esi,[tail]
 jmp .newItem
.copy:
 mov ecx,[length]
 cmp ecx,4095
 jae .tooLong
 mov al,[esi]
 mov byte [item+ecx],al
 inc [length]
 jmp .nextByte
.tooLong:
 mov [overflow],1
.nextByte:
 inc esi
 jmp .read
.yes:
 mov eax,1
 ret
.no:
 xor eax,eax
 ret
endp

; State holds {pending module-path value, match in last effective option}.
; JVM options are case-sensitive. A later module-path replaces an earlier one.
proc EModuleToken uses ebx esi edi,token,artifact,state
 local textLength:DWORD
 jmp .moduleCode
.shortOption db '-p',0
.longOption db '--module-path',0
.inlineOption db '--module-path=',0
.moduleCode:
 stdcall JsonString,[eMeta],[token],eValue,65536
 cmp eax,65536
 jae .invalid
 mov [textLength],eax
 invoke lstrlenA,eValue
 cmp eax,[textLength]
 jne .invalid
 mov ebx,[state]
 cmp dword [ebx],1
 jne .option
 mov dword [ebx],0
 stdcall EModulePathMatch,eValue,[artifact]
 mov ebx,[state]
 mov [ebx+4],eax
 ret
.option:
 invoke lstrcmpA,eValue,addr .shortOption
 test eax,eax
 jz .pending
 invoke lstrcmpA,eValue,addr .longOption
 test eax,eax
 jz .pending
 ; Do not use the case-folding helper for option recognition.
 lea esi,[.inlineOption]
 mov edi,eValue
.inline:
 lodsb
 test al,al
 jz .inlineValue
 cmp al,[edi]
 jne .done
 inc edi
 jmp .inline
.inlineValue:
 stdcall EModulePathMatch,edi,[artifact]
 mov ebx,[state]
 mov [ebx+4],eax
 ret
.pending:
 mov ebx,[state]
 mov dword [ebx],1
 mov dword [ebx+4],0
.done:
 ret
.invalid:
 mov ebx,[state]
 mov dword [ebx],0
 mov dword [ebx+4],0
 ret
endp

; Libraries explicitly placed on the effective JVM module path must not also
; appear on our relative classpath: BootstrapLauncher compares module locations
; as absolute Paths and otherwise sees the same JAR a second time as relative.
; Walk rule/value arrays exactly as EArgumentArray does, preserving other libs.
proc EModuleOnly uses ebx esi edi,artifact
 locals
  list dd ?
  count dd ?
  index dd ?
  token dd ?
  value dd ?
  subCount dd ?
  subIndex dd ?
  state rd 2
 endl
 mov dword [state],0
 mov dword [state+4],0
 stdcall JsonGet,[eMeta],[eRoot],eKeyArguments
 stdcall JsonGet,[eMeta],eax,eKeyJvm
 mov [list],eax
 stdcall JsonCount,[eMeta],eax
 mov [count],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .done
 stdcall JsonAt,[eMeta],[list],eax
 mov [token],eax
 stdcall JsonGet,[eMeta],eax,eKeyValue
 test eax,eax
 jz .string
 mov [value],eax
 stdcall EAllowed,[eMeta],[token]
 test eax,eax
 jz .next
 stdcall JsonCount,[eMeta],[value]
 test eax,eax
 jz .single
 mov [subCount],eax
 mov [subIndex],0
.subLoop:
 mov eax,[subIndex]
 cmp eax,[subCount]
 jae .next
 stdcall JsonAt,[eMeta],[value],eax
 stdcall EModuleToken,eax,[artifact],addr state
 inc [subIndex]
 jmp .subLoop
.single:
 mov eax,[value]
 jmp .emit
.string:
 mov eax,[token]
.emit:
 stdcall EModuleToken,eax,[artifact],addr state
.next:
 inc [index]
 jmp .loop
.done:
 mov eax,[state+4]
 ret
endp

proc EBuildClasspath uses ebx esi edi
 local list:DWORD, count:DWORD, idx:DWORD, lib:DWORD
 mov byte [eClasspath],0
 mov [eOutLen],0
 stdcall JsonGet,[eMeta],[eRoot],eKeyLibraries
 mov [list],eax
 stdcall JsonCount,[eMeta],eax
 mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .client
 stdcall JsonAt,[eMeta],[list],eax
 mov [lib],eax
 stdcall EAllowed,[eMeta],eax
 test eax,eax
 jz .next
 stdcall JsonGet,[eMeta],[lib],eKeyDownloads
 stdcall JsonGet,[eMeta],eax,eKeyArtifact
 test eax,eax
 jz .next
 stdcall JsonGet,[eMeta],eax,eKeyPath
 stdcall JsonString,[eMeta],eax,eRel,4096
 stdcall EArtifactPath
 test eax,eax
 jz .bad
 invoke GetFileAttributesW,ePath
 cmp eax,-1
 je .missing
 stdcall EModuleOnly,eRel
 test eax,eax
 jnz .next
 stdcall EAppendClasspath,ePath
 test eax,eax
 jz .bad
 .next: inc [idx]
 jmp .loop
 .client:
 invoke GetFileAttributesW,eClientPath
 cmp eax,-1
 je .missing
 stdcall EAppendClasspath,eClientPath
 ret
 .missing: stdcall SetStatusA,eErrMissing
 .bad: xor eax,eax
 ret
endp

; Encode a cache-root path relative to the actual Java working directory.
proc ERootRelative uses ebx esi edi,path,destination,capacity
 invoke lstrlenW,gameDir
 mov esi,[path]
 lea esi,[esi+eax*2+2]
 mov edi,[destination]
 mov ebx,[capacity]
 cmp [eRuntimeIsolated],0
 je .convert
 cmp ebx,10
 jb .bad
 invoke lstrcpyA,edi,eRuntimeParent
 add edi,9
 sub ebx,9
.convert:
 invoke WideCharToMultiByte,65001,0,esi,-1,edi,ebx,0,0
 test eax,eax
 jz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc EAppendClasspath uses esi edi, path
 ; Java @argfile parsing uses the Windows ANSI codepage even on JDK21.
 ; Keep classpath entries relative to the Unicode CreateProcess working dir.
 stdcall ERootRelative,[path],eScratch,32768
 test eax,eax
 jz .bad
 invoke lstrlenA,eScratch
 inc eax
 mov edx,eax
 add edx,[eOutLen]
 cmp edx,1048500
 jae .bad
 mov edi,eClasspath
 add edi,[eOutLen]
 cmp [eOutLen],0
 je @f
 mov al,';'
 stosb
 @@:
 mov esi,eScratch
 .copy: lodsb
 stosb
 test al,al
 jnz .copy
 dec edi
 sub edi,eClasspath
 mov [eOutLen],edi
 mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

; Metadata argument arrays may contain strings or rule/value objects.
proc EArgumentArray uses ebx esi edi, array, phase
 local idx:DWORD, count:DWORD, tok:DWORD, value:DWORD, subidx:DWORD, subcount:DWORD
 stdcall JsonCount,[eMeta],[array]
 mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .okay
 stdcall JsonAt,[eMeta],[array],eax
 mov [tok],eax
 stdcall JsonGet,[eMeta],eax,eKeyValue
 test eax,eax
 jz .string
 mov [value],eax
 stdcall EAllowed,[eMeta],[tok]
 test eax,eax
 jz .next
 stdcall JsonCount,[eMeta],[value]
 test eax,eax
 jz .single
 mov [subcount],eax
 mov [subidx],0
 .subloop:
 mov eax,[subidx]
 cmp eax,[subcount]
 jae .next
 stdcall JsonAt,[eMeta],[value],eax
 stdcall EEmitToken,eax,[phase]
 test eax,eax
 jz .bad
 inc [subidx]
 jmp .subloop
 .single: mov eax,[value]
 jmp .emit
 .string: mov eax,[tok]
 .emit:
 stdcall EEmitToken,eax,[phase]
 test eax,eax
 jz .bad
 .next: inc [idx]
 jmp .loop
 .okay: mov eax,1
 ret
 .bad: xor eax,eax
 ret
endp

proc EEmitToken, tok, phase
 stdcall JsonString,[eMeta],[tok],eValue,65536
 cmp eax,65536
 jae .bad
 cmp [phase],0
 jne .expand
 stdcall EContainsToken,eValue
 test eax,eax
 jnz .bad
 .expand:
 stdcall EExpand,eValue
 test eax,eax
 jz .bad
 cmp [phase],0
 jne .game
 stdcall EAppendJava,eExpanded
 ret
 .game: stdcall EAppendCommand,eExpanded
 ret
 .bad: xor eax,eax
 ret
endp

proc EContainsToken uses esi edi, source
 mov esi,[source]
 .next:
 cmp byte [esi],0
 je .no
 mov edi,eSecretPh
 mov edx,esi
 .compare:
 mov al,[edi]
 test al,al
 jz .yes
 cmp al,[edx]
 jne .advance
 inc edi
 inc edx
 jmp .compare
 .advance:
 inc esi
 jmp .next
 .yes: mov eax,1
 ret
 .no: xor eax,eax
 ret
endp

proc EPrepareLaunch uses ebx esi edi
 cmp [eRuntimeIsolated],0
 jne .runtimeReady
 invoke lstrcpyW,eRuntimeDirW,gameDir
.runtimeReady:
 invoke lstrcpyA,eDefaultType,eReleaseType
 stdcall JsonGet,[eMeta],[eRoot],eKeyType
 stdcall JsonString,[eMeta],eax,eBuf,128
 test eax,eax
 jz .identity
 cmp eax,32
 jae .identity
 invoke lstrcpyA,eDefaultType,eBuf
.identity:
 stdcall EIdentity
 test eax,eax
 jz .bad
 cmp word [javaPath],0
 je .java
 invoke GetFileAttributesW,javaPath
 cmp eax,-1
 je .java
 stdcall MemoryDetect
 stdcall MemoryValid,[memoryMb]
 test eax,eax
 jz .args
 stdcall EBuildClasspath
 test eax,eax
 jz .bad
 stdcall EnsureDirectory,eNativePath
 mov word [eGameUtf],'.'
 stdcall ERootRelative,eNativePath,eNativeUtf,8192
 test eax,eax
 jz .args
 invoke lstrcpyW,ePath,gameDir
 invoke lstrcatW,ePath,eAssets
 stdcall ERootRelative,ePath,eAssetsUtf,8192
 test eax,eax
 jz .args
 invoke lstrcpyW,ePath,gameDir
 invoke lstrcatW,ePath,eLibraries
 stdcall ERootRelative,ePath,eLibUtf,8192
 test eax,eax
 jz .args
 stdcall JsonGet,[eMeta],[eRoot],eKeyAssets
 stdcall JsonGet,[eMeta],eax,eKeyId
 stdcall JsonString,[eMeta],eax,eIndexId,128
 mov [eJLen],0
 mov [eCLen],0
 mov byte [eArgs],0
 mov byte [eCommand],0
 stdcall JsonGet,[eMeta],[eRoot],eKeyArguments
 mov ebx,eax
 stdcall JsonGet,[eMeta],ebx,eKeyJvm
 test eax,eax
 jz .args
 stdcall EArgumentArray,eax,0
 test eax,eax
 jz .args
 if defined OFFSET_LAUNCH_LOG
 stdcall EAppendJava,eLogStdout
 test eax,eax
 jz .args
 stdcall EAppendJava,eLogStderr
 test eax,eax
 jz .args
 stdcall EAppendJava,eLogLegacyStdout
 test eax,eax
 jz .args
 stdcall EAppendJava,eLogLegacyStderr
 test eax,eax
 jz .args
 end if
 cinvoke wsprintfA,eBuf,eFmtRam,[memoryMb]
 stdcall EAppendJava,eBuf
 test eax,eax
 jz .args
 stdcall JsonGet,[eMeta],[eRoot],eKeyMain
 test eax,eax
 jz .args
 stdcall EEmitToken,eax,0
 test eax,eax
 jz .args
 ; Main class ends the non-secret argument file. Session game params remain RAM-only.
 invoke CreateFileW,eArgPath,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .args
 mov [eArgFile],eax
 invoke WriteFile,eax,eArgs,[eJLen],eOutLen,0
 mov esi,eax
 invoke CloseHandle,[eArgFile]
 test esi,esi
 jz .args
 mov eax,[eOutLen]
 cmp eax,[eJLen]
 jne .args
 invoke WideCharToMultiByte,65001,0,javaPath,-1,eScratch,32768,0,0
 test eax,eax
 jz .args
 stdcall EAppendCommand,eScratch
 test eax,eax
 jz .args
 mov byte [eScratch],'@'
 stdcall ERootRelative,eArgPath,eScratch+1,32767
 test eax,eax
 jz .args
 stdcall EAppendCommand,eScratch
 test eax,eax
 jz .args
 stdcall JsonGet,[eMeta],ebx,eKeyGame
 test eax,eax
 jz .args
 stdcall EArgumentArray,eax,1
 test eax,eax
 jz .args
 invoke MultiByteToWideChar,65001,8,eCommand,-1,eCmd,32768
 test eax,eax
 jz .args
 mov eax,1
 ret
 .java: stdcall SetStatusA,eErrJava
 jmp .bad
 .args: stdcall SetStatusA,eErrArgs
 .bad: xor eax,eax
 ret
endp

proc EngineLaunch uses ebx esi edi, param
 local savedJava[1024]:WORD,savedMemory:DWORD
 local logExit:DWORD,logError:DWORD,logOutcome:DWORD,logReason:DWORD,logInherit:DWORD
 if defined OFFSET_LAUNCH_LOG
 stdcall LaunchLogReset
 mov [logExit],-1
 mov [logError],0
 mov [logOutcome],1
 mov [logReason],eErrVersion
 end if
 invoke lstrcpyW,addr savedJava,javaPath
 mov eax,[memoryMb]
 mov [savedMemory],eax
 mov [eRuntimeIsolated],0
 invoke lstrcpyW,eRuntimeDirW,gameDir
 if defined OFFSET_LAUNCH_LOG
 if defined OFFSET_VERSION_SETTINGS
 ; Resolve existing profile isolation for diagnostics even if metadata is broken.
 stdcall VsResolveData,gameDir,selectedVersion,eRuntimeDirW,2048
 test eax,eax
 jnz @f
 invoke lstrcpyW,eRuntimeDirW,gameDir
@@:
 end if
 end if
 mov [eResult],0
 stdcall EValidateVersion
 test eax,eax
 jz .version
 stdcall EBuildPaths
 if defined OFFSET_LAUNCH_LOG
 mov [logReason],llStageMetadata
 end if
 stdcall SetStatusA,eStatusScan
 stdcall ELoadMetadata
 test eax,eax
 jz .finished
 if defined OFFSET_VERSION_SETTINGS
 stdcall VsLaunchSettings,gameDir,selectedVersion
 test eax,eax
 jz .pathbad
 end if
 if defined OFFSET_LAUNCH_LOG
 stdcall LaunchLogBegin
 mov [logReason],llStagePrepare
 end if
 stdcall EPrepareLaunch
 test eax,eax
 jz .finished
 cmp [engineCancel],0
 jne .cancel
 mov edi,eInfo
 xor eax,eax
 mov ecx,sizeof.STARTUPINFO/4
 rep stosd
 mov [eInfo.cb],sizeof.STARTUPINFO
 stdcall SetStatusA,eStatusLaunch
 if defined OFFSET_LAUNCH_LOG
 stdcall LaunchLogPipe,eInfo
 mov [logInherit],eax
 invoke CreateProcessW,javaPath,eCmd,0,0,[logInherit],08000000h,0,eRuntimeDirW,eInfo,eProcess
 else
 invoke CreateProcessW,javaPath,eCmd,0,0,0,0,0,eRuntimeDirW,eInfo,eProcess
 end if
 test eax,eax
 jz .startbad
 invoke CloseHandle,[eProcess.hThread]
 if defined OFFSET_LAUNCH_LOG
 stdcall LaunchLogChildStarted
 end if
 mov [eResult],1
 stdcall SetStatusA,eStatusRunning
 ; Zero transient command line and expanded game arguments containing tokens.
 mov edi,eCommand
 xor eax,eax
 mov ecx,32768/4
 rep stosd
 mov edi,eCmd
 mov ecx,65536/4
 rep stosd
 mov edi,eExpanded
 mov ecx,1200000/4
 rep stosd
 if defined OFFSET_LAUNCH_LOG
 stdcall LaunchLogWait,[eProcess.hProcess]
 else
 invoke WaitForSingleObject,[eProcess.hProcess],-1
 end if
 invoke GetExitCodeProcess,[eProcess.hProcess],eOutLen
 if defined OFFSET_LAUNCH_LOG
 test eax,eax
 jnz @f
 invoke GetLastError
 mov [logError],eax
 mov [eOutLen],-1
@@:
 mov eax,[eOutLen]
 mov [logExit],eax
 end if
 invoke CloseHandle,[eProcess.hProcess]
 cmp [eOutLen],0
 jne .exitbad
 if defined OFFSET_LAUNCH_LOG
 mov [logOutcome],0
 mov [logReason],eStatusDone
 end if
 stdcall SetStatusA,eStatusDone
 jmp .finished
 .exitbad:
 if defined OFFSET_LAUNCH_LOG
 mov [logReason],eErrExit
 end if
 stdcall SetStatusA,eErrExit
 mov [eResult],0
 jmp .finished
 .version: stdcall SetStatusA,eErrVersion
 jmp .finished
 .startbad:
 if defined OFFSET_LAUNCH_LOG
 invoke GetLastError
 mov [logError],eax
 mov [logReason],eErrStart
 end if
 stdcall SetStatusA,eErrStart
 jmp .finished
 .cancel:
 if defined OFFSET_LAUNCH_LOG
 mov [logOutcome],2
 mov [logReason],eErrCancel
 end if
 stdcall SetStatusA,eErrCancel
 jmp .finished
 if defined OFFSET_VERSION_SETTINGS
 .pathbad:
 if defined OFFSET_LAUNCH_LOG
 mov [logReason],eErrPath
 end if
 stdcall SetStatusA,eErrPath
 end if
 .finished:
 if defined OFFSET_LAUNCH_LOG
 stdcall LaunchLogEnd,[logOutcome],[logExit],[logError],[logReason]
 ; Failed preparation/creation can also leave expanded session arguments.
 mov edi,eCommand
 xor eax,eax
 mov ecx,32768/4
 rep stosd
 mov edi,eCmd
 mov ecx,65536/4
 rep stosd
 mov edi,eExpanded
 mov ecx,1200000/4
 rep stosd
 end if
 stdcall ECleanup
 invoke lstrcpyW,javaPath,addr savedJava
 mov eax,[savedMemory]
 mov [memoryMb],eax
 mov [eRuntimeIsolated],0
 invoke lstrcpyW,eRuntimeDirW,gameDir
 invoke PostMessageW,[hWnd],WM_APP+2,0,0
 mov eax,[eResult]
 ret
endp

section '.engxtra' data readable writeable
eOfflineType db 'legacy',0
eErrMissing db 'Required game files are missing. Use Install / Repair for this version while online.',0




