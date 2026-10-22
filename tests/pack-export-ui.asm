; Native export form: independent source, explicit preview, filters and confirmation.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
section '.data' data readable writeable
hWnd dd 0
busy dd 0
currentPage dd 5
packTab dd 3
packUiAction dd 0
packUiResult dd 0
gameDir rw 1024
selectedVersion db '1.21.1',0,121 dup(0)
statusText rw 1024
comboClass du 'COMBOBOX',0
editClass du 'EDIT',0
emptyW dw 0
openVerb du 'open',0
baseRelative du 'tests\out\pack-export-ui-',0
serialFormat du '%08X',0
rootW rw 1024
otherRootW rw 1024
pathW rw 2048
chosenW rw 2048
textW rw 32768
serialW rw 32
fixtureIdA db '1.21.1',0
fixtureOtherIdA db 'fabric-1.21.1',0
fixtureOtherIdW du 'fabric-1.21.1',0
fixtureBadIdA db '../bad',0
fixtureDraft du 'Draft name',0
fixtureUnicode du 6574h,5408h,5305h,0
fixtureUnicodeA db 0E6h,095h,0B4h,0E5h,090h,088h,0E5h,08Ch,085h,0
fixtureAuthor du 4F5Ch,8005h,0
fixtureBad du 'bad',9,'name',0
fixtureDefaultA db '1.0.0',0
fixtureMrSuffix du '\export.mrpack',0
fixtureZipSuffix du '\export.zip',0
fixtureExistingSuffix du '\existing.mrpack',0
fixtureWrongSuffix du '\export.txt',0
fixtureOtherRootSuffix du '\other',0
fixtureDataSuffix du '\game',0
fixtureSlash du '\',0
fixtureSummary db 'mods/example.jar',13,10,'config/example.toml',0
fixtureSummaryW du 'mods/example.jar',13,10,'config/example.toml',0
peReady dd 0
peFileCount dd 0
peTotalBytes dd 0
peSummaryA rb 32768
peDataDirW rw 2048
peRootW rw 1024
peVersionA rb 128
peIncludeSaves dd 0
peIncludeMods dd 1
peIncludeConfig dd 1
peIncludeResources dd 1
peFormat dd 1
peNameA rb 256
peVersionLabelA rb 128
peAuthorA rb 256
peDestW rw 2048
chooseResult dd 0
chooseCalls dd 0
workerCalls dd 0
lastWorker dd 0
scanCalls dd 0
syncCalls dd 0
goCalls dd 0
lastPage dd -1
testStep dd 1
chooserPointer dd FixtureChooser
include '..\src\strings.inc'
section '.text' code readable executable
start:
 invoke CreateDirectoryW,fixtureOut,0
 invoke lstrcpyW,pathW,baseRelative
 invoke GetTickCount
 cinvoke wsprintfW,serialW,serialFormat,eax
 invoke lstrcatW,pathW,serialW
 invoke GetFullPathNameW,pathW,1024,rootW,0
 invoke CreateDirectoryW,rootW,0
 invoke lstrcpyW,gameDir,rootW
 stdcall CreatePackExportControls
 cmp [hPeUiSource],0
 je failed
 cmp [hPeUiPreview],0
 je failed
 stdcall PackExportUiSource
 cmp [peUiSaves],0
 jne failed
 cmp [peUiMods],1
 jne failed
 cmp [peUiConfig],1
 jne failed
 cmp [peUiResources],1
 jne failed
 cmp [peUiFormat],1
 jne failed
 invoke lstrcmpA,peUiVersionA,fixtureIdA
 test eax,eax
 jnz failed
 invoke lstrcmpiW,peUiRootW,rootW
 test eax,eax
 jnz failed
 invoke GetWindowTextW,[hPeUiVersion],textW,32768
 invoke lstrcmpiW,textW,peUiDefaultVersion
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 inc [testStep]
 ; A different export source never changes the launch selection or draft name.
 invoke SendMessageW,[hPeUiSource],CB_SETCURSEL,1,0
 stdcall PackExportUiCommand,(CBN_SELCHANGE shl 16)+411
 invoke lstrcmpA,peUiVersionA,fixtureOtherIdA
 test eax,eax
 jnz failed
 invoke SetWindowTextW,[hPeUiName],fixtureDraft
 mov [peUiReady],1
 stdcall PackExportUiSource
 invoke lstrcmpA,peUiVersionA,fixtureOtherIdA
 test eax,eax
 jnz failed
 cmp [peUiReady],1
 jne failed
 invoke lstrcatW,gameDir,fixtureSlash
 stdcall PackExportUiSource
 cmp [peUiReady],1
 jne failed
 invoke lstrcpyW,gameDir,rootW
 invoke GetWindowTextW,[hPeUiName],textW,32768
 invoke lstrcmpiW,textW,fixtureDraft
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 inc [testStep]
 ; Missing preview never opens the save dialog. Preview only queues read-only work.
 mov [peUiReady],0
 stdcall PackExportUiChoose
 cmp [chooseCalls],0
 jne failed
 mov [packUiResult],1
 stdcall PackExportUiPreview
 cmp [workerCalls],1
 jne failed
 cmp [lastWorker],PackWorker
 jne failed
 cmp [packUiAction],6
 jne failed
 cmp [packUiResult],0
 jne failed
 cmp [peUiReady],0
 jne failed
 stdcall CheckOriginal
 inc [testStep]
 ; Successful inspection publishes the UTF-8 file list and matching source tuple.
 invoke lstrcpyW,statusText,fixtureDraft
 stdcall FixturePreview
 stdcall PackExportUiFinish
 cmp [peUiReady],1
 jne failed
 cmp [peUiFiles],2
 jne failed
 cmp [peUiBytes],4096
 jne failed
 invoke lstrcmpiW,statusText,sReady
 test eax,eax
 jnz failed
 invoke GetWindowTextW,[hPeUiPreview],textW,32768
 invoke lstrcmpiW,textW,fixtureSummaryW
 test eax,eax
 jnz failed
 invoke SendMessageW,[hPeUiPreview],WM_CHAR,'X',0
 invoke GetWindowTextW,[hPeUiPreview],textW,32768
 invoke lstrcmpiW,textW,fixtureSummaryW
 test eax,eax
 jnz failed
 cmp [syncCalls],1
 jne failed
 inc [testStep]
 ; Cancelling a chooser preserves an earlier destination and writes no file.
 stdcall FixturePath,fixtureMrSuffix
 invoke lstrcpyW,peUiDestW,chosenW
 stdcall PackExportUiChoose
 cmp [chooseCalls],1
 jne failed
 invoke lstrcmpiW,peUiDestW,chosenW
 test eax,eax
 jnz failed
 invoke GetFileAttributesW,chosenW
 cmp eax,-1
 jne failed
 inc [testStep]
 ; Existing files and wrong extensions cannot replace a confirmed target.
 stdcall FixturePath,fixtureExistingSuffix
 invoke CreateFileW,chosenW,GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je failed
 invoke CloseHandle,eax
 mov [chooseResult],1
 stdcall PackExportUiChoose
 stdcall FixturePath,fixtureMrSuffix
 invoke lstrcmpiW,peUiDestW,chosenW
 test eax,eax
 jnz failed
 stdcall FixturePath,fixtureWrongSuffix
 stdcall PackExportUiChoose
 stdcall FixturePath,fixtureMrSuffix
 invoke lstrcmpiW,peUiDestW,chosenW
 test eax,eax
 jnz failed
 mov word [peUiDestW],0
 stdcall PackExportUiChoose
 invoke lstrcmpiW,peUiDestW,chosenW
 test eax,eax
 jnz failed
 invoke GetFileAttributesW,chosenW
 cmp eax,-1
 jne failed
 inc [testStep]
 ; Format switching retains content preview and asks for an appropriate new path.
 stdcall ClickPackExport,660,380
 cmp [peUiFormat],2
 jne failed
 cmp [peUiReady],1
 jne failed
 cmp word [peUiDestW],0
 jne failed
 stdcall FixturePath,fixtureZipSuffix
 stdcall PackExportUiChoose
 invoke lstrcmpiW,peUiDestW,chosenW
 test eax,eax
 jnz failed
 inc [testStep]
 ; Metadata has bounded UTF-8 conversion and the PCL default version.
 invoke SetWindowTextW,[hPeUiName],fixtureUnicode
 invoke SetWindowTextW,[hPeUiVersion],emptyW
 invoke SetWindowTextW,[hPeUiAuthor],fixtureAuthor
 stdcall PackExportUiMetadata
 test eax,eax
 jz failed
 invoke lstrcmpA,peNameA,fixtureUnicodeA
 test eax,eax
 jnz failed
 invoke lstrcmpA,peVersionLabelA,fixtureDefaultA
 test eax,eax
 jnz failed
 invoke SetWindowTextW,[hPeUiName],fixtureBad
 stdcall PackExportUiStart
 cmp [workerCalls],1
 jne failed
 invoke SetWindowTextW,[hPeUiName],fixtureUnicode
 mov [packUiResult],1
 stdcall PackExportUiStart
 cmp [workerCalls],2
 jne failed
 cmp [packUiAction],7
 jne failed
 cmp [packUiResult],0
 jne failed
 cmp [peFormat],2
 jne failed
 invoke lstrcmpiW,peDestW,peUiDestW
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 inc [testStep]
 ; A busy form cannot queue work, open a chooser or change content choices.
 mov [busy],1
 mov [chooseCalls],0
 stdcall PackExportUiPreview
 stdcall PackExportUiChoose
 stdcall PackExportUiStart
 stdcall ClickPackExport,960,432
 cmp [workerCalls],2
 jne failed
 cmp [chooseCalls],0
 jne failed
 cmp [peUiSaves],0
 jne failed
 mov [busy],0
 inc [testStep]
 ; Each category change invalidates the old file list and destination.
 stdcall ClickPackExport,470,432
 cmp [peUiMods],0
 jne failed
 cmp [peUiReady],0
 jne failed
 cmp word [peUiDestW],0
 jne failed
 stdcall ClickPackExport,630,432
 cmp [peUiConfig],0
 jne failed
 stdcall ClickPackExport,790,432
 cmp [peUiResources],0
 jne failed
 stdcall ClickPackExport,960,432
 cmp [peUiSaves],1
 jne failed
 stdcall PackExportUiPreview
 cmp [workerCalls],3
 jne failed
 cmp [peIncludeMods],0
 jne failed
 cmp [peIncludeConfig],0
 jne failed
 cmp [peIncludeResources],0
 jne failed
 inc [testStep]
 ; Changed backend source/settings cannot reuse a prior confirmation.
 stdcall FixturePreview
 stdcall PackExportUiFinish
 stdcall FixturePath,fixtureZipSuffix
 invoke lstrcpyW,peUiDestW,chosenW
 mov [peIncludeConfig],1
 stdcall PackExportUiStart
 cmp [workerCalls],3
 jne failed
 mov [peIncludeConfig],0
 mov byte [peVersionA],'X'
 stdcall PackExportUiStart
 cmp [workerCalls],3
 jne failed
 inc [testStep]
 ; Same version under another root invalidates the preview, never launch selection.
 invoke SendMessageW,[hPeUiSource],CB_SETCURSEL,0,0
 stdcall PackExportUiCommand,(CBN_SELCHANGE shl 16)+411
 invoke lstrcmpA,peUiVersionA,fixtureIdA
 test eax,eax
 jnz failed
 invoke lstrcpyW,otherRootW,rootW
 invoke lstrcatW,otherRootW,fixtureOtherRootSuffix
 invoke lstrcpyW,gameDir,otherRootW
 mov [peUiReady],1
 stdcall PackExportUiSource
 cmp [peUiReady],0
 jne failed
 cmp word [peUiDestW],0
 jne failed
 invoke lstrcmpA,selectedVersion,fixtureIdA
 test eax,eax
 jnz failed
 inc [testStep]
 ; Failed work cannot resurrect success; success stays on the export tab.
 mov [packUiAction],7
 mov [packUiResult],1
 stdcall PackExportUiFinish
 cmp [peUiExported],1
 jne failed
 cmp [packTab],3
 jne failed
 mov [packUiResult],0
 stdcall PackExportUiFinish
 cmp [peUiExported],0
 jne failed
 mov [peUiReady],1
 mov [packUiAction],6
 stdcall PackExportUiFinish
 cmp [peUiReady],0
 jne failed
 inc [testStep]
 ; A version-workspace entry preselects a different ID without selecting it to launch.
 invoke lstrcpyW,gameDir,rootW
 invoke SetWindowTextW,[hPeUiName],fixtureDraft
 invoke SetWindowTextW,[hPeUiAuthor],fixtureAuthor
 mov [peUiReady],1
 stdcall FixturePath,fixtureZipSuffix
 invoke lstrcpyW,peUiDestW,chosenW
 stdcall PackExportUiOpen,rootW,fixtureOtherIdA
 cmp eax,1
 jne failed
 cmp [goCalls],1
 jne failed
 cmp [lastPage],5
 jne failed
 cmp [packTab],3
 jne failed
 invoke lstrcmpA,peUiVersionA,fixtureOtherIdA
 test eax,eax
 jnz failed
 invoke GetWindowTextW,[hPeUiName],textW,32768
 invoke lstrcmpiW,textW,fixtureOtherIdW
 test eax,eax
 jnz failed
 invoke GetWindowTextW,[hPeUiVersion],textW,32768
 invoke lstrcmpiW,textW,peUiDefaultVersion
 test eax,eax
 jnz failed
 invoke GetWindowTextW,[hPeUiAuthor],textW,32768
 test eax,eax
 jnz failed
 cmp [peUiReady],0
 jne failed
 cmp word [peUiDestW],0
 jne failed
 stdcall CheckOriginal
 inc [testStep]
 ; Returning to the same source keeps the draft and reviewed preview.
 invoke SetWindowTextW,[hPeUiName],fixtureDraft
 mov [peUiReady],1
 invoke lstrcpyW,peUiDestW,chosenW
 stdcall PackExportUiOpen,rootW,fixtureOtherIdA
 cmp eax,1
 jne failed
 cmp [peUiReady],1
 jne failed
 invoke GetWindowTextW,[hPeUiName],textW,32768
 invoke lstrcmpiW,textW,fixtureDraft
 test eax,eax
 jnz failed
 invoke lstrcmpiW,peUiDestW,chosenW
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 inc [testStep]
 ; Foreign roots, invalid IDs and busy entry leave the export context intact.
 stdcall PackExportUiOpen,otherRootW,fixtureIdA
 test eax,eax
 jnz failed
 stdcall PackExportUiOpen,rootW,fixtureBadIdA
 test eax,eax
 jnz failed
 mov [busy],1
 stdcall PackExportUiOpen,rootW,fixtureIdA
 test eax,eax
 jnz failed
 mov [busy],0
 cmp [goCalls],2
 jne failed
 cmp [peUiReady],1
 jne failed
 invoke lstrcmpA,peUiVersionA,fixtureOtherIdA
 test eax,eax
 jnz failed
 stdcall CheckOriginal
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testStep]
proc CheckOriginal
 invoke lstrcmpiW,gameDir,rootW
 test eax,eax
 jnz failed
 invoke lstrcmpA,selectedVersion,fixtureIdA
 test eax,eax
 jnz failed
 ret
endp
proc FixturePath,suffix
 invoke lstrcpyW,chosenW,rootW
 invoke lstrcatW,chosenW,[suffix]
 ret
endp
proc FixturePreview
 mov [packUiAction],6
 mov [packUiResult],1
 mov [peReady],1
 mov [peFileCount],2
 mov [peTotalBytes],4096
 invoke lstrcpyA,peSummaryA,fixtureSummary
 invoke lstrcpyW,peRootW,peUiRootW
 invoke lstrcpyA,peVersionA,peUiVersionA
 invoke lstrcpyW,peDataDirW,peUiDataDirW
 mov eax,[peUiSaves]
 mov [peIncludeSaves],eax
 ret
endp
proc FixtureChooser uses esi,ofn
 inc [chooseCalls]
 cmp [chooseResult],0
 je .cancel
 mov esi,[ofn]
 invoke lstrcpyW,[esi+OPENFILENAME.lpstrFile],chosenW
 mov eax,1
 ret
.cancel:
 xor eax,eax
 ret
endp
proc NewControl,class,style,id,x,y,w,h
 mov eax,[style]
 or eax,WS_POPUP
 invoke CreateWindowExW,0,[class],emptyW,eax,[x],[y],[w],[h],0,0,0,0
 ret
endp
proc EngineScanVersions,combo
 inc [scanCalls]
 invoke SendMessageW,[combo],CB_RESETCONTENT,0,0
 invoke SendMessageA,[combo],CB_ADDSTRING,0,fixtureIdA
 invoke SendMessageA,[combo],CB_ADDSTRING,0,fixtureOtherIdA
 mov eax,2
 ret
endp
proc VsResolveData,root,id,dest,capacity
 invoke lstrcpyW,[dest],[root]
 invoke lstrcatW,[dest],fixtureDataSuffix
 mov eax,1
 ret
endp
proc VsSafeId,id
 invoke lstrcmpA,[id],fixtureIdA
 test eax,eax
 jz .good
 invoke lstrcmpA,[id],fixtureOtherIdA
 test eax,eax
 jnz .bad
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc GoPage,page
 inc [goCalls]
 mov eax,[page]
 mov [lastPage],eax
 mov [currentPage],eax
 ; Simulate GoPage's PullSettings modifying globals before SyncPage.
 invoke lstrcpyA,selectedVersion,fixtureOtherIdA
 invoke lstrcatW,gameDir,fixtureSlash
 stdcall ShowPackExportControls
 ret
endp
proc StartWorker,callback
 inc [workerCalls]
 mov eax,[callback]
 mov [lastWorker],eax
 ret
endp
proc PackWorker,param
 ret
endp
proc SyncPage
 inc [syncCalls]
 ret
endp
fixtureOut du 'tests\out',0
define GetSaveFileNameW chooserPointer
include '..\src\pack_export_ui.inc'
restore GetSaveFileNameW
include '..\src\imports.inc'

