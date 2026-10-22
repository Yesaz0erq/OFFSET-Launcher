; Read-only native viewer for sanitized launch output and crash reports.
section '.loguid' data readable writeable
logUiTab dd 0
logUiSeenSerial dd 0
hLogBody dd 0
logEyebrow du '08 / LOGS',0
logExportDefault du 'OFFSET-report.txt',0
logExportOutput du 'OFFSET-launch.log',0
logExportFilter du 'Logs and reports (*.log;*.txt)',0,'*.log;*.txt',0,0
logTextExt du 'txt',0
logSave OPENFILENAME
logExportW rw 2048
logUiPathW rw 2048
logUiBytes rb 262145
; The native EDIT text pointer must stay WCHAR-aligned after the byte buffer.
align 4
logUiRaw rw 262145
align 4
logUiWide rw 524289

section '.loguic' code readable executable
 nop
proc CreateLogControls
 stdcall NewControl,editClass,ES_MULTILINE+ES_AUTOVSCROLL+ES_AUTOHSCROLL+ES_READONLY+WS_VSCROLL+WS_HSCROLL,701,280,242,782,350
 mov [hLogBody],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,524288,0
 ret
endp
proc HideLogControls
 invoke ShowWindow,[hLogBody],SW_HIDE
 ret
endp
proc LogRefresh
 cmp [busy],0
 jne .done
 stdcall LaunchLogRead,[logUiTab],logUiBytes,262145
 test eax,eax
 jz .empty
 invoke MultiByteToWideChar,65001,0,logUiBytes,-1,logUiRaw,262145
 test eax,eax
 jz .empty
 stdcall LogNormalizeLines
 invoke SetWindowTextW,[hLogBody],logUiWide
 jmp .paint
.empty:
 mov eax,sLogEmpty
 cmp [logUiTab],0
 je @f
 mov eax,sLogNoCrash
@@:
 invoke SetWindowTextW,[hLogBody],eax
.paint:
 invoke SendMessageW,[hLogBody],EM_SETSEL,0,0
 invoke SendMessageW,[hLogBody],EM_SCROLLCARET,0,0
 invoke InvalidateRect,[hLogBody],0,1
 invoke UpdateWindow,[hLogBody]
.done:
 ret
endp
; Native EDIT controls need CRLF; exported files keep their original bytes.
proc LogNormalizeLines uses esi edi
 mov esi,logUiRaw
 mov edi,logUiWide
 xor edx,edx
.next:
 lodsw
 test ax,ax
 jz .finish
 cmp ax,10
 jne .copy
 cmp dx,13
 je .copy
 mov word [edi],13
 add edi,2
.copy:
 stosw
 mov dx,ax
 jmp .next
.finish:
 mov word [edi],0
 ret
endp
proc ShowLogControls
 cmp [busy],0
 jne .show
 cmp [launchLogSerial],0
 jne .show
 cmp [launchLogReady],0
 jne .show
 stdcall VsResolveData,gameDir,selectedVersion,logUiPathW,2048
 test eax,eax
 jz .show
 stdcall LaunchLogDiscover,logUiPathW
.show:
 invoke ShowWindow,[hLogBody],SW_SHOW
 stdcall LogRefresh
 ret
endp
proc LogFinishUI
 mov eax,[launchLogSerial]
 cmp eax,[logUiSeenSerial]
 je .refresh
 mov [logUiSeenSerial],eax
 cmp [launchLogSaveError],0
 je .report
 invoke lstrcpyW,statusText,sLogSaveFailed
.report:
 cmp [launchCrashReady],0
 je .refresh
 mov [logUiTab],1
 mov [currentPage],7
 mov [vmActive],0
 mov [detailActive],0
 stdcall StartTransition
 stdcall SyncPage
 ret
.refresh:
 cmp [currentPage],7
 jne .done
 stdcall LogRefresh
.done:
 ret
endp
proc LogExport uses edi
 cmp [busy],0
 jne .done
 mov eax,[launchLogReady]
 cmp [logUiTab],0
 je @f
 mov eax,[launchCrashReady]
@@:
 test eax,eax
 jz .empty
 mov eax,logExportDefault
 cmp [logUiTab],0
 jne @f
 mov eax,logExportOutput
@@:
 invoke lstrcpyW,logExportW,eax
 mov edi,logSave
 xor eax,eax
 mov ecx,sizeof.OPENFILENAME/4
 rep stosd
 mov [logSave.lStructSize],sizeof.OPENFILENAME
 mov eax,[hWnd]
 mov [logSave.hwndOwner],eax
 mov [logSave.lpstrFilter],logExportFilter
 mov [logSave.lpstrFile],logExportW
 mov [logSave.nMaxFile],2048
 mov [logSave.lpstrDefExt],logTextExt
 mov [logSave.Flags],OFN_PATHMUSTEXIST+OFN_NOCHANGEDIR+OFN_EXPLORER+OFN_OVERWRITEPROMPT
 invoke GetSaveFileNameW,logSave
 test eax,eax
 jz .done
 stdcall LaunchLogExportKindTo,[logUiTab],logExportW,1
 test eax,eax
 jz .failed
 mov eax,sLogExported
 jmp .status
.empty:
 mov eax,sLogEmpty
 cmp [logUiTab],0
 je .status
 mov eax,sLogNoCrash
 jmp .status
.failed:
 mov eax,sLogExportFailed
.status:
 invoke lstrcpyW,statusText,eax
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc LogOpenFolder
 cmp word [launchLogDataDirW],0
 je .done
 invoke lstrcpyW,logUiPathW,launchLogDataDirW
 invoke lstrcatW,logUiPathW,llUiFolderSuffix
 invoke GetFileAttributesW,logUiPathW
 cmp eax,-1
 je .done
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .done
 invoke ShellExecuteW,[hWnd],openVerb,logUiPathW,0,0,SW_SHOWNORMAL
.done:
 ret
endp
proc PaintLogs,dc
 stdcall PageHeader,logEyebrow,sLogsHeading,8
 mov [segLabels],sLogOutput
 mov [segLabels+4],sLogCrash
 stdcall Segmented,252,124,450,42,2,2,segLabels,[logUiTab],[fBody]
 stdcall Card,252,185,836,428,18
 mov eax,launchLogPathW
 cmp [logUiTab],0
 je @f
 mov eax,launchReportPathW
@@:
 cmp word [eax],0
 jne .path
 mov eax,sLogEmpty
.path:
 stdcall TextDraw,eax,280,200,782,28,[fSmall],T_SECOND,DT_LEFT+DT_PATH_ELLIPSIS,0
 stdcall Button,[dc],sLogRefresh,252,620,160,40,0
 stdcall Button,[dc],sLogFolder,432,620,200,40,0
 stdcall Button,[dc],sLogExport,862,620,226,40,1
 stdcall StatusLine,254,676,830,32
 stdcall BusyBar,254,664,834
 ret
endp
proc ClickLogs,x,y
 cmp [y],68
 jb .content
 cmp [y],100
 ja .content
 cmp [x],888
 jb .content
 cmp [x],1088
 ja .content
 if defined GoPage
 stdcall GoPage,3
 end if
 ret
.content:
 cmp [busy],0
 jne .done
 cmp [y],124
 jb .done
 cmp [y],166
 ja .buttons
 cmp [x],252
 jb .done
 cmp [x],702
 jae .done
 xor eax,eax
 cmp [x],477
 setae al
 cmp eax,[logUiTab]
 je .done
 mov [logUiTab],eax
 stdcall StartTransition
 stdcall SyncPage
 ret
.buttons:
 cmp [y],620
 jb .done
 cmp [y],660
 ja .done
 cmp [x],252
 jb .done
 cmp [x],412
 jbe .refresh
 cmp [x],432
 jb .done
 cmp [x],632
 jbe .folder
 cmp [x],862
 jb .done
 cmp [x],1088
 ja .done
 stdcall LogExport
 ret
.refresh:
 stdcall LogRefresh
 ret
.folder:
 stdcall LogOpenFolder
.done:
 ret
endp
section '.loguix' data readable writeable
llUiFolderSuffix du '\.offset-logs',0
