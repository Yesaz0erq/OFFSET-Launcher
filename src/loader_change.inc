; Original per-profile loader transaction. One worker at a time.
; Load is read-only. Fetch/Apply/Restore are synchronous, with no UI messages.
section '.lcdata' data readable writeable
lcKind dd 0
lcBackupReady dd 0
lcResult dd 0
lcSerial dd 0
lcFailureReason dd 0
lcScratchSeed dd 0
lcScratchSuffix du '\.offset-loader-change\',0
lcTagFormat db '%08X-%08X-%08X',0
lcScratchFormat db '%08X',0
lcNativeFormat db 'offset-natives-%s',0
lcPendingPrefixW du '\offset-loader-pending-',0
lcKeyNative db 'offsetNatives',0
lcKeyBefore db 'offsetLoaderBefore',0
lcKeyRuntime db 'offsetLoaderArtifacts',0
lcNeoFormFlagA db '--fml.neoFormVersion',0
lcRuntimeCoreFormat db 'net/neoforged/neoforge/%s/neoforge-%s-%s.jar',0
lcRuntimeMcFormat db 'net/minecraft/client/%s-%s/client-%s-%s-%s.jar',0
lcRuntimeClientA db 'client',0
lcRuntimeUniversalA db 'universal',0
lcRuntimeSrgA db 'srg',0
lcRuntimeExtraA db 'extra',0
lcRuntimeArrayOpen db '[',0
lcRuntimePathOpen db '{"path":',0
lcRuntimeHashOpen db ',"sha1":',0
lcHistoryFormat db 'offset-loader-before-%s.json',0
lcHistoryPrefixA db 'offset-loader-before-',0
lcMsgInvalid db 'This profile cannot change its loader safely.',0
lcMsgPreparing db 'Preparing loader change...',0
lcMsgChanged db 'Loader changed.',0
lcMsgRestored db 'Previous loader restored.',0
lcMsgFailed db 'Loader change failed. The original profile was preserved.',0
lcMsgCancelled db 'Loader change cancelled. The original profile was preserved.',0
lcMsgConflict db 'A shared library differs from the expected file. The original profile was preserved.',0
lcMsgBackup db 'The previous loader could not be restored.',0
lcMsgListed db 'Loader versions ready.',0
lcMsgNoVersions db 'No compatible loader versions found.',0
lcNeoFlagA db '--fml.neoForgeVersion',0
lcMcFlagA db '--fml.mcVersion',0
lcBootstrapA db 'cpw.mods.bootstraplauncher.BootstrapLauncher',0

lcBaseA rb 128
lcVersionA rb 128
lcRecord rb VS_SIZE
lcSourceA rb 128
lcTargetA rb 128
lcTagA rb 128
lcScratchLeafA rb 16
lcNativeNameA rb 128
lcHistoryLeafA rb 128
lcBaseHashA rb 64
lcOldHashA rb 64
lcRootW rw 1024
lcSavedRootW rw 1024
lcSavedJavaW rw 1024
lcScratchW rw 1024
lcProfileW rw 2048
lcSourceJsonW rw 2048
lcSourceJarW rw 2048
lcBaseJsonW rw 2048
lcBaseJarW rw 2048
lcBackupW rw 2048
lcHistoryW rw 2048
lcPendingW rw 2048
lcNativeTargetW rw 2048
lcNativeStageW rw 2048

section '.lccode' code readable executable
 nop

; Decode a single consistent core loader version, without guessing from IDs.
proc LcReadVersion uses ebx esi edi,ctx,base,version,kindOut
 locals
  root dd ?
  list dd ?
  count dd ?
  index dd ?
  kind dd ?
  found dd ?
  value rb 512
  candidate rb 128
  neo rb 128
  mc rb 128
 endl
 mov [found],0
 stdcall JsonRoot,[ctx]
 mov [root],eax
 stdcall JsonGet,[ctx],eax,pLibsKey
 mov [list],eax
 stdcall PIsArray,[ctx],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[ctx],[list]
 cmp eax,4096
 ja .bad
 mov [count],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .done
 stdcall JsonAt,[ctx],[list],eax
 stdcall JsonGet,[ctx],eax,pNameKey
 stdcall VsString,[ctx],eax,addr value,512
 test eax,eax
 jz .bad
 lea esi,[value]
 mov [kind],1
 mov edi,vsFabricA
 stdcall VsPrefix,esi,edi
 test eax,eax
 jnz .match
 mov [kind],2
 mov edi,vsForgeA
 stdcall VsPrefix,esi,edi
 test eax,eax
 jnz .match
 mov [kind],3
 mov edi,vsNeoA
 stdcall VsPrefix,esi,edi
 test eax,eax
 jnz .match
 mov [kind],4
 mov edi,vsQuiltA
 stdcall VsPrefix,esi,edi
 test eax,eax
 jz .next
.match:
 invoke lstrlenA,edi
 add esi,eax
 cmp [kind],2
 jne .suffix
 ; Forge Maven versions include the Minecraft version; the UI list does not.
 invoke lstrlenA,[base]
 mov ecx,eax
 mov edi,[base]
 repe cmpsb
 jne .bad
 cmp byte [esi],'-'
 jne .bad
 inc esi
.suffix:
 lea edi,[candidate]
 xor ecx,ecx
.copy:
 mov al,[esi]
 test al,al
 jz .copied
 cmp al,':'
 je .copied
 cmp al,'@'
 je .copied
 cmp ecx,120
 jae .bad
 mov [edi+ecx],al
 inc esi
 inc ecx
 jmp .copy
.copied:
 mov byte [edi+ecx],0
 stdcall LoaderSafeId,edi
 test eax,eax
 jz .bad
 cmp [found],0
 je .first
 mov eax,[kind]
 cmp eax,[found]
 jne .bad
 invoke lstrcmpA,edi,[version]
 test eax,eax
 jnz .bad
 jmp .next
.first:
 invoke lstrcpyA,[version],edi
 mov eax,[kind]
 mov [found],eax
.next:
 inc [index]
 jmp .loop
.done:
 ; Current NeoForge stores its version in game arguments, without a NeoForge
 ; Maven library. Require the official bootstrap entry point and Minecraft flag.
 stdcall LcGameFlag,[ctx],lcNeoFlagA,addr neo
 cmp eax,-1
 je .bad
 test eax,eax
 jz .withoutNeo
 stdcall LoaderSafeId,addr neo
 test eax,eax
 jz .bad
 stdcall LcGameFlag,[ctx],lcMcFlagA,addr mc
 cmp eax,1
 jne .bad
 invoke lstrcmpA,addr mc,[base]
 test eax,eax
 jnz .bad
 stdcall JsonGet,[ctx],[root],pMainKey
 stdcall VsString,[ctx],eax,addr mc,128
 test eax,eax
 jz .bad
 invoke lstrcmpA,addr mc,lcBootstrapA
 test eax,eax
 jnz .bad
 cmp [found],0
 je .neoFirst
 cmp [found],3
 jne .bad
 invoke lstrcmpA,addr neo,[version]
 test eax,eax
 jnz .bad
 jmp .withoutNeo
.neoFirst:
 invoke lstrcpyA,[version],addr neo
 mov [found],3
.withoutNeo:
 mov eax,[found]
 test eax,eax
 jz .bad
 mov edx,[kindOut]
 mov [edx],eax
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; 0 absent, 1 unique string value, -1 duplicate/truncated/invalid flag value.
proc LcGameFlag uses ebx,ctx,flag,destination
 local list:DWORD,count:DWORD,index:DWORD,found:DWORD,value[128]:BYTE
 mov [found],0
 stdcall JsonRoot,[ctx]
 stdcall JsonGet,[ctx],eax,pArgsKey
 stdcall JsonGet,[ctx],eax,pGameKey
 mov [list],eax
 stdcall JsonCount,[ctx],eax
 cmp eax,4096
 ja .bad
 mov [count],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .done
 stdcall JsonAt,[ctx],[list],eax
 stdcall VsString,[ctx],eax,addr value,128
 test eax,eax
 jz .next
 invoke lstrcmpA,addr value,[flag]
 test eax,eax
 jnz .next
 cmp [found],0
 jne .bad
 inc [index]
 mov eax,[index]
 cmp eax,[count]
 jae .bad
 stdcall JsonAt,[ctx],[list],eax
 stdcall VsString,[ctx],eax,[destination],128
 test eax,eax
 jz .bad
 mov [found],1
.next:
 inc [index]
 jmp .loop
.done:
 mov eax,[found]
 ret
.bad:
 mov eax,-1
 ret
endp

; Profiles accepted here are standalone launcher-normalized JSON documents.
proc LcCheckProfile uses ebx,ctx,id,base,hashOut
 local root:DWORD,value[128]:BYTE
 stdcall JsonRoot,[ctx]
 mov ebx,eax
 mov [root],eax
 stdcall JsonGet,[ctx],ebx,pVersionKey
 stdcall VsString,[ctx],eax,addr value,128
 test eax,eax
 jz .bad
 invoke lstrcmpA,addr value,[id]
 test eax,eax
 jnz .bad
 stdcall JsonGet,[ctx],ebx,pInheritKey
 test eax,eax
 jnz .bad
 stdcall JsonGet,[ctx],ebx,pNormalizedKey
 stdcall JsonBool,[ctx],eax
 cmp eax,1
 jne .bad
 stdcall JsonGet,[ctx],ebx,pBaseKey
 stdcall VsString,[ctx],eax,addr value,128
 test eax,eax
 jz .bad
 invoke lstrcmpA,addr value,[base]
 test eax,eax
 jnz .bad
 stdcall JsonGet,[ctx],ebx,pArgsKey
 mov ebx,eax
 stdcall JsonGet,[ctx],ebx,pJvmKey
 stdcall PIsArray,[ctx],eax
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],ebx,pGameKey
 stdcall PIsArray,[ctx],eax
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],[root],pMainKey
 stdcall VsString,[ctx],eax,addr value,128
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],[root],pDownloadsKey
 stdcall JsonGet,[ctx],eax,pClientKey
 stdcall JsonGet,[ctx],eax,pShaKey
 stdcall VsString,[ctx],eax,[hashOut],64
 test eax,eax
 jz .bad
 stdcall PIsHash,[hashOut]
 ret
.bad:
 xor eax,eax
 ret
endp

proc LoaderChangeLoad uses ebx,rootPath,id
 locals
  text dd ?
  ctx dd ?
  version rb 128
  kind dd ?
  hash rb 64
  path rw 2048
 endl
 mov [text],0
 mov [ctx],0
 mov [lcKind],0
 mov [lcBackupReady],0
 mov byte [lcBaseA],0
 mov byte [lcVersionA],0
 stdcall MemoryDetect
 stdcall VsRead,[rootPath],[id],lcRecord,1
 test eax,eax
 jz .bad
 invoke lstrcpyW,lcRootW,lcRecord+VS_ROOT
 invoke lstrlenW,lcRootW
 cmp eax,650
 ja .bad
 invoke lstrcpyA,lcSourceA,[id]
 invoke lstrcpyA,lcBaseA,lcRecord+VS_BASE
 stdcall LoaderSafeId,lcBaseA
 test eax,eax
 jz .bad
 invoke lstrcmpA,lcBaseA,lcSourceA
 test eax,eax
 jz .bad
 invoke lstrcpyW,lcProfileW,lcRecord+VS_PROFILE
 stdcall LcPath,lcProfileW,lcSourceA,pJsonW,lcSourceJsonW
 stdcall LcPath,lcProfileW,lcSourceA,pJarW,lcSourceJarW
 stdcall EReadFile,lcSourceJsonW
 test eax,eax
 jz .bad
 mov [text],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .bad
 mov [ctx],eax
 stdcall LcCheckProfile,eax,lcSourceA,lcBaseA,lcBaseHashA
 test eax,eax
 jz .bad
 stdcall LcReadVersion,[ctx],lcBaseA,lcVersionA,lcKind
 test eax,eax
 jz .bad
 stdcall JsonRoot,[ctx]
 stdcall JsonGet,[ctx],eax,lcKeyBefore
 test eax,eax
 jz .good
 stdcall VsString,[ctx],eax,addr version,128
 test eax,eax
 jz .good
 stdcall LcBeforeValid,addr version
 test eax,eax
 jz .good
 invoke MultiByteToWideChar,65001,8,addr version,-1,addr path,128
 test eax,eax
 jz .good
 invoke lstrcpyW,lcBackupW,lcProfileW
 invoke lstrcatW,lcBackupW,pSlashW
 invoke lstrcatW,lcBackupW,addr path
 stdcall JsonFree,[ctx]
 mov [ctx],0
 stdcall FreeMem,[text]
 mov [text],0
 ; Only expose a rollback when its JSON still matches this profile and game.
 stdcall VsRegularFile,lcBackupW
 test eax,eax
 jz .good
 stdcall EReadFile,lcBackupW
 test eax,eax
 jz .good
 mov [text],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .good
 mov [ctx],eax
 stdcall LcCheckProfile,eax,lcSourceA,lcBaseA,addr hash
 test eax,eax
 jz .good
 invoke lstrcmpiA,addr hash,lcBaseHashA
 test eax,eax
 jnz .good
 stdcall LcReadVersion,[ctx],lcBaseA,addr version,addr kind
 test eax,eax
 jz .good
 mov eax,[kind]
 cmp eax,[lcKind]
 jne .good
 mov [lcBackupReady],1
.good:
 mov ebx,1
 jmp .free
.bad:
 mov [lcKind],0
 mov [lcBackupReady],0
 xor ebx,ebx
.free:
 stdcall JsonFree,[ctx]
 stdcall FreeMem,[text]
 mov eax,ebx
 ret
endp

proc LcPath,parent,id,extension,destination
 local wide[128]:WORD
 invoke MultiByteToWideChar,65001,8,[id],-1,addr wide,128
 test eax,eax
 jz .done
 invoke lstrcpyW,[destination],[parent]
 invoke lstrcatW,[destination],pSlashW
 invoke lstrcatW,[destination],addr wide
 invoke lstrcatW,[destination],[extension]
 mov eax,1
.done:
 ret
endp

proc LoaderChangeFetch,rootPath,id
 stdcall LoaderChangeLoad,[rootPath],[id]
 test eax,eax
 jz .bad
 stdcall LoaderFetchVersions,lcBaseA,[lcKind]
 mov [lcResult],eax
 test eax,eax
 jnz .listed
 cmp [engineCancel],0
 je .return
 stdcall SetStatusA,lcMsgCancelled
 jmp .return
.listed:
 mov eax,lcMsgListed
 cmp [loaderVersionCount],0
 jne .status
 mov eax,lcMsgNoVersions
.status:
 stdcall SetStatusA,eax
.return:
 mov eax,[lcResult]
 ret
.bad:
 mov [loaderVersionCount],0
 mov [lcResult],0
 stdcall SetStatusA,lcMsgInvalid
 xor eax,eax
 ret
endp

proc LcTag uses ebx
 inc [lcSerial]
 invoke GetCurrentProcessId
 mov ebx,eax
 invoke GetTickCount
 mov edx,eax
 xor edx,ebx
 xor edx,[lcSerial]
 mov [lcScratchSeed],edx
 cinvoke wsprintfA,lcTagA,lcTagFormat,ebx,eax,[lcSerial]
 cinvoke wsprintfA,lcHistoryLeafA,lcHistoryFormat,lcTagA
 ret
endp

; Keep the staging basename short. Java supports long paths that native Win32
; callers cannot reopen without policy changes. CREATE_NEW ownership is still
; guaranteed by CreateDirectory and bounded collision retries.
proc LcCreateScratch uses ebx
 local base[1024]:WORD,wide[16]:WORD,attempt:DWORD
 mov [attempt],0
 invoke lstrcpyW,addr base,lcRootW
 invoke lstrcatW,addr base,lcScratchSuffix
 stdcall VsDirectoryChain,addr base,1
 test eax,eax
 jz .bad
 stdcall EnsureDirectory,addr base
 test eax,eax
 jz .bad
.try:
 mov ebx,[lcScratchSeed]
 add ebx,[attempt]
 cinvoke wsprintfA,lcScratchLeafA,lcScratchFormat,ebx
 invoke MultiByteToWideChar,65001,8,lcScratchLeafA,-1,addr wide,16
 invoke lstrcpyW,lcScratchW,addr base
 invoke lstrcatW,lcScratchW,addr wide
 invoke CreateDirectoryW,lcScratchW,0
 test eax,eax
 jnz .good
 invoke GetLastError
 cmp eax,183
 jne .bad
 inc [attempt]
 cmp [attempt],32
 jb .try
.bad:
 xor eax,eax
 ret
.good:
 mov eax,1
 ret
endp

proc LcBeforeValid uses esi,value
 stdcall LoaderSafeId,[value]
 test eax,eax
 jz .bad
 stdcall VsPrefix,[value],lcHistoryPrefixA
 test eax,eax
 jz .bad
 invoke lstrlenA,[value]
 cmp eax,26
 jb .bad
 mov esi,[value]
 cmp dword [esi+eax-5],'.jso'
 jne .bad
 cmp byte [esi+eax-1],'n'
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Re-serialize metadata, replacing the one rollback pointer atomically with
; the main JSON. Apply also installs its own complete native-directory name.
proc LcStamp uses ebx esi edi,ctx,nativeLeaf,runtimeTable
 local count:DWORD,index:DWORD,wrote:DWORD,key:DWORD,value:DWORD
 mov [pOutput],0
 mov [pLength],0
 mov [pFailed],0
 mov [wrote],0
 invoke GetProcessHeap
 invoke HeapAlloc,eax,HEAP_ZERO_MEMORY,P_CAP
 test eax,eax
 jz .bad
 mov [pOutput],eax
 stdcall PAdd,pObjectOpen
 stdcall JsonRoot,[ctx]
 mov ebx,eax
 stdcall JsonCount,[ctx],ebx
 mov [count],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .fields
 stdcall JsonKeyAt,[ctx],ebx,eax
 mov [key],eax
 stdcall VsString,[ctx],eax,pKey,256
 test eax,eax
 jz .bad
 invoke lstrcmpA,pKey,lcKeyBefore
 test eax,eax
 jz .next
 cmp [nativeLeaf],0
 je .emit
 invoke lstrcmpA,pKey,lcKeyNative
 test eax,eax
 jz .next
 invoke lstrcmpA,pKey,lcKeyRuntime
 test eax,eax
 jz .next
.emit:
 cmp [wrote],0
 je .first
 stdcall PAdd,pComma
.first:
 stdcall PToken,[ctx],[key]
 stdcall PAdd,pColon
 stdcall JsonValueAt,[ctx],ebx,[index]
 stdcall PToken,[ctx],eax
 inc [wrote]
.next:
 inc [index]
 jmp .loop
.fields:
 stdcall PAdd,pComma
 stdcall PQuoted,lcKeyBefore
 stdcall PAdd,pColon
 stdcall PQuoted,lcHistoryLeafA
 cmp [nativeLeaf],0
 je .close
 stdcall PAdd,pComma
 stdcall PQuoted,lcKeyNative
 stdcall PAdd,pColon
 stdcall PQuoted,[nativeLeaf]
 cmp [runtimeTable],0
 je .close
 stdcall PAdd,pComma
 stdcall PQuoted,lcKeyRuntime
 stdcall PAdd,pColon
 stdcall PAdd,[runtimeTable]
.close:
 stdcall PAdd,pClose
 cmp [pFailed],0
 jne .bad
 mov eax,[pOutput]
 mov [pOutput],0
 ret
.bad:
 stdcall FreeMem,[pOutput]
 mov [pOutput],0
 xor eax,eax
 ret
endp

; Official NeoForge installers generate these runtime JARs outside the
; profile's library array. FML resolves their Maven paths dynamically; keep
; them out of the bootstrap classpath and record their actual local hashes.
proc LcRuntimeItem uses ebx esi edi,relative
 local wide[1100]:WORD,path[2048]:WORD,parent[2048]:WORD,hash[64]:BYTE
 stdcall PSafePath,[relative]
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,[relative],-1,addr wide,1100
 test eax,eax
 jz .bad
 lea esi,[wide]
.slashes:
 mov ax,[esi]
 test ax,ax
 jz .path
 cmp ax,'/'
 jne .advance
 mov word [esi],92
.advance:
 add esi,2
 jmp .slashes
.path:
 invoke lstrcpyW,addr path,gameDir
 invoke lstrcatW,addr path,pLibrariesW
 invoke lstrcatW,addr path,addr wide
 invoke lstrcpyW,addr parent,addr path
 invoke lstrlenW,addr parent
 lea edi,[parent+eax*2]
.parent:
 cmp word [edi],92
 je .parentReady
 sub edi,2
 jmp .parent
.parentReady:
 mov word [edi],0
 stdcall VsDirectoryChain,addr parent,0
 test eax,eax
 jz .bad
 stdcall VsRegularFile,addr path
 test eax,eax
 jz .bad
 stdcall PHashFile,addr path,addr hash
 test eax,eax
 jz .bad
 stdcall PAdd,lcRuntimePathOpen
 stdcall PQuoted,[relative]
 stdcall PAdd,lcRuntimeHashOpen
 stdcall PQuoted,addr hash
 stdcall PAdd,pClose
 cmp [pFailed],0
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc LcBuildRuntimeTable uses ebx,childCtx
 local form[128]:BYTE,relative[1024]:BYTE
 stdcall LcGameFlag,[childCtx],lcNeoFormFlagA,addr form
 cmp eax,1
 jne .badEarly
 stdcall LoaderSafeId,addr form
 test eax,eax
 jz .badEarly
 mov [pOutput],0
 mov [pLength],0
 mov [pFailed],0
 invoke GetProcessHeap
 invoke HeapAlloc,eax,HEAP_ZERO_MEMORY,P_CAP
 test eax,eax
 jz .badEarly
 mov [pOutput],eax
 stdcall PAdd,lcRuntimeArrayOpen
 cinvoke wsprintfA,addr relative,lcRuntimeCoreFormat,lcTargetA,lcTargetA,lcRuntimeClientA
 stdcall LcRuntimeItem,addr relative
 test eax,eax
 jz .bad
 stdcall PAdd,pComma
 cinvoke wsprintfA,addr relative,lcRuntimeCoreFormat,lcTargetA,lcTargetA,lcRuntimeUniversalA
 stdcall LcRuntimeItem,addr relative
 test eax,eax
 jz .bad
 stdcall PAdd,pComma
 cinvoke wsprintfA,addr relative,lcRuntimeMcFormat,lcBaseA,addr form,lcBaseA,addr form,lcRuntimeSrgA
 stdcall LcRuntimeItem,addr relative
 test eax,eax
 jz .bad
 stdcall PAdd,pComma
 cinvoke wsprintfA,addr relative,lcRuntimeMcFormat,lcBaseA,addr form,lcBaseA,addr form,lcRuntimeExtraA
 stdcall LcRuntimeItem,addr relative
 test eax,eax
 jz .bad
 stdcall PAdd,pArrayClose
 cmp [pFailed],0
 jne .bad
 mov eax,[pOutput]
 mov [pOutput],0
 ret
.bad:
 stdcall FreeMem,[pOutput]
 mov [pOutput],0
.badEarly:
 xor eax,eax
 ret
endp

proc LcPromoteRuntime uses ebx,ctx
 local list:DWORD,index:DWORD,count:DWORD
 stdcall JsonRoot,[ctx]
 stdcall JsonGet,[ctx],eax,lcKeyRuntime
 test eax,eax
 jz .good
 mov [list],eax
 stdcall PIsArray,[ctx],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[ctx],[list]
 cmp eax,16
 ja .bad
 mov [count],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .good
 stdcall JsonAt,[ctx],[list],eax
 stdcall LcPromoteArtifact,[ctx],eax,1
 test eax,eax
 jz .bad
 inc [index]
 jmp .loop
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Pure read-only launch/restore validation, independent of transaction globals.
proc LcVerifyRuntimeArtifacts uses ebx esi edi,ctx,rootPath
 locals
  list dd ?
  count dd ?
  index dd ?
  artifact dd ?
  relative rb 1024
  hash rb 64
  wide rw 1100
  path rw 2048
  parent rw 2048
 endl
 stdcall JsonRoot,[ctx]
 stdcall JsonGet,[ctx],eax,lcKeyRuntime
 test eax,eax
 jz .good
 mov [list],eax
 stdcall PIsArray,[ctx],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[ctx],[list]
 cmp eax,16
 ja .bad
 mov [count],eax
 mov [index],0
 invoke lstrlenW,[rootPath]
 cmp eax,1000
 ja .bad
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .good
 stdcall JsonAt,[ctx],[list],eax
 mov [artifact],eax
 stdcall JsonGet,[ctx],eax,pPathKey
 stdcall VsString,[ctx],eax,addr relative,1024
 test eax,eax
 jz .bad
 stdcall PSafePath,addr relative
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],[artifact],pShaKey
 stdcall VsString,[ctx],eax,addr hash,64
 test eax,eax
 jz .bad
 stdcall PIsHash,addr hash
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,addr relative,-1,addr wide,1100
 test eax,eax
 jz .bad
 lea esi,[wide]
.slashes:
 mov ax,[esi]
 test ax,ax
 jz .path
 cmp ax,'/'
 jne .advance
 mov word [esi],92
.advance:
 add esi,2
 jmp .slashes
.path:
 invoke lstrcpyW,addr path,[rootPath]
 invoke lstrcatW,addr path,pLibrariesW
 invoke lstrcatW,addr path,addr wide
 invoke lstrcpyW,addr parent,addr path
 invoke lstrlenW,addr parent
 lea edi,[parent+eax*2]
.parent:
 cmp word [edi],92
 je .parentReady
 sub edi,2
 jmp .parent
.parentReady:
 mov word [edi],0
 stdcall VsDirectoryChain,addr parent,0
 test eax,eax
 jz .bad
 stdcall VsRegularFile,addr path
 test eax,eax
 jz .bad
 stdcall VerifySha1,addr path,addr hash
 test eax,eax
 jz .bad
 inc [index]
 jmp .loop
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Shared cache promotion never overwrites an existing library, even on failure.
proc LcPromoteArtifact uses ebx esi edi,ctx,artifact,copyNew
 locals
  rel rb 4096
  hash rb 64
  wide rw 2048
  source rw 2048
  dest rw 2048
  parent rw 2048
 endl
 stdcall JsonGet,[ctx],[artifact],pPathKey
 stdcall VsString,[ctx],eax,addr rel,4096
 test eax,eax
 jz .bad
 stdcall PSafePath,addr rel
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,addr rel,-1,addr wide,1100
 test eax,eax
 jz .bad
 lea esi,[wide]
.slashes:
 mov ax,[esi]
 test ax,ax
 jz .paths
 cmp ax,'/'
 jne .advance
 mov word [esi],92
.advance:
 add esi,2
 jmp .slashes
.paths:
 invoke lstrcpyW,addr source,lcScratchW
 invoke lstrcatW,addr source,pLibrariesW
 invoke lstrcatW,addr source,addr wide
 invoke lstrcpyW,addr dest,lcRootW
 invoke lstrcatW,addr dest,pLibrariesW
 invoke lstrcatW,addr dest,addr wide
 stdcall JsonGet,[ctx],[artifact],pShaKey
 stdcall VsString,[ctx],eax,addr hash,64
 test eax,eax
 jz .bad
 stdcall PIsHash,addr hash
 test eax,eax
 jz .bad
 invoke lstrcpyW,addr parent,addr dest
 invoke lstrlenW,addr parent
 lea edi,[parent+eax*2]
.parent:
 cmp word [edi],92
 je .parentReady
 sub edi,2
 jmp .parent
.parentReady:
 mov word [edi],0
 stdcall VsDirectoryChain,addr parent,1
 test eax,eax
 jz .bad
 cmp [copyNew],2
 je .seed
 invoke GetFileAttributesW,addr dest
 cmp eax,-1
 je .new
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 stdcall VerifySha1,addr dest,addr hash
 test eax,eax
 jz .conflict
 mov eax,1
 ret
.new:
 cmp [copyNew],0
 je .bad
 stdcall VsRegularFile,addr source
 test eax,eax
 jz .bad
 stdcall VerifySha1,addr source,addr hash
 test eax,eax
 jz .bad
 stdcall EnsureDirectory,addr parent
 test eax,eax
 jz .bad
 invoke CopyFileW,addr source,addr dest,1
 test eax,eax
 jz .bad
 stdcall VerifySha1,addr dest,addr hash
 ret
.seed:
 ; Seed scratch only from independently verified shared-cache bytes. Copies
 ; keep even an official installer from modifying the shared source via links.
 invoke GetFileAttributesW,addr dest
 cmp eax,-1
 je .seedMissing
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 stdcall VerifySha1,addr dest,addr hash
 test eax,eax
 jz .conflict
 invoke GetFileAttributesW,addr source
 cmp eax,-1
 je .seedCopy
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 stdcall VerifySha1,addr source,addr hash
 ret
.seedCopy:
 invoke lstrcpyW,addr parent,addr source
 invoke lstrlenW,addr parent
 lea edi,[parent+eax*2]
.seedParent:
 cmp word [edi],92
 je .seedReady
 sub edi,2
 jmp .seedParent
.seedReady:
 mov word [edi],0
 stdcall VsDirectoryChain,addr parent,1
 test eax,eax
 jz .bad
 stdcall EnsureDirectory,addr parent
 test eax,eax
 jz .bad
 invoke CopyFileW,addr dest,addr source,1
 ret
.seedMissing:
 mov eax,1
 ret
.conflict:
 mov [lcFailureReason],lcMsgConflict
 stdcall SetStatusA,lcMsgConflict
.bad:
 xor eax,eax
 ret
endp

; Validate the exact artifacts that the engine uses on this platform.
proc LcLibraries uses ebx esi edi,ctx,copyNew
 locals
  list dd ?
  count dd ?
  index dd ?
  lib dd ?
  downloads dd ?
  classifier rb 512
  key rb 512
 endl
 stdcall JsonRoot,[ctx]
 stdcall JsonGet,[ctx],eax,pLibsKey
 mov [list],eax
 stdcall PIsArray,[ctx],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[ctx],[list]
 cmp eax,4096
 ja .bad
 mov [count],eax
 mov [index],0
.loop:
 cmp [engineCancel],0
 jne .bad
 mov eax,[index]
 cmp eax,[count]
 jae .good
 stdcall JsonAt,[ctx],[list],eax
 mov [lib],eax
 stdcall EAllowed,[ctx],eax
 test eax,eax
 jz .next
 stdcall JsonGet,[ctx],[lib],pDownloadsKey
 mov [downloads],eax
 stdcall JsonGet,[ctx],eax,pArtifactKey
 test eax,eax
 jz .native
 stdcall LcPromoteArtifact,[ctx],eax,[copyNew]
 test eax,eax
 jz .bad
.native:
 stdcall JsonGet,[ctx],[lib],eKeyNatives
 stdcall JsonGet,[ctx],eax,eKeyWindows
 test eax,eax
 jz .next
 stdcall VsString,[ctx],eax,addr classifier,512
 test eax,eax
 jz .bad
 lea esi,[classifier]
 lea edi,[key]
.key:
 mov al,[esi]
 test al,al
 jz .keyEnd
 cmp dword [esi],'${ar'
 jne .char
 cmp dword [esi+4],'ch}'
 jne .char
 mov word [edi],'64'
 add edi,2
 add esi,7
 jmp .key
.char:
 mov [edi],al
 inc edi
 inc esi
 jmp .key
.keyEnd:
 mov byte [edi],0
 stdcall JsonGet,[ctx],[downloads],pClassifiersKey
 stdcall JsonGet,[ctx],eax,addr key
 test eax,eax
 jz .bad
 stdcall LcPromoteArtifact,[ctx],eax,[copyNew]
 test eax,eax
 jz .bad
.next:
 inc [index]
 jmp .loop
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Last metadata publication is a single same-volume rename. Historical backup
; names are CREATE_NEW and remain available if later changes or restores fail.
proc LcWriteNew uses ebx esi,path,text
 local file:DWORD,written:DWORD,length:DWORD
 invoke lstrlenA,[text]
 mov [length],eax
 invoke CreateFileW,[path],GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [file],eax
 invoke WriteFile,eax,[text],[length],addr written,0
 mov ebx,eax
 invoke CloseHandle,[file]
 test ebx,ebx
 jz .removeOwned
 mov eax,[written]
 cmp eax,[length]
 jne .removeOwned
 mov eax,1
 ret
.removeOwned:
 invoke DeleteFileW,[path]
.bad:
 xor eax,eax
 ret
endp

proc LcPublish uses ebx,text
 local wide[128]:WORD,currentHash[64]:BYTE,owned:DWORD
 mov [owned],0
 cmp [engineCancel],0
 jne .bad
 ; Refuse an externally edited source since the preparation started.
 stdcall PHashFile,lcSourceJsonW,addr currentHash
 test eax,eax
 jz .bad
 invoke lstrcmpiA,addr currentHash,lcOldHashA
 test eax,eax
 jnz .bad
 invoke MultiByteToWideChar,65001,8,lcHistoryLeafA,-1,addr wide,128
 invoke lstrcpyW,lcHistoryW,lcProfileW
 invoke lstrcatW,lcHistoryW,pSlashW
 invoke lstrcatW,lcHistoryW,addr wide
 invoke MultiByteToWideChar,65001,8,lcTagA,-1,addr wide,128
 invoke lstrcpyW,lcPendingW,lcProfileW
 invoke lstrcatW,lcPendingW,lcPendingPrefixW
 invoke lstrcatW,lcPendingW,addr wide
 invoke lstrcatW,lcPendingW,pJsonW
 ; CREATE_NEW leaves any foreign pending file untouched, including cleanup.
 stdcall LcWriteNew,lcPendingW,[text]
 test eax,eax
 jz .bad
 mov [owned],1
 invoke CopyFileW,lcSourceJsonW,lcHistoryW,1
 test eax,eax
 jz .clean
 cmp [engineCancel],0
 jne .clean
 ; The replacement JSON contains the unique backup's basename. Thus active
 ; metadata and its rollback pointer change together in this single rename.
 invoke MoveFileExW,lcPendingW,lcSourceJsonW,9
 test eax,eax
 jz .clean
 mov eax,1
 ret
.clean:
 cmp [owned],0
 je .bad
 invoke DeleteFileW,lcPendingW
.bad:
 xor eax,eax
 ret
endp
proc LoaderChangeApply uses ebx esi edi,rootPath,id,targetVersion
 locals
  baseText dd ?
  baseCtx dd ?
  childText dd ?
  childCtx dd ?
  merged dd ?
  mergedCtx dd ?
  stamped dd ?
  stampedCtx dd ?
  runtimeTable dd ?
  switched dd ?
  targetId rb 128
  version rb 128
  detected dd ?
  wide rw 128
  path rw 2048
  dir rw 2048
 endl
 mov [baseText],0
 mov [baseCtx],0
 mov [childText],0
 mov [childCtx],0
 mov [merged],0
 mov [mergedCtx],0
 mov [stamped],0
 mov [stampedCtx],0
 mov [runtimeTable],0
 mov [switched],0
 mov [lcResult],0
 mov [lcFailureReason],0
 stdcall LoaderSafeId,[targetVersion]
 test eax,eax
 jz .done
 invoke lstrcpyA,lcTargetA,[targetVersion]
 stdcall LoaderChangeLoad,[rootPath],[id]
 test eax,eax
 jz .done
 invoke lstrcmpA,lcTargetA,lcVersionA
 test eax,eax
 jz .done
 cmp [engineCancel],0
 jne .done
 stdcall SetStatusA,lcMsgPreparing
 stdcall PHashFile,lcSourceJsonW,lcOldHashA
 test eax,eax
 jz .done
 stdcall VerifySha1,lcSourceJarW,lcBaseHashA
 test eax,eax
 jz .done
 stdcall LcTag
 invoke lstrcpyW,lcSavedRootW,gameDir
 invoke lstrcpyW,lcSavedJavaW,javaPath
 mov [switched],1
 invoke lstrcpyW,gameDir,lcRootW
 stdcall LdProfilePaths,lcBaseA,addr dir,lcBaseJsonW
 test eax,eax
 jz .done
 stdcall VsDirectoryChain,addr dir,0
 test eax,eax
 jz .done
 stdcall LcPath,addr dir,lcBaseA,pJarW,lcBaseJarW
 stdcall VsRegularFile,lcBaseJsonW
 test eax,eax
 jz .done
 stdcall VsRegularFile,lcBaseJarW
 test eax,eax
 jz .done
 stdcall VerifySha1,lcBaseJarW,lcBaseHashA
 test eax,eax
 jz .done
 stdcall EReadFile,lcBaseJsonW
 test eax,eax
 jz .done
 mov [baseText],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [baseCtx],eax
 stdcall JsonRoot,eax
 mov ebx,eax
 stdcall JsonGet,[baseCtx],ebx,pInheritKey
 test eax,eax
 jnz .done
 stdcall JsonGet,[baseCtx],ebx,pVersionKey
 stdcall VsString,[baseCtx],eax,addr version,128
 test eax,eax
 jz .done
 invoke lstrcmpA,addr version,lcBaseA
 test eax,eax
 jnz .done
 stdcall JsonGet,[baseCtx],ebx,pDownloadsKey
 stdcall JsonGet,[baseCtx],eax,pClientKey
 stdcall JsonGet,[baseCtx],eax,pShaKey
 stdcall VsString,[baseCtx],eax,addr version,64
 test eax,eax
 jz .done
 invoke lstrcmpiA,addr version,lcBaseHashA
 test eax,eax
 jnz .done
 stdcall LcCreateScratch
 test eax,eax
 jz .done
 invoke lstrcpyW,gameDir,lcScratchW
 stdcall LdProfilePaths,lcBaseA,addr dir,addr path
 test eax,eax
 jz .done
 stdcall EnsureDirectory,addr dir
 test eax,eax
 jz .done
 invoke CopyFileW,lcBaseJsonW,addr path,1
 test eax,eax
 jz .done
 stdcall LcPath,addr dir,lcBaseA,pJarW,addr path
 invoke CopyFileW,lcBaseJarW,addr path,1
 test eax,eax
 jz .done
 cmp word [lcRecord+VS_JAVA],0
 je .javaReady
 invoke lstrcpyW,javaPath,lcRecord+VS_JAVA
.javaReady:
 stdcall LoaderInstall,lcBaseA,[lcKind],lcTargetA,addr targetId
 test eax,eax
 jz .done
 stdcall LdProfilePaths,addr targetId,addr dir,addr path
 test eax,eax
 jz .done
 stdcall VsDirectoryChain,addr dir,0
 test eax,eax
 jz .done
 stdcall VsRegularFile,addr path
 test eax,eax
 jz .done
 stdcall EReadFile,addr path
 test eax,eax
 jz .done
 mov [childText],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [childCtx],eax
 stdcall LdValidateProfile,eax,lcBaseA,addr version
 test eax,eax
 jz .done
 invoke lstrcmpA,addr version,addr targetId
 test eax,eax
 jnz .done
 stdcall LcReadVersion,[childCtx],lcBaseA,addr version,addr detected
 test eax,eax
 jz .done
 mov eax,[detected]
 cmp eax,[lcKind]
 jne .done
 invoke lstrcmpA,addr version,lcTargetA
 test eax,eax
 jnz .done
 cmp [lcKind],3
 jne .merge
 stdcall LcBuildRuntimeTable,[childCtx]
 test eax,eax
 jz .done
 mov [runtimeTable],eax
.merge:
 ; Merge the raw installer child once, so parent arguments never accumulate.
 stdcall ProfileMerge,[baseCtx],[childCtx],lcSourceA
 test eax,eax
 jz .done
 mov [merged],eax
 cinvoke wsprintfA,lcNativeNameA,lcNativeFormat,lcTagA
 stdcall JsonParse,[merged]
 test eax,eax
 jz .done
 mov [mergedCtx],eax
 stdcall LcCheckProfile,eax,lcSourceA,lcBaseA,addr version
 test eax,eax
 jz .done
 invoke lstrcmpiA,addr version,lcBaseHashA
 test eax,eax
 jnz .done
 stdcall LcStamp,[mergedCtx],lcNativeNameA,[runtimeTable]
 test eax,eax
 jz .done
 mov [stamped],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [stampedCtx],eax
 ; EInstallLibraries is confined to scratch, including native extraction.
 stdcall ECleanup
 mov eax,[mergedCtx]
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall LcLibraries,[mergedCtx],2
 test eax,eax
 jz .releaseEngine
 stdcall LdProfilePaths,lcSourceA,addr dir,addr path
 stdcall EnsureDirectory,addr dir
 invoke lstrcpyW,eNativePath,addr dir
 invoke lstrcatW,eNativePath,eNativesSuffix
 stdcall EnsureDirectory,eNativePath
 test eax,eax
 jz .releaseEngine
 stdcall EInstallLibraries
 test eax,eax
 jz .releaseEngine
 stdcall VsDirectoryChain,eNativePath,0
 test eax,eax
 jz .releaseEngine
 invoke lstrcpyW,lcNativeStageW,eNativePath
 mov [eMeta],0
 mov [eRoot],0
 stdcall LcLibraries,[mergedCtx],1
 test eax,eax
 jz .done
 stdcall LcPromoteRuntime,[stampedCtx]
 test eax,eax
 jz .done
 stdcall LcVerifyRuntimeArtifacts,[stampedCtx],lcRootW
 test eax,eax
 jz .done
 invoke MultiByteToWideChar,65001,8,lcNativeNameA,-1,addr wide,128
 invoke lstrcpyW,lcNativeTargetW,lcProfileW
 invoke lstrcatW,lcNativeTargetW,pSlashW
 invoke lstrcatW,lcNativeTargetW,addr wide
 cmp [engineCancel],0
 jne .done
 invoke MoveFileExW,lcNativeStageW,lcNativeTargetW,8
 test eax,eax
 jz .done
 stdcall LcPublish,[stamped]
 mov [lcResult],eax
 jmp .done
.releaseEngine:
 mov [eMeta],0
 mov [eRoot],0
.done:
 cmp [switched],0
 je .free
 invoke lstrcpyW,gameDir,lcSavedRootW
 invoke lstrcpyW,javaPath,lcSavedJavaW
.free:
 stdcall JsonFree,[baseCtx]
 stdcall JsonFree,[childCtx]
 stdcall JsonFree,[mergedCtx]
 stdcall JsonFree,[stampedCtx]
 stdcall FreeMem,[baseText]
 stdcall FreeMem,[childText]
 stdcall FreeMem,[merged]
 stdcall FreeMem,[stamped]
 stdcall FreeMem,[runtimeTable]
 cmp [lcResult],1
 je .success
 mov eax,lcMsgFailed
 cmp [lcFailureReason],0
 je .failureStatus
 mov eax,[lcFailureReason]
.failureStatus:
 cmp [engineCancel],0
 je .status
 mov eax,lcMsgCancelled
 jmp .status
.success:
 mov eax,lcMsgChanged
.status:
 stdcall SetStatusA,eax
 mov eax,[lcResult]
 ret
endp

proc LoaderChangeRestore uses ebx,rootPath,id
 locals
  text dd ?
  ctx dd ?
  stamped dd ?
  root dd ?
  kind dd ?
  version rb 128
  wide rw 128
  path rw 2048
 endl
 mov [text],0
 mov [ctx],0
 mov [stamped],0
 mov [lcResult],0
 mov [lcFailureReason],0
 stdcall LoaderChangeLoad,[rootPath],[id]
 test eax,eax
 jz .done
 cmp [lcBackupReady],1
 jne .done
 stdcall SetStatusA,lcMsgPreparing
 stdcall VerifySha1,lcSourceJarW,lcBaseHashA
 test eax,eax
 jz .done
 stdcall EReadFile,lcBackupW
 test eax,eax
 jz .done
 mov [text],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [ctx],eax
 stdcall LcCheckProfile,eax,lcSourceA,lcBaseA,addr version
 test eax,eax
 jz .done
 invoke lstrcmpiA,addr version,lcBaseHashA
 test eax,eax
 jnz .done
 stdcall LcReadVersion,[ctx],lcBaseA,addr version,addr kind
 test eax,eax
 jz .done
 mov eax,[kind]
 cmp eax,[lcKind]
 jne .done
 stdcall LcLibraries,[ctx],0
 test eax,eax
 jz .done
 stdcall LcVerifyRuntimeArtifacts,[ctx],lcRootW
 test eax,eax
 jz .done
 stdcall JsonRoot,[ctx]
 mov [root],eax
 stdcall JsonGet,[ctx],eax,lcKeyNative
 test eax,eax
 jz .defaultNative
 stdcall VsString,[ctx],eax,addr version,128
 test eax,eax
 jz .done
 stdcall ENativeDirValid,addr version
 test eax,eax
 jz .done
 invoke MultiByteToWideChar,65001,8,addr version,-1,addr wide,128
 invoke lstrcpyW,addr path,lcProfileW
 invoke lstrcatW,addr path,pSlashW
 invoke lstrcatW,addr path,addr wide
 jmp .native
.defaultNative:
 invoke lstrcpyW,addr path,lcProfileW
 invoke lstrcatW,addr path,eNativesSuffix
.native:
 stdcall VsDirectoryChain,addr path,0
 test eax,eax
 jz .done
 stdcall PHashFile,lcSourceJsonW,lcOldHashA
 test eax,eax
 jz .done
 stdcall LcTag
 stdcall LcStamp,[ctx],0,0
 test eax,eax
 jz .done
 mov [stamped],eax
 stdcall LcPublish,[stamped]
 mov [lcResult],eax
.done:
 stdcall JsonFree,[ctx]
 stdcall FreeMem,[text]
 stdcall FreeMem,[stamped]
 mov eax,lcMsgBackup
 cmp [lcResult],0
 jne .restored
 cmp [engineCancel],0
 je .status
 mov eax,lcMsgCancelled
 jmp .status
.restored:
 mov eax,lcMsgRestored
.status:
 stdcall SetStatusA,eax
 mov eax,[lcResult]
 ret
endp



