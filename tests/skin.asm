; Native GDI+/filesystem/JSON/multipart fixture. HTTP is mocked; no account
; is contacted or changed. Transport assertions include explicit binary sizes.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\version_types.inc'
OFFSET_SKIN_TEST_TRANSPORT = 1
section '.data' data readable writeable
authReady dd 1
authCancel dd 0
engineCancel dd 0
modCancel dd 0
authUuid db '00112233445566778899aabbccddeeff',0,31 dup(0)
authName db 'Skin_Test',0,54 dup(0)
authToken db 'fixture-token-do-not-persist',0,8164 dup(0)
gdipToken dd 0
gdipInit dd 1,0,0,0
testStep dd 1
testMode dd 0
posts dd 0
deletes dd 0
requests dd 0
ensures dd 0
invalidated dd 0
lastStatus dd 0
got dd 0
savedData dd 0
prefixW du 'tests\out\皮肤-skin-',0
tickFormatW du '%08X',0
sourceSuffixW du '-source.png',0
badSuffixW du '-bad.png',0
legacySuffixW du '-legacy.png',0
exportSuffixW du '-export.png',0
wrongSuffixW du '-export.ini',0
headerNeedleW du 'Authorization: Bearer fixture-token-do-not-persist',13,10,0
contentNeedleW du 'Content-Type: multipart/form-data; boundary=----OFFSET-Skin-',0
slimNeedleA db 'name="variant"',13,10,13,10,'slim',13,10,0
classicNeedleA db 'name="variant"',13,10,13,10,'classic',13,10,0
partNeedleA db 'name="file"; filename="skin.png"',13,10,'Content-Type: image/png',13,10,13,10,0
okA db '{}',0
emptyA db 0
badUuidA db '{"id":"f0112233445566778899aabbccddeefff","name":"Skin_Test","skins":[],"capes":[]}',0
profileA db '{"id":"00112233445566778899aabbccddeeff","name":"Skin_Test","skins":[{"id":"skin-id","state":"ACTIVE","url":"http://textures.minecraft.net/texture/0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef","variant":"SLIM"}],"capes":[{"id":"cape-id","state":"ACTIVE","url":"https://textures.minecraft.net/texture/ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff","alias":"Test cape"}]}',0
noSkinA db '{"id":"00112233445566778899aabbccddeeff","name":"Skin_Test","skins":[],"capes":[]}',0
evilUrlA db 'https://textures.minecraft.net.evil.invalid/texture/0123456789abcdef0123456789abcdef',0
queryUrlA db 'https://textures.minecraft.net/texture/0123456789abcdef0123456789abcdef?auth=x',0
traversalUrlA db 'https://textures.minecraft.net/texture/../0123456789abcdef0123456789abcdef',0
shortUrlA db 'https://textures.minecraft.net/texture/abcd',0
upgradedA db 'https://textures.minecraft.net/texture/0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef',0
png64 db 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,64,0,0,0,64,8,6,0,0,0,170,105,113,222,0,0,0,152,73,68,65,84,120,156,237,208,49,17,0,32,16,192,176,151,130,84,164,225,12,100,100,160,67,246,94,103,237,115,127,54,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,237,1,196,50,242,44,221,80,188,122,0,0,0,0,73,69,78,68,174,66,96,130
png64End:
png32 db 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,64,0,0,0,32,8,6,0,0,0,162,157,126,132,0,0,0,92,73,68,65,84,120,156,237,208,49,17,0,32,16,192,176,151,130,84,164,225,12,100,100,160,67,246,94,103,237,115,127,54,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,173,1,58,64,107,128,14,208,26,160,3,180,6,232,0,237,1,6,202,249,15,47,211,33,242,0,0,0,0,73,69,78,68,174,66,96,130
png32End:
baseW rw 1024
sourceW rw 1024
badW rw 1024
legacyW rw 1024
exportW rw 1024
wrongW rw 1024
savedPathW rw 2048
savedCurrentW rw 2048
numberW rw 64
urlA rb 512
readBuffer rb 2048
large rb 1048577

section '.text' code readable executable
start:
 invoke GdiplusStartup,gdipToken,gdipInit,0
 test eax,eax
 jnz failed
 invoke GetTickCount
 cinvoke wsprintfW,numberW,tickFormatW,eax
 invoke lstrcpyW,baseW,prefixW
 invoke lstrcatW,baseW,numberW
 stdcall TestPath,sourceW,sourceSuffixW
 stdcall TestPath,badW,badSuffixW
 stdcall TestPath,legacyW,legacySuffixW
 stdcall TestPath,exportW,exportSuffixW
 stdcall TestPath,wrongW,wrongSuffixW
 stdcall TestWrite,sourceW,png64,png64End-png64
 stdcall TestWrite,legacyW,png32,png32End-png32
 inc [testStep]
 stdcall SkinSelectLocal,sourceW
 cmp eax,1
 jne failed
 cmp [skinLocalReady],1
 jne failed
 cmp [skinLocalWidth],64
 jne failed
 cmp [skinLocalHeight],64
 jne failed
 cmp [requests],0
 jne failed
 mov eax,[skLocalData]
 mov [savedData],eax
 invoke lstrcpyW,savedPathW,skinLocalPathW
 inc [testStep]
 stdcall TestWrite,badW,png64,33
 stdcall SkinSelectLocal,badW
 test eax,eax
 jnz failed
 mov eax,[savedData]
 cmp eax,[skLocalData]
 jne failed
 invoke lstrcmpiW,savedPathW,skinLocalPathW
 test eax,eax
 jnz failed
 inc [testStep]
 invoke DeleteFileW,badW
 stdcall TestWrite,badW,large,1048577
 stdcall SkinSelectLocal,badW
 test eax,eax
 jnz failed
 cmp [skinLocalReady],1
 jne failed
 inc [testStep]
 invoke DeleteFileW,badW
 mov esi,png64
 mov edi,readBuffer
 mov ecx,png64End-png64
 rep movsb
 xor byte [readBuffer+64],1
 stdcall TestWrite,badW,readBuffer,png64End-png64
 stdcall SkinSelectLocal,badW
 test eax,eax
 jnz failed
 invoke DeleteFileW,badW
 stdcall TestWrite,badW,png64,png64End-png64-12
 stdcall SkinSelectLocal,badW
 test eax,eax
 jnz failed
 inc [testStep]
 stdcall SkinSelectLocal,legacyW
 cmp eax,1
 jne failed
 cmp [skinLocalHeight],32
 jne failed
 stdcall SkinSelectLocal,sourceW
 cmp eax,1
 jne failed
 inc [testStep]
 stdcall SkTextureUrl,evilUrlA,urlA
 test eax,eax
 jnz failed
 stdcall SkTextureUrl,queryUrlA,urlA
 test eax,eax
 jnz failed
 stdcall SkTextureUrl,traversalUrlA,urlA
 test eax,eax
 jnz failed
 stdcall SkTextureUrl,shortUrlA,urlA
 test eax,eax
 jnz failed
 inc [testStep]
 stdcall SkinRefresh
 cmp eax,1
 jne failed
 cmp [skinCurrentReady],1
 jne failed
 cmp [skinReady],1
 jne failed
 cmp [skinLocalReady],1
 jne failed
 cmp [skinModel],1
 jne failed
 cmp [skinCurrentHeight],64
 jne failed
 invoke lstrcmpA,skinUrlA,upgradedA
 test eax,eax
 jnz failed
 invoke lstrcpyW,savedCurrentW,skinCurrentPathW
 inc [testStep]
 stdcall SkinExportCurrent,exportW,0
 cmp eax,1
 jne failed
 stdcall TestReadPng,exportW
 stdcall SkinExportCurrent,exportW,0
 test eax,eax
 jnz failed
 stdcall TestReadPng,exportW
 stdcall SkinExportCurrent,exportW,1
 cmp eax,1
 jne failed
 stdcall SkinExportCurrent,skinCurrentPathW,1
 test eax,eax
 jnz failed
 stdcall SkinExportCurrent,wrongW,1
 test eax,eax
 jnz failed
 inc [testStep]
 ; Mutating the user's selected file after selection cannot change the upload.
 invoke DeleteFileW,sourceW
 stdcall TestWrite,sourceW,emptyA,1
 stdcall SkinUpload,1
 cmp eax,1
 jne failed
 cmp [posts],1
 jne failed
 cmp [skinPreviewError],0
 jne failed
 cmp [skinLocalReady],1
 jne failed
 inc [testStep]
 mov [testMode],4
 stdcall SkinUpload,0
 test eax,eax
 jnz failed
 cmp [posts],2
 jne failed
 cmp [invalidated],1
 jne failed
 cmp [skinResult],0
 jne failed
 inc [testStep]
 mov [testMode],3
 stdcall SkinReset
 cmp eax,1
 jne failed
 cmp [deletes],1
 jne failed
 cmp [skinPreviewError],1
 jne failed
 cmp [skinCurrentReady],0
 jne failed
 cmp [skinReady],1
 jne failed
 inc [testStep]
 mov [testMode],1
 stdcall SkinRefresh
 test eax,eax
 jnz failed
 cmp [skinReady],0
 jne failed
 cmp [skinCurrentReady],0
 jne failed
 inc [testStep]
 mov [testMode],5
 stdcall SkinRefresh
 cmp eax,1
 jne failed
 cmp [skinReady],1
 jne failed
 cmp [skinCurrentReady],0
 jne failed
 inc [testStep]
 mov [testMode],0
 stdcall SkinRefresh
 cmp eax,1
 jne failed
 mov eax,[requests]
 mov [savedData],eax
 mov [engineCancel],1
 stdcall SkinUpload,1
 test eax,eax
 jnz failed
 mov eax,[savedData]
 cmp eax,[requests]
 jne failed
 mov [engineCancel],0
 inc [testStep]
 ; Legacy atlases remain classic even when a caller requests model Slim.
 mov [testMode],6
 stdcall SkinSelectLocal,legacyW
 cmp eax,1
 jne failed
 mov [skinLocalModel],1
 stdcall SkinUpload,1
 cmp eax,1
 jne failed
 cmp [skinLocalHeight],32
 jne failed
 cmp [skinLocalModel],0
 jne failed
 cmp [posts],3
 jne failed
 inc [testStep]
 invoke lstrcpyW,savedCurrentW,skinCurrentPathW
 invoke lstrcpyW,savedPathW,skinLocalPathW
 stdcall SkinClear
 cmp [skinReady],0
 jne failed
 cmp [skinCurrentReady],0
 jne failed
 cmp [skinLocalReady],0
 jne failed
 cmp [skCurrentData],0
 jne failed
 cmp [skLocalData],0
 jne failed
 invoke GetFileAttributesW,savedCurrentW
 cmp eax,-1
 jne failed
 invoke GetFileAttributesW,savedPathW
 cmp eax,-1
 jne failed
 invoke GetFileAttributesW,sourceW
 cmp eax,-1
 je failed
 invoke GdiplusShutdown,[gdipToken]
 invoke ExitProcess,0
failed:
 stdcall SkinClear
 invoke ExitProcess,[testStep]

proc TestPath,dest,suffix
 invoke lstrcpyW,[dest],baseW
 invoke lstrcatW,[dest],[suffix]
 ret
endp
proc TestWrite uses ebx,path,data,bytes
 invoke CreateFileW,[path],GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je failed
 mov ebx,eax
 invoke WriteFile,ebx,[data],[bytes],got,0
 test eax,eax
 jz failed
 mov eax,[bytes]
 cmp [got],eax
 jne failed
 invoke CloseHandle,ebx
 ret
endp
proc TestReadPng uses ebx esi edi,path
 invoke CreateFileW,[path],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je failed
 mov ebx,eax
 invoke ReadFile,ebx,readBuffer,2048,got,0
 test eax,eax
 jz failed
 cmp [got],png64End-png64
 jne failed
 invoke CloseHandle,ebx
 mov esi,readBuffer
 mov edi,png64
 mov ecx,png64End-png64
 repe cmpsb
 jne failed
 ret
endp
proc SetStatusA,text
 mov eax,[text]
 mov [lastStatus],eax
 ret
endp
proc AuthEnsureSession
 inc [ensures]
 stdcall SkCancelled
 test eax,eax
 jnz .bad
 mov [authReady],1
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc AuthInvalidateSession
 inc [invalidated]
 mov [authReady],0
 ret
endp
proc TestContains uses ebx esi edi,data,bytes,needle,needleBytes
 mov ebx,[needleBytes]
 mov ecx,[bytes]
 sub ecx,ebx
 jc .bad
 inc ecx
 mov esi,[data]
.scan:
 push ecx esi
 mov edi,[needle]
 mov ecx,ebx
 repe cmpsb
 pop esi ecx
 je .good
 inc esi
 loop .scan
.bad:
 xor eax,eax
 ret
.good:
 mov eax,1
 ret
endp
proc TestWideContains,text,needle
 local bytes:DWORD
 invoke lstrlenW,[text]
 shl eax,1
 mov [bytes],eax
 invoke lstrlenW,[needle]
 shl eax,1
 stdcall TestContains,[text],[bytes],[needle],eax
 ret
endp
proc TestResponse uses ebx esi edi,data,bytes,status
 mov eax,[status]
 mov [skinHttpStatus],eax
 mov ebx,[bytes]
 mov [skResponseBytes],ebx
 inc ebx
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 test eax,eax
 jz failed
 mov edx,eax
 mov edi,eax
 mov esi,[data]
 mov ecx,[bytes]
 rep movsb
 mov byte [edi],0
 mov eax,edx
 ret
endp
proc SkinTestTransport uses ebx esi edi,kind,method,targetW,headersW,body,bodyBytes,limit
 inc [requests]
 cmp [kind],1
 je .texture
 cmp [kind],0
 jne failed
 stdcall TestWideContains,[headersW],headerNeedleW
 cmp eax,1
 jne failed
 invoke lstrcmpiW,[method],skPostW
 test eax,eax
 jz .upload
 invoke lstrcmpiW,[method],skDeleteW
 test eax,eax
 jz .reset
 invoke lstrcmpiW,[method],skGetW
 test eax,eax
 jnz failed
 invoke lstrcmpiW,[targetW],skProfileW
 test eax,eax
 jnz failed
 cmp [bodyBytes],0
 jne failed
 cmp [testMode],1
 je .wrongUuid
 cmp [testMode],5
 je .default
 invoke lstrlenA,profileA
 stdcall TestResponse,profileA,eax,200
 ret
.wrongUuid:
 invoke lstrlenA,badUuidA
 stdcall TestResponse,badUuidA,eax,200
 ret
.default:
 invoke lstrlenA,noSkinA
 stdcall TestResponse,noSkinA,eax,200
 ret
.upload:
 inc [posts]
 invoke lstrcmpiW,[targetW],skUploadW
 test eax,eax
 jnz failed
 stdcall TestWideContains,[headersW],contentNeedleW
 cmp eax,1
 jne failed
 invoke lstrlenA,[body]
 cmp eax,[bodyBytes]
 jae failed
 mov eax,png64
 mov edx,png64End-png64
 cmp [testMode],6
 jne @f
 mov eax,png32
 mov edx,png32End-png32
@@:
 stdcall TestContains,[body],[bodyBytes],eax,edx
 cmp eax,1
 jne failed
 invoke lstrlenA,partNeedleA
 stdcall TestContains,[body],[bodyBytes],partNeedleA,eax
 cmp eax,1
 jne failed
 mov ebx,slimNeedleA
 cmp [posts],1
 je .model
 mov ebx,classicNeedleA
.model:
 invoke lstrlenA,ebx
 stdcall TestContains,[body],[bodyBytes],ebx,eax
 cmp eax,1
 jne failed
 cmp [testMode],4
 je .unauthorized
 stdcall TestResponse,okA,2,200
 ret
.unauthorized:
 stdcall TestResponse,okA,2,401
 ret
.reset:
 inc [deletes]
 invoke lstrcmpiW,[targetW],skActiveW
 test eax,eax
 jnz failed
 cmp [bodyBytes],0
 jne failed
 stdcall TestResponse,emptyA,0,204
 ret
.texture:
 cmp [headersW],0
 jne failed
 cmp [bodyBytes],0
 jne failed
 cmp [testMode],3
 je .textureFail
 stdcall TestResponse,png64,png64End-png64,200
 ret
.textureFail:
 stdcall TestResponse,emptyA,0,503
 ret
endp

include '..\src\skin.inc'
include '..\src\common.inc'
include '..\src\version_settings.inc'
include '..\src\imports.inc'
