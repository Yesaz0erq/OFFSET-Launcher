; Per-profile loader changes. UI selection never changes the launch selection.
section '.lcuid' data readable writeable
hVmLoaderList dd 0
vmLoaderAction dd 0
vmLoaderContextReady dd 0
vmLoaderCount dd 0
vmLoaderSelected dd -1
vmLoaderKind dd 0
vmLoaderBackup dd 0
vmLoaderResult dd 0
vmLoaderRootW rw 1024
vmLoaderIdA rb 128
vmLoaderBaseA rb 128
vmLoaderCurrentA rb 128
vmLoaderTargetA rb 128
vmLoaderWorkerRootW rw 1024
vmLoaderWorkerIdA rb 128
vmLoaderBaseW rw 128
vmLoaderCurrentW rw 128
vmLoaderTextW rw 256
; Own the displayed list: catalogue downloads use the shared loaderVersions.
vmLoaderRows rb 128*VL_CAPACITY

section '.lcuic' code readable executable
 nop
proc VmLoaderCreate
 stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,641,272,382,794,206
 mov [hVmLoaderList],eax
 ret
endp
proc VmLoaderHide
 invoke ShowWindow,[hVmLoaderList],SW_HIDE
 ret
endp
proc VmLoaderEnable,enabled
 invoke EnableWindow,[hVmLoaderList],[enabled]
 ret
endp

; Clear a list only when its profile changes. Repainting and language changes
; preserve the selected target; entering the page never starts a download.
proc VmLoaderOpen
 cmp [busy],0
 jne .done
 invoke lstrcmpiW,vmLoaderRootW,vmRootW
 test eax,eax
 jnz .changed
 invoke lstrcmpA,vmLoaderIdA,vmIdA
 test eax,eax
 jz .context
.changed:
 mov [vmLoaderCount],0
 mov [vmLoaderSelected],-1
 invoke SendMessageW,[hVmLoaderList],LB_RESETCONTENT,0,0
 invoke lstrcpyW,vmLoaderRootW,vmRootW
 invoke lstrcpyA,vmLoaderIdA,vmIdA
.context:
 mov [vmLoaderContextReady],0
 mov [vmLoaderBackup],0
 mov byte [vmLoaderCurrentA],0
 mov word [vmLoaderBaseW],0
 mov word [vmLoaderCurrentW],0
 stdcall LoaderChangeLoad,vmLoaderRootW,vmLoaderIdA
 test eax,eax
 jz .done
 mov eax,[lcKind]
 cmp eax,[vmLoaderKind]
 jne .clearList
 invoke lstrcmpA,vmLoaderBaseA,lcBaseA
 test eax,eax
 jz .ready
.clearList:
 mov [vmLoaderCount],0
 mov [vmLoaderSelected],-1
 invoke SendMessageW,[hVmLoaderList],LB_RESETCONTENT,0,0
.ready:
 mov [vmLoaderContextReady],1
 mov eax,[lcKind]
 mov [vmLoaderKind],eax
 mov eax,[lcBackupReady]
 mov [vmLoaderBackup],eax
 invoke lstrcpyA,vmLoaderBaseA,lcBaseA
 invoke lstrcpyA,vmLoaderCurrentA,lcVersionA
 invoke MultiByteToWideChar,65001,8,vmLoaderBaseA,-1,vmLoaderBaseW,128
 invoke MultiByteToWideChar,65001,8,vmLoaderCurrentA,-1,vmLoaderCurrentW,128
.done:
 ret
endp
proc VmLoaderShow
 locals
  x dd ?
  y dd ?
  w dd ?
  h dd ?
 endl
 stdcall VmLoaderOpen
 invoke ShowWindow,[hVmLoaderList],SW_HIDE
 mov eax,272
 add eax,[contentX]
 stdcall Scale,eax
 mov [x],eax
 stdcall Scale,382
 mov [y],eax
 stdcall Scale,794
 mov [w],eax
 stdcall Scale,206
 mov [h],eax
 invoke SetWindowPos,[hVmLoaderList],0,[x],[y],[w],[h],SWP_NOZORDER+SWP_NOACTIVATE
 cmp [vmLoaderContextReady],1
 jne .done
 cmp [vmLoaderCount],0
 je .done
 invoke ShowWindow,[hVmLoaderList],SW_SHOW
.done:
 ret
endp
proc VmLoaderFill uses ebx esi
 mov [vmLoaderCount],0
 mov [vmLoaderSelected],-1
 invoke SendMessageW,[hVmLoaderList],LB_RESETCONTENT,0,0
 xor esi,esi
.row:
 cmp esi,[loaderVersionCount]
 jae .done
 cmp esi,VL_CAPACITY
 jae .done
 imul ebx,esi,VL_SIZE
 add ebx,loaderVersions
 mov eax,esi
 shl eax,7
 add eax,vmLoaderRows
 invoke lstrcpyA,eax,ebx
 invoke MultiByteToWideChar,65001,8,ebx,-1,vmLoaderTextW,256
 invoke SendMessageW,[hVmLoaderList],LB_ADDSTRING,0,vmLoaderTextW
 cmp eax,LB_ERR
 je .done
 cmp eax,LB_ERRSPACE
 je .done
 inc [vmLoaderCount]
 inc esi
 jmp .row
.done:
 invoke InvalidateRect,[hWnd],0,0
 ret
endp

; The label uses the same natural version comparator as the loader catalogue.
; EDX is -1 / 0 / 1 for downgrade / installed / upgrade; 2 means no target.
proc VmLoaderLabel
 mov eax,sLcApply
 mov edx,2
 cmp [vmLoaderContextReady],1
 jne .done
 mov ecx,[vmLoaderSelected]
 cmp ecx,[vmLoaderCount]
 jae .done
 shl ecx,7
 add ecx,vmLoaderRows
 stdcall LdCompare,ecx,vmLoaderCurrentA
 mov edx,eax
 test eax,eax
 jz .installed
 jg .upgrade
 mov eax,sLcDowngrade
 ret
.upgrade:
 mov eax,sLcUpgrade
 ret
.installed:
 mov eax,sLcInstalled
.done:
 ret
endp
proc VmLoaderPaint uses ebx,dc
 stdcall Card,252,185,836,132,18
 stdcall Text,[dc],sVmBase,280,204,154,26,[fSmall],C_MUTED
 mov eax,vmLoaderBaseW
 cmp [vmLoaderContextReady],1
 je @f
 mov eax,vmBaseW
@@:
 stdcall Text,[dc],eax,440,203,616,28,[fMedium],C_INK
 stdcall Text,[dc],sVmLoader,280,251,154,26,[fSmall],C_MUTED
 mov eax,[vsLoaderKind]
 cmp [vmLoaderContextReady],1
 jne @f
 mov eax,[vmLoaderKind]
@@:
 cmp eax,4
 jbe @f
 xor eax,eax
@@:
 stdcall Text,[dc],dword [vUiNames+eax*4],440,250,196,28,[fMedium],C_INK
 stdcall Text,[dc],vmLoaderCurrentW,656,250,400,28,[fMedium],C_INK
 stdcall Card,252,334,836,272,18
 stdcall Text,[dc],sLcTarget,280,347,760,26,[fSmall],C_MUTED
 cmp [vmLoaderContextReady],1
 jne .unsupported
 cmp [vmLoaderCount],0
 jne .buttons
 stdcall Text,[dc],sLcEmpty,292,405,736,30,[fBody],C_MUTED
 jmp .buttons
.unsupported:
 stdcall Text,[dc],sLcUnsupported,292,405,736,56,[fBody],C_MUTED
.buttons:
 stdcall VmLoaderButton,[dc],sLcRefresh,252,620,176,42,[vmLoaderContextReady],0
 stdcall VmLoaderLabel
 mov ebx,eax
 xor eax,eax
 cmp [vmLoaderContextReady],1
 jne @f
 cmp edx,2
 je @f
 test edx,edx
 setnz al
@@:
 stdcall VmLoaderButton,[dc],ebx,444,620,278,42,eax,1
 stdcall VmLoaderButton,[dc],sLcRestore,738,620,350,42,[vmLoaderBackup],0
 stdcall StatusLine,254,676,830,32
 stdcall BusyBar,254,664,834
 ret
endp
proc VmLoaderButton,dc,str,x,y,w,h,enabled,primary
 cmp [enabled],1
 jne .disabled
 cmp [busy],0
 jne .disabled
 stdcall Button,[dc],[str],[x],[y],[w],[h],[primary]
 ret
.disabled:
 stdcall FillRR,[x],[y],[w],[h],21,A_SECOND
 stdcall TextC,[dc],[str],[x],[y],[w],[h],[fMedium],C_MUTED
 ret
endp
proc VmLoaderCommand,wparam
 cmp [currentPage],1
 jne .done
 cmp [vmActive],1
 jne .done
 cmp [vmTab],3
 jne .done
 cmp [busy],0
 jne .done
 mov eax,[wparam]
 mov edx,eax
 shr edx,16
 and eax,0FFFFh
 cmp eax,641
 jne .done
 cmp edx,LBN_SELCHANGE
 je .select
 cmp edx,LBN_DBLCLK
 jne .done
.select:
 invoke SendMessageW,[hVmLoaderList],LB_GETCURSEL,0,0
 cmp eax,[vmLoaderCount]
 jae .done
 mov [vmLoaderSelected],eax
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc VmLoaderStart,action
 cmp [busy],0
 jne .done
 cmp [vmLoaderContextReady],1
 jne .done
 mov eax,[action]
 mov [vmLoaderAction],eax
 mov [vmLoaderResult],0
 invoke lstrcpyW,vmLoaderWorkerRootW,vmLoaderRootW
 invoke lstrcpyA,vmLoaderWorkerIdA,vmLoaderIdA
 stdcall StartWorker,VmLoaderWorker
 cmp [busy],0
 jne .done
 mov [vmLoaderAction],0
.done:
 ret
endp
proc VmLoaderClick,x,y
 cmp [busy],0
 jne .done
 cmp [y],620
 jb .done
 cmp [y],662
 ja .done
 cmp [x],252
 jb .done
 cmp [x],428
 jbe .refresh
 cmp [x],444
 jb .done
 cmp [x],722
 jbe .apply
 cmp [x],738
 jb .done
 cmp [x],1088
 ja .done
 cmp [vmLoaderBackup],1
 jne .done
 stdcall VmLoaderStart,3
 ret
.refresh:
 stdcall VmLoaderStart,1
 ret
.apply:
 stdcall VmLoaderLabel
 cmp edx,2
 je .done
 test edx,edx
 jz .done
 mov eax,[vmLoaderSelected]
 cmp eax,[vmLoaderCount]
 jae .done
 shl eax,7
 add eax,vmLoaderRows
 invoke lstrcpyA,vmLoaderTargetA,eax
 stdcall VmLoaderStart,2
.done:
 ret
endp
proc VmLoaderWorker,param
 mov eax,[vmLoaderAction]
 cmp eax,1
 je .fetch
 cmp eax,2
 je .apply
 cmp eax,3
 jne .post
 stdcall LoaderChangeRestore,vmLoaderWorkerRootW,vmLoaderWorkerIdA
 jmp .result
.fetch:
 stdcall LoaderChangeFetch,vmLoaderWorkerRootW,vmLoaderWorkerIdA
 jmp .result
.apply:
 stdcall LoaderChangeApply,vmLoaderWorkerRootW,vmLoaderWorkerIdA,vmLoaderTargetA
.result:
 mov [vmLoaderResult],eax
.post:
 invoke PostMessageW,[hWnd],WM_FINISHED,0,0
 mov eax,[vmLoaderResult]
 ret
endp
proc VmLoaderFinish uses ebx
 mov ebx,[vmLoaderAction]
 mov [vmLoaderAction],0
 test ebx,ebx
 jz .done
 cmp ebx,1
 jne .context
 cmp [vmLoaderResult],1
 jne .clear
 stdcall VmLoaderFill
 jmp .context
.clear:
 mov [vmLoaderCount],0
 mov [vmLoaderSelected],-1
 invoke SendMessageW,[hVmLoaderList],LB_RESETCONTENT,0,0
.context:
 cmp [vmActive],1
 jne .done
 cmp ebx,1
 je .show
 stdcall VersionPageRefresh
.show:
 stdcall VmLoaderOpen
 cmp [vmTab],3
 jne .done
 stdcall VmLoaderShow
.done:
 ret
endp
