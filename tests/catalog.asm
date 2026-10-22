; Native catalogue JSON/transport tests. No network or user files.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\mod_types.inc'
section '.data' data readable writeable
hWnd dd 0
engineCancel dd 0
gameDir rw 1024
step dd 1
fixtureCase dd 0
calls dd 0
downloadCalls dd 0
badRequest dd 0
query db 'a+b &c',0
fabric db 'fabric',0
game db '1.21.1',0
key db 'fixture-secret',0
badKey db 'fixture',13,10,'X:bad',0
searchFixture db '{"total_hits":2,"hits":[{"project_id":"PackTest","project_type":"modpack","title":"Pack","description":"Test pack"},{"project_id":"NotAPack","project_type":"mod","title":"Mod"}]}',0
versionsFixture db '[{"id":"BadFile","version_number":"wrong","game_versions":["1.21.1"],"loaders":["fabric"],"files":[{"primary":true,"filename":"../escape.mrpack","url":"https://cdn.modrinth.com/data/x.mrpack","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}}]},{"id":"GoodFile","version_number":"1.0","version_type":"release","game_versions":["1.21.1"],"loaders":["fabric"],"files":[{"primary":true,"filename":"valid.mrpack","url":"https://cdn.modrinth.com/data/valid.mrpack","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}}]},{"id":"OtherOne","version_number":"2.0","version_type":"beta","game_versions":["1.20.1"],"loaders":["forge"],"files":[{"primary":true,"filename":"other.mrpack","url":"https://cdn.modrinth.com/data/other.mrpack","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}}]}]',0
cfSearch db '{"data":[{"id":123,"gameId":432,"classId":4471,"name":"CF Pack","summary":"Summary","isAvailable":true,"allowModDistribution":true,"links":{"websiteUrl":"https://www.curseforge.com/minecraft/modpacks/test"}},{"id":124,"gameId":432,"classId":6,"name":"Wrong class","isAvailable":true}],"pagination":{"totalCount":1}}',0
cfVersions db '{"data":[{"id":1,"gameId":432,"modId":123,"fileName":"a.zip","displayName":"Good","releaseType":1,"isAvailable":true,"downloadUrl":"https://media.forgecdn.net/files/1/a.zip","hashes":[{"algo":1,"value":"0123456789abcdef0123456789abcdef01234567"}],"gameVersions":["1.21.1","Fabric"]},{"id":2,"gameId":432,"modId":123,"fileName":"b.zip","displayName":"Blocked","releaseType":1,"isAvailable":true,"downloadUrl":null,"hashes":[{"algo":1,"value":"0123456789abcdef0123456789abcdef01234567"}],"gameVersions":["1.21.1","Fabric"]},{"id":3,"gameId":432,"modId":999,"fileName":"wrong.zip","displayName":"Wrong","releaseType":1,"isAvailable":true,"gameVersions":["1.21.1","Fabric"]}]}',0
badJson db '{"data":BROKEN}',0
expectedName db 'valid.mrpack',0
expectedSearch db 'https://api.modrinth.com/v2/search?limit=20&query=a%2Bb%20%26c&facets=%5B%5B%22project_type%3Amodpack%22%5D%2C%5B%22versions%3A1.21.1%22%5D%2C%5B%22categories%3Afabric%22%5D%5D&offset=0',0
validArchive db 'valid.mrpack',0
badArchive db 'CON.mrpack',0
nulArchive db 'foo.zip ',0
destination du 'tests\out\catalog-fixture.mrpack',0
requestUrl rb 8192

section '.text' code readable executable
start:
 stdcall ModsInit
 invoke lstrcpyA,packQuery,query
 invoke lstrcpyA,packGameVersion,game
 invoke lstrcpyA,packLoader,fabric
 stdcall PackSearch
 cmp eax,1
 jne fail
 cmp [packProjectCount],1
 jne fail
 cmp [badRequest],0
 jne fail
 inc [step]
 mov [fixtureCase],1
 mov [packSelectedProject],0
 stdcall PackVersions
 cmp eax,1
 jne fail
 cmp [packFileCount],1
 jne fail
 invoke lstrcmpA,packFiles+MF_NAME,expectedName
 test eax,eax
 jnz fail
 inc [step]
 stdcall PackDownload,0,destination
 cmp eax,1
 jne fail
 cmp [downloadCalls],1
 jne fail
 inc [step]
 mov byte [packGameVersion],0
 mov byte [packLoader],0
 stdcall PackVersions
 cmp eax,1
 jne fail
 cmp [packFileCount],2
 jne fail
 inc [step]
 ; CurseForge catalog search was removed; only Modrinth is exercised here.
 inc [step]
 mov [engineCancel],1
 mov ebx,[calls]
 stdcall PackSearch
 test eax,eax
 jnz fail
 cmp [calls],ebx
 jne fail
 mov [engineCancel],0
 inc [step]
 stdcall PcArchiveFilename,validArchive,0
 cmp eax,1
 jne fail
 stdcall PcArchiveFilename,badArchive,0
 test eax,eax
 jnz fail
 stdcall PcArchiveFilename,nulArchive,1
 test eax,eax
 jnz fail
 invoke ExitProcess,0
fail:
 invoke ExitProcess,[step]
proc SetStatusA,text
 ret
endp
proc FixtureRequest uses ebx esi edi,url,method,headers,body
 inc [calls]
 invoke WideCharToMultiByte,65001,0,[url],-1,requestUrl,8192,0,0
 cmp [fixtureCase],0
 jne .response
 invoke lstrcmpA,requestUrl,expectedSearch
 test eax,eax
 jz .response
 mov [badRequest],1
.response:
 mov [httpStatus],200
 mov esi,searchFixture
 cmp [fixtureCase],1
 je .versions
 cmp [fixtureCase],3
 je .cfSearch
 cmp [fixtureCase],4
 je .cfVersions
 cmp [fixtureCase],7
 je .bad
 jmp .copy
.versions: mov esi,versionsFixture
 jmp .copy
.cfSearch: mov esi,cfSearch
 jmp .copy
.cfVersions: mov esi,cfVersions
 jmp .copy
.bad: mov esi,badJson
.copy:
 invoke lstrlenA,esi
 inc eax
 mov ebx,eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 mov edi,eax
 invoke lstrcpyA,edi,esi
 mov eax,edi
 ret
endp
proc FixtureDownload,url,destination,sha1
 inc [downloadCalls]
 stdcall ModIsSha1,[sha1]
 ret
endp
include '..\src\common.inc'
include '..\src\mods.inc'
define HttpRequest FixtureRequest
define HttpDownload FixtureDownload
include '..\src\modrinth.inc'
include '..\src\curseforge.inc'
include '..\src\catalog.inc'
restore HttpDownload
restore HttpRequest
include '..\src\imports.inc'
