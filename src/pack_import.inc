; Read-only import inspection and common launcher-instance archive metadata.
; No launcher configuration or third-party command is executed during import.
PK_MANIFEST_MAX = 8388608
section '.pimpd' data readable writeable
packInspectReady dd 0
packInspectFormat dd 0
packInspectLoaderKind dd 0
packInspectFiles dd 0
packInspectBytes dd 0
pkManifestIndex dd 0
pkCaptureSize dd 0
pkEmbeddedCount dd 0
piSecurity dd 12,0,1
pkMmcManifest db 'mmc-pack.json',0
pkInstanceCfg db 'instance.cfg',0
pkMinecraftDir db '.minecraft',0
pkMinecraftAlt db 'minecraft',0
pkMmcDefaultName db 'Imported instance',0
pkKeyComponents db 'components',0
pkKeyUid db 'uid',0
pkKeyDependencyOnly db 'dependencyOnly',0
pkKeyCachedDependencyOnly db 'cachedDependencyOnly',0
pkKeyDisabled db 'disabled',0
pkUidMinecraft db 'net.minecraft',0
pkUidFabric db 'net.fabricmc.fabric-loader',0
pkUidForge db 'net.minecraftforge',0
pkUidNeo db 'net.neoforged',0
pkUidQuilt db 'org.quiltmc.quilt-loader',0
pkUidLwjgl db 'org.lwjgl',0
pkUidLwjgl3 db 'org.lwjgl3',0
pkUidIntermediary db 'net.fabricmc.intermediary',0
pkCfgName db 'name',0
pkCfgPre db 'PreLaunchCommand',0
pkCfgPost db 'PostExitCommand',0
pkCfgWrapper db 'WrapperCommand',0
pkTarOutput db '-xOf',0
pkTarFast db '--fast-read',0
pkTarEnd db '--',0
pkNull du 'NUL',0
pkMsgInspect db '正在识别整合包…',0
pkMsgChanged db '整合包文件已改变，请重新选择文件。',0
packInspectName rb 256
packInspectBase rb 128
packInspectLoader rb 128
packInspectHash rb 41
pkArchivePrefix rb 512
pkManifestPathW rw 512
pkInspectArchiveW rw 1024
pkNameCandidate rb 256
pkInspectHashPending rb 41
pkMmcDataPrefix rb 512

section '.pimpc' code readable executable
 nop

; ZIP names have already passed exact central/local validation and safe paths.
; Root manifests win over an unrelated manifest inside a payload. Otherwise
; exactly one common wrapper directory is accepted, with no outside entries.
proc PkDetectArchive uses ebx esi edi
 local pass:DWORD,index:DWORD,chosen:DWORD,kind:DWORD,prefixLength:DWORD
 mov [pkManifest],0
 mov byte [pkArchivePrefix],0
 mov word [pkManifestPathW],0
 mov [pass],0
.pass:
 mov [chosen],-1
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[pkZipCount]
 jae .found
 imul ebx,eax,PK_ZSIZE
 add ebx,pkZipRecords
 cmp dword [ebx+1024],0
 jne .next
 invoke WideCharToMultiByte,65001,0,ebx,-1,pkValue,4096,0,0
 test eax,eax
 jz .bad
 mov esi,pkValue
 mov edi,esi
 mov [prefixLength],0
.depth:
 lodsb
 test al,al
 jz .basename
 cmp al,'/'
 jne .depth
 cmp [prefixLength],0
 jne .next
 mov edi,esi
 mov eax,esi
 sub eax,pkValue
 mov [prefixLength],eax
 jmp .depth
.basename:
 cmp [pass],0
 jne .wrapped
 cmp [prefixLength],0
 jne .next
 jmp .identify
.wrapped:
 cmp [prefixLength],0
 je .next
.identify:
 mov [kind],1
 invoke lstrcmpA,edi,pkMrManifest
 test eax,eax
 jz .candidate
 mov [kind],2
 invoke lstrcmpA,edi,pkCfManifest
 test eax,eax
 jz .candidate
 mov [kind],3
 invoke lstrcmpA,edi,pkMmcManifest
 test eax,eax
 jnz .next
.candidate:
 cmp [chosen],-1
 jne .bad
 mov eax,[index]
 mov [chosen],eax
 mov [pkManifestIndex],eax
 mov eax,[kind]
 mov [pkManifest],eax
 invoke lstrcpyW,pkManifestPathW,ebx
 mov esi,pkValue
 mov edi,pkArchivePrefix
 mov ecx,[prefixLength]
 rep movsb
 mov byte [edi],0
.next:
 inc [index]
 jmp .loop
.found:
 cmp [chosen],-1
 jne .verifyWrapper
 cmp [pass],0
 jne .bad
 mov [pass],1
 jmp .pass
.verifyWrapper:
 cmp [pass],0
 je .mmc
 invoke MultiByteToWideChar,65001,8,pkArchivePrefix,-1,pkWide,512
 test eax,eax
 jz .bad
 dec eax
 mov [prefixLength],eax
 xor ebx,ebx
.wrapperLoop:
 cmp ebx,[pkZipCount]
 jae .mmc
 imul esi,ebx,PK_ZSIZE
 add esi,pkZipRecords
 mov edi,pkWide
 mov ecx,[prefixLength]
 repe cmpsw
 jne .bad
 inc ebx
 jmp .wrapperLoop
.mmc:
 cmp [pkManifest],3
 jne .good
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,pkInstanceCfg
 stdcall PkFindEntry,pkValue
 cmp eax,-1
 je .bad
.good:
 mov eax,1
 ret
.bad:
 mov [pkManifest],0
 xor eax,eax
 ret
endp

proc PkFindEntry uses ebx esi,nameA
 invoke MultiByteToWideChar,65001,8,[nameA],-1,pkWide,512
 test eax,eax
 jz .bad
 xor ebx,ebx
.loop:
 cmp ebx,[pkZipCount]
 jae .bad
 imul esi,ebx,PK_ZSIZE
 add esi,pkZipRecords
 cmp dword [esi+1024],0
 jne .next
 invoke lstrcmpiW,esi,pkWide
 test eax,eax
 jz .found
.next:
 inc ebx
 jmp .loop
.found:
 mov eax,ebx
 ret
.bad:
 or eax,-1
 ret
endp

; Read one indexed archive entry through a bounded stdout pipe. tar receives
; no output directory, scripts, environment overrides or inherited stdin.
; Returns HeapAlloc text or 0; caller owns it. No disk writes occur here.
proc PkCaptureEntry uses ebx esi edi,index
 locals
  readPipe dd ?
  writePipe dd ?
  nullFile dd ?
  text dd ?
  limit dd ?
  used dd ?
  available dd ?
  got dd ?
  exited dd ?
  exitCode dd ?
  started dd ?
  result dd ?
  startTick dd ?
  info STARTUPINFO
  process PROCESS_INFORMATION
 endl
 mov [readPipe],0
 mov [writePipe],0
 mov [nullFile],-1
 mov [text],0
 mov [result],0
 mov [started],0
 mov [used],0
 mov [exited],0
 invoke GetTickCount
 mov [startTick],eax
 mov eax,[index]
 cmp eax,[pkZipCount]
 jae .done
 imul ebx,eax,PK_ZSIZE
 add ebx,pkZipRecords
 mov eax,[ebx+1028]
 test eax,eax
 jz .done
 cmp eax,PK_MANIFEST_MAX
 ja .done
 mov [limit],eax
 inc eax
 invoke GetProcessHeap
 mov edx,[limit]
 inc edx
 invoke HeapAlloc,eax,8,edx
 test eax,eax
 jz .done
 mov [text],eax
 invoke GetWindowsDirectoryW,eTarPath,990
 invoke lstrcatW,eTarPath,pkTarNative
 invoke GetFileAttributesW,eTarPath
 cmp eax,-1
 jne .tar
 invoke GetSystemDirectoryW,eTarPath,990
 invoke lstrcatW,eTarPath,pkTarName
 invoke GetFileAttributesW,eTarPath
 cmp eax,-1
 je .done
.tar:
 stdcall LdCommandStart,eTarPath
 test eax,eax
 jz .done
 stdcall EAppendCommand,pkTarOutput
 stdcall LdCommandWide,pkArchive
 test eax,eax
 jz .done
 stdcall EAppendCommand,pkTarFast
 stdcall EAppendCommand,pkTarEnd
 stdcall LdCommandWide,ebx
 test eax,eax
 jz .done
 invoke MultiByteToWideChar,65001,8,eCommand,-1,eCmd,32768
 test eax,eax
 jz .done
 invoke CreatePipe,addr readPipe,addr writePipe,piSecurity,0
 test eax,eax
 jz .done
 invoke SetHandleInformation,[readPipe],1,0
 test eax,eax
 jz .done
 invoke CreateFileW,pkNull,GENERIC_READ+GENERIC_WRITE,FILE_SHARE_READ+FILE_SHARE_WRITE,piSecurity,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .done
 mov [nullFile],eax
 lea edi,[info]
 xor eax,eax
 mov ecx,sizeof.STARTUPINFO/4
 rep stosd
 mov [info.cb],sizeof.STARTUPINFO
 mov [info.dwFlags],STARTF_USESTDHANDLES
 mov eax,[nullFile]
 mov [info.hStdInput],eax
 mov [info.hStdError],eax
 mov eax,[writePipe]
 mov [info.hStdOutput],eax
 invoke CreateProcessW,eTarPath,eCmd,0,0,1,08000000h,0,0,addr info,addr process
 test eax,eax
 jz .done
 mov [started],1
 invoke CloseHandle,[process.hThread]
 invoke CloseHandle,[writePipe]
 mov [writePipe],0
.pump:
 invoke GetTickCount
 sub eax,[startTick]
 cmp eax,60000
 jae .terminate
 cmp [engineCancel],0
 jne .terminate
 cmp [modCancel],0
 jne .terminate
 invoke PeekNamedPipe,[readPipe],0,0,0,addr available,0
 test eax,eax
 jz .endPipe
 mov eax,[available]
 test eax,eax
 jz .wait
 mov ecx,[limit]
 sub ecx,[used]
 cmp eax,ecx
 ja .terminate
 mov esi,[text]
 add esi,[used]
 invoke ReadFile,[readPipe],esi,[available],addr got,0
 test eax,eax
 jz .terminate
 mov eax,[got]
 add [used],eax
 jmp .pump
.wait:
 cmp [exited],1
 je .endPipe
 invoke WaitForSingleObject,[process.hProcess],50
 test eax,eax
 jnz .pump
 mov [exited],1
 jmp .pump
.terminate:
 invoke TerminateProcess,[process.hProcess],1
 invoke WaitForSingleObject,[process.hProcess],5000
 jmp .done
.endPipe:
 invoke WaitForSingleObject,[process.hProcess],5000
 test eax,eax
 jnz .terminate
 invoke GetExitCodeProcess,[process.hProcess],addr exitCode
 test eax,eax
 jz .done
 cmp [exitCode],0
 jne .done
 mov eax,[used]
 cmp eax,[limit]
 jne .done
 mov [pkCaptureSize],eax
 mov esi,[text]
 mov byte [esi+eax],0
 ; Embedded NUL would otherwise hide manifest or INI suffixes from parsing.
 invoke lstrlenA,esi
 cmp eax,[used]
 jne .done
 mov [result],esi
 mov [text],0
.done:
 cmp [started],0
 je .handles
 invoke CloseHandle,[process.hProcess]
.handles:
 cmp [readPipe],0
 je .write
 invoke CloseHandle,[readPipe]
.write:
 cmp [writePipe],0
 je .null
 invoke CloseHandle,[writePipe]
.null:
 cmp [nullFile],-1
 je .free
 invoke CloseHandle,[nullFile]
.free:
 stdcall FreeMem,[text]
 mov eax,[result]
 ret
endp

proc PackInspect uses ebx esi edi,archivePathW
 local lock:DWORD,result:DWORD
 mov [packInspectReady],0
 mov [packInspectFormat],0
 mov [packInspectLoaderKind],0
 mov [packInspectFiles],0
 mov [packInspectBytes],0
 mov byte [packInspectName],0
 mov byte [packInspectBase],0
 mov byte [packInspectLoader],0
 mov byte [packInspectHash],0
 mov [pkContext],0
 mov [pkText],0
 mov [lock],-1
 mov [result],0
 stdcall SetStatusA,pkMsgInspect
 invoke GetFullPathNameW,[archivePathW],1024,pkArchive,0
 test eax,eax
 jz .badArchive
 cmp eax,1024
 jae .badArchive
 invoke CreateFileW,pkArchive,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .badArchive
 mov [lock],eax
 stdcall PkReadZip,pkArchive
 test eax,eax
 jz .badArchive
 stdcall Sha1File,pkArchive,pkInspectHashPending
 test eax,eax
 jz .badArchive
 stdcall PkCaptureEntry,[pkManifestIndex]
 test eax,eax
 jz .badManifest
 mov [pkText],eax
 stdcall PkParseManifestText
 test eax,eax
 jz .badManifest
 cmp [engineCancel],0
 jne .done
 cmp [modCancel],0
 jne .done
 invoke lstrcpyA,packInspectName,packInstalledName
 invoke lstrcpyA,packInspectBase,pkBase
 invoke lstrcpyA,packInspectLoader,pkLoader
 invoke lstrcpyA,packInspectHash,pkInspectHashPending
 invoke lstrcpyW,pkInspectArchiveW,pkArchive
 mov eax,[pkFormat]
 mov [packInspectFormat],eax
 mov eax,[pkLoaderKind]
 mov [packInspectLoaderKind],eax
 mov eax,[pkFileCount]
 add eax,[pkEmbeddedCount]
 mov [packInspectFiles],eax
 mov eax,[pkZipTotal]
 mov [packInspectBytes],eax
 mov [packInspectReady],1
 mov [result],1
 jmp .done
.badArchive:
 stdcall SetStatusA,pkMsgArchive
 jmp .done
.badManifest:
 stdcall SetStatusA,pkMsgManifest
.done:
 stdcall PkCloseManifest
 cmp [lock],-1
 je .return
 invoke CloseHandle,[lock]
.return:
 cmp [engineCancel],0
 jne .cancelled
 cmp [modCancel],0
 je .result
.cancelled:
 mov [packInspectReady],0
 mov [result],0
 stdcall SetStatusA,pkMsgCancel
.result:
 mov eax,[result]
 ret
endp

; Prism and MultiMC export the same versioned component-list structure.
; Only standard Minecraft/loader components are translated to our installer.
; Custom patches, launch commands and arbitrary components are not executed.
proc PkReadMmc uses ebx esi edi,root
 locals
  list dd ?
  count dd ?
  index dd ?
  item dd ?
  cfg dd ?
  result dd ?
 endl
 mov [cfg],0
 mov [result],0
 mov byte [pkBase],0
 mov byte [pkLoader],0
 mov [pkLoaderKind],0
 mov [pkFileCount],0
 mov [pkEmbeddedCount],0
 stdcall JsonGet,[pkContext],[root],pkKeyFormat
 stdcall JsonInt,[pkContext],eax
 cmp eax,1
 jne .done
 stdcall JsonGet,[pkContext],[root],pkKeyComponents
 test eax,eax
 jz .done
 mov [list],eax
 stdcall JsonCount,[pkContext],eax
 test eax,eax
 jz .done
 cmp eax,16
 ja .done
 mov [count],eax
 mov [index],0
.component:
 mov eax,[index]
 cmp eax,[count]
 jae .componentsDone
 stdcall JsonAt,[pkContext],[list],eax
 mov [item],eax
 stdcall JsonGet,[pkContext],eax,pkKeyDisabled
 test eax,eax
 jz .uid
 stdcall JsonBool,[pkContext],eax
 test eax,eax
 jnz .done
.uid:
 mov eax,[item]
 stdcall JsonGet,[pkContext],eax,pkKeyUid
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .done
 invoke lstrcmpA,pkValue,pkUidMinecraft
 test eax,eax
 jz .minecraft
 mov ebx,1
 invoke lstrcmpA,pkValue,pkUidFabric
 test eax,eax
 jz .loader
 mov ebx,2
 invoke lstrcmpA,pkValue,pkUidForge
 test eax,eax
 jz .loader
 mov ebx,3
 invoke lstrcmpA,pkValue,pkUidNeo
 test eax,eax
 jz .loader
 mov ebx,4
 invoke lstrcmpA,pkValue,pkUidQuilt
 test eax,eax
 jz .loader
 ; Runtime libraries selected transitively by Minecraft/loader metadata may
 ; appear in exports. Only dependency-only records may be elided here.
 invoke lstrcmpA,pkValue,pkUidLwjgl3
 test eax,eax
 jz .dependency
 invoke lstrcmpA,pkValue,pkUidLwjgl
 test eax,eax
 jz .dependency
 invoke lstrcmpA,pkValue,pkUidIntermediary
 test eax,eax
 jnz .done
.dependency:
 stdcall JsonGet,[pkContext],[item],pkKeyDependencyOnly
 stdcall JsonBool,[pkContext],eax
 cmp eax,1
 je .next
 stdcall JsonGet,[pkContext],[item],pkKeyCachedDependencyOnly
 stdcall JsonBool,[pkContext],eax
 cmp eax,1
 jne .done
 jmp .next
.minecraft:
 stdcall PkMmcVersion,[item]
 test eax,eax
 jz .done
 cmp byte [pkBase],0
 jne .done
 invoke lstrlenA,pkValue2
 cmp eax,127
 ja .done
 invoke lstrcpyA,pkBase,pkValue2
 jmp .next
.loader:
 stdcall PkMmcVersion,[item]
 test eax,eax
 jz .done
 cmp [pkLoaderKind],0
 jne .done
 invoke lstrlenA,pkValue2
 cmp eax,127
 ja .done
 mov [pkLoaderKind],ebx
 invoke lstrcpyA,pkLoader,pkValue2
.next:
 inc [index]
 jmp .component
.componentsDone:
 cmp byte [pkBase],0
 je .done
 ; Reject custom launcher patches rather than silently dropping behavior.
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,.patches
 test eax,eax
 jz .done
 stdcall PkHasPrefix,pkValue
 test eax,eax
 jnz .done
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,.jarmods
 stdcall PkHasPrefix,pkValue
 test eax,eax
 jnz .done
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,pkInstanceCfg
 stdcall PkFindEntry,pkValue
 cmp eax,-1
 je .done
 stdcall PkCaptureEntry,eax
 test eax,eax
 jz .done
 mov [cfg],eax
 stdcall PkReadCfg,eax
 test eax,eax
 jz .done
 ; Exactly one game-data directory is supported, with an indexed payload.
 invoke lstrcpyA,pkOverrides,pkMinecraftDir
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,pkMinecraftDir
 stdcall ModAppend,pkValue,4096,.slash
 stdcall PkHasPrefix,pkValue
 mov ebx,eax
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,pkMinecraftAlt
 stdcall ModAppend,pkValue,4096,.slash
 stdcall PkHasPrefix,pkValue
 test ebx,ebx
 jz .alternate
 test eax,eax
 jnz .done
 jmp .accepted
.alternate:
 test eax,eax
 jz .done
 invoke lstrcpyA,pkOverrides,pkMinecraftAlt
.accepted:
 stdcall PkValidatePayload,pkOverrides
 test eax,eax
 jz .done
 mov [result],1
.done:
 stdcall FreeMem,[cfg]
 mov eax,[result]
 ret
.patches db 'patches/',0
.jarmods db 'jarmods/',0
.slash db '/',0
endp

; Only selected Minecraft/loader components require an explicit version.
; Dependency-only runtime records may carry cachedVersion with no version.
proc PkMmcVersion,item
 stdcall JsonGet,[pkContext],[item],pkKeyVersion
 stdcall PkString,[pkContext],eax,pkValue2,2048
 test eax,eax
 jz .done
 stdcall LoaderSafeId,pkValue2
.done:
 ret
endp

; True when an indexed entry starts with an exact UTF-8 prefix.
proc PkHasPrefix uses ebx esi edi,prefixA
 local prefix[512]:WORD,length:DWORD
 invoke MultiByteToWideChar,65001,8,[prefixA],-1,addr prefix,512
 test eax,eax
 jz .no
 dec eax
 mov [length],eax
 xor ebx,ebx
.loop:
 cmp ebx,[pkZipCount]
 jae .no
 imul esi,ebx,PK_ZSIZE
 add esi,pkZipRecords
 lea edi,[prefix]
 mov ecx,[length]
 repe cmpsw
 je .yes
 inc ebx
 jmp .loop
.yes:
 mov eax,1
 ret
.no:
 xor eax,eax
 ret
endp

; Validate every indexed data payload before downloading any game files.
proc PkValidatePayload uses ebx esi edi,prefixA
 local prefix[512]:WORD,length:DWORD,index:DWORD,count:DWORD
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,[prefixA]
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,pkValue,-1,addr prefix,512
 test eax,eax
 jz .bad
 dec eax
 mov [length],eax
 mov [index],0
 mov [count],0
.loop:
 mov eax,[index]
 cmp eax,[pkZipCount]
 jae .good
 imul ebx,eax,PK_ZSIZE
 add ebx,pkZipRecords
 cmp dword [ebx+1024],0
 jne .next
 mov esi,ebx
 lea edi,[prefix]
 mov ecx,[length]
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
 inc [count]
.next:
 inc [index]
 jmp .loop
.good:
 mov eax,[count]
 cmp [pkFormat],3
 jne .accepted
 mov [pkEmbeddedCount],eax
.accepted:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; instance.cfg is read as data only. Launch hooks are rejected, never copied
; into OFFSET settings. Qt's common plain UTF-8 name representation is kept.
proc PkReadCfg uses ebx esi edi,text
 locals
  line rb 2048
  key rb 128
  value rb 2048
  haveName dd ?
 endl
 mov [haveName],0
 invoke lstrcpyA,packInstalledName,pkMmcDefaultName
 mov esi,[text]
 cmp word [esi],0BBEFh
 jne .line
 cmp byte [esi+2],0BFh
 jne .line
 add esi,3
.line:
 cmp byte [esi],0
 je .done
 lea edi,[line]
 xor ebx,ebx
.read:
 lodsb
 test al,al
 jz .last
 cmp al,10
 je .parse
 cmp al,13
 je .read
 cmp ebx,2046
 jae .bad
 stosb
 inc ebx
 jmp .read
.last:
 dec esi
.parse:
 mov byte [edi],0
 push esi
 lea esi,[line]
 lea edi,[key]
 xor ebx,ebx
.key:
 lodsb
 test al,al
 jz .skip
 cmp al,'='
 je .value
 cmp ebx,126
 jae .skip
 stosb
 inc ebx
 jmp .key
.value:
 mov byte [edi],0
 lea edi,[value]
 invoke lstrcpyA,edi,esi
 lea ebx,[key]
 invoke lstrcmpA,ebx,pkCfgName
 test eax,eax
 jz .name
 invoke lstrcmpA,ebx,pkCfgPre
 test eax,eax
 jz .command
 invoke lstrcmpA,ebx,pkCfgPost
 test eax,eax
 jz .command
 invoke lstrcmpA,ebx,pkCfgWrapper
 test eax,eax
 jnz .skip
.command:
 cmp byte [edi],0
 jne .reject
 jmp .skip
.name:
 cmp [haveName],0
 jne .reject
 invoke lstrlenA,edi
 test eax,eax
 jz .skip
 cmp eax,255
 ja .reject
 invoke MultiByteToWideChar,65001,8,edi,-1,pkWide,512
 test eax,eax
 jz .reject
 invoke lstrcpyA,packInstalledName,edi
 mov [haveName],1
.skip:
 pop esi
 jmp .line
.reject:
 pop esi
.bad:
 xor eax,eax
 ret
.done:
 ; JSON-quote the bounded decoded UTF-8 name used by the complete marker.
 mov esi,packInstalledName
 mov edi,pkNameRaw
 mov al,'"'
 stosb
.quote:
 lodsb
 test al,al
 jz .close
 cmp al,32
 jb .bad
 cmp al,'"'
 je .escape
 cmp al,92
 jne .copy
.escape:
 mov byte [edi],92
 inc edi
.copy:
 stosb
 jmp .quote
.close:
 mov al,'"'
 stosb
 mov byte [edi],0
 mov eax,1
 ret
endp
