; Real pack installation: no mocked transport, installer or profile modules.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\mod_types.inc'
include '..\src\version_types.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
memoryMb dd 2048
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
testStep dd 1
envRoot du 'OFFSET_PACK_LIVE_ROOT',0
envArchive du 'OFFSET_PACK_LIVE_ARCHIVE',0
envDest du 'OFFSET_PACK_LIVE_DEST',0
envJava du 'OFFSET_PACK_LIVE_JAVA',0
envModSha du 'OFFSET_PACK_LIVE_MOD_SHA',0
envConfigSha du 'OFFSET_PACK_LIVE_CONFIG_SHA',0
expectedVersion db 'fabric-loader-0.19.3-1.21.1',0
modSuffix du '\mods\fabric-api-0.116.12+1.21.1.jar',0
configSuffix du '\config\offset-import-live.txt',0
markerSuffix du '\.offset-pack.json',0
savedRoot rw 1024
savedId rb 128
archive rw 1024
destination rw 1024
path rw 2048
sha rb 41
modSha rb 41
configSha rb 41
wideSha rw 41
newline db 13,10,0
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,envRoot,gameDir,1024
 test eax,eax
 jz failed
 invoke GetEnvironmentVariableW,envArchive,archive,1024
 test eax,eax
 jz failed
 invoke GetEnvironmentVariableW,envDest,destination,1024
 test eax,eax
 jz failed
 invoke GetEnvironmentVariableW,envJava,javaPath,1024
 test eax,eax
 jz failed
 invoke GetEnvironmentVariableW,envModSha,wideSha,41
 cmp eax,40
 jne failed
 invoke WideCharToMultiByte,65001,0,wideSha,-1,modSha,41,0,0
 invoke GetEnvironmentVariableW,envConfigSha,wideSha,41
 cmp eax,40
 jne failed
 invoke WideCharToMultiByte,65001,0,wideSha,-1,configSha,41,0,0
 invoke lstrcpyW,savedRoot,gameDir
 invoke lstrcpyA,savedId,selectedVersion
 inc [testStep]
 stdcall PackInspect,archive
 cmp eax,1
 jne failed
 invoke lstrcpyA,sha,packInspectHash
 cmp [packInspectLoaderKind],1
 jne failed
 invoke lstrcmpA,packInspectBase,savedId
 test eax,eax
 jnz failed
 inc [testStep]
 stdcall PackInstallChecked,archive,destination,sha
 cmp eax,1
 jne failed
 invoke lstrcmpiW,gameDir,savedRoot
 test eax,eax
 jnz failed
 invoke lstrcmpA,selectedVersion,savedId
 test eax,eax
 jnz failed
 invoke lstrcmpA,packInstalledVersion,expectedVersion
 test eax,eax
 jnz failed
 inc [testStep]
 invoke lstrcpyW,path,destination
 invoke lstrcatW,path,modSuffix
 stdcall VerifySha1,path,modSha
 cmp eax,1
 jne failed
 invoke lstrcpyW,path,destination
 invoke lstrcatW,path,configSuffix
 stdcall VerifySha1,path,configSha
 cmp eax,1
 jne failed
 invoke lstrcpyW,path,destination
 invoke lstrcatW,path,markerSuffix
 invoke GetFileAttributesW,path
 cmp eax,-1
 je failed
 inc [testStep]
 ; Check the installed child via actual engine metadata/classpath handling.
 invoke lstrcpyW,gameDir,destination
 invoke lstrcpyA,selectedVersion,packInstalledVersion
 stdcall EValidateVersion
 test eax,eax
 jz failed
 stdcall EBuildPaths
 stdcall ELoadMetadata
 test eax,eax
 jz failed
 stdcall EBuildClasspath
 test eax,eax
 jz failed
 stdcall ECleanup
 invoke lstrcpyW,gameDir,savedRoot
 invoke lstrcpyA,selectedVersion,savedId
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testStep]
proc SetStatusA uses ebx,text
 local written:DWORD
 invoke lstrlenA,[text]
 mov ebx,eax
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,[text],ebx,addr written,0
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,newline,2,addr written,0
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\loaders.inc'
include '..\src\profiles.inc'
include '..\src\mods.inc'
include '..\src\curseforge.inc'
include '..\src\packs.inc'
include '..\src\imports.inc'
