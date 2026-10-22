; Original read-only Java SLP client. Private socket/JSON state per worker.
SP_SOCKET = 0
SP_START = 4
SP_BUDGET = 5000
SP_FRAME_CAP = 65536
section '.spd' data readable writeable
spNext dd 0
spPlayerKey db 'players',0
spOnlineKey db 'online',0
spMaxKey db 'max',0
spPortFormat du '%u',0
spSrvPrefix du '_minecraft._tcp.',0
spThreads rd 8

section '.spc' code readable executable
 nop

; Parse a saved address without changing it. Host excludes brackets/port.
proc SpParseAddress uses ebx esi edi,address,host,port,explicitPort,numeric
 local count:DWORD,start:DWORD,limit:DWORD,split:DWORD,bracket:DWORD
 mov eax,[port]
 mov dword [eax],25565
 mov eax,[explicitPort]
 mov dword [eax],0
 mov eax,[numeric]
 mov dword [eax],0
 mov esi,[address]
 xor ecx,ecx
.length:
 cmp ecx,256
 jae .bad
 cmp word [esi+ecx*2],0
 je .trim
 inc ecx
 jmp .length
.trim:
 test ecx,ecx
 jz .bad
 cmp word [esi],32
 jne .tail
 add esi,2
 dec ecx
 jmp .trim
.tail:
 test ecx,ecx
 jz .bad
 cmp word [esi+ecx*2-2],32
 jne .scan
 dec ecx
 jmp .tail
.scan:
 mov [count],ecx
 mov [start],esi
 lea eax,[esi+ecx*2]
 mov [limit],eax
 mov [split],0
 mov [bracket],0
 cmp word [esi],'['
 jne .plain
 mov [bracket],1
 add esi,2
 mov [start],esi
.bracketLoop:
 cmp esi,[limit]
 jae .bad
 cmp word [esi],']'
 je .bracketEnd
 cmp word [esi],':'
 jne .bracketAdvance
 mov eax,[numeric]
 mov dword [eax],1
.bracketAdvance:
 add esi,2
 jmp .bracketLoop
.bracketEnd:
 mov [split],esi
 mov eax,[numeric]
 cmp dword [eax],1
 jne .bad
 add esi,2
 cmp esi,[limit]
 je .copyHost
 cmp word [esi],':'
 jne .bad
 add esi,2
 jmp .parsePort
.plain:
 cmp esi,[limit]
 jae .copyHost
 cmp word [esi],':'
 je .colon
 add esi,2
 jmp .plain
.colon:
 ; Multiple colons without brackets are a bare IPv6 literal using the default
 ; port; only [IPv6]:port is interpreted as an IPv6 port override.
 lea edi,[esi+2]
.moreColons:
 cmp edi,[limit]
 jae .singleColon
 cmp word [edi],':'
 je .rawIpv6
 add edi,2
 jmp .moreColons
.rawIpv6:
 mov [bracket],1
 mov eax,[numeric]
 mov dword [eax],1
 jmp .copyHost
.singleColon:
 mov [split],esi
 add esi,2
.parsePort:
 mov eax,[explicitPort]
 mov dword [eax],1
 cmp esi,[limit]
 jae .bad
 xor ebx,ebx
.portLoop:
 cmp esi,[limit]
 jae .portDone
 movzx eax,word [esi]
 sub eax,'0'
 cmp eax,9
 ja .bad
 imul ebx,10
 add ebx,eax
 cmp ebx,65535
 ja .bad
 add esi,2
 jmp .portLoop
.portDone:
 test ebx,ebx
 jz .bad
 mov eax,[port]
 mov [eax],ebx
.copyHost:
 mov esi,[start]
 mov edx,[split]
 test edx,edx
 jnz .hostEnd
 mov edx,[limit]
.hostEnd:
 cmp esi,edx
 jae .bad
 mov edi,[host]
 mov ebx,1
.hostLoop:
 cmp esi,edx
 jae .hostDone
 lodsw
 cmp ax,32
 jbe .bad
 cmp ax,127
 je .bad
 cmp ax,'/'
 je .bad
 cmp ax,92
 je .bad
 cmp ax,'@'
 je .bad
 cmp ax,'#'
 je .bad
 cmp ax,'?'
 je .bad
 cmp ax,'['
 je .bad
 cmp ax,']'
 je .bad
 cmp ax,':'
 jne .digits
 cmp [bracket],1
 jne .bad
.digits:
 cmp ax,'.'
 je .store
 cmp ax,'0'
 jb .notNumeric
 cmp ax,'9'
 jbe .store
.notNumeric:
 xor ebx,ebx
.store:
 stosw
 jmp .hostLoop
.hostDone:
 mov word [edi],0
 cmp [bracket],1
 je .good
 mov eax,[numeric]
 mov [eax],ebx
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; SRV uses Windows DNS timeouts. Explicit ports and numeric hosts skip this.
; Lowest priority wins; weight is a deterministic tie breaker for display.
proc SpSrvLookup uses ebx esi edi,host,connectHost,port
 local query[280]:WORD,records:DWORD,best:DWORD,priority:DWORD,weight:DWORD,result:DWORD
 mov [records],0
 mov [best],0
 mov [priority],65536
 mov [weight],0
 mov [result],0
 cmp [engineCancel],0
 jne .done
 cmp [modCancel],0
 jne .done
 invoke lstrcpyW,addr query,spSrvPrefix
 invoke lstrcatW,addr query,[host]
 invoke DnsQuery_W,addr query,33,0,0,addr records,0
 test eax,eax
 jnz .done
 mov esi,[records]
.loop:
 test esi,esi
 jz .chosen
 cmp word [esi+8],33
 jne .next
 cmp word [esi+10],12
 jb .next
 movzx eax,word [esi+28]
 cmp eax,[priority]
 ja .next
 jb .take
 movzx eax,word [esi+30]
 cmp eax,[weight]
 jbe .next
.take:
 mov eax,[esi+24]
 test eax,eax
 jz .next
 movzx eax,word [esi+32]
 test eax,eax
 jz .next
 mov [best],esi
 movzx eax,word [esi+28]
 mov [priority],eax
 movzx eax,word [esi+30]
 mov [weight],eax
.next:
 mov esi,[esi]
 jmp .loop
.chosen:
 mov esi,[best]
 test esi,esi
 jz .done
 invoke lstrlenW,dword [esi+24]
 test eax,eax
 jz .done
 cmp eax,256
 jae .done
 mov ebx,eax
 cmp ebx,1
 jne .target
 mov edi,[esi+24]
 cmp word [edi],'.'
 je .disabled
.target:
 invoke lstrcpyW,[connectHost],dword [esi+24]
 mov edi,[connectHost]
 cmp word [edi+ebx*2-2],'.'
 jne .port
 dec ebx
 mov word [edi+ebx*2],0
.port:
 test ebx,ebx
 jz .done
 mov edi,[port]
 movzx eax,word [esi+32]
 mov [edi],eax
 mov [result],1
 jmp .done
.disabled:
 mov [result],-1
.done:
 cmp [records],0
 je .return
 invoke DnsRecordListFree,[records],1
.return:
 mov eax,[result]
 ret
endp

proc SpAlive,context
 cmp [engineCancel],0
 jne .bad
 cmp [modCancel],0
 jne .bad
 invoke GetTickCount
 mov edx,[context]
 sub eax,[edx+SP_START]
 cmp eax,SP_BUDGET
 jae .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; 50 ms select polls provide cooperative cancellation for socket operations.
proc SpWait uses ebx,context,writeReady
 local readSet[2]:DWORD,writeSet[2]:DWORD,errorSet[2]:DWORD,timeout[2]:DWORD
.again:
 stdcall SpAlive,[context]
 test eax,eax
 jz .bad
 mov ebx,[context]
 mov eax,[ebx+SP_SOCKET]
 mov dword [readSet],1
 mov [readSet+4],eax
 mov dword [writeSet],1
 mov [writeSet+4],eax
 mov dword [errorSet],1
 mov [errorSet+4],eax
 mov dword [timeout],0
 mov dword [timeout+4],50000
 cmp [writeReady],0
 jne .write
 invoke select,0,addr readSet,0,addr errorSet,addr timeout
 jmp .selected
.write:
 invoke select,0,0,addr writeSet,addr errorSet,addr timeout
.selected:
 cmp eax,-1
 je .bad
 test eax,eax
 jz .again
 cmp dword [errorSet],0
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SpSend uses ebx esi edi,context,data,bytes
 mov esi,[data]
 mov edi,[bytes]
 mov ebx,[context]
.loop:
 test edi,edi
 jz .good
 stdcall SpWait,ebx,1
 test eax,eax
 jz .bad
 invoke send,dword [ebx+SP_SOCKET],esi,edi,0
 test eax,eax
 jg .sent
 cmp eax,0
 je .bad
 invoke WSAGetLastError
 cmp eax,10035
 je .loop
 jmp .bad
.sent:
 add esi,eax
 sub edi,eax
 jmp .loop
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SpRecv uses ebx esi edi,context,data,bytes
 mov esi,[data]
 mov edi,[bytes]
 mov ebx,[context]
.loop:
 test edi,edi
 jz .good
 stdcall SpWait,ebx,0
 test eax,eax
 jz .bad
 invoke recv,dword [ebx+SP_SOCKET],esi,edi,0
 test eax,eax
 jg .received
 cmp eax,0
 je .bad
 invoke WSAGetLastError
 cmp eax,10035
 je .loop
 jmp .bad
.received:
 add esi,eax
 sub edi,eax
 jmp .loop
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SpReadVar uses ebx esi edi,context,value
 local byteValue:BYTE
 xor ebx,ebx
 xor esi,esi
.loop:
 cmp esi,5
 jae .bad
 stdcall SpRecv,[context],addr byteValue,1
 test eax,eax
 jz .bad
 movzx edi,[byteValue]
 cmp esi,4
 jne .bits
 test edi,0F8h
 jnz .bad
.bits:
 mov eax,edi
 and eax,7Fh
 mov ecx,esi
 imul ecx,7
 shl eax,cl
 or ebx,eax
 inc esi
 test edi,80h
 jnz .loop
 mov eax,[value]
 mov [eax],ebx
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SpBufferVar uses ebx esi edi,position,limit,value
 mov edi,[position]
 mov esi,[edi]
 xor ebx,ebx
 xor ecx,ecx
.loop:
 cmp esi,[limit]
 jae .bad
 cmp ecx,35
 jae .bad
 movzx edx,byte [esi]
 inc esi
 cmp ecx,28
 jne .bits
 test edx,0F8h
 jnz .bad
.bits:
 mov eax,edx
 and eax,7Fh
 shl eax,cl
 or ebx,eax
 add ecx,7
 test edx,80h
 jnz .loop
 mov [edi],esi
 mov eax,[value]
 mov [eax],ebx
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SpWriteVar uses edi,value,dest
 mov eax,[value]
 mov edi,[dest]
 xor ecx,ecx
.loop:
 mov dl,al
 and dl,7Fh
 shr eax,7
 test eax,eax
 jz .last
 or dl,80h
.last:
 mov [edi],dl
 inc edi
 inc ecx
 test eax,eax
 jnz .loop
 mov eax,ecx
 ret
endp

; Exact nonnegative JSON int32. Missing, quoted, fractional or overflow is bad.
proc SpCount uses ebx esi,ctx,token,value
 local text[16]:BYTE,length:DWORD
 stdcall JsonRaw,[ctx],[token],addr text,16
 test eax,eax
 jz .bad
 cmp eax,10
 ja .bad
 mov [length],eax
 lea esi,[text]
 xor ebx,ebx
 xor ecx,ecx
.digit:
 cmp ecx,[length]
 jae .done
 movzx eax,byte [esi+ecx]
 sub eax,'0'
 cmp eax,9
 ja .bad
 cmp ebx,214748364
 ja .bad
 jb .append
 cmp eax,7
 ja .bad
.append:
 imul ebx,10
 add ebx,eax
 inc ecx
 jmp .digit
.done:
 mov eax,[value]
 mov [eax],ebx
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SpStatusJson uses ebx,buffer,length,row
 local ctx:DWORD,players:DWORD,online:DWORD,maximum:DWORD
 mov [ctx],0
 mov ebx,[length]
 cmp ebx,2
 jb .bad
 cmp ebx,SP_FRAME_CAP
 ja .bad
 mov edx,[buffer]
 xor ecx,ecx
.zero:
 cmp ecx,ebx
 jae .utf8
 cmp byte [edx+ecx],0
 je .bad
 inc ecx
 jmp .zero
.utf8:
 invoke MultiByteToWideChar,65001,8,[buffer],ebx,0,0
 test eax,eax
 jz .bad
 stdcall JsonParse,[buffer]
 test eax,eax
 jz .bad
 mov [ctx],eax
 stdcall JsonRoot,eax
 stdcall JsonGet,[ctx],eax,spPlayerKey
 test eax,eax
 jz .bad
 mov [players],eax
 stdcall JsonGet,[ctx],eax,spOnlineKey
 stdcall SpCount,[ctx],eax,addr online
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],[players],spMaxKey
 stdcall SpCount,[ctx],eax,addr maximum
 test eax,eax
 jz .bad
 mov edx,[row]
 mov eax,[online]
 mov [edx+SV_ONLINE],eax
 mov eax,[maximum]
 mov [edx+SV_MAX],eax
 stdcall JsonFree,[ctx]
 mov eax,1
 ret
.bad:
 stdcall JsonFree,[ctx]
 xor eax,eax
 ret
endp

proc SpQueryRow uses ebx esi edi,row
 locals
  context rd 2
  host rw 256
  connectHost rw 256
  hostA rb 1024
  service rw 16
  explicitPort dd ?
  numeric dd ?
  port dd ?
  hints rd 11
  result dd ?
  current dd ?
  enabled dd ?
  error dd ?
  errorBytes dd ?
  packet rb 1100
  body rb 1080
  length dd ?
  frame dd ?
  heap dd ?
  position dd ?
  limit dd ?
  value dd ?
  state dd ?
 endl
 mov [result],0
 mov [heap],0
 mov [context+SP_SOCKET],-1
 mov [state],SV_OFF
 mov ebx,[row]
 mov dword [ebx+SV_ONLINE],-1
 mov dword [ebx+SV_MAX],-1
 mov dword [ebx+SV_LATENCY],-1
 lea eax,[ebx+SV_ADDRESS]
 stdcall SpParseAddress,eax,addr host,addr port,addr explicitPort,addr numeric
 test eax,eax
 jz .invalid
 invoke lstrcpyW,addr connectHost,addr host
 cmp [explicitPort],0
 jne .resolvedSrv
 cmp [numeric],0
 jne .resolvedSrv
 stdcall SpSrvLookup,addr host,addr connectHost,addr port
 cmp eax,-1
 je .done
.resolvedSrv:
 cmp [engineCancel],0
 jne .done
 cmp [modCancel],0
 jne .done
 lea edi,[hints]
 xor eax,eax
 mov ecx,11
 rep stosd
 mov dword [hints+8],1
 mov dword [hints+12],6
 cinvoke wsprintfW,addr service,spPortFormat,[port]
 invoke GetAddrInfoExW,addr connectHost,addr service,12,0,addr hints,addr result,0,0,0,0
 test eax,eax
 jnz .done
 ; Resolver/SRV follow Windows DNS timeouts. The separate socket budget starts
 ; once DNS completes and is checked every select poll.
 invoke GetTickCount
 mov [context+SP_START],eax
 mov eax,[result]
 mov [current],eax
.address:
 stdcall SpAlive,addr context
 test eax,eax
 jz .done
 mov esi,[current]
 test esi,esi
 jz .done
 mov eax,[esi+4]
 cmp eax,2
 je .socket
 cmp eax,23
 jne .nextAddress
.socket:
 invoke socket,eax,1,6
 cmp eax,-1
 je .nextAddress
 mov [context+SP_SOCKET],eax
 mov [enabled],1
 invoke ioctlsocket,eax,8004667Eh,addr enabled
 test eax,eax
 jnz .closeNext
 invoke connect,dword [context+SP_SOCKET],dword [esi+24],dword [esi+16]
 test eax,eax
 jz .connected
 invoke WSAGetLastError
 cmp eax,10035
 jne .closeNext
 stdcall SpWait,addr context,1
 test eax,eax
 jz .closeNext
 mov [error],-1
 mov [errorBytes],4
 invoke getsockopt,dword [context+SP_SOCKET],0FFFFh,1007h,addr error,addr errorBytes
 test eax,eax
 jnz .closeNext
 cmp [error],0
 je .connected
.closeNext:
 invoke closesocket,dword [context+SP_SOCKET]
 mov [context+SP_SOCKET],-1
.nextAddress:
 mov eax,[esi+40]
 mov [current],eax
 jmp .address
.connected:
 invoke WideCharToMultiByte,65001,80h,addr host,-1,addr hostA,1024,0,0
 test eax,eax
 jz .invalid
 dec eax
 cmp eax,1020
 ja .invalid
 mov [length],eax
 lea edi,[body]
 mov byte [edi],0
 mov byte [edi+1],47
 add edi,2
 stdcall SpWriteVar,[length],edi
 add edi,eax
 lea esi,[hostA]
 mov ecx,[length]
 rep movsb
 mov eax,[port]
 mov [edi],ah
 mov [edi+1],al
 mov byte [edi+2],1
 add edi,3
 lea eax,[body]
 sub edi,eax
 mov [length],edi
 stdcall SpWriteVar,edi,addr packet
 lea edi,[packet+eax]
 lea esi,[body]
 mov ecx,[length]
 rep movsb
 mov byte [edi],1
 mov byte [edi+1],0
 add edi,2
 lea eax,[packet]
 sub edi,eax
 stdcall SpSend,addr context,addr packet,edi
 test eax,eax
 jz .done
 mov [state],SV_UNSUPPORTED
 stdcall SpReadVar,addr context,addr frame
 test eax,eax
 jz .done
 cmp [frame],3
 jb .done
 cmp [frame],SP_FRAME_CAP
 ja .done
 invoke GetProcessHeap
 mov edx,[frame]
 inc edx
 invoke HeapAlloc,eax,HEAP_ZERO_MEMORY,edx
 test eax,eax
 jz .done
 mov [heap],eax
 stdcall SpRecv,addr context,eax,[frame]
 test eax,eax
 jz .done
 mov eax,[heap]
 mov [position],eax
 add eax,[frame]
 mov [limit],eax
 stdcall SpBufferVar,addr position,[limit],addr value
 test eax,eax
 jz .done
 cmp [value],0
 jne .done
 stdcall SpBufferVar,addr position,[limit],addr value
 test eax,eax
 jz .done
 mov eax,[limit]
 sub eax,[position]
 cmp eax,[value]
 jne .done
 stdcall SpStatusJson,[position],[value],[row]
 test eax,eax
 jz .done
 mov [state],SV_ON
 invoke GetTickCount
 sub eax,[context+SP_START]
 mov edx,[row]
 mov [edx+SV_LATENCY],eax
 jmp .done
.invalid:
 mov [state],SV_INVALID
.done:
 cmp [context+SP_SOCKET],-1
 je .freeDns
 invoke closesocket,dword [context+SP_SOCKET]
.freeDns:
 cmp [result],0
 je .freeHeap
 invoke FreeAddrInfoExW,[result]
.freeHeap:
 stdcall FreeMem,[heap]
 cmp [engineCancel],0
 jne .cancelled
 cmp [modCancel],0
 je .state
.cancelled:
 mov [state],SV_CANCELLED
.state:
 mov eax,[state]
 mov edx,[row]
 mov [edx+SV_STATE],eax
 cmp eax,SV_ON
 sete al
 movzx eax,al
 ret
endp

proc SpWorker uses ebx,param
.next:
 mov eax,1
 lock xadd [spNext],eax
 cmp eax,[serversCount]
 jae .done
 imul eax,SV_SIZE
 add eax,serversRows
 mov ebx,eax
 mov dword [ebx+SV_STATE],SV_BUSY
 stdcall SpQueryRow,ebx
 jmp .next
.done:
 xor eax,eax
 ret
endp

proc ServersRefresh uses ebx esi edi
 local data[512]:BYTE,count:DWORD,index:DWORD,started:DWORD,fallback:DWORD
 mov [spNext],0
 mov [started],0
 mov [fallback],0
 cmp [serversCount],SV_CAPACITY
 ja .bad
 xor esi,esi
.reset:
 cmp esi,[serversCount]
 jae .startup
 imul edi,esi,SV_SIZE
 add edi,serversRows
 mov dword [edi+SV_ONLINE],-1
 mov dword [edi+SV_MAX],-1
 mov dword [edi+SV_LATENCY],-1
 mov dword [edi+SV_STATE],SV_WAIT
 inc esi
 jmp .reset
.startup:
 invoke WSAStartup,202h,addr data
 test eax,eax
 jnz .bad
 mov eax,[serversCount]
 cmp eax,8
 jbe .count
 mov eax,8
.count:
 mov [count],eax
 mov [index],0
.spawn:
 mov esi,[index]
 cmp esi,[count]
 jae .join
 invoke CreateThread,0,0,SpWorker,0,0,0
 mov [spThreads+esi*4],eax
 test eax,eax
 jnz .spawned
 mov [fallback],1
.spawned:
 inc [index]
 jmp .spawn
.join:
 cmp [fallback],0
 je .wait
 stdcall SpWorker,0
.wait:
 mov [index],0
.handle:
 mov esi,[index]
 cmp esi,[count]
 jae .clean
 mov ebx,[spThreads+esi*4]
 test ebx,ebx
 jz .nextHandle
 invoke WaitForSingleObject,ebx,-1
 invoke CloseHandle,ebx
.nextHandle:
 inc [index]
 jmp .handle
.clean:
 invoke WSACleanup
 cmp [engineCancel],0
 jne .bad
 cmp [modCancel],0
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp


