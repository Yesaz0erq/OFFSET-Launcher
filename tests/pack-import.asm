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


importRoot du 'tests\out\pack-import-%u',0
importFixturePrefix du 'tests\fixtures\pack-import\',0
importMr db 'modrinth.zip',0
importCf db 'curseforge.zip',0
importWrappedMr db 'wrapped-mr.zip',0
importWrappedCf db 'wrapped-cf.zip',0
importPrism db 'prism.zip',0
importMulti db 'multimc.zip',0
importFabric db 'prism-fabric.zip',0
importForge db 'prism-forge.zip',0
importNeo db 'prism-neoforge.zip',0
importQuilt db 'prism-quilt.zip',0
importAmbiguous db 'ambiguous.zip',0
importWrappedAmbiguous db 'ambiguous-wrapped.zip',0
importOutside db 'outside-wrapper.zip',0
importHook db 'hook.zip',0
importPatches db 'patches.zip',0
importUnknown db 'unknown-component.zip',0
importDuplicate db 'duplicate-game.zip',0
importOwned db 'owned-payload.zip',0
importBoth db 'both-data.zip',0
importNul db 'embedded-nul.zip',0
importNestedManifest db 'nested-manifest.zip',0
importDependencies db 'prism-dependencies.zip',0
importRuntimeNoVersion db 'runtime-no-version.zip',0
importUnknownNoVersion db 'unknown-no-version.zip',0
importDisabled db 'disabled-loader.zip',0
importGood dd importMr,importCf,importWrappedMr,importWrappedCf,importPrism,importMulti,importFabric,importForge,importNeo,importQuilt,importNestedManifest,importDependencies
importKinds dd 1,2,1,2,3,3,3,3,3,3,1,3
importLoaders dd 0,0,0,0,0,0,1,2,3,4,0,1
importBad dd importAmbiguous,importWrappedAmbiguous,importOutside,importHook,importPatches,importUnknown,importDuplicate,importOwned,importBoth,importNul,importRuntimeNoVersion,importUnknownNoVersion,importDisabled
importIndex dd 0
importName db 'Prism 中文',0
importHash rb 41
importWrongHash db '0000000000000000000000000000000000000000',0
importOutputRoot rw 1024
importSuffix du '\config\import.txt',0
importStageSuffix du '.offset-stage',0
importExpectedMr db 'MR',10,0
importExpectedCf db 'CF',10,0
importExpectedWrapped db 'client',10,0
importExpectedWrappedCf db 'CF wrapped',10,0
importExpectedPrism db 'Prism',10,0
importExpectedMulti db 'MultiMC',10,0
importContents dd importExpectedMr,importExpectedCf,importExpectedWrapped,importExpectedWrappedCf,importExpectedPrism,importExpectedMulti,importExpectedPrism,importExpectedPrism,importExpectedPrism,importExpectedPrism
importLoaderTarget db 'import-loader',0
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,fixtureLiveArchive,fixtureArchivePath,1024
 test eax,eax
 jz .unit
 stdcall PackInspect,fixtureArchivePath
 xor eax,1
 invoke ExitProcess,eax
.unit:
 invoke lstrcpyW,fixtureSavedGame,gameDir
 invoke lstrcpyA,fixtureSavedVersion,selectedVersion
 invoke GetTickCount
 cinvoke wsprintfW,importOutputRoot,importRoot,eax
.good:
 mov eax,[importIndex]
 cmp eax,12
 jae .badStart
 stdcall ImportPath,dword [importGood+eax*4]
 stdcall PackInspect,fixtureArchivePath
 cmp eax,1
 jne failed
 cmp [packInspectReady],1
 jne failed
 mov eax,[importIndex]
 mov ecx,[importKinds+eax*4]
 cmp ecx,[packInspectFormat]
 jne failed
 mov ecx,[importLoaders+eax*4]
 cmp ecx,[packInspectLoaderKind]
 jne failed
 invoke lstrcmpA,packInspectBase,fixtureSavedVersion
 test eax,eax
 jnz failed
 cmp [mockGameCalls],0
 jne failed
 invoke GetFileAttributesW,importOutputRoot
 cmp eax,-1
 jne failed
 stdcall ImportUnchanged
 test eax,eax
 jz failed
 inc [importIndex]
 inc [testStep]
 jmp .good
.badStart:
 mov [importIndex],0
.bad:
 mov eax,[importIndex]
 cmp eax,13
 jae .checked
 stdcall ImportPath,dword [importBad+eax*4]
 stdcall PackInspect,fixtureArchivePath
 test eax,eax
 jnz failed
 cmp [packInspectReady],0
 jne failed
 cmp [mockGameCalls],0
 jne failed
 inc [importIndex]
 inc [testStep]
 jmp .bad
.checked:
 stdcall ImportPath,importPrism
 stdcall PackInspect,fixtureArchivePath
 test eax,eax
 jz failed
 invoke lstrcmpA,packInspectName,importName
 test eax,eax
 jnz failed
 invoke lstrcpyA,importHash,packInspectHash
 invoke lstrcpyW,fixtureDestination,importOutputRoot
 stdcall PackInstallChecked,fixtureArchivePath,fixtureDestination,importWrongHash
 test eax,eax
 jnz failed
 invoke GetFileAttributesW,fixtureDestination
 cmp eax,-1
 jne failed
 invoke lstrcpyW,fixtureBuffer,fixtureDestination
 invoke lstrcatW,fixtureBuffer,importStageSuffix
 invoke GetFileAttributesW,fixtureBuffer
 cmp eax,-1
 jne failed
 stdcall PackInstallChecked,fixtureArchivePath,fixtureDestination,importHash
 cmp eax,1
 jne failed
 stdcall ImportUnchanged
 test eax,eax
 jz failed
 inc [testStep]
 mov [importIndex],0
.install:
 mov eax,[importIndex]
 cmp eax,10
 jae .cancel
 stdcall ImportPath,dword [importGood+eax*4]
 stdcall PackInspect,fixtureArchivePath
 test eax,eax
 jz failed
 invoke lstrcpyA,importHash,packInspectHash
 invoke GetTickCount
 cinvoke wsprintfW,fixtureDestination,importRoot,eax
 stdcall PackInstallChecked,fixtureArchivePath,fixtureDestination,importHash
 cmp eax,1
 jne failed
 stdcall ImportUnchanged
 test eax,eax
 jz failed
 invoke lstrcpyW,fixtureBuffer,fixtureDestination
 invoke lstrcatW,fixtureBuffer,importSuffix
 stdcall EReadFile,fixtureBuffer
 test eax,eax
 jz failed
 mov ebx,eax
 mov ecx,[importIndex]
 invoke lstrcmpA,eax,dword [importContents+ecx*4]
 push eax
 stdcall FreeMem,ebx
 pop eax
 test eax,eax
 jnz failed
 inc [importIndex]
 inc [testStep]
 jmp .install
.cancel:
 mov [engineCancel],1
 stdcall ImportPath,importMr
 stdcall PackInspect,fixtureArchivePath
 test eax,eax
 jnz failed
 cmp [packInspectReady],0
 jne failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testStep]
proc ImportPath,nameA
 invoke lstrcpyW,fixtureArchivePath,importFixturePrefix
 invoke MultiByteToWideChar,65001,8,[nameA],-1,fixtureBuffer,2048
 invoke lstrcatW,fixtureArchivePath,fixtureBuffer
 ret
endp
proc ImportUnchanged
 invoke lstrcmpiW,gameDir,fixtureSavedGame
 test eax,eax
 jnz .bad
 invoke lstrcmpA,selectedVersion,fixtureSavedVersion
 test eax,eax
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc ImportLoader,base,kind,version,target
 invoke lstrcmpA,[base],fixtureSavedVersion
 test eax,eax
 jnz .bad
 mov eax,[importIndex]
 mov eax,[importLoaders+eax*4]
 cmp eax,[kind]
 jne .bad
 invoke lstrcpyA,[target],importLoaderTarget
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc ImportNormalize,child,base
 mov eax,1
 ret
endp
proc ImportMetadata
 mov eax,1
 ret
endp
proc ImportLibraries
 mov eax,1
 ret
endp
proc SetStatusA uses ebx,text
 local written:DWORD
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
define LoaderInstall ImportLoader
define ProfileNormalize ImportNormalize
define ELoadMetadata ImportMetadata
define EInstallLibraries ImportLibraries
include '..\src\packs.inc'
restore EInstallLibraries
restore ELoadMetadata
restore ProfileNormalize
restore LoaderInstall
restore EInstallCore
restore HttpDownload
include '..\src\imports.inc'
