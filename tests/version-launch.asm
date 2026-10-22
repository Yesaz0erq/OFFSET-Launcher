; Real JVM launches verify settings application, isolation and restoration.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
WM_APP=8000h
OFFSET_VERSION_SETTINGS=1
section '.data' data readable writeable
hWnd dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion rb 128
memoryMb dd 2048
onlineMode dd 0
authReady dd 0
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
step dd 1
testGameEnv du 'OFFSET_TEST_GAME',0
testJavaEnv du 'OFFSET_TEST_JAVA',0
testJava rw 1024
testOriginalJava du 'Z:\not-a-java-runtime\java.exe',0
testShared db 'probe-shared',0
testIsolated db 'probe-isolated',0
testDraft du 'Untouched draft',0
testClientSuffix du '\probe-isolated.json',0
testStatus db 13,10,0
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,testGameEnv,gameDir,1024
 test eax,eax
 jz failed
 invoke GetEnvironmentVariableW,testJavaEnv,testJava,1024
 test eax,eax
 jz failed
 invoke lstrcpyW,javaPath,testOriginalJava
 inc [step]
 stdcall VsLoad,gameDir,testShared
 test eax,eax
 jz failed
 invoke lstrcpyW,vsJavaW,testJava
 mov [vsMemory],768
 mov [vsIsolate],0
 stdcall VsSave
 test eax,eax
 jz failed
 inc [step]
 stdcall VsLoad,gameDir,testIsolated
 test eax,eax
 jz failed
 invoke lstrcpyW,vsJavaW,testJava
 mov [vsMemory],1024
 mov [vsIsolate],1
 stdcall VsSave
 test eax,eax
 jz failed
 invoke lstrcpyW,vsNameW,testDraft
 inc [step]
 invoke lstrcpyA,selectedVersion,testShared
 stdcall EngineLaunch,0
 cmp eax,1
 jne failed
 stdcall CheckRestored
 test eax,eax
 jz failed
 inc [step]
 invoke lstrcpyA,selectedVersion,testIsolated
 stdcall EngineLaunch,0
 cmp eax,1
 jne failed
 stdcall CheckRestored
 test eax,eax
 jz failed
 inc [step]
 mov [engineCancel],1
 stdcall EngineLaunch,0
 test eax,eax
 jnz failed
 stdcall CheckRestored
 test eax,eax
 jz failed
 mov [engineCancel],0
 inc [step]
 ; An existing ordinary file is a valid stored path but cannot start a JVM.
 ; Failure after overrides were applied must also restore shared preferences.
 invoke lstrcpyW,vsJavaW,vsProfileDirW
 invoke lstrcatW,vsJavaW,testClientSuffix
 stdcall VsSave
 test eax,eax
 jz failed
 stdcall EngineLaunch,0
 test eax,eax
 jnz failed
 stdcall CheckRestored
 test eax,eax
 jz failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[step]
proc CheckRestored
 cmp [memoryMb],2048
 jne .bad
 cmp [eRuntimeIsolated],0
 jne .bad
 invoke lstrcmpiW,javaPath,testOriginalJava
 test eax,eax
 jnz .bad
 invoke lstrcmpiW,eRuntimeDirW,gameDir
 test eax,eax
 jnz .bad
 invoke lstrcmpiW,vsNameW,testDraft
 test eax,eax
 jnz .bad
 invoke lstrcmpA,vsIdA,testIsolated
 test eax,eax
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc SetStatusA uses ebx,text
 local written:DWORD
 invoke GetStdHandle,-11
 mov ebx,eax
 invoke lstrlenA,[text]
 invoke WriteFile,ebx,[text],eax,addr written,0
 invoke WriteFile,ebx,testStatus,2,addr written,0
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\version_settings.inc'
include '..\src\imports.inc'
