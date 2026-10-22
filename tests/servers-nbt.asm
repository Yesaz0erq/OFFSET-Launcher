; Genuine binary NBT, Modified UTF-8, bounds, staging, and Unicode file paths.
format PE console 6.0
entry start
stack 200000h,80000h
include 'win32ax.inc'
include '..\src\servers_types.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Probe',0,58 dup(0)
selectedVersion db 'fixture',0,120 dup(0)
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
testNo dd 1
testIndex dd 0
fixtureContext du 'tests\fixtures\servers-nbt\'
 dw 8DEFh,5F84h,0
fixtureMissing du 'tests\out\servers-nbt-missing',0
fixtureLarge du 'tests\fixtures\servers-nbt\oversized',0
fixtureIoFormat du 'tests\out\servers-nbt-%u',0
fixtureServerFile du '\servers.dat',0
fixtureId db 'fixture',0
fixtureBadId db '../unsafe',0
fixtureExpected dw 672Ch,5730h,32,0D83Ch,0DF0Dh,0
fixtureAddress du '[::1]:25565',0
fixtureSecond du 'Second',0
fixtureOld du 'Keep old list',0
fixturePath rw 2048
fixtureData rw 2048
fixtureExpectedPath rw 2048
fixtureIoRoot rw 2048
fixtureIoPath rw 2048
fixtureName rb 64
testTable dd nbtFixture2,nbtFixture2End-nbtFixture2,SV_ERR_NBT
 dd nbtFixture3,nbtFixture3End-nbtFixture3,SV_ERR_NBT
 dd nbtFixture4,nbtFixture4End-nbtFixture4,SV_ERR_LIMIT
 dd nbtFixture5,nbtFixture5End-nbtFixture5,SV_ERR_TEXT
 dd nbtFixture6,nbtFixture6End-nbtFixture6,SV_ERR_TEXT
 dd nbtFixture7,nbtFixture7End-nbtFixture7,SV_ERR_TEXT
 dd nbtFixture8,nbtFixture8End-nbtFixture8,SV_ERR_TEXT
 dd nbtFixture9,nbtFixture9End-nbtFixture9,SV_ERR_NBT
 dd nbtFixture10,nbtFixture10End-nbtFixture10,SV_ERR_NBT
 dd nbtFixture11,nbtFixture11End-nbtFixture11,SV_ERR_NBT
 dd nbtFixture12,nbtFixture12End-nbtFixture12,SV_ERR_NBT
 dd nbtFixture13,nbtFixture13End-nbtFixture13,SV_ERR_NBT
 dd nbtFixture14,nbtFixture14End-nbtFixture14,SV_ERR_NBT
testTableEnd:
include 'fixtures\servers-nbt\fixtures.inc'
section '.text' code readable executable
start:
 stdcall EngineInit
 stdcall SnParse,nbtFixture0,nbtFixture0End-nbtFixture0
 cmp eax,1
 jne failed
 cmp [snCount],2
 jne failed
 invoke lstrcmpiW,snRows+SV_NAME,fixtureExpected
 test eax,eax
 jnz failed
 invoke lstrcmpiW,snRows+SV_ADDRESS,fixtureAddress
 test eax,eax
 jnz failed
 invoke lstrcmpiW,snRows+SV_SIZE+SV_NAME,fixtureSecond
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall SnParse,nbtFixture1,nbtFixture1End-nbtFixture1
 cmp eax,1
 jne failed
 cmp [snCount],0
 jne failed
 inc [testNo]
 mov ebx,testTable
.badLoop:
 cmp ebx,testTableEnd
 jae .cancel
 stdcall SnParse,dword [ebx],dword [ebx+4]
 test eax,eax
 jnz failed
 mov eax,[ebx+8]
 cmp eax,[snError]
 jne failed
 add ebx,12
 inc [testNo]
 jmp .badLoop
.cancel:
 stdcall SnParse,nbtFixture15,nbtFixture15End-nbtFixture15
 cmp eax,1
 jne failed
 cmp [snCount],1
 jne failed
 invoke lstrlenW,snRows+SV_NAME
 cmp eax,127
 jne failed
 invoke lstrlenW,snRows+SV_ADDRESS
 cmp eax,255
 jne failed
 inc [testNo]
 stdcall SnParse,nbtFixture16,nbtFixture16End-nbtFixture16
 cmp eax,1
 jne failed
 cmp [snCount],256
 jne failed
 inc [testNo]
 mov [engineCancel],1
 stdcall SnParse,nbtFixture0,nbtFixture0End-nbtFixture0
 test eax,eax
 jnz failed
 cmp [snError],SV_ERR_CANCEL
 jne failed
 mov [engineCancel],0
 inc [testNo]
 ; VsResolveData must honor the version's isolation setting and Unicode path.
 invoke GetFullPathNameW,fixtureContext,2048,fixturePath,0
 test eax,eax
 jz failed
 stdcall ServersLoad,fixturePath,fixtureId
 cmp eax,1
 jne failed
 cmp [serversCount],2
 jne failed
 cmp [serversLastError],0
 jne failed
 invoke lstrcmpiW,serversRows+SV_NAME,fixtureExpected
 test eax,eax
 jnz failed
 invoke lstrcpyW,fixtureExpectedPath,fixturePath
 invoke lstrcatW,fixtureExpectedPath,wideDataSuffix
 invoke lstrcmpiW,serversDataDirW,fixtureExpectedPath
 test eax,eax
 jnz failed
 inc [testNo]
 ; Cancellation in the same context preserves the last fully valid list.
 mov [engineCancel],1
 stdcall ServersLoad,fixturePath,fixtureId
 test eax,eax
 jnz failed
 cmp [serversLastError],SV_ERR_CANCEL
 jne failed
 cmp [serversCount],2
 jne failed
 invoke lstrcmpiW,serversRows+SV_NAME,fixtureExpected
 test eax,eax
 jnz failed
 mov [engineCancel],0
 inc [testNo]
 ; Reading a malformed saved file in the same context must not publish half
 ; a parsed list or discard the earlier fully validated list.
 invoke GetTickCount
 cinvoke wsprintfW,fixturePath,fixtureIoFormat,eax
 invoke GetFullPathNameW,fixturePath,2048,fixtureIoRoot,0
 stdcall EnsureDirectory,fixtureIoRoot
 invoke lstrcpyW,fixtureIoPath,fixtureIoRoot
 invoke lstrcatW,fixtureIoPath,fixtureServerFile
 stdcall FixtureWrite,fixtureIoPath,nbtFixture0,nbtFixture0End-nbtFixture0
 test eax,eax
 jz failed
 stdcall ServersLoad,fixtureIoRoot,fixtureId
 cmp eax,1
 jne failed
 cmp [serversCount],2
 jne failed
 stdcall FixtureWrite,fixtureIoPath,nbtFixture2,nbtFixture2End-nbtFixture2
 test eax,eax
 jz failed
 stdcall ServersLoad,fixtureIoRoot,fixtureId
 test eax,eax
 jnz failed
 cmp [serversLastError],SV_ERR_NBT
 jne failed
 cmp [serversCount],2
 jne failed
 invoke lstrcmpiW,serversRows+SV_NAME,fixtureExpected
 test eax,eax
 jnz failed
 inc [testNo]
 ; The file-size bound is checked before allocating/reading its bytes, and
 ; a different context cannot show the previous context's cached servers.
 invoke GetFullPathNameW,fixtureLarge,2048,fixturePath,0
 stdcall ServersLoad,fixturePath,fixtureId
 test eax,eax
 jnz failed
 cmp [serversLastError],SV_ERR_LIMIT
 jne failed
 cmp [serversCount],0
 jne failed
 invoke lstrcmpiW,serversDataDirW,fixturePath
 test eax,eax
 jnz failed
 inc [testNo]
 ; Missing servers.dat in a different valid context publishes an empty list.
 invoke GetFullPathNameW,fixtureMissing,2048,fixturePath,0
 test eax,eax
 jz failed
 stdcall EnsureDirectory,fixturePath
 stdcall ServersLoad,fixturePath,fixtureId
 cmp eax,1
 jne failed
 cmp [serversCount],0
 jne failed
 cmp [serversLastError],0
 jne failed
 inc [testNo]
 mov [serversCount],1
 stdcall ServersLoad,fixturePath,fixtureBadId
 test eax,eax
 jnz failed
 cmp [serversCount],0
 jne failed
 cmp [serversLastError],SV_ERR_PATH
 jne failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[testNo]
proc SetStatusA,text
 ret
endp
proc FixtureWrite uses ebx,path,buffer,length
 local written:DWORD,result:DWORD
 mov [result],0
 invoke CreateFileW,[path],GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .return
 mov ebx,eax
 invoke WriteFile,ebx,[buffer],[length],addr written,0
 test eax,eax
 jz .close
 mov eax,[length]
 cmp eax,[written]
 jne .close
 mov [result],1
.close:
 invoke CloseHandle,ebx
.return:
 mov eax,[result]
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\version_settings.inc'
include '..\src\servers_nbt.inc'
include '..\src\imports.inc'
section '.enddata' data readable writeable
wideDataSuffix du '\versions\fixture\game',0
