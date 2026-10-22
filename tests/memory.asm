format PE console 4.0
entry start
include '..\tools\INCLUDE\win32ax.inc'
section '.data' data readable writeable
n64 du '65536',0
n128 du '131072',0
nMax du '4294967295',0
nOverflow du '4294967296',0
nTooLong du '00000000001',0
nEmpty du '',0
nPlus du '+512',0
nMinus du '-512',0
nSpace du ' 512',0
nMixed du '512m',0
actualStatus db 64,63 dup(0)
actualLimit dd 0
section '.text' code readable executable
start:
    stdcall MemoryFromBytes,0,1
    cmp eax,4096
    jne fail1
    stdcall MemoryFromBytes,0FFFFFFFFh,0
    cmp eax,4095
    jne fail1
    stdcall MemoryFromBytes,0,8
    cmp eax,32768
    jne fail1
    stdcall MemoryFromBytes,0,32
    cmp eax,131072
    jne fail1
    stdcall MemoryFromBytes,0FFFFFFFFh,0FFFFFh
    cmp eax,0FFFFFFFFh
    jne fail1
    stdcall MemoryFromBytes,0,100000h
    cmp eax,0FFFFFFFFh
    jne fail1
    stdcall MemoryFromBytes,0FFFFFFFFh,0FFFFFFFFh
    cmp eax,0FFFFFFFFh
    jne fail1
    stdcall MemoryFromBytes,0,0
    cmp eax,512
    jne fail1
    stdcall MemoryFromBytes,001FFFFFh,0
    cmp eax,512
    jne fail1
    mov [memoryLimitMb],131072
    stdcall MemoryValid,65536
    cmp eax,1
    jne fail2
    stdcall MemoryValid,131072
    cmp eax,1
    jne fail2
    stdcall MemoryValid,131073
    test eax,eax
    jnz fail2
    stdcall MemoryValid,511
    test eax,eax
    jnz fail2
    mov [memoryLimitMb],0FFFFFFFFh
    stdcall MemoryValid,0FFFFFFFFh
    cmp eax,1
    jne fail2
    stdcall MemoryParseW,n64
    cmp eax,65536
    jne fail3
    stdcall MemoryParseW,n128
    cmp eax,131072
    jne fail3
    stdcall MemoryParseW,nMax
    cmp eax,0FFFFFFFFh
    jne fail3
    stdcall MemoryParseW,nOverflow
    test eax,eax
    jnz fail3
    stdcall MemoryParseW,nTooLong
    test eax,eax
    jnz fail3
    stdcall MemoryParseW,nEmpty
    test eax,eax
    jnz fail3
    stdcall MemoryParseW,nPlus
    test eax,eax
    jnz fail3
    stdcall MemoryParseW,nMinus
    test eax,eax
    jnz fail3
    stdcall MemoryParseW,nSpace
    test eax,eax
    jnz fail3
    stdcall MemoryParseW,nMixed
    test eax,eax
    jnz fail3
    ; Compare the actual API's 64-bit total physical memory to our result.
    invoke GlobalMemoryStatusEx,actualStatus
    test eax,eax
    jz fail4
    stdcall MemoryFromBytes,dword [actualStatus+8],dword [actualStatus+12]
    mov [actualLimit],eax
    stdcall MemoryDetect
    cmp eax,[actualLimit]
    jne fail4
    cmp eax,[memoryLimitMb]
    jne fail4
    stdcall MemoryValid,eax
    cmp eax,1
    jne fail4
    invoke ExitProcess,0
fail1: invoke ExitProcess,11
fail2: invoke ExitProcess,12
fail3: invoke ExitProcess,13
fail4: invoke ExitProcess,14
include '..\src\memory.inc'
section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL'
import kernel32,GlobalMemoryStatusEx,'GlobalMemoryStatusEx',ExitProcess,'ExitProcess'
