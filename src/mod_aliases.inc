; Original native Chinese name -> English search index. Input remains visible
; exactly as typed. IDs and compatibility always come from Modrinth responses.
section '.cndata' data readable writeable
include 'mod_aliases_data.inc'
align 4
cnMatched dd 0
cnSeparator db ' / ',0
cnInput rw 512
cnMapped rw 512
cnNormal rw 512
cnTitleBuffer rb 320

section '.cncode' code readable executable

; UTF-8 -> simplified Chinese, ASCII case fold, punctuation/spacing removed.
; The Windows mapping also handles traditional input without a bundled DLL.
proc CnNormalize uses ebx esi edi,source,destination
    invoke MultiByteToWideChar,65001,8,[source],-1,cnInput,512
    test eax,eax
    jz .bad
    invoke LCMapStringW,0804h,02000000h,cnInput,-1,cnMapped,512
    test eax,eax
    jz .bad
    mov esi,cnMapped
    mov edi,[destination]
.loop:
    lodsw
    test ax,ax
    jz .finished
    cmp ax,0FF01h
    jb .space
    cmp ax,0FF5Eh
    ja .space
    sub ax,0FEE0h
.space:
    cmp ax,32
    jbe .loop
    cmp ax,03000h
    jb .ascii
    cmp ax,0303Fh
    jbe .loop
.ascii:
    cmp ax,128
    jae .copy
    cmp ax,'0'
    jb .loop
    cmp ax,'9'
    jbe .copy
    cmp ax,'A'
    jb .loop
    cmp ax,'Z'
    ja .lower
    add ax,32
    jmp .copy
.lower:
    cmp ax,'a'
    jb .loop
    cmp ax,'z'
    ja .loop
.copy:
    stosw
    jmp .loop
.finished:
    mov word [edi],0
    mov eax,edi
    sub eax,[destination]
    shr eax,1
    ret
.bad:
    mov edi,[destination]
    mov word [edi],0
    xor eax,eax
    ret
endp

; UTF-16 substring test; empty needles are never matches.
proc CnContains uses ebx esi edi,haystack,needle
    mov esi,[haystack]
    mov ebx,[needle]
    cmp word [ebx],0
    je .no
.outer:
    cmp word [esi],0
    je .no
    mov edi,esi
    mov ecx,ebx
.inner:
    mov ax,[ecx]
    test ax,ax
    jz .yes
    cmp ax,[edi]
    jne .next
    add ecx,2
    add edi,2
    jmp .inner
.next:
    add esi,2
    jmp .outer
.yes:
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

; Exact aliases outrank partial aliases. Single-character names (Na/Li/etc.)
; only match exactly, preventing accidental matches in unrelated Chinese text.
; Unknown and blank queries fall through unchanged to the original provider.
proc CnResolve uses ebx esi edi,source
    locals
        length dd ?
        score dd ?
        best dd ?
    endl
    mov [cnMatched],0
    mov eax,[source]
    mov [best],eax
    mov [score],0
    stdcall CnNormalize,[source],cnNormal
    test eax,eax
    jz .done
    mov [length],eax
    mov esi,cnAliasTable
    mov edi,CN_ALIAS_COUNT
.loop:
    cmp [modCancel],0
    jne .cancel
    invoke lstrcmpiW,cnNormal,dword [esi]
    test eax,eax
    jz .exact
    cmp [length],2
    jb .next
    cmp dword [esi+4],2
    jb .next
    stdcall CnContains,dword [esi],cnNormal
    test eax,eax
    jz .next
    mov eax,1000
    sub eax,[esi+4]
    cmp eax,[score]
    jle .next
    mov [score],eax
    mov eax,[esi+8]
    mov [best],eax
    mov [cnMatched],1
.next:
    add esi,12
    dec edi
    jnz .loop
    jmp .done
.exact:
    mov eax,[esi+8]
    mov [best],eax
    mov [cnMatched],1
.done:
    mov eax,[best]
    ret
.cancel:
    mov [cnMatched],0
    mov eax,[source]
    ret
endp

; Add the known Chinese name to a verified official project identity, retaining
; the original title. Never infer identity from a fuzzy search result title.
proc CnDecorateTitle uses ebx esi edi,record
    mov ebx,[record]
if defined uiLang
    cmp [uiLang],0
    je .lookup
end if
    cmp [cnMatched],0
    je .done
.lookup:
    mov esi,cnProjectTable
    mov edi,CN_PROJECT_COUNT
.loop:
    invoke lstrcmpA,ebx,dword [esi]
    test eax,eax
    jz .found
    add esi,8
    dec edi
    jnz .loop
    ret
.found:
    lea eax,[ebx+MP_TITLE]
    invoke lstrcmpiA,eax,dword [esi+4]
    test eax,eax
    jz .done
    invoke lstrlenA,dword [esi+4]
    push eax
    lea eax,[ebx+MP_TITLE]
    invoke lstrlenA,eax
    pop ecx
    add eax,ecx
    add eax,3
    cmp eax,320
    jae .done
    invoke lstrcpyA,cnTitleBuffer,dword [esi+4]
    invoke lstrcatA,cnTitleBuffer,cnSeparator
    lea eax,[ebx+MP_TITLE]
    invoke lstrcatA,cnTitleBuffer,eax
    lea eax,[ebx+MP_TITLE]
    invoke lstrcpyA,eax,cnTitleBuffer
.done:
    ret
endp
