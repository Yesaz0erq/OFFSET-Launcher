; Local export workspace. Source selection never changes the launch selection.
section '.peuid' data readable writeable
hPeUiSource dd 0
hPeUiName dd 0
hPeUiVersion dd 0
hPeUiAuthor dd 0
hPeUiPreview dd 0
peUiControls dd 5 dup(0)
peUiInitialized dd 0
peUiQuiet dd 0
peUiReady dd 0
peUiFormat dd 1
peUiSaves dd 0
peUiMods dd 1
peUiConfig dd 1
peUiResources dd 1
peUiServers dd 0
peUiFiles dd 0
peUiBytes dd 0
peUiExported dd 0
peUiDefaultVersion du '1.0.0',0
peUiDefaultMr du 'OFFSET-pack.mrpack',0
peUiDefaultZip du 'OFFSET-pack.zip',0
peUiMrExt du 'mrpack',0
peUiZipExt du 'zip',0
peUiMrSuffix du '.mrpack',0
peUiZipSuffix du '.zip',0
peUiBadChars du '\/:*?"<>|',0
peUiMrFilter du 'Modrinth (*.mrpack)',0,'*.mrpack',0,0
peUiZipFilter du 'Minecraft modpacks (*.zip)',0,'*.zip',0,0
peUiSave OPENFILENAME
peUiRootW rw 1024
peUiCandidateRootW rw 1024
peUiVersionA rb 128
peUiListA rb 128
peUiWantedA rb 128
peUiVersionW rw 128
peUiListW rw 128
peUiNameW rw 128
peUiLabelW rw 64
peUiAuthorW rw 128
peUiDataDirW rw 2048
peUiDestW rw 2048
peUiPickW rw 2048
peUiScratchW rw 2048
peUiSummaryW rw 32768

section '.peuic' code readable executable
 nop
proc CreatePackExportControls
 stdcall NewControl,comboClass,CBS_DROPDOWNLIST+WS_VSCROLL,411,440,202,624,72
 mov [hPeUiSource],eax
 mov [peUiControls],eax
 stdcall NewControl,editClass,ES_AUTOHSCROLL,412,452,310,598,24
 mov [hPeUiName],eax
 mov [peUiControls+4],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,80,0
 stdcall NewControl,editClass,ES_AUTOHSCROLL,413,452,358,192,24
 mov [hPeUiVersion],eax
 mov [peUiControls+8],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,40,0
 stdcall NewControl,editClass,ES_AUTOHSCROLL,414,812,358,238,24
 mov [hPeUiAuthor],eax
 mov [peUiControls+12],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,80,0
 stdcall NewControl,editClass,ES_MULTILINE+ES_AUTOVSCROLL+ES_READONLY,415,280,500,782,56
 mov [hPeUiPreview],eax
 mov [peUiControls+16],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,32767,0
 ret
endp
proc HidePackExportControls uses esi
 xor esi,esi
.loop:
 invoke ShowWindow,dword [peUiControls+esi*4],SW_HIDE
 inc esi
 cmp esi,5
 jb .loop
 ret
endp
proc EnablePackExportControls uses esi,enabled
 xor esi,esi
.loop:
 invoke EnableWindow,dword [peUiControls+esi*4],[enabled]
 inc esi
 cmp esi,5
 jb .loop
 ret
endp
proc PackExportUiInvalidate
 mov [peUiReady],0
 mov [peUiFiles],0
 mov [peUiBytes],0
 mov [peUiExported],0
 mov word [peUiDestW],0
 invoke SetWindowTextW,[hPeUiPreview],sPackExportEmpty
 ret
endp
proc PackExportUiCanonical,path,destination
 invoke GetFullPathNameW,[path],1024,[destination],0
 test eax,eax
 jz .bad
 cmp eax,1024
 jae .bad
 mov edx,[destination]
.trim:
 cmp eax,3
 jbe .good
 cmp word [edx+eax*2-2],92
 jne .good
 dec eax
 mov word [edx+eax*2],0
 jmp .trim
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
; A version workspace can open export without selecting that version for launch.
proc PackExportUiOpen uses ebx,root,id
 locals
  sourceRoot rw 1024
  currentRoot rw 1024
  previousRoot rw 1024
  previousId rb 128
 endl
 cmp [busy],0
 jne .bad
 stdcall VsSafeId,[id]
 test eax,eax
 jz .bad
 stdcall PackExportUiCanonical,[root],addr sourceRoot
 test eax,eax
 jz .bad
 stdcall PackExportUiCanonical,gameDir,addr currentRoot
 test eax,eax
 jz .bad
 invoke lstrcmpiW,addr sourceRoot,addr currentRoot
 test eax,eax
 jnz .bad
 cmp [peUiInitialized],1
 jne .changed
 invoke lstrcmpiW,peUiRootW,addr sourceRoot
 test eax,eax
 jnz .changed
 invoke lstrcmpA,peUiVersionA,[id]
 test eax,eax
 jz .open
.changed:
 invoke MultiByteToWideChar,65001,8,[id],-1,peUiVersionW,128
 test eax,eax
 jz .bad
 invoke lstrcpyW,peUiRootW,addr sourceRoot
 invoke lstrcpyA,peUiVersionA,[id]
 mov [peUiInitialized],1
 invoke SetWindowTextW,[hPeUiName],peUiVersionW
 invoke SetWindowTextW,[hPeUiVersion],peUiDefaultVersion
 invoke SetWindowTextW,[hPeUiAuthor],emptyW
 stdcall PackExportUiInvalidate
 mov word [peUiDataDirW],0
 stdcall VsResolveData,peUiRootW,peUiVersionA,peUiDataDirW,2048
.open:
 invoke lstrcpyW,addr previousRoot,gameDir
 invoke lstrcpyA,addr previousId,selectedVersion
 mov [packTab],3
 stdcall GoPage,5
 invoke lstrcpyW,gameDir,addr previousRoot
 invoke lstrcpyA,selectedVersion,addr previousId
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PackExportUiSource uses ebx
 local changed:DWORD
 cmp [busy],0
 jne .done
 mov [changed],0
 mov [peUiQuiet],1
 stdcall PackExportUiCanonical,gameDir,peUiCandidateRootW
 test eax,eax
 jz .invalidRoot
 cmp [peUiInitialized],0
 je .current
 invoke lstrcmpiW,peUiRootW,peUiCandidateRootW
 test eax,eax
 jnz .current
 mov eax,peUiVersionA
 jmp .wanted
.current:
 mov [changed],1
 mov eax,selectedVersion
.wanted:
 invoke lstrcpyA,peUiWantedA,eax
 invoke lstrcpyW,peUiRootW,peUiCandidateRootW
 stdcall EngineScanVersions,[hPeUiSource]
 invoke SendMessageA,[hPeUiSource],CB_FINDSTRINGEXACT,-1,peUiWantedA
 cmp eax,-1
 jne .select
 xor eax,eax
.select:
 invoke SendMessageW,[hPeUiSource],CB_SETCURSEL,eax,0
 invoke GetWindowTextA,[hPeUiSource],peUiVersionA,128
 cmp [peUiInitialized],0
 je .initial
 cmp [changed],1
 je .invalidate
 invoke lstrcmpA,peUiVersionA,peUiWantedA
 test eax,eax
 jz .resolve
.invalidate:
 stdcall PackExportUiInvalidate
 jmp .resolve
.initial:
 mov [peUiInitialized],1
 invoke MultiByteToWideChar,65001,8,peUiVersionA,-1,peUiVersionW,128
 invoke SetWindowTextW,[hPeUiName],peUiVersionW
 invoke SetWindowTextW,[hPeUiVersion],peUiDefaultVersion
 stdcall PackExportUiInvalidate
.resolve:
 mov word [peUiDataDirW],0
 stdcall VsResolveData,peUiRootW,peUiVersionA,peUiDataDirW,2048
 mov [peUiQuiet],0
 jmp .done
.invalidRoot:
 mov [peUiQuiet],0
 mov byte [peUiVersionA],0
 mov word [peUiDataDirW],0
 stdcall PackExportUiInvalidate
.done:
 ret
endp
proc ShowPackExportControls uses esi
 stdcall PackExportUiSource
 cmp [peUiReady],0
 jne .show
 invoke SetWindowTextW,[hPeUiPreview],sPackExportEmpty
.show:
 xor esi,esi
.loop:
 invoke ShowWindow,dword [peUiControls+esi*4],SW_SHOW
 inc esi
 cmp esi,5
 jb .loop
 ; The source combo is kept as a data model; the visible list is drawn below.
 invoke ShowWindow,[hPeUiSource],SW_HIDE
 ret
endp

; First visible row of the 3-row source list, keeping the selection in view.
proc PackExportListTop
    invoke SendMessageA,[hPeUiSource],CB_GETCOUNT,0,0
    mov ecx,eax
    sub ecx,3
    jge @f
    xor ecx,ecx
@@:
    push ecx
    invoke SendMessageA,[hPeUiSource],CB_GETCURSEL,0,0
    pop ecx
    dec eax
    jge @f
    xor eax,eax
@@:
    cmp eax,ecx
    jle @f
    mov eax,ecx
@@:
    ret
endp

proc PaintPackExportVersionList uses ebx esi edi,dc
    local top:DWORD,count:DWORD
    stdcall FillRR,440,202,624,72,7,A_FIELD
    stdcall PackExportListTop
    mov [top],eax
    invoke SendMessageA,[hPeUiSource],CB_GETCOUNT,0,0
    mov [count],eax
    invoke SendMessageA,[hPeUiSource],CB_GETCURSEL,0,0
    mov edi,eax
    xor esi,esi
.row:
    cmp esi,3
    jae .done
    mov eax,esi
    add eax,[top]
    cmp eax,[count]
    jae .done
    invoke SendMessageA,[hPeUiSource],CB_GETLBTEXT,eax,peUiListA
    invoke MultiByteToWideChar,65001,0,peUiListA,-1,peUiListW,128
    ; row y lives in ebx: FillRR returns a value in eax
    mov ebx,esi
    imul ebx,24
    add ebx,204
    mov eax,esi
    add eax,[top]
    cmp eax,edi
    jne .plain
    stdcall FillRR,444,ebx,616,22,4,[themeAccentARGB]
    stdcall Text,0,peUiListW,452,ebx,596,22,[fBody],T_WHITE
    jmp .next
.plain:
    stdcall Text,0,peUiListW,452,ebx,596,22,[fBody],T_INK
.next:
    inc esi
    jmp .row
.done:
    ret
endp
proc PackExportUiCommand,wparam
 cmp [currentPage],5
 jne .done
 cmp [packTab],3
 jne .done
 cmp [busy],0
 jne .done
 cmp [peUiQuiet],0
 jne .done
 mov eax,[wparam]
 mov edx,eax
 shr edx,16
 and eax,0FFFFh
 cmp eax,411
 jne .done
 cmp edx,CBN_SELCHANGE
 jne .done
 invoke GetWindowTextA,[hPeUiSource],peUiVersionA,128
 stdcall PackExportUiInvalidate
 mov word [peUiDataDirW],0
 stdcall VsResolveData,peUiRootW,peUiVersionA,peUiDataDirW,2048
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc PackExportUiPreview
 cmp [busy],0
 jne .done
 cmp byte [peUiVersionA],0
 je .missing
 stdcall PackExportUiInvalidate
 mov eax,[peUiMods]
 mov [peIncludeMods],eax
 mov eax,[peUiConfig]
 mov [peIncludeConfig],eax
 mov eax,[peUiResources]
 mov [peIncludeResources],eax
 mov eax,[peUiServers]
 mov [peIncludeServers],eax
 mov [packUiResult],0
 mov [packUiAction],6
 stdcall StartWorker,PackWorker
 ret
.missing:
 invoke lstrcpyW,statusText,sVChooseVersion
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc PackExportUiFinish
 cmp [packUiAction],6
 jne .built
 mov [peUiReady],0
 cmp [packUiResult],1
 jne .show
 cmp [peReady],1
 jne .show
 invoke MultiByteToWideChar,65001,8,peSummaryA,-1,peUiSummaryW,32768
 test eax,eax
 jz .show
 invoke SetWindowTextW,[hPeUiPreview],peUiSummaryW
 invoke lstrcpyW,peUiDataDirW,peDataDirW
 mov eax,[peFileCount]
 mov [peUiFiles],eax
 mov eax,[peTotalBytes]
 mov [peUiBytes],eax
 mov [peUiReady],1
 invoke lstrcpyW,statusText,sReady
 jmp .show
.built:
 cmp [packUiAction],7
 jne .done
 mov [peUiExported],0
 cmp [packUiResult],1
 jne .show
 mov [peUiExported],1
 invoke MultiByteToWideChar,65001,8,peSummaryA,-1,peUiSummaryW,32768
 test eax,eax
 jz .show
 invoke SetWindowTextW,[hPeUiPreview],peUiSummaryW
 mov eax,[peFileCount]
 mov [peUiFiles],eax
 mov eax,[peTotalBytes]
 mov [peUiBytes],eax
 mov [peUiReady],1
.show:
 cmp [currentPage],5
 jne .done
 stdcall SyncPage
.done:
 ret
endp
; Only newly named files of the selected format can become export targets.
proc PackExportUiTarget,path
 invoke GetFullPathNameW,[path],2048,peUiScratchW,0
 test eax,eax
 jz .bad
 cmp eax,1900
 ja .bad
 mov edx,peUiZipSuffix
 mov ecx,4
 cmp [peUiFormat],1
 jne .suffix
 mov edx,peUiMrSuffix
 mov ecx,7
.suffix:
 cmp eax,ecx
 jb .bad
 sub eax,ecx
 lea eax,[peUiScratchW+eax*2]
 invoke lstrcmpiW,eax,edx
 test eax,eax
 jnz .bad
 invoke GetFileAttributesW,peUiScratchW
 cmp eax,-1
 jne .bad
 invoke GetLastError
 cmp eax,2
 je .good
 cmp eax,3
 jne .bad
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
; Returns 1 when a new, valid target was chosen.
proc PackExportUiChoose uses esi edi
 cmp [busy],0
 jne .no
 cmp word [peUiDestW],0
 je .suggest
 invoke lstrcpyW,peUiPickW,peUiDestW
 jmp .dialog
.suggest:
 ; Suggest "<pack name>.mrpack|.zip" with characters Windows forbids replaced.
 invoke GetWindowTextW,[hPeUiName],peUiPickW,200
 mov esi,peUiPickW
.clean:
 mov ax,[esi]
 test ax,ax
 jz .named
 cmp ax,32
 jb .replace
 mov edi,peUiBadChars
.bad:
 cmp word [edi],0
 je .nextChar
 cmp ax,[edi]
 je .replace
 add edi,2
 jmp .bad
.replace:
 mov word [esi],'-'
.nextChar:
 add esi,2
 jmp .clean
.named:
 cmp word [peUiPickW],0
 jne .suffix
 mov eax,peUiDefaultZip
 cmp [peUiFormat],1
 jne @f
 mov eax,peUiDefaultMr
@@:
 invoke lstrcpyW,peUiPickW,eax
 jmp .dialog
.suffix:
 mov eax,peUiZipSuffix
 cmp [peUiFormat],1
 jne @f
 mov eax,peUiMrSuffix
@@:
 invoke lstrcatW,peUiPickW,eax
.dialog:
 mov edi,peUiSave
 xor eax,eax
 mov ecx,sizeof.OPENFILENAME/4
 rep stosd
 mov [peUiSave.lStructSize],sizeof.OPENFILENAME
 mov eax,[hWnd]
 mov [peUiSave.hwndOwner],eax
 mov [peUiSave.lpstrFile],peUiPickW
 mov [peUiSave.nMaxFile],2048
 mov eax,peUiZipFilter
 mov edx,peUiZipExt
 cmp [peUiFormat],1
 jne @f
 mov eax,peUiMrFilter
 mov edx,peUiMrExt
@@:
 mov [peUiSave.lpstrFilter],eax
 mov [peUiSave.lpstrDefExt],edx
 mov [peUiSave.Flags],OFN_PATHMUSTEXIST+OFN_NOCHANGEDIR+OFN_EXPLORER
 invoke GetSaveFileNameW,peUiSave
 test eax,eax
 jz .no
 stdcall PackExportUiTarget,peUiPickW
 test eax,eax
 jz .invalid
 invoke lstrcpyW,peUiDestW,peUiScratchW
 mov [peUiExported],0
 invoke InvalidateRect,[hWnd],0,0
 mov eax,1
 ret
.invalid:
 invoke lstrcpyW,statusText,sPackExportDestinationBad
 invoke InvalidateRect,[hWnd],0,0
.no:
 xor eax,eax
 ret
endp
proc PackExportUiClean uses esi,text,allowEmpty
 mov esi,[text]
 cmp word [esi],0
 jne .scan
 mov eax,[allowEmpty]
 ret
.scan:
 lodsw
 test ax,ax
 jz .good
 cmp ax,32
 jb .bad
 cmp ax,127
 je .bad
 jmp .scan
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PackExportUiMetadata
 invoke GetWindowTextW,[hPeUiName],peUiNameW,128
 invoke GetWindowTextW,[hPeUiVersion],peUiLabelW,64
 cmp word [peUiLabelW],0
 jne @f
 invoke lstrcpyW,peUiLabelW,peUiDefaultVersion
@@:
 invoke GetWindowTextW,[hPeUiAuthor],peUiAuthorW,128
 stdcall PackExportUiClean,peUiNameW,0
 test eax,eax
 jz .bad
 stdcall PackExportUiClean,peUiLabelW,1
 test eax,eax
 jz .bad
 stdcall PackExportUiClean,peUiAuthorW,1
 test eax,eax
 jz .bad
 invoke WideCharToMultiByte,65001,80h,peUiNameW,-1,peNameA,256,0,0
 test eax,eax
 jz .bad
 invoke WideCharToMultiByte,65001,80h,peUiLabelW,-1,peVersionLabelA,128,0,0
 test eax,eax
 jz .bad
 invoke WideCharToMultiByte,65001,80h,peUiAuthorW,-1,peAuthorA,256,0,0
 test eax,eax
 jz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PackExportUiStart
 cmp [busy],0
 jne .done
 cmp byte [peUiVersionA],0
 je .missing
 cmp word [peUiDestW],0
 je .choose
 ; A previously chosen file may exist now (for example an earlier export).
 stdcall PackExportUiTarget,peUiDestW
 test eax,eax
 jnz .go
 mov word [peUiDestW],0
.choose:
 stdcall PackExportUiChoose
 test eax,eax
 jz .done
.go:
 stdcall PackExportUiMetadata
 test eax,eax
 jz .invalid
 mov eax,[peUiFormat]
 mov [peFormat],eax
 invoke lstrcpyW,peDestW,peUiDestW
 invoke lstrcpyW,peRootW,peUiRootW
 invoke lstrcpyA,peVersionA,peUiVersionA
 mov eax,[peUiSaves]
 mov [peIncludeSaves],eax
 mov eax,[peUiMods]
 mov [peIncludeMods],eax
 mov eax,[peUiConfig]
 mov [peIncludeConfig],eax
 mov eax,[peUiResources]
 mov [peIncludeResources],eax
 mov eax,[peUiServers]
 mov [peIncludeServers],eax
 mov [peUiExported],0
 mov [packUiResult],0
 mov [packUiAction],7
 stdcall StartWorker,PackWorker
 ret
.missing:
 invoke lstrcpyW,statusText,sVChooseVersion
 jmp .redraw
.invalid:
 invoke lstrcpyW,statusText,sPackExportInvalid
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc PackExportUiFolder uses edi
 cmp [peUiExported],1
 jne .done
 invoke GetFileAttributesW,peUiDestW
 cmp eax,-1
 je .done
 test eax,FILE_ATTRIBUTE_DIRECTORY+400h
 jnz .done
 invoke lstrcpyW,peUiScratchW,peUiDestW
 invoke lstrlenW,peUiScratchW
 lea edi,[peUiScratchW+eax*2]
.back:
 cmp edi,peUiScratchW
 jbe .done
 sub edi,2
 cmp word [edi],92
 jne .back
 cmp edi,peUiScratchW+4
 jne @f
 cmp word [peUiScratchW+2],':'
 jne @f
 add edi,2
@@:
 mov word [edi],0
 invoke ShellExecuteW,[hWnd],openVerb,peUiScratchW,0,0,SW_SHOWNORMAL
.done:
 ret
endp
proc PaintPackExport,dc
 stdcall Card,252,185,836,428,18
 stdcall Text,[dc],sPackExportSource,280,210,154,26,[fSmall],C_MUTED
 stdcall PaintPackExportVersionList,[dc]
 stdcall TextDraw,peUiDataDirW,440,280,624,20,[fSmall],T_MUTED,DT_LEFT+DT_PATH_ELLIPSIS,0
 stdcall Text,[dc],sPackImportName,280,310,154,26,[fSmall],C_MUTED
 stdcall Field,440,304,624,36,[hPeUiName]
 stdcall Text,[dc],sPackExportVersion,280,358,154,26,[fSmall],C_MUTED
 stdcall Field,440,352,216,36,[hPeUiVersion]
 stdcall Text,[dc],sPackExportAuthor,690,358,100,26,[fSmall],C_MUTED
 stdcall Field,800,352,264,36,[hPeUiAuthor]
 stdcall Text,[dc],sPackImportFormat,280,406,154,26,[fSmall],C_MUTED
 mov [segLabels],sModrinth
 mov [segLabels+4],sCurseForge
 mov [segLabels+8],packFormatPrism
 mov eax,[peUiFormat]
 dec eax
 stdcall Segmented,440,396,624,40,3,4,segLabels,eax,[fBody]
 stdcall Text,[dc],sPackExportContents,280,464,154,26,[fSmall],C_MUTED
 stdcall Button,[dc],sPackExportMods,440,456,116,40,[peUiMods]
 stdcall Button,[dc],sPackExportConfig,567,456,116,40,[peUiConfig]
 stdcall Button,[dc],sPackExportResources,694,456,116,40,[peUiResources]
 stdcall Button,[dc],sPackExportSaves,821,456,116,40,[peUiSaves]
 stdcall Button,[dc],sPackExportServers,948,456,116,40,[peUiServers]
 mov eax,[peUiBytes]
 add eax,1048575
 shr eax,20
 cinvoke wsprintfW,peUiScratchW,sPackExportContent,[peUiFiles],eax
 stdcall Text,[dc],sPackExportOutput,280,570,154,24,[fSmall],C_MUTED
 mov eax,peUiDestW
 cmp word [peUiDestW],0
 jne @f
 mov eax,sPackExportChoose
@@:
 stdcall TextDraw,eax,440,570,624,24,[fSmall],T_SECOND,DT_LEFT+DT_PATH_ELLIPSIS,0
 stdcall Button,[dc],sPackExportPreview,252,620,160,40,0
 cmp [peUiExported],1
 jne .export
 stdcall Button,[dc],sLogFolder,432,620,200,40,0
.export:
 stdcall Button,[dc],sPackExportStart,862,620,226,40,1
 ret
endp
proc ClickPackExport uses ebx,x,y
 cmp [busy],0
 jne .done
 cmp [y],202
 jb .format
 cmp [y],274
 ja .format
 cmp [x],440
 jb .done
 cmp [x],1064
 jae .done
 mov eax,[y]
 sub eax,202
 xor edx,edx
 mov ecx,24
 div ecx
 cmp eax,3
 jae .done
 mov ebx,eax
 stdcall PackExportListTop
 add ebx,eax
 invoke SendMessageA,[hPeUiSource],CB_GETCOUNT,0,0
 cmp ebx,eax
 jae .done
 invoke SendMessageA,[hPeUiSource],CB_SETCURSEL,ebx,0
 invoke SendMessageA,[hPeUiSource],CB_GETLBTEXT,ebx,peUiVersionA
 invoke MultiByteToWideChar,65001,0,peUiVersionA,-1,peUiVersionW,128
 invoke SetWindowTextW,[hPeUiName],peUiVersionW
 stdcall PackExportUiInvalidate
 mov word [peUiDataDirW],0
 stdcall VsResolveData,peUiRootW,peUiVersionA,peUiDataDirW,2048
 jmp .redraw
.format:
 cmp [y],396
 jb .done
 cmp [y],436
 ja .saves
 cmp [x],440
 jb .done
 cmp [x],1064
 jae .done
 mov eax,[x]
 sub eax,440
 xor edx,edx
 mov ecx,208
 div ecx
 inc eax
 cmp eax,[peUiFormat]
 je .done
 mov [peUiFormat],eax
 mov word [peUiDestW],0
 mov [peUiExported],0
 jmp .redraw
.saves:
 cmp [y],456
 jb .done
 cmp [y],496
 ja .buttons
 cmp [x],440
 jb .done
 cmp [x],1064
 jae .done
 mov eax,[x]
 sub eax,440
 xor edx,edx
 mov ecx,127
 div ecx
 cmp edx,116
 jae .done
 cmp eax,4
 je .serverMode
 cmp eax,3
 je .saveMode
 cmp eax,0
 je .mods
 cmp eax,1
 je .config
 xor [peUiResources],1
 jmp .invalidate
.serverMode:
 xor [peUiServers],1
 jmp .invalidate
.mods:
 xor [peUiMods],1
 jmp .invalidate
.config:
 xor [peUiConfig],1
 jmp .invalidate
.saveMode:
 xor [peUiSaves],1
.invalidate:
 stdcall PackExportUiInvalidate
 jmp .redraw
.buttons:
 cmp [y],620
 jb .done
 cmp [y],660
 ja .done
 cmp [x],252
 jb .done
 cmp [x],412
 jbe .preview
 cmp [x],432
 jb .done
 cmp [x],632
 jbe .folder
 cmp [x],862
 jb .done
 cmp [x],1088
 ja .done
 stdcall PackExportUiStart
 ret
.preview:
 stdcall PackExportUiPreview
 ret
.folder:
 stdcall PackExportUiFolder
 ret
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
