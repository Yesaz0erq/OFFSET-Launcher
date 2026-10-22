format PE console 4.0
entry start
stack 100000h,40000h
include '..\tools\INCLUDE\win32ax.inc'
section '.data' data readable writeable
manifestW du 'https://piston-meta.mojang.com/mc/game/version_manifest_v2.json',0
manifestA db 'https://piston-meta.mojang.com/mc/game/version_manifest_v2.json',0
manifestPath du 'tests\cache\manifest.json',0
metadataPath du 'tests\cache\version.json',0
insecureW du 'http://piston-meta.mojang.com/mc/game/version_manifest_v2.json',0
redirectW du 'https://aka.ms/devicelogin',0
resultPath du 'tests\http-result.txt',0
keyLatest db 'latest',0
keyRelease db 'release',0
keyVersions db 'versions',0
keyUrl db 'url',0
keySha1 db 'sha1',0
metadataUrl rb 2048
metadataSha rb 41
wrongSha db '0000000000000000000000000000000000000000',0
response dd 0
context dd 0
line rb 256
outHandle dd 0
written dd 0
yes db 'HTTP request, JSON manifest, streaming download, SHA1 cache, atomic failure, HTTPS and redirect enforcement: passed',13,10,0
no db 'HTTP TEST FAILED',13,10,0
section '.text' code readable executable
start:
    invoke CreateFileW,resultPath,40000000h,1,0,2,80h,0
    mov [outHandle],eax
    stdcall HttpRequest,manifestW,0,0,0
    test eax,eax
    jz fail
    mov [response],eax
    cmp [httpStatus],200
    jne fail
    stdcall JsonParse,eax
    test eax,eax
    jz fail
    mov [context],eax
    stdcall JsonRoot,eax
    stdcall JsonGet,[context],eax,keyLatest
    stdcall JsonGet,[context],eax,keyRelease
    stdcall JsonString,[context],eax,line,256
    test eax,eax
    jz fail
    invoke WriteFile,[outHandle],line,eax,written,0
    stdcall JsonRoot,[context]
    stdcall JsonGet,[context],eax,keyVersions
    stdcall JsonAt,[context],eax,0
    push eax
    stdcall JsonGet,[context],eax,keyUrl
    stdcall JsonString,[context],eax,metadataUrl,2048
    pop eax
    stdcall JsonGet,[context],eax,keySha1
    stdcall JsonString,[context],eax,metadataSha,41
    cmp eax,40
    jne fail
    stdcall JsonFree,[context]
    stdcall FreeMem,[response]
    stdcall HttpDownload,manifestA,manifestPath,0
    test eax,eax
    jz fail
    stdcall HttpDownload,metadataUrl,metadataPath,metadataSha
    test eax,eax
    jz fail
    stdcall HttpDownload,metadataUrl,metadataPath,metadataSha
    test eax,eax
    jz fail
    ; Hash mismatch must fail while preserving the verified destination.
    stdcall HttpDownload,metadataUrl,metadataPath,wrongSha
    test eax,eax
    jnz fail
    stdcall VerifySha1,metadataPath,metadataSha
    test eax,eax
    jz fail
    stdcall HttpRequest,insecureW,0,0,0
    test eax,eax
    jnz fail
    ; A Microsoft shortlink must remain a 30x response; auth redirects stop here.
    stdcall HttpRequest,redirectW,0,0,0
    test eax,eax
    jz fail
    stdcall FreeMem,eax
    cmp [httpStatus],300
    jb fail
    cmp [httpStatus],400
    jae fail
    invoke lstrlenA,yes
    invoke WriteFile,[outHandle],yes,eax,written,0
    invoke ExitProcess,0
fail:
    invoke lstrlenA,no
    invoke WriteFile,[outHandle],no,eax,written,0
    invoke ExitProcess,1
include '..\src\common.inc'
include '..\src\imports.inc'
