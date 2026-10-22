format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
WM_APP=8000h
OFFSET_LAUNCH_LOG=1
section '.data' data readable writeable
hWnd dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion rb 128
memoryMb dd 768
onlineMode dd 1
clientId rb 128
authReady dd 1
authName db 'Notch',0,58 dup(0)
authUuid db 'b50ad385829d3141a2167e7d7539ba7f',0,31 dup(0)
authToken db 'session-token-0123456789-abcdefghijklmnopqrstuvwxyz-END',0
 rb 8192-($-authToken)
authXuid rb 64
curseApiKey db 'fixture-curse-key-0123456789-abcdef',0
 rb 256-($-curseApiKey)
testRootEnv du 'OFFSET_LOG_GAME',0
testJavaEnv du 'OFFSET_LOG_JAVA',0
testRoot rw 1024
testJava rw 1024
testInvalidJava du '\invalid-java.exe',0
testFailureDir du '\log-save-failure',0
testIniSuffix du '\settings.ini',0
testFailureReason db 'fixture preparation failure',0
testSuccess db 'success',0
testCrash db 'crash',0
testSpam db 'spam',0
testMissing db 'missing',0
testExport du '\export.txt',0
testKindExport du '\startup-export.log',0
testExportPath rw 2048
testSavedLog rw 2048
testOutput rb 131072
testNeedStdout db 'stdout unicode 标准输出 中文 日本語',0
testNeedStderr db 'stderr captured 初始化失败 中文 日本語',0
testNeedRedacted db '[redacted]',0
testNeedFresh db 'fresh-game-log',0
testNeedCrash db 'fresh-crash-report',0
testNeedJvm db 'fresh-jvm-report',0
testStale db 'STALE-MUST-NOT-BE-ATTACHED',0
testTruncated db 'Output limit reached',0
step dd 1
testSerial dd 0
testWritten dd 0
testDigest rb 41
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,testRootEnv,testRoot,1024
 test eax,eax
 jz failed
 invoke GetEnvironmentVariableW,testJavaEnv,testJava,1024
 test eax,eax
 jz failed
 invoke lstrcpyW,gameDir,testRoot
 invoke lstrcpyW,javaPath,testJava
 invoke lstrcpyA,selectedVersion,testSuccess
 stdcall EngineLaunch,0
 cmp eax,1
 jne failed
 cmp [launchLogReady],1
 jne failed
 cmp [launchCrashReady],0
 jne failed
 cmp [launchExitCode],0
 jne failed
 stdcall LaunchLogRead,0,testOutput,131072
 test eax,eax
 jz failed
 stdcall CheckOutput
 test eax,eax
 jz failed
 invoke lstrcpyW,testSavedLog,launchLogPathW
 inc [step]
 invoke lstrcpyA,selectedVersion,testCrash
 stdcall EngineLaunch,0
 test eax,eax
 jnz failed
 cmp [launchCrashReady],1
 jne failed
 cmp [launchExitCode],23
 jne failed
 invoke lstrcmpiW,testSavedLog,launchLogPathW
 test eax,eax
 jz failed
 stdcall LaunchLogRead,1,testOutput,131072
 test eax,eax
 jz failed
 stdcall CheckOutput
 test eax,eax
 jz failed
 stdcall Contains,testOutput,testNeedFresh
 test eax,eax
 jz failed
 stdcall Contains,testOutput,testNeedCrash
 test eax,eax
 jz failed
 stdcall Contains,testOutput,testNeedJvm
 test eax,eax
 jz failed
 stdcall Contains,testOutput,testStale
 test eax,eax
 jnz failed
 mov eax,[launchLogSerial]
 mov [testSerial],eax
 mov [launchLogReady],0
 mov [launchCrashReady],0
 stdcall LaunchLogDiscover,gameDir
 cmp eax,1
 jne failed
 cmp [launchCrashReady],1
 jne failed
 mov eax,[testSerial]
 cmp eax,[launchLogSerial]
 jne failed
 ; Discovery must make both actual files readable on the first request.
 stdcall LaunchLogRead,0,testOutput,131072
 cmp eax,16
 jb failed
 cmp dword [testOutput],'OFFS'
 jne failed
 stdcall CheckOutput
 test eax,eax
 jz failed
 stdcall LaunchLogRead,1,testOutput,131072
 cmp eax,16
 jb failed
 cmp dword [testOutput],'OFFS'
 jne failed
 stdcall CheckOutput
 test eax,eax
 jz failed
 inc [step]
 invoke lstrcpyW,testExportPath,gameDir
 invoke lstrcatW,testExportPath,testExport
 stdcall LaunchLogExport,testExportPath
 cmp eax,1
 jne failed
 stdcall LaunchLogExport,testExportPath
 test eax,eax
 jnz failed
 stdcall LaunchLogExport,launchReportPathW
 test eax,eax
 jnz failed
 stdcall LaunchLogExportTo,testExportPath,1
 cmp eax,1
 jne failed
 invoke lstrcpyW,testExportPath,gameDir
 invoke lstrcatW,testExportPath,testKindExport
 stdcall LaunchLogExportKindTo,0,testExportPath,1
 cmp eax,1
 jne failed
 stdcall Sha1File,launchLogPathW,testDigest
 test eax,eax
 jz failed
 stdcall VerifySha1,testExportPath,testDigest
 cmp eax,1
 jne failed
 stdcall LaunchLogExportKindTo,2,testExportPath,1
 test eax,eax
 jnz failed
 invoke lstrcpyW,testExportPath,gameDir
 invoke lstrcatW,testExportPath,testIniSuffix
 stdcall LaunchLogExportTo,testExportPath,1
 test eax,eax
 jnz failed
 inc [step]
 invoke lstrcpyW,javaPath,gameDir
 invoke lstrcatW,javaPath,testInvalidJava
 stdcall EngineLaunch,0
 test eax,eax
 jnz failed
 cmp [launchCrashReady],1
 jne failed
 cmp [launchExitCode],-1
 jne failed
 cmp [launchWin32Error],0
 je failed
 inc [step]
 invoke lstrcpyW,javaPath,testJava
 invoke lstrcpyA,selectedVersion,testMissing
 stdcall EngineLaunch,0
 test eax,eax
 jnz failed
 cmp [launchCrashReady],1
 jne failed
 cmp [launchExitCode],-1
 jne failed
 cmp [launchWin32Error],0
 jne failed
 inc [step]
 invoke lstrcpyA,selectedVersion,testSuccess
 mov [engineCancel],1
 stdcall EngineLaunch,0
 test eax,eax
 jnz failed
 mov [engineCancel],0
 cmp [launchLogReady],1
 jne failed
 cmp [launchCrashReady],0
 jne failed
 cmp [launchExitCode],-1
 jne failed
 inc [step]
 invoke lstrcpyA,selectedVersion,testSpam
 stdcall EngineLaunch,0
 cmp eax,1
 jne failed
 cmp [launchCrashReady],0
 jne failed
 stdcall LaunchLogRead,0,testOutput,131072
 test eax,eax
 jz failed
 stdcall Contains,testOutput,testTruncated
 test eax,eax
 jz failed
 stdcall Contains,testOutput,authToken
 test eax,eax
 jnz failed
 ; Discovery associates only the most recent completed run, not an old crash.
 mov [launchLogReady],0
 mov [launchCrashReady],0
 stdcall LaunchLogDiscover,gameDir
 cmp eax,1
 jne failed
 cmp [launchLogReady],1
 jne failed
 cmp [launchCrashReady],0
 jne failed
 inc [step]
 ; A failed output path must not advertise a saved log or report.
 invoke lstrcpyW,gameDir,testRoot
 invoke lstrcatW,gameDir,testFailureDir
 invoke lstrcpyA,selectedVersion,testMissing
 stdcall EngineLaunch,0
 test eax,eax
 jnz failed
 cmp [launchLogReady],0
 jne failed
 cmp [launchCrashReady],0
 jne failed
 cmp [launchLogSaveError],1
 jne failed
 cmp [llRead],0
 jne failed
 cmp [llWrite],0
 jne failed
 cmp [llInput],-1
 jne failed
 cmp [llFile],-1
 jne failed
 inc [step]
 ; Long credentials spanning multiple pipe-sized chunks are never persisted.
 invoke lstrcpyW,gameDir,testRoot
 invoke lstrcpyW,eRuntimeDirW,testRoot
 mov edi,authToken
 mov ecx,8191
 mov al,'A'
 rep stosb
 mov byte [edi],0
 mov byte [curseApiKey],0
 stdcall LaunchLogReset
 stdcall LaunchLogBegin
 stdcall LlFeed,authToken,8191
 stdcall LaunchLogEnd,0,0,0,testFailureReason
 cmp [launchLogReady],1
 jne failed
 stdcall LaunchLogRead,0,testOutput,131072
 test eax,eax
 jz failed
 stdcall Contains,testOutput,authToken
 test eax,eax
 jnz failed
 stdcall Contains,testOutput,testNeedRedacted
 test eax,eax
 jz failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[step]
proc CheckOutput
 stdcall Contains,testOutput,testNeedStdout
 test eax,eax
 jz .bad
 stdcall Contains,testOutput,testNeedStderr
 test eax,eax
 jz .bad
 stdcall Contains,testOutput,testNeedRedacted
 test eax,eax
 jz .bad
 stdcall Contains,testOutput,authToken
 test eax,eax
 jnz .bad
 stdcall Contains,testOutput,curseApiKey
 test eax,eax
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc Contains uses esi edi ebx,text,needle
 mov esi,[text]
.next:
 cmp byte [esi],0
 je .no
 mov edi,[needle]
 mov ebx,esi
.compare:
 mov al,[edi]
 test al,al
 jz .yes
 cmp al,[ebx]
 jne .advance
 inc edi
 inc ebx
 jmp .compare
.advance:
 inc esi
 jmp .next
.yes:
 mov eax,1
 ret
.no:
 xor eax,eax
 ret
endp
proc SetStatusA,text
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\launch_log.inc'
include '..\src\imports.inc'
