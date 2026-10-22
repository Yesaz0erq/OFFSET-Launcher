; OFFSET native support. Original code; FASM 1.73 / Win32 stdcall.
; JSON contexts borrow the input buffer. Token layout is private.
J_STR = 1
J_OBJ = 2
J_ARR = 3
J_NUM = 4
J_LIT = 5
J_SIZE = 20

section '.cdata' data readable writeable
httpStatus dd 0
cSession dd 0
cAgent du 'OFFSET/0.9.0',0
cGet du 'GET',0
cHex db '0123456789abcdef',0
cPart du '.part',0
cDownloadLock dd 0
cDownloadBytes dq 0
cDownloadFiles dd 0

section '.ctext' code readable executable

; Short critical section shared by download workers; no I/O or UI messages.
proc DownloadCount,bytes,files
.lock:
    mov eax,1
    xchg eax,[cDownloadLock]
    test eax,eax
    jnz .lock
    mov eax,[bytes]
    add dword [cDownloadBytes],eax
    adc dword [cDownloadBytes+4],0
    mov eax,[files]
    add [cDownloadFiles],eax
    mov [cDownloadLock],0
    ret
endp

; Snapshot in KiB (up to 4 TiB) and successfully published network files.
proc DownloadSnapshot,destination
.lock:
    mov eax,1
    xchg eax,[cDownloadLock]
    test eax,eax
    jnz .lock
    mov eax,dword [cDownloadBytes]
    mov edx,dword [cDownloadBytes+4]
    shrd eax,edx,10
    mov edx,[destination]
    mov [edx],eax
    mov eax,[cDownloadFiles]
    mov [edx+4],eax
    mov [cDownloadLock],0
    ret
endp

proc FreeMem p
    cmp [p],0
    je .done
    invoke GetProcessHeap
    invoke HeapFree,eax,0,[p]
.done:
    ret
endp

proc JsonWhite uses esi, p
    mov esi,[p]
.loop:
    mov al,byte [esi]
    cmp al,' '
    je .next
    cmp al,9
    je .next
    cmp al,10
    je .next
    cmp al,13
    jne .done
.next:
    inc esi
    jmp .loop
.done:
    mov eax,esi
    ret
endp

; Parse one token recursively, max depth 128. Cursor is a pointer to a pointer.
proc JsonValue uses ebx esi edi, ctx, pScan, depth
    locals
        token dd ?
        start dd ?
        kind dd ?
        n dd ?
    endl
    cmp [depth],128
    ja .bad
    mov ebx,[ctx]
    mov eax,[pScan]
    stdcall JsonWhite,dword [eax]
    mov esi,eax
    mov [start],eax
    mov eax,dword [ebx+4]
    cmp eax,dword [ebx+8]
    jae .bad
    imul eax,J_SIZE
    lea edi,[ebx+eax+16]
    mov [token],edi
    inc dword [ebx+4]
    mov dword [edi+16],0
    mov [edi+4],esi
    mov dword [edi+8],0
    mov al,byte [esi]
    cmp al,'"'
    je .string
    cmp al,'{'
    je .object
    cmp al,'['
    je .array
    cmp al,'t'
    je .true
    cmp al,'f'
    je .false
    cmp al,'n'
    je .null
    cmp al,'-'
    je .number
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .number
    jmp .bad
.string:
    mov dword [edi],J_STR
    inc esi
    mov [edi+4],esi
.strloop:
    mov al,byte [esi]
    cmp al,'"'
    je .strend
    cmp al,32
    jb .bad
    cmp al,92
    jne .strnext
    inc esi
    mov al,byte [esi]
    cmp al,'u'
    je .unicode
    cmp al,'"'
    je .strnext
    cmp al,92
    je .strnext
    cmp al,'/'
    je .strnext
    cmp al,'b'
    je .strnext
    cmp al,'f'
    je .strnext
    cmp al,'n'
    je .strnext
    cmp al,'r'
    je .strnext
    cmp al,'t'
    jne .bad
    jmp .strnext
.unicode:
    mov ecx,4
.hexloop:
    inc esi
    mov al,byte [esi]
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .hexok
    or al,32
    cmp al,'a'
    jb .bad
    cmp al,'f'
    ja .bad
.hexok:
    loop .hexloop
.strnext:
    inc esi
    jmp .strloop
.strend:
    mov eax,esi
    sub eax,dword [edi+4]
    mov [edi+8],eax
    inc esi
    jmp .finish
.object:
    mov [kind],J_OBJ
    mov dword [edi],J_OBJ
    jmp .container
.array:
    mov [kind],J_ARR
    mov dword [edi],J_ARR
.container:
    inc esi
    stdcall JsonWhite,esi
    mov esi,eax
    mov dl,'}'
    cmp [kind],J_OBJ
    je .emptycheck
    mov dl,']'
.emptycheck:
    cmp [esi],dl
    je .containerend
.child:
    mov eax,[pScan]
    mov [eax],esi
    cmp [kind],J_OBJ
    jne .arraychild
    cmp byte [esi],'"'
    jne .bad
    mov eax,[depth]
    inc eax
    stdcall JsonValue,[ctx],[pScan],eax
    test eax,eax
    jz .bad
    mov eax,[pScan]
    stdcall JsonWhite,dword [eax]
    mov esi,eax
    cmp byte [esi],':'
    jne .bad
    inc esi
    mov eax,[pScan]
    mov [eax],esi
.arraychild:
    mov eax,[depth]
    inc eax
    stdcall JsonValue,[ctx],[pScan],eax
    test eax,eax
    jz .bad
    mov edi,[token]
    inc dword [edi+16]
    mov eax,[pScan]
    stdcall JsonWhite,dword [eax]
    mov esi,eax
    mov dl,'}'
    cmp [kind],J_OBJ
    je .closecheck
    mov dl,']'
.closecheck:
    cmp [esi],dl
    je .containerend
    cmp byte [esi],','
    jne .bad
    inc esi
    stdcall JsonWhite,esi
    mov esi,eax
    jmp .child
.containerend:
    inc esi
    mov edi,[token]
    mov eax,esi
    sub eax,[start]
    mov [edi+8],eax
    jmp .finish
.true:
    cmp dword [esi],'true'
    jne .bad
    add esi,4
    jmp .literal
.false:
    cmp dword [esi],'fals'
    jne .bad
    cmp byte [esi+4],'e'
    jne .bad
    add esi,5
    jmp .literal
.null:
    cmp dword [esi],'null'
    jne .bad
    add esi,4
.literal:
    mov dword [edi],J_LIT
    jmp .rawend
.number:
    mov dword [edi],J_NUM
    cmp byte [esi],'-'
    jne .positive
    inc esi
.positive:
    cmp byte [esi],'0'
    jne .nonzero
    inc esi
    jmp .fraction
.nonzero:
    cmp byte [esi],'1'
    jb .bad
    cmp byte [esi],'9'
    ja .bad
.digits:
    inc esi
    cmp byte [esi],'0'
    jb .fraction
    cmp byte [esi],'9'
    jbe .digits
.fraction:
    cmp byte [esi],'.'
    jne .exponent
    inc esi
    cmp byte [esi],'0'
    jb .bad
    cmp byte [esi],'9'
    ja .bad
.fracdigits:
    inc esi
    cmp byte [esi],'0'
    jb .exponent
    cmp byte [esi],'9'
    jbe .fracdigits
.exponent:
    mov al,byte [esi]
    or al,32
    cmp al,'e'
    jne .rawend
    inc esi
    cmp byte [esi],'+'
    je .expsign
    cmp byte [esi],'-'
    jne .expfirst
.expsign:
    inc esi
.expfirst:
    cmp byte [esi],'0'
    jb .bad
    cmp byte [esi],'9'
    ja .bad
.expdigits:
    inc esi
    cmp byte [esi],'0'
    jb .rawend
    cmp byte [esi],'9'
    jbe .expdigits
.rawend:
    mov eax,esi
    sub eax,[start]
    mov [edi+8],eax
.finish:
    mov ebx,[ctx]
    mov eax,dword [ebx+4]
    imul eax,J_SIZE
    lea eax,[ebx+eax+16]
    mov [edi+12],eax
    mov eax,[pScan]
    mov [eax],esi
    mov eax,[token]
    ret
.bad:
    xor eax,eax
    ret
endp

proc JsonParse uses ebx esi, source
    local pScan:DWORD, capacity:DWORD
    cmp [source],0
    je .bad
    invoke lstrlenA,[source]
    test eax,eax
    jz .bad
    cmp eax,33554432
    ja .bad
    shr eax,1
    add eax,16
    mov [capacity],eax
    imul eax,J_SIZE
    add eax,16
    mov ebx,eax
    invoke GetProcessHeap
    invoke HeapAlloc,eax,8,ebx
    test eax,eax
    jz .bad
    mov ebx,eax
    mov eax,[source]
    mov [ebx],eax
    mov [pScan],eax
    mov eax,[capacity]
    mov [ebx+8],eax
    stdcall JsonValue,ebx,addr pScan,0
    test eax,eax
    jz .freebad
    stdcall JsonWhite,[pScan]
    cmp byte [eax],0
    jne .freebad
    mov eax,ebx
    ret
.freebad:
    stdcall FreeMem,ebx
.bad:
    xor eax,eax
    ret
endp

proc JsonFree ctx
    stdcall FreeMem,[ctx]
    ret
endp

proc JsonRoot ctx
    mov eax,[ctx]
    test eax,eax
    jz .done
    add eax,16
.done:
    ret
endp

proc JsonCount ctx, token
    mov eax,[token]
    test eax,eax
    jz .done
    mov eax,dword [eax+16]
.done:
    ret
endp

proc JsonAt uses esi,ctx, token, index
    mov esi,[token]
    test esi,esi
    jz .bad
    cmp dword [esi],J_ARR
    jne .bad
    mov ecx,[index]
    cmp ecx,dword [esi+16]
    jae .bad
    add esi,J_SIZE
.loop:
    test ecx,ecx
    jz .done
    mov esi,dword [esi+12]
    dec ecx
    jmp .loop
.done:
    mov eax,esi
    ret
.bad:
    xor eax,eax
    ret
endp

proc JsonKeyAt uses esi,ctx, token, index
    mov esi,[token]
    test esi,esi
    jz .bad
    cmp dword [esi],J_OBJ
    jne .bad
    mov ecx,[index]
    cmp ecx,dword [esi+16]
    jae .bad
    add esi,J_SIZE
.loop:
    test ecx,ecx
    jz .done
    mov esi,dword [esi+12]
    mov esi,dword [esi+12]
    dec ecx
    jmp .loop
.done:
    mov eax,esi
    ret
.bad:
    xor eax,eax
    ret
endp

proc JsonValueAt ctx,token,index
    stdcall JsonKeyAt,[ctx],[token],[index]
    test eax,eax
    jz .done
    mov eax,dword [eax+12]
.done:
    ret
endp

; Sequential traversal skips a token's complete subtree in constant time.
proc JsonNext ctx,token
    mov eax,[ctx]
    test eax,eax
    jz .done
    mov edx,[eax+4]
    imul edx,J_SIZE
    lea edx,[eax+edx+16]
    mov eax,[token]
    test eax,eax
    jz .done
    mov eax,[eax+12]
    cmp eax,edx
    jb .done
    xor eax,eax
.done:
    ret
endp

proc JsonGet uses ebx esi edi,ctx, token, key
    locals
        keybuf rb 256
        left dd ?
        keylen dd ?
    endl
    mov esi,[token]
    test esi,esi
    jz .bad
    cmp dword [esi],J_OBJ
    jne .bad
    invoke lstrlenA,[key]
    mov [keylen],eax
    mov eax,dword [esi+16]
    mov [left],eax
    add esi,J_SIZE
.loop:
    cmp [left],0
    je .bad
    stdcall JsonString,[ctx],esi,addr keybuf,256
    cmp eax,[keylen]
    jne .skip
    cmp eax,255
    ja .skip
    lea ebx,[keybuf]
    mov edi,[key]
.compare:
    mov al,byte [ebx]
    cmp al,byte [edi]
    jne .skip
    inc ebx
    inc edi
    test al,al
    jnz .compare
    mov eax,dword [esi+12]
    ret
.skip:
    mov esi,dword [esi+12]
    mov esi,dword [esi+12]
    dec [left]
    jmp .loop
.bad:
    xor eax,eax
    ret
endp

; ECX codepoint -> 1..4 UTF8 bytes in EAX, EDX count. Little endian.
proc JsonUtf8 uses ebx, code
    mov ecx,[code]
    cmp ecx,7fh
    ja .two
    mov eax,ecx
    mov edx,1
    ret
.two:
    cmp ecx,7ffh
    ja .three
    mov eax,ecx
    shr eax,6
    or eax,0c0h
    and ecx,3fh
    or ecx,80h
    shl ecx,8
    or eax,ecx
    mov edx,2
    ret
.three:
    cmp ecx,0ffffh
    ja .four
    mov eax,ecx
    shr eax,12
    or eax,0e0h
    mov ebx,ecx
    shr ebx,6
    and ebx,3fh
    or ebx,80h
    shl ebx,8
    or eax,ebx
    and ecx,3fh
    or ecx,80h
    shl ecx,16
    or eax,ecx
    mov edx,3
    ret
.four:
    mov eax,ecx
    shr eax,18
    or eax,0f0h
    mov ebx,ecx
    shr ebx,12
    and ebx,3fh
    or ebx,80h
    shl ebx,8
    or eax,ebx
    mov ebx,ecx
    shr ebx,6
    and ebx,3fh
    or ebx,80h
    shl ebx,16
    or eax,ebx
    and ecx,3fh
    or ecx,80h
    shl ecx,24
    or eax,ecx
    mov edx,4
    ret
endp

proc JsonHex4 uses esi, p
    mov esi,[p]
    xor eax,eax
    mov ecx,4
.loop:
    shl eax,4
    movzx edx,byte [esi]
    cmp dl,'9'
    jbe .digit
    or dl,32
    sub dl,'a'-10
    jmp .put
.digit:
    sub dl,'0'
.put:
    or eax,edx
    inc esi
    loop .loop
    ret
endp

proc JsonString uses ebx esi edi,ctx,token,dest,capacity
    locals
        last dd ?
        needed dd ?
        surrogate dd ?
    endl
    mov [needed],0
    mov edi,[dest]
    cmp [capacity],0
    je .token
    mov byte [edi],0
.token:
    mov eax,[token]
    test eax,eax
    jz .finish
    cmp dword [eax],J_STR
    jne .finish
    mov esi,dword [eax+4]
    mov edx,dword [eax+8]
    add edx,esi
    mov [last],edx
.loop:
    cmp esi,[last]
    jae .finish
    movzx eax,byte [esi]
    inc esi
    mov edx,1
    cmp al,92
    jne .emit
    movzx eax,byte [esi]
    inc esi
    cmp al,'u'
    je .unicode
    cmp al,'b'
    jne .form
    mov eax,8
    jmp .emit
.form:
    cmp al,'f'
    jne .newline
    mov eax,12
    jmp .emit
.newline:
    cmp al,'n'
    jne .return
    mov eax,10
    jmp .emit
.return:
    cmp al,'r'
    jne .tab
    mov eax,13
    jmp .emit
.tab:
    cmp al,'t'
    jne .emit
    mov eax,9
    jmp .emit
.unicode:
    stdcall JsonHex4,esi
    add esi,4
    cmp eax,0d800h
    jb .encode
    cmp eax,0dbffh
    ja .lowcheck
    mov [surrogate],eax
    lea eax,dword [esi+6]
    cmp eax,[last]
    ja .replacement
    cmp word [esi],0755ch
    jne .replacement
    lea eax,[esi+2]
    stdcall JsonHex4,eax
    cmp eax,0dc00h
    jb .replacement
    cmp eax,0dfffh
    ja .replacement
    add esi,6
    sub eax,0dc00h
    mov ecx,[surrogate]
    sub ecx,0d800h
    shl ecx,10
    add eax,ecx
    add eax,10000h
    jmp .encode
.lowcheck:
    cmp eax,0dfffh
    ja .encode
.replacement:
    mov eax,0fffdh
.encode:
    stdcall JsonUtf8,eax
.emit:
    mov ecx,edx
.byte:
    mov ebx,[needed]
    inc ebx
    cmp ebx,[capacity]
    jae .nocopy
    mov ebx,[needed]
    mov [edi+ebx],al
.nocopy:
    inc [needed]
    shr eax,8
    loop .byte
    jmp .loop
.finish:
    mov eax,[needed]
    mov ecx,[capacity]
    test ecx,ecx
    jz .done
    dec ecx
    cmp eax,ecx
    jae .terminate
    mov ecx,eax
.terminate:
    mov byte [edi+ecx],0
.done:
    ret
endp

; Return true=1, false=0, missing/null/nonboolean=-1.
; Exact source JSON, including quotes for strings. Same truncation contract as JsonString.
proc JsonRaw uses ebx esi edi,ctx,token,dest,capacity
    xor ebx,ebx
    cmp [ctx],0
    je .empty
    mov eax,[token]
    test eax,eax
    jz .empty
    cmp dword [eax],J_STR
    jb .empty
    cmp dword [eax],J_LIT
    ja .empty
    mov esi,[eax+4]
    mov ebx,[eax+8]
    cmp dword [eax],J_STR
    jne .copy
    dec esi
    add ebx,2
.copy:
    mov ecx,[capacity]
    test ecx,ecx
    jz .done
    dec ecx
    cmp ecx,ebx
    jbe .bounded
    mov ecx,ebx
.bounded:
    mov edi,[dest]
    rep movsb
    mov byte [edi],0
    jmp .done
.empty:
    cmp [capacity],0
    je .done
    mov edi,[dest]
    mov byte [edi],0
.done:
    mov eax,ebx
    ret
endp

proc JsonBool uses esi,ctx,token
    mov eax,[token]
    test eax,eax
    jz .invalid
    cmp dword [eax],J_LIT
    jne .invalid
    mov esi,[eax+4]
    cmp dword [eax+8],4
    jne .false
    cmp dword [esi],065757274h
    jne .invalid
    mov eax,1
    ret
.false:
    cmp dword [eax+8],5
    jne .invalid
    cmp dword [esi],0736C6166h
    jne .invalid
    cmp byte [esi+4],'e'
    jne .invalid
    xor eax,eax
    ret
.invalid:
    or eax,-1
    ret
endp

proc JsonInt uses ebx esi edi,ctx,token
    mov esi,[token]
    test esi,esi
    jz .bad
    cmp dword [esi],J_NUM
    jne .bad
    mov ecx,dword [esi+8]
    mov esi,dword [esi+4]
    xor edi,edi
    xor eax,eax
    cmp byte [esi],'-'
    jne .loop
    inc esi
    dec ecx
    mov edi,1
.loop:
    test ecx,ecx
    jz .done
    movzx ebx,byte [esi]
    sub ebx,'0'
    cmp ebx,9
    ja .bad
    cmp eax,214748364
    ja .bad
    imul eax,10
    add eax,ebx
    cmp eax,80000000h
    ja .bad
    inc esi
    dec ecx
    jmp .loop
.done:
    test edi,edi
    jnz .negative
    test eax,eax
    js .bad
    ret
.negative:
    neg eax
    ret
.bad:
    xor eax,eax
    ret
endp

proc EnsureDirectory uses ebx esi edi,path
    local buffer[2048]:WORD
    invoke lstrlenW,[path]
    cmp eax,2047
    ja .bad
    test eax,eax
    jz .bad
    mov ecx,eax
    inc ecx
    mov esi,[path]
    lea edi,[buffer]
    rep movsw
    lea esi,[buffer]
    xor ebx,ebx
    cmp word [esi+2],':'
    jne .scan
    cmp word [esi+4],92
    je .driveroot
    cmp word [esi+4],'/'
    jne .bad
.driveroot:
    add esi,6
.scan:
    mov ax,word [esi]
    test ax,ax
    jz .final
    cmp ax,92
    je .sep
    cmp ax,'/'
    jne .next
.sep:
    mov [esi],bx
    invoke CreateDirectoryW,addr buffer,0
    mov word [esi],92
.next:
    add esi,2
    jmp .scan
.final:
    invoke CreateDirectoryW,addr buffer,0
    invoke GetFileAttributesW,addr buffer
    cmp eax,-1
    je .bad
    and eax,10h
    setnz al
    movzx eax,al
    ret
.bad:
    xor eax,eax
    ret
endp

; Open HTTPS, retaining all handles in caller's 3-DWORD array.
proc NetStart uses ebx esi edi,url,method,headers,body,handles,allowRedirect,statusOut
    locals
        uc rb 60
        host rw 256
        target rw 8192
        bodylen dd ?
        headerlen dd ?
        size dd ?
        redirectPolicy dd ?
        sessionCandidate dd ?
    endl
    mov edi,[handles]
    xor eax,eax
    mov [edi],eax
    mov [edi+4],eax
    mov [edi+8],eax
    lea edi,[uc]
    mov ecx,15
    rep stosd
    mov dword [uc],60
    mov dword [uc+8],-1
    mov dword [uc+20],-1
    mov dword [uc+32],-1
    mov dword [uc+40],-1
    mov dword [uc+48],-1
    mov dword [uc+56],-1
    invoke WinHttpCrackUrl,[url],0,0,addr uc
    test eax,eax
    jz .bad
    cmp dword [uc+12],2
    jne .bad
    cmp dword [uc+32],0
    jne .bad
    cmp dword [uc+40],0
    jne .bad
    mov ecx,dword [uc+20]
    test ecx,ecx
    jz .bad
    cmp ecx,255
    ja .bad
    mov esi,dword [uc+16]
    lea edi,[host]
    rep movsw
    mov word [edi],0
    mov ecx,dword [uc+48]
    mov eax,dword [uc+56]
    add eax,ecx
    cmp eax,8191
    ja .bad
    mov esi,dword [uc+44]
    lea edi,[target]
    rep movsw
    mov ecx,dword [uc+56]
    mov esi,dword [uc+52]
.extra:
    test ecx,ecx
    jz .targetdone
    lodsw
    cmp ax,'#'
    je .bad
    stosw
    dec ecx
    jmp .extra
.targetdone:
    mov word [edi],0
    ; Keep one process-lifetime session so asset workers share WinHTTP's TLS
    ; connection pool. Each operation still owns its connect/request handles.
    mov eax,[cSession]
    test eax,eax
    jnz .sessionready
    invoke WinHttpOpen,cAgent,0,0,0,0
    test eax,eax
    jz .bad
    mov [sessionCandidate],eax
    invoke WinHttpSetTimeouts,eax,15000,15000,30000,30000
    mov edx,[sessionCandidate]
    xor eax,eax
    lock cmpxchg [cSession],edx
    jz .sessionpublished
    ; Another worker published its fully configured session first.
    invoke WinHttpCloseHandle,[sessionCandidate]
.sessionpublished:
    mov eax,[cSession]
.sessionready:
    mov ebx,[handles]
    mov [ebx],eax
    movzx eax,word [uc+24]
    invoke WinHttpConnect,dword [ebx],addr host,eax,0
    mov [ebx+4],eax
    test eax,eax
    jz .bad
    mov ecx,[method]
    test ecx,ecx
    jnz .methodok
    mov ecx,cGet
.methodok:
    invoke WinHttpOpenRequest,eax,ecx,addr target,0,0,0,00800000h
    mov [ebx+8],eax
    test eax,eax
    jz .bad
    cmp [allowRedirect],0
    jne .redirectdone
    mov [redirectPolicy],0
    invoke WinHttpSetOption,eax,88,addr redirectPolicy,4
    test eax,eax
    jz .bad
.redirectdone:
    mov [bodylen],0
    cmp [body],0
    je .headers
    invoke lstrlenA,[body]
    mov [bodylen],eax
.headers:
    mov [headerlen],0
    cmp [headers],0
    je .send
    mov [headerlen],-1
.send:
    invoke WinHttpSendRequest,dword [ebx+8],[headers],[headerlen],[body],[bodylen],[bodylen],0
    test eax,eax
    jz .bad
    invoke WinHttpReceiveResponse,dword [ebx+8],0
    test eax,eax
    jz .bad
    mov [size],4
    invoke WinHttpQueryHeaders,dword [ebx+8],20000013h,0,[statusOut],addr size,0
    test eax,eax
    jz .bad
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc NetClose uses esi,handles
    mov esi,[handles]
    cmp dword [esi+8],0
    je .connection
    invoke WinHttpCloseHandle,dword [esi+8]
.connection:
    cmp dword [esi+4],0
    je .session
    invoke WinHttpCloseHandle,dword [esi+4]
.session:
    ; cSession intentionally survives until process exit for connection reuse.
.done:
    ret
endp

proc HttpRequest uses ebx esi edi,url,method,headers,body
    locals
        handles rd 3
        buffer dd ?
        total dd ?
        capacity dd ?
        got dd ?
    endl
    mov [httpStatus],0
    mov [buffer],0
    mov [total],0
    mov [capacity],65537
    stdcall NetStart,[url],[method],[headers],[body],addr handles,0,httpStatus
    test eax,eax
    jz .fail
    invoke GetProcessHeap
    invoke HeapAlloc,eax,0,[capacity]
    mov [buffer],eax
    test eax,eax
    jz .fail
.read:
    mov eax,[total]
    add eax,65537
    cmp eax,[capacity]
    jbe .space
    cmp eax,33554433
    ja .fail
    mov [capacity],eax
    mov ebx,eax
    invoke GetProcessHeap
    invoke HeapReAlloc,eax,0,[buffer],ebx
    test eax,eax
    jz .fail
    mov [buffer],eax
.space:
    mov eax,[buffer]
    add eax,[total]
    invoke WinHttpReadData,[handles+8],eax,65536,addr got
    test eax,eax
    jz .fail
    cmp [got],0
    je .success
    mov eax,[got]
    add [total],eax
    jmp .read
.success:
    mov eax,[buffer]
    mov ecx,[total]
    mov byte [eax+ecx],0
    stdcall NetClose,addr handles
    mov eax,[buffer]
    ret
.fail:
    stdcall NetClose,addr handles
    stdcall FreeMem,[buffer]
    xor eax,eax
    ret
endp

proc Sha1File uses ebx esi edi,path,output
    locals
        provider dd ?
        hash dd ?
        fileHandle dd ?
        buffer dd ?
        got dd ?
        digest rb 20
        size dd ?
        result dd ?
    endl
    mov [provider],0
    mov [hash],0
    mov [fileHandle],-1
    mov [buffer],0
    mov [result],0
    invoke CryptAcquireContextW,addr provider,0,0,1,0f0000000h
    test eax,eax
    jz .done
    invoke CryptCreateHash,[provider],8004h,0,0,addr hash
    test eax,eax
    jz .done
    invoke CreateFileW,[path],80000000h,1,0,3,80h,0
    mov [fileHandle],eax
    cmp eax,-1
    je .done
    invoke GetProcessHeap
    invoke HeapAlloc,eax,0,65536
    mov [buffer],eax
    test eax,eax
    jz .done
.read:
    invoke ReadFile,[fileHandle],[buffer],65536,addr got,0
    test eax,eax
    jz .done
    cmp [got],0
    je .digest
    invoke CryptHashData,[hash],[buffer],[got],0
    test eax,eax
    jz .done
    jmp .read
.digest:
    mov [size],20
    invoke CryptGetHashParam,[hash],2,addr digest,addr size,0
    test eax,eax
    jz .done
    lea esi,[digest]
    mov edi,[output]
    mov ecx,20
.hex:
    lodsb
    movzx ebx,al
    shr ebx,4
    mov dl,[cHex+ebx]
    mov [edi],dl
    and eax,15
    mov dl,[cHex+eax]
    mov [edi+1],dl
    add edi,2
    loop .hex
    mov byte [edi],0
    mov [result],1
.done:
    cmp [fileHandle],-1
    je .hash
    invoke CloseHandle,[fileHandle]
.hash:
    cmp [hash],0
    je .provider
    invoke CryptDestroyHash,[hash]
.provider:
    cmp [provider],0
    je .buffer
    invoke CryptReleaseContext,[provider],0
.buffer:
    stdcall FreeMem,[buffer]
    mov eax,[result]
    ret
endp

proc VerifySha1 uses ebx esi edi,path,expected
    local digest[41]:BYTE
    cmp [expected],0
    je .bad
    invoke lstrlenA,[expected]
    cmp eax,40
    jne .bad
    stdcall Sha1File,[path],addr digest
    test eax,eax
    jz .bad
    lea esi,[digest]
    mov edi,[expected]
    mov ecx,40
.compare:
    lodsb
    mov bl,byte [edi]
    or bl,32
    cmp al,bl
    jne .bad
    inc edi
    loop .compare
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc HttpDownload uses ebx esi edi,urlA,path,expected
    locals
        urlW rw 8192
        temp rw 2048
        parent rw 2048
        handles rd 3
        fileHandle dd ?
        buffer dd ?
        got dd ?
        written dd ?
        result dd ?
        responseStatus dd ?
    endl
    mov [responseStatus],0
    mov [result],0
    mov [fileHandle],-1
    mov [buffer],0
    mov word [temp],0
    mov dword [handles],0
    mov dword [handles+4],0
    mov dword [handles+8],0
    cmp [expected],0
    je .url
    stdcall VerifySha1,[path],[expected]
    test eax,eax
    jz .url
    mov eax,1
    ret
.url:
    invoke MultiByteToWideChar,65001,8,[urlA],-1,addr urlW,8192
    test eax,eax
    jz .done
    invoke lstrlenW,[path]
    cmp eax,2042
    ja .done
    mov ecx,eax
    inc ecx
    mov esi,[path]
    lea edi,[temp]
    rep movsw
    sub edi,2
    mov esi,cPart
    mov ecx,6
    rep movsw
    mov esi,[path]
    lea edi,[parent]
    xor ebx,ebx
.copyparent:
    lodsw
    stosw
    cmp ax,92
    je .saveparent
    cmp ax,'/'
    jne .parentnext
.saveparent:
    lea ebx,[edi-2]
.parentnext:
    test ax,ax
    jnz .copyparent
    test ebx,ebx
    jz .open
    mov word [ebx],0
    stdcall EnsureDirectory,addr parent
    test eax,eax
    jz .done
.open:
    stdcall NetStart,addr urlW,0,0,0,addr handles,1,addr responseStatus
    test eax,eax
    jz .done
    cmp [responseStatus],200
    jne .done
    invoke CreateFileW,addr temp,40000000h,0,0,2,80h,0
    mov [fileHandle],eax
    cmp eax,-1
    je .done
    invoke GetProcessHeap
    invoke HeapAlloc,eax,0,65536
    mov [buffer],eax
    test eax,eax
    jz .done
.read:
    invoke WinHttpReadData,[handles+8],[buffer],65536,addr got
    test eax,eax
    jz .done
    cmp [got],0
    je .complete
    invoke WriteFile,[fileHandle],[buffer],[got],addr written,0
    test eax,eax
    jz .done
    mov eax,[got]
    cmp eax,[written]
    jne .done
    stdcall DownloadCount,[got],0
    jmp .read
.complete:
    invoke CloseHandle,[fileHandle]
    mov [fileHandle],-1
    cmp [expected],0
    je .move
    stdcall VerifySha1,addr temp,[expected]
    test eax,eax
    jz .done
.move:
    invoke MoveFileExW,addr temp,[path],9
    test eax,eax
    jz .done
    mov [result],1
.done:
    cmp [fileHandle],-1
    je .cleanup
    invoke CloseHandle,[fileHandle]
.cleanup:
    stdcall NetClose,addr handles
    stdcall FreeMem,[buffer]
    cmp [result],0
    jne .return
    ; temp is initialized only after URL/path validation.
    cmp word [temp],0
    je .return
    invoke DeleteFileW,addr temp
.return:
    stdcall DownloadCount,0,[result]
    mov eax,[result]
    ret
endp
