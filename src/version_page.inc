; Installed-version workspace. Opening a page never selects or launches a game.
section '.vmpd' data readable writeable
vmActive dd 0
vmTab dd 0
vmDraftIsolate dd 0
hVmName dd 0
hVmJava dd 0
hVmMemory dd 0
vmEyebrow du 'VERSION / MANAGE',0
vmSlash du '\',0
vmModsSuffix du '\mods',0
vmSavesSuffix du '\saves',0
vmLogsSuffix du '\logs',0
vmResourcesSuffix du '\resourcepacks',0
vmFolderSuffixes dd 0,0,vmModsSuffix,vmSavesSuffix,vmLogsSuffix,vmResourcesSuffix
vmRootW rw 1024
vmIdA rb 128
vmDataDirW rw 2048
vmTitleW rw 256
vmBaseW rw 128
vmTextW rw 2048
vmMemoryText rw 16

section '.vmpc' code readable executable
 nop

proc CreateVersionPageControls
 stdcall NewControl,editClass,ES_AUTOHSCROLL,601,452,206,598,24
 mov [hVmName],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,36,0
 stdcall NewControl,editClass,ES_AUTOHSCROLL,602,452,276,598,24
 mov [hVmJava],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,1000,0
 stdcall NewControl,editClass,ES_NUMBER+ES_AUTOHSCROLL,603,452,386,176,24
 mov [hVmMemory],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,10,0
 stdcall CreateVmModsControls
 ret
endp

proc HideVersionPageControls
 invoke ShowWindow,[hVmName],SW_HIDE
 invoke ShowWindow,[hVmJava],SW_HIDE
 invoke ShowWindow,[hVmMemory],SW_HIDE
 stdcall VmModsHide
 ret
endp

proc EnableVersionPageControls,enabled
 invoke EnableWindow,[hVmName],[enabled]
 invoke EnableWindow,[hVmJava],[enabled]
 invoke EnableWindow,[hVmMemory],[enabled]
 stdcall VmModsEnable,[enabled]
 ret
endp

proc VersionPageFillControls
 invoke SetWindowTextW,[hVmName],vsNameW
 invoke SetWindowTextW,[hVmJava],vsJavaW
 cinvoke wsprintfW,vmMemoryText,fmtNum,[vsMemory]
 invoke SetWindowTextW,[hVmMemory],vmMemoryText
 mov eax,[vsIsolate]
 mov [vmDraftIsolate],eax
 ret
endp

proc VersionPageUpdateContext
 invoke lstrcpyW,vmRootW,vsRootW
 invoke lstrcpyA,vmIdA,vsIdA
 invoke lstrcpyW,vmDataDirW,vsDataDirW
 invoke MultiByteToWideChar,65001,8,vmIdA,-1,vmTitleW,256
 cmp word [vsNameW],0
 je .base
 invoke lstrcpyW,vmTitleW,vsNameW
.base:
 invoke MultiByteToWideChar,65001,8,vsBaseVersionA,-1,vmBaseW,128
 ret
endp

proc VersionPageOpen,rootW,idA
 cmp [busy],0
 jne .bad
 stdcall VsLoad,[rootW],[idA]
 test eax,eax
 jz .unavailable
 stdcall VersionPageUpdateContext
 stdcall VersionPageFillControls
 mov [vmActive],1
 mov [vmTab],0
 mov [currentPage],1
 stdcall StartTransition
 stdcall SyncPage
 mov eax,1
 ret
.unavailable:
 mov [vmActive],0
 mov [versionTab],1
 stdcall VersionStatusW,sVmUnavailable
 stdcall SyncPage
.bad:
 xor eax,eax
 ret
endp

proc OpenInstalledVersionPage
 stdcall VersionInstalledTarget
 test eax,eax
 jz .done
 stdcall VersionPageOpen,gameDir,vUiTarget
.done:
 ret
endp

proc VersionPageRefresh
 cmp [vmActive],0
 je .done
 cmp [busy],0
 jne .done
 stdcall VsLoad,vmRootW,vmIdA
 test eax,eax
 jz .unavailable
 stdcall VersionPageUpdateContext
 stdcall VersionPageFillControls
 jmp .done
.unavailable:
 mov [vmActive],0
 stdcall VersionStatusW,sVmUnavailable
 stdcall SyncPage
.done:
 ret
endp

proc ShowVersionPageControls
 cmp [vmActive],0
 je .done
 cmp [vmTab],1
 je .settings
 cmp [vmTab],2
 jne .done
 stdcall VmModsShow
 ret
.settings:
 invoke ShowWindow,[hVmName],SW_SHOW
 invoke ShowWindow,[hVmJava],SW_SHOW
 invoke ShowWindow,[hVmMemory],SW_SHOW
.done:
 ret
endp

proc VersionPageFinishUI
 stdcall VmModsFinishUI
 ret
endp

; Read UI drafts only on explicit save. A failed save restores the loaded model.
proc VersionPageSave uses ebx esi edi
 locals
  savedName rw 40
  savedJava rw 1024
  savedMemory dd ?
  savedIsolation dd ?
  parsed dd ?
 endl
 invoke GetWindowTextW,[hVmMemory],vmMemoryText,16
 stdcall MemoryParseW,vmMemoryText
 test eax,eax
 jnz .nonzero
 cmp word [vmMemoryText],'0'
 jne .invalid
 cmp word [vmMemoryText+2],0
 jne .invalid
 jmp .memory
.nonzero:
 mov [parsed],eax
 stdcall MemoryValid,eax
 test eax,eax
 jz .invalid
 mov eax,[parsed]
.memory:
 mov [parsed],eax
 invoke lstrcpyW,addr savedName,vsNameW
 invoke lstrcpyW,addr savedJava,vsJavaW
 mov eax,[vsMemory]
 mov [savedMemory],eax
 mov eax,[vsIsolate]
 mov [savedIsolation],eax
 invoke GetWindowTextW,[hVmName],vsNameW,40
 invoke GetWindowTextW,[hVmJava],vsJavaW,1024
 mov eax,[parsed]
 mov [vsMemory],eax
 mov eax,[vmDraftIsolate]
 mov [vsIsolate],eax
 stdcall VsSave
 test eax,eax
 jz .restore
 stdcall VersionPageUpdateContext
 stdcall VersionPageFillControls
 stdcall VersionStatusW,sVmSaved
 mov eax,1
 ret
.restore:
 invoke lstrcpyW,vsNameW,addr savedName
 invoke lstrcpyW,vsJavaW,addr savedJava
 mov eax,[savedMemory]
 mov [vsMemory],eax
 mov eax,[savedIsolation]
 mov [vsIsolate],eax
.invalid:
 stdcall VersionStatusW,sVmInvalid
 xor eax,eax
 ret
endp

; Open a persisted version path, rejecting a substituted reparse directory.
proc VersionPageOpenFolder uses ebx,pathKind
 mov ebx,[pathKind]
 cmp ebx,5
 ja .done
 stdcall VsLoad,vmRootW,vmIdA
 test eax,eax
 jz .unavailable
 stdcall VersionPageUpdateContext
 mov eax,vmDataDirW
 cmp ebx,1
 jne .copy
 mov eax,vsProfileDirW
.copy:
 invoke lstrcpyW,vmTextW,eax
 cmp ebx,2
 jb .ensure
 invoke lstrcatW,vmTextW,dword [vmFolderSuffixes+ebx*4]
.ensure:
 stdcall EnsureDirectory,vmTextW
 test eax,eax
 jz .unavailable
 invoke GetFileAttributesW,vmTextW
 cmp eax,-1
 je .unavailable
 test eax,400h
 jnz .unavailable
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .unavailable
 invoke ShellExecuteW,[hWnd],openVerb,vmTextW,0,0,SW_SHOWNORMAL
 ret
.unavailable:
 stdcall VersionStatusW,sVmUnavailable
.done:
 ret
endp

proc VersionPageCommand,wparam
 cmp [currentPage],1
 jne .done
 cmp [busy],0
 jne .done
 mov eax,[wparam]
 mov edx,eax
 shr edx,16
 and eax,0FFFFh
 cmp [vmActive],0
 jne .mods
 cmp eax,306
 jne .done
 cmp edx,LBN_DBLCLK
 jne .done
 stdcall OpenInstalledVersionPage
 ret
.mods:
 cmp [vmTab],2
 jne .done
 stdcall VmModsCommand,[wparam]
.done:
 ret
endp

proc PaintVersionPage uses ebx,dc
 stdcall PageHeader,vmEyebrow,vmTitleW,2
 mov [segLabels],sVmOverview
 mov [segLabels+4],sVmSettings
 mov [segLabels+8],sVmMods
 stdcall Segmented,252,124,450,42,3,2,segLabels,[vmTab],[fBody]
 mov eax,sVmBack
 cmp [busy],0
 je @f
 mov eax,sModCancel
@@:
 stdcall Button,[dc],eax,916,124,172,42,0
 cmp [vmTab],1
 je .settings
 cmp [vmTab],2
 jne .overview
 stdcall VmModsPaint,[dc]
 ret
.overview:
 stdcall Card,252,185,836,170,18
 stdcall Text,[dc],sVmBase,280,202,154,26,[fSmall],C_MUTED
 stdcall Text,[dc],vmBaseW,440,201,620,28,[fMedium],C_INK
 stdcall Text,[dc],sVmLoader,280,244,154,26,[fSmall],C_MUTED
 mov eax,[vsLoaderKind]
 cmp eax,4
 jbe @f
 xor eax,eax
@@:
 stdcall Text,[dc],dword [vUiNames+eax*4],440,243,620,28,[fMedium],C_INK
 stdcall Text,[dc],sVmDataDir,280,286,154,26,[fSmall],C_MUTED
 stdcall TextDraw,vmDataDirW,440,286,620,26,[fSmall],C_INK,DT_LEFT+DT_PATH_ELLIPSIS,0
 stdcall Card,252,372,490,230,18
 stdcall Text,[dc],sVmJava,280,390,430,24,[fSmall],C_MUTED
 mov eax,vsJavaW
 cmp word [vsJavaW],0
 jne @f
 mov eax,javaPath
@@:
 stdcall TextDraw,eax,280,422,430,42,[fBody],C_INK,DT_LEFT+DT_PATH_ELLIPSIS,0
 stdcall Text,[dc],sVmMemory,280,477,430,26,[fSmall],C_MUTED
 mov eax,[vsMemory]
 test eax,eax
 jnz @f
 mov eax,[memoryMb]
@@:
 cinvoke wsprintfW,vmTextW,fmtRam,eax
 stdcall Text,[dc],vmTextW,280,510,430,30,[fMedium],C_INK
 stdcall Card,762,372,326,230,18
 stdcall Button,[dc],sLaunch,786,390,278,44,1
 stdcall Button,[dc],sVmUse,786,450,278,40,0
 stdcall Button,[dc],sVmRepair,786,502,278,40,0
 stdcall Button,[dc],sPackExportTab,786,552,278,40,0
 stdcall Button,[dc],sVmDataDir,252,620,128,40,0
 stdcall Button,[dc],sVmVersionDir,392,620,128,40,0
 stdcall Button,[dc],sVmModsDir,532,620,128,40,0
 stdcall Button,[dc],sVmSaves,672,620,128,40,0
 stdcall Button,[dc],sVmLogs,812,620,128,40,0
 stdcall Button,[dc],sVmResources,952,620,136,40,0
 jmp .footer
.settings:
 stdcall Card,252,185,836,428,18
 stdcall Text,[dc],sVmName,280,206,154,26,[fSmall],C_MUTED
 stdcall Field,440,200,624,38,[hVmName]
 stdcall Text,[dc],sVmJava,280,276,154,26,[fSmall],C_MUTED
 stdcall Field,440,270,624,38,[hVmJava]
 stdcall Text,[dc],sVmInherit,440,315,624,24,[fSmall],C_MUTED
 stdcall Text,[dc],sVmMemory,280,350,760,26,[fSmall],C_MUTED
 stdcall Field,440,380,200,38,[hVmMemory]
 cinvoke wsprintfW,vmTextW,sVmMemoryRange,[memoryLimitMb]
 stdcall Text,[dc],vmTextW,660,386,400,26,[fSmall],C_MUTED
 stdcall Text,[dc],sVmIsolation,280,450,154,26,[fSmall],C_MUTED
 xor eax,eax
 cmp [vmDraftIsolate],0
 setz al
 stdcall Button,[dc],sVmShared,440,440,140,42,eax
 xor eax,eax
 cmp [vmDraftIsolate],1
 setz al
 stdcall Button,[dc],sVmIsolated,598,440,140,42,eax
 stdcall Text,[dc],sVmIsolationNote,280,510,760,42,[fSmall],C_MUTED
 stdcall Button,[dc],sVmSave,252,620,200,42,1
.footer:
 stdcall StatusLine,254,676,830,32
 stdcall BusyBar,254,664,834
 ret
endp

proc ClickVersionPage uses ebx,x,y
 cmp [y],124
 jb .done
 cmp [y],166
 ja .body
 cmp [x],916
 jb .tabs
 cmp [x],1088
 ja .done
 cmp [busy],0
 jne .cancel
 mov [vmActive],0
 mov [versionTab],1
 stdcall StartTransition
 stdcall SyncPage
 ret
.cancel:
 mov [engineCancel],1
 mov [modCancel],1
 ret
.tabs:
 cmp [busy],0
 jne .done
 cmp [x],252
 jb .done
 cmp [x],702
 jae .done
 mov eax,[x]
 sub eax,252
 xor edx,edx
 mov ecx,150
 div ecx
 cmp eax,[vmTab]
 je .done
 mov [vmTab],eax
 stdcall StartTransition
 stdcall SyncPage
 ret
.body:
 cmp [busy],0
 jne .done
 cmp [vmTab],2
 jne .notMods
 stdcall VmModsClick,[x],[y]
 ret
.notMods:
 cmp [vmTab],1
 je .settings
 cmp [y],620
 jb .overviewActions
 cmp [y],660
 ja .done
 cmp [x],252
 jb .done
 cmp [x],1088
 ja .done
 mov eax,[x]
 sub eax,252
 xor edx,edx
 mov ecx,140
 div ecx
 cmp eax,5
 je .folderCall
 cmp edx,128
 jae .done
.folderCall:
 stdcall VersionPageOpenFolder,eax
 ret
.overviewActions:
 cmp [x],786
 jb .done
 cmp [x],1064
 ja .done
 cmp [y],390
 jb .done
 cmp [y],434
 jbe .launch
 cmp [y],450
 jb .done
 cmp [y],490
 jbe .use
 cmp [y],502
 jb .done
 cmp [y],542
 jbe .repair
 cmp [y],552
 jb .done
 cmp [y],592
 ja .done
 stdcall PackExportUiOpen,vmRootW,vmIdA
 ret
.repair:
 stdcall VersionPageRepair
 ret
.launch:
 stdcall VersionPageLaunch
 ret
.use:
 stdcall VersionPageUse
 ret
.settings:
 cmp [y],440
 jb .save
 cmp [y],482
 ja .save
 cmp [x],440
 jb .done
 cmp [x],580
 jbe .shared
 cmp [x],598
 jb .done
 cmp [x],738
 ja .done
 mov [vmDraftIsolate],1
 jmp .redraw
.shared:
 mov [vmDraftIsolate],0
 jmp .redraw
.save:
 cmp [y],620
 jb .done
 cmp [y],662
 ja .done
 cmp [x],252
 jb .done
 cmp [x],452
 ja .done
 stdcall VersionPageSave
 ret
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
