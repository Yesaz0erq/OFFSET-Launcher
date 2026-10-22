; UI routing: show only a new crash, preserve selection, handle absent logs.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
section '.data' data readable writeable
hWnd dd 0
busy dd 0
currentPage dd 0
vmActive dd 1
detailActive dd 1
launchLogSerial dd 0
launchLogReady dd 0
launchCrashReady dd 0
launchLogSaveError dd 0
launchLogPathW rw 2048
launchReportPathW rw 2048
launchLogDataDirW rw 2048
statusText rw 1024
editClass du 'EDIT',0
emptyW dw 0
openVerb du 'open',0
gameDir du 'original-root',0
selectedVersion db 'original-version',0
expectedRoot du 'original-root',0
expectedVersion db 'original-version',0
fixtureOutput db 'stdout + stderr',10,'second line',13,10,0
expectedOutput du 'stdout + stderr',13,10,'second line',13,10,0
fixtureReport db 'Crash: exit code 23',13,10,0
step dd 1
readCalls dd 0
resultText rw 256
include '..\src\strings.inc'
section '.text' code readable executable
start:
 stdcall CreateLogControls
 test eax,eax
 jz failed
 stdcall LogRefresh
 invoke GetWindowTextW,[hLogBody],resultText,256
 invoke lstrcmpiW,resultText,sLogEmpty
 test eax,eax
 jnz failed
 inc [step]
 mov [logUiTab],1
 stdcall LogRefresh
 invoke GetWindowTextW,[hLogBody],resultText,256
 invoke lstrcmpiW,resultText,sLogNoCrash
 test eax,eax
 jnz failed
 inc [step]
 ; New failed run routes to the report, clears detail ownership only.
 mov [launchLogSerial],1
 mov [launchLogReady],1
 mov [launchCrashReady],1
 stdcall LogFinishUI
 cmp [currentPage],7
 jne failed
 cmp [logUiSeenSerial],1
 jne failed
 cmp [vmActive],0
 jne failed
 cmp [detailActive],0
 jne failed
 cmp [logUiTab],1
 jne failed
 invoke GetWindowTextW,[hLogBody],resultText,256
 cmp word [resultText],'C'
 jne failed
 inc [step]
 ; A later download must not reopen the old crash report.
 mov [currentPage],4
 stdcall LogFinishUI
 cmp [currentPage],4
 jne failed
 ; New successful run also leaves the current page alone.
 mov [launchLogSerial],2
 mov [launchCrashReady],0
 stdcall LogFinishUI
 cmp [currentPage],4
 jne failed
 cmp [logUiSeenSerial],2
 jne failed
 stdcall LogExport
 invoke lstrcmpiW,statusText,sLogNoCrash
 test eax,eax
 jnz failed
 inc [step]
 ; Busy UI neither reads a file being written nor changes report tabs.
 mov eax,[readCalls]
 mov ebx,eax
 mov [busy],1
 stdcall LogRefresh
 stdcall ClickLogs,300,140
 cmp [logUiTab],1
 jne failed
 cmp ebx,[readCalls]
 jne failed
 mov [busy],0
 inc [step]
 mov [currentPage],7
 stdcall ClickLogs,300,140
 cmp [logUiTab],0
 jne failed
 invoke GetWindowTextW,[hLogBody],resultText,256
 invoke lstrcmpiW,resultText,expectedOutput
 test eax,eax
 jnz failed
 invoke lstrcmpiW,gameDir,expectedRoot
 test eax,eax
 jnz failed
 invoke lstrcmpA,selectedVersion,expectedVersion
 test eax,eax
 jnz failed
 invoke DestroyWindow,[hLogBody]
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[step]
proc NewControl,cls,style,id,x,y,w,h
 mov eax,[style]
 or eax,WS_POPUP
 invoke CreateWindowExW,0,[cls],emptyW,eax,0,0,[w],[h],0,0,0,0
 ret
endp
proc LaunchLogRead,kind,destination,capacity
 inc [readCalls]
 cmp [launchLogReady],0
 je .empty
 mov eax,fixtureOutput
 cmp [kind],0
 je .copy
 cmp [launchCrashReady],0
 je .empty
 mov eax,fixtureReport
.copy:
 invoke lstrcpyA,[destination],eax
 invoke lstrlenA,[destination]
 ret
.empty:
 xor eax,eax
 ret
endp
proc LaunchLogExportKindTo,kind,destination,replaceAllowed
 xor eax,eax
 ret
endp
proc LaunchLogDiscover,pDataDir
 xor eax,eax
 ret
endp
proc VsResolveData,pGameRoot,pVersionId,pOutput,cchOutput
 xor eax,eax
 ret
endp
proc StartTransition
 ret
endp
proc SyncPage
 stdcall ShowLogControls
 ret
endp
include '..\src\logs_ui.inc'
include '..\src\imports.inc'
