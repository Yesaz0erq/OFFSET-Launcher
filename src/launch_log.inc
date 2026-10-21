; Bounded UTF-8 launch output and crash reports. Only sanitized bytes reach disk.
LL_LIMIT = 4194304
LL_OUTPUT_LIMIT = 3145728
LL_TAIL = 1048576
struct LL_SECURITY_ATTRIBUTES
 nLength dd ?
 lpSecurityDescriptor dd ?
 bInheritHandle dd ?
ends

section '.llogd' data readable writeable
launchLogReady dd 0
launchCrashReady dd 0
launchLogSerial dd 0
launchExitCode dd -1
launchWin32Error dd 0
launchLogSaveError dd 0
llActive dd 0
llFile dd -1
llBytes dd 0
llCap dd LL_LIMIT
llFailed dd 0
llRead dd 0
llWrite dd 0
llInput dd -1
llPendingCount dd 0
llSecretLen dd 0,0
llMaxSecret dd 1
llTruncated dd 0
llStart FILETIME
llEndTime FILETIME
llTime SYSTEMTIME
llAttr LL_SECURITY_ATTRIBUTES
llFind WIN32_FIND_DATAW
llBestTime FILETIME
llDirSuffix du '\.offset-logs',0
llSlash du '\',0
llRunName du '\launch-%04u%02u%02u-%02u%02u%02u-%03u-%08X-%u.log',0
llLatest du '\logs\latest.log',0
llCrashGlob du '\crash-reports\*.txt',0
llCrashDir du '\crash-reports\',0
llHsGlob du '\hs_err_pid*.log',0
llNul du 'NUL',0
llTmpSuffix du '.tmp',0
llFinalGlob du '\.offset-logs\launch-*.log',0
llTxtExt du '.txt',0
llLogExt du '.log',0
llExportFmt du '.offset-export-%08X.tmp',0
llHeader db 'OFFSET launch log',13,10,'UTF-8; session credentials redacted; output bounded.',13,10,0
llTimeFmt db 'Time: %04u-%02u-%02u %02u:%02u:%02u.%03u',13,10,0
llVersionLabel db 'Version: ',0
llJavaLabel db 'Java: ',0
llDataLabel db 'Data directory: ',0
llRamFmt db 'Maximum memory: %u MiB',13,10,13,10,'--- Java stdout / stderr ---',13,10,0
llSummaryFmt db 13,10,'--- Result ---',13,10,'Outcome: %u (0=normal exit, 1=failure, 2=cancelled)',13,10,'Exit code: %08X (FFFFFFFF=no process)',13,10,'Win32 error: %u',13,10,'Stage: ',0
llReportHeader db 'OFFSET crash report',13,10,'This report contains the launcher output and files modified during this launch.',13,10,13,10,0
llAttachment db 13,10,13,10,'--- File: ',0
llEndLabel db ' ---',13,10,0
llNewline db 13,10,0
llRedacted db '[redacted]'
llRedactedEnd:
llOmitted db 13,10,'[Output limit reached; remaining process output was drained.]',13,10,0
llTailLabel db '[Only the last 1 MiB of this file follows.]',13,10,0
llCaptureMissing db '[Process output capture unavailable.]',13,10,0
llStageMetadata db 'Loading installed version metadata',0
llStagePrepare db 'Validating Java, identity, dependencies and arguments',0
launchLogPathW rw 2048
launchReportPathW rw 2048
launchLogDataDirW rw 2048
llDirectoryW rw 2048
llPathW rw 2048
llBestW rw 2048
llTempPathW rw 2048
llWideName rw 256
llScratch rb 16384
llPending rb 16384
llOut rb 16384
llSecrets rb 8192+256

section '.llogc' code readable executable
 nop

proc LaunchLogReset uses edi
 stdcall LlClosePipes
 cmp [llFile],-1
 je @f
 invoke CloseHandle,[llFile]
@@:
 inc [launchLogSerial]
 mov [launchLogReady],0
 mov [launchCrashReady],0
 mov [launchExitCode],-1
 mov [launchWin32Error],0
 mov [launchLogSaveError],0
 mov [llActive],0
 mov [llFile],-1
 mov [llPendingCount],0
 mov [llFailed],0
 mov [llTruncated],0
 mov [llSecretLen],0
 mov [llSecretLen+4],0
 mov [llMaxSecret],1
 mov word [launchLogPathW],0
 mov word [launchReportPathW],0
 mov edi,llSecrets
 xor eax,eax
 mov ecx,(8192+256)/4
 rep stosd
 invoke GetSystemTimeAsFileTime,llStart
 ret
endp

; Source must be a current credential buffer; bounded snapshots stay in memory.
proc LlSecret uses esi edi,source,destination,capacity,index
 mov esi,[source]
 mov edi,[destination]
 mov ecx,[capacity]
 dec ecx
 xor edx,edx
.copy:
 lodsb
 test al,al
 jz .secretEnd
 stosb
 inc edx
 loop .copy
.secretEnd:
 mov byte [edi],0
 mov eax,[index]
 mov [llSecretLen+eax*4],edx
 cmp edx,[llMaxSecret]
 jbe .done
 mov [llMaxSecret],edx
.done:
 ret
endp

proc LlRaw uses ebx esi,data,length
 local wrote:DWORD
 cmp [llFile],-1
 je .done
 cmp [llFailed],0
 jne .done
 mov ebx,[length]
 test ebx,ebx
 jz .done
 mov eax,[llCap]
 sub eax,[llBytes]
 jbe .done
 cmp ebx,eax
 jbe @f
 mov ebx,eax
@@:
 invoke WriteFile,[llFile],[data],ebx,addr wrote,0
 test eax,eax
 jz .bad
 cmp [wrote],ebx
 jne .bad
 add [llBytes],ebx
 ret
.bad:
 mov [llFailed],1
 mov [launchLogSaveError],1
.done:
 ret
endp

; Retain a secret-sized tail so a credential split over pipe reads is redacted.
proc LlFlush uses ebx esi edi,final
 locals
  until dd ?
  position dd ?
  outCount dd ?
  matched dd ?
 endl
 mov eax,[llPendingCount]
 cmp [final],0
 jne .limit
 sub eax,[llMaxSecret]
 inc eax
 jle .done
.limit:
 mov [until],eax
 mov [position],0
 mov [outCount],0
.next:
 cmp [outCount],8192
 jb @f
 stdcall LlRaw,llOut,[outCount]
 mov [outCount],0
@@:
 mov ebx,[position]
 cmp ebx,[until]
 jae .write
 mov [matched],0
 xor edi,edi
.secret:
 mov ecx,[llSecretLen+edi*4]
 test ecx,ecx
 jz .another
 mov eax,[llPendingCount]
 sub eax,ebx
 cmp eax,ecx
 jb .another
 mov esi,llSecrets
 test edi,edi
 jz @f
 add esi,8192
@@:
 lea edx,[llPending+ebx]
.compare:
 mov al,[edx]
 cmp al,[esi]
 jne .another
 inc edx
 inc esi
 loop .compare
 mov eax,[llSecretLen+edi*4]
 mov [matched],eax
 jmp .replace
.another:
 inc edi
 cmp edi,2
 jb .secret
 mov eax,[outCount]
 mov dl,[llPending+ebx]
 mov [llOut+eax],dl
 inc [outCount]
 inc [position]
 jmp .next
.replace:
 mov eax,[matched]
 add [position],eax
 mov esi,llRedacted
 mov edi,llOut
 add edi,[outCount]
 mov ecx,llRedactedEnd-llRedacted
 rep movsb
 add [outCount],llRedactedEnd-llRedacted
 jmp .next
.write:
 stdcall LlRaw,llOut,[outCount]
 mov esi,llPending
 add esi,[position]
 mov edi,llPending
 mov ecx,[llPendingCount]
 sub ecx,[position]
 mov [llPendingCount],ecx
 rep movsb
.done:
 ret
endp

proc LlFeed uses ebx esi edi,data,length
 mov esi,[data]
 mov ebx,[length]
.loop:
 test ebx,ebx
 jz .done
 mov ecx,4096
 cmp ebx,ecx
 jae @f
 mov ecx,ebx
@@:
 mov edi,llPending
 add edi,[llPendingCount]
 add [llPendingCount],ecx
 sub ebx,ecx
 rep movsb
 push esi
 stdcall LlFlush,0
 pop esi
 jmp .loop
.done:
 ret
endp

proc LlText,text
 invoke lstrlenA,[text]
 stdcall LlFeed,[text],eax
 ret
endp
proc LlWide,text
 invoke WideCharToMultiByte,65001,0,[text],-1,llScratch,16384,0,0
 test eax,eax
 jz .done
 dec eax
 stdcall LlFeed,llScratch,eax
.done:
 ret
endp

proc LaunchLogBegin uses ebx esi edi
 cmp [llActive],0
 jne .done
 mov [llActive],1
 stdcall LlSecret,authToken,llSecrets,8192,0
 stdcall LlSecret,curseApiKey,llSecrets+8192,256,1
 invoke lstrcpyW,launchLogDataDirW,eRuntimeDirW
 invoke lstrlenW,eRuntimeDirW
 cmp eax,1800
 ja .bad
 invoke lstrcpyW,llDirectoryW,eRuntimeDirW
 invoke lstrcatW,llDirectoryW,llDirSuffix
 invoke GetFileAttributesW,llDirectoryW
 cmp eax,-1
 je .mkdir
 test eax,400h
 jnz .bad
.mkdir:
 stdcall EnsureDirectory,llDirectoryW
 test eax,eax
 jz .bad
 invoke GetLocalTime,llTime
 invoke GetTickCount
 mov ebx,eax
 movzx eax,[llTime.wYear]
 movzx ecx,[llTime.wMonth]
 movzx edx,[llTime.wDay]
 movzx esi,[llTime.wHour]
 movzx edi,[llTime.wMinute]
 push [launchLogSerial]
 push ebx
 movzx ebx,[llTime.wMilliseconds]
 push ebx
 movzx ebx,[llTime.wSecond]
 push ebx
 push edi esi edx ecx eax
 push llRunName llWideName
 call [wsprintfW]
 add esp,44
 invoke lstrcpyW,launchLogPathW,llDirectoryW
 invoke lstrcatW,launchLogPathW,llWideName
 ; Report filename shares the same unique run suffix; only extension differs.
 invoke lstrcpyW,launchReportPathW,launchLogPathW
 invoke lstrlenW,launchReportPathW
 mov word [launchReportPathW+eax*2-6],'t'
 mov word [launchReportPathW+eax*2-4],'x'
 mov word [launchReportPathW+eax*2-2],'t'
 invoke lstrcpyW,llTempPathW,launchLogPathW
 invoke lstrcatW,llTempPathW,llTmpSuffix
 invoke CreateFileW,llTempPathW,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [llFile],eax
 mov [llBytes],0
 mov [llCap],LL_OUTPUT_LIMIT
 stdcall LlText,llHeader
 movzx eax,[llTime.wYear]
 movzx ecx,[llTime.wMonth]
 movzx edx,[llTime.wDay]
 movzx esi,[llTime.wHour]
 movzx edi,[llTime.wMinute]
 movzx ebx,[llTime.wMilliseconds]
 push ebx
 movzx ebx,[llTime.wSecond]
 push ebx edi esi edx ecx eax
 push llTimeFmt llScratch
 call [wsprintfA]
 add esp,36
 stdcall LlText,llScratch
 stdcall LlText,llVersionLabel
 stdcall LlText,selectedVersion
 stdcall LlText,llNewline
 stdcall LlText,llJavaLabel
 stdcall LlWide,javaPath
 stdcall LlText,llNewline
 stdcall LlText,llDataLabel
 stdcall LlWide,eRuntimeDirW
 stdcall LlText,llNewline
 cinvoke wsprintfA,llScratch,llRamFmt,[memoryMb]
 stdcall LlText,llScratch
.done:
 mov eax,1
 ret
.bad:
 mov [launchLogSaveError],1
 mov word [launchLogPathW],0
 mov word [launchReportPathW],0
 xor eax,eax
 ret
endp

proc LlClosePipes
 cmp [llRead],0
 je @f
 invoke CloseHandle,[llRead]
 mov [llRead],0
@@:
 cmp [llWrite],0
 je @f
 invoke CloseHandle,[llWrite]
 mov [llWrite],0
@@:
 cmp [llInput],-1
 je @f
 invoke CloseHandle,[llInput]
 mov [llInput],-1
@@:
 ret
endp

proc LaunchLogPipe,startup
 mov [llAttr.nLength],sizeof.LL_SECURITY_ATTRIBUTES
 mov [llAttr.lpSecurityDescriptor],0
 mov [llAttr.bInheritHandle],1
 invoke CreatePipe,llRead,llWrite,llAttr,0
 test eax,eax
 jz .bad
 invoke SetHandleInformation,[llRead],1,0
 test eax,eax
 jz .bad
 invoke CreateFileW,llNul,GENERIC_READ,FILE_SHARE_READ+FILE_SHARE_WRITE,llAttr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 mov [llInput],eax
 cmp eax,-1
 je .bad
 mov edx,[startup]
 mov [edx+STARTUPINFO.hStdInput],eax
 mov eax,[llWrite]
 mov [edx+STARTUPINFO.hStdOutput],eax
 mov [edx+STARTUPINFO.hStdError],eax
 or dword [edx+STARTUPINFO.dwFlags],STARTF_USESTDHANDLES+STARTF_USESHOWWINDOW
 mov word [edx+STARTUPINFO.wShowWindow],SW_HIDE
 mov eax,1
 ret
.bad:
 stdcall LlClosePipes
 stdcall LlText,llCaptureMissing
 xor eax,eax
 ret
endp
proc LaunchLogChildStarted
 cmp [llWrite],0
 je @f
 invoke CloseHandle,[llWrite]
 mov [llWrite],0
@@:
 cmp [llInput],-1
 je @f
 invoke CloseHandle,[llInput]
 mov [llInput],-1
@@:
 ret
endp

; Drain continuously even after the output budget is exhausted.
proc LaunchLogWait uses ebx,process
 locals
  available dd ?
  got dd ?
  exited dd ?
  exitedAt dd ?
 endl
 mov [exited],0
.again:
 cmp [exited],0
 je @f
 invoke GetTickCount
 sub eax,[exitedAt]
 cmp eax,500
 jae .done
@@:
 cmp [llRead],0
 je .wait
 invoke PeekNamedPipe,[llRead],0,0,0,addr available,0
 test eax,eax
 jz .wait
 mov ebx,[available]
 test ebx,ebx
 jz .wait
 cmp ebx,4096
 jbe @f
 mov ebx,4096
@@:
 invoke ReadFile,[llRead],llScratch,ebx,addr got,0
 test eax,eax
 jz .wait
 cmp [got],0
 je .wait
 cmp [llBytes],LL_OUTPUT_LIMIT
 jae .truncate
 stdcall LlFeed,llScratch,[got]
 jmp .check
.truncate:
 cmp [llTruncated],0
 jne .check
 mov [llTruncated],1
 ; Do not publish a credential prefix retained at the output boundary.
 mov [llPendingCount],0
.check:
 invoke WaitForSingleObject,[process],0
 test eax,eax
 jnz .again
 cmp [exited],1
 je .again
 mov [exited],1
 invoke GetTickCount
 mov [exitedAt],eax
 jmp .again
.wait:
 cmp [exited],1
 je .done
 invoke WaitForSingleObject,[process],30
 test eax,eax
 jnz .again
 mov [exited],1
 invoke GetTickCount
 mov [exitedAt],eax
 jmp .again
.done:
 stdcall LlClosePipes
 ret
endp

; Copy a bounded tail into the report, redacting with the same streaming filter.
proc LlAppendFile uses ebx esi edi,path,fresh
 locals
  h dd ?
  size dd ?
  high dd ?
  got dd ?
  remaining dd ?
  stamp FILETIME
 endl
 mov [h],-1
 invoke GetFileAttributesW,[path]
 cmp eax,-1
 je .done
 test eax,400h+FILE_ATTRIBUTE_DIRECTORY
 jnz .done
 invoke CreateFileW,[path],GENERIC_READ,FILE_SHARE_READ+FILE_SHARE_WRITE+FILE_SHARE_DELETE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .done
 mov [h],eax
 cmp [fresh],0
 je .size
 invoke GetFileTime,[h],0,0,addr stamp
 test eax,eax
 jz .close
 invoke CompareFileTime,addr stamp,llStart
 cmp eax,0
 jl .close
 invoke CompareFileTime,addr stamp,llEndTime
 cmp eax,0
 jg .close
.size:
 invoke GetFileSize,[h],addr high
 cmp eax,-1
 je .close
 mov [size],eax
 mov [remaining],eax
 stdcall LlText,llAttachment
 stdcall LlWide,[path]
 stdcall LlText,llEndLabel
 cmp [high],0
 jne .tail
 cmp [size],LL_TAIL
 jbe .read
.tail:
 invoke SetFilePointer,[h],-LL_TAIL,0,FILE_END
 cmp eax,-1
 je .close
 mov [remaining],LL_TAIL
 stdcall LlText,llTailLabel
.read:
 mov ebx,[remaining]
 test ebx,ebx
 jz .close
 cmp [llBytes],LL_LIMIT-16384
 jae .close
 cmp ebx,4096
 jbe @f
 mov ebx,4096
@@:
 invoke ReadFile,[h],llScratch,ebx,addr got,0
 test eax,eax
 jz .close
 cmp [got],0
 je .close
 mov eax,[got]
 sub [remaining],eax
 stdcall LlFeed,llScratch,eax
 jmp .read
.close:
 invoke CloseHandle,[h]
.done:
 ret
endp

proc LlNewest uses ebx,glob,suffix
 mov word [llBestW],0
 mov dword [llBestTime],0
 mov dword [llBestTime+4],0
 invoke lstrcpyW,llPathW,launchLogDataDirW
 invoke lstrcatW,llPathW,[glob]
 invoke FindFirstFileW,llPathW,llFind
 cmp eax,-1
 je .done
 mov ebx,eax
.next:
 test [llFind.dwFileAttributes],400h+FILE_ATTRIBUTE_DIRECTORY
 jnz .advance
 invoke CompareFileTime,llFind.ftLastWriteTime,llStart
 cmp eax,0
 jl .advance
 invoke CompareFileTime,llFind.ftLastWriteTime,llEndTime
 cmp eax,0
 jg .advance
 invoke CompareFileTime,llFind.ftLastWriteTime,llBestTime
 cmp eax,0
 jl .advance
 mov eax,[llFind.ftLastWriteTime.dwLowDateTime]
 mov [llBestTime.dwLowDateTime],eax
 mov eax,[llFind.ftLastWriteTime.dwHighDateTime]
 mov [llBestTime.dwHighDateTime],eax
 invoke lstrcpyW,llBestW,launchLogDataDirW
 invoke lstrcatW,llBestW,[suffix]
 invoke lstrcatW,llBestW,llFind.cFileName
.advance:
 invoke FindNextFileW,ebx,llFind
 test eax,eax
 jnz .next
 invoke FindClose,ebx
 cmp word [llBestW],0
 je .done
 stdcall LlAppendFile,llBestW,1
.done:
 ret
endp

proc LaunchLogEnd uses edi,outcome,exitCode,winError,reason
 invoke GetSystemTimeAsFileTime,llEndTime
 ; Directory enumeration can round a just-closed file's timestamp ahead slightly.
 add [llEndTime.dwLowDateTime],10000000
 adc [llEndTime.dwHighDateTime],0
 cmp [llActive],0
 jne @f
 stdcall LaunchLogBegin
@@:
 stdcall LaunchLogChildStarted
 stdcall LlClosePipes
 mov eax,[exitCode]
 mov [launchExitCode],eax
 mov eax,[winError]
 mov [launchWin32Error],eax
 stdcall LlFlush,1
 mov [llCap],LL_LIMIT
 cmp [llBytes],LL_OUTPUT_LIMIT
 jb @f
 mov [llTruncated],1
@@:
 cmp [llTruncated],0
 je @f
 stdcall LlText,llOmitted
@@:
 cinvoke wsprintfA,llScratch,llSummaryFmt,[outcome],[exitCode],[winError]
 stdcall LlText,llScratch
 stdcall LlText,[reason]
 stdcall LlText,llNewline
 stdcall LlFlush,1
 cmp [llFile],-1
 je .cleanup
 invoke CloseHandle,[llFile]
 mov [llFile],-1
 cmp [llFailed],0
 jne .cleanup
 invoke MoveFileExW,llTempPathW,launchLogPathW,8
 test eax,eax
 jz .reportBad
 mov [launchLogReady],1
 cmp [outcome],1
 jne .cleanup
 mov [llBytes],0
 invoke lstrcpyW,llTempPathW,launchReportPathW
 invoke lstrcatW,llTempPathW,llTmpSuffix
 invoke CreateFileW,llTempPathW,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .reportBad
 mov [llFile],eax
 stdcall LlText,llReportHeader
 stdcall LlAppendFile,launchLogPathW,0
 invoke lstrcpyW,llPathW,launchLogDataDirW
 invoke lstrcatW,llPathW,llLatest
 stdcall LlAppendFile,llPathW,1
 stdcall LlNewest,llCrashGlob,llCrashDir
 stdcall LlNewest,llHsGlob,llSlash
 stdcall LlFlush,1
 invoke CloseHandle,[llFile]
 mov [llFile],-1
 cmp [llFailed],0
 jne .cleanup
 invoke MoveFileExW,llTempPathW,launchReportPathW,8
 test eax,eax
 jz .reportBad
 mov [launchCrashReady],1
 jmp .cleanup
.reportBad:
 mov [launchLogSaveError],1
.cleanup:
 mov [llActive],0
 mov edi,llSecrets
 xor eax,eax
 mov ecx,(8192+256)/4
 rep stosd
 mov edi,llPending
 mov ecx,16384/4
 rep stosd
 mov [llPendingCount],0
 ret
endp

; UI reads only published, sanitized files, never engine JSON/file buffers.
proc LaunchLogRead uses ebx,kind,destination,capacity
 local h:DWORD,size:DWORD,high:DWORD,got:DWORD
 mov eax,[destination]
 cmp [capacity],2
 jb .bad
 mov byte [eax],0
 mov eax,launchLogPathW
 cmp [kind],0
 jne .crash
 cmp [launchLogReady],1
 jne .bad
 jmp .open
.crash:
 cmp [launchCrashReady],1
 jne .bad
 mov eax,launchReportPathW
.open:
 invoke CreateFileW,eax,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [h],eax
 invoke GetFileSize,eax,addr high
 mov [size],eax
 mov ebx,[capacity]
 dec ebx
 cmp [high],0
 jne .tail
 cmp eax,ebx
 jbe .read
.tail:
 neg ebx
 invoke SetFilePointer,[h],ebx,0,FILE_END
 neg ebx
.read:
 invoke ReadFile,[h],[destination],ebx,addr got,0
 mov ebx,eax
 invoke CloseHandle,[h]
 test ebx,ebx
 jz .bad
 mov edx,[destination]
 mov eax,[got]
 mov byte [edx+eax],0
 ret
.bad:
 xor eax,eax
 ret
endp

proc LaunchLogExport,destination
 stdcall LaunchLogExportTo,[destination],0
 ret
endp

; The UI may allow replacement only after the native Save dialog confirms it.
proc LaunchLogExportTo,destination,replaceAllowed
 mov eax,launchReportPathW
 cmp [launchCrashReady],1
 je .copy
 cmp [launchLogReady],1
 jne .bad
 mov eax,launchLogPathW
.copy:
 stdcall LlExportPath,eax,[destination],[replaceAllowed]
 ret
.bad:
 xor eax,eax
 ret
endp

proc LaunchLogExportKindTo,kind,destination,replaceAllowed
 cmp [kind],0
 je .launch
 cmp [kind],1
 jne .bad
 cmp [launchCrashReady],1
 jne .bad
 mov eax,launchReportPathW
 jmp .copy
.launch:
 cmp [launchLogReady],1
 jne .bad
 mov eax,launchLogPathW
.copy:
 stdcall LlExportPath,eax,[destination],[replaceAllowed]
 ret
.bad:
 xor eax,eax
 ret
endp

proc LlExportPath uses ebx esi edi,source,destination,replaceAllowed
 locals
  sourcePath rw 2048
  targetPath rw 2048
  tempPath rw 2048
  parentPath rw 2048
  suffix rw 64
 endl
 invoke lstrcpyW,addr sourcePath,[source]
 invoke GetFileAttributesW,addr sourcePath
 cmp eax,-1
 je .bad
 test eax,400h+FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 invoke GetFullPathNameW,[destination],2048,addr targetPath,0
 test eax,eax
 jz .bad
 cmp eax,1980
 ja .bad
 cmp eax,5
 jb .bad
 ; Refuse alternate streams and invalid file-name syntax even after canonicalizing.
 lea esi,[targetPath]
 xor edi,edi
.syntax:
 mov ax,[esi+edi*2]
 test ax,ax
 jz .syntaxDone
 cmp ax,32
 jb .bad
 cmp ax,127
 je .bad
 cmp ax,':'
 jne .ordinary
 cmp edi,1
 jne .bad
 jmp .nextChar
.ordinary:
 cmp ax,'"'
 je .bad
 cmp ax,'<'
 je .bad
 cmp ax,'>'
 je .bad
 cmp ax,'|'
 je .bad
 cmp ax,'?'
 je .bad
 cmp ax,'*'
 je .bad
.nextChar:
 inc edi
 jmp .syntax
.syntaxDone:
 invoke lstrlenW,addr targetPath
 lea esi,[targetPath]
 lea esi,[esi+eax*2-8]
 invoke lstrcmpiW,esi,llTxtExt
 test eax,eax
 jz .extension
 invoke lstrcmpiW,esi,llLogExt
 test eax,eax
 jnz .bad
.extension:
 invoke lstrcmpiW,addr sourcePath,addr targetPath
 test eax,eax
 jz .bad
 invoke GetFileAttributesW,addr targetPath
 cmp eax,-1
 je .parent
 test eax,400h+FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 cmp [replaceAllowed],1
 jne .bad
.parent:
 invoke lstrcpyW,addr parentPath,addr targetPath
 invoke lstrlenW,addr parentPath
 lea edi,[parentPath]
 lea edi,[edi+eax*2]
.back:
 sub edi,2
 lea eax,[parentPath]
 cmp edi,eax
 jbe .bad
 cmp word [edi],92
 jne .back
 lea eax,[parentPath]
 add eax,4
 cmp edi,eax
 jne .trimParent
 cmp word [edi-2],':'
 jne .trimParent
 mov word [edi+2],0
 jmp .parentReady
.trimParent:
 mov word [edi],0
.parentReady:
 if defined OFFSET_VERSION_SETTINGS
 stdcall VsDirectoryChain,addr parentPath,0
 test eax,eax
 jz .bad
 else
 invoke GetFileAttributesW,addr parentPath
 cmp eax,-1
 je .bad
 test eax,400h
 jnz .bad
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .bad
 end if
 invoke lstrcpyW,addr tempPath,addr targetPath
 invoke GetTickCount
 cinvoke wsprintfW,addr suffix,llExportFmt,eax
 invoke lstrcatW,addr tempPath,addr suffix
 invoke CopyFileW,addr sourcePath,addr tempPath,1
 test eax,eax
 jz .bad
 mov ebx,8
 cmp [replaceAllowed],1
 jne @f
 or ebx,1
@@:
 invoke MoveFileExW,addr tempPath,addr targetPath,ebx
 test eax,eax
 jnz .done
 invoke DeleteFileW,addr tempPath
 jmp .bad
.done:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Read-only recovery after restarting the launcher. Normal runs hide older crashes.
proc LaunchLogDiscover uses ebx esi edi,dataDirectory
 cmp [llActive],0
 jne .bad
 mov [launchLogReady],0
 mov [launchCrashReady],0
 mov word [launchLogPathW],0
 mov word [launchReportPathW],0
 invoke lstrlenW,[dataDirectory]
 cmp eax,1800
 ja .bad
 invoke lstrcpyW,llPathW,[dataDirectory]
 invoke lstrcatW,llPathW,llDirSuffix
 invoke GetFileAttributesW,llPathW
 cmp eax,-1
 je .bad
 test eax,400h
 jnz .bad
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .bad
 mov word [llBestW],0
 mov dword [llBestTime],0
 mov dword [llBestTime+4],0
 invoke lstrcpyW,llPathW,[dataDirectory]
 invoke lstrcatW,llPathW,llFinalGlob
 invoke FindFirstFileW,llPathW,llFind
 cmp eax,-1
 je .bad
 mov ebx,eax
.next:
 test [llFind.dwFileAttributes],400h+FILE_ATTRIBUTE_DIRECTORY
 jnz .advance
 invoke CompareFileTime,llFind.ftLastWriteTime,llBestTime
 cmp eax,0
 jl .advance
 mov eax,[llFind.ftLastWriteTime.dwLowDateTime]
 mov [llBestTime.dwLowDateTime],eax
 mov eax,[llFind.ftLastWriteTime.dwHighDateTime]
 mov [llBestTime.dwHighDateTime],eax
 invoke lstrcpyW,llBestW,[dataDirectory]
 invoke lstrcatW,llBestW,llDirSuffix
 invoke lstrcatW,llBestW,llSlash
 invoke lstrcatW,llBestW,llFind.cFileName
.advance:
 invoke FindNextFileW,ebx,llFind
 test eax,eax
 jnz .next
 invoke FindClose,ebx
 cmp word [llBestW],0
 je .bad
 invoke lstrcpyW,launchLogPathW,llBestW
 invoke lstrcpyW,launchReportPathW,llBestW
 invoke lstrcpyW,launchLogDataDirW,[dataDirectory]
 mov [launchLogReady],1
 invoke lstrlenW,launchReportPathW
 mov word [launchReportPathW+eax*2-6],'t'
 mov word [launchReportPathW+eax*2-4],'x'
 mov word [launchReportPathW+eax*2-2],'t'
 invoke GetFileAttributesW,launchReportPathW
 cmp eax,-1
 je .done
 test eax,400h+FILE_ATTRIBUTE_DIRECTORY
 jnz .done
 mov [launchCrashReady],1
.done:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
