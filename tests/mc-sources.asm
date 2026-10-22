; Original source-routing tests: real JSON parser, deterministic HTTP transport.
format PE console 6.0
stack 100000h,40000h
entry start
include 'win32ax.inc'

section '.data' data readable writeable
engineCancel dd 0
testNo dd 0
fixtureBad dd 0
downloadCalls dd 0
downloadSuccessAt dd -1
cancelDownload dd 0
requestCalls dd 0
requestMode dd 0
responseCount dd 0
contextCount dd 0
savedResponse dd 0
responsePointers rd 8
downloadUrls rb 3*4096
outBuffer rb 4096
outGuard dd 0DEADBEEFh
fixturePath du 'tests\out\mc-source-fixture.bin',0
fixtureSha db '0123456789abcdef0123456789abcdef01234567',0
targetVersion db '1.21.1',0
officialMeta db 'https://piston-meta.mojang.com/v1/packages/abc/1.21.1.json',0
officialData db 'https://piston-data.mojang.com/v1/objects/abc/client.jar',0
officialLauncher db 'https://launcher.mojang.com/v1/objects/abc/client.jar',0
officialLaunchermeta db 'https://launchermeta.mojang.com/v1/packages/abc/1.21.1.json',0
officialLibrary db 'https://libraries.minecraft.net/org/example/example.jar',0
officialAsset db 'https://resources.download.minecraft.net/01/0123456789abcdef',0
mixedCaseHost db 'HTTPS://PISTON-META.MOJANG.COM/v1/Packages/ABC/Case.JSON',0
mixedCaseMirror db 'https://bmclapi2.bangbang93.com/v1/Packages/ABC/Case.JSON',0
mirrorMeta db 'https://bmclapi2.bangbang93.com/v1/packages/abc/1.21.1.json',0
mirrorData db 'https://bmclapi2.bangbang93.com/v1/objects/abc/client.jar',0
mirrorLibrary db 'https://bmclapi2.bangbang93.com/maven/org/example/example.jar',0
secondLibrary db 'https://bmclapi2.bangbang93.com/libraries/org/example/example.jar',0
mirrorMetaLibrary db 'https://bmclapi2.bangbang93.com/maven/v1/packages/abc/1.21.1.json',0
mirrorDataLibrary db 'https://bmclapi2.bangbang93.com/maven/v1/objects/abc/client.jar',0
secondMetaLibrary db 'https://bmclapi2.bangbang93.com/libraries/v1/packages/abc/1.21.1.json',0
secondDataLibrary db 'https://bmclapi2.bangbang93.com/libraries/v1/objects/abc/client.jar',0
mirrorAsset db 'https://bmclapi2.bangbang93.com/assets/01/0123456789abcdef',0
unknownUrl db 'https://example.invalid/file.jar',0
fakeHost db 'https://piston-meta.mojang.com.evil.invalid/file.json',0
fakeUser db 'https://piston-meta.mojang.com@evil.invalid/file.json',0
fakePort db 'https://piston-meta.mojang.com:443/file.json',0
insecureUrl db 'http://piston-meta.mojang.com/file.json',0
mirrorManifest du 'https://bmclapi2.bangbang93.com/mc/game/version_manifest_v2.json',0
officialManifest du 'https://piston-meta.mojang.com/mc/game/version_manifest_v2.json',0
goodJson db '{"versions":[{"id":"1.21.1","url":"https://piston-meta.mojang.com/v1/packages/abc/1.21.1.json","sha1":"0123456789abcdef0123456789abcdef01234567"}]}',0
missingJson db '{"versions":[{"id":"1.20.1","url":"https://piston-meta.mojang.com/v1/packages/old/1.20.1.json","sha1":"0123456789abcdef0123456789abcdef01234567"}]}',0
badJson db '{"versions":broken}',0
objectJson db '{"versions":{"id":"1.21.1","url":"https://example.invalid/","sha1":"0123456789abcdef0123456789abcdef01234567"}}',0
shortShaJson db '{"versions":[{"id":"1.21.1","url":"https://piston-meta.mojang.com/a.json","sha1":"0123"}]}',0
emptyJson db '{"versions":[]}',0
mapCases dd officialMeta,mirrorMeta,0,0, officialData,mirrorData,0,0
 dd officialLauncher,mirrorData,0,0, officialLaunchermeta,mirrorMeta,0,0
 dd officialLibrary,mirrorLibrary,1,0, officialLibrary,secondLibrary,1,1
 dd officialMeta,mirrorMetaLibrary,1,0, officialData,mirrorDataLibrary,1,0
 dd officialMeta,secondMetaLibrary,1,1, officialData,secondDataLibrary,1,1
 dd officialAsset,mirrorAsset,2,0
 dd mixedCaseHost,mixedCaseMirror,0,0
mapCount = ($-mapCases)/16
badMapCases dd fakeHost,0,0, fakeUser,0,0, fakePort,0,0, insecureUrl,0,0
 dd unknownUrl,0,0, officialMeta,0,1, officialAsset,2,1
 dd officialMeta,3,0, officialLibrary,1,2, officialAsset,0,0
badMapCount = ($-badMapCases)/12

section '.text' code readable executable
start:
 mov esi,mapCases
 mov edi,mapCount
.map:
 inc [testNo]
 stdcall McMirrorUrl,dword [esi],outBuffer,4096,dword [esi+8],dword [esi+12]
 cmp eax,1
 jne failed
 invoke lstrcmpA,outBuffer,dword [esi+4]
 test eax,eax
 jnz failed
 cmp [outGuard],0DEADBEEFh
 jne failed
 add esi,16
 dec edi
 jnz .map
 mov esi,badMapCases
 mov edi,badMapCount
.badMap:
 inc [testNo]
 mov byte [outBuffer],'X'
 stdcall McMirrorUrl,dword [esi],outBuffer,4096,dword [esi+4],dword [esi+8]
 test eax,eax
 jnz failed
 cmp byte [outBuffer],0
 jne failed
 add esi,12
 dec edi
 jnz .badMap
 ; Exact capacity, empty/zero capacity, and alias must not corrupt input.
 inc [testNo]
 invoke lstrlenA,mirrorMeta
 inc eax
 stdcall McMirrorUrl,officialMeta,outBuffer,eax,0,0
 cmp eax,1
 jne failed
 inc [testNo]
 invoke lstrlenA,mirrorMeta
 stdcall McMirrorUrl,officialMeta,outBuffer,eax,0,0
 test eax,eax
 jnz failed
 cmp byte [outBuffer],0
 jne failed
 inc [testNo]
 mov word [outBuffer],0A5FFh
 stdcall McMirrorUrl,officialMeta,outBuffer,1,0,0
 test eax,eax
 jnz failed
 cmp word [outBuffer],0A500h
 jne failed
 inc [testNo]
 mov byte [outBuffer],'Q'
 stdcall McMirrorUrl,officialMeta,outBuffer,0,0,0
 test eax,eax
 jnz failed
 cmp byte [outBuffer],'Q'
 jne failed
 inc [testNo]
 invoke lstrcpyA,outBuffer,officialMeta
 stdcall McMirrorUrl,outBuffer,outBuffer,4096,0,0
 test eax,eax
 jnz failed
 invoke lstrcmpA,outBuffer,officialMeta
 test eax,eax
 jnz failed
 ; One mirror succeeds, otherwise fall through to original official URL.
 inc [testNo]
 stdcall FixtureReset
 mov [downloadSuccessAt],1
 stdcall McDownload,officialMeta,fixturePath,fixtureSha,0
 cmp eax,1
 jne failed
 cmp [downloadCalls],1
 jne failed
 invoke lstrcmpA,downloadUrls,mirrorMeta
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall FixtureReset
 mov [downloadSuccessAt],2
 stdcall McDownload,officialMeta,fixturePath,fixtureSha,0
 cmp eax,1
 jne failed
 cmp [downloadCalls],2
 jne failed
 invoke lstrcmpA,downloadUrls+4096,officialMeta
 test eax,eax
 jnz failed
 ; Libraries have both PCL mirror prefixes before the official candidate.
 inc [testNo]
 stdcall FixtureReset
 mov [downloadSuccessAt],3
 stdcall McDownload,officialLibrary,fixturePath,fixtureSha,1
 cmp eax,1
 jne failed
 cmp [downloadCalls],3
 jne failed
 invoke lstrcmpA,downloadUrls,mirrorLibrary
 test eax,eax
 jnz failed
 invoke lstrcmpA,downloadUrls+4096,secondLibrary
 test eax,eax
 jnz failed
 invoke lstrcmpA,downloadUrls+8192,officialLibrary
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall FixtureReset
 mov [downloadSuccessAt],2
 stdcall McDownload,officialLibrary,fixturePath,fixtureSha,1
 cmp eax,1
 jne failed
 cmp [downloadCalls],2
 jne failed
 inc [testNo]
 stdcall FixtureReset
 stdcall McDownload,officialAsset,fixturePath,fixtureSha,2
 test eax,eax
 jnz failed
 cmp [downloadCalls],2
 jne failed
 invoke lstrcmpA,downloadUrls,mirrorAsset
 test eax,eax
 jnz failed
 invoke lstrcmpA,downloadUrls+4096,officialAsset
 test eax,eax
 jnz failed
 ; Unknown URLs are not rewritten, and cancellation stops later attempts.
 inc [testNo]
 stdcall FixtureReset
 mov [downloadSuccessAt],1
 stdcall McDownload,unknownUrl,fixturePath,fixtureSha,0
 cmp eax,1
 jne failed
 cmp [downloadCalls],1
 jne failed
 invoke lstrcmpA,downloadUrls,unknownUrl
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall FixtureReset
 mov [engineCancel],1
 stdcall McDownload,officialLibrary,fixturePath,fixtureSha,1
 test eax,eax
 jnz failed
 cmp [downloadCalls],0
 jne failed
 inc [testNo]
 stdcall FixtureReset
 mov [cancelDownload],1
 stdcall McDownload,officialLibrary,fixturePath,fixtureSha,1
 test eax,eax
 jnz failed
 cmp [downloadCalls],1
 jne failed
 ; Cancellation observed just after a successful transport still aborts.
 inc [testNo]
 stdcall FixtureReset
 mov [cancelDownload],1
 mov [downloadSuccessAt],1
 stdcall McDownload,officialMeta,fixturePath,fixtureSha,0
 test eax,eax
 jnz failed
 cmp [downloadCalls],1
 jne failed
 ; Manifest modes: good; malformed; missing target; transport failure;
 ; HTTP denial; both malformed; official missing target; cancelled after
 ; first response; object masquerading as versions array; invalid SHA;
 ; empty mirrored array. All rejected buffers/contexts must be released.
 xor esi,esi
.manifest:
 inc [testNo]
 stdcall FixtureReset
 mov [requestMode],esi
 stdcall McManifestRequest,targetVersion
 cmp esi,5
 je .expectFail
 cmp esi,7
 je .expectFail
 test eax,eax
 jz failed
 mov [savedResponse],eax
 cmp esi,0
 jne .twoCalls
 cmp [requestCalls],1
 jne failed
 jmp .freeResponse
.twoCalls:
 cmp [requestCalls],2
 jne failed
.freeResponse:
 stdcall FixtureFree,[savedResponse]
 jmp .clean
.expectFail:
 test eax,eax
 jnz failed
 mov eax,2
 cmp esi,7
 jne .failCalls
 mov eax,1
.failCalls:
 cmp [requestCalls],eax
 jne failed
.clean:
 stdcall FixtureClean
 test eax,eax
 jz failed
 inc esi
 cmp esi,11
 jb .manifest
 inc [testNo]
 stdcall FixtureReset
 mov [engineCancel],1
 stdcall McManifestRequest,targetVersion
 test eax,eax
 jnz failed
 cmp [requestCalls],0
 jne failed
 stdcall FixtureClean
 test eax,eax
 jz failed
 invoke ExitProcess,0
failed:
 mov eax,[testNo]
 add eax,100
 invoke ExitProcess,eax

proc FixtureReset
 mov [engineCancel],0
 mov [downloadCalls],0
 mov [downloadSuccessAt],-1
 mov [cancelDownload],0
 mov [requestCalls],0
 mov [requestMode],0
 ret
endp

proc FixtureClean
 xor eax,eax
 cmp [fixtureBad],0
 jne .done
 cmp [responseCount],0
 jne .done
 cmp [contextCount],0
 jne .done
 inc eax
.done:
 ret
endp

proc FixtureDownload uses ebx,url,path,sha
 inc [downloadCalls]
 cmp [downloadCalls],3
 ja .bad
 stdcall FixtureCompareW,[path],fixturePath
 test eax,eax
 jnz .bad
 invoke lstrcmpA,[sha],fixtureSha
 test eax,eax
 jnz .bad
 mov ebx,[downloadCalls]
 dec ebx
 shl ebx,12
 add ebx,downloadUrls
 invoke lstrcpyA,ebx,[url]
 cmp [cancelDownload],0
 je .result
 mov [engineCancel],1
.result:
 xor eax,eax
 mov ecx,[downloadCalls]
 cmp ecx,[downloadSuccessAt]
 sete al
 ret
.bad:
 mov [fixtureBad],1
 xor eax,eax
 ret
endp

proc FixtureRequest uses ebx esi edi,url,method,headers,body
 inc [requestCalls]
 cmp [requestCalls],2
 ja .bad
 cmp [method],0
 jne .bad
 cmp [headers],0
 jne .bad
 cmp [body],0
 jne .bad
 mov eax,mirrorManifest
 cmp [requestCalls],1
 je .compare
 mov eax,officialManifest
.compare:
 stdcall FixtureCompareW,[url],eax
 test eax,eax
 jnz .bad
 mov [httpStatus],200
 mov esi,goodJson
 cmp [requestCalls],1
 jne .official
 cmp [requestMode],1
 je .malformed
 cmp [requestMode],2
 je .missing
 cmp [requestMode],3
 je .transport
 cmp [requestMode],4
 je .denied
 cmp [requestMode],5
 je .malformed
 cmp [requestMode],6
 je .missing
 cmp [requestMode],7
 je .cancelled
 cmp [requestMode],8
 je .object
 cmp [requestMode],9
 je .shortSha
 cmp [requestMode],10
 je .empty
 jmp .copy
.official:
 cmp [requestMode],5
 je .malformed
 cmp [requestMode],6
 je .missing
 jmp .copy
.malformed:
 mov esi,badJson
 jmp .copy
.missing:
 mov esi,missingJson
 jmp .copy
.transport:
 mov [httpStatus],0
 xor eax,eax
 ret
.denied:
 mov [httpStatus],403
 jmp .copy
.cancelled:
 mov [engineCancel],1
 mov esi,badJson
 jmp .copy
.object:
 mov esi,objectJson
 jmp .copy
.shortSha:
 mov esi,shortShaJson
 jmp .copy
.empty:
 mov esi,emptyJson
.copy:
 invoke lstrlenA,esi
 inc eax
 mov ebx,eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,8,ebx
 test eax,eax
 jz .bad
 mov edi,eax
 invoke lstrcpyA,edi,esi
 mov ecx,8
 mov ebx,responsePointers
.slot:
 cmp dword [ebx],0
 je .store
 add ebx,4
 loop .slot
 invoke GetProcessHeap
 invoke HeapFree,eax,0,edi
 jmp .bad
.store:
 mov [ebx],edi
 inc [responseCount]
 mov eax,edi
 ret
.bad:
 mov [fixtureBad],1
 xor eax,eax
 ret
endp

proc FixtureFree uses esi,pointerArg
 cmp [pointerArg],0
 je .done
 mov esi,responsePointers
 mov ecx,8
.find:
 mov eax,[pointerArg]
 cmp [esi],eax
 je .found
 add esi,4
 loop .find
 mov [fixtureBad],1
 ret
.found:
 mov dword [esi],0
 dec [responseCount]
 stdcall FreeMem,[pointerArg]
.done:
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

proc FixtureCompareW uses esi edi,left,right
 mov esi,[left]
 mov edi,[right]
.next:
 mov ax,[esi]
 cmp ax,[edi]
 jne .different
 test ax,ax
 jz .equal
 add esi,2
 add edi,2
 jmp .next
.equal:
 xor eax,eax
 ret
.different:
 mov eax,1
 ret
endp

include '..\src\common.inc'
define HttpRequest FixtureRequest
define HttpDownload FixtureDownload
define FreeMem FixtureFree
define JsonParse FixtureJsonParse
define JsonFree FixtureJsonFree
include '..\src\mc_sources.inc'
restore JsonFree
restore JsonParse
restore FreeMem
restore HttpDownload
restore HttpRequest
include '..\src\imports.inc'
