format PE console 4.0
entry start
stack 100000h,40000h
include '..\tools\INCLUDE\win32ax.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion db '1.21.1',0,121 dup(0)
memoryMb dd 4096
onlineMode dd 0
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authReady dd 0
authXuid rb 64
expectedUuid db 'b50ad385829d3141a2167e7d7539ba7f',0
fixtureDirEnv du 'OFFSET_TEST_GAME',0
fixtureJavaEnv du 'OFFSET_TEST_JAVA',0
fixtureVersion db '1.21.1',0
fixtureVersionEnv du 'OFFSET_TEST_VERSION',0
statusEnv du 'OFFSET_TEST_STATUS',0
statusPath rw 2048
sample db '{"rules":[{"action":"allow","os":{"name":"windows"}}],"arguments":{"jvm":["-cp","${classpath}"],"game":["--username","${auth_player_name}"]}}',0
sample2 db 'x${auth_player_name}/${auth_uuid}',0
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,fixtureDirEnv,gameDir,1024
 test eax,eax
 jz .configbad
 invoke GetEnvironmentVariableW,fixtureJavaEnv,javaPath,1024
 test eax,eax
 jz .configbad
 invoke lstrcpyA,selectedVersion,fixtureVersion
 invoke GetEnvironmentVariableW,fixtureVersionEnv,eWide,128
 test eax,eax
 jz .launch
 cmp eax,128
 jae .configbad
 invoke WideCharToMultiByte,65001,0,eWide,-1,selectedVersion,128,0,0
 test eax,eax
 jz .configbad
.launch:
 stdcall EngineLaunch,0
 xor eax,1
 invoke ExitProcess,eax
 .configbad: invoke ExitProcess,2
proc SetStatusA uses ebx esi, text
 local f:DWORD, n:DWORD
 invoke GetEnvironmentVariableW,statusEnv,statusPath,2048
 test eax,eax
 jz .done
 invoke CreateFileW,statusPath,GENERIC_WRITE,FILE_SHARE_READ or FILE_SHARE_WRITE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .done
 mov [f],eax
 invoke lstrlenA,[text]
 invoke WriteFile,[f],[text],eax,addr n,0
 invoke CloseHandle,[f]
 .done:
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\imports.inc'







