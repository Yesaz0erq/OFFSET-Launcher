; Per-version action callbacks: explicit selection and transactional repair scope.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
VS_SIZE=12640
VS_MEMORY=2128
VS_JAVA=80
WM_FINISHED=8002h
section '.data' data readable writeable
hWnd dd 0
hGameDir dd 0
hVersion dd 0
busy dd 0
currentPage dd 1
modTab dd 0
memoryMb dd 4096
gameDir rw 1024
javaPath rw 1024
selectedVersion rb 128
vmRootW rw 1024
vmIdA rb 128
statusText rw 1024
sVCurrentSet du 'selected',0
eStatusInstalled db 'installed',0
oldRoot du 'C:\fixture\shared',0
targetRoot du 'C:\fixture\other',0
oldId db '1.21.1',0
targetId db 'fabric-loader-test',0
step dd 1
pullValid dd 1
profileValid dd 1
saveValid dd 1
installResult dd 1
saveCalls dd 0
installCalls dd 0
postCalls dd 0
postWrong dd 0
installWrong dd 0
workerStarted dd 0
testPostMessageApi dd TestPostMessage

section '.text' code readable executable
start:
 stdcall ResetState
 ; Merely asking for overview settings leaves the current launch selection.
 stdcall RefreshHomeVersionSettings
 stdcall CheckOriginal
 test eax,eax
 jz failed
 inc [step]
 ; Explicit use selects the target. It does not start a worker by itself.
 stdcall VersionPageUse
 cmp eax,1
 jne failed
 invoke lstrcmpiW,gameDir,targetRoot
 test eax,eax
 jnz failed
 invoke lstrcmpA,selectedVersion,targetId
 test eax,eax
 jnz failed
 cmp [saveCalls],1
 jne failed
 cmp [workerStarted],0
 jne failed
 inc [step]
 ; Failed persistence restores the original root and selected ID.
 stdcall ResetState
 mov [saveValid],0
 stdcall VersionPageUse
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 test eax,eax
 jz failed
 inc [step]
 ; Invalid metadata and busy state cannot silently change the selection.
 stdcall ResetState
 mov [profileValid],0
 stdcall VersionPageUse
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 test eax,eax
 jz failed
 cmp [saveCalls],0
 jne failed
 mov [profileValid],1
 mov [busy],1
 stdcall VersionPageUse
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 test eax,eax
 jz failed
 inc [step]
 ; A successful repair observes the VM context only inside EInstallCore.
 stdcall ResetState
 stdcall VersionPageRepairWorker,0
 stdcall CheckOriginal
 test eax,eax
 jz failed
 cmp [vmActionResult],1
 jne failed
 cmp [installCalls],1
 jne failed
 cmp [installWrong],0
 jne failed
 cmp [postCalls],1
 jne failed
 cmp [postWrong],0
 jne failed
 inc [step]
 ; Error return preserves context and posts exactly one completion as well.
 stdcall ResetState
 mov [installResult],0
 stdcall VersionPageRepairWorker,0
 stdcall CheckOriginal
 test eax,eax
 jz failed
 cmp [vmActionResult],0
 jne failed
 cmp [postCalls],1
 jne failed
 cmp [installWrong],0
 jne failed
 cmp [postWrong],0
 jne failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[step]

proc ResetState
 invoke lstrcpyW,gameDir,oldRoot
 invoke lstrcpyW,vmRootW,targetRoot
 invoke lstrcpyA,selectedVersion,oldId
 invoke lstrcpyA,vmIdA,targetId
 mov [busy],0
 mov [pullValid],1
 mov [profileValid],1
 mov [saveValid],1
 mov [installResult],1
 mov [saveCalls],0
 mov [installCalls],0
 mov [postCalls],0
 mov [postWrong],0
 mov [installWrong],0
 mov [workerStarted],0
 ret
endp
proc CheckOriginal
 invoke lstrcmpiW,gameDir,oldRoot
 test eax,eax
 jnz .bad
 invoke lstrcmpA,selectedVersion,oldId
 test eax,eax
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PullSettings
 mov eax,[pullValid]
 ret
endp
proc SaveSettings
 inc [saveCalls]
 mov eax,[saveValid]
 ret
endp
proc VsLoad,root,id
 mov eax,[profileValid]
 ret
endp
proc VsRead,root,id,record,installed
 xor eax,eax
 ret
endp
proc EngineScanVersions,combo
 xor eax,eax
 ret
endp
proc EInstallCore
 inc [installCalls]
 invoke lstrcmpiW,gameDir,targetRoot
 test eax,eax
 jnz .wrong
 invoke lstrcmpA,selectedVersion,targetId
 test eax,eax
 jz .result
.wrong:
 mov [installWrong],1
.result:
 mov eax,[installResult]
 ret
endp
proc ECleanup
 ret
endp
proc SetStatusA,text
 ret
endp
proc StartWorker,callback
 mov eax,[callback]
 mov [workerStarted],eax
 ret
endp
proc EngineLaunch,param
 ret
endp
proc GoPage,page
 mov eax,[page]
 mov [currentPage],eax
 ret
endp
proc VersionPageOpen,root,id
 ret
endp
proc TestPostMessage,window,message,wparam,lparam
 inc [postCalls]
 cmp [message],WM_FINISHED
 je .okay
 mov [postWrong],1
.okay:
 mov eax,1
 ret
endp
define PostMessageW testPostMessageApi
include '..\src\version_actions.inc'
restore PostMessageW
include '..\src\imports.inc'
