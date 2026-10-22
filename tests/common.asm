format PE console 4.0
entry start
include '..\tools\INCLUDE\win32ax.inc'

section '.data' data readable writeable
testJson db '{"nested":{"array":["one",2,true,null,{"escaped":"a\n\u4e2d\ud83d\ude00"}]},"num":-2147483648}',0
keyNested db 'nested',0
keyArray db 'array',0
keyEscaped db 'escaped',0
keyNum db 'num',0
bad1 db '{"x":1,}',0
bad2 db '[01]',0
bad3 db '[1e]',0
bad4 db 'falsex',0
bad5 db '"a',0
bad6 db '[,]',0
bad7 db '"\x"',0
unpaired db '"\ud800\u0041"',0
nulKey db '{"num\u0000suffix":1,"num":2}',0
boolJson db '[true,false,null,1]',0
rawJson db '["a\n",{"k":1}]',0
rawString db '"a\n"',0
rawObject db '{"k":1}',0
expected db 'a',10,0e4h,0b8h,0adh,0f0h,09fh,098h,080h,0
outfile du 'tests\hash.txt',0
digestExpected db 'a9993e364706816aba3e25717850c26c9cd0d89d',0
fixture db 'abc'
ctx dd 0
outbuf rb 256
digest rb 41
written dd 0
fileHandle dd 0
success db 'common: JSON, UTF8, strict syntax, truncation, SHA1 passed',13,10,0
failure db 'common: TEST FAILED',13,10,0
stdOut dd 0

section '.text' code readable executable
start:
    invoke GetStdHandle,-11
    mov [stdOut],eax
    stdcall JsonParse,rawJson
    mov [ctx],eax
    stdcall JsonRoot,eax
    stdcall JsonAt,[ctx],eax,0
    push eax
    stdcall JsonRaw,[ctx],eax,outbuf,256
    cmp eax,5
    pop eax
    jne fail
    push eax
    invoke lstrcmpA,outbuf,rawString
    test eax,eax
    pop eax
    jnz fail
    stdcall JsonRaw,[ctx],eax,outbuf,3
    cmp eax,5
    jne fail
    cmp byte [outbuf+2],0
    jne fail
    stdcall JsonRoot,[ctx]
    stdcall JsonAt,[ctx],eax,1
    stdcall JsonRaw,[ctx],eax,outbuf,256
    cmp eax,7
    jne fail
    invoke lstrcmpA,outbuf,rawObject
    test eax,eax
    jnz fail
    stdcall JsonRaw,[ctx],0,outbuf,256
    test eax,eax
    jnz fail
    cmp byte [outbuf],0
    jne fail
    stdcall JsonFree,[ctx]
    stdcall JsonParse,boolJson
    mov [ctx],eax
    stdcall JsonRoot,eax
    stdcall JsonAt,[ctx],eax,0
    stdcall JsonBool,[ctx],eax
    cmp eax,1
    jne fail
    stdcall JsonRoot,[ctx]
    stdcall JsonAt,[ctx],eax,1
    stdcall JsonBool,[ctx],eax
    test eax,eax
    jnz fail
    stdcall JsonRoot,[ctx]
    stdcall JsonAt,[ctx],eax,2
    stdcall JsonBool,[ctx],eax
    cmp eax,-1
    jne fail
    stdcall JsonBool,[ctx],0
    cmp eax,-1
    jne fail
    stdcall JsonRoot,[ctx]
    stdcall JsonAt,[ctx],eax,3
    stdcall JsonBool,[ctx],eax
    cmp eax,-1
    jne fail
    stdcall JsonFree,[ctx]
    stdcall JsonParse,testJson
    test eax,eax
    jz fail
    mov [ctx],eax
    stdcall JsonRoot,eax
    stdcall JsonCount,[ctx],eax
    cmp eax,2
    jne fail
    stdcall JsonRoot,[ctx]
    stdcall JsonGet,[ctx],eax,keyNested
    stdcall JsonGet,[ctx],eax,keyArray
    push eax
    stdcall JsonCount,[ctx],eax
    cmp eax,5
    pop eax
    jne fail
    stdcall JsonAt,[ctx],eax,4
    stdcall JsonGet,[ctx],eax,keyEscaped
    push eax
    stdcall JsonString,[ctx],eax,outbuf,256
    cmp eax,9
    pop eax
    jne fail
    push eax
    mov esi,outbuf
    mov edi,expected
    mov ecx,10
    repe cmpsb
    pop eax
    jne fail
    stdcall JsonString,[ctx],eax,outbuf,3
    cmp eax,9
    jne fail
    cmp byte [outbuf+2],0
    jne fail
    stdcall JsonRoot,[ctx]
    stdcall JsonGet,[ctx],eax,keyNum
    stdcall JsonInt,[ctx],eax
    cmp eax,80000000h
    jne fail
    stdcall JsonFree,[ctx]
    stdcall JsonParse,unpaired
    mov [ctx],eax
    test eax,eax
    jz fail
    stdcall JsonRoot,eax
    stdcall JsonString,[ctx],eax,outbuf,256
    cmp eax,4
    jne fail
    cmp dword [outbuf],041bdbfefh
    jne fail
    stdcall JsonFree,[ctx]
    stdcall JsonParse,nulKey
    mov [ctx],eax
    test eax,eax
    jz fail
    stdcall JsonRoot,eax
    stdcall JsonGet,[ctx],eax,keyNum
    stdcall JsonInt,[ctx],eax
    cmp eax,2
    jne fail
    stdcall JsonRoot,[ctx]
    stdcall JsonValueAt,[ctx],eax,0
    stdcall JsonNext,[ctx],eax
    stdcall JsonNext,[ctx],eax
    stdcall JsonInt,[ctx],eax
    cmp eax,2
    jne fail
    stdcall JsonFree,[ctx]
    stdcall JsonParse,bad1
    test eax,eax
    jnz fail
    stdcall JsonParse,bad2
    test eax,eax
    jnz fail
    stdcall JsonParse,bad3
    test eax,eax
    jnz fail
    stdcall JsonParse,bad4
    test eax,eax
    jnz fail
    stdcall JsonParse,bad5
    test eax,eax
    jnz fail
    stdcall JsonParse,bad6
    test eax,eax
    jnz fail
    stdcall JsonParse,bad7
    test eax,eax
    jnz fail
    invoke CreateFileW,outfile,40000000h,0,0,2,80h,0
    cmp eax,-1
    je fail
    mov [fileHandle],eax
    invoke WriteFile,eax,fixture,3,written,0
    invoke CloseHandle,[fileHandle]
    stdcall VerifySha1,outfile,digestExpected
    test eax,eax
    jz fail
    invoke lstrlenA,success
    invoke WriteFile,[stdOut],success,eax,written,0
    invoke ExitProcess,0
fail:
    invoke lstrlenA,failure
    invoke WriteFile,[stdOut],failure,eax,written,0
    invoke ExitProcess,1

include '..\src\common.inc'
section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL',winhttp,'WINHTTP.DLL',advapi32,'ADVAPI32.DLL'
import kernel32,GetProcessHeap,'GetProcessHeap',HeapAlloc,'HeapAlloc',HeapReAlloc,'HeapReAlloc',HeapFree,'HeapFree',lstrlenA,'lstrlenA',lstrlenW,'lstrlenW',lstrcmpA,'lstrcmpA',MultiByteToWideChar,'MultiByteToWideChar',GetFileAttributesW,'GetFileAttributesW',CreateDirectoryW,'CreateDirectoryW',CreateFileW,'CreateFileW',ReadFile,'ReadFile',WriteFile,'WriteFile',CloseHandle,'CloseHandle',DeleteFileW,'DeleteFileW',MoveFileExW,'MoveFileExW',GetFileSizeEx,'GetFileSizeEx',GetLastError,'GetLastError',ExitProcess,'ExitProcess',GetStdHandle,'GetStdHandle'
import winhttp,WinHttpOpen,'WinHttpOpen',WinHttpConnect,'WinHttpConnect',WinHttpOpenRequest,'WinHttpOpenRequest',WinHttpSendRequest,'WinHttpSendRequest',WinHttpReceiveResponse,'WinHttpReceiveResponse',WinHttpQueryHeaders,'WinHttpQueryHeaders',WinHttpQueryDataAvailable,'WinHttpQueryDataAvailable',WinHttpReadData,'WinHttpReadData',WinHttpCloseHandle,'WinHttpCloseHandle',WinHttpCrackUrl,'WinHttpCrackUrl',WinHttpSetTimeouts,'WinHttpSetTimeouts'
import advapi32,CryptAcquireContextW,'CryptAcquireContextW',CryptCreateHash,'CryptCreateHash',CryptHashData,'CryptHashData',CryptGetHashParam,'CryptGetHashParam',CryptDestroyHash,'CryptDestroyHash',CryptReleaseContext,'CryptReleaseContext'
