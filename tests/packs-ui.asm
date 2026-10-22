; Isolated native pack-browser regression. No visible UI, network or user data.
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
saved dd 0
lastPage dd 9
testStep dd 1
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
gameDir rw 1024
javaPath rw 1024
selectedVersion db 'old',0,124 dup(0)
playerName db 'Player',0,57 dup(0)
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
iniPath rw 1024
emptyW dw 0
listClass du 'LISTBOX',0
editClass du 'EDIT',0
comboClass du 'COMBOBOX',0
baseRelative du 'tests\out\pack-ui-native',0
validSuffix du '\valid',0
badSuffix du '\bad-nul',0
brokenSuffix du '\broken',0
versionsSuffix du '\versions',0
childSuffix du '\1.21.1',0
jsonSuffix du '\1.21.1.json',0
jarSuffix du '\1.21.1.jar',0
brokenName du 'Broken',0
longName dw 50 dup(6574h),0
validMarker db '{"complete":true,"name":"',50 dup('整'),'","version":"1.21.1"}',0
badMarker db '{"complete":true,"name":"bad","version":"1.21.1\u0000-alias"}',0
brokenMarker db '{"complete":true,"name":"Broken","version":"1.21.1"}',0
metadata db '{"id":"1.21.1","arguments":{"jvm":[],"game":[]},"mainClass":"example.Main"}',0
dummyJar db 'fixture',0
expectedVersion db '1.21.1',0
path rw 2048
validDir rw 1024
clientPath rw 2048
section '.text' code readable executable
start:
 invoke GetProcessHeap
 mov [eHeap],eax
 invoke CreateDirectoryW,testOut,0
 invoke CreateDirectoryW,baseRelative,0
 invoke GetFullPathNameW,baseRelative,1024,packRoot,0
 test eax,eax
 jz failed
 invoke CreateWindowExW,0,listClass,emptyW,WS_POPUP,0,0,300,300,0,0,0,0
 mov [hPackLocal],eax
 test eax,eax
 jz failed
 invoke CreateWindowExW,0,comboClass,emptyW,WS_POPUP+CBS_DROPDOWN,0,0,300,300,0,0,0,0
 mov [hVersion],eax
 invoke CreateWindowExW,0,editClass,emptyW,WS_POPUP,0,0,300,300,0,0,0,0
 mov [hGameDir],eax
 invoke lstrcpyW,validDir,packRoot
 invoke lstrcatW,validDir,validSuffix
 invoke CreateDirectoryW,validDir,0
 invoke lstrcpyW,path,validDir
 invoke lstrcatW,path,packMarkerSuffix
 stdcall Put,path,validMarker
 invoke lstrcpyW,path,packRoot
 invoke lstrcatW,path,badSuffix
 invoke CreateDirectoryW,path,0
 invoke lstrcatW,path,packMarkerSuffix
 stdcall Put,path,badMarker
 invoke lstrcpyW,path,packRoot
 invoke lstrcatW,path,brokenSuffix
 invoke CreateDirectoryW,path,0
 invoke lstrcatW,path,packMarkerSuffix
 stdcall Put,path,brokenMarker
 invoke lstrcpyW,path,validDir
 invoke lstrcatW,path,versionsSuffix
 invoke CreateDirectoryW,path,0
 invoke lstrcatW,path,childSuffix
 invoke CreateDirectoryW,path,0
 invoke lstrcpyW,clientPath,path
 invoke lstrcatW,clientPath,jarSuffix
 stdcall Put,clientPath,dummyJar
 invoke lstrcatW,path,jsonSuffix
 stdcall Put,path,metadata
 stdcall PackScanLocal
 cmp [packLocalCount],2
 jne failed
 invoke SendMessageW,[hPackLocal],LB_FINDSTRINGEXACT,-1,longName
 cmp eax,-1
 je failed
 mov [testStep],2
 invoke SendMessageW,[hPackLocal],LB_FINDSTRINGEXACT,-1,brokenName
 cmp eax,-1
 je failed
 invoke SendMessageW,[hPackLocal],LB_SETCURSEL,eax,0
 stdcall PackActivateSelected
 cmp [saved],0
 jne failed
 cmp [lastPage],9
 jne failed
 mov [testStep],3
 invoke SendMessageW,[hPackLocal],LB_FINDSTRINGEXACT,-1,longName
 invoke SendMessageW,[hPackLocal],LB_SETCURSEL,eax,0
 stdcall PackActivateSelected
 cmp [saved],1
 jne failed
 cmp [lastPage],0
 jne failed
 invoke lstrcmpiW,gameDir,validDir
 test eax,eax
 jnz failed
 invoke lstrcmpA,selectedVersion,expectedVersion
 test eax,eax
 jnz failed
 mov [testStep],4
 mov [saved],0
 mov [lastPage],9
 invoke DeleteFileW,clientPath
 stdcall PackActivateSelected
 cmp [saved],0
 jne failed
 cmp [lastPage],9
 jne failed
 stdcall Put,clientPath,dummyJar
 mov [testStep],5
 stdcall PackUiDirectory,clientPath
 test eax,eax
 jnz failed
 stdcall PackUiRegularFile,validDir
 test eax,eax
 jnz failed
 mov [testStep],6
 mov edi,iniPath
 mov ecx,995
 mov ax,'A'
 rep stosw
 mov word [edi],0
 stdcall PackUiInit
 cmp word [packRoot],0
 jne failed
 cmp word [packCacheRoot],0
 jne failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testStep]
proc Put uses ebx,filename,text
 local written:DWORD
 invoke CreateFileW,[filename],GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je failed
 mov ebx,eax
 invoke lstrlenA,[text]
 invoke WriteFile,ebx,[text],eax,addr written,0
 invoke CloseHandle,ebx
 ret
endp
proc PullSettings
 mov eax,1
 ret
endp
proc SaveSettings
 inc [saved]
 mov eax,1
 ret
endp
proc GoPage,page
 mov eax,[page]
 mov [lastPage],eax
 ret
endp
proc SetStatusA,text
 ret
endp
proc EngineScanVersions,combo
 xor eax,eax
 ret
endp
testOut du 'tests\out',0
include '..\src\common.inc'
; Only EStandaloneProfile and EReadFile are linked from the real engine.
define EngineScanVersions UnusedEngineScanVersions
include '..\src\engine.inc'
restore EngineScanVersions
include '..\src\versions.inc'
include '..\src\packs_ui.inc'
include '..\src\imports.inc'
