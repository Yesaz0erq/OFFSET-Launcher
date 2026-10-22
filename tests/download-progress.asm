; Concurrent progress accounting across the 4 GiB boundary.
format PE console 6.0
entry start
include 'win32ax.inc'
section '.data' data readable writeable
threads rd 8
snapshot rd 2
section '.text' code readable executable
start:
 xor esi,esi
.spawn:
 invoke CreateThread,0,0,CountWorker,0,0,0
 test eax,eax
 jz failed
 mov [threads+esi*4],eax
 inc esi
 cmp esi,8
 jb .spawn
 xor esi,esi
.join:
 invoke WaitForSingleObject,dword [threads+esi*4],-1
 invoke CloseHandle,dword [threads+esi*4]
 inc esi
 cmp esi,8
 jb .join
 stdcall DownloadSnapshot,snapshot
 cmp dword [snapshot],4194304
 jne failed
 cmp dword [snapshot+4],65536
 jne failed
 cmp dword [cDownloadBytes],0
 jne failed
 cmp dword [cDownloadBytes+4],1
 jne failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,1
proc CountWorker uses ebx,param
 mov ebx,8192
.count:
 stdcall DownloadCount,65536,1
 dec ebx
 jnz .count
 ret
endp
include '..\src\common.inc'
include '..\src\imports.inc'
