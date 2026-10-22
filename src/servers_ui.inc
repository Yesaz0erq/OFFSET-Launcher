; Saved Java server list. Native report cells are copies, never live worker data.
struct SV_UI_COLUMN
 mask dd ?
 fmt dd ?
 cx dd ?
 pszText dd ?
 cchTextMax dd ?
 iSubItem dd ?
 iImage dd ?
 iOrder dd ?
ends
struct SV_UI_ITEM
 mask dd ?
 iItem dd ?
 iSubItem dd ?
 state dd ?
 stateMask dd ?
 pszText dd ?
 cchTextMax dd ?
 iImage dd ?
 lParam dd ?
 iIndent dd ?
ends

section '.svuid' data readable writeable
hServersList dd 0
serversUiSeen dd 0
serversUiPending dd 0
serversUiCount dd 0
serversUiLoadResult dd 0
serversUiRefreshResult dd 0
serversUiFailed dd 0
serversUiError dd 0
serversUiLang dd -1
serversUiCommon dd 8,1
serversUiClass du 'SysListView32',0
serversUiEyebrow du '09 / SERVERS',0
serversUiPlayerFormat du '%u / %u',0
serversUiColumn SV_UI_COLUMN
serversUiItem SV_UI_ITEM
serversUiRootW rw 1024
serversUiIdA rb 128
serversUiWorkerRootW rw 1024
serversUiWorkerIdA rb 128
serversUiDataW rw 2048
serversUiVersionW rw 128
serversUiTextW rw 256

section '.svuic' code readable executable
 nop
proc ServersUiCreate
 invoke InitCommonControlsEx,serversUiCommon
 stdcall NewControl,serversUiClass,LVS_REPORT+LVS_SINGLESEL+LVS_SHOWSELALWAYS+LVS_NOSORTHEADER+WS_VSCROLL,901,272,270,794,324
 mov [hServersList],eax
 invoke SendMessageW,eax,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT+LVS_EX_LABELTIP
 mov [serversUiColumn.mask],LVCF_TEXT+LVCF_WIDTH+LVCF_SUBITEM
 mov [serversUiColumn.pszText],sServersName
 mov [serversUiColumn.cx],242
 mov [serversUiColumn.iSubItem],0
 invoke SendMessageW,[hServersList],LVM_INSERTCOLUMNW,0,serversUiColumn
 mov [serversUiColumn.pszText],sServersAddress
 mov [serversUiColumn.cx],346
 mov [serversUiColumn.iSubItem],1
 invoke SendMessageW,[hServersList],LVM_INSERTCOLUMNW,1,serversUiColumn
 mov [serversUiColumn.pszText],sServersPlayers
 mov [serversUiColumn.cx],168
 mov [serversUiColumn.iSubItem],2
 invoke SendMessageW,[hServersList],LVM_INSERTCOLUMNW,2,serversUiColumn
 mov eax,[hServersList]
 ret
endp
proc ServersUiHide
 invoke ShowWindow,[hServersList],SW_HIDE
 ret
endp
; Navigation explicitly invalidates the view; language and sidebar sync do not.
proc ServersUiInvalidate
 cmp [busy],0
 jne .done
 mov [serversUiSeen],0
 mov [serversUiCount],0
 mov [serversUiLoadResult],0
 mov [serversUiRefreshResult],0
 mov [serversUiFailed],0
 mov [serversUiError],0
 mov word [serversUiRootW],0
 mov byte [serversUiIdA],0
 mov word [serversUiDataW],0
 mov word [serversUiVersionW],0
 invoke SendMessageW,[hServersList],LVM_DELETEALLITEMS,0,0
 invoke ShowWindow,[hServersList],SW_HIDE
.done:
 ret
endp
proc ServersUiEnable,enabled
 invoke EnableWindow,[hServersList],[enabled]
 ret
endp
proc ServersUiHeaders
 invoke SendMessageW,[hServersList],WM_SETFONT,[fInput],1
 mov [serversUiColumn.mask],LVCF_TEXT+LVCF_WIDTH
 mov [serversUiColumn.pszText],sServersName
 stdcall Scale,242
 mov [serversUiColumn.cx],eax
 invoke SendMessageW,[hServersList],LVM_SETCOLUMNW,0,serversUiColumn
 mov [serversUiColumn.pszText],sServersAddress
 stdcall Scale,346
 mov [serversUiColumn.cx],eax
 invoke SendMessageW,[hServersList],LVM_SETCOLUMNW,1,serversUiColumn
 mov [serversUiColumn.pszText],sServersPlayers
 stdcall Scale,168
 mov [serversUiColumn.cx],eax
 invoke SendMessageW,[hServersList],LVM_SETCOLUMNW,2,serversUiColumn
 ret
endp
proc ServersUiShow
 locals
  x dd ?
  y dd ?
  w dd ?
  h dd ?
 endl
 stdcall ServersUiHeaders
 mov eax,272
 add eax,[contentX]
 stdcall Scale,eax
 mov [x],eax
 stdcall Scale,270
 mov [y],eax
 stdcall Scale,794
 mov [w],eax
 stdcall Scale,324
 mov [h],eax
 invoke SetWindowPos,[hServersList],0,[x],[y],[w],[h],SWP_NOZORDER+SWP_NOACTIVATE
 cmp [busy],0
 jne .visible
 cmp [serversUiSeen],0
 je .changed
 invoke lstrcmpiW,serversUiRootW,gameDir
 test eax,eax
 jnz .changed
 invoke lstrcmpA,serversUiIdA,selectedVersion
 test eax,eax
 jnz .changed
 mov eax,[uiLang]
 cmp eax,[serversUiLang]
 je .visible
 cmp [serversUiCount],0
 je .visible
 stdcall ServersUiFill
 jmp .visible
.changed:
 mov [serversUiCount],0
 mov [serversUiFailed],0
 mov [serversUiError],0
 mov word [serversUiDataW],0
 invoke SendMessageW,[hServersList],LVM_DELETEALLITEMS,0,0
 invoke lstrcpyW,serversUiRootW,gameDir
 invoke lstrcpyA,serversUiIdA,selectedVersion
 invoke MultiByteToWideChar,65001,8,serversUiIdA,-1,serversUiVersionW,128
 mov [serversUiSeen],1
 stdcall ServersUiStart
.visible:
 cmp [serversUiCount],0
 je .hide
 invoke ShowWindow,[hServersList],SW_SHOW
 ret
.hide:
 invoke ShowWindow,[hServersList],SW_HIDE
 ret
endp
proc ServersUiStateText,row
 mov eax,[row]
 mov edx,[eax+SV_STATE]
 cmp edx,1
 je .online
 mov eax,sServersPending
 cmp edx,0
 je .done
 mov eax,sServersOffline
 cmp edx,2
 je .done
 mov eax,sServersLoading
 cmp edx,3
 je .done
 mov eax,sServersInvalid
 cmp edx,4
 je .done
 mov eax,sServersIncompatible
 cmp edx,5
 je .done
 mov eax,sServersCancelled
 cmp edx,6
 je .done
 mov eax,sServersPending
.done:
 ret
.online:
 mov eax,[row]
 cinvoke wsprintfW,serversUiTextW,serversUiPlayerFormat,dword [eax+SV_ONLINE],dword [eax+SV_MAX]
 mov eax,serversUiTextW
 ret
endp
proc ServersUiFill uses ebx esi
 mov [serversUiCount],0
 invoke SendMessageW,[hServersList],WM_SETREDRAW,0,0
 invoke SendMessageW,[hServersList],LVM_DELETEALLITEMS,0,0
 xor esi,esi
.row:
 cmp esi,[serversCount]
 jae .done
 cmp esi,SV_CAPACITY
 jae .done
 imul ebx,esi,SV_SIZE
 add ebx,serversRows
 mov [serversUiItem.mask],LVIF_TEXT
 mov [serversUiItem.iItem],esi
 mov [serversUiItem.iSubItem],0
 mov [serversUiItem.pszText],ebx
 invoke SendMessageW,[hServersList],LVM_INSERTITEMW,0,serversUiItem
 cmp eax,-1
 je .done
 mov [serversUiItem.iSubItem],1
 lea eax,[ebx+SV_ADDRESS]
 mov [serversUiItem.pszText],eax
 invoke SendMessageW,[hServersList],LVM_SETITEMTEXTW,esi,serversUiItem
 mov [serversUiItem.iSubItem],2
 stdcall ServersUiStateText,ebx
 mov [serversUiItem.pszText],eax
 invoke SendMessageW,[hServersList],LVM_SETITEMTEXTW,esi,serversUiItem
 inc [serversUiCount]
 inc esi
 jmp .row
.done:
 mov eax,[uiLang]
 mov [serversUiLang],eax
 invoke SendMessageW,[hServersList],WM_SETREDRAW,1,0
 invoke InvalidateRect,[hServersList],0,1
 ret
endp
proc ServersUiStart
 cmp [busy],0
 jne .done
 mov [serversUiPending],1
 mov [serversUiLoadResult],0
 mov [serversUiRefreshResult],0
 mov [serversUiFailed],0
 mov [serversUiError],0
 invoke lstrcpyW,serversUiWorkerRootW,serversUiRootW
 invoke lstrcpyA,serversUiWorkerIdA,serversUiIdA
 stdcall StartWorker,ServersUiWorker
 cmp [busy],0
 jne .done
 mov [serversUiPending],0
 mov [serversUiFailed],1
.done:
 ret
endp
proc ServersUiWorker,param
 stdcall ServersLoad,serversUiWorkerRootW,serversUiWorkerIdA
 mov [serversUiLoadResult],eax
 test eax,eax
 jz .post
 stdcall ServersRefresh
 mov [serversUiRefreshResult],eax
.post:
 invoke PostMessageW,[hWnd],WM_FINISHED,0,0
 mov eax,[serversUiLoadResult]
 ret
endp
proc ServersUiFinish
 cmp [serversUiPending],1
 jne .done
 mov [serversUiPending],0
 mov eax,[serversLastError]
 mov [serversUiError],eax
 cmp [serversUiLoadResult],1
 jne .failed
 invoke lstrcpyW,serversUiDataW,serversDataDirW
 stdcall ServersUiFill
 mov eax,sServersCancelled
 cmp [engineCancel],0
 jne .successStatus
 cmp [modCancel],0
 jne .successStatus
 mov eax,sServersUnavailable
 cmp [serversUiRefreshResult],1
 jne .successStatus
 mov eax,sReady
.successStatus:
 invoke lstrcpyW,statusText,eax
 jmp .show
.failed:
 mov [serversUiFailed],1
 cmp [serversUiCount],0
 je .errorStatus
 cmp [serversCount],0
 je .clearCache
 invoke lstrcmpiW,serversUiDataW,serversDataDirW
 test eax,eax
 jz .errorStatus
.clearCache:
 mov [serversUiCount],0
 invoke SendMessageW,[hServersList],LVM_DELETEALLITEMS,0,0
 invoke lstrcpyW,serversUiDataW,serversDataDirW
.errorStatus:
 mov eax,sServersReadFailed
 cmp [serversUiCount],0
 jne @f
 mov eax,sServersUnavailable
@@:
 cmp [serversUiError],SV_ERR_CANCEL
 jne @f
 mov eax,sServersCancelled
@@:
 invoke lstrcpyW,statusText,eax
.show:
 cmp [currentPage],8
 jne .done
 stdcall ServersUiShow
.done:
 ret
endp
proc ServersUiPaint,dc
 stdcall PageHeader,serversUiEyebrow,sServersHeading,9
 mov eax,sServersRefresh
 cmp [serversUiPending],1
 jne @f
 mov eax,sModCancel
@@:
 stdcall Button,[dc],eax,916,124,172,42,0
 stdcall Card,252,185,836,428,18
 stdcall TextDraw,serversUiVersionW,280,203,536,28,[fMedium],C_INK,DT_LEFT+DT_END_ELLIPSIS,0
 cinvoke wsprintfW,serversUiTextW,sServersCount,[serversUiCount]
 stdcall TextR,[dc],serversUiTextW,828,204,232,26,[fSmall],C_MUTED
 stdcall TextDraw,serversUiDataW,280,239,776,22,[fMonoSmall],C_MUTED,DT_LEFT+DT_PATH_ELLIPSIS,0
 cmp [serversUiCount],0
 jne .footer
 mov eax,sServersEmpty
 cmp [serversUiFailed],1
 jne @f
 mov eax,sServersUnavailable
@@:
 cmp [serversUiPending],1
 jne @f
 mov eax,sServersLoading
@@:
 stdcall Text,[dc],eax,292,350,736,48,[fBody],C_MUTED
.footer:
 stdcall StatusLine,254,676,830,32
 stdcall BusyBar,254,664,834
 ret
endp
proc ServersUiClick,x,y
 cmp [x],916
 jb .done
 cmp [x],1088
 ja .done
 cmp [y],124
 jb .done
 cmp [y],166
 ja .done
 cmp [busy],0
 je .refresh
 cmp [serversUiPending],1
 jne .done
 mov [engineCancel],1
 mov [modCancel],1
 ret
.refresh:
 stdcall ServersUiStart
.done:
 ret
endp
proc ServersUiCommand,wparam
 ; A report view has no edit, connect, reorder or delete commands.
 ret
endp
