; Deterministic catalogue and queue tests. Real JSON, mocked install/network.
format PE console 6.0
stack 100000h,40000h
entry start
include 'win32ax.inc'
include '..\src\version_types.inc'
WM_APP = 8000h
macro check_true {inc [testNo]
 test eax,eax
 jz failed}
macro check_false {inc [testNo]
 test eax,eax
 jnz failed}
macro check value,expected {inc [testNo]
 cmp value,expected
 jne failed}

section '.data' data readable writeable
hWnd dd 0
engineCancel dd 0
selectedVersion db 'original-profile',0,111 dup(0)
testNo dd 0
fixtureBad dd 0
requestCalls dd 0
requestMode dd 0
responseCount dd 0
contextCount dd 0
responsePointer dd 0
postCalls dd 0
installCalls dd 0
cleanupCalls dd 0
loaderCalls dd 0
normalizeCalls dd 0
validateCalls dd 0
buildPathsCalls dd 0
metadataCalls dd 0
libraryCalls dd 0
failInstallAt dd 0
failLoader dd 0
failNormalize dd 0
failChildStage dd 0
cancelStage dd 0
fixturePostImport dd FixturePost
base1 db '1.21.1',0
base2 db '24w33a',0
loader1 db '0.16.10',0
loader2 db '0.16.9',0
original db 'original-profile',0
installedTarget db 'fabric-profile',0
empty db 0
badId db '../escape',0
conId db 'CON',0
conSuffix db 'con.json',0
lptId db 'LPT9.jar',0
comId db 'com1',0
trailingDot db '1.20.',0
leadingDot db '.profile',0
percentId db '1.2%2f3',0
validConPrefix db 'console-1.2',0
badIds dd badId,conId,conSuffix,lptId,comId,trailingDot,leadingDot,percentId,empty
badIdCount=($-badIds)/4
goodJson db '{"versions":[{"id":"1.21.1","type":"release","releaseTime":"2024-08-08T12:00:00Z"},{"id":"24w33a","type":"snapshot","releaseTime":"2024-08-14T12:00:00Z"},{"id":"a1.2.3","type":"old_alpha"},{"id":"../escape","type":"release"},{"id":"CON","type":"release"},{"id":"1.20.","type":"release"}]}',0
badJson db '{"versions":broken}',0
objectJson db '{"versions":{"id":"1.21.1","type":"release"}}',0
emptyJson db '{"versions":[]}',0
nulJson db '{"versions":[{"id":"1.21.1\u0000bad","type":"release"},{"id":"1.21.1","type":"release\u0000"}]}',0
unsupportedJson db '{"versions":[{"id":"b1.7.3","type":"old_beta"}]}',0
queueFmt db '1.%u',0
expectedFirst db '1.1',0
caseVersion rb 128
lastStatus rb 1024

section '.text' code readable executable
start:
 mov esi,badIds
 mov edi,badIdCount
.badIds:
 stdcall VersionsSafeId,dword [esi]
 check_false
 add esi,4
 dec edi
 jnz .badIds
 stdcall VersionsSafeId,validConPrefix
 check_true
 stdcall VersionsSafeId,base1
 check_true
 stdcall VersionsSafeId,base2
 check_true
 stdcall VersionsParse,goodJson
 check_true
 check [versionCount],2
 check [versionCatalogReady],1
 invoke lstrcmpA,versionRecords,base1
 check eax,0
 check [versionRecords+VC_TYPE],0
 invoke lstrcmpA,versionRecords+VC_SIZE,base2
 check eax,0
 check [versionRecords+VC_SIZE+VC_TYPE],1
 stdcall VersionsParse,badJson
 check_false
 check [versionCount],2
 stdcall VersionsParse,objectJson
 check_false
 stdcall VersionsParse,emptyJson
 check_false
 stdcall VersionsParse,nulJson
 check_false
 stdcall VersionsParse,unsupportedJson
 check_false
 mov [engineCancel],1
 stdcall VersionsParse,goodJson
 check_false
 check [versionCount],2
 mov [engineCancel],0
 check [contextCount],0
 ; Good mirror, malformed mirror, status failure, empty mirror and transport.
 mov [requestMode],0
 stdcall FixtureFetchReset
 stdcall VersionsFetch
 check_true
 check [requestCalls],1
 check [versionCount],2
 check [responseCount],0
 check [contextCount],0
 mov [requestMode],1
 stdcall FixtureFetchReset
 stdcall VersionsFetch
 check_true
 check [requestCalls],2
 check [responseCount],0
 check [contextCount],0
 mov [requestMode],2
 stdcall FixtureFetchReset
 stdcall VersionsFetch
 check_true
 check [requestCalls],2
 mov [requestMode],4
 stdcall FixtureFetchReset
 stdcall VersionsFetch
 check_true
 check [requestCalls],2
 mov [requestMode],6
 stdcall FixtureFetchReset
 stdcall VersionsFetch
 check_true
 check [requestCalls],2
 mov [requestMode],3
 stdcall FixtureFetchReset
 stdcall VersionsFetch
 check_false
 check [requestCalls],2
 check [versionCount],2
 mov [requestMode],5
 stdcall FixtureFetchReset
 stdcall VersionsFetch
 check_false
 check [requestCalls],1
 check [responseCount],0
 check [contextCount],0
 mov [engineCancel],0
 ; Queue validation, duplicate coalescing, and a distinct loader combination.
 stdcall VersionQueueClear
 check_true
 stdcall VersionQueueAdd,badId,0,0
 check_false
 stdcall VersionQueueAdd,base1,5,loader1
 check_false
 stdcall VersionQueueAdd,base1,1,empty
 check_false
 stdcall VersionQueueAdd,base1,0,0
 check_true
 stdcall VersionQueueAdd,base1,0,0
 check_true
 check [versionQueueCount],1
 invoke lstrcmpA,versionQueue+VQ_TARGET,base1
 check eax,0
 stdcall VersionQueueAdd,base1,1,loader1
 check_true
 check [versionQueueCount],2
 stdcall VersionQueueAdd,base1,1,loader1
 check_true
 check [versionQueueCount],2
 stdcall VersionQueueAdd,base1,1,loader2
 check_true
 check [versionQueueCount],3
 stdcall VersionQueueRemove,1
 check_true
 check [versionQueueCount],2
 invoke lstrcmpA,versionQueue+VQ_SIZE+VQ_VERSION,loader2
 check eax,0
 stdcall VersionQueueRemove,999
 check_false
 mov [versionQueueIndex],0
 stdcall VersionQueueClear
 check_false
 stdcall VersionQueueRemove,0
 check_false
 stdcall VersionQueueAdd,base2,0,0
 check_false
 mov [versionQueueIndex],-1
 mov [versionQueue+VQ_STATE],1
 stdcall VersionQueueClear
 check_false
 stdcall VersionQueueRemove,0
 check_false
 mov [versionQueue+VQ_STATE],0
 stdcall VersionQueueClear
 check_true
 ; Bounds: exactly capacity unique rows, then reject one extra.
 xor esi,esi
.capacity:
 cinvoke wsprintfA,caseVersion,queueFmt,esi
 stdcall VersionQueueAdd,caseVersion,0,0
 check_true
 inc esi
 cmp esi,VQ_CAPACITY
 jb .capacity
 check [versionQueueCount],VQ_CAPACITY
 stdcall VersionQueueAdd,base2,0,0
 check_false
 stdcall VersionQueueRemove,0
 check_true
 check [versionQueueCount],VQ_CAPACITY-1
 invoke lstrcmpA,versionQueue,expectedFirst
 check eax,0
 stdcall VersionQueueClear
 check_true
 ; A failed task must not prevent a later loader task from installing.
 stdcall FixtureInstallReset
 stdcall FixtureThreeRows
 mov [failInstallAt],2
 stdcall VersionQueueRun,0
 check_false
 check [installCalls],3
 check [cleanupCalls],4
 check [loaderCalls],1
 check [normalizeCalls],1
 check [validateCalls],1
 check [buildPathsCalls],1
 check [metadataCalls],1
 check [libraryCalls],1
 check [postCalls],1
 check [versionQueueIndex],-1
 check [versionQueue+VQ_STATE],2
 check [versionQueue+VQ_SIZE+VQ_STATE],3
 check [versionQueue+VQ_SIZE*2+VQ_STATE],2
 check [versionQueueDone],2
 check [versionQueueFailed],1
 invoke lstrcmpA,selectedVersion,original
 check eax,0
 invoke lstrcmpA,versionQueue+VQ_SIZE*2+VQ_TARGET,installedTarget
 check eax,0
 ; Non-pending rows are skipped, with a single completion notification.
 stdcall FixtureInstallReset
 stdcall VersionQueueRun,0
 check_false
 check [installCalls],0
 check [postCalls],1
 ; Adding a failed duplicate schedules only that row for repair.
 stdcall VersionQueueAdd,base2,0,0
 check_true
 check [versionQueueCount],3
 check [versionQueue+VQ_SIZE+VQ_STATE],0
 stdcall FixtureInstallReset
 stdcall VersionQueueRun,0
 check_true
 check [installCalls],1
 check [postCalls],1
 check [versionQueueDone],3
 check [versionQueueFailed],0
 ; Cancellation before start preserves every pending row and selected version.
 stdcall VersionQueueClear
 stdcall FixtureThreeRows
 stdcall FixtureInstallReset
 mov [engineCancel],1
 stdcall VersionQueueRun,0
 check_false
 check [installCalls],0
 check [postCalls],1
 check [versionQueue+VQ_STATE],0
 check [versionQueue+VQ_SIZE+VQ_STATE],0
 invoke lstrcmpA,selectedVersion,original
 check eax,0
 ; Cancellation after vanilla install marks only the active row cancelled.
 mov [engineCancel],0
 stdcall VersionQueueClear
 stdcall FixtureThreeRows
 stdcall FixtureInstallReset
 mov [cancelStage],1
 stdcall VersionQueueRun,0
 check_false
 check [installCalls],1
 check [cleanupCalls],1
 check [postCalls],1
 check [versionQueue+VQ_STATE],4
 check [versionQueue+VQ_SIZE+VQ_STATE],0
 check [versionQueueIndex],-1
 invoke lstrcmpA,selectedVersion,original
 check eax,0
 ; Loader and normalization failure each leave an explicit failed row.
 mov [engineCancel],0
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall VersionQueueAdd,base2,0,0
 stdcall FixtureInstallReset
 mov [failLoader],1
 stdcall VersionQueueRun,0
 check_false
 check [installCalls],2
 check [loaderCalls],1
 check [normalizeCalls],0
 check [libraryCalls],0
 check [versionQueue+VQ_STATE],3
 check [versionQueue+VQ_SIZE+VQ_STATE],2
 check [postCalls],1
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall FixtureInstallReset
 mov [failNormalize],1
 stdcall VersionQueueRun,0
 check_false
 check [normalizeCalls],1
 check [libraryCalls],0
 check [versionQueue+VQ_STATE],3
 check [postCalls],1
 ; Cancellation returned from a loader/normalizer wins over success.
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall FixtureInstallReset
 mov [cancelStage],2
 stdcall VersionQueueRun,0
 check_false
 check [versionQueue+VQ_STATE],4
 check [normalizeCalls],0
 check [libraryCalls],0
 check [postCalls],1
 mov [engineCancel],0
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall FixtureInstallReset
 mov [cancelStage],3
 stdcall VersionQueueRun,0
 check_false
 check [versionQueue+VQ_STATE],4
 check [normalizeCalls],1
 check [libraryCalls],0
 check [postCalls],1
 invoke lstrcmpA,selectedVersion,original
 check eax,0
 ; Post-normalization child paths/metadata/libraries must succeed before done.
 mov [engineCancel],0
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall FixtureInstallReset
 mov [failChildStage],1
 stdcall VersionQueueRun,0
 check_false
 check [validateCalls],1
 check [buildPathsCalls],0
 check [metadataCalls],0
 check [libraryCalls],0
 check [cleanupCalls],2
 check [versionQueue+VQ_STATE],3
 check [postCalls],1
 invoke lstrcmpA,selectedVersion,original
 check eax,0
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall FixtureInstallReset
 mov [failChildStage],2
 stdcall VersionQueueRun,0
 check_false
 check [validateCalls],1
 check [buildPathsCalls],1
 check [metadataCalls],1
 check [libraryCalls],0
 check [cleanupCalls],2
 check [versionQueue+VQ_STATE],3
 check [postCalls],1
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall VersionQueueAdd,base2,0,0
 stdcall FixtureInstallReset
 mov [failChildStage],3
 stdcall VersionQueueRun,0
 check_false
 check [libraryCalls],1
 check [cleanupCalls],3
 check [versionQueue+VQ_STATE],3
 check [versionQueue+VQ_SIZE+VQ_STATE],2
 check [postCalls],1
 invoke lstrcmpA,selectedVersion,original
 check eax,0
 ; Cancellation during child library/native extraction marks only active row.
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,base1,1,loader1
 stdcall VersionQueueAdd,base2,0,0
 stdcall FixtureInstallReset
 mov [cancelStage],4
 stdcall VersionQueueRun,0
 check_false
 check [libraryCalls],1
 check [cleanupCalls],2
 check [versionQueue+VQ_STATE],4
 check [versionQueue+VQ_SIZE+VQ_STATE],0
 check [versionQueueIndex],-1
 check [postCalls],1
 invoke lstrcmpA,selectedVersion,original
 check eax,0
 check [responseCount],0
 check [contextCount],0
 check [fixtureBad],0
 invoke ExitProcess,0
failed:
 mov eax,[testNo]
 add eax,100
 invoke ExitProcess,eax

proc SetStatusA,text
 invoke lstrcpyA,lastStatus,[text]
 ret
endp

proc FixtureFetchReset
 mov [requestCalls],0
 mov [engineCancel],0
 ret
endp

proc FixtureRequest uses ebx esi edi,url,method,headers,body
 inc [requestCalls]
 cmp [requestCalls],2
 ja .bad
 mov eax,vManifestMirror
 cmp [requestCalls],1
 je .compare
 mov eax,vManifestOfficial
.compare:
 invoke lstrcmpiW,[url],eax
 test eax,eax
 jnz .bad
 mov [httpStatus],200
 mov esi,goodJson
 cmp [requestMode],3
 je .broken
 cmp [requestCalls],1
 jne .allocate
 cmp [requestMode],1
 je .broken
 cmp [requestMode],2
 je .denied
 cmp [requestMode],4
 je .empty
 cmp [requestMode],5
 jne .transport
 mov [engineCancel],1
 jmp .allocate
.transport:
 cmp [requestMode],6
 je .zero
 jmp .allocate
.denied:
 mov [httpStatus],403
 jmp .allocate
.empty:
 mov esi,emptyJson
 jmp .allocate
.broken:
 mov esi,badJson
.allocate:
 invoke lstrlenA,esi
 inc eax
 mov ebx,eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,8,ebx
 test eax,eax
 jz .bad
 mov edi,eax
 invoke lstrcpyA,edi,esi
 cmp [responsePointer],0
 jne .bad
 mov [responsePointer],edi
 inc [responseCount]
 mov eax,edi
 ret
.bad:
 mov [fixtureBad],1
.zero:
 xor eax,eax
 ret
endp

proc FixtureFree,pointerArg
 cmp [pointerArg],0
 je .done
 mov eax,[pointerArg]
 cmp eax,[responsePointer]
 jne .bad
 dec [responseCount]
 mov [responsePointer],0
 stdcall FreeMem,[pointerArg]
.done:
 ret
.bad:
 mov [fixtureBad],1
 ret
endp

proc FixtureJsonParse,source
 stdcall JsonParse,[source]
 test eax,eax
 jz .done
 inc [contextCount]
.done:
 ret
endp
proc FixtureJsonFree,ctx
 cmp [ctx],0
 je .done
 dec [contextCount]
 stdcall JsonFree,[ctx]
.done:
 ret
endp

proc FixtureInstallReset
 mov [engineCancel],0
 mov [postCalls],0
 mov [installCalls],0
 mov [cleanupCalls],0
 mov [loaderCalls],0
 mov [normalizeCalls],0
 mov [validateCalls],0
 mov [buildPathsCalls],0
 mov [metadataCalls],0
 mov [libraryCalls],0
 mov [failInstallAt],0
 mov [failLoader],0
 mov [failNormalize],0
 mov [failChildStage],0
 mov [cancelStage],0
 ret
endp
proc FixtureThreeRows
 stdcall VersionQueueAdd,base1,0,0
 stdcall VersionQueueAdd,base2,0,0
 stdcall VersionQueueAdd,base1,1,loader1
 ret
endp

proc EInstallCore
 inc [installCalls]
 cmp [cancelStage],1
 jne .result
 mov [engineCancel],1
.result:
 mov eax,[installCalls]
 cmp eax,[failInstallAt]
 setne al
 movzx eax,al
 ; Deliberately violate stdcall to exercise queue's installer-boundary guard.
 mov ebx,0DEADBEEFh
 mov esi,0BADF00Dh
 ret
endp
proc ECleanup
 inc [cleanupCalls]
 ret
endp
proc LoaderInstall,baseArg,kind,versionArg,targetArg
 inc [loaderCalls]
 cmp [kind],1
 jne .bad
 invoke lstrcmpA,[versionArg],loader1
 test eax,eax
 jnz .bad
 invoke lstrcmpA,[baseArg],base1
 test eax,eax
 jnz .bad
 invoke lstrcpyA,[targetArg],installedTarget
 cmp [cancelStage],2
 jne .result
 mov [engineCancel],1
.result:
 xor eax,eax
 cmp [failLoader],0
 sete al
 ret
.bad:
 mov [fixtureBad],1
 xor eax,eax
 ret
endp
proc ProfileNormalize,targetArg,baseArg
 inc [normalizeCalls]
 invoke lstrcmpA,[targetArg],installedTarget
 test eax,eax
 jnz .bad
 invoke lstrcmpA,[baseArg],base1
 test eax,eax
 jnz .bad
 cmp [cancelStage],3
 jne .result
 mov [engineCancel],1
.result:
 xor eax,eax
 cmp [failNormalize],0
 sete al
 ret
.bad:
 mov [fixtureBad],1
 xor eax,eax
 ret
endp
proc FixtureChildSelected
 invoke lstrcmpA,selectedVersion,installedTarget
 test eax,eax
 jz .good
 mov [fixtureBad],1
 xor eax,eax
 ret
.good:
 mov eax,1
 ret
endp
proc EValidateVersion
 inc [validateCalls]
 stdcall FixtureChildSelected
 test eax,eax
 jz .done
 xor eax,eax
 cmp [failChildStage],1
 setne al
.done:
 ret
endp
proc EBuildPaths
 inc [buildPathsCalls]
 stdcall FixtureChildSelected
 ret
endp
proc ELoadMetadata
 inc [metadataCalls]
 stdcall FixtureChildSelected
 test eax,eax
 jz .done
 xor eax,eax
 cmp [failChildStage],2
 setne al
.done:
 ret
endp
proc EInstallLibraries
 inc [libraryCalls]
 stdcall FixtureChildSelected
 test eax,eax
 jz .done
 cmp [cancelStage],4
 jne .result
 mov [engineCancel],1
.result:
 xor eax,eax
 cmp [failChildStage],3
 setne al
.done:
 ret
endp
proc FixturePost,window,message,wparam,lparam
 inc [postCalls]
 cmp [message],WM_APP+2
 jne .bad
 cmp [wparam],0
 jne .bad
 cmp [lparam],0
 jne .bad
 mov eax,1
 ret
.bad:
 mov [fixtureBad],1
 xor eax,eax
 ret
endp

include '..\src\common.inc'
define HttpRequest FixtureRequest
define FreeMem FixtureFree
define JsonParse FixtureJsonParse
define JsonFree FixtureJsonFree
define PostMessageW fixturePostImport
include '..\src\versions.inc'
restore PostMessageW
restore JsonFree
restore JsonParse
restore FreeMem
restore HttpRequest
include '..\src\imports.inc'
