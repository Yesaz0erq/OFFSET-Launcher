; Native list and page routing regression; transport and installation are stubs.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\version_types.inc'
WM_FINISHED=8002h
C_MUTED=0
C_INK=0
A_SECOND=0
section '.data' data readable writeable
hWnd dd 0
busy dd 0
currentPage dd 1
vmActive dd 1
vmTab dd 3
contentX dd 0
dpi dd 96
fSmall dd 0
fBody dd 0
fMedium dd 0
vsLoaderKind dd 1
vmRootW du 'fixture-root',0,1011 dup(0)
vmIdA db 'profile-one',0,116 dup(0)
vmBaseW du '1.21.1',0
listClass du 'LISTBOX',0
emptyW dw 0
fixtureRootW du 'fixture-root',0
fixtureOtherW du 'other-root',0
fixtureIdA db 'profile-one',0
fixtureOtherIdA db 'profile-two',0
fixtureBaseA db '1.21.1',0
fixtureBaseOtherA db '1.20.1',0
fixtureCurrentA db '0.16.0',0
fixtureUpgradeA db '0.17.0',0
fixtureDowngradeA db '0.15.0',0
fixtureMutatedA db '99.99.99',0
fixtureUpgradeW du '0.17.0',0
fixtureDowngradeW du '0.15.0',0
lcKind dd 1
lcBackupReady dd 0
lcBaseA rb 128
lcVersionA rb 128
lcResult dd 0
loaderVersionCount dd 0
loaderVersions rb VL_SIZE*VL_CAPACITY
vUiFabric du 'Fabric',0
vUiForge du 'Forge',0
vUiNeoForge du 'NeoForge',0
vUiQuilt du 'Quilt',0
vUiNames dd sVVanilla,vUiFabric,vUiForge,vUiNeoForge,vUiQuilt
loadCalls dd 0
fetchCalls dd 0
applyCalls dd 0
restoreCalls dd 0
refreshCalls dd 0
workerCalls dd 0
postCalls dd 0
lastWorker dd 0
lastAction dd 0
startReject dd 0
loadReject dd 0
fetchReject dd 0
lastRootW rw 1024
lastIdA rb 128
lastTargetA rb 128
textW rw 256
rect RECT
postPointer dd FixturePost
testStep dd 1
failureFormat db 'loader-change-ui step %u failed',13,10,0
include '..\src\strings.inc'
section '.text' code readable executable
start:
 invoke lstrcpyA,lcBaseA,fixtureBaseA
 invoke lstrcpyA,lcVersionA,fixtureCurrentA
 stdcall VmLoaderCreate
 test eax,eax
 jz failed
 ; Opening and repainting a loader page perform no catalogue request.
 stdcall VmLoaderOpen
 cmp [vmLoaderContextReady],1
 jne failed
 cmp [vmLoaderKind],1
 jne failed
 cmp [workerCalls],0
 jne failed
 cmp [fetchCalls],0
 jne failed
 stdcall VmLoaderShow
 invoke GetWindowLongW,[hVmLoaderList],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 inc [testStep]
 ; A refresh queues a single scoped worker and posts one completion.
 stdcall VmLoaderClick,300,640
 cmp [workerCalls],1
 jne failed
 cmp [vmLoaderAction],1
 jne failed
 cmp [lastWorker],VmLoaderWorker
 jne failed
 stdcall VmLoaderWorker,0
 cmp [postCalls],1
 jne failed
 cmp [fetchCalls],1
 jne failed
 mov [busy],0
 stdcall VmLoaderFinish
 cmp [vmLoaderAction],0
 jne failed
 cmp [vmLoaderCount],3
 jne failed
 invoke SendMessageW,[hVmLoaderList],LB_GETCOUNT,0,0
 cmp eax,3
 jne failed
 invoke SendMessageW,[hVmLoaderList],LB_GETTEXT,0,textW
 invoke lstrcmpiW,textW,fixtureUpgradeW
 test eax,eax
 jnz failed
 ; The shared catalogue cannot replace the displayed target behind its row.
 invoke lstrcpyA,loaderVersions,fixtureMutatedA
 inc [testStep]
 invoke SendMessageW,[hVmLoaderList],LB_SETCURSEL,0,0
 stdcall VmLoaderCommand,(LBN_SELCHANGE shl 16)+641
 cmp [vmLoaderSelected],0
 jne failed
 stdcall VmLoaderLabel
 cmp eax,sLcUpgrade
 jne failed
 cmp edx,1
 jne failed
 stdcall VmLoaderClick,560,640
 cmp [vmLoaderAction],2
 jne failed
 invoke lstrcmpA,vmLoaderTargetA,fixtureUpgradeA
 test eax,eax
 jnz failed
 ; Busy clicks and notifications cannot overwrite a pending operation.
 stdcall VmLoaderClick,900,640
 cmp [vmLoaderAction],2
 jne failed
 invoke SendMessageW,[hVmLoaderList],LB_SETCURSEL,2,0
 stdcall VmLoaderCommand,(LBN_SELCHANGE shl 16)+641
 cmp [vmLoaderSelected],0
 jne failed
 ; Worker scope is copied before scheduling and never reads the current page.
 invoke lstrcpyW,vmRootW,fixtureOtherW
 invoke lstrcpyA,vmIdA,fixtureOtherIdA
 stdcall VmLoaderWorker,0
 cmp [applyCalls],1
 jne failed
 invoke lstrcmpiW,lastRootW,fixtureRootW
 test eax,eax
 jnz failed
 invoke lstrcmpA,lastIdA,fixtureIdA
 test eax,eax
 jnz failed
 invoke lstrcmpA,lastTargetA,fixtureUpgradeA
 test eax,eax
 jnz failed
 invoke lstrcpyW,vmRootW,fixtureRootW
 invoke lstrcpyA,vmIdA,fixtureIdA
 mov [busy],0
 stdcall VmLoaderFinish
 cmp [refreshCalls],1
 jne failed
 cmp [vmLoaderBackup],1
 jne failed
 inc [testStep]
 ; Reinstalling the current version never queues a redundant transaction.
 invoke SendMessageW,[hVmLoaderList],LB_SETCURSEL,0,0
 stdcall VmLoaderCommand,(LBN_SELCHANGE shl 16)+641
 stdcall VmLoaderLabel
 cmp eax,sLcInstalled
 jne failed
 cmp edx,0
 jne failed
 stdcall VmLoaderClick,560,640
 cmp [workerCalls],2
 jne failed
 ; Older rows explicitly become downgrades and keep the original profile ID.
 invoke SendMessageW,[hVmLoaderList],LB_SETCURSEL,2,0
 stdcall VmLoaderCommand,(LBN_DBLCLK shl 16)+641
 stdcall VmLoaderLabel
 cmp eax,sLcDowngrade
 jne failed
 cmp edx,-1
 jne failed
 stdcall VmLoaderClick,560,640
 stdcall VmLoaderWorker,0
 mov [busy],0
 stdcall VmLoaderFinish
 invoke lstrcmpA,lastTargetA,fixtureDowngradeA
 test eax,eax
 jnz failed
 cmp [applyCalls],2
 jne failed
 inc [testStep]
 ; Restore is a separate action, and is unavailable without a backup.
 stdcall VmLoaderClick,900,640
 cmp [vmLoaderAction],3
 jne failed
 stdcall VmLoaderWorker,0
 mov [busy],0
 stdcall VmLoaderFinish
 cmp [restoreCalls],1
 jne failed
 mov [lcBackupReady],0
 stdcall VmLoaderOpen
 stdcall VmLoaderClick,900,640
 cmp [workerCalls],4
 jne failed
 inc [testStep]
 ; Native placement follows both the collapsed sidebar and device scaling.
 mov [contentX],-76
 mov [dpi],144
 stdcall VmLoaderShow
 invoke GetWindowRect,[hVmLoaderList],rect
 cmp [rect.left],294
 jne failed
 cmp [rect.top],573
 jne failed
 mov eax,[rect.right]
 sub eax,[rect.left]
 cmp eax,1191
 jne failed
 invoke GetWindowLongW,[hVmLoaderList],GWL_STYLE
 test eax,WS_VISIBLE
 jz failed
 stdcall VmLoaderEnable,0
 invoke IsWindowEnabled,[hVmLoaderList]
 test eax,eax
 jnz failed
 stdcall VmLoaderEnable,1
 stdcall VmLoaderHide
 invoke GetWindowLongW,[hVmLoaderList],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 inc [testStep]
 ; Changing base/kind invalidates rows even if the profile ID stays the same.
 invoke lstrcpyA,lcBaseA,fixtureBaseOtherA
 stdcall VmLoaderOpen
 cmp [vmLoaderCount],0
 jne failed
 cmp [vmLoaderSelected],-1
 jne failed
 ; Unsupported profiles and declined worker creation do not leak an action.
 mov [loadReject],1
 stdcall VmLoaderOpen
 cmp [vmLoaderContextReady],0
 jne failed
 cmp word [vmLoaderCurrentW],0
 jne failed
 stdcall VmLoaderShow
 invoke GetWindowLongW,[hVmLoaderList],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 stdcall VmLoaderClick,300,640
 cmp [workerCalls],4
 jne failed
 mov [loadReject],0
 stdcall VmLoaderOpen
 mov [startReject],1
 stdcall VmLoaderClick,300,640
 cmp [vmLoaderAction],0
 jne failed
 inc [testStep]
 ; Failed catalogue refresh clears stale targets instead of applying old data.
 mov [startReject],0
 mov [fetchReject],1
 stdcall VmLoaderClick,300,640
 stdcall VmLoaderWorker,0
 mov [busy],0
 stdcall VmLoaderFinish
 cmp [vmLoaderCount],0
 jne failed
 cmp [vmLoaderSelected],-1
 jne failed
 cmp [postCalls],5
 jne failed
 invoke DestroyWindow,[hVmLoaderList]
 invoke ExitProcess,0
failed:
 cinvoke printf,failureFormat,[testStep]
 invoke ExitProcess,[testStep]

proc NewControl,cls,style,id,x,y,w,h
 mov eax,[style]
 or eax,WS_POPUP
 invoke CreateWindowExW,0,[cls],emptyW,eax,[x],[y],[w],[h],0,0,0,0
 ret
endp
proc Scale,value
 invoke MulDiv,[value],[dpi],96
 ret
endp
proc LoaderChangeLoad,root,id
 inc [loadCalls]
 cmp [loadReject],1
 je .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc LoaderChangeFetch,root,id
 inc [fetchCalls]
 cmp [fetchReject],1
 je .bad
 mov [loaderVersionCount],3
 invoke lstrcpyA,loaderVersions,fixtureUpgradeA
 invoke lstrcpyA,loaderVersions+VL_SIZE,fixtureCurrentA
 invoke lstrcpyA,loaderVersions+VL_SIZE*2,fixtureDowngradeA
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc LoaderChangeApply,root,id,target
 inc [applyCalls]
 invoke lstrcpyW,lastRootW,[root]
 invoke lstrcpyA,lastIdA,[id]
 invoke lstrcpyA,lastTargetA,[target]
 invoke lstrcpyA,lcVersionA,[target]
 mov [lcBackupReady],1
 mov eax,1
 ret
endp
proc LoaderChangeRestore,root,id
 inc [restoreCalls]
 invoke lstrcpyA,lcVersionA,fixtureCurrentA
 mov eax,1
 ret
endp
proc LdCompare,left,right
 invoke lstrcmpA,[left],[right]
 test eax,eax
 jz .done
 jg .positive
 mov eax,-1
 ret
.positive:
 mov eax,1
.done:
 ret
endp
proc StartWorker,callback
 inc [workerCalls]
 mov eax,[callback]
 mov [lastWorker],eax
 cmp [startReject],1
 je .done
 mov [busy],1
.done:
 ret
endp
proc VersionPageRefresh
 inc [refreshCalls]
 ret
endp
proc FixturePost,window,message,wparam,lparam
 inc [postCalls]
 mov eax,1
 ret
endp
proc Card,x,y,w,h,r
 ret
endp
proc Text,dc,str,x,y,w,h,font,color
 ret
endp
proc Button,dc,str,x,y,w,h,selected
 ret
endp
proc FillRR,x,y,w,h,r,color
 ret
endp
proc TextC,dc,str,x,y,w,h,font,color
 ret
endp
proc StatusLine,x,y,w,h
 ret
endp
proc BusyBar,x,y,w
 ret
endp
define PostMessageW postPointer
include '..\src\loader_change_ui.inc'
restore PostMessageW
section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL',user32,'USER32.DLL',msvcrt,'MSVCRT.DLL'
import kernel32,ExitProcess,'ExitProcess',MulDiv,'MulDiv',lstrcpyA,'lstrcpyA',lstrcpyW,'lstrcpyW',lstrcmpA,'lstrcmpA',lstrcmpiW,'lstrcmpiW',MultiByteToWideChar,'MultiByteToWideChar'
import user32,CreateWindowExW,'CreateWindowExW',ShowWindow,'ShowWindow',EnableWindow,'EnableWindow',IsWindowEnabled,'IsWindowEnabled',SetWindowPos,'SetWindowPos',SendMessageW,'SendMessageW',InvalidateRect,'InvalidateRect',GetWindowRect,'GetWindowRect',GetWindowLongW,'GetWindowLongW',DestroyWindow,'DestroyWindow'
import msvcrt,printf,'printf'
