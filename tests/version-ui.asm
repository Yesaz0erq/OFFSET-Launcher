; Native hidden-list regression for atomic multi-selection queue admission.
; No network, visible windows, user files, or tests/out source dependencies.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\version_types.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
hVersion dd 0
busy dd 0
currentPage dd 1
loaderVersionCount dd 0
loaderVersions rb VL_SIZE*VL_CAPACITY
selectedVersion db '1.21.1',0,121 dup(0)
gameDir rw 1024
statusText rw 1024
emptyW dw 0
emptyA db 0
testListClass du 'LISTBOX',0
testOne db '1.21.1',0
testTwo db '1.21.2',0
testOneW du '1.21.1',0
testTwoW du '1.21.2',0
testSample db '{}',0
testOtherFmt db 'queue-%u',0
testOther rb 128
testStep dd 1
testSnapshot rb VQ_SIZE*VQ_CAPACITY
include '..\src\strings.inc'
section '.text' code readable executable
start:
 stdcall JsonParse,testSample
 stdcall JsonFree,eax
 invoke CreateWindowExW,0,testListClass,emptyW,WS_POPUP+LBS_EXTENDEDSEL,0,0,300,300,0,0,0,0
 test eax,eax
 jz failed
 mov [hVCList],eax
 mov [hWnd],eax
 invoke CreateWindowExW,0,testListClass,emptyW,WS_POPUP,0,0,300,300,0,0,0,0
 test eax,eax
 jz failed
 mov [hVQueue],eax
 invoke SendMessageW,[hVCList],LB_ADDSTRING,0,testOneW
 invoke SendMessageW,[hVCList],LB_ADDSTRING,0,testTwoW
 invoke SendMessageW,[hVCList],LB_SETSEL,1,-1
 mov [vUiVisible],2
 mov [vUiMap],0
 mov [vUiMap+4],1
 invoke lstrcpyA,versionRecords,testOne
 invoke lstrcpyA,versionRecords+VC_SIZE,testTwo
 ; 63 occupied + 2 new: fail before even the first row is modified.
 stdcall FillOther,63
 mov esi,versionQueue
 mov edi,testSnapshot
 mov ecx,VQ_SIZE*VQ_CAPACITY/4
 rep movsd
 stdcall VersionAddSelected
 test eax,eax
 jnz failed
 cmp [versionQueueCount],63
 jne failed
 mov esi,versionQueue
 mov edi,testSnapshot
 mov ecx,VQ_SIZE*VQ_CAPACITY
 repe cmpsb
 jne failed
 invoke lstrcmpiW,statusText,sVQueueLimit
 test eax,eax
 jnz failed
 ; An existing completed item takes no new slot and is only reset on commit.
 inc [testStep]
 stdcall VersionQueueClear
 stdcall VersionQueueAdd,testOne,0,emptyA
 mov dword [versionQueue+VQ_STATE],2
 stdcall FillOther,62
 stdcall VersionAddSelected
 cmp eax,1
 jne failed
 cmp [versionQueueCount],64
 jne failed
 cmp dword [versionQueue+VQ_STATE],0
 jne failed
 invoke lstrcmpA,versionQueue+63*VQ_SIZE,testTwo
 test eax,eax
 jnz failed
 ; A completely full queue accepts two duplicate combinations.
 inc [testStep]
 mov dword [versionQueue+VQ_STATE],3
 mov dword [versionQueue+63*VQ_SIZE+VQ_STATE],4
 stdcall VersionAddSelected
 cmp eax,1
 jne failed
 cmp [versionQueueCount],64
 jne failed
 cmp dword [versionQueue+VQ_STATE],0
 jne failed
 cmp dword [versionQueue+63*VQ_SIZE+VQ_STATE],0
 jne failed
 ; Two selected duplicate catalogue rows reserve a single new slot.
 inc [testStep]
 stdcall VersionQueueClear
 stdcall FillOther,63
 mov [vUiMap+4],0
 stdcall VersionAddSelected
 cmp eax,1
 jne failed
 cmp [versionQueueCount],64
 jne failed
 invoke DestroyWindow,[hVCList]
 invoke DestroyWindow,[hVQueue]
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testStep]
proc FillOther uses esi,count
 xor esi,esi
.loop:
 cmp esi,[count]
 jae .done
 cinvoke wsprintfA,testOther,testOtherFmt,esi
 stdcall VersionQueueAdd,testOther,0,emptyA
 inc esi
 jmp .loop
.done:
 ret
endp
proc SetStatusA,text
 ret
endp
include '..\src\common.inc'
include '..\src\versions.inc'
include '..\src\versions_ui.inc'
include '..\src\imports.inc'
