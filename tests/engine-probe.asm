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
fixtureVersion db 'probe',0
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
 stdcall EngineLaunch,0
 xor eax,1
 invoke ExitProcess,eax
 .configbad: invoke ExitProcess,2
proc SetStatusA,text
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\imports.inc'





