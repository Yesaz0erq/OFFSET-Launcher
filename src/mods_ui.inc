; Original native mod browser UI. Provider/manager data is kept outside the UI.
section '.modui' data readable writeable
modApiConsole du 'https://console.curseforge.com/',0
modDllFolder du '\mods',0
modUiDefault db '1.21.1',0
modUiKeyMessage db '请选择一个项目或文件。',0

section '.moduc' code readable executable
proc CreateModControls uses ebx esi
    stdcall NewControl,editClass,ES_AUTOHSCROLL,204,628,232,262,24
    mov [hModQuery],eax
    mov [modControls],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,120,0
    invoke SendMessageW,[hModQuery],1501h,0,sModQueryCue
    stdcall NewControl,editClass,ES_AUTOHSCROLL,205,288,232,122,24
    mov [hModGame],eax
    mov [modControls+4],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,48,0
    invoke SetWindowTextA,[hModGame],modGameVersion
    stdcall NewControl,comboClass,CBS_DROPDOWNLIST+WS_VSCROLL,206,440,230,150,180
    mov [hModLoader],eax
    mov [modControls+8],eax
    xor esi,esi
.loaders:
    invoke SendMessageA,[hModLoader],CB_ADDSTRING,0,dword [modLoaderNames+esi*4]
    inc esi
    cmp esi,4
    jb .loaders
    invoke SendMessageW,[hModLoader],CB_SETCURSEL,0,0
    stdcall NewControl,listClass,LBS_NOTIFY+LBS_OWNERDRAWFIXED+LBS_HASSTRINGS+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,207,272,369,374,224
    mov [hModProjects],eax
    mov [modControls+12],eax
    stdcall Scale,54
    invoke SendMessageW,[hModProjects],LB_SETITEMHEIGHT,0,eax
    stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,208,674,369,390,160
    mov [hModFiles],eax
    mov [modControls+16],eax
    stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,209,272,262,794,280
    mov [hModLocal],eax
    mov [modControls+20],eax
    stdcall NewControl,editClass,ES_AUTOHSCROLL+ES_PASSWORD,210,288,290,744,26
    mov [hCurseKey],eax
    mov [modControls+24],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,255,0
    invoke SetWindowTextA,[hCurseKey],curseApiKey
    ret
endp

proc HideModControls uses esi
    xor esi,esi
.loop:
    invoke ShowWindow,dword [modControls+esi*4],SW_HIDE
    inc esi
    cmp esi,7
    jb .loop
    ret
endp

proc EnableModControls uses esi,enabled
    xor esi,esi
.loop:
    invoke EnableWindow,dword [modControls+esi*4],[enabled]
    inc esi
    cmp esi,7
    jb .loop
    ret
endp

proc ShowModControls
    stdcall ModSyncSelectedProfile
    cmp [modTab],1
    jbe .tabValid
    mov [modTab],0
.tabValid:
    cmp [modTab],0
    je .discover
    cmp [modTab],1
    je .local
    invoke ShowWindow,[hCurseKey],SW_SHOW
    ret
.discover:
    invoke ShowWindow,[hModQuery],SW_SHOW
    invoke ShowWindow,[hModGame],SW_SHOW
    invoke ShowWindow,[hModLoader],SW_SHOW
    invoke ShowWindow,[hModProjects],SW_SHOW
    invoke ShowWindow,[hModFiles],SW_SHOW
    ret
.local:
    cmp [busy],0
    jne .showLocal
    stdcall ModsScan
    stdcall RefreshLocalList
.showLocal:
    invoke ShowWindow,[hModLocal],SW_SHOW
    ret
endp

proc ClearModResults
    mov [modProjectCount],0
    mov [modFileCount],0
    mov [modTotal],0
    mov [modSelectedProject],-1
    mov [modSelectedFile],-1
    invoke SendMessageW,[hModProjects],LB_RESETCONTENT,0,0
    invoke SendMessageW,[hModFiles],LB_RESETCONTENT,0,0
    invoke InvalidateRect,[hWnd],0,0
    ret
endp

proc ReadModInputs uses esi
    invoke GetWindowTextW,[hModQuery],modUiText,512
    invoke WideCharToMultiByte,65001,0,modUiText,-1,modQuery,512,0,0
    test eax,eax
    jz .bad
    invoke GetWindowTextA,[hModGame],modGameVersion,64
    test eax,eax
    jz .bad
    mov esi,modGameVersion
.version:
    lodsb
    test al,al
    jz .loader
    cmp al,'0'
    jb .punct
    cmp al,'9'
    jbe .version
    cmp al,'A'
    jb .punct
    cmp al,'Z'
    jbe .version
    cmp al,'a'
    jb .punct
    cmp al,'z'
    jbe .version
.punct:
    cmp al,'.'
    je .version
    cmp al,'-'
    je .version
    cmp al,'_'
    je .version
    jmp .bad
.loader:
    invoke SendMessageW,[hModLoader],CB_GETCURSEL,0,0
    cmp eax,3
    ja .bad
    invoke lstrcpyA,modLoader,dword [modLoaderNames+eax*4]
    invoke GetWindowTextA,[hCurseKey],curseApiKey,256
    mov eax,1
    ret
.bad:
    invoke lstrcpyW,statusText,sModBadFilter
    invoke InvalidateRect,[hWnd],0,0
    xor eax,eax
    ret
endp

proc BeginModSearch
    cmp [busy],0
    jne .done
if defined OFFSET_BROWSE_DEFAULTS
    mov [modDefaultAttempted],1
end if
    stdcall ReadModInputs
    test eax,eax
    jz .done
    stdcall ClearModResults
    mov [modAction],1
    stdcall StartWorker,ModWorker
.done:
    ret
endp

proc ModWorker uses ebx,param
    mov ebx,[modAction]
    cmp ebx,1
    je .search
    cmp ebx,2
    je .versions
    cmp ebx,3
    je .download
    cmp ebx,4
    je .toggle
    stdcall ModsTrash,[modSelectedLocal]
    jmp .finish
.toggle:
    stdcall ModsToggle,[modSelectedLocal]
    jmp .finish
.download:
    stdcall ModsInstall,[modSelectedFile]
    jmp .finish
.search:
    stdcall ModrinthSearch
    jmp .finish
.versions:
    stdcall ModrinthVersions
.finish:
    invoke PostMessageW,[hWnd],WM_FINISHED,0,0
    ret
endp

proc ModFinishUI uses ebx esi edi
    cmp [modAction],0
    je .done
    cmp [modAction],1
    je .projects
    cmp [modAction],2
    je .files
    stdcall ModsScan
    stdcall RefreshLocalList
    jmp .reset
.projects:
    invoke SendMessageW,[hModProjects],LB_RESETCONTENT,0,0
    xor esi,esi
.projectLoop:
    cmp esi,[modProjectCount]
    jae .reset
    imul ebx,esi,MP_SIZE
    add ebx,modProjects+MP_TITLE
    invoke MultiByteToWideChar,65001,0,ebx,-1,modUiText,2048
    invoke SendMessageW,[hModProjects],LB_ADDSTRING,0,modUiText
    inc esi
    jmp .projectLoop
.files:
    invoke SendMessageW,[hModFiles],LB_RESETCONTENT,0,0
    xor esi,esi
.fileLoop:
    cmp esi,[modFileCount]
    jae .selectFile
    imul ebx,esi,MF_SIZE
    add ebx,modFiles+MF_TITLE
    invoke MultiByteToWideChar,65001,0,ebx,-1,modUiText,2048
    invoke SendMessageW,[hModFiles],LB_ADDSTRING,0,modUiText
    inc esi
    jmp .fileLoop
.selectFile:
    cmp [modFileCount],0
    je .reset
    mov [modSelectedFile],0
    invoke SendMessageW,[hModFiles],LB_SETCURSEL,0,0
.reset:
    mov [modAction],0
.done:
    ret
endp

proc RefreshLocalList uses ebx esi edi
    invoke SendMessageW,[hModLocal],LB_RESETCONTENT,0,0
    mov [modSelectedLocal],-1
    xor esi,esi
.loop:
    cmp esi,[modLocalCount]
    jae .done
    imul ebx,esi,ML_SIZE
    add ebx,modLocals
    mov eax,sModOnPrefix
    cmp dword [ebx+ML_DISABLED],0
    je .prefix
    mov eax,sModOffPrefix
.prefix:
    invoke lstrcpyW,modUiText,eax
    invoke lstrcatW,modUiText,ebx
    invoke SendMessageW,[hModLocal],LB_ADDSTRING,0,modUiText
    inc esi
    jmp .loop
.done:
    ret
endp

proc ModCommand uses ebx esi,wparam
    cmp [detailActive],0
    jne .done
    cmp [currentPage],4
    jne .done
    cmp [busy],0
    jne .done
    mov eax,[wparam]
    mov edx,eax
    shr edx,16
    and eax,0FFFFh
    cmp eax,205
    je .gameFilter
    cmp eax,206
    je .loaderFilter
    cmp eax,207
    je .project
    cmp eax,208
    je .file
    cmp eax,209
    je .local
    jmp .done
.gameFilter:
    cmp edx,EN_CHANGE
    jne .done
    jmp .clear
.loaderFilter:
    cmp edx,CBN_SELCHANGE
    jne .done
.clear:
    mov [modOffset],0
    stdcall ClearModResults
    jmp .done
.project:
    cmp edx,LBN_SELCHANGE
    jne .done
    invoke SendMessageW,[hModProjects],LB_GETCURSEL,0,0
    cmp eax,[modProjectCount]
    jae .done
    mov [modSelectedProject],eax
    mov [modSelectedFile],-1
    mov [modFileCount],0
    invoke SendMessageW,[hModFiles],LB_RESETCONTENT,0,0
    stdcall ReadModInputs
    test eax,eax
    jz .done
    mov [modAction],2
    stdcall StartWorker,ModWorker
    jmp .done
.file:
    cmp edx,LBN_SELCHANGE
    jne .done
    invoke SendMessageW,[hModFiles],LB_GETCURSEL,0,0
    mov [modSelectedFile],eax
    jmp .redraw
.local:
    cmp edx,LBN_SELCHANGE
    jne .done
    invoke SendMessageW,[hModLocal],LB_GETCURSEL,0,0
    mov [modSelectedLocal],eax
.redraw:
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp

proc DrawModProject uses ebx esi edi,info
    locals
        topY dd ?
        rowWidth dd ?
        savedGfx dd ?
        savedDC dd ?
        savedX dd ?
        savedY dd ?
    endl
    mov ebx,[info]
    cmp dword [ebx+4],207
    jne .done
    ; Owner-drawn rows reuse the canvas kit on the list's own DC.
    mov eax,[gfx]
    mov [savedGfx],eax
    mov eax,[gfxDC]
    mov [savedDC],eax
    mov eax,[gOffX]
    mov [savedX],eax
    mov eax,[gOffY]
    mov [savedY],eax
    mov [gOffX],0
    mov [gOffY],0
    invoke MulDiv,dword [ebx+32],96,[dpi]
    mov [topY],eax
    mov ecx,[ebx+36]
    sub ecx,[ebx+28]
    invoke MulDiv,ecx,96,[dpi]
    mov [rowWidth],eax
    stdcall GfxBegin,dword [ebx+24]
    stdcall FillRR,0,[topY],[rowWidth],54,0,A_CARD
    test dword [ebx+16],ODS_SELECTED
    jz .content
    mov eax,[topY]
    add eax,2
    mov ecx,[rowWidth]
    sub ecx,6
    stdcall FillRR,2,eax,ecx,50,10,[themeAccentSoft]
    mov eax,[topY]
    add eax,14
    stdcall FillRR,6,eax,3,26,1,[themeAccentARGB]
.content:
    mov eax,[ebx+8]
    cmp eax,[modProjectCount]
    jae .restore
    imul esi,eax,MP_SIZE
    add esi,modProjects
    lea eax,[esi+MP_TITLE]
    invoke MultiByteToWideChar,65001,0,eax,-1,modUiText,2048
    mov eax,[topY]
    add eax,6
    mov edx,[rowWidth]
    sub edx,34
    stdcall Text,0,modUiText,16,eax,edx,24,[fBodyStrong],C_INK
    lea eax,[esi+MP_DESC]
    invoke MultiByteToWideChar,65001,0,eax,-1,modUiDetail,2048
    mov eax,[topY]
    add eax,29
    mov edx,[rowWidth]
    sub edx,34
    stdcall Text,0,modUiDetail,16,eax,edx,20,[fSmall],C_MUTED
.restore:
    stdcall GfxEnd
    mov eax,[savedGfx]
    mov [gfx],eax
    mov eax,[savedDC]
    mov [gfxDC],eax
    mov eax,[savedX]
    mov [gOffX],eax
    mov eax,[savedY]
    mov [gOffY],eax
.done:
    ret
endp

proc PaintMods uses ebx esi,dc
    cmp [detailActive],0
    je .normal
    stdcall PaintProjectDetail,[dc]
    ret
.normal:
    stdcall PageHeader,eyebrowMods,sModHeading,5
    mov [segLabels],sModDiscover
    mov [segLabels+4],sModLocal
    stdcall Segmented,252,124,450,42,2,3,segLabels,[modTab],[fBody]
    cmp [modTab],0
    je .discover
    cmp [modTab],1
    je .local
    stdcall PaintModApi,[dc]
    jmp .footer
.discover:
    stdcall Card,252,185,836,123,18
    stdcall Text,[dc],sModGame,280,195,150,26,[fSmall],C_MUTED
    stdcall Text,[dc],sModLoader,440,195,160,26,[fSmall],C_MUTED
    stdcall Text,[dc],sModKeyword,620,195,270,26,[fSmall],C_MUTED
    stdcall Field,278,224,142,40,[hModGame]
    stdcall Field,618,224,282,40,[hModQuery]
    mov eax,sModSearch
    cmp [busy],0
    je .searchButton
    mov eax,sModCancel
.searchButton:
    stdcall Button,[dc],eax,916,223,148,42,1
    stdcall Text,[dc],sModLoaderNote,280,276,775,23,[fSmall],C_MUTED
    stdcall Card,252,328,406,278,18
    stdcall Card,670,328,418,278,18
    mov eax,[modOffset]
    xor edx,edx
    mov ecx,20
    div ecx
    inc eax
    cinvoke wsprintfW,modUiText,modCountFormat,[modTotal],eax
    stdcall Text,[dc],modUiText,278,337,365,25,[fSmall],C_MUTED
    stdcall Text,[dc],sModFiles,690,337,370,25,[fMedium],C_INK
    mov eax,[modSelectedFile]
    cmp eax,[modFileCount]
    jae .noVersion
    imul ebx,eax,MF_SIZE
    add ebx,modFiles
    cmp dword [ebx+MF_REQUIRED],0
    jne .hasDependencies
    stdcall Text,[dc],sModNoDeps,690,533,374,30,[fSmall],C_MUTED
    jmp .dependencyLink
.hasDependencies:
    lea eax,[ebx+MF_DEPS]
    invoke MultiByteToWideChar,65001,0,eax,-1,modUiText,2048
    stdcall Text,[dc],modUiText,690,533,374,30,[fSmall],C_MUTED
.dependencyLink:
    stdcall HotAdd,690,565,370,26
    stdcall HoverLevel,eax
    mov ecx,[themeAccentText]
    test eax,eax
    jz @f
    stdcall Mix,[themeAccentText],T_INK,eax
    mov ecx,eax
@@:
    stdcall Text,[dc],sModDependencies,690,565,370,26,[fSmall],ecx
    jmp .buttons
.noVersion:
    stdcall Text,[dc],sModSelectPrompt,690,538,374,40,[fSmall],C_MUTED
.buttons:
    stdcall Button,[dc],sModPrev,252,616,92,38,0
    stdcall Button,[dc],sModNext,352,616,92,38,0
    stdcall Button,[dc],sDetailHeading,452,616,96,38,0
    stdcall Button,[dc],sDetailMcmod,556,616,102,38,0
    stdcall Button,[dc],sModWebsite,670,612,172,44,0
    mov eax,sModDownload
    mov ecx,[modSelectedFile]
    cmp ecx,[modFileCount]
    jae .downloadButton
    imul ecx,MF_SIZE
    cmp dword [modFiles+ecx+MF_BLOCKED],0
    je .downloadButton
    mov eax,sModOfficialOnly
.downloadButton:
    stdcall Button,[dc],eax,862,612,226,44,1
    stdcall StatusLine,254,676,830,32
    stdcall BusyBar,254,664,834
    ret
.local:
    stdcall Card,252,185,836,428,18
    stdcall CornerTicks,252,185,836,428,[themeCrossARGB]
    cinvoke wsprintfW,modUiText,modLocalFormat,[modLocalCount]
    stdcall Text,[dc],modUiText,280,199,760,28,[fTitle],C_INK
    stdcall ModsDataRoot,modUiText,2048
    test eax,eax
    jz .localButtons
    stdcall TextDraw,modUiText,281,230,776,22,[fMonoSmall],T_MUTED,DT_LEFT+DT_PATH_ELLIPSIS,0
.localButtons:
    stdcall LineA,272,550,1066,550,A_DIVIDER,10
    stdcall Button,[dc],sModRescan,280,563,168,40,0
    stdcall Button,[dc],sModToggle,464,563,168,40,0
    stdcall Button,[dc],sModTrash,648,563,168,40,0
    stdcall Button,[dc],sModOpenFolder,832,563,224,40,1
    stdcall Text,[dc],sModTrashHelp,254,624,830,30,[fSmall],C_MUTED
.footer:
    stdcall StatusLine,254,676,830,32
    stdcall BusyBar,254,664,834
    ret
endp

proc PaintModApi dc
    stdcall Card,252,185,836,343,18
    stdcall CornerTicks,252,185,836,343,[themeCrossARGB]
    stdcall Text,[dc],sModApiHeading,280,205,764,34,[fTitle],C_INK
    stdcall Text,[dc],sModKeyLabel,280,251,764,26,[fSmall],C_MUTED
    stdcall Field,278,282,768,42,[hCurseKey]
    stdcall Button,[dc],sModSaveKey,280,351,180,44,1
    stdcall Button,[dc],sModGetKey,480,351,210,44,0
    stdcall LineA,278,410,1060,410,A_DIVIDER,10
    stdcall Text,[dc],sModKeyHelp,280,420,765,30,[fSmall],C_MUTED
    stdcall Text,[dc],sModKeyHelp2,280,450,765,30,[fSmall],C_MUTED
    stdcall Text,[dc],sModSourceNote,254,548,830,42,[fBody],C_MUTED
    ret
endp

proc OpenModWebsite uses ebx
    mov eax,[modSelectedProject]
    cmp eax,[modProjectCount]
    jae .none
    imul ebx,eax,MP_SIZE
    add ebx,modProjects+MP_URL
    cmp byte [ebx],0
    je .none
    invoke MultiByteToWideChar,65001,0,ebx,-1,modUiText,2048
    test eax,eax
    jz .none
    invoke ShellExecuteW,[hWnd],openVerb,modUiText,0,0,SW_SHOWNORMAL
    ret
.none:
    stdcall SetStatusA,modUiKeyMessage
    ret
endp

proc ClickMods uses ebx esi edi,x,y
    cmp [detailActive],0
    je .normal
    stdcall ClickProjectDetail,[x],[y]
    ret
.normal:
    cmp [busy],0
    je .tabs
    cmp [y],124
    jb .done
    cmp [y],166
    ja .busyCancel
    cmp [x],252
    jb .done
    cmp [x],702
    jbe .tabs
    jmp .done
.busyCancel:
if defined OFFSET_BROWSE_DEFAULTS
    cmp [workerCallback],ModWorker
    jne .done
end if
    cmp [y],223
    jb .done
    cmp [y],265
    ja .done
    cmp [x],916
    jb .done
    mov [modCancel],1
    jmp .done
.tabs:
    cmp [y],124
    jb .done
    cmp [y],166
    ja .body
    cmp [x],702
    ja .done
    cmp [x],252
    jb .done
    mov eax,[x]
    sub eax,252
    xor edx,edx
    mov ecx,225
    div ecx
    cmp eax,[modTab]
    je .redraw
    mov [modTab],eax
    stdcall StartTransition
    stdcall SyncPage
    jmp .redraw
.body:
    cmp [busy],0
    jne .done
    cmp [modTab],0
    je .discover
    cmp [modTab],1
    je .local
    cmp [y],351
    jb .done
    cmp [y],395
    ja .done
    cmp [x],460
    ja .getKey
    cmp [x],280
    jb .done
    invoke GetWindowTextA,[hCurseKey],curseApiKey,256
    stdcall ModsSaveKey
    jmp .redraw
.getKey:
    cmp [x],480
    jb .done
    cmp [x],690
    ja .done
    invoke ShellExecuteW,[hWnd],openVerb,modApiConsole,0,0,SW_SHOWNORMAL
    jmp .done
.discover:
    cmp [y],223
    jb .done
    cmp [y],265
    ja .lower
    cmp [x],916
    jb .done
    mov [modOffset],0
    stdcall BeginModSearch
    jmp .done
.lower:
    cmp [y],565
    jb .done
    cmp [y],592
    ja .bottom
    cmp [x],690
    jb .done
    mov eax,[modSelectedFile]
    cmp eax,[modFileCount]
    jae .done
    imul ebx,eax,MF_SIZE
    cmp dword [modFiles+ebx+MF_REQUIRED],0
    jne .showDependencies
    invoke lstrcpyW,modUiText,sModNoDeps
    jmp .dependencyDialog
.showDependencies:
    lea eax,[modFiles+ebx+MF_DEPS]
    invoke MultiByteToWideChar,65001,0,eax,-1,modUiText,2048
.dependencyDialog:
    invoke MessageBoxW,[hWnd],modUiText,sModDependencies,MB_OK+MB_ICONINFORMATION
    jmp .done
.bottom:
    cmp [y],612
    jb .done
    cmp [y],656
    ja .done
    cmp [x],342
    jbe .prev
    cmp [x],442
    jbe .next
    cmp [x],452
    jb .done
    cmp [x],548
    jbe .details
    cmp [x],556
    jb .done
    cmp [x],658
    jbe .mcmod
    cmp [x],670
    jb .done
    cmp [x],842
    jbe .website
    cmp [x],862
    jb .done
    mov eax,[modSelectedFile]
    cmp eax,[modFileCount]
    jae .done
    imul ebx,eax,MF_SIZE
    cmp dword [modFiles+ebx+MF_BLOCKED],0
    jne .website
    mov [modAction],3
    stdcall StartWorker,ModWorker
    jmp .done
.website:
    stdcall OpenModWebsite
    jmp .done
.details:
    mov eax,[modSelectedProject]
    cmp eax,[modProjectCount]
    jae .done
    imul eax,MP_SIZE
    add eax,modProjects
    stdcall DetailOpen,eax,[modProvider],0
    ret
.mcmod:
    mov eax,[modSelectedProject]
    cmp eax,[modProjectCount]
    jae .done
    imul eax,MP_SIZE
    add eax,modProjects+MP_TITLE
    stdcall OpenMcmodQuery,eax
    ret
.prev:
    cmp [modOffset],20
    jb .done
    sub [modOffset],20
    stdcall BeginModSearch
    jmp .done
.next:
    mov eax,[modOffset]
    add eax,20
    cmp eax,[modTotal]
    jae .done
    mov [modOffset],eax
    stdcall BeginModSearch
    jmp .done
.local:
    cmp [y],563
    jb .done
    cmp [y],603
    ja .done
    cmp [x],280
    jb .done
    cmp [x],448
    jbe .scan
    cmp [x],832
    jae .folder
    mov eax,[modSelectedLocal]
    cmp eax,[modLocalCount]
    jae .done
    cmp [x],632
    jbe .toggle
    cmp [x],648
    jb .done
    cmp [x],816
    ja .done
    invoke MessageBoxW,[hWnd],sModTrashQuestion,sModTrash,MB_YESNO+MB_ICONQUESTION+MB_DEFBUTTON2
    cmp eax,IDYES
    jne .done
    mov [modAction],5
    stdcall StartWorker,ModWorker
    jmp .done
.toggle:
    cmp [x],464
    jb .done
    mov [modAction],4
    stdcall StartWorker,ModWorker
    jmp .done
.scan:
    stdcall ModsScan
    stdcall RefreshLocalList
    jmp .redraw
.folder:
    stdcall ModsDataRoot,modUiText,2048
    test eax,eax
    jz .done
    invoke lstrcatW,modUiText,modDllFolder
    stdcall EnsureDirectory,modUiText
    invoke ShellExecuteW,[hWnd],openVerb,modUiText,0,0,SW_SHOWNORMAL
    jmp .done
.redraw:
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp
