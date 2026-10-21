; OFFSET: physical-memory limit, independent of the x86 launcher's address space.
section '.memdata' data readable writeable
memoryLimitMb dd 512

section '.memcode' code readable executable
; Keep the section valid in installer-only builds that omit every memory proc.
    nop
; DWORD MiB limit from a 64-bit physical byte count. Saturate, never wrap.
proc MemoryFromBytes,lowBytes,highBytes
    mov eax,[lowBytes]
    mov edx,[highBytes]
    cmp edx,100000h
    jae .maximum
    shrd eax,edx,20
    cmp eax,512
    jae .done
    mov eax,512
    ret
.maximum:
    mov eax,0FFFFFFFFh
.done:
    ret
endp

proc MemoryDetect uses edi
    local status[64]:BYTE
    lea edi,[status]
    xor eax,eax
    mov ecx,16
    rep stosd
    mov dword [status],64
    invoke GlobalMemoryStatusEx,addr status
    test eax,eax
    jz .failed
    stdcall MemoryFromBytes,dword [status+8],dword [status+12]
    jmp .store
.failed:
    mov eax,512
.store:
    mov [memoryLimitMb],eax
    ret
endp

proc MemoryValid,value
    mov eax,[value]
    cmp eax,512
    jb .bad
    cmp eax,[memoryLimitMb]
    ja .bad
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; Exact decimal uint32, at most ten digits. Return 0 for invalid input.
proc MemoryParseW uses esi,source
    mov esi,[source]
    xor eax,eax
    xor ecx,ecx
.digit:
    movzx edx,word [esi+ecx*2]
    test edx,edx
    jz .done
    cmp ecx,10
    jae .bad
    sub edx,'0'
    cmp edx,9
    ja .bad
    cmp eax,429496729
    ja .bad
    jb .append
    cmp edx,5
    ja .bad
.append:
    imul eax,10
    add eax,edx
    inc ecx
    jmp .digit
.done:
    test ecx,ecx
    jz .bad
    ret
.bad:
    xor eax,eax
    ret
endp
