; Real filesystem/JSON/hash/merge transactions; only installer/download are mocked.
format PE console 6.0
entry start
stack 200000h,100000h
include 'win32ax.inc'
include '..\src\version_types.inc'
WM_APP=8000h
define OFFSET_LOADER_CHANGE 1
section '.data' data readable writeable
hWnd dd 0
memoryMb dd 2048
onlineMode dd 0
authReady dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Player',0,57 dup(0)
selectedVersion db 'home-version',0,115 dup(0)
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
step dd 1
fixtureFail dd 0
fixtureLock dd 0
fixtureContext dd 0
fixtureRoot rw 1024
fixtureRootFormat du 'tests\out\loader-change-%08X',0
fixtureSentinelRoot du 'C:\untouched-home-root',0
fixtureSentinelJava du 'C:\untouched-java.exe',0
fixtureSource db 'instance',0
fixtureBase db '1.21.1',0
fixtureOld db '0.16.9',0
fixtureNew db '0.16.10',0
fixtureOlder db '0.16.8',0
fixtureConflict db '0.16.11',0
fixtureForge db '52.0.48',0
fixtureNeo db '21.1.81',0
fixtureNeoNext db '21.1.82',0
fixtureNeoForm db '20240808.144430',0
fixtureQuilt db '0.27.1',0
fixtureBad db '../invalid',0
fixtureOtherId db 'fabric-loader-0.16.10-1.21.1',0
fixtureJar db 'abc',0
fixtureCorrupt db 'different file',0
fixtureHash db 'a9993e364706816aba3e25717850c26c9cd0d89d',0
fixtureHistoryHash rb 64
fixtureKeepHash rb 64
fixtureForeignHash rb 64
fixtureSettingsHash rb 64
fixtureWorldHash rb 64
fixtureJavaW rw 1024
fixtureObservedJavaW rw 1024
fixtureHexW rw 4112
fixtureBaseJson db '{"id":"1.21.1","type":"release","mainClass":"base.Main","downloads":{"client":{"url":"https://example.invalid/base.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}},"assetIndex":{"id":"test"},"arguments":{"jvm":["--base-jvm"],"game":["--base"]},"libraries":[]}',0
fixtureOldJson db '{"id":"instance","offsetNormalized":true,"offsetBase":"1.21.1","mainClass":"old.Main","downloads":{"client":{"url":"https://example.invalid/base.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}},"arguments":{"jvm":["--base-jvm","--old-jvm"],"game":["--base","--old"]},"libraries":[{"name":"net.fabricmc:fabric-loader:0.16.9","downloads":{"artifact":{"path":"net/fabricmc/fabric-loader/0.16.9/fabric-loader-0.16.9.jar","url":"https://example.invalid/old.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}}}]}',0
fixtureMixedJson db '{"id":"instance","offsetNormalized":true,"offsetBase":"1.21.1","mainClass":"old.Main","downloads":{"client":{"sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}},"arguments":{"jvm":[],"game":[]},"libraries":[{"name":"net.fabricmc:fabric-loader:0.16.9"},{"name":"net.minecraftforge:forge:1.21.1-52.0.1:client"}]}',0
fixtureForgeJson db '{"mainClass":"cpw.mods.bootstraplauncher.BootstrapLauncher","libraries":[{"name":"net.minecraftforge:forge:1.21.1-52.0.48:client"},{"name":"net.minecraftforge:forge:1.21.1-52.0.48:universal"}],"arguments":{"game":[]}}',0
fixtureQuiltJson db '{"mainClass":"org.quiltmc.loader.impl.launch.knot.KnotClient","libraries":[{"name":"org.quiltmc:quilt-loader:0.27.1"}],"arguments":{"game":[]}}',0
fixtureNeoJson db '{"mainClass":"cpw.mods.bootstraplauncher.BootstrapLauncher","libraries":[{"name":"net.neoforged.fancymodloader:loader:4.0.45"}],"arguments":{"game":["--fml.neoForgeVersion","21.1.81","--fml.mcVersion","1.21.1"]}}',0
fixtureNeoWrongMc db '{"mainClass":"cpw.mods.bootstraplauncher.BootstrapLauncher","libraries":[],"arguments":{"game":["--fml.neoForgeVersion","21.1.81","--fml.mcVersion","1.20.1"]}}',0
fixtureNeoDuplicate db '{"mainClass":"cpw.mods.bootstraplauncher.BootstrapLauncher","libraries":[],"arguments":{"game":["--fml.neoForgeVersion","21.1.81","--fml.neoForgeVersion","21.1.81","--fml.mcVersion","1.21.1"]}}',0
fixtureNeoWrongMain db '{"mainClass":"custom.Main","libraries":[],"arguments":{"game":["--fml.neoForgeVersion","21.1.81","--fml.mcVersion","1.21.1"]}}',0
fixtureNeoSource db '{"id":"instance","offsetNormalized":true,"offsetBase":"1.21.1","mainClass":"cpw.mods.bootstraplauncher.BootstrapLauncher","downloads":{"client":{"url":"https://example.invalid/base.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}},"arguments":{"jvm":[],"game":["--fml.neoForgeVersion","21.1.80","--fml.mcVersion","1.21.1","--fml.neoFormVersion","20240808.144430"]},"libraries":[]}',0
fixtureNeoChild db '{"id":"neoforge-%s","inheritsFrom":"1.21.1","mainClass":"cpw.mods.bootstraplauncher.BootstrapLauncher","arguments":{"jvm":[],"game":["--fml.neoForgeVersion","%s","--fml.mcVersion","1.21.1","--fml.neoFormVersion","20240808.144430"]},"libraries":[]}',0
fixtureNeoTarget db 'neoforge-%s',0
fixtureChildFormat db '{"id":"fabric-loader-%s-1.21.1","inheritsFrom":"1.21.1","mainClass":"new.Main","arguments":{"jvm":["--new-jvm"],"game":["--new"]},"libraries":[{"name":"net.fabricmc:fabric-loader:%s","downloads":{"artifact":{"path":"net/fabricmc/fabric-loader/%s/fabric-loader-%s.jar","url":"https://example.invalid/new.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}}}]}',0
fixtureTargetFormat db 'fabric-loader-%s-1.21.1',0
fixtureChild rb 4096
fixtureTarget rb 128
fixturePath rw 2048
fixtureDir rw 2048
fixtureSourcePath rw 2048
fixtureOtherPath rw 2048
fixtureBeforePath rw 2048
fixtureSettingsPath rw 2048
fixtureWorldPath rw 2048
fixtureNeoClientPath rw 2048
fixtureRuntimeRel rb 1024
fixtureOldLib du '\libraries\net\fabricmc\fabric-loader\0.16.9',0
fixtureOldLibJar du '\fabric-loader-0.16.9.jar',0
fixtureConflictDir du '\libraries\net\fabricmc\fabric-loader\0.16.11',0
fixtureConflictJar du '\fabric-loader-0.16.11.jar',0
fixtureSourceDir du '\versions\instance',0
fixtureBaseDir du '\versions\1.21.1',0
fixtureOtherDir du '\versions\fabric-loader-0.16.10-1.21.1',0
fixtureSettings du '\.offset-profile.ini',0
fixtureSettingsText db '[Profile]',13,10,'Isolate=1',13,10,'MemoryMiB=4096',13,10,0
fixtureData du '\game\saves',0
fixtureMarker du '\world.dat',0
fixtureArgBase db '--base',0
fixtureArgNew db '--new',0

section '.text' code readable executable
start:
 stdcall EngineInit
 stdcall AssertMetadata,fixtureForgeJson,2,fixtureForge
 test eax,eax
 jz failed
 inc [step]
 stdcall AssertMetadata,fixtureQuiltJson,4,fixtureQuilt
 test eax,eax
 jz failed
 inc [step]
 stdcall AssertMetadata,fixtureNeoJson,3,fixtureNeo
 test eax,eax
 jz failed
 inc [step]
 stdcall AssertMetadata,fixtureNeoWrongMc,0,0
 test eax,eax
 jz failed
 inc [step]
 stdcall AssertMetadata,fixtureNeoDuplicate,0,0
 test eax,eax
 jz failed
 inc [step]
 stdcall AssertMetadata,fixtureNeoWrongMain,0,0
 test eax,eax
 jz failed
 inc [step]
 invoke GetTickCount
 cinvoke wsprintfW,fixturePath,fixtureRootFormat,eax
 invoke GetFullPathNameW,fixturePath,1024,fixtureRoot,0
 invoke lstrcpyW,gameDir,fixtureRoot
 invoke lstrcpyW,fixtureDir,fixtureRoot
 invoke lstrcatW,fixtureDir,fixtureBaseDir
 stdcall EnsureDirectory,fixtureDir
 stdcall LcPath,fixtureDir,fixtureBase,pJsonW,fixturePath
 stdcall LdWriteFile,fixturePath,fixtureBaseJson
 stdcall LcPath,fixtureDir,fixtureBase,pJarW,fixturePath
 stdcall LdWriteFile,fixturePath,fixtureJar
 invoke lstrcpyW,fixtureDir,fixtureRoot
 invoke lstrcatW,fixtureDir,fixtureSourceDir
 stdcall EnsureDirectory,fixtureDir
 stdcall LcPath,fixtureDir,fixtureSource,pJsonW,fixtureSourcePath
 stdcall LdWriteFile,fixtureSourcePath,fixtureOldJson
 stdcall LcPath,fixtureDir,fixtureSource,pJarW,fixturePath
 stdcall LdWriteFile,fixturePath,fixtureJar
 invoke lstrcpyW,fixturePath,fixtureDir
 invoke lstrcatW,fixturePath,eNativesSuffix
 stdcall EnsureDirectory,fixturePath
 invoke lstrcpyW,fixturePath,fixtureDir
 invoke lstrcatW,fixturePath,fixtureSettings
 stdcall LdWriteFile,fixturePath,fixtureSettingsText
 invoke GetModuleFileNameW,0,fixtureJavaW,1024
 stdcall VsEncode,fixtureJavaW,fixtureHexW
 invoke WritePrivateProfileStringW,vsSection,vsKeyJava,fixtureHexW,fixturePath
 invoke lstrcpyW,fixtureSettingsPath,fixturePath
 stdcall PHashFile,fixtureSettingsPath,fixtureSettingsHash
 invoke lstrcatW,fixtureDir,fixtureData
 stdcall EnsureDirectory,fixtureDir
 invoke lstrcatW,fixtureDir,fixtureMarker
 stdcall LdWriteFile,fixtureDir,fixtureCorrupt
 invoke lstrcpyW,fixtureWorldPath,fixtureDir
 stdcall PHashFile,fixtureWorldPath,fixtureWorldHash
 invoke lstrcpyW,fixturePath,fixtureRoot
 invoke lstrcatW,fixturePath,fixtureOldLib
 stdcall EnsureDirectory,fixturePath
 invoke lstrcatW,fixturePath,fixtureOldLibJar
 stdcall LdWriteFile,fixturePath,fixtureJar
 ; A canonical target already exists and must remain byte-for-byte unchanged.
 invoke lstrcpyW,fixturePath,fixtureRoot
 invoke lstrcatW,fixturePath,fixtureOtherDir
 stdcall EnsureDirectory,fixturePath
 stdcall LcPath,fixturePath,fixtureOtherId,pJsonW,fixtureOtherPath
 stdcall LdWriteFile,fixtureOtherPath,fixtureCorrupt
 stdcall PHashFile,fixtureOtherPath,fixtureKeepHash
 invoke lstrcpyW,gameDir,fixtureSentinelRoot
 invoke lstrcpyW,javaPath,fixtureSentinelJava

 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 cmp [lcKind],1
 jne failed
 invoke lstrcmpA,lcVersionA,fixtureOld
 test eax,eax
 jnz failed
 cmp [lcBackupReady],0
 jne failed
 inc [step]
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNew
 cmp eax,1
 jne failed
 stdcall CheckGlobalState
 test eax,eax
 jz failed
 invoke lstrcmpiW,fixtureObservedJavaW,fixtureJavaW
 test eax,eax
 jnz failed
 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 invoke lstrcmpA,lcVersionA,fixtureNew
 test eax,eax
 jnz failed
 cmp [lcBackupReady],1
 jne failed
 cmp dword [lcRecord+VS_MEMORY],4096
 jne failed
 cmp dword [lcRecord+VS_ISOLATE],1
 jne failed
 inc [step]
 ; The normalized arguments contain the base exactly once, no prior loader.
 stdcall EReadFile,fixtureSourcePath
 mov ebx,eax
 stdcall JsonParse,eax
 mov esi,eax
 stdcall JsonRoot,eax
 stdcall JsonGet,esi,eax,pArgsKey
 stdcall JsonGet,esi,eax,pGameKey
 mov edi,eax
 stdcall JsonCount,esi,eax
 cmp eax,2
 jne failed
 stdcall JsonAt,esi,edi,0
 stdcall VsString,esi,eax,fixtureTarget,128
 invoke lstrcmpA,fixtureTarget,fixtureArgBase
 test eax,eax
 jnz failed
 stdcall JsonAt,esi,edi,1
 stdcall VsString,esi,eax,fixtureTarget,128
 invoke lstrcmpA,fixtureTarget,fixtureArgNew
 test eax,eax
 jnz failed
 stdcall JsonFree,esi
 stdcall FreeMem,ebx
 inc [step]
 stdcall LoaderChangeRestore,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 invoke lstrcmpA,lcVersionA,fixtureOld
 test eax,eax
 jnz failed
 inc [step]
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureOlder
 cmp eax,1
 jne failed
 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 invoke lstrcmpA,lcVersionA,fixtureOlder
 test eax,eax
 jnz failed
 inc [step]
 stdcall PHashFile,fixtureSourcePath,fixtureHistoryHash
 mov [engineCancel],1
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNew
 test eax,eax
 jnz failed
 mov [engineCancel],0
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 mov [fixtureFail],1
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNew
 test eax,eax
 jnz failed
 mov [fixtureFail],0
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 stdcall CheckGlobalState
 test eax,eax
 jz failed
 inc [step]
 ; An official installer may be cancelled after creating its staged profile.
 mov [fixtureFail],2
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNew
 test eax,eax
 jnz failed
 mov [fixtureFail],0
 mov [engineCancel],0
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 ; A source opened without FILE_SHARE_DELETE prevents the atomic publication.
 invoke CreateFileW,fixtureSourcePath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je failed
 mov [fixtureLock],eax
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNew
 mov ebx,eax
 invoke CloseHandle,[fixtureLock]
 test ebx,ebx
 jnz failed
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 ; A foreign pending collision is never truncated or removed by cleanup.
 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 stdcall PHashFile,fixtureSourcePath,lcOldHashA
 stdcall LcTag
 invoke MultiByteToWideChar,65001,8,lcTagA,-1,fixtureHexW,128
 invoke lstrcpyW,fixtureBeforePath,lcProfileW
 invoke lstrcatW,fixtureBeforePath,lcPendingPrefixW
 invoke lstrcatW,fixtureBeforePath,fixtureHexW
 invoke lstrcatW,fixtureBeforePath,pJsonW
 stdcall LdWriteFile,fixtureBeforePath,fixtureCorrupt
 stdcall PHashFile,fixtureBeforePath,fixtureForeignHash
 stdcall LcPublish,fixtureOldJson
 test eax,eax
 jnz failed
 stdcall VerifySha1,fixtureBeforePath,fixtureForeignHash
 cmp eax,1
 jne failed
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 ; Never replace an existing shared library whose bytes differ.
 invoke lstrcpyW,fixturePath,fixtureRoot
 invoke lstrcatW,fixturePath,fixtureConflictDir
 stdcall EnsureDirectory,fixturePath
 invoke lstrcatW,fixturePath,fixtureConflictJar
 stdcall LdWriteFile,fixturePath,fixtureCorrupt
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureConflict
 test eax,eax
 jnz failed
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureBad
 test eax,eax
 jnz failed
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 ; Metadata-only legacy/mixed loader profiles are rejected without guessing.
 stdcall LdWriteFile,fixtureSourcePath,fixtureMixedJson
 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 test eax,eax
 jnz failed
 stdcall LdWriteFile,fixtureSourcePath,fixtureOldJson
 inc [step]
 ; The unrelated canonical installation remains untouched throughout.
 stdcall VerifySha1,fixtureOtherPath,fixtureKeepHash
 cmp eax,1
 jne failed
 stdcall VerifySha1,fixtureSettingsPath,fixtureSettingsHash
 cmp eax,1
 jne failed
 stdcall VerifySha1,fixtureWorldPath,fixtureWorldHash
 cmp eax,1
 jne failed
 stdcall CheckGlobalState
 test eax,eax
 jz failed
 inc [step]
 ; NeoForge's generated runtime files are separate from its bootstrap libs.
 stdcall LdWriteFile,fixtureSourcePath,fixtureNeoSource
 stdcall PHashFile,fixtureSourcePath,fixtureHistoryHash
 mov [fixtureFail],3
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNeo
 test eax,eax
 jnz failed
 mov [fixtureFail],0
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNeo
 cmp eax,1
 jne failed
 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 cmp [lcKind],3
 jne failed
 stdcall EReadFile,fixtureSourcePath
 mov ebx,eax
 stdcall JsonParse,eax
 mov esi,eax
 stdcall LcVerifyRuntimeArtifacts,esi,fixtureRoot
 cmp eax,1
 jne failed
 stdcall JsonRoot,esi
 stdcall JsonGet,esi,eax,lcKeyRuntime
 stdcall JsonCount,esi,eax
 cmp eax,4
 jne failed
 stdcall JsonFree,esi
 stdcall FreeMem,ebx
 inc [step]
 stdcall LoaderChangeApply,fixtureRoot,fixtureSource,fixtureNeoNext
 cmp eax,1
 jne failed
 stdcall PHashFile,fixtureSourcePath,fixtureHistoryHash
 cinvoke wsprintfA,fixtureRuntimeRel,lcRuntimeCoreFormat,fixtureNeo,fixtureNeo,lcRuntimeClientA
 invoke MultiByteToWideChar,65001,8,fixtureRuntimeRel,-1,fixtureHexW,1100
 invoke lstrcpyW,fixtureNeoClientPath,fixtureRoot
 invoke lstrcatW,fixtureNeoClientPath,pLibrariesW
 invoke lstrcatW,fixtureNeoClientPath,fixtureHexW
 stdcall LdWriteFile,fixtureNeoClientPath,fixtureCorrupt
 stdcall LoaderChangeRestore,fixtureRoot,fixtureSource
 test eax,eax
 jnz failed
 stdcall VerifySha1,fixtureSourcePath,fixtureHistoryHash
 cmp eax,1
 jne failed
 inc [step]
 stdcall LdWriteFile,fixtureNeoClientPath,fixtureJar
 stdcall LoaderChangeRestore,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 stdcall LoaderChangeLoad,fixtureRoot,fixtureSource
 cmp eax,1
 jne failed
 invoke lstrcmpA,lcVersionA,fixtureNeo
 test eax,eax
 jnz failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[step]

proc AssertMetadata uses ebx esi,text,kind,version
 local actual:DWORD,found[128]:BYTE,result:DWORD
 mov [result],0
 stdcall JsonParse,[text]
 test eax,eax
 jz .bad
 mov esi,eax
 stdcall LcReadVersion,esi,fixtureBase,addr found,addr actual
 cmp [kind],0
 je .negative
 test eax,eax
 jz .free
 mov eax,[actual]
 cmp eax,[kind]
 jne .free
 invoke lstrcmpA,addr found,[version]
 test eax,eax
 jnz .free
 mov [result],1
 jmp .free
.negative:
 test eax,eax
 jnz .free
 mov [result],1
.free:
 stdcall JsonFree,esi
 mov eax,[result]
 ret
.bad:
 xor eax,eax
 ret
endp

proc CheckGlobalState
 invoke lstrcmpiW,gameDir,fixtureSentinelRoot
 test eax,eax
 jnz .bad
 invoke lstrcmpiW,javaPath,fixtureSentinelJava
 test eax,eax
 jnz .bad
 invoke lstrcmpA,selectedVersion,fixtureSelected
 test eax,eax
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc FixtureInstaller uses ebx,base,kind,version,target
 cmp [fixtureFail],1
 je .bad
 invoke lstrcpyW,fixtureObservedJavaW,javaPath
 cmp [kind],3
 je .neo
 cinvoke wsprintfA,fixtureTarget,fixtureTargetFormat,[version]
 cinvoke wsprintfA,fixtureChild,fixtureChildFormat,[version],[version],[version],[version]
 jmp .publish
.neo:
 cinvoke wsprintfA,fixtureTarget,fixtureNeoTarget,[version]
 cinvoke wsprintfA,fixtureChild,fixtureNeoChild,[version],[version]
 cinvoke wsprintfA,fixtureRuntimeRel,lcRuntimeCoreFormat,[version],[version],lcRuntimeClientA
 stdcall FixtureGenerated,fixtureRuntimeRel
 cinvoke wsprintfA,fixtureRuntimeRel,lcRuntimeCoreFormat,[version],[version],lcRuntimeUniversalA
 stdcall FixtureGenerated,fixtureRuntimeRel
 cinvoke wsprintfA,fixtureRuntimeRel,lcRuntimeMcFormat,fixtureBase,fixtureNeoForm,fixtureBase,fixtureNeoForm,lcRuntimeSrgA
 stdcall FixtureGenerated,fixtureRuntimeRel
 cmp [fixtureFail],3
 je .publish
 cinvoke wsprintfA,fixtureRuntimeRel,lcRuntimeMcFormat,fixtureBase,fixtureNeoForm,fixtureBase,fixtureNeoForm,lcRuntimeExtraA
 stdcall FixtureGenerated,fixtureRuntimeRel
.publish:
 stdcall LdProfilePaths,fixtureTarget,fixtureDir,fixturePath
 stdcall EnsureDirectory,fixtureDir
 stdcall LdWriteFile,fixturePath,fixtureChild
 invoke lstrcpyA,[target],fixtureTarget
 cmp [fixtureFail],2
 jne .good
 mov [engineCancel],1
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc FixtureGenerated uses esi,relative
 local path[2048]:WORD,wide[1100]:WORD
 invoke MultiByteToWideChar,65001,8,[relative],-1,addr wide,1100
 lea esi,[wide]
.slashes:
 mov ax,[esi]
 test ax,ax
 jz .path
 cmp ax,'/'
 jne .advance
 mov word [esi],92
.advance:
 add esi,2
 jmp .slashes
.path:
 invoke lstrcpyW,addr path,gameDir
 invoke lstrcatW,addr path,pLibrariesW
 invoke lstrcatW,addr path,addr wide
 stdcall FixtureDownload,fixtureJar,addr path,fixtureHash,1
 ret
endp
proc FixtureDownload uses ebx esi,pathUrl,pathW,hash,kind
 local path[2048]:WORD
 invoke lstrcpyW,addr path,[pathW]
 invoke lstrlenW,addr path
 lea esi,[path+eax*2]
.parent:
 cmp word [esi],92
 je .ready
 sub esi,2
 jmp .parent
.ready:
 mov word [esi],0
 stdcall EnsureDirectory,addr path
 stdcall LdWriteFile,[pathW],fixtureJar
 stdcall VerifySha1,[pathW],[hash]
 ret
endp
proc SetStatusA,text
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
define McDownload FixtureDownload
include '..\src\engine.inc'
restore McDownload
include '..\src\mc_sources.inc'
include '..\src\version_settings.inc'
define McDownload FixtureDownload
include '..\src\profiles.inc'
restore McDownload
include '..\src\loaders.inc'
define LoaderInstall FixtureInstaller
include '..\src\loader_change.inc'
restore LoaderInstall
include '..\src\imports.inc'
section '.last' data readable writeable
fixtureSelected db 'home-version',0
