; Real provider + real shared helpers/JSON; fixture transport only.
format PE console 4.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '../src/mod_types.inc'
section '.data' data readable writeable
hWnd dd 0
fixtureCase dd 0
fixtureCalls dd 0
fixtureBad dd 0
lastStatus dd 0
fixtureQuery db 'a+b &c',0
badFilter db '1.21.1"',0
gameNormal db '1.21.1',0
expectedSearch db 'https://api.modrinth.com/v2/search?limit=20&query=a%2Bb%20%26c&facets=%5B%5B%22project_type%3Amod%22%5D%2C%5B%22versions%3A1.21.1%22%5D%2C%5B%22categories%3Afabric%22%5D%5D&offset=0',0
expectedVersions db 'https://api.modrinth.com/v2/project/A1b2C3d4/version?include_changelog=false&game_versions=%5B%221.21.1%22%5D&loaders=%5B%22fabric%22%5D',0
expectedFile db 'valid.jar',0
expectedId db 'A1b2C3d4',0
searchFixture db '{"total_hits":3,"hits":[{"project_id":"A1b2C3d4","project_type":"mod","title":"Project Alpha","description":"A real fixture"},{"project_id":"../escape","project_type":"mod","title":"Rejected traversal"},{"project_id":"BadPacks","project_type":"modpack","title":"Not a mod"}]}',0
versionsFixture db '[{"id":"WrongOne","version_number":"wrong-loader","game_versions":["1.21.1"],"loaders":["forge"],"files":[{"primary":true,"filename":"wrong.jar","url":"https://cdn.modrinth.com/data/A1b2C3d4/versions/WrongOne/wrong.jar","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}}]},{"id":"WrongTwo","version_number":"wrong-game","game_versions":["1.20.1"],"loaders":["fabric"],"files":[{"primary":true,"filename":"wrong.jar","url":"https://cdn.modrinth.com/data/A1b2C3d4/versions/WrongTwo/wrong.jar","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}}]},{"id":"GoodFile","version_number":"1.0.0","version_type":"release","game_versions":["1.21.1"],"loaders":["fabric"],"files":[{"primary":false,"filename":"sources.jar","url":"https://cdn.modrinth.com/data/A1b2C3d4/versions/GoodFile/sources.jar","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}},{"primary":true,"filename":"valid.jar","url":"https://cdn.modrinth.com/data/A1b2C3d4/versions/GoodFile/valid.jar","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}}],"dependencies":[{"project_id":"NeedThis","dependency_type":"required"},{"project_id":"Optional","dependency_type":"optional"}]}]',0
unsafeFixture db '[{"id":"BadFiles","version_number":"0","game_versions":["1.21.1"],"loaders":["fabric"],"files":[{"primary":true,"filename":"../escape.jar","url":"https://cdn.modrinth.com/data/a.jar","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}},{"primary":true,"filename":"other.jar","url":"https://cdn.modrinth.com.attacker.invalid/a.jar","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}},{"primary":true,"filename":"nohash.jar","url":"https://cdn.modrinth.com/data/a.jar","hashes":{}},{"filename":"missingprimary.jar","url":"https://cdn.modrinth.com/data/a.jar","hashes":{"sha1":"0123456789abcdef0123456789abcdef01234567"}}]}]',0
emptyFixture db '{"hits":[],"total_hits":0}',0
malformedFixture db '{"hits":[',0
nulStringFixture db '"safe.jar\u0000trailing"',0
testContext dd 0
gameDir rw 1024
requestUrl rb 8192

section '.text' code readable executable
start:
    stdcall ModsInit
    invoke lstrcpyA,modQuery,fixtureQuery
    stdcall ModrinthSearch
    cmp eax,1
    jne failed
    cmp [modProjectCount],1
    jne failed
    cmp [modTotal],3
    jne failed
    invoke lstrcmpA,modProjects,expectedId
    test eax,eax
    jnz failed
    cmp [fixtureBad],0
    jne failed
    mov [fixtureCase],1
    mov [modSelectedProject],0
    stdcall ModrinthVersions
    cmp eax,1
    jne failed
    cmp [modFileCount],1
    jne failed
    cmp dword [modFiles+MF_REQUIRED],1
    jne failed
    cmp dword [modFiles+MF_KIND],1
    jne failed
    invoke lstrcmpA,modFiles+MF_NAME,expectedFile
    test eax,eax
    jnz failed
    cmp [fixtureBad],0
    jne failed
    mov [fixtureCase],2
    stdcall ModrinthVersions
    cmp eax,1
    jne failed
    cmp [modFileCount],0
    jne failed
    mov [fixtureCase],3
    stdcall ModrinthSearch
    test eax,eax
    jnz failed
    cmp [modProjectCount],0
    jne failed
    mov [fixtureCase],4
    stdcall ModrinthSearch
    test eax,eax
    jnz failed
    mov [fixtureCase],5
    mov eax,[fixtureCalls]
    mov ebx,eax
    mov [modCancel],1
    stdcall ModrinthSearch
    test eax,eax
    jnz failed
    cmp [fixtureCalls],ebx
    jne failed
    mov [modCancel],0
    mov [fixtureCase],6
    invoke lstrcpyA,modGameVersion,badFilter
    stdcall ModrinthSearch
    test eax,eax
    jnz failed
    cmp [fixtureCalls],ebx
    jne failed
    invoke lstrcpyA,modGameVersion,gameNormal
    mov [fixtureCase],7
    stdcall ModrinthSearch
    cmp eax,1
    jne failed
    cmp [modProjectCount],0
    jne failed
    cmp [fixtureBad],0
    jne failed
    mov [fixtureCase],8
    stdcall JsonParse,nulStringFixture
    test eax,eax
    jz failed
    mov [testContext],eax
    stdcall JsonRoot,eax
    stdcall MrCopy,[testContext],eax,requestUrl,8192
    cmp eax,-1
    jne failed
    cmp byte [requestUrl],0
    jne failed
    stdcall JsonFree,[testContext]
    invoke ExitProcess,0
failed:
    mov eax,[fixtureCase]
    add eax,10
    invoke ExitProcess,eax

proc SetStatusA,source
    mov eax,[source]
    mov [lastStatus],eax
    ret
endp

proc ModrinthFixtureRequest uses ebx esi edi,url,method,headers,body
    inc [fixtureCalls]
    cmp [method],0
    jne .bad
    cmp [body],0
    jne .bad
    cmp [headers],mrHeader
    jne .bad
    invoke WideCharToMultiByte,65001,0,[url],-1,requestUrl,8192,0,0
    mov ebx,expectedSearch
    cmp [fixtureCase],1
    je .versionUrl
    cmp [fixtureCase],2
    jne .check
.versionUrl:
    mov ebx,expectedVersions
.check:
    invoke lstrcmpA,requestUrl,ebx
    test eax,eax
    jnz .bad
    mov [httpStatus],200
    mov esi,searchFixture
    cmp [fixtureCase],1
    je .versions
    cmp [fixtureCase],2
    je .unsafe
    cmp [fixtureCase],3
    je .rate
    cmp [fixtureCase],4
    je .malformed
    cmp [fixtureCase],7
    je .empty
    jmp .copy
.versions:
    mov esi,versionsFixture
    jmp .copy
.unsafe:
    mov esi,unsafeFixture
    jmp .copy
.rate:
    mov [httpStatus],429
    mov esi,emptyFixture
    jmp .copy
.malformed:
    mov esi,malformedFixture
    jmp .copy
.empty:
    mov esi,emptyFixture
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
.bad:
    mov [fixtureBad],1
    xor eax,eax
    ret
endp

include '../src/common.inc'
include '../src/mods.inc'
define HttpRequest ModrinthFixtureRequest
include '../src/modrinth.inc'
restore HttpRequest
include '../src/imports.inc'
