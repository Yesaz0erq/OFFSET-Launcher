; Independent-instance modpack browser. Archive and instance paths stay beside EXE.
section '.puid' data readable writeable
packTab dd 0
packUiAction dd 0
packUiResult dd 0
packLocalCount dd 0
hPackQuery dd 0
hPackGame dd 0
hPackProjects dd 0
hPackFiles dd 0
hPackLocal dd 0
hPackImportFolder dd 0
packImportReady dd 0
packImportFormat dd 0
packUiControls dd 6 dup(0)
packKeyName db 'name',0
packKeyVersion db 'version',0
packKeyComplete db 'complete',0
packInstancesSuffix du '\instances',0
packCacheSuffix du '\downloads\packs',0
packMarkerSuffix du '\.offset-pack.json',0
packWildcard du '\*',0
packBackslash du '\',0
packIdentityFmt du '%u-%s-%s',0
packImportFmt du 'import-%08X',0
packMrExt du '.mrpack',0
packCfExt du '.zip',0
packFileFilter du 'Modrinth / CurseForge / Prism / MultiMC (*.mrpack;*.zip)',0,'*.mrpack;*.zip',0,'All files',0,'*.*',0,0
packFormatMr du 'Modrinth',0
packFormatCf du 'CurseForge',0
packFormatPrism du 'Prism / MultiMC',0
packFormatNames dd packFormatMr,packFormatCf,packFormatPrism
packPreviewSeparator du ' / ',0
packPreviewSpace du ' ',0
packRoot rw 1024
packCacheRoot rw 1024
packArchive rw 2048
packDestination rw 2048
packUiIdentity rw 256
packUiWide rw 2048
packUiWide2 rw 2048
packUiIdA rb 128
packUiFileIdA rb 128
packUiNameA rb 1024
packImportArchiveW rw 2048
packImportHashA rb 41
packImportNameW rw 256
packImportVersionW rw 384
packImportFolderW rw 128
packImportFolderA rb 128
packLocalNames rw 256*128
packLocalDirs rw 1024*128
packLocalVersions rb 128*128
packFind WIN32_FIND_DATAW
packOpen OPENFILENAME

section '.puic' code readable executable
 nop
proc PackUiInit uses esi edi
 invoke lstrlenW,iniPath
 cmp eax,990
 jae .invalid
 invoke lstrcpyW,packRoot,iniPath
 mov edi,packRoot
.findEnd:
 cmp word [edi],0
 je .back
 add edi,2
 jmp .findEnd
.back:
 cmp edi,packRoot
 jbe .invalid
 sub edi,2
 cmp word [edi],92
 jne .back
.suffix:
 mov word [edi],0
 invoke lstrcpyW,packCacheRoot,packRoot
 invoke lstrcatW,packRoot,packInstancesSuffix
 invoke lstrcatW,packCacheRoot,packCacheSuffix
 ret
.invalid:
 mov word [packRoot],0
 mov word [packCacheRoot],0
 ret
endp
proc CreatePackControls uses esi
 stdcall PackUiInit
 stdcall NewControl,editClass,ES_AUTOHSCROLL,401,288,215,490,26
 mov [hPackQuery],eax
 mov [packUiControls],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,120,0
 invoke SendMessageW,eax,1501h,0,sPackQueryCue
 stdcall NewControl,editClass,ES_AUTOHSCROLL,402,810,215,244,26
 mov [hPackGame],eax
 mov [packUiControls+4],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,48,0
 invoke SendMessageW,eax,1501h,0,sPackGameCue
 stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,404,272,336,386,252
 mov [hPackProjects],eax
 mov [packUiControls+8],eax
 stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,405,700,336,366,252
 mov [hPackFiles],eax
 mov [packUiControls+12],eax
 stdcall NewControl,listClass,LBS_NOTIFY+LBS_NOINTEGRALHEIGHT+WS_VSCROLL,406,272,250,794,292
 mov [hPackLocal],eax
 mov [packUiControls+16],eax
 stdcall NewControl,editClass,ES_AUTOHSCROLL,407,452,416,598,24
 mov [hPackImportFolder],eax
 mov [packUiControls+20],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,80,0
 stdcall CreatePackExportControls
 ret
endp
proc HidePackControls uses esi
 xor esi,esi
.loop:
 invoke ShowWindow,dword [packUiControls+esi*4],SW_HIDE
 inc esi
 cmp esi,6
 jb .loop
 stdcall HidePackExportControls
 ret
endp
proc EnablePackControls uses esi,enabled
 xor esi,esi
.loop:
 invoke EnableWindow,dword [packUiControls+esi*4],[enabled]
 inc esi
 cmp esi,6
 jb .loop
 stdcall EnablePackExportControls,[enabled]
 ret
endp
proc ShowPackControls
 cmp [detailActive],0
 jne .detail
 cmp [packTab],0
 jne .local
 invoke ShowWindow,[hPackQuery],SW_SHOW
 invoke ShowWindow,[hPackGame],SW_SHOW
 invoke ShowWindow,[hPackProjects],SW_SHOW
 invoke ShowWindow,[hPackFiles],SW_SHOW
 ret
.local:
 cmp [packTab],3
 je .export
 cmp [packTab],2
 je .import
 stdcall PackScanLocal
 invoke ShowWindow,[hPackLocal],SW_SHOW
 ret
.import:
 cmp [packImportReady],0
 je .done
 invoke ShowWindow,[hPackImportFolder],SW_SHOW
.done:
 ret
.export:
 stdcall ShowPackExportControls
 ret
.detail:
 stdcall ShowDetailControls
 ret
endp
proc PackClearResults
 mov [packProjectCount],0
 mov [packFileCount],0
 mov [packTotal],0
 mov [packSelectedProject],-1
 mov [packSelectedFile],-1
 invoke SendMessageW,[hPackProjects],LB_RESETCONTENT,0,0
 invoke SendMessageW,[hPackFiles],LB_RESETCONTENT,0,0
 ret
endp
proc PackBeginSearch
 invoke GetWindowTextW,[hPackQuery],packUiWide,2048
 invoke WideCharToMultiByte,65001,0,packUiWide,-1,packQuery,512,0,0
 test eax,eax
 jz .done
 invoke GetWindowTextA,[hPackGame],packGameVersion,64
 mov byte [packLoader],0
 stdcall PackClearResults
 mov [packUiResult],0
 mov [packUiAction],1
 stdcall StartWorker,PackWorker
.done:
 ret
endp
proc PackBuildPaths uses ebx esi
 cmp word [packRoot],0
 je .bad
 cmp word [packCacheRoot],0
 je .bad
 cmp [packProvider],1
 ja .bad
 mov eax,[packSelectedProject]
 cmp eax,[packProjectCount]
 jae .bad
 imul eax,MP_SIZE
 add eax,packProjects
 invoke lstrcpyA,packUiIdA,eax
 stdcall VersionsSafeId,packUiIdA
 test eax,eax
 jz .bad
 mov eax,[packSelectedFile]
 cmp eax,[packFileCount]
 jae .bad
 imul eax,MF_SIZE
 add eax,packFiles
 invoke lstrcpyA,packUiFileIdA,eax
 stdcall VersionsSafeId,packUiFileIdA
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,0,packUiIdA,-1,packUiWide,2048
 invoke MultiByteToWideChar,65001,0,packUiFileIdA,-1,packUiWide2,2048
 cinvoke wsprintfW,packUiIdentity,packIdentityFmt,[packProvider],packUiWide,packUiWide2
 invoke lstrcpyW,packDestination,packRoot
 invoke lstrcatW,packDestination,packBackslash
 invoke lstrcatW,packDestination,packUiIdentity
 invoke lstrcpyW,packArchive,packCacheRoot
 invoke lstrcatW,packArchive,packBackslash
 invoke lstrcatW,packArchive,packUiIdentity
 mov eax,packMrExt
 cmp [packProvider],0
 je @f
 mov eax,packCfExt
@@:
 invoke lstrcatW,packArchive,eax
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PackImportLocal uses edi
 cmp [busy],0
 jne .done
 cmp word [packRoot],0
 je .done
 ; Keep an existing preview when the chooser is cancelled.
 mov word [packUiWide2],0
 mov edi,packOpen
 xor eax,eax
 mov ecx,sizeof.OPENFILENAME/4
 rep stosd
 mov [packOpen.lStructSize],sizeof.OPENFILENAME
 mov eax,[hWnd]
 mov [packOpen.hwndOwner],eax
 mov [packOpen.lpstrFilter],packFileFilter
 mov [packOpen.lpstrFile],packUiWide2
 mov [packOpen.nMaxFile],2048
 mov [packOpen.Flags],OFN_FILEMUSTEXIST+OFN_PATHMUSTEXIST+OFN_NOCHANGEDIR+OFN_EXPLORER
 invoke GetOpenFileNameW,packOpen
 test eax,eax
 jz .done
 invoke lstrcpyW,packImportArchiveW,packUiWide2
 mov [packImportReady],0
 mov byte [packImportHashA],0
 mov [packTab],2
 stdcall StartTransition
 stdcall SyncPage
 mov [packUiResult],0
 mov [packUiAction],5
 stdcall StartWorker,PackWorker
.done:
 ret
endp

; A preview is immutable until another archive is selected. Merely inspecting
; it neither creates an instance nor changes the current game directory.
proc PackImportPreview uses ebx
 cmp [packInspectReady],1
 jne .bad
 mov eax,[packInspectFormat]
 cmp eax,1
 jb .bad
 cmp eax,3
 ja .bad
 mov [packImportFormat],eax
 invoke MultiByteToWideChar,65001,8,packInspectName,-1,packImportNameW,256
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,packInspectBase,-1,packImportVersionW,384
 test eax,eax
 jz .bad
 mov ebx,[packInspectLoaderKind]
 cmp ebx,4
 ja .bad
 cmp ebx,0
 je .identity
 invoke lstrcatW,packImportVersionW,packPreviewSeparator
 invoke lstrcatW,packImportVersionW,dword [vUiNames+ebx*4]
 invoke MultiByteToWideChar,65001,8,packInspectLoader,-1,packUiWide,2048
 test eax,eax
 jz .bad
 invoke lstrlenW,packImportVersionW
 mov ebx,eax
 invoke lstrlenW,packUiWide
 add eax,ebx
 cmp eax,382
 jae .bad
 invoke lstrcatW,packImportVersionW,packPreviewSpace
 invoke lstrcatW,packImportVersionW,packUiWide
.identity:
 invoke lstrlenA,packInspectHash
 cmp eax,40
 jne .bad
 invoke lstrcpyA,packImportHashA,packInspectHash
 invoke GetTickCount
 cinvoke wsprintfW,packUiIdentity,packImportFmt,eax
 invoke SetWindowTextW,[hPackImportFolder],packUiIdentity
 mov [packImportReady],1
 mov eax,1
 ret
.bad:
 mov [packImportReady],0
 xor eax,eax
 ret
endp

proc PackImportDestination uses ebx esi
 cmp [packImportReady],1
 jne .bad
 invoke GetWindowTextW,[hPackImportFolder],packImportFolderW,128
 test eax,eax
 jz .bad
 cmp eax,80
 ja .bad
 mov esi,packImportFolderW
.ascii:
 mov ax,[esi]
 test ax,ax
 jz .convert
 cmp ax,127
 ja .bad
 add esi,2
 jmp .ascii
.convert:
 invoke WideCharToMultiByte,65001,0,packImportFolderW,-1,packImportFolderA,128,0,0
 test eax,eax
 jz .bad
 stdcall VersionsSafeId,packImportFolderA
 test eax,eax
 jz .bad
 invoke lstrlenW,packRoot
 mov ebx,eax
 invoke lstrlenW,packImportFolderW
 add eax,ebx
 cmp eax,990
 jae .bad
 invoke lstrcpyW,packDestination,packRoot
 invoke lstrcatW,packDestination,packBackslash
 invoke lstrcatW,packDestination,packImportFolderW
 invoke GetFileAttributesW,packDestination
 cmp eax,-1
 jne .bad
 invoke GetLastError
 cmp eax,2 ; ERROR_FILE_NOT_FOUND
 je .good
 cmp eax,3 ; ERROR_PATH_NOT_FOUND
 jne .bad
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PackImportStart
 cmp [busy],0
 jne .done
 stdcall PackImportDestination
 test eax,eax
 jz .invalid
 mov [packUiResult],0
 mov [packUiAction],4
 stdcall StartWorker,PackWorker
 ret
.invalid:
 invoke lstrcpyW,statusText,sPackImportInvalidFolder
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp

proc PackImportClear
 cmp [busy],0
 jne .done
 mov [packImportReady],0
 mov byte [packImportHashA],0
 mov word [packImportArchiveW],0
 mov word [packImportNameW],0
 mov word [packImportVersionW],0
 invoke SetWindowTextW,[hPackImportFolder],emptyW
 invoke lstrcpyW,statusText,sReady
 stdcall SyncPage
.done:
 ret
endp

proc PackWorker,param
 mov [packUiResult],0
 cmp [packUiAction],1
 je .search
 cmp [packUiAction],2
 je .versions
 cmp [packUiAction],5
 je .inspect
 cmp [packUiAction],6
 je .exportPreview
 cmp [packUiAction],7
 je .exportBuild
 cmp [packUiAction],4
 je .install
 cmp [packUiAction],3
 jne .finish
 stdcall PackDownload,[packSelectedFile],packArchive
 test eax,eax
 jz .finish
.install:
 stdcall EnsureDirectory,packRoot
 test eax,eax
 jnz .readyInstall
 stdcall SetStatusA,pkMsgDestination
 jmp .finish
.readyInstall:
 cmp [packUiAction],4
 jne .downloadInstall
 stdcall PackInstallChecked,packImportArchiveW,packDestination,packImportHashA
 jmp .installed
.downloadInstall:
 stdcall PackInstall,packArchive,packDestination
.installed:
 mov [packUiResult],eax
 jmp .finish
.inspect:
 stdcall PackInspect,packImportArchiveW
 mov [packUiResult],eax
 jmp .finish
.exportPreview:
 stdcall PackExportInspect,peUiRootW,peUiVersionA,[peUiSaves]
 mov [packUiResult],eax
 jmp .finish
.exportBuild:
 stdcall PackExportBuild
 mov [packUiResult],eax
 jmp .finish
.search:
 stdcall PackSearch
 jmp .finish
.versions:
 stdcall PackVersions
.finish:
 invoke PostMessageW,[hWnd],WM_FINISHED,0,0
 ret
endp
proc PackFinishUI uses ebx esi
 cmp [packUiAction],0
 je .done
 cmp [packUiAction],1
 je .projects
 cmp [packUiAction],2
 je .files
 cmp [packUiAction],5
 je .preview
 cmp [packUiAction],6
 jae .export
 cmp [packUiResult],1
 jne .reset
 cmp [packUiAction],4
 jne .installed
 mov [packImportReady],0
 mov byte [packImportHashA],0
.installed:
 mov [packTab],1
 stdcall PackScanLocal
 stdcall SyncPage
 jmp .reset
.preview:
 mov [packImportReady],0
 cmp [packUiResult],1
 jne .previewShown
 stdcall PackImportPreview
 test eax,eax
 jz .previewShown
 invoke lstrcpyW,statusText,sPackImportReady
.previewShown:
 cmp [currentPage],5
 jne .reset
 stdcall SyncPage
 jmp .reset
.export:
 stdcall PackExportUiFinish
 jmp .reset
.projects:
 invoke SendMessageW,[hPackProjects],LB_RESETCONTENT,0,0
 xor esi,esi
.projectLoop:
 cmp esi,[packProjectCount]
 jae .reset
 imul ebx,esi,MP_SIZE
 add ebx,packProjects+MP_TITLE
 invoke MultiByteToWideChar,65001,0,ebx,-1,packUiWide,2048
 invoke SendMessageW,[hPackProjects],LB_ADDSTRING,0,packUiWide
 inc esi
 jmp .projectLoop
.files:
 invoke SendMessageW,[hPackFiles],LB_RESETCONTENT,0,0
 xor esi,esi
.fileLoop:
 cmp esi,[packFileCount]
 jae .select
 imul ebx,esi,MF_SIZE
 add ebx,packFiles+MF_TITLE
 invoke MultiByteToWideChar,65001,0,ebx,-1,packUiWide,2048
 invoke SendMessageW,[hPackFiles],LB_ADDSTRING,0,packUiWide
 inc esi
 jmp .fileLoop
.select:
 cmp [packFileCount],0
 je .reset
 mov [packSelectedFile],0
 invoke SendMessageW,[hPackFiles],LB_SETCURSEL,0,0
.reset:
 mov [packUiAction],0
.done:
 ret
endp
proc PackScanLocal uses ebx esi edi
 locals
  find dd ?
  response dd ?
  ctx dd ?
  root dd ?
  textLength dd ?
 endl
 mov [packLocalCount],0
 invoke SendMessageW,[hPackLocal],LB_RESETCONTENT,0,0
 cmp word [packRoot],0
 je .done
 stdcall PackUiDirectory,packRoot
 test eax,eax
 jz .done
 invoke lstrcpyW,packUiWide,packRoot
 invoke lstrcatW,packUiWide,packWildcard
 invoke FindFirstFileW,packUiWide,packFind
 cmp eax,-1
 je .done
 mov [find],eax
.next:
 test [packFind.dwFileAttributes],400h
 jnz .advance
 test [packFind.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY
 jz .advance
 cmp word [packFind.cFileName],'.'
 je .advance
 cmp [packLocalCount],128
 jae .close
 ; Every stored directory must fit gameDir's 1024 UTF-16 units.
 invoke lstrlenW,packRoot
 mov esi,eax
 invoke lstrlenW,packFind.cFileName
 add eax,esi
 inc eax
 cmp eax,1024
 jae .advance
 mov ebx,[packLocalCount]
 imul ebx,2048
 add ebx,packLocalDirs
 invoke lstrcpyW,ebx,packRoot
 invoke lstrcatW,ebx,packBackslash
 invoke lstrcatW,ebx,packFind.cFileName
 invoke lstrcpyW,packUiWide,ebx
 invoke lstrcatW,packUiWide,packMarkerSuffix
 stdcall PackUiRegularFile,packUiWide
 test eax,eax
 jz .advance
 stdcall EReadFile,packUiWide
 test eax,eax
 jz .advance
 mov [response],eax
 stdcall JsonParse,eax
 mov [ctx],eax
 test eax,eax
 jz .freeResponse
 stdcall JsonRoot,eax
 mov [root],eax
 stdcall JsonGet,[ctx],eax,packKeyComplete
 stdcall JsonBool,[ctx],eax
 cmp eax,1
 jne .freeContext
 stdcall JsonGet,[ctx],[root],packKeyVersion
 mov esi,[packLocalCount]
 shl esi,7
 add esi,packLocalVersions
 stdcall JsonString,[ctx],eax,esi,128
 test eax,eax
 jz .freeContext
 cmp eax,128
 jae .freeContext
 mov [textLength],eax
 invoke lstrlenA,esi
 cmp eax,[textLength]
 jne .freeContext
 stdcall VersionsSafeId,esi
 test eax,eax
 jz .freeContext
 stdcall JsonGet,[ctx],[root],packKeyName
 stdcall JsonString,[ctx],eax,packUiNameA,1024
 test eax,eax
 jz .freeContext
 cmp eax,1024
 jae .freeContext
 mov [textLength],eax
 invoke lstrlenA,packUiNameA
 cmp eax,[textLength]
 jne .freeContext
 mov edi,[packLocalCount]
 shl edi,9
 add edi,packLocalNames
 invoke MultiByteToWideChar,65001,8,packUiNameA,-1,edi,256
 test eax,eax
 jz .freeContext
 invoke SendMessageW,[hPackLocal],LB_ADDSTRING,0,edi
 inc [packLocalCount]
.freeContext:
 stdcall JsonFree,[ctx]
.freeResponse:
 stdcall FreeMem,[response]
.advance:
 invoke FindNextFileW,[find],packFind
 test eax,eax
 jnz .next
.close:
 invoke FindClose,[find]
.done:
 ret
endp
; Local markers and profiles are data, never authority to follow reparse points.
proc PackUiDirectory,path
 invoke GetFileAttributesW,[path]
 cmp eax,-1
 je .bad
 test eax,400h
 jnz .bad
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PackUiRegularFile,path
 invoke GetFileAttributesW,[path]
 cmp eax,-1
 je .bad
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc PackActivateSelected uses ebx esi
 cmp [busy],0
 jne .done
 stdcall PullSettings
 test eax,eax
 jz .done
 invoke SendMessageW,[hPackLocal],LB_GETCURSEL,0,0
 cmp eax,[packLocalCount]
 jae .done
 mov ebx,eax
 shl eax,11
 add eax,packLocalDirs
 mov esi,eax
 stdcall PackUiDirectory,esi
 test eax,eax
 jz .done
 ; Recheck the installed child and its parents before changing the workspace.
 invoke lstrcpyW,packUiWide,esi
 invoke lstrcatW,packUiWide,eVersions
 stdcall PackUiDirectory,packUiWide
 test eax,eax
 jz .done
 mov eax,ebx
 shl eax,7
 add eax,packLocalVersions
 invoke MultiByteToWideChar,65001,8,eax,-1,packUiWide2,2048
 test eax,eax
 jz .done
 invoke lstrcatW,packUiWide,packUiWide2
 stdcall PackUiDirectory,packUiWide
 test eax,eax
 jz .done
 invoke lstrcatW,packUiWide,packBackslash
 invoke lstrcatW,packUiWide,packUiWide2
 invoke lstrcatW,packUiWide,eJsonExt
 stdcall PackUiRegularFile,packUiWide
 test eax,eax
 jz .done
 stdcall EStandaloneProfile,packUiWide
 test eax,eax
 jz .done
 invoke lstrlenW,packUiWide
 mov word [packUiWide+eax*2-10],'.'
 mov word [packUiWide+eax*2-8],'j'
 mov word [packUiWide+eax*2-6],'a'
 mov word [packUiWide+eax*2-4],'r'
 mov word [packUiWide+eax*2-2],0
 stdcall PackUiRegularFile,packUiWide
 test eax,eax
 jz .done
 invoke lstrcpyW,gameDir,esi
 invoke SetWindowTextW,[hGameDir],gameDir
 mov eax,ebx
 shl eax,7
 add eax,packLocalVersions
 invoke lstrcpyA,selectedVersion,eax
 stdcall EngineScanVersions,[hVersion]
 invoke SetWindowTextA,[hVersion],selectedVersion
 stdcall SaveSettings
 test eax,eax
 jz .done
 stdcall GoPage,0
.done:
 ret
endp
proc PackCommand,wparam
 stdcall PackExportUiCommand,[wparam]
 cmp [currentPage],5
 jne .done
 cmp [busy],0
 jne .done
 mov eax,[wparam]
 mov edx,eax
 shr edx,16
 and eax,0FFFFh
 cmp eax,404
 je .project
 cmp eax,405
 je .file
 ret
.project:
 cmp edx,LBN_SELCHANGE
 jne .done
 invoke SendMessageW,[hPackProjects],LB_GETCURSEL,0,0
 cmp eax,[packProjectCount]
 jae .done
 mov [packSelectedProject],eax
 mov [packSelectedFile],-1
 mov [packFileCount],0
 invoke SendMessageW,[hPackFiles],LB_RESETCONTENT,0,0
 mov [packUiResult],0
 mov [packUiAction],2
 stdcall StartWorker,PackWorker
 ret
.file:
 cmp edx,LBN_SELCHANGE
 jne .done
 invoke SendMessageW,[hPackFiles],LB_GETCURSEL,0,0
 mov [packSelectedFile],eax
.done:
 ret
endp
proc PaintPacks,dc
 cmp [detailActive],0
 je .browser
 stdcall PaintProjectDetail,[dc]
 ret
.browser:
 stdcall PageHeader,eyebrowPacks,sPacks,6
 mov [segLabels],sPackBrowse
 mov [segLabels+4],sVInstalledTab
 mov [segLabels+8],sPackImportTab
 mov [segLabels+12],sPackExportTab
 stdcall Segmented,252,124,450,42,4,2,segLabels,[packTab],[fBody]
 mov eax,sPackImport
 cmp [packTab],3
 jne @f
 mov eax,sPackExportChoose
 jmp .topButton
@@:
 cmp [packTab],2
 jne @f
 mov eax,sPackChooseArchive
@@:
.topButton:
 cmp [busy],0
 je @f
 mov eax,sModCancel
@@:
 stdcall Button,[dc],eax,916,124,172,42,0
 cmp [packTab],3
 je .export
 cmp [packTab],2
 je .import
 cmp [packTab],0
 jne .local
 mov [segLabels],sModrinth
 mov [segLabels+4],sCurseForge
 stdcall Segmented,728,264,360,42,2,4,segLabels,[packProvider],[fBody]
 stdcall Card,252,185,836,72,18
 stdcall Text,[dc],sPackQueryCue,280,186,490,20,[fSmall],C_MUTED
 stdcall Text,[dc],sPackGameCue,810,186,244,20,[fSmall],C_MUTED
 stdcall Field,278,207,510,42,[hPackQuery]
 stdcall Field,800,207,264,42,[hPackGame]
 mov eax,sModSearch
 cmp [busy],0
 je @f
 mov eax,sModCancel
@@:
 stdcall Button,[dc],eax,252,264,180,42,1
 stdcall Card,252,320,418,286,18
 stdcall Card,686,320,402,286,18
 stdcall Button,[dc],sModPrev,252,616,88,40,0
 stdcall Button,[dc],sModNext,350,616,88,40,0
 stdcall Button,[dc],sDetailHeading,448,616,96,40,0
 stdcall Button,[dc],sDetailMcmod,554,616,106,40,0
 stdcall Button,[dc],sModWebsite,670,616,172,40,0
 stdcall Button,[dc],sPackInstall,862,616,226,40,1
 jmp .status
.local:
 stdcall Card,252,185,836,428,18
 stdcall Text,[dc],sPackIsolated,280,201,776,40,[fBody],C_MUTED
 stdcall Button,[dc],sPackPlay,280,563,220,40,1
 stdcall Button,[dc],sRefresh,520,563,220,40,0
 jmp .status
.import:
 stdcall Card,252,185,836,428,18
 stdcall Text,[dc],sPackImportFormats,280,201,776,30,[fBody],C_MUTED
 cmp [packImportReady],1
 je .preview
 stdcall Text,[dc],sPackImportEmpty,280,300,776,42,[fMedium],C_INK
 stdcall Button,[dc],sPackChooseArchive,280,374,240,44,1
 jmp .status
.preview:
 stdcall Text,[dc],sPackImportArchive,280,252,154,24,[fSmall],C_MUTED
 stdcall TextDraw,packImportArchiveW,440,252,624,26,[fBody],C_INK,DT_LEFT+DT_PATH_ELLIPSIS,0
 stdcall Text,[dc],sPackImportFormat,280,296,154,24,[fSmall],C_MUTED
 mov eax,[packImportFormat]
 dec eax
 cmp eax,2
 ja .status
 stdcall Text,[dc],dword [packFormatNames+eax*4],440,296,624,26,[fBody],C_INK
 stdcall Text,[dc],sPackImportName,280,340,154,24,[fSmall],C_MUTED
 stdcall Text,[dc],packImportNameW,440,340,624,26,[fBody],C_INK
 stdcall Text,[dc],sPackImportVersion,280,374,154,24,[fSmall],C_MUTED
 stdcall Text,[dc],packImportVersionW,440,374,624,26,[fBody],C_INK
 stdcall Text,[dc],sPackImportFolder,280,416,154,26,[fSmall],C_MUTED
 stdcall Field,440,410,624,38,[hPackImportFolder]
 stdcall Text,[dc],sPackImportFolderHelp,440,456,624,26,[fSmall],C_MUTED
 stdcall TextDraw,packRoot,280,520,776,26,[fMonoSmall],T_MUTED,DT_LEFT+DT_PATH_ELLIPSIS,0
 stdcall Button,[dc],sPackImportStart,252,620,220,42,1
 stdcall Button,[dc],sPackImportReset,492,620,172,42,0
 jmp .status
.export:
 stdcall PaintPackExport,[dc]
.status:
 stdcall StatusLine,254,676,830,32
 stdcall BusyBar,254,664,834
 ret
endp
proc PackOpenWebsite uses ebx
 mov eax,[packSelectedProject]
 cmp eax,[packProjectCount]
 jae .done
 imul eax,MP_SIZE
 lea ebx,[packProjects+eax+MP_URL]
 cmp byte [ebx],0
 je .done
 invoke MultiByteToWideChar,65001,0,ebx,-1,packUiWide,2048
 invoke ShellExecuteW,[hWnd],openVerb,packUiWide,0,0,SW_SHOWNORMAL
.done:
 ret
endp
proc ClickPacks uses ebx,x,y
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
 ja .busySearch
 cmp [x],916
 jb .done
 cmp [x],1088
 ja .done
 jmp .cancel
.busySearch:
 cmp [packTab],0
 jne .done
 cmp [y],264
 jb .done
 cmp [y],306
 ja .done
 cmp [x],252
 jb .done
 cmp [x],432
 ja .done
.cancel:
 mov [modCancel],1
 mov [engineCancel],1
 ret
.tabs:
 cmp [y],124
 jb .done
 cmp [y],166
 ja .body
 cmp [x],916
 jae .import
 cmp [x],252
 jb .done
 cmp [x],702
 jae .done
 mov eax,[x]
 sub eax,252
 imul eax,4
 xor edx,edx
 mov ecx,450
 div ecx
 mov [packTab],eax
 stdcall StartTransition
 stdcall SyncPage
 jmp .done
.import:
 cmp [x],1088
 ja .done
 cmp [packTab],3
 je .exportChoose
 stdcall PackImportLocal
 ret
.exportChoose:
 stdcall PackExportUiChoose
 ret
.body:
 cmp [packTab],3
 je .exportBody
 cmp [packTab],2
 je .importBody
 cmp [packTab],0
 jne .local
 cmp [y],264
 jb .done
 cmp [y],306
 ja .bottom
 cmp [x],432
 jbe .search
 cmp [x],728
 jb .done
 cmp [x],1088
 ja .done
 xor eax,eax
 cmp [x],908
 setae al
 mov [packProvider],eax
 mov [packOffset],0
 stdcall PackClearResults
 jmp .redraw
.search:
 cmp [x],252
 jb .done
 mov [packOffset],0
 stdcall PackBeginSearch
 ret
.bottom:
 cmp [y],616
 jb .done
 cmp [y],656
 ja .done
 cmp [x],252
 jb .done
 cmp [x],340
 jbe .prev
 cmp [x],350
 jb .done
 cmp [x],438
 jbe .next
 cmp [x],448
 jb .done
 cmp [x],544
 jbe .detail
 cmp [x],554
 jb .done
 cmp [x],660
 jbe .mcmod
 cmp [x],670
 jb .done
 cmp [x],842
 jbe .website
 cmp [x],862
 jb .done
 cmp [x],1088
 ja .done
 stdcall PackBuildPaths
 test eax,eax
 jz .done
 mov [packUiResult],0
 mov [packUiAction],3
 stdcall StartWorker,PackWorker
 ret
.prev:
 cmp [packOffset],20
 jb .done
 sub [packOffset],20
 stdcall PackBeginSearch
 ret
.next:
 mov eax,[packOffset]
 add eax,20
 cmp eax,[packTotal]
 jae .done
 mov [packOffset],eax
 stdcall PackBeginSearch
 ret
.detail:
 mov eax,[packSelectedProject]
 cmp eax,[packProjectCount]
 jae .done
 imul eax,MP_SIZE
 add eax,packProjects
 stdcall DetailOpen,eax,[packProvider],1
 ret
.mcmod:
 mov eax,[packSelectedProject]
 cmp eax,[packProjectCount]
 jae .done
 imul eax,MP_SIZE
 add eax,packProjects+MP_TITLE
 stdcall OpenMcmodQuery,eax
 ret
.website:
 stdcall PackOpenWebsite
 ret
.local:
 cmp [y],563
 jb .done
 cmp [y],603
 ja .done
 cmp [x],280
 jb .done
 cmp [x],500
 jbe .activate
 cmp [x],520
 jb .done
 cmp [x],740
 ja .done
 stdcall PackScanLocal
 jmp .redraw
.activate:
 stdcall PackActivateSelected
 ret
.importBody:
 cmp [packImportReady],1
 je .importActions
 cmp [y],374
 jb .done
 cmp [y],418
 ja .done
 cmp [x],280
 jb .done
 cmp [x],520
 ja .done
 stdcall PackImportLocal
 ret
.importActions:
 cmp [y],620
 jb .done
 cmp [y],662
 ja .done
 cmp [x],252
 jb .done
 cmp [x],472
 jbe .importStart
 cmp [x],492
 jb .done
 cmp [x],664
 ja .done
 stdcall PackImportClear
 ret
.importStart:
 stdcall PackImportStart
 ret
.exportBody:
 stdcall ClickPackExport,[x],[y]
 ret
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp

include 'pack_export_ui.inc'
