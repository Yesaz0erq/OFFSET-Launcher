; Actual saved files -> discovery -> both reads -> native EDIT, without mocks.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
busy dd 0
currentPage dd 7
vmActive dd 0
detailActive dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion db 'probe',0,122 dup(0)
memoryMb dd 768
onlineMode dd 0
clientId rb 128
authReady dd 0
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
curseApiKey rb 256
statusText rw 1024
editClass du 'EDIT',0
emptyW dw 0
openVerb du 'open',0
fixtureRootEnv du 'OFFSET_LOG_DISCOVER_ROOT',0
fixtureRoot rw 1024
resultText rw 64
directOutput rb 262145
stage dd 1
include '..\src\strings.inc'
section '.text' code readable executable
start:
 invoke GetEnvironmentVariableW,fixtureRootEnv,fixtureRoot,1024
 test eax,eax
 jz failed
 invoke lstrcpyW,gameDir,fixtureRoot
 stdcall EngineInit
 stdcall CreateLogControls
 test eax,eax
 jz failed
 stdcall ShowLogControls
 cmp [launchLogReady],1
 jne failed
 inc [stage]
 invoke GetWindowTextW,[hLogBody],resultText,64
 cmp word [resultText],'O'
 jne failed
 cmp word [resultText+2],'F'
 jne failed
 inc [stage]
 stdcall LaunchLogRead,0,directOutput,262145
 cmp eax,16
 jb failed
 cmp dword [directOutput],'OFFS'
 jne failed
 inc [stage]
 mov [logUiTab],1
 stdcall LogRefresh
 invoke GetWindowTextW,[hLogBody],resultText,64
 cmp word [resultText],'O'
 jne failed
 stdcall LaunchLogRead,1,directOutput,262145
 cmp eax,16
 jb failed
 cmp dword [directOutput],'OFFS'
 jne failed
 inc [stage]
 mov [logUiTab],0
 stdcall LogRefresh
 invoke GetWindowTextW,[hLogBody],resultText,64
 cmp word [resultText],'O'
 jne failed
 invoke DestroyWindow,[hLogBody]
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[stage]
proc NewControl,cls,style,id,x,y,w,h
 mov eax,[style]
 or eax,WS_POPUP
 invoke CreateWindowExW,0,[cls],emptyW,eax,0,0,[w],[h],0,0,0,0
 ret
endp
proc VsResolveData,root,id,destination,capacity
 invoke lstrcpyW,[destination],fixtureRoot
 mov eax,1
 ret
endp
proc StartTransition
 ret
endp
proc SyncPage
 stdcall ShowLogControls
 ret
endp
proc SetStatusA,text
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\launch_log.inc'
include '..\src\logs_ui.inc'
include '..\src\imports.inc'
