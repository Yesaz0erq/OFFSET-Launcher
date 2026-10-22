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
sample db '{"rules":[{"action":"allow","os":{"name":"windows"}}],"arguments":{"jvm":["-cp","${classpath}"],"game":["--username","${auth_player_name}"]}}',0
sample2 db 'x${auth_player_name}/${auth_uuid}',0
badVersion db '../escape',0
goodVersion db '1.21.1',0
badName db 'bad/name',0
goodName db 'Notch',0
unknownPh db '${unknown_placeholder}',0
secretPh db '-Dsecret=${auth_access_token}',0
secretJson db '"-Dsecret=${auth_access_token}"',0
otherRules db '{"rules":[{"action":"allow","os":{"name":"linux"}}]}',0
nativeRoot du 'fixture-root',0
nativeVersionW du '1.21.1',0
nativeExpected du 'fixture-root\versions\1.21.1\offset-natives-A1-b2',0
nativeDefault du 'fixture-root\versions\1.21.1\offset-natives',0
nativeValid db '{"offsetNatives":"offset-natives-A1-b2"}',0
nativeMissing db '{}',0
nativeEscape db '{"offsetNatives":"offset-natives-../other"}',0
nativeNul db '{"offsetNatives":"offset-natives-safe\u0000/other"}',0
nativeEmpty db '{"offsetNatives":"offset-natives-"}',0
nativeType db '{"offsetNatives":1}',0
nativeAbsolute db '{"offsetNatives":"C:/other"}',0
nativeCases dd nativeEscape,nativeNul,nativeEmpty,nativeType,nativeAbsolute,0
section '.text' code readable executable
start:
 stdcall EngineInit
 stdcall EIdentity
 test eax,eax
 jz fail1
 invoke lstrcmpA,eUuid,expectedUuid
 test eax,eax
 jnz fail2
 stdcall JsonParse,sample
 test eax,eax
 jz fail3
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall EAllowed,[eMeta],eax
 cmp eax,1
 jne fail4
 stdcall EExpand,sample2
 test eax,eax
 jz fail5
 stdcall JsonFree,[eMeta]
 stdcall JsonParse,secretJson
 mov [eMeta],eax
 stdcall JsonRoot,eax
 stdcall EEmitToken,eax,0
 test eax,eax
 jnz fail11
 stdcall JsonFree,[eMeta]
 stdcall EContainsToken,secretPh
 cmp eax,1
 jne fail6
 stdcall EContainsToken,sample2
 test eax,eax
 jnz fail6
 stdcall EExpand,unknownPh
 test eax,eax
 jnz fail7
 invoke lstrcpyA,selectedVersion,badVersion
 stdcall EValidateVersion
 test eax,eax
 jnz fail8
 invoke lstrcpyA,selectedVersion,goodVersion
 invoke lstrcpyA,playerName,badName
 stdcall EIdentity
 test eax,eax
 jnz fail9
 invoke lstrcpyA,playerName,goodName
 stdcall JsonParse,otherRules
 mov [eMeta],eax
 stdcall JsonRoot,eax
 stdcall EAllowed,[eMeta],eax
 test eax,eax
 jnz fail10
 stdcall JsonFree,[eMeta]
 stdcall NativePathTests
 test eax,eax
 jz fail12
 ; Reference worker entries so all code is checked by FASM's used optimization.
 mov eax,EngineInstall
 mov eax,EngineLaunch
 invoke ExitProcess,0
fail1: invoke ExitProcess,11
fail2: invoke ExitProcess,12
fail3: invoke ExitProcess,13
fail4: invoke ExitProcess,14
fail5: invoke ExitProcess,15
fail6: invoke ExitProcess,16
fail7: invoke ExitProcess,17
fail8: invoke ExitProcess,18
fail9: invoke ExitProcess,19
fail10: invoke ExitProcess,20
fail11: invoke ExitProcess,21
fail12: invoke ExitProcess,22

proc NativePathTests uses esi
 invoke lstrcpyW,gameDir,nativeRoot
 invoke lstrcpyW,eVersionW,nativeVersionW
 stdcall JsonParse,nativeValid
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall ESelectNativePath
 test eax,eax
 jz .bad
 invoke lstrcmpiW,eNativePath,nativeExpected
 test eax,eax
 jnz .bad
 stdcall JsonFree,[eMeta]
 mov esi,nativeCases
.next:
 lodsd
 test eax,eax
 jz .default
 stdcall JsonParse,eax
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall ESelectNativePath
 test eax,eax
 jnz .bad
 stdcall JsonFree,[eMeta]
 jmp .next
.default:
 stdcall JsonParse,nativeMissing
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall ESelectNativePath
 test eax,eax
 jz .bad
 invoke lstrcmpiW,eNativePath,nativeDefault
 test eax,eax
 jnz .bad
 stdcall JsonFree,[eMeta]
 mov [eMeta],0
 mov eax,1
 ret
.bad:
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
include '..\src\imports.inc'




