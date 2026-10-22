; Real JSON and isolated file operations, deterministic loader HTTP fixtures.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\version_types.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion db '1.21.1',0,121 dup(0)
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
testNo dd 1
fixtureMode dd 0
requestCalls dd 0
liveMode dd 0
liveKind dd 0
liveBase rb 128
liveVersion rb 128
liveGameEnv du 'OFFSET_LOADER_GAME',0
liveBaseEnv du 'OFFSET_LOADER_BASE',0
liveKindEnv du 'OFFSET_LOADER_KIND',0
liveVersionEnv du 'OFFSET_LOADER_VERSION',0
liveJavaEnv du 'OFFSET_LOADER_JAVA',0
liveResult du '\loader-result.txt',0
liveNewline db 13,10,0
fixtureRoot du 'tests\out\loader-unit-%u',0
fixtureBase db '1.21.1',0
fixtureOld db '1.20.1',0
fixtureSnapshot db '25w14a',0
fixtureFuture db '26.1',0
fixtureModernPatch db '26.1.2',0
fixtureModernLatest db '26.3',0
fixtureModernSnapshot db '26.3-snapshot-1',0
fixtureModernPrefix db '26.1.0.',0
fixtureModernPatchPrefix db '26.1.2.',0
fixtureModernLatestPrefix db '26.3.0.',0
fixtureModernNeo db '26.1.0.13',0
fixturePre db '1.21.1-pre1',0
fixtureVersion db '0.16.9',0
fixtureNewer db '0.16.10',0
fixtureRelease db '0.17.0',0
fixtureBeta db '0.17.0-beta.1',0
fixtureForgeNew db '52.0.10',0
fixtureNeoNew db '21.1.10',0
fixtureQuiltNew db '0.28.0',0
fixtureBadId db '../escape',0
fixtureColon db '1.21.1:unsafe',0
fixtureDot db 'bad..id',0
fixtureEmpty db 0
fixturePrefix db '21.1.',0
fixtureId db 'fabric-loader-0.16.9-1.21.1',0
fixtureFabricJson db '[{"loader":{"version":"0.16.9","stable":true}},{"loader":{"version":"0.16.10","stable":true}},{"loader":{"version":"0.17.0-beta.1","stable":false}},{"loader":{"version":"0.17.0","stable":true}},{"loader":{"version":"0.16.10","stable":true}},{"loader":{"version":"bad/unsafe"}},{"loader":{"version":"0.19.0\u0000evil"}}]',0
fixtureQuiltJson db '[{"loader":{"version":"0.27.9"}},{"loader":{"version":"0.28.0-beta.1"}},{"loader":{"version":"0.28.0"}}]',0
fixtureForgeJson db '{"1.21.1":["1.21.1-52.0.9","1.21.1-52.0.10","1.21.10-99.1.0","1.21.1-../unsafe"]}',0
fixtureNeoJson db '{"versions":["21.1.9","21.1.10","21.1.10-beta","21.10.1","20.4.1","26.1.0.12","26.1.0.13","26.1.0.0-alpha.0+snapshot-1.20260303.175753","26.1.0.11-beta"]}',0
fixtureBadJson db '{"versions":broken}',0
fixtureGoodProfile db '{"id":"fabric-loader-0.16.9-1.21.1","inheritsFrom":"1.21.1","mainClass":"test.Main","arguments":{"jvm":[],"game":[]},"libraries":[]}',0
fixtureChangedProfile db '{"id":"fabric-loader-0.16.9-1.21.1","inheritsFrom":"1.21.1","mainClass":"test.Changed","arguments":{"jvm":[],"game":[]},"libraries":[]}',0
fixtureBadParent db '{"id":"fabric-loader-0.16.9-1.21.1","inheritsFrom":"1.20.1"}',0
fixtureBadProfile db '{"id":"../escape","inheritsFrom":"1.21.1"}',0
fixtureNulProfile db '{"id":"fabric-loader-0.16.9-1.21.1\u0000alias","inheritsFrom":"1.21.1"}',0
fixtureSha db '0123456789ABCDEF0123456789abcdef01234567',13,10,0
fixtureBadSha db '0123456789abcdef0123456789abcdef01234567 extra',0
expectedSha db '0123456789abcdef0123456789abcdef01234567',0
fixtureJava du 'C:\Program Files\Java\bin\javaw.exe',0
fixtureArgument du 'C:\Unicode Path\',0
expectedCommand db '"C:\Program Files\Java\bin\javaw.exe" "-jar" "C:\Unicode Path\\"',0
fixtureTarget rb 128
fixtureBuffer rb 256
fixtureWide rw 2048
fixtureDirectory rw 2048
fixturePath rw 2048
fixtureBackup rw 2048

section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,liveGameEnv,gameDir,1024
 test eax,eax
 jnz liveInstall
 stdcall LoaderSafeId,fixtureBadId
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LoaderSafeId,fixtureColon
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LoaderSafeId,fixtureDot
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LoaderSafeId,fixtureEmpty
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LoaderSafeId,fixtureId
 cmp eax,1
 jne failed
 inc [testNo]
 stdcall LdCompare,fixtureNewer,fixtureVersion
 cmp eax,1
 jne failed
 inc [testNo]
 stdcall LdCompare,fixtureRelease,fixtureBeta
 cmp eax,1
 jne failed
 inc [testNo]
 stdcall LdCompare,fixtureBeta,fixtureRelease
 cmp eax,-1
 jne failed
 inc [testNo]
 stdcall LdNeoPrefix,fixtureBase,fixtureBuffer
 cmp eax,1
 jne failed
 invoke lstrcmpA,fixtureBuffer,fixturePrefix
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdNeoPrefix,fixtureOld,fixtureBuffer
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdNeoPrefix,fixtureSnapshot,fixtureBuffer
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdNeoPrefix,fixturePre,fixtureBuffer
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdNeoPrefix,fixtureFuture,fixtureBuffer
 cmp eax,1
 jne failed
 invoke lstrcmpA,fixtureBuffer,fixtureModernPrefix
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdNeoPrefix,fixtureModernPatch,fixtureBuffer
 cmp eax,1
 jne failed
 invoke lstrcmpA,fixtureBuffer,fixtureModernPatchPrefix
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdNeoPrefix,fixtureModernLatest,fixtureBuffer
 cmp eax,1
 jne failed
 invoke lstrcmpA,fixtureBuffer,fixtureModernLatestPrefix
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdNeoPrefix,fixtureModernSnapshot,fixtureBuffer
 test eax,eax
 jnz failed
 inc [testNo]
 mov [fixtureMode],0
 stdcall LoaderFetchVersions,fixtureBase,1
 cmp eax,1
 jne failed
 cmp [loaderVersionCount],4
 jne failed
 invoke lstrcmpA,loaderVersions,fixtureRelease
 test eax,eax
 jnz failed
 cmp dword [loaderVersions+VL_STABLE],1
 jne failed
 invoke lstrcmpA,loaderVersions+VL_SIZE,fixtureBeta
 test eax,eax
 jnz failed
 cmp dword [loaderVersions+VL_SIZE+VL_STABLE],0
 jne failed
 inc [testNo]
 mov [fixtureMode],1
 stdcall LoaderFetchVersions,fixtureBase,4
 cmp eax,1
 jne failed
 cmp [loaderVersionCount],3
 jne failed
 invoke lstrcmpA,loaderVersions,fixtureQuiltNew
 test eax,eax
 jnz failed
 inc [testNo]
 mov [fixtureMode],2
 stdcall LoaderFetchVersions,fixtureBase,2
 cmp eax,1
 jne failed
 cmp [loaderVersionCount],2
 jne failed
 invoke lstrcmpA,loaderVersions,fixtureForgeNew
 test eax,eax
 jnz failed
 inc [testNo]
 mov [fixtureMode],3
 stdcall LoaderFetchVersions,fixtureBase,3
 cmp eax,1
 jne failed
 cmp [loaderVersionCount],3
 jne failed
 invoke lstrcmpA,loaderVersions,fixtureNeoNew
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LoaderFetchVersions,fixtureFuture,3
 cmp eax,1
 jne failed
 cmp [loaderVersionCount],2
 jne failed
 invoke lstrcmpA,loaderVersions,fixtureModernNeo
 test eax,eax
 jnz failed
 inc [testNo]
 mov eax,[requestCalls]
 mov ebx,eax
 stdcall LoaderFetchVersions,fixtureSnapshot,3
 cmp eax,1
 jne failed
 cmp [loaderVersionCount],0
 jne failed
 cmp [requestCalls],ebx
 jne failed
 inc [testNo]
 mov [fixtureMode],4
 stdcall LoaderFetchVersions,fixtureBase,3
 test eax,eax
 jnz failed
 cmp [loaderVersionCount],0
 jne failed
 inc [testNo]
 mov [engineCancel],1
 mov ebx,[requestCalls]
 stdcall LoaderFetchVersions,fixtureBase,1
 test eax,eax
 jnz failed
 cmp [requestCalls],ebx
 jne failed
 mov [engineCancel],0
 inc [testNo]
 stdcall LdSha1,fixtureSha,fixtureBuffer
 cmp eax,1
 jne failed
 invoke lstrcmpA,fixtureBuffer,expectedSha
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdSha1,fixtureBadSha,fixtureBuffer
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall LdCommandStart,fixtureJava
 test eax,eax
 jz failed
 stdcall EAppendCommand,ldJavaJar
 stdcall LdCommandWide,fixtureArgument
 test eax,eax
 jz failed
 invoke lstrcmpA,eCommand,expectedCommand
 test eax,eax
 jnz failed
 ; File work is in a unique per-run test directory, preserving old fixtures.
 inc [testNo]
 invoke GetTickCount
 cinvoke wsprintfW,fixtureWide,fixtureRoot,eax
 invoke GetFullPathNameW,fixtureWide,1024,gameDir,0
 test eax,eax
 jz failed
 mov [fixtureMode],5
 stdcall LoaderInstall,fixtureBase,1,fixtureVersion,fixtureTarget
 cmp eax,1
 jne failed
 invoke lstrcmpA,fixtureTarget,fixtureId
 test eax,eax
 jnz failed
 stdcall LdProfilePaths,fixtureId,fixtureDirectory,fixturePath
 stdcall EReadFile,fixturePath
 test eax,eax
 jz failed
 mov ebx,eax
 invoke lstrcmpA,ebx,fixtureGoodProfile
 mov esi,eax
 stdcall FreeMem,ebx
 test esi,esi
 jnz failed
 inc [testNo]
 mov [fixtureMode],6
 stdcall LoaderInstall,fixtureBase,1,fixtureVersion,fixtureTarget
 cmp eax,1
 jne failed
 invoke lstrcpyW,fixtureBackup,fixtureDirectory
 invoke lstrcatW,fixtureBackup,ldBackup
 stdcall EReadFile,fixtureBackup
 test eax,eax
 jz failed
 mov ebx,eax
 invoke lstrcmpA,ebx,fixtureGoodProfile
 mov esi,eax
 stdcall FreeMem,ebx
 test esi,esi
 jnz failed
 inc [testNo]
 mov [fixtureMode],5
 stdcall LoaderInstall,fixtureBase,1,fixtureVersion,fixtureTarget
 cmp eax,1
 jne failed
 stdcall EReadFile,fixtureBackup
 test eax,eax
 jz failed
 mov ebx,eax
 invoke lstrcmpA,ebx,fixtureGoodProfile
 mov esi,eax
 stdcall FreeMem,ebx
 test esi,esi
 jnz failed
 inc [testNo]
 mov [fixtureMode],7
 stdcall LoaderInstall,fixtureBase,1,fixtureVersion,fixtureTarget
 test eax,eax
 jnz failed
 cmp byte [fixtureTarget],0
 jne failed
 inc [testNo]
 mov [fixtureMode],8
 stdcall LoaderInstall,fixtureBase,1,fixtureVersion,fixtureTarget
 test eax,eax
 jnz failed
 inc [testNo]
 mov [fixtureMode],9
 stdcall LoaderInstall,fixtureBase,1,fixtureVersion,fixtureTarget
 test eax,eax
 jnz failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testNo]

liveInstall:
 cmp eax,1024
 jae .failed
 mov [liveMode],1
 invoke GetEnvironmentVariableW,liveBaseEnv,fixtureWide,128
 test eax,eax
 jz .failed
 cmp eax,128
 jae .failed
 invoke WideCharToMultiByte,65001,0,fixtureWide,-1,liveBase,128,0,0
 test eax,eax
 jz .failed
 invoke GetEnvironmentVariableW,liveVersionEnv,fixtureWide,128
 test eax,eax
 jz .failed
 cmp eax,128
 jae .failed
 invoke WideCharToMultiByte,65001,0,fixtureWide,-1,liveVersion,128,0,0
 test eax,eax
 jz .failed
 invoke GetEnvironmentVariableW,liveKindEnv,fixtureWide,8
 cmp eax,1
 jne .failed
 movzx eax,word [fixtureWide]
 sub eax,'0'
 cmp eax,1
 jb .failed
 cmp eax,4
 ja .failed
 mov [liveKind],eax
 invoke GetEnvironmentVariableW,liveJavaEnv,javaPath,1024
 test eax,eax
 jz .failed
 cmp eax,1024
 jae .failed
 stdcall LoaderInstall,liveBase,[liveKind],liveVersion,fixtureTarget
 cmp eax,1
 jne .failed
 invoke lstrcpyW,fixturePath,gameDir
 invoke lstrcatW,fixturePath,liveResult
 stdcall LdWriteFile,fixturePath,fixtureTarget
 test eax,eax
 jz .failed
 stdcall SetStatusA,fixtureTarget
 invoke ExitProcess,0
.failed:
 invoke ExitProcess,1

proc SetStatusA uses ebx,text
 local written:DWORD
 cmp [liveMode],0
 je .done
 invoke lstrlenA,[text]
 mov ebx,eax
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,[text],ebx,addr written,0
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,liveNewline,2,addr written,0
.done:
 ret
endp
proc FixtureRequest uses ebx esi,url,method,headers,body
 cmp [liveMode],0
 je .fixture
 stdcall HttpRequest,[url],[method],[headers],[body]
 ret
.fixture:
 inc [requestCalls]
 mov [httpStatus],200
 mov eax,[fixtureMode]
 mov esi,fixtureFabricJson
 cmp eax,0
 je .copy
 mov esi,fixtureQuiltJson
 cmp eax,1
 je .copy
 mov esi,fixtureForgeJson
 cmp eax,2
 je .copy
 mov esi,fixtureNeoJson
 cmp eax,3
 je .copy
 mov esi,fixtureBadJson
 cmp eax,4
 je .copy
 mov esi,fixtureGoodProfile
 cmp eax,5
 je .copy
 mov esi,fixtureChangedProfile
 cmp eax,6
 je .copy
 mov esi,fixtureBadParent
 cmp eax,7
 je .copy
 mov esi,fixtureBadProfile
 cmp eax,8
 je .copy
 mov esi,fixtureNulProfile
.copy:
 invoke lstrlenA,esi
 inc eax
 mov ebx,eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 test eax,eax
 jz .done
 invoke lstrcpyA,eax,esi
.done:
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
define HttpRequest FixtureRequest
include '..\src\loaders.inc'
restore HttpRequest
include '..\src\imports.inc'
