; Real archive validation/tar/JSON/filesystem, mocked network/game installation.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\mod_types.inc'
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
testStep dd 1
mockGameCalls dd 0
mockGameFail dd 0
liveMode dd 0
cfFixtureMode dd 0
fixtureRoot du 'tests\out\pack-unit-%u',0
fixtureLiveArchive du 'OFFSET_PACK_ARCHIVE',0
fixtureLiveDestination du 'OFFSET_PACK_DEST',0
fixtureLiveJava du 'OFFSET_PACK_JAVA',0
fixtureArchive du 'tests\fixtures\packs\valid.mrpack',0
fixtureTraversal du 'tests\fixtures\packs\traversal.mrpack',0
fixtureAlias du 'tests\fixtures\packs\alias.mrpack',0
fixtureDevice du 'tests\fixtures\packs\device.mrpack',0
fixtureLink du 'tests\fixtures\packs\symlink.mrpack',0
fixtureMismatch du 'tests\fixtures\packs\local-mismatch.mrpack',0
fixtureBadManifest du 'tests\fixtures\packs\bad-manifest.mrpack',0
fixtureBadHash du 'tests\fixtures\packs\bad-hash.mrpack',0
fixtureCf du 'tests\fixtures\packs\curseforge.zip',0
fixtureFallback du 'tests\fixtures\packs\fallback.mrpack',0
fixtureUnicodeExtra du 'tests\fixtures\packs\unicode-extra.mrpack',0
fixtureTimestampExtra du 'tests\fixtures\packs\timestamp-extra.mrpack',0
fixtureDescriptor du 'tests\fixtures\packs\descriptor.mrpack',0
fixtureDescriptorBad du 'tests\fixtures\packs\descriptor-mismatch.mrpack',0
fixturePayload du 'tests\fixtures\packs\payload.txt',0
fixtureSuffix du '\instance',0
fixtureBadSuffix du '\bad-hash',0
fixtureManifestSuffix du '\bad-manifest',0
fixtureResumeSuffix du '\resume',0
fixtureFallbackSuffix du '\fallback',0
fixtureMarker du '\.offset-pack.json',0
fixtureOverride du '\config\override.txt',0
fixtureDownloaded du '\config\native.txt',0
fixtureServer du '\mods\server.jar',0
fixtureChild du '\sub',0
fixtureSha db '180b11ce8513690ccad51ccffb777bf80dbfdae5',0
fixtureClient db 'client',10,0
fixturePath db 'config/test.cfg',0
fixtureUnicode db 'config/',0E4h,0B8h,0ADh,0E6h,096h,087h,'.txt',0
fixtureDot db '../escape',0
fixtureAbsolute db 'C:/outside',0
fixtureDeviceA db 'config/con.txt',0
fixtureDeviceAlias db 'config/COM1.ini',0
fixtureDotAlias db 'config/test. /file',0
fixtureEnginePath db 'versions/1.21.1/1.21.1.json',0
fixtureOffsetPath db '.offset-pack.json',0
fixtureNulJson db '{"formatVersion":1,"game":"minecraft","name":"Alias","dependencies":{"minecraft":"1.21.1"},"files":[{"path":"config/a\u0000.txt"}]}',0
fixtureCfBlocked db '{"data":{"id":456,"modId":123,"isAvailable":true,"fileName":"test.jar","downloadUrl":null,"hashes":[]}}',0
fixtureCfGood db '{"data":{"id":456,"modId":123,"isAvailable":true,"fileName":"test.jar","downloadUrl":"https://mediafilez.forgecdn.net/files/0/456/test.jar","hashes":[{"algo":1,"value":"180b11ce8513690ccad51ccffb777bf80dbfdae5"}]}}',0
fixtureCfProjectGood db '{"data":{"id":123,"gameId":432,"classId":6,"isAvailable":true,"allowModDistribution":null}}',0
fixtureCfProjectDenied db '{"data":{"id":123,"gameId":432,"classId":6,"isAvailable":true,"allowModDistribution":false}}',0
fixtureCfProjectResource db '{"data":{"id":123,"gameId":432,"classId":12,"isAvailable":true,"allowModDistribution":true}}',0
fixtureCfZip db '{"data":{"id":456,"modId":123,"isAvailable":true,"fileName":"pack.zip","downloadUrl":"https://mediafilez.forgecdn.net/files/0/456/pack.zip","hashes":[{"algo":1,"value":"180b11ce8513690ccad51ccffb777bf80dbfdae5"}]}}',0
fixtureCfResourcePath db 'resourcepacks/pack.zip',0
fixtureCfProjectUrl du 'https://api.curseforge.com/v1/mods/123',0
fixtureApiKey db 'fixture-key-not-network',0
fixtureFailureUrl db 'https://cdn.modrinth.com/fixture-failure',0
fixtureUnknownDeps db '{"dependencies":{"minecraft":"1.21.1","mystery":"1.0"}}',0
fixtureCfManifest db '{"minecraft":{"version":"1.21.1","modLoaders":[{"id":"forge-52.1.20"}]},"manifestType":"minecraftModpack","manifestVersion":1,"name":"CF Fixture","overrides":"overrides","files":[{"projectID":123,"fileID":456,"required":true}]}',0
fixtureNewline db 13,10,0
fixtureSavedGame rw 1024
fixtureSavedVersion rb 128
fixtureDestination rw 1024
fixtureBadDestination rw 1024
fixtureBuffer rw 2048
fixtureArchivePath rw 1024

section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,fixtureLiveArchive,fixtureArchivePath,1024
 test eax,eax
 jz .unit
 mov [liveMode],1
 invoke GetEnvironmentVariableW,fixtureLiveDestination,fixtureDestination,1024
 test eax,eax
 jz failed
 invoke GetEnvironmentVariableW,fixtureLiveJava,javaPath,1024
 stdcall PackInstall,fixtureArchivePath,fixtureDestination
 xor eax,1
 invoke ExitProcess,eax
.unit:
 stdcall PkSafePath,fixturePath,0
 cmp eax,1
 jne failed
 stdcall PkSafePath,fixtureUnicode,0
 cmp eax,1
 jne failed
 inc [testStep]
 stdcall PkSafePath,fixtureDot,0
 test eax,eax
 jnz failed
 stdcall PkSafePath,fixtureAbsolute,0
 test eax,eax
 jnz failed
 stdcall PkSafePath,fixtureDeviceA,0
 test eax,eax
 jnz failed
 stdcall PkSafePath,fixtureDeviceAlias,0
 test eax,eax
 jnz failed
 stdcall PkSafePath,fixtureDotAlias,0
 test eax,eax
 jnz failed
 stdcall PkPayloadSafe,fixtureEnginePath
 test eax,eax
 jnz failed
 stdcall PkPayloadSafe,fixtureOffsetPath
 test eax,eax
 jnz failed
 inc [testStep]
 stdcall PkReadZip,fixtureArchive
 cmp eax,1
 jne failed
 cmp [pkZipCount],3
 jne failed
 inc [testStep]
 stdcall PkReadZip,fixtureTraversal
 test eax,eax
 jnz failed
 stdcall PkReadZip,fixtureAlias
 test eax,eax
 jnz failed
 stdcall PkReadZip,fixtureDevice
 test eax,eax
 jnz failed
 stdcall PkReadZip,fixtureLink
 test eax,eax
 jnz failed
 stdcall PkReadZip,fixtureMismatch
 test eax,eax
 jnz failed
 stdcall PkReadZip,fixtureUnicodeExtra
 test eax,eax
 jnz failed
 stdcall PkReadZip,fixtureTimestampExtra
 cmp eax,1
 jne failed
 stdcall PkReadZip,fixtureDescriptor
 cmp eax,1
 jne failed
 stdcall PkReadZip,fixtureDescriptorBad
 test eax,eax
 jnz failed
 inc [testStep]
 invoke GetTickCount
 cinvoke wsprintfW,fixtureBuffer,fixtureRoot,eax
 stdcall EnsureDirectory,fixtureBuffer
 invoke lstrcpyW,fixtureDestination,fixtureBuffer
 invoke lstrcatW,fixtureDestination,fixtureSuffix
 invoke lstrcpyW,fixtureBadDestination,fixtureBuffer
 invoke lstrcatW,fixtureBadDestination,fixtureBadSuffix
 invoke lstrcpyW,fixtureSavedGame,gameDir
 invoke lstrcpyA,fixtureSavedVersion,selectedVersion
 stdcall PackInstall,fixtureArchive,fixtureDestination
 cmp eax,1
 jne failed
 cmp [mockGameCalls],1
 jne failed
 invoke lstrcmpiW,gameDir,fixtureSavedGame
 test eax,eax
 jnz failed
 invoke lstrcmpA,selectedVersion,fixtureSavedVersion
 test eax,eax
 jnz failed
 inc [testStep]
 invoke lstrcpyW,fixtureBuffer,fixtureDestination
 invoke lstrcatW,fixtureBuffer,fixtureMarker
 stdcall EReadFile,fixtureBuffer
 test eax,eax
 jz failed
 mov ebx,eax
 stdcall JsonParse,eax
 test eax,eax
 jz failed
 mov esi,eax
 stdcall JsonRoot,eax
 stdcall JsonGet,esi,eax,pkKeyName
 stdcall PkString,esi,eax,pkValue,4096
 test eax,eax
 jz failed
 stdcall JsonFree,esi
 stdcall FreeMem,ebx
 inc [testStep]
 invoke lstrcpyW,fixtureBuffer,fixtureDestination
 invoke lstrcatW,fixtureBuffer,fixtureDownloaded
 stdcall VerifySha1,fixtureBuffer,fixtureSha
 cmp eax,1
 jne failed
 invoke lstrcpyW,fixtureBuffer,fixtureDestination
 invoke lstrcatW,fixtureBuffer,fixtureServer
 invoke GetFileAttributesW,fixtureBuffer
 cmp eax,-1
 jne failed
 inc [testStep]
 invoke lstrcpyW,fixtureBuffer,fixtureDestination
 invoke lstrcatW,fixtureBuffer,fixtureOverride
 stdcall EReadFile,fixtureBuffer
 test eax,eax
 jz failed
 mov ebx,eax
 invoke lstrcmpA,eax,fixtureClient
 mov esi,eax
 stdcall FreeMem,ebx
 test esi,esi
 jnz failed
 inc [testStep]
 stdcall PackInstall,fixtureArchive,fixtureDestination
 test eax,eax
 jnz failed
 cmp [mockGameCalls],1
 jne failed
 inc [testStep]
 stdcall PackInstall,fixtureBadHash,fixtureBadDestination
 test eax,eax
 jnz failed
 cmp [mockGameCalls],1
 jne failed
 ; Same owned stage may resume but a different archive cannot take it over.
 stdcall PackInstall,fixtureArchive,fixtureBadDestination
 test eax,eax
 jnz failed
 cmp [mockGameCalls],1
 jne failed
 inc [testStep]
 invoke lstrcpyA,curseApiKey,fixtureApiKey
 mov [pkFileCount],1
 mov dword [pkFiles+PK_FPROJECT],123
 mov dword [pkFiles+PK_FID],456
 stdcall PkResolveCf,0
 test eax,eax
 jnz failed
 mov [cfFixtureMode],1
 stdcall PkResolveCf,0
 cmp eax,1
 jne failed
 mov [cfFixtureMode],2
 stdcall PkResolveCf,0
 test eax,eax
 jnz failed
 mov [cfFixtureMode],3
 stdcall PkResolveCf,0
 cmp eax,1
 jne failed
 invoke lstrcmpA,pkFiles,fixtureCfResourcePath
 test eax,eax
 jnz failed
 inc [testStep]
 mov [engineCancel],1
 stdcall PackInstall,fixtureArchive,fixtureBadDestination
 test eax,eax
 jnz failed
 cmp [mockGameCalls],1
 jne failed
 mov [engineCancel],0
 inc [testStep]
 stdcall JsonParse,fixtureUnknownDeps
 mov [pkContext],eax
 stdcall JsonRoot,eax
 stdcall PkReadDependencies,eax
 mov ebx,eax
 stdcall PkCloseManifest
 test ebx,ebx
 jnz failed
 inc [testStep]
 stdcall JsonParse,fixtureCfManifest
 mov [pkContext],eax
 stdcall JsonRoot,eax
 stdcall PkReadCf,eax
 mov ebx,eax
 stdcall PkCloseManifest
 cmp ebx,1
 jne failed
 cmp [pkLoaderKind],2
 jne failed
 cmp [pkFileCount],1
 jne failed
 cmp dword [pkFiles+PK_FPROJECT],123
 jne failed
 inc [testStep]
 invoke lstrcpyW,fixtureBuffer,fixtureDestination
 invoke lstrcatW,fixtureBuffer,fixtureResumeSuffix
 ; A sibling path whose parent already exists, unique to this test run.
 invoke lstrcpyW,fixtureBadDestination,fixtureDestination
 invoke lstrcatW,fixtureBadDestination,fixtureResumeSuffix
 ; Parent of this target is the common per-test root, not instance data.
 mov [mockGameFail],1
 stdcall PackInstall,fixtureArchive,fixtureBadDestination
 test eax,eax
 jnz failed
 mov [mockGameFail],0
 stdcall PackInstall,fixtureArchive,fixtureBadDestination
 cmp eax,1
 jne failed
 inc [testStep]
 invoke lstrcpyW,fixtureBadDestination,fixtureDestination
 invoke lstrcatW,fixtureBadDestination,fixtureFallbackSuffix
 stdcall PackInstall,fixtureFallback,fixtureBadDestination
 cmp eax,1
 jne failed
 invoke ExitProcess,0
failed:
 cinvoke wsprintfA,pkValue,pkCfFileFmt,[testStep],[packLastError]
 invoke lstrlenA,pkValue
 mov ebx,eax
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,pkValue,ebx,fixtureBuffer,0
 invoke ExitProcess,[testStep]

proc SetStatusA uses ebx,text
 local written:DWORD
 cmp [liveMode],0
 je .done
 invoke lstrlenA,[text]
 mov ebx,eax
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,[text],ebx,addr written,0
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,fixtureNewline,2,addr written,0
.done: ret
endp
proc FixtureRequest uses ebx esi,url,method,headers,body
 cmp [liveMode],0
 jne .live
 mov [httpStatus],200
 invoke lstrcmpiW,[url],fixtureCfProjectUrl
 test eax,eax
 jnz .file
 mov esi,fixtureCfProjectGood
 cmp [cfFixtureMode],2
 jne .projectResource
 mov esi,fixtureCfProjectDenied
.projectResource:
 cmp [cfFixtureMode],3
 jne .copy
 mov esi,fixtureCfProjectResource
 jmp .copy
.file:
 mov esi,fixtureCfBlocked
 cmp [cfFixtureMode],0
 je .copy
 mov esi,fixtureCfGood
 cmp [cfFixtureMode],3
 jne .copy
 mov esi,fixtureCfZip
.copy:
 invoke lstrlenA,esi
 inc eax
 mov ebx,eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 test eax,eax
 jz .done
 invoke lstrcpyA,eax,esi
.done: ret
.live:
 stdcall HttpRequest,[url],[method],[headers],[body]
 ret
endp
proc FixtureDownload uses ebx,url,path,hash
 cmp [liveMode],0
 jne .live
 invoke lstrcmpA,[url],fixtureFailureUrl
 test eax,eax
 jz .fail
 stdcall EnsureDirectory,pkStage
 ; HttpDownload normally creates its parent directories.
 invoke lstrcpyW,fixtureBuffer,[path]
 invoke lstrlenW,fixtureBuffer
 lea ebx,[fixtureBuffer+eax*2]
.parent:
 sub ebx,2
 cmp word [ebx],'/'
 je .split
 cmp word [ebx],92
 jne .parent
.split:
 mov word [ebx],0
 stdcall EnsureDirectory,fixtureBuffer
 invoke CopyFileW,fixturePayload,[path],0
 test eax,eax
 jz .done
 stdcall VerifySha1,[path],[hash]
.done: ret
.fail:
 xor eax,eax
 ret
.live:
 stdcall HttpDownload,[url],[path],[hash]
 ret
endp
proc FixtureGame
 cmp [liveMode],0
 jne .live
 inc [mockGameCalls]
 cmp [mockGameFail],0
 jne .failed
 mov eax,1
 ret
.failed:
 xor eax,eax
 ret
.live:
 stdcall EInstallCore
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\loaders.inc'
include '..\src\profiles.inc'
include '..\src\mods.inc'
define HttpRequest FixtureRequest
include '..\src\curseforge.inc'
restore HttpRequest
define HttpDownload FixtureDownload
define EInstallCore FixtureGame
include '..\src\packs.inc'
restore EInstallCore
restore HttpDownload
include '..\src\imports.inc'
