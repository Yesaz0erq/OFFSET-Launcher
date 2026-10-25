; Native version browser and installation queue. All runtime logic is assembly.
section '.veruid' data readable writeable
versionTab dd 0
versionUiAction dd 0
vUiKind dd 0
vUiVisible dd 0
hVCFilter dd 0
hVCQuery dd 0
hVCList dd 0
hVLoader dd 0
hVLoaderVersion dd 0
hVInstalled dd 0
hVQueue dd 0
vUiControls dd 7 dup(0)
vUiFabric du 'Fabric',0
vUiForge du 'Forge',0
vUiNeoForge du 'NeoForge',0
vUiQuilt du 'Quilt',0
vUiNames dd sVVanilla,vUiFabric,vUiForge,vUiNeoForge,vUiQuilt
vUiStates dd sVPending,sVRunning,sVComplete,sVFailed,sVCancelled
vUiBase rb 128
vUiQuery rb 256
vUiMap rd VC_CAPACITY
vUiSelections rd VQ_CAPACITY
vUiText rw 2048
vUiDetail rw 1024
vUiDate rw 256
vUiTarget rb 128

section '.veruic' code readable executable
    nop
proc CreateVersionControls uses esi
    stdcall NewControl,comboClass,CBS_DROPDOWNLIST+WS_VSCROLL,301,280,196,128,160
    mov [hVCFilter],eax
    mov [vUiControls],eax
    invoke SendMessageW,eax,CB_ADDSTRING,0,sVRelease
    invoke SendMessageW,[hVCFilter],CB_ADDSTRING,0,sVSnapshot
    invoke SendMessageW,[hVCFilter],CB_ADDSTRING,0,sVAll
    invoke SendMessageW,[hVCFilter],CB_SETCURSEL,0,0
    stdcall NewControl,editClass,ES_AUTOHSCROLL,302,438,200,222,24
    mov [hVCQuery],eax
    mov [vUiControls+4],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,100,0
    invoke SendMessageW,[hVCQuery],1501h,0,sVSearchCue
    stdcall NewControl,listClass,LBS_NOTIFY+LBS_EXTENDEDSEL+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,303,268,250,402,348
    mov [hVCList],eax
    mov [vUiControls+8],eax
    stdcall NewControl,comboClass,CBS_DROPDOWNLIST+WS_VSCROLL,304,728,290,332,200
    mov [hVLoader],eax
    mov [vUiControls+12],eax
    xor esi,esi
.kinds:
    invoke SendMessageW,[hVLoader],CB_ADDSTRING,0,dword [vUiNames+esi*4]
    inc esi
    cmp esi,5
    jb .kinds
    invoke SendMessageW,[hVLoader],CB_SETCURSEL,0,0
    stdcall NewControl,comboClass,CBS_DROPDOWNLIST+WS_VSCROLL,305,728,380,332,240
    mov [hVLoaderVersion],eax
    mov [vUiControls+16],eax
    stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,306,272,205,794,330
    mov [hVInstalled],eax
    mov [vUiControls+20],eax
    stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,307,272,205,794,330
    mov [hVQueue],eax
    mov [vUiControls+24],eax
    ret
endp

proc HideVersionControls uses esi
    xor esi,esi
.next:
    invoke ShowWindow,dword [vUiControls+esi*4],SW_HIDE
    inc esi
    cmp esi,7
    jb .next
    ret
endp

proc EnableVersionControls uses esi,enabled
    xor esi,esi
.next:
    invoke EnableWindow,dword [vUiControls+esi*4],[enabled]
    inc esi
    cmp esi,7
    jb .next
    ret
endp

proc ShowVersionControls
    cmp [versionTab],1
    je .installed
    cmp [versionTab],2
    je .queue
    invoke ShowWindow,[hVCFilter],SW_SHOW
    invoke ShowWindow,[hVCQuery],SW_SHOW
    invoke ShowWindow,[hVCList],SW_SHOW
    invoke ShowWindow,[hVLoader],SW_SHOW
    cmp [vUiKind],0
    je .catalogReady
    invoke ShowWindow,[hVLoaderVersion],SW_SHOW
.catalogReady:
    cmp [versionCatalogReady],0
    jne .done
    cmp [busy],0
    jne .done
    mov [versionUiAction],1
    stdcall StartWorker,VersionWorker
    ret
.installed:
    stdcall VersionFillInstalled
    invoke ShowWindow,[hVInstalled],SW_SHOW
    ret
.queue:
    stdcall VersionFillQueue
    invoke ShowWindow,[hVQueue],SW_SHOW
.done:
    ret
endp

proc VersionWorker,param
    cmp [versionUiAction],1
    je .catalog
    cmp [versionUiAction],2
    je .loaders
    stdcall VersionQueueRun,0
    ret
.catalog:
    stdcall VersionsFetch
    jmp .done
.loaders:
    stdcall LoaderFetchVersions,vUiBase,[vUiKind]
.done:
    invoke PostMessageW,[hWnd],WM_FINISHED,0,0
    ret
endp

proc VersionStatusW,message
    invoke lstrcpyW,statusText,[message]
    invoke InvalidateRect,[hWnd],0,0
    ret
endp

; ASCII-insensitive substring matching preserves punctuation in version IDs.
proc VersionContains uses ebx esi edi,source,query
    mov esi,[source]
    mov edi,[query]
    cmp byte [edi],0
    je .yes
.start:
    cmp byte [esi],0
    je .no
    xor ecx,ecx
.compare:
    mov al,[edi+ecx]
    test al,al
    jz .yes
    mov dl,[esi+ecx]
    test dl,dl
    jz .no
    cmp al,'A'
    jb @f
    cmp al,'Z'
    ja @f
    add al,32
@@:
    cmp dl,'A'
    jb @f
    cmp dl,'Z'
    ja @f
    add dl,32
@@:
    cmp al,dl
    jne .advance
    inc ecx
    jmp .compare
.advance:
    inc esi
    jmp .start
.yes:
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

proc VersionFillCatalog uses ebx esi edi
    local filter:DWORD
    invoke SendMessageW,[hVCFilter],CB_GETCURSEL,0,0
    mov [filter],eax
    invoke GetWindowTextA,[hVCQuery],vUiQuery,256
    invoke SendMessageW,[hVCList],WM_SETREDRAW,0,0
    invoke SendMessageW,[hVCList],LB_RESETCONTENT,0,0
    mov [vUiVisible],0
    mov byte [vUiBase],0
    xor esi,esi
.next:
    cmp esi,[versionCount]
    jae .finish
    imul ebx,esi,VC_SIZE
    add ebx,versionRecords
    cmp [filter],2
    je .query
    mov eax,[filter]
    cmp eax,[ebx+VC_TYPE]
    jne .skip
.query:
    stdcall VersionContains,ebx,vUiQuery
    test eax,eax
    jz .skip
    invoke MultiByteToWideChar,65001,0,ebx,-1,vUiText,512
    test eax,eax
    jz .skip
    lea eax,[ebx+VC_DATE]
    invoke MultiByteToWideChar,65001,0,eax,-1,vUiDate,256
    mov word [vUiDate+20],0
    invoke lstrcpyW,vUiDetail,vUiText
    cinvoke wsprintfW,vUiText,sVCatalogRow,vUiDetail,vUiDate
    invoke SendMessageW,[hVCList],LB_ADDSTRING,0,vUiText
    mov eax,[vUiVisible]
    mov [vUiMap+eax*4],esi
    inc [vUiVisible]
.skip:
    inc esi
    jmp .next
.finish:
    cmp [vUiVisible],0
    je .paint
    invoke SendMessageW,[hVCList],LB_SETSEL,1,0
    invoke SendMessageW,[hVCList],LB_SETCARETINDEX,0,0
    mov eax,[vUiMap]
    imul eax,VC_SIZE
    add eax,versionRecords
    invoke lstrcpyA,vUiBase,eax
.paint:
    invoke SendMessageW,[hVCList],WM_SETREDRAW,1,0
    invoke InvalidateRect,[hVCList],0,1
    invoke InvalidateRect,[hWnd],0,0
    ret
endp

proc VersionFillLoaderVersions uses ebx esi
    invoke SendMessageW,[hVLoaderVersion],CB_RESETCONTENT,0,0
    xor esi,esi
.next:
    cmp esi,[loaderVersionCount]
    jae .finish
    imul ebx,esi,VL_SIZE
    add ebx,loaderVersions
    invoke MultiByteToWideChar,65001,0,ebx,-1,vUiText,512
    invoke SendMessageW,[hVLoaderVersion],CB_ADDSTRING,0,vUiText
    inc esi
    jmp .next
.finish:
    cmp [loaderVersionCount],0
    je .paint
    invoke SendMessageW,[hVLoaderVersion],CB_SETCURSEL,0,0
.paint:
    invoke InvalidateRect,[hWnd],0,0
    ret
endp

proc VersionFillInstalled uses esi
    local count:DWORD
    cmp [busy],0
    jne .done
    stdcall EngineScanVersions,[hVersion]
    invoke SetWindowTextA,[hVersion],selectedVersion
    invoke SendMessageW,[hVInstalled],LB_RESETCONTENT,0,0
    invoke SendMessageW,[hVersion],CB_GETCOUNT,0,0
    mov [count],eax
    xor esi,esi
.next:
    cmp esi,[count]
    jae .done
    invoke SendMessageW,[hVersion],CB_GETLBTEXT,esi,vUiText
    invoke SendMessageW,[hVInstalled],LB_ADDSTRING,0,vUiText
    inc esi
    jmp .next
.done:
    ret
endp

proc VersionFillQueue uses ebx esi edi
    invoke SendMessageW,[hVQueue],WM_SETREDRAW,0,0
    invoke SendMessageW,[hVQueue],LB_RESETCONTENT,0,0
    xor esi,esi
.next:
    cmp esi,[versionQueueCount]
    jae .done
    imul ebx,esi,VQ_SIZE
    add ebx,versionQueue
    invoke MultiByteToWideChar,65001,0,ebx,-1,vUiDetail,512
    lea eax,[ebx+VQ_VERSION]
    invoke MultiByteToWideChar,65001,0,eax,-1,vUiDate,64
    mov eax,[ebx+VQ_LOADER]
    cmp eax,4
    ja .skip
    mov edi,[vUiNames+eax*4]
    mov eax,[ebx+VQ_STATE]
    cmp eax,4
    ja .skip
    cinvoke wsprintfW,vUiText,sVQueueRow,vUiDetail,edi,vUiDate,dword [vUiStates+eax*4]
    invoke SendMessageW,[hVQueue],LB_ADDSTRING,0,vUiText
.skip:
    inc esi
    jmp .next
.done:
    invoke SendMessageW,[hVQueue],WM_SETREDRAW,1,0
    invoke InvalidateRect,[hVQueue],0,1
    invoke InvalidateRect,[hWnd],0,0
    ret
endp

proc VersionFinishUI uses ebx
    mov ebx,[versionUiAction]
    mov [versionUiAction],0
    cmp ebx,1
    je .catalog
    cmp ebx,2
    je .loaders
    cmp ebx,3
    jne .done
    stdcall VersionFillInstalled
    stdcall VersionFillQueue
    jmp .done
.catalog:
    mov [vUiKind],0
    mov [loaderVersionCount],0
    invoke ShowWindow,[hVLoaderVersion],SW_HIDE
    invoke SendMessageW,[hVLoader],CB_SETCURSEL,0,0
    invoke SendMessageW,[hVLoaderVersion],CB_RESETCONTENT,0,0
    stdcall VersionFillCatalog
    jmp .done
.loaders:
    stdcall VersionFillLoaderVersions
.done:
    ret
endp

proc VersionSelectBase
    invoke SendMessageW,[hVCList],LB_GETCARETINDEX,0,0
    cmp eax,[vUiVisible]
    jae .done
    mov eax,[vUiMap+eax*4]
    imul eax,VC_SIZE
    add eax,versionRecords
    invoke lstrcpyA,vUiBase,eax
    invoke SendMessageW,[hVLoaderVersion],CB_RESETCONTENT,0,0
    mov [loaderVersionCount],0
    cmp [vUiKind],0
    je .paint
    mov [versionUiAction],2
    stdcall StartWorker,VersionWorker
.paint:
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp

proc VersionCommand uses ebx,wparam
    cmp [currentPage],1
    jne .done
    cmp [busy],0
    jne .done
    mov eax,[wparam]
    mov edx,eax
    shr edx,16
    and eax,0FFFFh
    cmp eax,301
    je .filter
    cmp eax,302
    je .query
    cmp eax,303
    je .selected
    cmp eax,304
    je .loader
    jmp .done
.filter:
    cmp edx,CBN_SELCHANGE
    jne .done
    jmp .resetFilter
.query:
    cmp edx,EN_CHANGE
    jne .done
.resetFilter:
    mov [vUiKind],0
    invoke ShowWindow,[hVLoaderVersion],SW_HIDE
    invoke SendMessageW,[hVLoader],CB_SETCURSEL,0,0
    invoke SendMessageW,[hVLoaderVersion],CB_RESETCONTENT,0,0
    mov [loaderVersionCount],0
    stdcall VersionFillCatalog
    jmp .done
.selected:
    cmp edx,LBN_SELCHANGE
    jne .done
    stdcall VersionSelectBase
    jmp .done
.loader:
    cmp edx,CBN_SELCHANGE
    jne .done
    invoke SendMessageW,[hVLoader],CB_GETCURSEL,0,0
    cmp eax,4
    ja .done
    mov [vUiKind],eax
    mov edx,SW_HIDE
    test eax,eax
    jz .loaderVisible
    mov edx,SW_SHOW
.loaderVisible:
    invoke ShowWindow,[hVLoaderVersion],edx
    stdcall VersionSelectBase
.done:
    ret
endp

proc VersionAddSelected uses ebx esi edi
    locals
        count dd ?
        loader dd ?
        additions dd ?
    endl
    mov [loader],0
    invoke SendMessageW,[hVCList],LB_GETSELCOUNT,0,0
    cmp eax,1
    jb .noSelection
    cmp eax,VQ_CAPACITY
    ja .tooMany
    mov [count],eax
    cmp [vUiKind],0
    je .selection
    cmp eax,1
    jne .oneBase
    invoke SendMessageW,[hVLoaderVersion],CB_GETCURSEL,0,0
    cmp eax,[loaderVersionCount]
    jae .noLoader
    imul eax,VL_SIZE
    add eax,loaderVersions
    mov [loader],eax
.selection:
    invoke SendMessageW,[hVCList],LB_GETSELITEMS,VQ_CAPACITY,vUiSelections
    cmp eax,[count]
    jne .bad
    ; Read-only preflight: capacity errors must never commit a partial batch.
    mov [additions],0
    xor esi,esi
.preflight:
    cmp esi,[count]
    jae .capacity
    mov eax,[vUiSelections+esi*4]
    cmp eax,[vUiVisible]
    jae .bad
    mov eax,[vUiMap+eax*4]
    imul eax,VC_SIZE
    lea ebx,[versionRecords+eax]
    mov eax,emptyW
    cmp [loader],0
    je .existing
    mov eax,[loader]
.existing:
    stdcall VersionUiQueueHas,ebx,[vUiKind],eax
    test eax,eax
    jnz .preflightNext
    ; A duplicate manifest row also must not reserve a second queue slot.
    xor edi,edi
.selectedBefore:
    cmp edi,esi
    jae .newItem
    mov eax,[vUiSelections+edi*4]
    mov eax,[vUiMap+eax*4]
    imul eax,VC_SIZE
    add eax,versionRecords
    invoke lstrcmpA,ebx,eax
    test eax,eax
    jz .preflightNext
    inc edi
    jmp .selectedBefore
.newItem:
    inc [additions]
.preflightNext:
    inc esi
    jmp .preflight
.capacity:
    mov eax,[versionQueueCount]
    add eax,[additions]
    cmp eax,VQ_CAPACITY
    ja .tooMany
    xor esi,esi
.next:
    cmp esi,[count]
    jae .success
    mov eax,[vUiSelections+esi*4]
    cmp eax,[vUiVisible]
    jae .bad
    mov eax,[vUiMap+eax*4]
    imul eax,VC_SIZE
    add eax,versionRecords
    mov ebx,emptyW
    cmp [loader],0
    je .add
    mov ebx,[loader]
.add:
    stdcall VersionQueueAdd,eax,[vUiKind],ebx
    test eax,eax
    jz .bad
    inc esi
    jmp .next
.success:
    stdcall VersionFillQueue
    stdcall VersionStatusW,sVAdded
    mov eax,1
    ret
.noSelection:
    stdcall VersionStatusW,sVChooseVersion
    jmp .bad
.tooMany:
    stdcall VersionStatusW,sVQueueLimit
    jmp .bad
.oneBase:
    stdcall VersionStatusW,sVOneBase
    jmp .bad
.noLoader:
    stdcall VersionStatusW,sVNoLoader
.bad:
    xor eax,eax
    ret
endp

proc VersionUiQueueHas uses ebx esi,base,kind,loader
    mov ebx,versionQueue
    xor esi,esi
.next:
    cmp esi,[versionQueueCount]
    jae .no
    mov eax,[kind]
    cmp eax,[ebx+VQ_LOADER]
    jne .advance
    invoke lstrcmpA,ebx,[base]
    test eax,eax
    jnz .advance
    cmp [kind],0
    je .yes
    lea eax,[ebx+VQ_VERSION]
    invoke lstrcmpA,eax,[loader]
    test eax,eax
    jz .yes
.advance:
    add ebx,VQ_SIZE
    inc esi
    jmp .next
.yes:
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

proc VersionStartQueue uses esi
    xor esi,esi
.next:
    cmp esi,[versionQueueCount]
    jae .empty
    imul eax,esi,VQ_SIZE
    cmp dword [versionQueue+eax+VQ_STATE],0
    je .start
    inc esi
    jmp .next
.start:
    mov [versionTab],2
    stdcall HideVersionControls
    stdcall ShowVersionControls
    mov [versionUiAction],3
    stdcall StartWorker,VersionWorker
    ret
.empty:
    stdcall VersionStatusW,sVNoPending
    ret
endp

proc VersionInstalledTarget
    invoke SendMessageW,[hVInstalled],LB_GETCURSEL,0,0
    cmp eax,-1
    je .bad
    invoke SendMessageW,[hVInstalled],LB_GETTEXT,eax,vUiText
    invoke WideCharToMultiByte,65001,0,vUiText,-1,vUiTarget,128,0,0
    test eax,eax
    jz .bad
    mov eax,1
    ret
.bad:
    stdcall VersionStatusW,sVChooseVersion
    xor eax,eax
    ret
endp

proc PaintVersionManager uses ebx,dc
    stdcall PageHeader,eyebrowVersions,sVersionHeading,2
    cinvoke wsprintfW,vUiText,sVTasksTab,[versionQueueCount]
    mov [segLabels],sVDownloadTab
    mov [segLabels+4],sVInstalledTab
    mov [segLabels+8],vUiText
    stdcall Segmented,252,124,450,42,3,2,segLabels,[versionTab],[fBody]
    mov eax,sRefresh
    cmp [busy],0
    je .refresh
    mov eax,sVCancel
.refresh:
    stdcall Button,[dc],eax,916,124,172,42,0
    cmp [versionTab],1
    je .installed
    cmp [versionTab],2
    je .queue
    stdcall Card,252,185,434,426,18
    stdcall Field,428,194,240,36,[hVCQuery]
    stdcall LineA,272,240,666,240,A_DIVIDER,10
    stdcall Card,704,185,384,426,18
    stdcall CornerTicks,704,185,384,426,[themeCrossARGB]
    stdcall TextDraw,monoSelected,729,203,300,14,[fMonoSmall],T_MUTED,DT_LEFT,1
    invoke MultiByteToWideChar,65001,0,vUiBase,-1,vUiText,512
    stdcall Text,[dc],vUiText,728,218,332,34,[fTitle],C_INK
    stdcall Text,[dc],sVLoaderLabel,728,262,332,24,[fSmall],C_MUTED
    cmp [vUiKind],0
    je .buttons
    stdcall Text,[dc],sVLoaderVersion,728,352,332,24,[fSmall],C_MUTED
    cmp [loaderVersionCount],0
    jne .buttons
    cmp [busy],0
    jne .buttons
    stdcall Text,[dc],sVNoLoader,728,418,332,24,[fSmall],C_MUTED
.buttons:
    stdcall Button,[dc],sVAddQueue,728,459,332,48,0
    stdcall Button,[dc],sVDownloadNow,728,529,332,52,1
    jmp .footer
.installed:
    stdcall Card,252,185,836,426,18
    stdcall CornerTicks,252,185,836,426,[themeCrossARGB]
    stdcall LineA,272,550,1066,550,A_DIVIDER,10
    stdcall Button,[dc],sVSetCurrent,280,563,220,40,1
    stdcall Button,[dc],sVOpenVersion,520,563,220,40,0
    stdcall Button,[dc],sVmManage,760,563,296,40,0
    jmp .footer
.queue:
    stdcall Card,252,185,836,426,18
    stdcall CornerTicks,252,185,836,426,[themeCrossARGB]
    stdcall LineA,272,550,1066,550,A_DIVIDER,10
    stdcall Button,[dc],sVStart,280,563,180,40,1
    stdcall Button,[dc],sVRetry,478,563,180,40,0
    stdcall Button,[dc],sVRemove,676,563,180,40,0
    stdcall Button,[dc],sVClearDone,874,563,182,40,0
.footer:
    stdcall StatusLine,254,676,830,32
    stdcall BusyBar,254,664,834
    ret
endp

proc ClickVersionManager uses ebx esi edi,x,y
    cmp [busy],0
    je .tabs
    cmp [y],124
    jb .done
    cmp [y],166
    ja .done
    cmp [x],252
    jb .done
    cmp [x],702
    jb .tabs
    cmp [y],124
    jb .done
    cmp [y],166
    ja .done
    cmp [x],916
    jb .done
    cmp [x],1088
    ja .done
    mov [engineCancel],1
    jmp .done
.tabs:
    cmp [y],124
    jb .done
    cmp [y],166
    ja .body
    cmp [x],252
    jb .done
    cmp [x],702
    jae .refresh
    mov eax,[x]
    sub eax,252
    xor edx,edx
    mov ecx,150
    div ecx
    cmp eax,[versionTab]
    je .paint
    mov [versionTab],eax
    stdcall StartTransition
    stdcall SyncPage
    jmp .paint
.refresh:
    cmp [x],916
    jb .done
    cmp [x],1088
    ja .done
    cmp [versionTab],0
    jne .refreshLocal
    mov [versionUiAction],1
    stdcall StartWorker,VersionWorker
    jmp .done
.refreshLocal:
    stdcall VersionFillInstalled
    stdcall VersionFillQueue
    jmp .done
.body:
    cmp [busy],0
    jne .done
    cmp [versionTab],1
    je .installed
    cmp [versionTab],2
    je .queue
    cmp [x],728
    jb .done
    cmp [x],1060
    ja .done
    cmp [y],459
    jb .done
    cmp [y],507
    jbe .add
    cmp [y],529
    jb .done
    cmp [y],581
    ja .done
    stdcall VersionAddSelected
    test eax,eax
    jz .done
    stdcall VersionStartQueue
    jmp .done
.add:
    stdcall VersionAddSelected
    jmp .done
.installed:
    cmp [y],563
    jb .done
    cmp [y],603
    ja .done
    cmp [x],280
    jb .done
    cmp [x],500
    jbe .selectCurrent
    cmp [x],520
    jb .done
    cmp [x],740
    jbe .openVersion
    cmp [x],760
    jb .done
    cmp [x],1056
    ja .done
    stdcall OpenInstalledVersionPage
    jmp .done
.openVersion:
    stdcall VersionInstalledTarget
    test eax,eax
    jz .done
    invoke lstrcpyW,textBuffer,gameDir
    invoke lstrcatW,textBuffer,eVersions
    invoke MultiByteToWideChar,65001,0,vUiTarget,-1,vUiText,512
    invoke lstrcatW,textBuffer,vUiText
    invoke ShellExecuteW,[hWnd],openVerb,textBuffer,0,0,SW_SHOWNORMAL
    jmp .done
.selectCurrent:
    stdcall VersionInstalledTarget
    test eax,eax
    jz .done
    invoke lstrcpyA,selectedVersion,vUiTarget
    invoke SetWindowTextA,[hVersion],selectedVersion
    stdcall VersionStatusW,sVCurrentSet
    jmp .done
.queue:
    cmp [y],563
    jb .done
    cmp [y],603
    ja .done
    cmp [x],280
    jb .done
    cmp [x],460
    jbe .start
    cmp [x],478
    jb .done
    cmp [x],658
    jbe .retry
    cmp [x],676
    jb .done
    cmp [x],856
    jbe .remove
    cmp [x],874
    jb .done
    cmp [x],1056
    ja .done
    mov esi,[versionQueueCount]
.clearLoop:
    test esi,esi
    jz .queuePaint
    dec esi
    imul eax,esi,VQ_SIZE
    cmp dword [versionQueue+eax+VQ_STATE],2
    jne .clearLoop
    stdcall VersionQueueRemove,esi
    jmp .clearLoop
.retry:
    xor esi,esi
.retryLoop:
    cmp esi,[versionQueueCount]
    jae .start
    imul eax,esi,VQ_SIZE
    cmp dword [versionQueue+eax+VQ_STATE],3
    jb .retryNext
    mov dword [versionQueue+eax+VQ_STATE],0
.retryNext:
    inc esi
    jmp .retryLoop
.start:
    stdcall VersionStartQueue
    jmp .done
.remove:
    invoke SendMessageW,[hVQueue],LB_GETCURSEL,0,0
    cmp eax,[versionQueueCount]
    jae .done
    stdcall VersionQueueRemove,eax
.queuePaint:
    stdcall VersionFillQueue
.paint:
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp
