; Opt-in real HTTP/official installer probe. Never launches Minecraft.
; OFFSET_LC_GAME, OFFSET_LC_ID, OFFSET_LC_JAVA and optional TARGET/ACTION.
; ACTION = load / fetch / apply (default) / restore. Use an isolated game root.
format PE console 6.0
entry start
stack 200000h,80000h
include 'win32ax.inc'
include '..\src\version_types.inc'
WM_APP=8000h
OFFSET_VERSION_SETTINGS=1
section '.data' data readable writeable
hWnd dd 0
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'LiveProbe',0,54 dup(0)
selectedVersion db 'global-selection-sentinel',0,102 dup(0)
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
envRoot du 'OFFSET_LC_GAME',0
envId du 'OFFSET_LC_ID',0
envJava du 'OFFSET_LC_JAVA',0
envTarget du 'OFFSET_LC_TARGET',0
envAction du 'OFFSET_LC_ACTION',0
sentinelRoot du 'C:\OFFSET-loader-change-sentinel',0
sentinelSelection db 'global-selection-sentinel',0
actionLoad db 'load',0
actionFetch db 'fetch',0
actionRestore db 'restore',0
defaultAction db 'apply',0
newline db 13,10,0
fmtPair db '%s=%s',0
fmtNum db '%s=%u',0
kAction db 'action',0
kKind db 'kind',0
kBeforeVersion db 'version_before',0
kAfterVersion db 'version_after',0
kBase db 'base',0
kJsonBefore db 'json_sha1_before',0
kJsonAfter db 'json_sha1_after',0
kJarBefore db 'jar_sha1_before',0
kJarAfter db 'jar_sha1_after',0
kGlobalRoot db 'global_root_preserved',0
kGlobalJava db 'global_java_preserved',0
kGlobalSelection db 'global_selection_preserved',0
kGlobalMemory db 'global_memory_preserved',0
kResult db 'result',0
kStep db 'step',0
kNative db 'native_path',0
kIsolate db 'isolate',0
kData db 'data_path',0
probeRoot rw 1024
probeJava rw 1024
probeWide rw 2048
probeId rb 128
probeTarget rb 128
probeAction rb 32
probeBefore rb 128
probeJsonBefore rb 64
probeJarBefore rb 64
probeHash rb 64
probeLine rb 8192
probeUtf8 rb 8192
probeResult dd 0
probeStep dd 1
probeFailed dd 0
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,envRoot,probeRoot,1024
 test eax,eax
 jz failed
 cmp eax,1024
 jae failed
 stdcall ReadUtf8Env,envId,probeId,128
 test eax,eax
 jz failed
 stdcall ReadUtf8Env,envTarget,probeTarget,128
 stdcall ReadUtf8Env,envAction,probeAction,32
 test eax,eax
 jnz .actionReady
 invoke lstrcpyA,probeAction,defaultAction
.actionReady:
 invoke GetEnvironmentVariableW,envJava,javaPath,1024
 test eax,eax
 jz failed
 cmp eax,1024
 jae failed
 invoke lstrcpyW,probeJava,javaPath
 invoke lstrcpyW,gameDir,sentinelRoot
 stdcall Pair,kAction,probeAction
 stdcall LoaderChangeLoad,probeRoot,probeId
 cmp eax,1
 jne failed
 inc [probeStep]
 invoke lstrcpyA,probeBefore,lcVersionA
 stdcall Pair,kBeforeVersion,probeBefore
 stdcall Pair,kBase,lcBaseA
 stdcall Number,kKind,[lcKind]
 stdcall PHashFile,lcSourceJsonW,probeJsonBefore
 test eax,eax
 jz failed
 stdcall Pair,kJsonBefore,probeJsonBefore
 stdcall PHashFile,lcSourceJarW,probeJarBefore
 test eax,eax
 jz failed
 stdcall Pair,kJarBefore,probeJarBefore
 inc [probeStep]
 invoke lstrcmpA,probeAction,actionLoad
 test eax,eax
 jz .loaded
 invoke lstrcmpA,probeAction,actionFetch
 test eax,eax
 jz .fetch
 invoke lstrcmpA,probeAction,actionRestore
 test eax,eax
 jz .restore
 invoke lstrcmpA,probeAction,defaultAction
 test eax,eax
 jnz failed
 stdcall LoaderChangeApply,probeRoot,probeId,probeTarget
 jmp .result
.restore:
 stdcall LoaderChangeRestore,probeRoot,probeId
 jmp .result
.fetch:
 stdcall LoaderChangeFetch,probeRoot,probeId
 jmp .result
.loaded:
 mov eax,1
.result:
 mov [probeResult],eax
 stdcall Number,kResult,eax
 inc [probeStep]
 invoke lstrcmpiW,gameDir,sentinelRoot
 test eax,eax
 setz al
 movzx eax,al
 stdcall AssertOne,kGlobalRoot,eax
 invoke lstrcmpiW,javaPath,probeJava
 test eax,eax
 setz al
 movzx eax,al
 stdcall AssertOne,kGlobalJava,eax
 invoke lstrcmpA,selectedVersion,sentinelSelection
 test eax,eax
 setz al
 movzx eax,al
 stdcall AssertOne,kGlobalSelection,eax
 xor eax,eax
 cmp [memoryMb],4096
 sete al
 stdcall AssertOne,kGlobalMemory,eax
 stdcall LoaderChangeLoad,probeRoot,probeId
 cmp eax,1
 jne failed
 stdcall Pair,kAfterVersion,lcVersionA
 stdcall PHashFile,lcSourceJsonW,probeHash
 test eax,eax
 jz failed
 stdcall Pair,kJsonAfter,probeHash
 stdcall PHashFile,lcSourceJarW,probeHash
 test eax,eax
 jz failed
 stdcall Pair,kJarAfter,probeHash
 invoke lstrcmpiA,probeHash,probeJarBefore
 test eax,eax
 jnz failed
 inc [probeStep]
 ; Resolve the engine's real native path without building or running a JVM.
 invoke lstrcpyW,gameDir,probeRoot
 invoke lstrcpyA,selectedVersion,probeId
 stdcall EValidateVersion
 test eax,eax
 jz failed
 stdcall EBuildPaths
 stdcall ELoadMetadata
 test eax,eax
 jz failed
 stdcall VsDirectoryChain,eNativePath,0
 test eax,eax
 jz failed
 invoke WideCharToMultiByte,65001,0,eNativePath,-1,probeUtf8,8192,0,0
 test eax,eax
 jz failed
 stdcall Pair,kNative,probeUtf8
 stdcall Number,kIsolate,dword [lcRecord+VS_ISOLATE]
 invoke WideCharToMultiByte,65001,0,lcRecord+VS_DATA,-1,probeUtf8,8192,0,0
 test eax,eax
 jz failed
 stdcall Pair,kData,probeUtf8
 stdcall ECleanup
 cmp [probeResult],1
 jne failed
 cmp [probeFailed],0
 jne failed
 invoke ExitProcess,0
failed:
 stdcall Number,kStep,[probeStep]
 invoke ExitProcess,[probeStep]

proc ReadUtf8Env,name,dest,capacity
 invoke GetEnvironmentVariableW,[name],probeWide,2048
 test eax,eax
 jz .bad
 cmp eax,2048
 jae .bad
 invoke WideCharToMultiByte,65001,0,probeWide,-1,[dest],[capacity],0,0
 ret
.bad:
 xor eax,eax
 ret
endp
proc Pair,key,value
 cinvoke wsprintfA,probeLine,fmtPair,[key],[value]
 stdcall SetStatusA,probeLine
 ret
endp
proc Number,key,value
 cinvoke wsprintfA,probeLine,fmtNum,[key],[value]
 stdcall SetStatusA,probeLine
 ret
endp
proc AssertOne,key,value
 cmp [value],1
 je .good
 mov [probeFailed],1
.good:
 stdcall Number,[key],[value]
 ret
endp
proc SetStatusA uses ebx,text
 local written:DWORD
 invoke lstrlenA,[text]
 mov ebx,eax
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,[text],ebx,addr written,0
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,newline,2,addr written,0
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\version_settings.inc'
include '..\src\profiles.inc'
include '..\src\loaders.inc'
include '..\src\loader_change.inc'
include '..\src\imports.inc'
