; Native report view and worker-scope regression; no network or servers.dat writes.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\servers_types.inc'
WM_FINISHED=8002h
C_INK=0
C_MUTED=0
section '.data' data readable writeable
hWnd dd 0
busy dd 0
currentPage dd 8
contentX dd 0
dpi dd 96
engineCancel dd 0
modCancel dd 0
uiLang dd 0
fInput dd 0
fBody dd 0
fSmall dd 0
fMedium dd 0
fMonoSmall dd 0
emptyW dw 0
gameDir rw 1024
selectedVersion rb 128
statusText rw 1024
serversCount dd 0
serversLastError dd 0
serversRows rb SV_SIZE*SV_CAPACITY
serversDataDirW rw 2048
fixtureRootW du 'fixture-root',0
fixtureOtherW du 'other-root',0
fixtureIdA db 'profile-one',0
fixtureOtherIdA db 'profile-two',0
fixtureDataW du 'fixture-root\versions\profile-one\game',0
fixtureNameW du 04E2Dh,06587h,' ', 'Server',0
fixtureAddressW du 'play.example.test:25566',0
fixtureNumbersW du '42 / 100',0
readPointer dd FixturePost
loadCalls dd 0
refreshCalls dd 0
workerCalls dd 0
postCalls dd 0
lastWorker dd 0
fixtureLoadFailure dd 0
fixtureEmpty dd 0
fixtureReject dd 0
fixtureRowCount dd 7
fixtureNetFailure dd 0
lastRootW rw 1024
lastIdA rb 128
textW rw 256
columnTextW rw 128
rect RECT
testStep dd 1
fixtureState dd 0
failureFormat db 'servers-ui step %u failed',13,10,0
include '..\src\strings.inc'
section '.text' code readable executable
start:
 stdcall FixtureLang,0
 invoke lstrcpyW,gameDir,fixtureRootW
 invoke lstrcpyA,selectedVersion,fixtureIdA
 stdcall ServersUiCreate
 test eax,eax
 jz failed
 ; This is a native report view with editing and header sorting disabled.
 invoke GetWindowLongW,[hServersList],GWL_STYLE
 test eax,LVS_EDITLABELS
 jnz failed
 test eax,LVS_NOSORTHEADER
 jz failed
 ; Entering during another worker defers the first lookup until idle.
 mov [busy],1
 stdcall ServersUiShow
 cmp [workerCalls],0
 jne failed
 cmp [serversUiSeen],0
 jne failed
 cmp [serversUiPending],0
 jne failed
 mov [busy],0
 stdcall ServersUiShow
 cmp [workerCalls],1
 jne failed
 cmp [serversUiPending],1
 jne failed
 cmp [loadCalls],0
 jne failed
 cmp [refreshCalls],0
 jne failed
 stdcall ServersUiShow
 cmp [workerCalls],1
 jne failed
 inc [testStep]
 ; The worker uses its captured profile even if the active page changes.
 invoke lstrcpyW,gameDir,fixtureOtherW
 invoke lstrcpyA,selectedVersion,fixtureOtherIdA
 stdcall ServersUiWorker,0
 invoke lstrcmpiW,lastRootW,fixtureRootW
 test eax,eax
 jnz failed
 invoke lstrcmpA,lastIdA,fixtureIdA
 test eax,eax
 jnz failed
 cmp [postCalls],1
 jne failed
 ; No half-updated backend rows appear before WM_FINISHED is processed.
 cmp [serversUiCount],0
 jne failed
 stdcall ServersUiShow
 invoke SendMessageW,[hServersList],LVM_GETITEMCOUNT,0,0
 test eax,eax
 jnz failed
 invoke lstrcpyW,gameDir,fixtureRootW
 invoke lstrcpyA,selectedVersion,fixtureIdA
 mov [busy],0
 stdcall ServersUiFinish
 cmp [serversUiPending],0
 jne failed
 invoke lstrcmpiW,statusText,sReady
 test eax,eax
 jnz failed
 cmp [serversUiCount],7
 jne failed
 invoke SendMessageW,[hServersList],LVM_GETITEMCOUNT,0,0
 cmp eax,7
 jne failed
 stdcall FixtureCell,0,0,textW
 invoke lstrcmpiW,textW,fixtureNameW
 test eax,eax
 jnz failed
 stdcall FixtureCell,0,1,textW
 invoke lstrcmpiW,textW,fixtureAddressW
 test eax,eax
 jnz failed
 stdcall FixtureCell,0,2,textW
 invoke lstrcmpiW,textW,fixtureNumbersW
 test eax,eax
 jnz failed
 stdcall FixtureCell,1,2,textW
 invoke lstrcmpiW,textW,sServersOffline
 test eax,eax
 jnz failed
 stdcall FixtureCell,4,2,textW
 invoke lstrcmpiW,textW,sServersInvalid
 test eax,eax
 jnz failed
 stdcall FixtureCell,5,2,textW
 invoke lstrcmpiW,textW,sServersIncompatible
 test eax,eax
 jnz failed
 inc [testStep]
 ; Reopening does not query again; language/font updates keep cell meanings.
 stdcall ServersUiShow
 cmp [workerCalls],1
 jne failed
 stdcall FixtureLang,1
 stdcall ServersUiShow
 stdcall FixtureCell,1,2,textW
 invoke lstrcmpiW,textW,sServersOffline
 test eax,eax
 jnz failed
 cmp word [textW],'O'
 jne failed
 cmp word [textW+2],'f'
 jne failed
 mov [serversUiColumn.mask],LVCF_TEXT
 mov [serversUiColumn.pszText],columnTextW
 mov [serversUiColumn.cchTextMax],128
 invoke SendMessageW,[hServersList],LVM_GETCOLUMNW,2,serversUiColumn
 invoke lstrcmpiW,columnTextW,sServersPlayers
 test eax,eax
 jnz failed
 cmp [workerCalls],1
 jne failed
 inc [testStep]
 ; Report geometry follows the collapsed sidebar and high DPI.
 mov [contentX],-76
 mov [dpi],144
 stdcall ServersUiShow
 invoke GetWindowRect,[hServersList],rect
 cmp [rect.left],294
 jne failed
 cmp [rect.top],405
 jne failed
 mov eax,[rect.right]
 sub eax,[rect.left]
 cmp eax,1191
 jne failed
 stdcall ServersUiEnable,0
 invoke IsWindowEnabled,[hServersList]
 test eax,eax
 jnz failed
 stdcall ServersUiEnable,1
 stdcall ServersUiHide
 invoke GetWindowLongW,[hServersList],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 inc [testStep]
 ; Refresh cancellation cannot schedule a second worker or rewrite the file.
 stdcall ServersUiClick,950,140
 cmp [workerCalls],2
 jne failed
 stdcall ServersUiClick,950,140
 cmp [engineCancel],1
 jne failed
 cmp [modCancel],1
 jne failed
 cmp [workerCalls],2
 jne failed
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 stdcall FixtureCell,0,2,textW
 invoke lstrcmpiW,textW,sServersCancelled
 test eax,eax
 jnz failed
 cmp [postCalls],2
 jne failed
 invoke lstrcmpiW,statusText,sServersCancelled
 test eax,eax
 jnz failed
 inc [testStep]
 ; Malformed file refresh keeps this context's copied, last successful rows.
 mov [fixtureLoadFailure],1
 stdcall ServersUiClick,950,140
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 cmp [serversUiCount],7
 jne failed
 cmp [serversUiFailed],1
 jne failed
 invoke lstrcmpiW,statusText,sServersReadFailed
 test eax,eax
 jnz failed
 cmp [refreshCalls],2
 jne failed
 inc [testStep]
 ; Switching profiles clears old names before a new query, even on failure.
 invoke lstrcpyW,gameDir,fixtureOtherW
 invoke lstrcpyA,selectedVersion,fixtureOtherIdA
 stdcall ServersUiShow
 cmp [serversUiCount],0
 jne failed
 invoke SendMessageW,[hServersList],LVM_GETITEMCOUNT,0,0
 test eax,eax
 jnz failed
 cmp word [serversUiDataW],0
 jne failed
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 cmp [serversUiCount],0
 jne failed
 invoke GetWindowLongW,[hServersList],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 inc [testStep]
 ; Missing servers.dat is a successful empty collection, with no fake rows.
 mov [fixtureLoadFailure],0
 mov [fixtureEmpty],1
 stdcall ServersUiClick,950,140
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 cmp [serversUiCount],0
 jne failed
 cmp [serversUiFailed],0
 jne failed
 cmp [serversUiLoadResult],1
 jne failed
 cmp [postCalls],5
 jne failed
 ; An unrelated completion neither rebuilds nor starts a new query.
 stdcall ServersUiFinish
 cmp [postCalls],5
 jne failed
 cmp [workerCalls],5
 jne failed
 inc [testStep]
 ; Reentering explicitly rereads even the same selected profile.
 mov [fixtureEmpty],0
 stdcall ServersUiInvalidate
 cmp [serversUiSeen],0
 jne failed
 cmp [serversUiCount],0
 jne failed
 stdcall ServersUiShow
 cmp [workerCalls],6
 jne failed
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 cmp [serversUiCount],7
 jne failed
 inc [testStep]
 ; An externally changed effective data directory cannot keep old results.
 mov [fixtureLoadFailure],1
 stdcall ServersUiClick,950,140
 invoke lstrcpyW,serversDataDirW,fixtureOtherW
 mov [serversCount],0
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 cmp [serversUiCount],0
 jne failed
 invoke SendMessageW,[hServersList],LVM_GETITEMCOUNT,0,0
 test eax,eax
 jnz failed
 invoke lstrcmpiW,statusText,sServersUnavailable
 test eax,eax
 jnz failed
 inc [testStep]
 ; Large valid collections are scrollable and bounded to the shared capacity.
 mov [fixtureLoadFailure],0
 mov [fixtureRowCount],SV_CAPACITY
 stdcall ServersUiClick,950,140
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 cmp [serversUiCount],SV_CAPACITY
 jne failed
 invoke SendMessageW,[hServersList],LVM_GETITEMCOUNT,0,0
 cmp eax,SV_CAPACITY
 jne failed
 invoke SendMessageW,[hServersList],LVM_ENSUREVISIBLE,SV_CAPACITY-1,0
 invoke SendMessageW,[hServersList],LVM_GETTOPINDEX,0,0
 test eax,eax
 jz failed
 inc [testStep]
 ; A transport-start failure is reported instead of retaining "Working".
 mov [fixtureNetFailure],1
 stdcall ServersUiClick,950,140
 stdcall ServersUiWorker,0
 mov [busy],0
 stdcall ServersUiFinish
 invoke lstrcmpiW,statusText,sServersUnavailable
 test eax,eax
 jnz failed
 invoke DestroyWindow,[hServersList]
 invoke ExitProcess,0
failed:
 cinvoke printf,failureFormat,[testStep]
 invoke ExitProcess,[testStep]

proc FixtureLang uses ebx esi edi,index
 mov eax,[index]
 mov [uiLang],eax
 mov esi,[uiLangData+eax*4]
 mov edi,uiStringSlots
 mov ebx,UI_STRING_COUNT
.slot:
 mov ecx,[edi]
 add edi,4
.char:
 lodsw
 mov [ecx],ax
 add ecx,2
 test ax,ax
 jnz .char
 dec ebx
 jnz .slot
 ret
endp
proc FixtureCell,row,column,destination
 mov eax,[column]
 mov [serversUiItem.iSubItem],eax
 mov eax,[destination]
 mov [serversUiItem.pszText],eax
 mov [serversUiItem.cchTextMax],256
 invoke SendMessageW,[hServersList],LVM_GETITEMTEXTW,[row],serversUiItem
 ret
endp
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
proc StartWorker,callback
 inc [workerCalls]
 mov eax,[callback]
 mov [lastWorker],eax
 mov [engineCancel],0
 mov [modCancel],0
 cmp [fixtureReject],1
 je .done
 mov [busy],1
.done:
 ret
endp
proc ServersLoad uses ebx esi,root,id
 inc [loadCalls]
 invoke lstrcpyW,lastRootW,[root]
 invoke lstrcpyA,lastIdA,[id]
 cmp [fixtureLoadFailure],1
 je .bad
 invoke lstrcpyW,serversDataDirW,fixtureDataW
 mov [serversLastError],0
 mov [serversCount],0
 cmp [fixtureEmpty],1
 je .good
 mov eax,[fixtureRowCount]
 mov [serversCount],eax
 xor esi,esi
.row:
 imul ebx,esi,SV_SIZE
 add ebx,serversRows
 invoke lstrcpyW,ebx,fixtureNameW
 lea eax,[ebx+SV_ADDRESS]
 invoke lstrcpyW,eax,fixtureAddressW
 mov [ebx+SV_STATE],esi
 mov dword [ebx+SV_ONLINE],42
 mov dword [ebx+SV_MAX],100
 inc esi
 cmp esi,[fixtureRowCount]
 jb .row
 ; Row zero is the online example, row two remains the unqueried example.
 mov dword [serversRows+SV_STATE],1
 mov dword [serversRows+SV_SIZE+SV_STATE],2
 mov dword [serversRows+SV_SIZE*2+SV_STATE],0
.good:
 mov eax,1
 ret
.bad:
 mov [serversLastError],SV_ERR_NBT
 xor eax,eax
 ret
endp
proc ServersRefresh uses ebx esi
 inc [refreshCalls]
 cmp [fixtureNetFailure],1
 jne @f
 xor eax,eax
 ret
@@:
 cmp [engineCancel],0
 je .good
 xor esi,esi
.row:
 cmp esi,[serversCount]
 jae .cancelled
 imul ebx,esi,SV_SIZE
 add ebx,serversRows
 mov dword [ebx+SV_STATE],SV_CANCELLED
 inc esi
 jmp .row
.cancelled:
 mov [serversLastError],SV_ERR_CANCEL
 xor eax,eax
 ret
.good:
 mov eax,1
 ret
endp
proc FixturePost,window,message,wparam,lparam
 inc [postCalls]
 mov eax,1
 ret
endp
proc PageHeader,eyebrow,title,page
 ret
endp
proc Button,dc,str,x,y,w,h,primary
 ret
endp
proc Card,x,y,w,h,r
 ret
endp
proc TextDraw,str,x,y,w,h,font,color,flags,extra
 ret
endp
proc TextR,dc,str,x,y,w,h,font,color
 ret
endp
proc Text,dc,str,x,y,w,h,font,color
 ret
endp
proc StatusLine,x,y,w,h
 ret
endp
proc BusyBar,x,y,w
 ret
endp
define PostMessageW readPointer
include '..\src\servers_ui.inc'
restore PostMessageW
section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL',user32,'USER32.DLL',comctl32,'COMCTL32.DLL',msvcrt,'MSVCRT.DLL'
import kernel32,ExitProcess,'ExitProcess',MulDiv,'MulDiv',lstrcpyA,'lstrcpyA',lstrcpyW,'lstrcpyW',lstrcmpA,'lstrcmpA',lstrcmpiW,'lstrcmpiW',MultiByteToWideChar,'MultiByteToWideChar'
import user32,CreateWindowExW,'CreateWindowExW',ShowWindow,'ShowWindow',EnableWindow,'EnableWindow',IsWindowEnabled,'IsWindowEnabled',SetWindowPos,'SetWindowPos',SendMessageW,'SendMessageW',InvalidateRect,'InvalidateRect',GetWindowRect,'GetWindowRect',GetWindowLongW,'GetWindowLongW',DestroyWindow,'DestroyWindow',wsprintfW,'wsprintfW'
import comctl32,InitCommonControlsEx,'InitCommonControlsEx'
import msvcrt,printf,'printf'
