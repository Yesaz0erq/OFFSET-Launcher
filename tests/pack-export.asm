; Real exporter/importer, ZIP/CRC/tar/JSON/filesystem. Only game download and
; loader installation are mocked, so this is not a Minecraft-runtime claim.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\mod_types.inc'
include '..\src\version_types.inc'
WM_APP=8000h
OFFSET_LOADER_CHANGE=1
section '.data' data readable writeable
hWnd dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion db '1.21.1',0,121 dup(0)
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
testStep dd 1
exportEnv du 'OFFSET_EXPORT_TEST_ROOT',0
sourceSuffix du '\source',0
loaderSuffix du '\loader',0
forgeSuffix du '\forge',0
neoSuffix du '\neo',0
neoModernSuffix du '\neo-modern',0
quiltSuffix du '\quilt',0
isolateSuffix du '\isolated',0
reparseSuffix du '\reparse',0
oversizeSuffix du '\oversize',0
format1Suffix du '\output\roundtrip.mrpack',0
format2Suffix du '\output\roundtrip-cf.zip',0
format3Suffix du '\output\roundtrip-prism.zip',0
archiveSuffixes dd format1Suffix,format2Suffix,format3Suffix
loader1Suffix du '\output\loader.mrpack',0
loader2Suffix du '\output\loader-cf.zip',0
loader3Suffix du '\output\loader-prism.zip',0
loaderArchiveSuffixes dd loader1Suffix,loader2Suffix,loader3Suffix
loaderRoundtripFormat du '\loader-roundtrip-%u',0
roundtripFormat du '\roundtrip-%u',0
existingSuffix du '\output\existing.zip',0
insideSuffix du '\source\should-not-export.zip',0
badExtensionSuffix du '\output\wrong-extension.txt',0
cancelSuffix du '\output\cancel.zip',0
savesSuffix du '\output\with-saves.zip',0
configOnlySuffix du '\output\config-only.zip',0
lockedSuffix du '\output\locked-failure.zip',0
payloadSuffix du '\mods\a.jar',0
outputLinkSuffix du '\output-linked\reparse-destination.zip',0
emptyW dw 0
baseId db '1.21.1',0
fabricId db 'fabric-test',0
forgeId db 'forge-test',0
neoId db 'neo-test',0
quiltId db 'quilt-test',0
isolateId db 'isolate-test',0
fixtureName db 'OFFSET 中文整合包',0
fixtureVersion db '2.0.0',0
fixtureAuthor db 'Test Author',0
fixtureChild db 'imported-loader',0
newline db 13,10,0
rootW rw 1024
sourceW rw 1024
savedGame rw 1024
savedVersion rb 128
archiveW rw 2048
destinationW rw 2048
suffixW rw 128
pathW rw 2048
digest rb 41
previewHash rb 41
loopFormat dd 0
loopIndex dd 0
sourceLock dd -1

section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,exportEnv,rootW,1024
 test eax,eax
 jz failed
 invoke lstrcpyW,savedGame,gameDir
 invoke lstrcpyA,savedVersion,selectedVersion
 stdcall TestSource,sourceSuffix
 stdcall PackExportInspect,sourceW,baseId,0
 cmp eax,1
 jne failed
 cmp [peFileCount],14
 jne failed
 inc [testStep]
 mov [peIncludeMods],0
 stdcall PackExportInspect,sourceW,baseId,0
 test eax,eax
 jz failed
 cmp [peFileCount],12
 jne failed
 mov [peIncludeMods],1
 mov [peIncludeConfig],0
 stdcall PackExportInspect,sourceW,baseId,0
 test eax,eax
 jz failed
 cmp [peFileCount],6
 jne failed
 mov [peIncludeConfig],1
 mov [peIncludeResources],0
 stdcall PackExportInspect,sourceW,baseId,0
 test eax,eax
 jz failed
 cmp [peFileCount],10
 jne failed
 mov [peIncludeResources],1
 inc [testStep]
 stdcall PackExportInspect,sourceW,baseId,1
 test eax,eax
 jz failed
 cmp [peFileCount],15
 jne failed
 inc [testStep]
 invoke lstrcpyA,peNameA,fixtureName
 invoke lstrcpyA,peVersionLabelA,fixtureVersion
 invoke lstrcpyA,peAuthorA,fixtureAuthor
 mov [loopFormat],1
.roundtrip:
 mov eax,[loopFormat]
 mov [peFormat],eax
 invoke lstrcpyW,peDestW,rootW
 mov eax,[loopFormat]
 dec eax
 invoke lstrcatW,peDestW,dword [archiveSuffixes+eax*4]
 stdcall PackExportInspect,sourceW,baseId,0
 cmp eax,1
 jne failed
 stdcall PackExportBuild
 cmp eax,1
 jne failed
 stdcall TestUnchanged
 test eax,eax
 jz failed
 invoke lstrcpyW,archiveW,peDestW
 stdcall PackInspect,archiveW
 cmp eax,1
 jne failed
 mov eax,[loopFormat]
 cmp eax,[packInspectFormat]
 jne failed
 invoke lstrcmpA,packInspectName,fixtureName
 test eax,eax
 jnz failed
 invoke lstrcpyA,previewHash,packInspectHash
 cinvoke wsprintfW,suffixW,roundtripFormat,[loopFormat]
 invoke lstrcpyW,destinationW,rootW
 invoke lstrcatW,destinationW,suffixW
 stdcall PackInstallChecked,archiveW,destinationW,previewHash
 cmp eax,1
 jne failed
 stdcall TestUnchanged
 test eax,eax
 jz failed
 mov [loopIndex],0
.verify:
 mov eax,[loopIndex]
 cmp eax,[peFileCount]
 jae .nextFormat
 imul ebx,eax,PE_ENTRY
 add ebx,peRecords
 lea eax,[ebx+PE_PATH]
 stdcall Sha1File,eax,digest
 test eax,eax
 jz failed
 invoke lstrcpyW,pathW,destinationW
 invoke lstrcatW,pathW,peSlashW
 invoke MultiByteToWideChar,65001,8,ebx,-1,peComponentW,512
 invoke lstrcatW,pathW,peComponentW
 stdcall VerifySha1,pathW,digest
 cmp eax,1
 jne failed
 inc [loopIndex]
 jmp .verify
.nextFormat:
 inc [testStep]
 inc [loopFormat]
 cmp [loopFormat],3
 jbe .roundtrip
 ; Stored selection is applied to the written archive, not just the count.
 mov [peFormat],2
 stdcall PackExportInspect,sourceW,baseId,1
 cmp eax,1
 jne failed
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,savesSuffix
 stdcall PackExportBuild
 cmp eax,1
 jne failed
 mov [peIncludeMods],0
 mov [peIncludeResources],0
 stdcall PackExportInspect,sourceW,baseId,0
 test eax,eax
 jz failed
 cmp [peFileCount],8
 jne failed
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,configOnlySuffix
 stdcall PackExportBuild
 cmp eax,1
 jne failed
 mov [peIncludeMods],1
 mov [peIncludeResources],1
 inc [testStep]
 ; A source read-lock failure happens after metadata was written. The owned
 ; partial archive must be closed/deleted; source and requested output survive.
 invoke lstrcpyW,pathW,sourceW
 invoke lstrcatW,pathW,payloadSuffix
 invoke CreateFileW,pathW,GENERIC_WRITE,FILE_SHARE_READ+FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je failed
 mov [sourceLock],eax
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,lockedSuffix
 stdcall PackExportBuild
 test eax,eax
 jnz failed
 invoke CloseHandle,[sourceLock]
 mov [sourceLock],-1
 invoke GetFileAttributesW,peTempW
 cmp eax,-1
 jne failed
 invoke GetFileAttributesW,peDestW
 cmp eax,-1
 jne failed
 inc [testStep]
 ; Existing output, source-contained output, mismatched extension and empty
 ; destination must fail without changing any original file or source.
 mov [peFormat],2
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,existingSuffix
 stdcall Sha1File,peDestW,digest
 stdcall PackExportBuild
 test eax,eax
 jnz failed
 stdcall VerifySha1,peDestW,digest
 cmp eax,1
 jne failed
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,insideSuffix
 stdcall PackExportBuild
 test eax,eax
 jnz failed
 invoke GetFileAttributesW,peDestW
 cmp eax,-1
 jne failed
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,badExtensionSuffix
 stdcall PackExportBuild
 test eax,eax
 jnz failed
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,outputLinkSuffix
 stdcall PackExportBuild
 test eax,eax
 jnz failed
 mov word [peDestW],0
 stdcall PackExportBuild
 test eax,eax
 jnz failed
 inc [testStep]
 ; Each normalized loader's exact version is derived from local metadata.
 stdcall TestSource,loaderSuffix
 stdcall PackExportInspect,sourceW,fabricId,0
 test eax,eax
 jz failed
 cmp [peLoaderKind],1
 jne failed
 invoke lstrcmpA,peLoaderA,.fabricVersion
 test eax,eax
 jnz failed
 mov [loopFormat],1
.loaderRoundtrip:
 mov eax,[loopFormat]
 mov [peFormat],eax
 stdcall PackExportInspect,sourceW,fabricId,0
 cmp eax,1
 jne failed
 invoke lstrcpyW,peDestW,rootW
 mov eax,[loopFormat]
 dec eax
 invoke lstrcatW,peDestW,dword [loaderArchiveSuffixes+eax*4]
 stdcall PackExportBuild
 cmp eax,1
 jne failed
 stdcall PackInspect,peDestW
 cmp eax,1
 jne failed
 cmp [packInspectLoaderKind],1
 jne failed
 invoke lstrcmpA,packInspectLoader,.fabricVersion
 test eax,eax
 jnz failed
 invoke lstrcmpA,packInspectBase,baseId
 test eax,eax
 jnz failed
 invoke lstrcpyA,previewHash,packInspectHash
 cinvoke wsprintfW,suffixW,loaderRoundtripFormat,[loopFormat]
 invoke lstrcpyW,destinationW,rootW
 invoke lstrcatW,destinationW,suffixW
 stdcall PackInstallChecked,peDestW,destinationW,previewHash
 cmp eax,1
 jne failed
 stdcall TestUnchanged
 test eax,eax
 jz failed
 inc [loopFormat]
 cmp [loopFormat],3
 jbe .loaderRoundtrip
 stdcall TestSource,forgeSuffix
 stdcall PackExportInspect,sourceW,forgeId,0
 test eax,eax
 jz failed
 cmp [peLoaderKind],2
 jne failed
 invoke lstrcmpA,peLoaderA,.forgeVersion
 test eax,eax
 jnz failed
 stdcall TestSource,neoSuffix
 stdcall PackExportInspect,sourceW,neoId,0
 test eax,eax
 jz failed
 cmp [peLoaderKind],3
 jne failed
 stdcall TestSource,neoModernSuffix
 stdcall PackExportInspect,sourceW,neoId,0
 test eax,eax
 jz failed
 cmp [peLoaderKind],3
 jne failed
 invoke lstrcmpA,peLoaderA,.neoVersion
 test eax,eax
 jnz failed
 stdcall TestSource,quiltSuffix
 stdcall PackExportInspect,sourceW,quiltId,0
 test eax,eax
 jz failed
 cmp [peLoaderKind],4
 jne failed
 inc [testStep]
 stdcall TestSource,isolateSuffix
 stdcall PackExportInspect,sourceW,isolateId,0
 cmp eax,1
 jne failed
 cmp [peFileCount],1
 jne failed
 inc [testStep]
 stdcall TestSource,reparseSuffix
 stdcall PackExportInspect,sourceW,baseId,0
 test eax,eax
 jnz failed
 stdcall TestSource,oversizeSuffix
 stdcall PackExportInspect,sourceW,baseId,0
 test eax,eax
 jnz failed
 stdcall PackExportInspect,emptyW,baseId,0
 test eax,eax
 jnz failed
 inc [testStep]
 stdcall TestSource,sourceSuffix
 stdcall PackExportInspect,sourceW,baseId,0
 test eax,eax
 jz failed
 invoke lstrcpyW,peDestW,rootW
 invoke lstrcatW,peDestW,cancelSuffix
 mov [engineCancel],1
 stdcall PackExportBuild
 test eax,eax
 jnz failed
 mov [engineCancel],0
 invoke GetFileAttributesW,peDestW
 cmp eax,-1
 jne failed
 stdcall TestUnchanged
 test eax,eax
 jz failed
 invoke ExitProcess,0
.fabricVersion db '0.19.3',0
.forgeVersion db '52.1.20',0
.neoVersion db '21.1.264',0
failed:
 invoke ExitProcess,[testStep]
proc TestSource,suffix
 invoke lstrcpyW,sourceW,rootW
 invoke lstrcatW,sourceW,[suffix]
 ret
endp
proc TestUnchanged
 invoke lstrcmpiW,gameDir,savedGame
 test eax,eax
 jnz .bad
 invoke lstrcmpA,selectedVersion,savedVersion
 test eax,eax
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
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
proc ExportGame
 mov eax,1
 ret
endp
proc ExportLoader,base,kind,version,target
 invoke lstrcpyA,[target],fixtureChild
 mov eax,1
 ret
endp
proc ExportNormalize,child,base
 mov eax,1
 ret
endp
proc ExportMetadata
 mov eax,1
 ret
endp
proc ExportLibraries
 mov eax,1
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\version_settings.inc'
include '..\src\loaders.inc'
include '..\src\profiles.inc'
include '..\src\loader_change.inc'
include '..\src\mods.inc'
include '..\src\curseforge.inc'
define EInstallCore ExportGame
define LoaderInstall ExportLoader
define ProfileNormalize ExportNormalize
define ELoadMetadata ExportMetadata
define EInstallLibraries ExportLibraries
include '..\src\packs.inc'
restore EInstallLibraries
restore ELoadMetadata
restore ProfileNormalize
restore LoaderInstall
restore EInstallCore
include '..\src\pack_export.inc'
include '..\src\imports.inc'
