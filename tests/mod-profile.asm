; Real isolated profile files and JSON, mocked native controls.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\version_types.inc'
WM_APP=8000h
macro check value,expected {inc [testNo]
 cmp value,expected
 jne failed}
macro same left,right {inc [testNo]
 invoke lstrcmpA,left,right
 test eax,eax
 jnz failed}
section '.data' data readable writeable
busy dd 0
engineCancel dd 0
hWnd dd 0
hModGame dd 101
hModLoader dd 102
testNo dd 0
fixtureBad dd 0
readCalls dd 0
textCalls dd 0
comboCalls dd 0
clearCalls dd 0
uiIndex dd 3
textImport dd FixtureSetText
messageImport dd FixtureMessage
fixtureDir du 'tests\out\mod-profile-fixture',0
fixtureDir2 du 'tests\out\mod-profile-fixture-2',0
versionsDir du '\versions\',0
fileSlash du '\',0
jsonExt du '.json',0
manualGame db '1.19.4',0
fabricName db 'fabric',0
forgeName db 'forge',0
neoName db 'neoforge',0
quiltName db 'quilt',0
userQuery db 'sodium fabric',0
mc121 db '1.21.1',0
mc120 db '1.20.1',0
mc1206 db '1.20.6',0
snapshotId db '24w33a',0
fabricId db 'fabric-loader-0.16.10-1.21.1',0
forgeId db '1.20.1-forge-47.4.10',0
neoId db 'neoforge-21.1.219',0
quiltId db 'quilt-loader-0.29.0-1.21.1',0
missingId db 'fabric-loader-missing',0
badJsonId db 'bad-json',0
badBaseId db 'fabric-invalid-base',0
longBaseId db 'fabric-long-base',0
nulBaseId db 'fabric-nul-base',0
mismatchId db 'mismatch',0
rawId db 'fabric-raw',0
partialId db 'fabric-partial',0
customId db 'custom-profile',0
falseId db 'false-normalized',0
badCoordinateId db 'bad-coordinate',0
longCoordinateId db 'long-coordinate',0
traversalId db '../escape',0
reservedId db 'CON',0
sharedId db 'shared-profile',0
fabricJson db '{"id":"fabric-loader-0.16.10-1.21.1","offsetNormalized":true,"offsetBase":"1.21.1","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
forgeJson db '{"id":"1.20.1-forge-47.4.10","offsetNormalized":true,"offsetBase":"1.20.1","libraries":[{"name":"net.minecraftforge:forge:1.20.1-47.4.10:universal"}]}',0
neoJson db '{"id":"neoforge-21.1.219","offsetNormalized":true,"offsetBase":"1.21.1","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"},{"name":"net.minecraftforge:forge:1.21.1-52.1.0"},{"name":"net.neoforged:neoforge:21.1.219"}]}',0
quiltJson db '{"id":"quilt-loader-0.29.0-1.21.1","offsetNormalized":true,"offsetBase":"1.21.1","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"},{"name":"org.quiltmc:quilt-loader:0.29.0"}]}',0
vanillaJson db '{"id":"1.21.1","libraries":[]}',0
snapshotJson db '{"id":"24w33a","libraries":[]}',0
badJson db '{"id":invalid}',0
badBaseJson db '{"id":"fabric-invalid-base","offsetNormalized":true,"offsetBase":"1.21.1../escape","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
longBaseJson db '{"id":"fabric-long-base","offsetNormalized":true,"offsetBase":"1.',70 dup('1'),'","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
nulBaseJson db '{"id":"fabric-nul-base","offsetNormalized":true,"offsetBase":"1.21.1\u0000ignored","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
mismatchJson db '{"id":"another-profile","offsetNormalized":true,"offsetBase":"1.21.1","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
rawJson db '{"id":"fabric-raw","inheritsFrom":"1.21.1","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
partialJson db '{"id":"fabric-partial","offsetNormalized":true,"offsetBase":"1.21.1","inheritsFrom":"1.21.1","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
customJson db '{"id":"custom-profile","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
falseJson db '{"id":"false-normalized","offsetNormalized":false,"offsetBase":"1.21.1","libraries":[{"name":"net.fabricmc:fabric-loader:0.16.10"}]}',0
badCoordinateJson db '{"id":"bad-coordinate","offsetNormalized":true,"offsetBase":"1.20.6","libraries":[{"name":"net.fabricmc:fabric-loader:\u0000junk"},{"name":"net.fabricmc:fabric-loader.evil:0.16.10"}]}',0
longCoordinateJson db '{"id":"long-coordinate","offsetNormalized":true,"offsetBase":"1.20.6","libraries":[{"name":"net.fabricmc:fabric-loader:',550 dup('x'),'"}]}',0
sharedForgeJson db '{"id":"shared-profile","offsetNormalized":true,"offsetBase":"1.20.1","libraries":[{"name":"net.minecraftforge:forge:1.20.1-47.4.10"}]}',0
sharedNeoJson db '{"id":"shared-profile","offsetNormalized":true,"offsetBase":"1.21.1","libraries":[{"name":"net.neoforged:neoforge:21.1.219"}]}',0
fixturePairs dd fabricId,fabricJson,forgeId,forgeJson,neoId,neoJson,quiltId,quiltJson
 dd mc121,vanillaJson,snapshotId,snapshotJson,badJsonId,badJson,badBaseId,badBaseJson
 dd longBaseId,longBaseJson,nulBaseId,nulBaseJson,mismatchId,mismatchJson,rawId,rawJson
 dd partialId,partialJson,customId,customJson,falseId,falseJson
 dd badCoordinateId,badCoordinateJson,longCoordinateId,longCoordinateJson
 dd sharedId,sharedForgeJson
fixturePairCount=($-fixturePairs)/8
invalidProfiles dd missingId,badJsonId,badBaseId,longBaseId,nulBaseId,mismatchId,rawId,partialId,customId,falseId
invalidProfileCount=($-invalidProfiles)/4
gameDir rw 1024
firstGameDir rw 1024
secondGameDir rw 1024
selectedVersion rb 128
modGameVersion rb 64
modLoader rb 16
modQuery rb 512
uiText rb 64
fixturePath rw 2048
fixtureName rw 128

section '.text' code readable executable
start:
 invoke GetFullPathNameW,fixtureDir,1024,gameDir,0
 test eax,eax
 jz failed
 invoke lstrcpyW,firstGameDir,gameDir
 invoke GetFullPathNameW,fixtureDir2,1024,secondGameDir,0
 test eax,eax
 jz failed
 mov esi,fixturePairs
 mov edi,fixturePairCount
.write:
 stdcall FixtureWrite,dword [esi],dword [esi+4]
 test eax,eax
 jz failed
 add esi,8
 dec edi
 jnz .write
 invoke lstrcpyW,gameDir,secondGameDir
 stdcall FixtureWrite,sharedId,sharedNeoJson
 test eax,eax
 jz failed
 invoke lstrcpyW,gameDir,firstGameDir
 stdcall FixtureManual
 stdcall FixtureSelected,fabricId
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc121
 same uiText,mc121
 same modLoader,fabricName
 check [uiIndex],0
 check [readCalls],1
 check [textCalls],1
 check [comboCalls],1
 check [clearCalls],1
 ; Manual filters survive every later visit for this same selected profile.
 stdcall FixtureManual
 stdcall ModSyncSelectedProfile
 same modGameVersion,manualGame
 same uiText,manualGame
 same modLoader,quiltName
 check [uiIndex],3
 check [readCalls],1
 check [textCalls],0
 check [comboCalls],0
 check [clearCalls],0
 ; A busy call does not consume the next profile-change synchronization.
 stdcall FixtureSelected,forgeId
 mov [busy],1
 stdcall ModSyncSelectedProfile
 same modGameVersion,manualGame
 check [readCalls],0
 check [clearCalls],0
 mov [busy],0
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc120
 same modLoader,forgeName
 check [uiIndex],1
 check [clearCalls],1
 stdcall FixtureManual
 stdcall FixtureSelected,neoId
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc121
 same modLoader,neoName
 check [uiIndex],2
 check [clearCalls],1
 stdcall FixtureManual
 stdcall FixtureSelected,quiltId
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc121
 same modLoader,quiltName
 check [uiIndex],3
 check [clearCalls],1
 ; Vanilla and snapshots update only the game version, preserving loader choice.
 stdcall FixtureManual
 stdcall FixtureSelected,mc121
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc121
 same modLoader,quiltName
 check [uiIndex],3
 check [comboCalls],0
 check [clearCalls],1
 stdcall FixtureManual
 stdcall FixtureSelected,snapshotId
 stdcall ModSyncSelectedProfile
 same modGameVersion,snapshotId
 same modLoader,quiltName
 check [comboCalls],0
 check [clearCalls],1
 ; Missing, malformed, inherited, mismatched and unsafe base metadata preserve UI.
 mov esi,invalidProfiles
 mov edi,invalidProfileCount
.invalid:
 stdcall FixtureManual
 stdcall FixtureSelected,dword [esi]
 stdcall ModSyncSelectedProfile
 same modGameVersion,manualGame
 same uiText,manualGame
 same modLoader,quiltName
 check [uiIndex],3
 check [textCalls],0
 check [comboCalls],0
 check [clearCalls],0
 add esi,4
 dec edi
 jnz .invalid
 ; Never turn a truncated or embedded-NUL Maven prefix into a loader match.
 stdcall FixtureManual
 stdcall FixtureSelected,badCoordinateId
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc1206
 same modLoader,quiltName
 check [comboCalls],0
 check [clearCalls],1
 stdcall FixtureManual
 stdcall FixtureSelected,longCoordinateId
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc1206
 same modLoader,quiltName
 check [comboCalls],0
 check [clearCalls],1
 ; Invalid selected IDs never cause a file access or a query/filter change.
 stdcall FixtureManual
 stdcall FixtureSelected,traversalId
 stdcall ModSyncSelectedProfile
 check [readCalls],0
 check [clearCalls],0
 same modGameVersion,manualGame
 stdcall FixtureSelected,reservedId
 stdcall ModSyncSelectedProfile
 check [readCalls],0
 check [clearCalls],0
 same modGameVersion,manualGame
 ; Root directory is part of the profile identity, even with the same name.
 stdcall FixtureManual
 stdcall FixtureSelected,sharedId
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc120
 same modLoader,forgeName
 check [uiIndex],1
 check [readCalls],1
 check [clearCalls],1
 stdcall FixtureManual
 stdcall FixtureSelected,sharedId
 invoke lstrcpyW,gameDir,secondGameDir
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc121
 same modLoader,neoName
 check [uiIndex],2
 check [readCalls],1
 check [clearCalls],1
 stdcall FixtureManual
 stdcall ModSyncSelectedProfile
 same modGameVersion,manualGame
 same modLoader,quiltName
 check [readCalls],1
 check [textCalls],0
 check [clearCalls],0
 ; Busy root changes are applied only when idle, never consuming the cache key.
 invoke lstrcpyW,gameDir,firstGameDir
 mov [busy],1
 stdcall ModSyncSelectedProfile
 same modGameVersion,manualGame
 check [readCalls],1
 mov [busy],0
 stdcall ModSyncSelectedProfile
 same modGameVersion,mc120
 same modLoader,forgeName
 check [readCalls],2
 check [clearCalls],1
 ; An empty game root must not read a drive-root versions directory.
 stdcall FixtureManual
 mov word [gameDir],0
 mov [readCalls],0
 stdcall ModSyncSelectedProfile
 check [readCalls],0
 check [clearCalls],0
 same modGameVersion,manualGame
 invoke lstrcpyW,gameDir,firstGameDir
 same modQuery,userQuery
 check [fixtureBad],0
 invoke ExitProcess,0
failed:
 mov eax,[testNo]
 add eax,100
 invoke ExitProcess,eax

proc FixtureManual
 invoke lstrcpyA,modGameVersion,manualGame
 invoke lstrcpyA,uiText,manualGame
 invoke lstrcpyA,modLoader,quiltName
 invoke lstrcpyA,modQuery,userQuery
 mov [uiIndex],3
 mov [textCalls],0
 mov [comboCalls],0
 mov [clearCalls],0
 ret
endp
proc FixtureSelected,id
 invoke lstrcpyA,selectedVersion,[id]
 mov [readCalls],0
 mov [textCalls],0
 mov [comboCalls],0
 mov [clearCalls],0
 ret
endp
proc FixtureWrite uses ebx esi edi,id,text
 local file:DWORD, written:DWORD, length:DWORD
 invoke lstrcpyW,fixturePath,gameDir
 invoke lstrcatW,fixturePath,versionsDir
 invoke MultiByteToWideChar,65001,8,[id],-1,fixtureName,128
 invoke lstrcatW,fixturePath,fixtureName
 stdcall EnsureDirectory,fixturePath
 test eax,eax
 jz .bad
 invoke lstrcatW,fixturePath,fileSlash
 invoke lstrcatW,fixturePath,fixtureName
 invoke lstrcatW,fixturePath,jsonExt
 invoke CreateFileW,fixturePath,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [file],eax
 invoke lstrlenA,[text]
 mov [length],eax
 invoke WriteFile,[file],[text],eax,addr written,0
 mov ebx,eax
 invoke CloseHandle,[file]
 test ebx,ebx
 jz .bad
 mov eax,[length]
 cmp eax,[written]
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc FixtureReadFile uses ebx esi edi,path
 local file:DWORD,size:DWORD,got:DWORD
 inc [readCalls]
 invoke CreateFileW,[path],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [file],eax
 invoke GetFileSize,eax,0
 cmp eax,1048576
 ja .closeBad
 mov [size],eax
 inc eax
 mov ebx,eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,8,ebx
 test eax,eax
 jz .closeBad
 mov esi,eax
 invoke ReadFile,[file],esi,[size],addr got,0
 mov ebx,eax
 invoke CloseHandle,[file]
 test ebx,ebx
 jz .freeBad
 mov eax,[size]
 cmp eax,[got]
 jne .freeBad
 mov eax,esi
 ret
.freeBad:
 stdcall FreeMem,esi
 jmp .bad
.closeBad:
 invoke CloseHandle,[file]
.bad:
 xor eax,eax
 ret
endp

proc FixtureSetText,window,text
 inc [textCalls]
 cmp [window],101
 jne .bad
 invoke lstrlenA,[text]
 cmp eax,64
 jae .bad
 invoke lstrcpyA,uiText,[text]
 mov eax,1
 ret
.bad:
 mov [fixtureBad],1
 xor eax,eax
 ret
endp
proc FixtureMessage,window,message,wparam,lparam
 inc [comboCalls]
 cmp [window],102
 jne .bad
 cmp [message],CB_SETCURSEL
 jne .bad
 cmp [wparam],3
 ja .bad
 cmp [lparam],0
 jne .bad
 mov eax,[wparam]
 mov [uiIndex],eax
 ret
.bad:
 mov [fixtureBad],1
 or eax,-1
 ret
endp
proc ClearModResults
 inc [clearCalls]
 invoke lstrcmpA,modQuery,userQuery
 test eax,eax
 jz .done
 mov [fixtureBad],1
.done:
 ret
endp

include '..\src\common.inc'
include '..\src\versions.inc'
define EReadFile FixtureReadFile
define SetWindowTextA textImport
define SendMessageW messageImport
include '..\src\mod_profile.inc'
restore SendMessageW
restore SetWindowTextA
restore EReadFile
include '..\src\imports.inc'
