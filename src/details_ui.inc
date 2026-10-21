; Native scrollable project introductions, shared by mods and modpacks.
section '.dtuid' data readable writeable
detailActive dd 0
detailOwner dd 4
detailProvider dd 0
detailPack dd 0
hDetailBody dd 0
detailId rb 64
detailWebsite rb 1024
detailFallback rb 512
detailWide rw 131072
detailUrlWide rw 4096

section '.dtuic' code readable executable
 nop
proc CreateDetailControls
 stdcall NewControl,editClass,ES_MULTILINE+ES_AUTOVSCROLL+ES_READONLY+WS_VSCROLL,451,280,292,782,300
 mov [hDetailBody],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,131071,0
 ret
endp
proc HideDetailControls
 invoke ShowWindow,[hDetailBody],SW_HIDE
 ret
endp
proc ShowDetailControls
 invoke ShowWindow,[hDetailBody],SW_SHOW
 ret
endp
proc DetailOpen uses ebx,record,provider,isPack
 cmp [busy],0
 jne .done
 cmp [provider],1
 ja .done
 cmp [isPack],1
 ja .done
 cmp [record],0
 je .done
 mov ebx,[record]
 invoke lstrcpyA,detailId,ebx
 lea eax,[ebx+MP_TITLE]
 invoke lstrcpyA,detailFallback,eax
 lea eax,[ebx+MP_URL]
 invoke lstrcpyA,detailWebsite,eax
 mov eax,[provider]
 mov [detailProvider],eax
 mov eax,[isPack]
 mov [detailPack],eax
 mov eax,[currentPage]
 mov [detailOwner],eax
 mov [detailActive],1
 mov byte [detailTitle],0
 mov byte [detailBody],0
 mov byte [detailMeta],0
 invoke SetWindowTextW,[hDetailBody],sDetailLoading
 stdcall SyncPage
 stdcall StartWorker,DetailWorker
.done:
 ret
endp
proc DetailWorker,param
 stdcall ProjectFetchDetails,[detailProvider],detailId,[detailPack]
 invoke PostMessageW,[hWnd],WM_FINISHED,0,0
 ret
endp
proc DetailFinishUI
 cmp [detailActive],0
 je .done
 cmp byte [detailTitle],0
 jne .body
 invoke lstrcpyA,detailTitle,detailFallback
.body:
 cmp byte [detailBody],0
 je .empty
 invoke MultiByteToWideChar,65001,0,detailBody,-1,detailWide,131072
 test eax,eax
 jz .empty
 invoke SetWindowTextW,[hDetailBody],detailWide
 jmp .show
.empty:
 invoke SetWindowTextW,[hDetailBody],sDetailUnavailable
.show:
 stdcall SyncPage
.done:
 ret
endp
proc PaintProjectDetail,dc
 stdcall PageHeader,eyebrowDetails,sDetailHeading,5
 mov eax,sDetailBack
 cmp [busy],0
 je @f
 mov eax,sModCancel
@@:
 stdcall Button,[dc],eax,916,124,172,42,0
 stdcall Card,252,185,836,428,18
 invoke MultiByteToWideChar,65001,0,detailFallback,-1,detailWide,131072
 cmp byte [detailTitle],0
 je @f
 invoke MultiByteToWideChar,65001,0,detailTitle,-1,detailWide,131072
@@:
 stdcall Text,[dc],detailWide,280,201,780,35,[fTitle],C_INK
 invoke MultiByteToWideChar,65001,0,detailMeta,-1,detailWide,131072
 stdcall DetailMetaLine,detailWide
 stdcall Text,[dc],detailWide,280,241,782,38,[fSmall],C_MUTED
 stdcall Button,[dc],sDetailMcmod,280,620,210,40,0
 stdcall Button,[dc],sModWebsite,510,620,210,40,0
 stdcall StatusLine,254,676,830,32
 stdcall BusyBar,254,664,834
 ret
endp
proc DetailMetaLine uses esi,text
 mov esi,[text]
.loop:
 mov ax,[esi]
 test ax,ax
 jz .done
 cmp ax,13
 je .space
 cmp ax,10
 jne .next
.space:
 mov word [esi],' '
.next:
 add esi,2
 jmp .loop
.done:
 ret
endp
proc OpenMcmodQuery,query
 stdcall McmodSearchUrl,[query],detailUrlWide,4096
 test eax,eax
 jz .done
 invoke ShellExecuteW,[hWnd],openVerb,detailUrlWide,0,0,SW_SHOWNORMAL
.done:
 ret
endp
proc ClickProjectDetail,x,y
 cmp [y],124
 jb .done
 cmp [y],166
 ja .lower
 cmp [x],916
 jb .done
 cmp [x],1088
 ja .done
 cmp [busy],0
 jne .cancel
 mov [detailActive],0
 stdcall SyncPage
 jmp .done
.cancel:
 mov [modCancel],1
 mov [engineCancel],1
 jmp .done
.lower:
 cmp [busy],0
 jne .done
 cmp [y],620
 jb .done
 cmp [y],660
 ja .done
 cmp [x],280
 jb .done
 cmp [x],490
 ja .website
 stdcall OpenMcmodQuery,detailFallback
 jmp .done
.website:
 cmp [x],510
 jb .done
 cmp [x],720
 ja .done
 cmp byte [detailWebsite],0
 je .done
 invoke MultiByteToWideChar,65001,0,detailWebsite,-1,detailUrlWide,4096
 test eax,eax
 jz .done
 invoke ShellExecuteW,[hWnd],openVerb,detailUrlWide,0,0,SW_SHOWNORMAL
.done:
 ret
endp
