; Per-version local mod view. Explicit scope never changes the launch selection.
section '.vmodd' data readable writeable
hVmModsList dd 0
vmModsAction dd 0
vmModsSelected dd -1
vmModsWorkerRootW rw 2048
vmModsWide rw 2048

section '.vmodc' code readable executable
    nop
proc CreateVmModsControls
    stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,621,272,250,794,286
    mov [hVmModsList],eax
    ret
endp
proc VmModsHide
    invoke ShowWindow,[hVmModsList],SW_HIDE
    ret
endp
proc VmModsEnable,enabled
    invoke EnableWindow,[hVmModsList],[enabled]
    ret
endp
proc VmModsRefresh uses ebx esi
    mov [vmModsSelected],-1
    mov [modLocalCount],0
    invoke SendMessageW,[hVmModsList],LB_RESETCONTENT,0,0
    stdcall ModsSetDataRoot,vmDataDirW
    test eax,eax
    jz .clear
    stdcall ModsScan
    xor esi,esi
.loop:
    cmp esi,[modLocalCount]
    jae .clear
    imul ebx,esi,ML_SIZE
    add ebx,modLocals
    mov eax,sModOnPrefix
    cmp dword [ebx+ML_DISABLED],0
    je .prefix
    mov eax,sModOffPrefix
.prefix:
    invoke lstrcpyW,vmModsWide,eax
    invoke lstrcatW,vmModsWide,ebx
    invoke SendMessageW,[hVmModsList],LB_ADDSTRING,0,vmModsWide
    inc esi
    jmp .loop
.clear:
    stdcall ModsSetDataRoot,0
    ret
endp
proc VmModsShow
    stdcall VmModsRefresh
    invoke ShowWindow,[hVmModsList],SW_SHOW
    ret
endp
proc VmModsPaint,dc
    stdcall Card,252,185,836,428,18
    cinvoke wsprintfW,vmModsWide,modLocalFormat,[modLocalCount]
    stdcall Text,[dc],vmModsWide,280,197,760,28,[fTitle],C_INK
    stdcall TextDraw,vmDataDirW,280,230,776,20,[fMonoSmall],T_MUTED,DT_LEFT+DT_PATH_ELLIPSIS,0
    stdcall LineA,272,550,1066,550,A_DIVIDER,10
    stdcall Button,[dc],sModRescan,280,563,168,40,0
    stdcall Button,[dc],sModToggle,464,563,168,40,0
    stdcall Button,[dc],sModTrash,648,563,168,40,0
    stdcall Button,[dc],sModOpenFolder,832,563,224,40,0
    stdcall Button,[dc],sModDiscover,862,620,226,40,1
    stdcall Text,[dc],sModTrashHelp,254,620,580,34,[fSmall],C_MUTED
    stdcall StatusLine,254,676,830,32
    stdcall BusyBar,254,664,834
    ret
endp
proc VmModsCommand,wparam
    cmp [vmActive],0
    je .done
    cmp [vmTab],2
    jne .done
    cmp [busy],0
    jne .done
    mov eax,[wparam]
    mov edx,eax
    shr edx,16
    and eax,0FFFFh
    cmp eax,621
    jne .done
    cmp edx,LBN_SELCHANGE
    jne .done
    invoke SendMessageW,[hVmModsList],LB_GETCURSEL,0,0
    cmp eax,[modLocalCount]
    jae .done
    mov [vmModsSelected],eax
.done:
    ret
endp
proc VmModsWorker uses ebx,param
    stdcall ModsSetDataRoot,vmModsWorkerRootW
    test eax,eax
    jz .finish
    mov eax,[vmModsSelected]
    cmp [vmModsAction],1
    jne .trash
    stdcall ModsToggle,eax
    jmp .finish
.trash:
    cmp [vmModsAction],2
    jne .finish
    stdcall ModsTrash,eax
.finish:
    stdcall ModsSetDataRoot,0
    invoke PostMessageW,[hWnd],WM_FINISHED,0,0
    ret
endp
proc VmModsFinishUI
    cmp [vmModsAction],0
    je .done
    mov [vmModsAction],0
    stdcall ModsSetDataRoot,0
    cmp [vmActive],0
    je .done
    cmp [vmTab],2
    jne .done
    stdcall VmModsRefresh
.done:
    ret
endp
proc VmModsClick,x,y
    cmp [busy],0
    jne .done
    cmp [y],563
    jb .done
    cmp [y],603
    ja .download
    cmp [x],280
    jb .done
    cmp [x],448
    jbe .scan
    cmp [x],464
    jb .done
    cmp [x],632
    jbe .toggle
    cmp [x],648
    jb .done
    cmp [x],816
    jbe .trash
    cmp [x],832
    jb .done
    cmp [x],1056
    ja .done
    stdcall ModsSetDataRoot,vmDataDirW
    test eax,eax
    jz .clear
    stdcall ModsDataRoot,vmModsWide,2048
    test eax,eax
    jz .clear
    invoke lstrcatW,vmModsWide,mlModsSuffix
    stdcall EnsureDirectory,vmModsWide
    test eax,eax
    jz .clear
    invoke ShellExecuteW,[hWnd],openVerb,vmModsWide,0,0,SW_SHOWNORMAL
.clear:
    stdcall ModsSetDataRoot,0
    ret
.scan:
    stdcall VmModsRefresh
    invoke InvalidateRect,[hWnd],0,0
    ret
.toggle:
    mov [vmModsAction],1
    jmp .start
.trash:
    mov eax,[vmModsSelected]
    cmp eax,[modLocalCount]
    jae .done
    invoke MessageBoxW,[hWnd],sModTrashQuestion,sModTrash,MB_YESNO+MB_ICONQUESTION+MB_DEFBUTTON2
    cmp eax,IDYES
    jne .done
    mov [vmModsAction],2
.start:
    mov eax,[vmModsSelected]
    cmp eax,[modLocalCount]
    jae .reset
    invoke lstrcpyW,vmModsWorkerRootW,vmDataDirW
    stdcall StartWorker,VmModsWorker
    cmp [busy],0
    jne .done
.reset:
    mov [vmModsAction],0
    ret
.download:
    cmp [y],620
    jb .done
    cmp [y],660
    ja .done
    cmp [x],862
    jb .done
    cmp [x],1088
    ja .done
    stdcall VersionPageBrowseMods
.done:
    ret
endp
