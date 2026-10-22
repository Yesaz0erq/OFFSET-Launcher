; Native import workflow: inspection, confirmation, safe names and cancellation.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\mod_types.inc'
include '..\src\version_types.inc'
WM_APP=8000h
WM_FINISHED=WM_APP+2
section '.data' data readable writeable
hWnd dd 0
hVersion dd 0
hGameDir dd 0
busy dd 0
currentPage dd 5
detailActive dd 0
modCancel dd 0
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
gameDir rw 1024
javaPath rw 1024
selectedVersion db 'original',0,119 dup(0)
playerName db 'Player',0,57 dup(0)
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
iniPath rw 1024
statusText rw 1024
emptyW dw 0
listClass du 'LISTBOX',0
editClass du 'EDIT',0
originalRoot du 'C:\original-game',0
originalVersion db 'original',0
fixtureRelative du 'tests\out\pack-import-ui-',0
fixtureSerial du '%08X',0
fixtureFile du 'C:\fixture\pack.mrpack',0
fixtureOtherFile du 'C:\fixture\other.zip',0
fixtureName du 'Pack preview',0
fixtureVersion du '1.21.1 / Fabric 0.16.0',0
fixtureFolder du 'My-Pack_01.21',0
fixtureBadName du '..\escape',0
fixtureDosName du 'CON.txt',0
fixtureUnicodeName du '实例',0
fixtureExistingName du 'existing',0
fixtureExistingSuffix du '\existing',0
fixtureCatalogFile du 'C:\fixture\catalog.zip',0
fixtureHash db 40 dup('a'),0
fixtureVanilla du 'Vanilla',0
fixtureFabric du 'Fabric',0
fixtureForge du 'Forge',0
fixtureNeo du 'NeoForge',0
fixtureQuilt du 'Quilt',0
vUiNames dd fixtureVanilla,fixtureFabric,fixtureForge,fixtureNeo,fixtureQuilt
packInspectReady dd 0
packInspectFormat dd 1
packInspectName db 'Pack preview',0
packInspectBase db '1.21.1',0
packInspectLoaderKind dd 1
packInspectLoader db '0.16.0',0
packInspectHash db 40 dup('a'),0
peReady dd 0
peFileCount dd 0
peTotalBytes dd 0
peSummaryA db 0
peDataDirW rw 2048
packProvider dd 0
packSelectedProject dd 0
packSelectedFile dd 0
packProjectCount dd 0
packFileCount dd 0
packOffset dd 0
packTotal dd 0
packQuery rb 512
packGameVersion rb 64
packLoader rb 64
packProjects rb 20*MP_SIZE
packFiles rb 32*MF_SIZE
pkMsgDestination db 'destination',0
chooseResult dd 0
chooseCalls dd 0
workerCalls dd 0
lastWorker dd 0
inspectCalls dd 0
installCalls dd 0
installResult dd 0
inspectResult dd 1
checkedWrong dd 0
ensureCalls dd 0
syncCalls dd 0
postCalls dd 0
testStep dd 1
chooserPointer dd FixtureChooser
ensurePointer dd FixtureEnsure
postPointer dd FixturePost
path rw 2048
serial rw 32
include '..\src\strings.inc'
section '.text' code readable executable
start:
 invoke GetProcessHeap
 mov [eHeap],eax
 stdcall EnsureDirectory,fixtureOut
 invoke lstrcpyW,path,fixtureRelative
 invoke GetTickCount
 cinvoke wsprintfW,serial,fixtureSerial,eax
 invoke lstrcatW,path,serial
 invoke GetFullPathNameW,path,1024,packRoot,0
 stdcall EnsureDirectory,packRoot
 invoke lstrcpyW,path,packRoot
 invoke lstrcatW,path,fixtureExistingSuffix
 stdcall EnsureDirectory,path
 invoke CreateWindowExW,0,editClass,emptyW,WS_POPUP,0,0,300,30,0,0,0,0
 mov [hPackImportFolder],eax
 test eax,eax
 jz failed
 invoke CreateWindowExW,0,listClass,emptyW,WS_POPUP,0,0,300,300,0,0,0,0
 mov [hPackLocal],eax
 invoke lstrcpyW,gameDir,originalRoot
 invoke lstrcpyW,packImportArchiveW,fixtureFile
 mov [packImportReady],1
 ; Cancelling the native chooser preserves an earlier reviewed archive.
 stdcall PackImportLocal
 cmp [workerCalls],0
 jne failed
 cmp [packImportReady],1
 jne failed
 invoke lstrcmpiW,packImportArchiveW,fixtureFile
 test eax,eax
 jnz failed
 inc [testStep]
 ; Choosing a file queues inspection; no install or directory creation.
 mov [chooseResult],1
 mov [ensureCalls],0
 mov [packUiResult],1
 stdcall PackImportLocal
 cmp [packTab],2
 jne failed
 cmp [packUiAction],5
 jne failed
 cmp [packImportReady],0
 jne failed
 cmp [packUiResult],0
 jne failed
 cmp [workerCalls],1
 jne failed
 cmp [lastWorker],PackWorker
 jne failed
 cmp [installCalls],0
 jne failed
 cmp [ensureCalls],0
 jne failed
 stdcall CheckOriginal
 test eax,eax
 jz failed
 inc [testStep]
 ; A worker only inspects. Completion publishes the native preview.
 stdcall PackWorker,0
 cmp [inspectCalls],1
 jne failed
 cmp [installCalls],0
 jne failed
 cmp [ensureCalls],0
 jne failed
 cmp [postCalls],1
 jne failed
 stdcall PackFinishUI
 cmp [packImportReady],1
 jne failed
 cmp [packUiAction],0
 jne failed
 cmp [packTab],2
 jne failed
 invoke lstrcmpiW,packImportNameW,fixtureName
 test eax,eax
 jnz failed
 invoke lstrcmpiW,packImportVersionW,fixtureVersion
 test eax,eax
 jnz failed
 invoke lstrcmpA,packImportHashA,fixtureHash
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 test eax,eax
 jz failed
 inc [testStep]
 ; Import archive ownership is independent of catalogue download paths.
 invoke lstrcpyW,packArchive,fixtureCatalogFile
 invoke lstrcmpiW,packImportArchiveW,fixtureOtherFile
 test eax,eax
 jnz failed
 ; Existing, traversal, DOS-device and non-ASCII folder names are rejected.
 invoke SetWindowTextW,[hPackImportFolder],fixtureBadName
 stdcall PackImportDestination
 test eax,eax
 jnz failed
 invoke SetWindowTextW,[hPackImportFolder],fixtureDosName
 stdcall PackImportDestination
 test eax,eax
 jnz failed
 invoke SetWindowTextW,[hPackImportFolder],fixtureUnicodeName
 stdcall PackImportDestination
 test eax,eax
 jnz failed
 invoke SetWindowTextW,[hPackImportFolder],fixtureExistingName
 stdcall PackImportDestination
 test eax,eax
 jnz failed
 cmp [ensureCalls],0
 jne failed
 inc [testStep]
 ; Clear and confirm have no effect while an operation owns the controls.
 mov [busy],1
 mov [workerCalls],0
 stdcall PackImportClear
 stdcall PackImportStart
 stdcall PackImportLocal
 cmp [packImportReady],1
 jne failed
 cmp [workerCalls],0
 jne failed
 cmp [chooseCalls],2
 jne failed
 mov [busy],0
 inc [testStep]
 ; Confirmation validates a new destination and supplies the reviewed hash.
 invoke SetWindowTextW,[hPackImportFolder],fixtureFolder
 stdcall PackImportDestination
 cmp eax,1
 jne failed
 stdcall PackImportStart
 cmp [packUiAction],4
 jne failed
 cmp [workerCalls],1
 jne failed
 stdcall PackWorker,0
 cmp [installCalls],1
 jne failed
 cmp [checkedWrong],0
 jne failed
 cmp [packUiResult],0
 jne failed
 stdcall PackFinishUI
 cmp [packTab],2
 jne failed
 cmp [packImportReady],1
 jne failed
 stdcall CheckOriginal
 test eax,eax
 jz failed
 inc [testStep]
 ; A successful install enters the installed list without selecting it.
 mov [installResult],1
 stdcall PackImportStart
 stdcall PackWorker,0
 stdcall PackFinishUI
 cmp [packTab],1
 jne failed
 cmp [packUiAction],0
 jne failed
 cmp [packImportReady],0
 jne failed
 cmp [installCalls],2
 jne failed
 stdcall CheckOriginal
 test eax,eax
 jz failed
 inc [testStep]
 ; A failed next inspection cannot resurrect an older metadata preview.
 mov [packTab],2
 mov [inspectResult],0
 stdcall PackImportLocal
 stdcall PackWorker,0
 stdcall PackFinishUI
 cmp [packImportReady],0
 jne failed
 cmp [packTab],2
 jne failed
 inc [testStep]
 ; Clearing a preview is purely UI state and preserves catalogue paths.
 mov [packImportReady],1
 stdcall PackImportClear
 cmp [packImportReady],0
 jne failed
 cmp word [packImportArchiveW],0
 jne failed
 cmp byte [packImportHashA],0
 jne failed
 invoke lstrcmpiW,packArchive,fixtureCatalogFile
 test eax,eax
 jnz failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testStep]
proc CheckOriginal
 invoke lstrcmpiW,gameDir,originalRoot
 test eax,eax
 jnz .bad
 invoke lstrcmpA,selectedVersion,originalVersion
 test eax,eax
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc FixtureChooser uses esi,ofn
 inc [chooseCalls]
 cmp [chooseResult],0
 je .cancel
 mov esi,[ofn]
 invoke lstrcpyW,[esi+OPENFILENAME.lpstrFile],fixtureOtherFile
 mov eax,1
 ret
.cancel:
 xor eax,eax
 ret
endp
proc FixtureEnsure,pathW
 inc [ensureCalls]
 mov eax,1
 ret
endp
proc FixturePost,window,message,wparam,lparam
 inc [postCalls]
 mov eax,1
 ret
endp
proc PackInspect,archiveW
 inc [inspectCalls]
 mov eax,[inspectResult]
 mov [packInspectReady],eax
 ret
endp
proc PackExportInspect,rootW,idA,saves
 xor eax,eax
 ret
endp
proc PackExportBuild
 xor eax,eax
 ret
endp
proc PackInstallChecked,archiveW,destW,hashA
 inc [installCalls]
 invoke lstrcmpiW,[archiveW],fixtureOtherFile
 test eax,eax
 jnz .wrong
 invoke lstrcmpA,[hashA],fixtureHash
 test eax,eax
 jz .result
.wrong:
 mov [checkedWrong],1
.result:
 mov eax,[installResult]
 ret
endp
proc PackInstall,archiveW,destW
 mov [checkedWrong],1
 xor eax,eax
 ret
endp
proc PackSearch
 mov eax,1
 ret
endp
proc PackVersions
 mov eax,1
 ret
endp
proc PackDownload,index,dest
 mov eax,1
 ret
endp
proc StartTransition
 ret
endp
proc SyncPage
 inc [syncCalls]
 ret
endp
proc StartWorker,callback
 inc [workerCalls]
 mov eax,[callback]
 mov [lastWorker],eax
 ret
endp
proc SetStatusA,text
 ret
endp
fixtureOut du 'tests\out',0
include '..\src\common.inc'
include '..\src\engine.inc'
include '..\src\versions.inc'
define GetOpenFileNameW chooserPointer
define EnsureDirectory FixtureEnsure
define PostMessageW postPointer
include '..\src\packs_ui.inc'
restore GetOpenFileNameW
restore EnsureDirectory
restore PostMessageW
include '..\src\imports.inc'
