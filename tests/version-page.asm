; Native UI draft/context regression. Backend storage is tested separately.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
section '.data' data readable writeable
busy dd 0
engineCancel dd 0
modCancel dd 0
currentPage dd 0
versionTab dd 1
hWnd dd 0
hVInstalled dd 0
gameDir du 'global-root',0
selectedVersion db 'current-version',0,112 dup(0)
expectedSelected db 'current-version',0
expectedGame du 'global-root',0
editClass du 'EDIT',0
listClass du 'LISTBOX',0
emptyW dw 0
openVerb du 'open',0
fmtNum du '%u',0
vsMemory dd 0
vsIsolate dd 0
vsLoaderKind dd 0
vsContextValid dd 0
vsRootW rw 1024
vsIdA rb 128
vsNameW rw 40
vsJavaW rw 1024
vsBaseVersionA rb 128
vsDataDirW rw 2048
vsProfileDirW rw 2048
vUiTarget rb 128
fixtureRoot du 'fixture-root',0
fixtureOtherRoot du 'other-root',0
fixtureData du 'fixture-root\versions\1.21.1\game',0
fixtureId db '1.21.1',0
fixtureOtherId db '24w33a',0
fixtureBad db 'missing',0
fixtureIdW du '1.21.1',0
fixtureName du 'Original',0
fixtureNewName du 'New alias',0
fixtureDraftName du 'Draft alias',0
fixtureMemory du '768',0
fixtureZero du '0',0
fixtureBadMemory du '0x',0
fixtureOverLimit du '8193',0
fixtureInvalidName du 'Rejected',0
fixtureJava du 'C:\Java\bin\javaw.exe',0
callsSave dd 0
callsLoad dd 0
savedCalls dd 0
fixtureAttributesPointer dd FixtureAttributes
forceReject dd 0
callsModsShow dd 0
callsModsCommand dd 0
callsExport dd 0
callsRepair dd 0
lastExportRoot dd 0
lastExportId dd 0
lastStatus dd 0
testStep dd 1
include '..\src\strings.inc'
section '.text' code readable executable
start:
 mov [memoryLimitMb],8192
 stdcall CreateVersionPageControls
 stdcall VersionPageOpen,fixtureRoot,fixtureId
 cmp eax,1
 jne .failed
 cmp [vmActive],1
 jne .failed
 cmp [currentPage],1
 jne .failed
 invoke lstrcmpA,selectedVersion,expectedSelected
 test eax,eax
 jnz .failed
 invoke lstrcmpiW,gameDir,expectedGame
 test eax,eax
 jnz .failed
 invoke lstrcmpiW,vmRootW,fixtureRoot
 test eax,eax
 jnz .failed
 invoke lstrcmpA,vmIdA,fixtureId
 test eax,eax
 jnz .failed
 ; An unsaved draft stays in its native edit across tab switches.
 inc [testStep]
 mov [vmTab],1
 stdcall SyncPage
 invoke SetWindowTextW,[hVmName],fixtureDraftName
 stdcall ClickVersionPage,600,145
 cmp [vmTab],2
 jne .failed
 cmp [callsModsShow],1
 jne .failed
 stdcall ClickVersionPage,480,145
 cmp [vmTab],1
 jne .failed
 invoke GetWindowTextW,[hVmName],vmTextW,2048
 invoke lstrcmpiW,vmTextW,fixtureDraftName
 test eax,eax
 jnz .failed
 invoke lstrcmpiW,vsNameW,fixtureName
 test eax,eax
 jnz .failed
 ; Isolation toggle is a draft, and only Save publishes model changes.
 inc [testStep]
 stdcall ClickVersionPage,650,460
 cmp [vmDraftIsolate],1
 jne .failed
 cmp [vsIsolate],0
 jne .failed
 invoke SetWindowTextW,[hVmName],fixtureNewName
 invoke SetWindowTextW,[hVmJava],fixtureJava
 invoke SetWindowTextW,[hVmMemory],fixtureMemory
 stdcall VersionPageSave
 cmp eax,1
 jne .failed
 cmp [callsSave],1
 jne .failed
 cmp [vsMemory],768
 jne .failed
 cmp [vsIsolate],1
 jne .failed
 invoke lstrcmpiW,vsNameW,fixtureNewName
 test eax,eax
 jnz .failed
 invoke lstrcmpiW,vmDataDirW,fixtureData
 test eax,eax
 jnz .failed
 invoke lstrcmpA,selectedVersion,expectedSelected
 test eax,eax
 jnz .failed
 ; Malformed and over-limit memory never reaches persistence.
 inc [testStep]
 invoke SetWindowTextW,[hVmMemory],fixtureBadMemory
 stdcall VersionPageSave
 test eax,eax
 jnz .failed
 cmp [callsSave],1
 jne .failed
 cmp [vsMemory],768
 jne .failed
 invoke SetWindowTextW,[hVmMemory],fixtureOverLimit
 stdcall VersionPageSave
 test eax,eax
 jnz .failed
 cmp [callsSave],1
 jne .failed
 ; Backend refusal restores the previously loaded model, including Java/name.
 inc [testStep]
 mov [forceReject],1
 invoke SetWindowTextW,[hVmName],fixtureInvalidName
 invoke SetWindowTextW,[hVmMemory],fixtureZero
 stdcall VersionPageSave
 test eax,eax
 jnz .failed
 cmp [vsMemory],768
 jne .failed
 cmp [vsIsolate],1
 jne .failed
 invoke lstrcmpiW,vsNameW,fixtureNewName
 test eax,eax
 jnz .failed
 invoke lstrcmpiW,vsJavaW,fixtureJava
 test eax,eax
 jnz .failed
 ; Busy input neither leaves the page nor changes isolation or settings.
 inc [testStep]
 mov [busy],1
 stdcall ClickVersionPage,320,145
 cmp [vmTab],1
 jne .failed
 stdcall VersionPageOpen,fixtureOtherRoot,fixtureOtherId
 test eax,eax
 jnz .failed
 invoke lstrcmpiW,vmRootW,fixtureRoot
 test eax,eax
 jnz .failed
 mov [busy],0
 ; Another version opens its own context without selecting it globally.
 inc [testStep]
 stdcall VersionPageOpen,fixtureOtherRoot,fixtureOtherId
 cmp eax,1
 jne .failed
 invoke lstrcmpiW,vmRootW,fixtureOtherRoot
 test eax,eax
 jnz .failed
 invoke lstrcmpA,vmIdA,fixtureOtherId
 test eax,eax
 jnz .failed
 cmp [vmTab],0
 jne .failed
 invoke lstrcmpA,selectedVersion,expectedSelected
 test eax,eax
 jnz .failed
 ; The last folder button includes its final eight pixels; gaps stay inactive.
 inc [testStep]
 mov eax,[callsLoad]
 mov [savedCalls],eax
 stdcall ClickVersionPage,390,640
 mov eax,[callsLoad]
 cmp eax,[savedCalls]
 jne .failed
 stdcall ClickVersionPage,1087,640
 mov eax,[savedCalls]
 inc eax
 cmp eax,[callsLoad]
 jne .failed
 ; Overview export uses its own version context, preserving the launch selection.
 inc [testStep]
 mov [busy],1
 stdcall ClickVersionPage,900,570
 cmp [callsExport],0
 jne .failed
 mov [busy],0
 stdcall ClickVersionPage,900,520
 cmp [callsRepair],1
 jne .failed
 stdcall ClickVersionPage,900,548
 cmp [callsExport],0
 jne .failed
 stdcall ClickVersionPage,900,570
 cmp [callsExport],1
 jne .failed
 cmp [lastExportRoot],vmRootW
 jne .failed
 cmp [lastExportId],vmIdA
 jne .failed
 invoke lstrcmpiW,[lastExportRoot],fixtureOtherRoot
 test eax,eax
 jnz .failed
 invoke lstrcmpA,[lastExportId],fixtureOtherId
 test eax,eax
 jnz .failed
 invoke lstrcmpiW,gameDir,expectedGame
 test eax,eax
 jnz .failed
 invoke lstrcmpA,selectedVersion,expectedSelected
 test eax,eax
 jnz .failed
 ; An unavailable profile leaves a safe list, not stale workspace state.
 inc [testStep]
 stdcall VersionPageOpen,fixtureRoot,fixtureBad
 test eax,eax
 jnz .failed
 cmp [vmActive],0
 jne .failed
 cmp [versionTab],1
 jne .failed
 invoke lstrcmpiW,gameDir,expectedGame
 test eax,eax
 jnz .failed
 invoke ExitProcess,0
.failed:
 invoke ExitProcess,[testStep]

proc NewControl,cls,style,id,x,y,w,h
 mov eax,[style]
 or eax,WS_POPUP
 invoke CreateWindowExW,0,[cls],emptyW,eax,0,0,[w],[h],0,0,0,0
 ret
endp
proc VsLoad uses esi edi,sourceRoot,sourceId
 inc [callsLoad]
 invoke lstrcmpA,[sourceId],fixtureBad
 test eax,eax
 jz .bad
 invoke lstrcpyW,vsRootW,[sourceRoot]
 invoke lstrcpyA,vsIdA,[sourceId]
 invoke lstrcpyW,vsDataDirW,[sourceRoot]
 invoke lstrcpyW,vsProfileDirW,[sourceRoot]
 invoke lstrcpyA,vsBaseVersionA,[sourceId]
 invoke lstrcpyW,vsNameW,fixtureName
 mov word [vsJavaW],0
 mov [vsMemory],1024
 mov [vsIsolate],0
 mov [vsContextValid],1
 mov eax,1
 ret
.bad:
 mov [vsContextValid],0
 xor eax,eax
 ret
endp
proc VsSave
 inc [callsSave]
 cmp [forceReject],0
 jne .bad
 cmp [vsIsolate],1
 jne .good
 invoke lstrcpyW,vsDataDirW,fixtureData
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc VersionStatusW,source
 mov eax,[source]
 mov [lastStatus],eax
 ret
endp
proc StartTransition
 ret
endp
proc SyncPage
 stdcall HideVersionPageControls
 stdcall ShowVersionPageControls
 ret
endp
proc CreateVmModsControls
 ret
endp
proc VmModsHide
 ret
endp
proc VmModsEnable,enabled
 ret
endp
proc VmModsShow
 inc [callsModsShow]
 ret
endp
proc VmModsCommand,wparam
 inc [callsModsCommand]
 ret
endp
proc VmModsClick,x,y
 ret
endp
proc VersionPageLaunch
 ret
endp
proc VersionPageUse
 ret
endp
proc VersionPageRepair
 inc [callsRepair]
 ret
endp
proc PackExportUiOpen,root,id
 inc [callsExport]
 mov eax,[root]
 mov [lastExportRoot],eax
 mov eax,[id]
 mov [lastExportId],eax
 mov eax,1
 ret
endp
proc EnsureDirectory,path
 mov eax,1
 ret
endp
proc FixtureAttributes,path
 or eax,-1
 ret
endp
include '..\src\memory.inc'
define GetFileAttributesW fixtureAttributesPointer
include '..\src\version_page.inc'
restore GetFileAttributesW
include '..\src\imports.inc'
