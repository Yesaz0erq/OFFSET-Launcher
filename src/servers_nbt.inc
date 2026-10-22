; Read-only Java servers.dat (uncompressed NBT). One background worker.
; No partial list is published; unknown fields are bounded and skipped.
SN_FILE_CAP = 16777216
SN_DEPTH_CAP = 32
section '.snbd' data readable writeable
serversCount dd 0
serversLastError dd 0
snCount dd 0
snError dd 0
snSuffix du '\servers.dat',0
snServers db 'servers',0
snName db 'name',0
snIp db 'ip',0
snBuffer dd 0
snPos dd 0
snEnd dd 0
snKeyPtr dd 0
snKeyLen dd 0
serversRows rb SV_SIZE*SV_CAPACITY
serversDataDirW rw 2048
snRows rb SV_SIZE*SV_CAPACITY
snPath rw 2048
snNewData rw 2048
section '.snbc' code readable executable
 nop

; All parsers use a sticky error and never advance beyond the input buffer.
proc SnTake uses ebx,count
 mov eax,[snPos]
 mov ebx,eax
 add ebx,[count]
 jc .bad
 cmp ebx,[snEnd]
 ja .bad
 mov [snPos],ebx
 ret
.bad:
 mov [snError],SV_ERR_NBT
 xor eax,eax
 ret
endp
proc SnByte
 stdcall SnTake,1
 test eax,eax
 jz .done
 movzx eax,byte [eax]
.done:
 ret
endp
proc SnShort
 stdcall SnTake,2
 test eax,eax
 jz .done
 movzx eax,word [eax]
 xchg al,ah
.done:
 ret
endp
proc SnInt
 stdcall SnTake,4
 test eax,eax
 jz .done
 mov eax,[eax]
 bswap eax
.done:
 ret
endp

; Strict Modified UTF-8 to UTF-16. Java encodes supplementary characters as
; two three-byte surrogate code units, rather than four-byte UTF-8 sequences.
; A null output validates/skips strings without allocating or truncating them.
proc SnMutf uses ebx esi edi,source,length,destination,capacity
 local count:DWORD,remaining:DWORD
 mov esi,[source]
 mov edi,[destination]
 mov eax,[length]
 mov [remaining],eax
 mov [count],0
.loop:
 cmp [remaining],0
 je .okay
 movzx ebx,byte [esi]
 inc esi
 dec [remaining]
 test ebx,ebx
 jz .bad
 cmp ebx,128
 jb .unit
 mov eax,ebx
 and eax,0E0h
 cmp eax,0C0h
 je .two
 mov eax,ebx
 and eax,0F0h
 cmp eax,0E0h
 jne .bad
 cmp [remaining],2
 jb .bad
 movzx eax,byte [esi]
 movzx edx,byte [esi+1]
 mov ecx,eax
 and ecx,0C0h
 cmp ecx,80h
 jne .bad
 mov ecx,edx
 and ecx,0C0h
 cmp ecx,80h
 jne .bad
 and ebx,0Fh
 shl ebx,12
 and eax,3Fh
 shl eax,6
 or ebx,eax
 and edx,3Fh
 or ebx,edx
 add esi,2
 sub [remaining],2
 cmp ebx,800h
 jb .bad
 jmp .unit
.two:
 cmp [remaining],1
 jb .bad
 movzx eax,byte [esi]
 mov edx,eax
 and edx,0C0h
 cmp edx,80h
 jne .bad
 and ebx,1Fh
 shl ebx,6
 and eax,3Fh
 or ebx,eax
 inc esi
 dec [remaining]
 test ebx,ebx
 jz .unit
 cmp ebx,80h
 jb .bad
.unit:
 cmp [destination],0
 je .loop
 test ebx,ebx
 jz .bad
 mov ecx,[count]
 inc ecx
 cmp ecx,[capacity]
 jae .bad
 mov eax,[count]
 mov [edi+eax*2],bx
 mov [count],ecx
 jmp .loop
.okay:
 cmp [destination],0
 je .valid
 mov eax,[count]
 mov word [edi+eax*2],0
.valid:
 mov eax,1
 ret
.bad:
 mov [snError],SV_ERR_TEXT
 xor eax,eax
 ret
endp
proc SnString,destination,capacity
 local length:DWORD
 stdcall SnShort
 mov [length],eax
 cmp [snError],0
 jne .bad
 stdcall SnTake,eax
 test eax,eax
 jz .bad
 stdcall SnMutf,eax,[length],[destination],[capacity]
 ret
.bad:
 xor eax,eax
 ret
endp
proc SnKey
 stdcall SnShort
 mov [snKeyLen],eax
 cmp [snError],0
 jne .bad
 stdcall SnTake,eax
 mov [snKeyPtr],eax
 test eax,eax
 jz .bad
 stdcall SnMutf,eax,[snKeyLen],0,0
 ret
.bad:
 xor eax,eax
 ret
endp
proc SnKeyIs uses esi edi,key,length
 mov eax,[length]
 cmp eax,[snKeyLen]
 jne .no
 mov esi,[snKeyPtr]
 mov edi,[key]
 mov ecx,eax
 repe cmpsb
 jne .no
 mov eax,1
 ret
.no:
 xor eax,eax
 ret
endp
proc SnCancel
 cmp [engineCancel],0
 je .okay
 mov [snError],SV_ERR_CANCEL
 xor eax,eax
 ret
.okay:
 mov eax,1
 ret
endp

; Skip the payload of every Java NBT type. Signed lengths, nesting, overflow,
; and invalid nonempty TAG_End lists are rejected before reading their data.
proc SnSkip uses ebx esi,tag,depth
 local count:DWORD,item:DWORD
 cmp [depth],SN_DEPTH_CAP
 ja .bad
 stdcall SnCancel
 test eax,eax
 jz .failed
 mov eax,[tag]
 cmp eax,1
 je .one
 cmp eax,2
 je .two
 cmp eax,3
 je .four
 cmp eax,4
 je .eight
 cmp eax,5
 je .four
 cmp eax,6
 je .eight
 cmp eax,7
 je .byteArray
 cmp eax,8
 je .string
 cmp eax,9
 je .list
 cmp eax,10
 je .compound
 cmp eax,11
 je .intArray
 cmp eax,12
 jne .bad
 mov ebx,8
 jmp .array
.intArray:
 mov ebx,4
 jmp .array
.byteArray:
 mov ebx,1
.array:
 stdcall SnInt
 test eax,eax
 js .bad
 mul ebx
 test edx,edx
 jnz .bad
 jmp .take
.one: mov eax,1
 jmp .take
.two: mov eax,2
 jmp .take
.four: mov eax,4
 jmp .take
.eight: mov eax,8
.take:
 cmp [snError],0
 jne .failed
 stdcall SnTake,eax
 test eax,eax
 jz .failed
 jmp .good
.string:
 stdcall SnString,0,0
 ret
.list:
 stdcall SnByte
 mov [item],eax
 cmp eax,12
 ja .bad
 stdcall SnInt
 mov [count],eax
 test eax,eax
 js .bad
 jz .good
 cmp eax,SN_FILE_CAP
 ja .bad
 cmp [item],0
 je .bad
.listLoop:
 cmp [count],0
 je .good
 mov eax,[depth]
 inc eax
 stdcall SnSkip,[item],eax
 test eax,eax
 jz .failed
 dec [count]
 jmp .listLoop
.compound:
 stdcall SnCancel
 test eax,eax
 jz .failed
 stdcall SnByte
 cmp [snError],0
 jne .failed
 test eax,eax
 jz .good
 mov [item],eax
 stdcall SnKey
 test eax,eax
 jz .failed
 mov eax,[depth]
 inc eax
 stdcall SnSkip,[item],eax
 test eax,eax
 jz .failed
 jmp .compound
.good:
 cmp [snError],0
 jne .failed
 mov eax,1
 ret
.bad:
 mov [snError],SV_ERR_NBT
.failed:
 xor eax,eax
 ret
endp

proc SnServer uses ebx,record
 local tag:DWORD,flags:DWORD
 mov [flags],0
 mov ebx,[record]
.loop:
 stdcall SnCancel
 test eax,eax
 jz .bad
 stdcall SnByte
 mov [tag],eax
 cmp [snError],0
 jne .bad
 test eax,eax
 jz .done
 stdcall SnKey
 test eax,eax
 jz .bad
 stdcall SnKeyIs,snName,4
 test eax,eax
 jnz .name
 stdcall SnKeyIs,snIp,2
 test eax,eax
 jnz .ip
 stdcall SnSkip,[tag],2
 test eax,eax
 jz .bad
 jmp .loop
.name:
 test [flags],1
 jnz .invalid
 or [flags],1
 cmp [tag],8
 jne .invalid
 lea eax,[ebx+SV_NAME]
 stdcall SnString,eax,128
 test eax,eax
 jz .bad
 jmp .loop
.ip:
 test [flags],2
 jnz .invalid
 or [flags],2
 cmp [tag],8
 jne .invalid
 lea eax,[ebx+SV_ADDRESS]
 stdcall SnString,eax,256
 test eax,eax
 jz .bad
 jmp .loop
.done:
 cmp [flags],3
 jne .invalid
 mov eax,1
 ret
.invalid:
 mov [snError],SV_ERR_NBT
.bad:
 xor eax,eax
 ret
endp
proc SnParse uses ebx esi edi,buffer,length
 local tag:DWORD,found:DWORD,count:DWORD
 mov eax,[buffer]
 mov [snPos],eax
 add eax,[length]
 jc .invalid
 mov [snEnd],eax
 mov [snError],0
 mov [snCount],0
 mov [found],0
 mov edi,snRows
 xor eax,eax
 mov ecx,SV_SIZE*SV_CAPACITY/4
 rep stosd
 stdcall SnByte
 cmp eax,10
 jne .invalid
 stdcall SnKey
 test eax,eax
 jz .bad
.root:
 stdcall SnCancel
 test eax,eax
 jz .bad
 stdcall SnByte
 mov [tag],eax
 cmp [snError],0
 jne .bad
 test eax,eax
 jz .done
 stdcall SnKey
 test eax,eax
 jz .bad
 stdcall SnKeyIs,snServers,7
 test eax,eax
 jnz .servers
 stdcall SnSkip,[tag],1
 test eax,eax
 jz .bad
 jmp .root
.servers:
 cmp [found],0
 jne .invalid
 mov [found],1
 cmp [tag],9
 jne .invalid
 stdcall SnByte
 mov [tag],eax
 cmp eax,12
 ja .invalid
 stdcall SnInt
 cmp [snError],0
 jne .bad
 test eax,eax
 js .invalid
 cmp eax,SV_CAPACITY
 ja .limit
 mov [count],eax
 test eax,eax
 jz .root
 cmp [tag],10
 jne .invalid
.item:
 cmp [count],0
 je .root
 mov eax,[snCount]
 imul eax,SV_SIZE
 add eax,snRows
 stdcall SnServer,eax
 test eax,eax
 jz .bad
 inc [snCount]
 dec [count]
 jmp .item
.done:
 mov eax,[snPos]
 cmp eax,[snEnd]
 jne .invalid
 cmp [snError],0
 jne .bad
 mov eax,1
 ret
.limit:
 mov [snError],SV_ERR_LIMIT
 jmp .bad
.invalid:
 mov [snError],SV_ERR_NBT
.bad:
 xor eax,eax
 ret
endp

proc ServersLoad uses ebx esi edi,rootPath,id
 locals
  file dd ?
  size dq ?
  read dd ?
  switched dd ?
  result dd ?
 endl
 mov [file],-1
 mov [result],0
 mov [snBuffer],0
 mov [snError],0
 mov [switched],0
 mov [serversLastError],0
 mov word [snNewData],0
 stdcall VsResolveData,[rootPath],[id],snNewData,2048
 test eax,eax
 jz .pathError
 invoke lstrcmpiW,snNewData,serversDataDirW
 test eax,eax
 setnz al
 movzx eax,al
 mov [switched],eax
 invoke lstrlenW,snNewData
 ; Native file APIs in this build use ordinary Win32 paths. Report a path
 ; limit explicitly instead of misclassifying an existing long file as absent.
 cmp eax,247
 ja .pathError
 invoke lstrcpyW,snPath,snNewData
 invoke lstrcatW,snPath,snSuffix
 stdcall SnCancel
 test eax,eax
 jz .done
 invoke GetFileAttributesW,snPath
 cmp eax,-1
 je .open
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .pathError
.open:
 invoke CreateFileW,snPath,GENERIC_READ,FILE_SHARE_READ+FILE_SHARE_WRITE+FILE_SHARE_DELETE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .openFailed
 mov [file],eax
 invoke GetFileSizeEx,eax,addr size
 test eax,eax
 jz .ioError
 cmp dword [size+4],0
 jne .limit
 cmp dword [size],SN_FILE_CAP
 ja .limit
 cmp dword [size],0
 je .nbtError
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,dword [size]
 test eax,eax
 jz .ioError
 mov [snBuffer],eax
 invoke ReadFile,[file],eax,dword [size],addr read,0
 test eax,eax
 jz .ioError
 mov eax,dword [size]
 cmp eax,[read]
 jne .ioError
 stdcall SnParse,[snBuffer],eax
 test eax,eax
 jz .done
 jmp .publish
.openFailed:
 invoke GetLastError
 cmp eax,2
 je .empty
 cmp eax,3
 jne .ioError
.empty:
 mov [snCount],0
.publish:
 stdcall SnCancel
 test eax,eax
 jz .done
 mov edi,serversRows
 mov esi,snRows
 mov eax,[snCount]
 imul ecx,eax,SV_SIZE/4
 rep movsd
 mov eax,[snCount]
 mov [serversCount],eax
 invoke lstrcpyW,serversDataDirW,snNewData
 mov [result],1
 jmp .done
.pathError:
 mov [switched],1
 mov [snError],SV_ERR_PATH
 jmp .done
.ioError:
 mov [snError],SV_ERR_IO
 jmp .done
.limit:
 mov [snError],SV_ERR_LIMIT
 jmp .done
.nbtError:
 mov [snError],SV_ERR_NBT
.done:
 cmp [file],-1
 je .free
 invoke CloseHandle,[file]
.free:
 stdcall FreeMem,[snBuffer]
 mov [snBuffer],0
 mov eax,[snError]
 mov [serversLastError],eax
 cmp [result],1
 je .return
 cmp [switched],0
 je .return
 mov [serversCount],0
 invoke lstrcpyW,serversDataDirW,snNewData
.return:
 mov eax,[result]
 ret
endp
