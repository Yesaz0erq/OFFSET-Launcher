format PE console 4.0
entry start
include 'win32ax.inc'

section '.data' data readable writeable
hWnd dd 0
authReady dd 0
authName rb 64
authUuid rb 64
authToken rb 8192
clientId rb 128
httpStatus dd 0
lastStatus dd 0
httpCalls dd 0
unitSource db 'A+B /?&=',0
unitFormExpected db 'A%2BB%20%2F%3F%26%3D',0
unitJsonSource db 'a"',92,13,10,9,1,'z',0
unitJsonExpected db 'a',92,'"',92,92,92,'u000D',92,'u000A',92,'u0009',92,'u0001','z',0
unitValidId db '01234567-89ab-CDEF-0123-456789abcdef',0
unitBadId db '01234567-89ab-CDEF-0123-456789abcdeg',0
unitOutput rb 256

section '.text' code readable executable
start:
    stdcall AuthFormEncode,unitSource,unitOutput,256
    cmp eax,1
    jne failed
    stdcall AuthEqual,unitOutput,unitFormExpected
    cmp eax,1
    jne failed
    stdcall AuthFormEncode,unitSource,unitOutput,4
    test eax,eax
    jnz failed
    cmp byte [unitOutput],0
    jne failed
    stdcall AuthJsonEscape,unitJsonSource,unitOutput,256
    cmp eax,1
    jne failed
    stdcall AuthEqual,unitOutput,unitJsonExpected
    cmp eax,1
    jne failed
    stdcall AuthJsonEscape,unitJsonSource,unitOutput,8
    test eax,eax
    jnz failed
    cmp byte [unitOutput],0
    jne failed
    invoke lstrcpyA,clientId,unitValidId
    stdcall AuthValidateClient
    cmp eax,1
    jne failed
    invoke lstrcpyA,clientId,unitBadId
    stdcall AuthValidateClient
    test eax,eax
    jnz failed
    mov byte [clientId],0
    stdcall AuthLogin,0
    test eax,eax
    jnz failed
    cmp [lastStatus],authMsgClient
    jne failed
    cmp [httpCalls],0
    jne failed
    cmp [authReady],0
    jne failed
    invoke lstrcpyA,clientId,unitBadId
    stdcall AuthLogin,0
    cmp [lastStatus],authMsgInvalidId
    jne failed
    cmp [httpCalls],0
    jne failed
    mov [authReady],1
    mov byte [authToken],'x'
    mov byte [authToken+8191],'y'
    mov byte [authName],'a'
    mov byte [authUuid],'b'
    stdcall AuthLogout
    cmp [authReady],0
    jne failed
    cmp [authCancel],1
    jne failed
    cmp byte [authToken],0
    jne failed
    cmp byte [authToken+8191],0
    jne failed
    cmp byte [authName],0
    jne failed
    cmp byte [authUuid],0
    jne failed
    invoke ExitProcess,0
failed:
    invoke ExitProcess,1
proc SetStatusA,pText
    mov eax,[pText]
    mov [lastStatus],eax
    ret
endp
proc HttpRequest,a,b,c,d
    inc [httpCalls]
    xor eax,eax
    ret
endp
proc FreeMem,a
    ret
endp
proc JsonFree,a
    ret
endp
proc JsonParse,a
    xor eax,eax
    ret
endp
proc JsonRoot,a
    xor eax,eax
    ret
endp
proc JsonGet,a,b,c
    xor eax,eax
    ret
endp
proc JsonString,a,b,c,d
    xor eax,eax
    ret
endp
proc JsonInt,a,b
    xor eax,eax
    ret
endp
proc JsonAt,a,b,c
    xor eax,eax
    ret
endp
proc JsonCount,a,b
    xor eax,eax
    ret
endp

include '../src/auth.inc'
include '../src/imports.inc'
