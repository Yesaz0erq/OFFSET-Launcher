; Exercises cold session initialization and per-request status under concurrency.
format PE console 4.0
entry start
stack 100000h,40000h
include '..\tools\INCLUDE\win32ax.inc'
section '.data' data readable writeable
parallelGood db 'https://piston-meta.mojang.com/mc/game/version_manifest_v2.json',0
parallelBad db 'https://resources.download.minecraft.net/00/0000000000000000000000000000000000000000',0
parallelPaths dd path0,path1,path2,path3,path4,path5,path6,path7
path0 du 'tests\cache\parallel0.json',0
path1 du 'tests\cache\parallel1.json',0
path2 du 'tests\cache\parallel2.json',0
path3 du 'tests\cache\parallel3.json',0
path4 du 'tests\cache\parallel4.json',0
path5 du 'tests\cache\parallel5.json',0
path6 du 'tests\cache\parallel6.json',0
path7 du 'tests\cache\parallel7.json',0
parallelHandles rd 8
parallelResults rd 8
section '.text' code readable executable
start:
    xor esi,esi
.spawn:
    invoke CreateThread,0,0,ParallelWorker,esi,0,0
    test eax,eax
    jz fail
    mov [parallelHandles+esi*4],eax
    inc esi
    cmp esi,8
    jb .spawn
    xor esi,esi
.wait:
    invoke WaitForSingleObject,dword [parallelHandles+esi*4],-1
    invoke CloseHandle,dword [parallelHandles+esi*4]
    cmp dword [parallelResults+esi*4],1
    jne fail
    inc esi
    cmp esi,8
    jb .wait
    invoke ExitProcess,0
fail:
    invoke ExitProcess,1
proc ParallelWorker uses ebx esi edi, index
    mov esi,[index]
    mov eax,parallelGood
    test esi,1
    jz .download
    mov eax,parallelBad
.download:
    stdcall HttpDownload,eax,dword [parallelPaths+esi*4],0
    test esi,1
    jz .expected
    xor eax,1
.expected:
    mov [parallelResults+esi*4],eax
    ret
endp
include '..\src\common.inc'
include '..\src\imports.inc'
