; Original native Minecraft skin client. No account token is written to disk.
SK_PNG_MAX = 1048576
SK_JSON_MAX = 1048576
section '.skd' data readable writeable
skinReady dd 0
skinCurrentReady dd 0
skinLocalReady dd 0
skinModel dd 0
skinLocalModel dd 0
skinCurrentWidth dd 0
skinCurrentHeight dd 0
skinLocalWidth dd 0
skinLocalHeight dd 0
skinResult dd 0
skinAction dd 0
skinSerial dd 0
skinHttpStatus dd 0
skinPreviewError dd 0
skCurrentData dd 0
skCurrentBytes dd 0
skLocalData dd 0
skLocalBytes dd 0
skResponseBytes dd 0
skTempSerial dd 0
skAgentW du 'OFFSET/0.9.0',0
skApiHostW du 'api.minecraftservices.com',0
skTextureHostW du 'textures.minecraft.net',0
skProfileW du '/minecraft/profile',0
skUploadW du '/minecraft/profile/skins',0
skActiveW du '/minecraft/profile/skins/active',0
skGetW du 'GET',0
skPostW du 'POST',0
skDeleteW du 'DELETE',0
skAcceptW du 'Accept: application/json',13,10,0
skAuthW du 'Authorization: Bearer ',0
skCrLfW du 13,10,0
skMultipartW du 'Content-Type: multipart/form-data; boundary=',0
skHttpsA db 'https://textures.minecraft.net/texture/',0
skHttpA db 'http://textures.minecraft.net/texture/',0
skTexturePathA db '/texture/',0
skBoundaryFormatA db '----OFFSET-Skin-%08X-%08X-%08X',0
skTempFormatW du 'OFFSET-skin-%08X-%08X-%08X.png',0
skExportFormatW du '.offset-skin-%08X-%08X.tmp',0
skPngExtW du '.png',0
skConW du 'CON',0
skPrnW du 'PRN',0
skAuxW du 'AUX',0
skNulW du 'NUL',0
skConInW du 'CONIN$',0
skConOutW du 'CONOUT$',0
skPartOneA db 13,10,'Content-Disposition: form-data; name="variant"',13,10,13,10,0
skPartTwoA db 13,10,'Content-Disposition: form-data; name="file"; filename="skin.png"',13,10,'Content-Type: image/png',13,10,13,10,0
skDashA db '--',0
skCrLfA db 13,10,0
skClassicA db 'classic',0
skSlimA db 'slim',0
skActiveA db 'ACTIVE',0
skKeyIdA db 'id',0
skKeyNameA db 'name',0
skKeySkinsA db 'skins',0
skKeyCapesA db 'capes',0
skKeyStateA db 'state',0
skKeyUrlA db 'url',0
skKeyVariantA db 'variant',0
skKeyAliasA db 'alias',0
skMsgRefreshA db '正在读取正版皮肤…',0
skMsgReadyA db '皮肤已刷新。',0
skMsgLocalA db '皮肤已选择，尚未上传。',0
skMsgInvalidA db '请选择不超过 1 MiB 的 64×64 或 64×32 PNG 皮肤。',0
skMsgSessionA db '请先登录正版账号。',0
skMsgUploadA db '正在上传皮肤…',0
skMsgUploadedA db '皮肤已上传。',0
skMsgResetA db '皮肤已重置。',0
skMsgExportA db '皮肤已导出。',0
skMsgFailedA db '皮肤操作失败，请稍后重试。',0
skMsgPreviewA db '操作已完成，皮肤预览暂时无法刷新。',0
skMsgCancelA db '皮肤操作已取消。',0
skMsgExpiredA db '登录已失效，请刷新登录后重试。',0
skinNameA rb 64
skinUuidA rb 64
skinUrlA rb 512
skinSkinIdA rb 64
skinCapeUrlA rb 512
skinCapeIdA rb 64
skinCapeAliasA rb 128
skinCurrentPathW rw 2048
skinLocalPathW rw 2048
skinPreviewPathW rw 2048
skPendingStart:
skPendingNameA rb 64
skPendingUuidA rb 64
skPendingUrlA rb 512
skPendingSkinIdA rb 64
skPendingCapeUrlA rb 512
skPendingCapeIdA rb 64
skPendingCapeAliasA rb 128
skPendingModel dd ?
skPendingEnd:

section '.skc' code readable executable
 nop
proc SkCancelled
 if defined authCancel
 cmp [authCancel],0
 jne .yes
 end if
 if defined engineCancel
 cmp [engineCancel],0
 jne .yes
 end if
 if defined modCancel
 cmp [modCancel],0
 jne .yes
 end if
 xor eax,eax
 ret
.yes:
 mov eax,1
 ret
endp

proc SkEqualA uses esi edi,left,right
 mov esi,[left]
 mov edi,[right]
.next:
 lodsb
 mov dl,[edi]
 inc edi
 cmp al,'A'
 jb .right
 cmp al,'Z'
 ja .right
 add al,32
.right:
 cmp dl,'A'
 jb .compare
 cmp dl,'Z'
 ja .compare
 add dl,32
.compare:
 cmp al,dl
 jne .bad
 test al,al
 jnz .next
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SkString uses ebx esi,ctx,token,dest,capacity
 local wide[512]:WORD
 cmp [token],0
 je .bad
 stdcall JsonString,[ctx],[token],[dest],[capacity]
 test eax,eax
 jz .bad
 cmp eax,[capacity]
 jae .bad
 mov ebx,eax
 invoke lstrlenA,[dest]
 cmp eax,ebx
 jne .bad
 invoke MultiByteToWideChar,65001,8,[dest],-1,addr wide,512
 test eax,eax
 jz .bad
 mov esi,[dest]
.characters:
 lodsb
 test al,al
 jz .good
 cmp al,32
 jb .bad
 cmp al,127
 je .bad
 jmp .characters
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Profile IDs are exactly the authenticated Minecraft UUID (32 hex digits).
proc SkUuid uses esi,text
 invoke lstrlenA,[text]
 cmp eax,32
 jne .bad
 mov esi,[text]
 mov ecx,32
.next:
 lodsb
 cmp al,'0'
 jb .bad
 cmp al,'9'
 jbe .advance
 or al,32
 cmp al,'a'
 jb .bad
 cmp al,'f'
 ja .bad
.advance:
 loop .next
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Only the official texture host and one hexadecimal texture component.
; Old official http metadata is upgraded before any network operation.
proc SkTextureUrl uses ebx esi edi,urlA,destA
 mov esi,[urlA]
 mov edi,skHttpsA
 invoke lstrlenA,edi
 mov ebx,eax
 mov ecx,eax
 repe cmpsb
 je .hash
 mov esi,[urlA]
 mov edi,skHttpA
 invoke lstrlenA,edi
 mov ecx,eax
 repe cmpsb
 jne .bad
.hash:
 mov ebx,esi
 xor ecx,ecx
.hex:
 lodsb
 test al,al
 jz .hashEnd
 inc ecx
 cmp ecx,128
 ja .bad
 cmp al,'0'
 jb .bad
 cmp al,'9'
 jbe .hex
 cmp al,'a'
 jb .bad
 cmp al,'f'
 jbe .hex
 jmp .bad
.hashEnd:
 cmp ecx,32
 jb .bad
 invoke lstrcpyA,[destA],skHttpsA
 invoke lstrcatA,[destA],ebx
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SkClearCurrent
 cmp word [skinCurrentPathW],0
 je .free
 invoke DeleteFileW,skinCurrentPathW
.free:
 stdcall FreeMem,[skCurrentData]
 mov [skCurrentData],0
 mov [skCurrentBytes],0
 mov [skinReady],0
 mov [skinCurrentReady],0
 mov [skinModel],0
 mov [skinCurrentWidth],0
 mov [skinCurrentHeight],0
 mov byte [skinNameA],0
 mov byte [skinUuidA],0
 mov byte [skinUrlA],0
 mov byte [skinSkinIdA],0
 mov byte [skinCapeUrlA],0
 mov byte [skinCapeIdA],0
 mov byte [skinCapeAliasA],0
 mov word [skinCurrentPathW],0
 mov word [skinPreviewPathW],0
 ret
endp

proc SkinClear
 stdcall SkClearCurrent
 cmp word [skinLocalPathW],0
 je .free
 invoke DeleteFileW,skinLocalPathW
.free:
 stdcall FreeMem,[skLocalData]
 mov [skLocalData],0
 mov [skLocalBytes],0
 mov [skinLocalReady],0
 mov [skinLocalWidth],0
 mov [skinLocalHeight],0
 mov [skinLocalModel],0
 mov [skinResult],0
 mov [skinPreviewError],0
 mov word [skinLocalPathW],0
 ret
endp

; Own session/connection/request; redirects are disabled for BOTH hosts.
; Binary request length and a bounded binary response are always explicit.
proc SkTransport uses ebx esi edi,kind,method,targetW,headersW,body,bodyBytes,limit
 locals
  session dd ?
  connection dd ?
  request dd ?
  buffer dd ?
  total dd ?
  got dd ?
  size dd ?
  policy dd ?
  result dd ?
 endl
 mov [session],0
 mov [connection],0
 mov [request],0
 mov [buffer],0
 mov [total],0
 mov [result],0
 mov [skinHttpStatus],0
 mov [skResponseBytes],0
 stdcall SkCancelled
 test eax,eax
 jnz .done
 cmp [limit],SK_PNG_MAX
 ja .done
 invoke WinHttpOpen,skAgentW,0,0,0,0
 test eax,eax
 jz .done
 mov [session],eax
 invoke WinHttpSetTimeouts,eax,15000,15000,30000,30000
 test eax,eax
 jz .done
 mov eax,skApiHostW
 cmp [kind],0
 je .host
 cmp [kind],1
 jne .done
 cmp [headersW],0
 jne .done
 cmp [bodyBytes],0
 jne .done
 mov eax,skTextureHostW
.host:
 invoke WinHttpConnect,[session],eax,443,0
 test eax,eax
 jz .done
 mov [connection],eax
 invoke WinHttpOpenRequest,eax,[method],[targetW],0,0,0,00800000h
 test eax,eax
 jz .done
 mov [request],eax
 mov [policy],0
 invoke WinHttpSetOption,eax,88,addr policy,4
 test eax,eax
 jz .done
 stdcall SkCancelled
 test eax,eax
 jnz .done
 xor eax,eax
 cmp [headersW],0
 je .send
 dec eax
.send:
 invoke WinHttpSendRequest,[request],[headersW],eax,[body],[bodyBytes],[bodyBytes],0
 test eax,eax
 jz .done
 invoke WinHttpReceiveResponse,[request],0
 test eax,eax
 jz .done
 mov [size],4
 invoke WinHttpQueryHeaders,[request],20000013h,0,skinHttpStatus,addr size,0
 test eax,eax
 jz .done
 mov ebx,[limit]
 inc ebx
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 test eax,eax
 jz .done
 mov [buffer],eax
.read:
 stdcall SkCancelled
 test eax,eax
 jnz .done
 mov ebx,[limit]
 inc ebx
 sub ebx,[total]
 jbe .done
 cmp ebx,65536
 jbe .length
 mov ebx,65536
.length:
 mov edi,[buffer]
 add edi,[total]
 invoke WinHttpReadData,[request],edi,ebx,addr got
 test eax,eax
 jz .done
 mov eax,[got]
 cmp eax,ebx
 ja .done
 test eax,eax
 jz .complete
 add [total],eax
 mov eax,[total]
 cmp eax,[limit]
 ja .done
 jmp .read
.complete:
 stdcall SkCancelled
 test eax,eax
 jnz .done
 mov eax,[buffer]
 mov edx,[total]
 mov byte [eax+edx],0
 mov [skResponseBytes],edx
 mov [result],eax
 mov [buffer],0
.done:
 cmp [request],0
 je .connection
 invoke WinHttpCloseHandle,[request]
.connection:
 cmp [connection],0
 je .session
 invoke WinHttpCloseHandle,[connection]
.session:
 cmp [session],0
 je .free
 invoke WinHttpCloseHandle,[session]
.free:
 stdcall FreeMem,[buffer]
 mov eax,[result]
 ret
endp

; The test seam replaces transport only, retaining real multipart/parser/files.
proc SkFetch,kind,method,targetW,headersW,body,bodyBytes,limit
 if defined OFFSET_SKIN_TEST_TRANSPORT
 stdcall SkinTestTransport,[kind],[method],[targetW],[headersW],[body],[bodyBytes],[limit]
 else
 stdcall SkTransport,[kind],[method],[targetW],[headersW],[body],[bodyBytes],[limit]
 end if
 ret
endp

proc SkApi uses ebx esi edi,method,targetW,body,bodyBytes,extraW
 locals
  header rw 17000
  result dd ?
 endl
 mov [result],0
 mov word [header],0
 cmp [authReady],1
 jne .done
 stdcall SkUuid,authUuid
 test eax,eax
 jz .done
 invoke lstrlenA,authToken
 test eax,eax
 jz .done
 cmp eax,8191
 ja .done
 mov esi,authToken
.token:
 lodsb
 test al,al
 jz .header
 cmp al,33
 jb .done
 cmp al,126
 ja .done
 jmp .token
.header:
 invoke lstrcpyW,addr header,skAuthW
 invoke lstrlenW,addr header
 lea edi,[header+eax*2]
 invoke MultiByteToWideChar,65001,8,authToken,-1,edi,8192
 test eax,eax
 jz .done
 invoke lstrcatW,addr header,skCrLfW
 invoke lstrcatW,addr header,skAcceptW
 cmp [extraW],0
 je .fetch
 invoke lstrlenW,[extraW]
 cmp eax,512
 ja .done
 invoke lstrcatW,addr header,[extraW]
.fetch:
 stdcall SkFetch,0,[method],[targetW],addr header,[body],[bodyBytes],SK_JSON_MAX
 mov [result],eax
 cmp [skinHttpStatus],401
 jne .done
 stdcall AuthInvalidateSession
.done:
 lea edi,[header]
 xor eax,eax
 mov ecx,17000
 rep stosw
 mov eax,[result]
 ret
endp

proc SkPngCrc uses ebx esi,bytesPtr,bytes
 mov esi,[bytesPtr]
 mov ecx,[bytes]
 mov eax,-1
.byte:
 movzx edx,byte [esi]
 xor eax,edx
 mov ebx,8
.bit:
 shr eax,1
 jnc .next
 xor eax,0EDB88320h
.next:
 dec ebx
 jnz .bit
 inc esi
 dec ecx
 jnz .byte
 not eax
 ret
endp

; Bounded PNG chunk walk, including all CRCs and a final IEND. GDI+ can
; tolerate some incomplete images, so decoding alone is not file validation.
proc SkPngChunks uses ebx esi edi,data,bytes
 local length:DWORD,seenIdat:DWORD,closedIdat:DWORD,index:DWORD
 mov [seenIdat],0
 mov [closedIdat],0
 mov [index],0
 mov esi,[data]
 add esi,8
 mov edi,[bytes]
 sub edi,8
.chunk:
 stdcall SkCancelled
 test eax,eax
 jnz .bad
 cmp edi,12
 jb .bad
 inc [index]
 cmp [index],65536
 ja .bad
 mov eax,[esi]
 bswap eax
 mov [length],eax
 mov edx,edi
 sub edx,12
 cmp eax,edx
 ja .bad
 mov ebx,[esi+4]
 cmp ebx,052444849h ; IHDR
 jne .notHeader
 cmp [index],1
 jne .bad
 cmp [length],13
 jne .bad
 jmp .crc
.notHeader:
 cmp [index],1
 je .bad
 cmp ebx,054414449h ; IDAT
 jne .notData
 cmp [closedIdat],0
 jne .bad
 mov [seenIdat],1
 jmp .crc
.notData:
 cmp [seenIdat],0
 je .crc
 mov [closedIdat],1
.crc:
 lea eax,[esi+4]
 mov edx,[length]
 add edx,4
 stdcall SkPngCrc,eax,edx
 mov edx,[length]
 mov edx,[esi+edx+8]
 bswap edx
 cmp eax,edx
 jne .bad
 cmp ebx,0444E4549h ; IEND
 jne .advance
 cmp [length],0
 jne .bad
 cmp [seenIdat],1
 jne .bad
 cmp edi,12
 jne .bad
 mov eax,1
 ret
.advance:
 mov eax,[length]
 add eax,12
 add esi,eax
 sub edi,eax
 jmp .chunk
.bad:
 xor eax,eax
 ret
endp

; Validate PNG type/size before decoding. A cloned full raster forces pixels.
; Temporary names are CREATE_NEW and only a successfully created file is owned.
proc SkPngFile uses ebx esi edi,data,bytes,outPathW,outWidth,outHeight
 locals
  path rw 2048
  name rw 128
  file dd ?
  owned dd ?
  image dd ?
  clone dd ?
  written dd ?
  width dd ?
  height dd ?
  result dd ?
  tries dd ?
 endl
 mov [file],-1
 mov [owned],0
 mov [image],0
 mov [clone],0
 mov [result],0
 cmp [bytes],33
 jb .done
 cmp [bytes],SK_PNG_MAX
 ja .done
 mov esi,[data]
 cmp dword [esi],0474E5089h
 jne .done
 cmp dword [esi+4],0A1A0A0Dh
 jne .done
 cmp dword [esi+8],0D000000h
 jne .done
 cmp dword [esi+12],052444849h
 jne .done
 cmp dword [esi+16],040000000h
 jne .done
 mov eax,[esi+20]
 cmp eax,040000000h
 je .size
 cmp eax,020000000h
 jne .done
.size:
 stdcall SkPngChunks,[data],[bytes]
 test eax,eax
 jz .done
 stdcall SkCancelled
 test eax,eax
 jnz .done
 invoke GetTempPathW,1800,addr path
 test eax,eax
 jz .done
 cmp eax,1800
 jae .done
 mov [tries],8
.create:
 inc [skTempSerial]
 invoke GetCurrentProcessId
 mov ebx,eax
 invoke GetTickCount
 cinvoke wsprintfW,addr name,skTempFormatW,ebx,eax,[skTempSerial]
 invoke GetTempPathW,1800,addr path
 test eax,eax
 jz .done
 cmp eax,1800
 jae .done
 invoke lstrcatW,addr path,addr name
 invoke CreateFileW,addr path,GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,0
 cmp eax,-1
 jne .owned
 invoke GetLastError
 cmp eax,80
 je .retry
 cmp eax,183
 jne .done
.retry:
 dec [tries]
 jnz .create
 jmp .done
.owned:
 mov [file],eax
 mov [owned],1
 mov esi,[data]
 mov ebx,[bytes]
.write:
 stdcall SkCancelled
 test eax,eax
 jnz .done
 invoke WriteFile,[file],esi,ebx,addr written,0
 test eax,eax
 jz .done
 mov eax,[written]
 test eax,eax
 jz .done
 cmp eax,ebx
 ja .done
 add esi,eax
 sub ebx,eax
 jnz .write
 invoke CloseHandle,[file]
 mov [file],-1
 test eax,eax
 jz .done
 invoke GdipLoadImageFromFile,addr path,addr image
 test eax,eax
 jnz .done
 invoke GdipGetImageWidth,[image],addr width
 test eax,eax
 jnz .done
 invoke GdipGetImageHeight,[image],addr height
 test eax,eax
 jnz .done
 cmp [width],64
 jne .done
 cmp [height],64
 je .clone
 cmp [height],32
 jne .done
.clone:
 invoke GdipCloneBitmapAreaI,0,0,[width],[height],26200Ah,[image],addr clone
 test eax,eax
 jnz .done
 stdcall SkCancelled
 test eax,eax
 jnz .done
 mov edx,[outWidth]
 mov eax,[width]
 mov [edx],eax
 mov edx,[outHeight]
 mov eax,[height]
 mov [edx],eax
 invoke lstrcpyW,[outPathW],addr path
 mov [owned],0
 mov [result],1
.done:
 cmp [clone],0
 je .image
 invoke GdipDisposeImage,[clone]
.image:
 cmp [image],0
 je .file
 invoke GdipDisposeImage,[image]
.file:
 cmp [file],-1
 je .delete
 invoke CloseHandle,[file]
.delete:
 cmp [owned],0
 je .return
 invoke DeleteFileW,addr path
.return:
 mov eax,[result]
 ret
endp

proc SkinSelectLocal uses ebx esi edi,pathW
 locals
  file dd ?
  data dd ?
  bytes dd ?
  got dd ?
  remaining dd ?
  info BY_HANDLE_FILE_INFORMATION
  temp rw 2048
  width dd ?
  height dd ?
  result dd ?
 endl
 mov [file],-1
 mov [data],0
 mov [result],0
 mov [skinResult],0
 mov [skinPreviewError],0
 mov [skinHttpStatus],0
 invoke lstrlenW,[pathW]
 cmp eax,2000
 ja .done
 invoke CreateFileW,[pathW],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL+00200000h,0
 cmp eax,-1
 je .done
 mov [file],eax
 invoke GetFileInformationByHandle,eax,addr info
 test eax,eax
 jz .done
 test [info.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY+400h
 jnz .done
 cmp [info.nFileSizeHigh],0
 jne .done
 mov ebx,[info.nFileSizeLow]
 cmp ebx,33
 jb .done
 cmp ebx,SK_PNG_MAX
 ja .done
 mov [bytes],ebx
 mov [remaining],ebx
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 test eax,eax
 jz .done
 mov [data],eax
 mov esi,eax
.read:
 stdcall SkCancelled
 test eax,eax
 jnz .done
 invoke ReadFile,[file],esi,[remaining],addr got,0
 test eax,eax
 jz .done
 mov eax,[got]
 test eax,eax
 jz .done
 cmp eax,[remaining]
 ja .done
 add esi,eax
 sub [remaining],eax
 jnz .read
 stdcall SkPngFile,[data],[bytes],addr temp,addr width,addr height
 test eax,eax
 jz .done
 cmp word [skinLocalPathW],0
 je .replace
 invoke DeleteFileW,skinLocalPathW
.replace:
 stdcall FreeMem,[skLocalData]
 mov eax,[data]
 mov [skLocalData],eax
 mov [data],0
 mov eax,[bytes]
 mov [skLocalBytes],eax
 mov eax,[width]
 mov [skinLocalWidth],eax
 mov eax,[height]
 mov [skinLocalHeight],eax
 mov [skinLocalReady],1
 invoke lstrcpyW,skinLocalPathW,addr temp
 invoke lstrcpyW,skinPreviewPathW,addr temp
 mov [result],1
.done:
 cmp [file],-1
 je .free
 invoke CloseHandle,[file]
.free:
 stdcall FreeMem,[data]
 mov eax,[result]
 mov [skinResult],eax
 test eax,eax
 jz .bad
 stdcall SetStatusA,skMsgLocalA
 mov eax,1
 ret
.bad:
 stdcall SetStatusA,skMsgInvalidA
 stdcall SkCancelled
 test eax,eax
 jz .return
 stdcall SetStatusA,skMsgCancelA
.return:
 xor eax,eax
 ret
endp

; Parse into private staging buffers with a private context, then publish.
proc SkParseProfile uses ebx esi edi,text
 locals
  ctx dd ?
  root dd ?
  list dd ?
  item dd ?
  count dd ?
  index dd ?
  pass dd ?
  active dd ?
  state rb 64
  value rb 512
  result dd ?
 endl
 mov [ctx],0
 mov [result],0
 mov edi,skPendingStart
 xor eax,eax
 mov ecx,skPendingEnd-skPendingStart
 rep stosb
 stdcall JsonParse,[text]
 test eax,eax
 jz .done
 mov [ctx],eax
 stdcall JsonRoot,eax
 test eax,eax
 jz .done
 mov [root],eax
 stdcall JsonGet,[ctx],eax,skKeyIdA
 stdcall SkString,[ctx],eax,skPendingUuidA,64
 test eax,eax
 jz .done
 stdcall SkUuid,skPendingUuidA
 test eax,eax
 jz .done
 stdcall SkEqualA,skPendingUuidA,authUuid
 test eax,eax
 jz .done
 stdcall JsonGet,[ctx],[root],skKeyNameA
 stdcall SkString,[ctx],eax,skPendingNameA,64
 test eax,eax
 jz .done
 invoke lstrlenA,skPendingNameA
 cmp eax,16
 ja .done
 mov esi,skPendingNameA
.name:
 lodsb
 test al,al
 jz .arrays
 cmp al,'_'
 je .name
 cmp al,'0'
 jb .done
 cmp al,'9'
 jbe .name
 or al,32
 cmp al,'a'
 jb .done
 cmp al,'z'
 ja .done
 jmp .name
.arrays:
 mov [pass],0
.array:
 mov eax,skKeySkinsA
 cmp [pass],0
 je .key
 mov eax,skKeyCapesA
.key:
 stdcall JsonGet,[ctx],[root],eax
 test eax,eax
 jz .done
 mov [list],eax
 ; JsonCount returns zero for wrong types, so also assert a raw array.
 stdcall JsonRaw,[ctx],eax,addr value,512
 cmp eax,2
 jb .done
 cmp byte [value],'['
 jne .done
 stdcall JsonCount,[ctx],[list]
 cmp eax,64
 ja .done
 mov [count],eax
 mov [index],0
 mov [active],0
.entry:
 mov eax,[index]
 cmp eax,[count]
 jae .nextArray
 stdcall JsonAt,[ctx],[list],eax
 mov [item],eax
 stdcall JsonGet,[ctx],eax,skKeyStateA
 stdcall SkString,[ctx],eax,addr state,64
 test eax,eax
 jz .done
 invoke lstrcmpA,addr state,skActiveA
 test eax,eax
 jnz .next
 cmp [active],0
 jne .done
 mov [active],1
 stdcall JsonGet,[ctx],[item],skKeyUrlA
 stdcall SkString,[ctx],eax,addr value,512
 test eax,eax
 jz .done
 mov edi,skPendingUrlA
 cmp [pass],0
 je .url
 mov edi,skPendingCapeUrlA
.url:
 stdcall SkTextureUrl,addr value,edi
 test eax,eax
 jz .done
 stdcall JsonGet,[ctx],[item],skKeyIdA
 mov edi,skPendingSkinIdA
 cmp [pass],0
 je .id
 mov edi,skPendingCapeIdA
.id:
 stdcall SkString,[ctx],eax,edi,64
 test eax,eax
 jz .done
 cmp [pass],0
 jne .cape
 stdcall JsonGet,[ctx],[item],skKeyVariantA
 stdcall SkString,[ctx],eax,addr value,512
 test eax,eax
 jz .done
 stdcall SkEqualA,addr value,skClassicA
 test eax,eax
 jnz .next
 stdcall SkEqualA,addr value,skSlimA
 test eax,eax
 jz .done
 mov [skPendingModel],1
 jmp .next
.cape:
 stdcall JsonGet,[ctx],[item],skKeyAliasA
 test eax,eax
 jz .next
 stdcall SkString,[ctx],eax,skPendingCapeAliasA,128
 test eax,eax
 jz .done
.next:
 inc [index]
 jmp .entry
.nextArray:
 inc [pass]
 cmp [pass],2
 jb .array
 mov [result],1
.done:
 cmp [ctx],0
 je .return
 stdcall JsonFree,[ctx]
.return:
 mov eax,[result]
 ret
endp

proc SkLoadCurrent uses ebx esi edi
 locals
  response dd ?
  data dd ?
  bytes dd ?
  target rw 512
  temp rw 2048
  width dd ?
  height dd ?
  result dd ?
 endl
 mov [response],0
 mov [data],0
 mov [result],0
 stdcall SkClearCurrent
 stdcall SkApi,skGetW,skProfileW,0,0,0
 mov [response],eax
 test eax,eax
 jz .done
 cmp [skinHttpStatus],200
 jne .done
 invoke lstrlenA,[response]
 cmp eax,[skResponseBytes]
 jne .done
 mov eax,[response]
 stdcall SkParseProfile,eax
 test eax,eax
 jz .done
 stdcall SkCancelled
 test eax,eax
 jnz .done
 invoke lstrcpyA,skinNameA,skPendingNameA
 invoke lstrcpyA,skinUuidA,skPendingUuidA
 invoke lstrcpyA,skinUrlA,skPendingUrlA
 invoke lstrcpyA,skinSkinIdA,skPendingSkinIdA
 invoke lstrcpyA,skinCapeUrlA,skPendingCapeUrlA
 invoke lstrcpyA,skinCapeIdA,skPendingCapeIdA
 invoke lstrcpyA,skinCapeAliasA,skPendingCapeAliasA
 mov eax,[skPendingModel]
 mov [skinModel],eax
 mov [skinReady],1
 cmp byte [skinUrlA],0
 je .success
 ; The staged URL is canonical HTTPS. Only its safe /texture/hash tail is sent.
 invoke lstrlenA,skHttpsA
 lea ebx,[skinUrlA+eax]
 invoke lstrcpyA,skPendingUrlA,skTexturePathA
 invoke lstrcatA,skPendingUrlA,ebx
 invoke MultiByteToWideChar,65001,8,skPendingUrlA,-1,addr target,512
 test eax,eax
 jz .done
 stdcall SkFetch,1,skGetW,addr target,0,0,0,SK_PNG_MAX
 mov [data],eax
 test eax,eax
 jz .done
 cmp [skinHttpStatus],200
 jne .done
 mov eax,[skResponseBytes]
 mov [bytes],eax
 stdcall SkPngFile,[data],eax,addr temp,addr width,addr height
 test eax,eax
 jz .done
 mov eax,[data]
 mov [skCurrentData],eax
 mov [data],0
 mov eax,[bytes]
 mov [skCurrentBytes],eax
 mov eax,[width]
 mov [skinCurrentWidth],eax
 mov eax,[height]
 mov [skinCurrentHeight],eax
 invoke lstrcpyW,skinCurrentPathW,addr temp
 invoke lstrcpyW,skinPreviewPathW,addr temp
 mov [skinCurrentReady],1
.success:
 mov [result],1
.done:
 stdcall FreeMem,[data]
 stdcall SkFreeText,[response]
 mov eax,[result]
 ret
endp

proc SkFailureStatus
 stdcall SkCancelled
 test eax,eax
 jnz .cancel
 cmp [skinHttpStatus],401
 je .expired
 stdcall SetStatusA,skMsgFailedA
 xor eax,eax
 ret
.expired:
 stdcall SetStatusA,skMsgExpiredA
 xor eax,eax
 ret
.cancel:
 stdcall SetStatusA,skMsgCancelA
 xor eax,eax
 ret
endp

proc SkFreeText uses esi edi,response
 cmp [response],0
 je .done
 invoke lstrlenA,[response]
 mov ecx,eax
 mov edi,[response]
 xor eax,eax
 rep stosb
 stdcall FreeMem,[response]
.done:
 ret
endp

proc SkinRefresh
 inc [skinSerial]
 mov [skinResult],0
 mov [skinPreviewError],0
 mov [skinHttpStatus],0
 stdcall SetStatusA,skMsgRefreshA
 stdcall AuthEnsureSession
 test eax,eax
 jz .session
 stdcall SkLoadCurrent
 test eax,eax
 jz .bad
 mov [skinResult],1
 stdcall SetStatusA,skMsgReadyA
 mov eax,1
 ret
.session:
 stdcall SkClearCurrent
 stdcall SkCancelled
 test eax,eax
 jnz .cancel
 stdcall SetStatusA,skMsgSessionA
 xor eax,eax
 ret
.cancel:
 stdcall SkFailureStatus
 ret
.bad:
 mov [skinPreviewError],1
 stdcall SkFailureStatus
 ret
endp

; Append a bounded ASCII part to a preallocated multipart body.
proc SkAppend uses esi edi,buffer,used,text
 mov edi,[buffer]
 mov edx,[used]
 mov ecx,[edx]
 add edi,ecx
 mov esi,[text]
.next:
 lodsb
 test al,al
 jz .done
 stosb
 inc ecx
 jmp .next
.done:
 mov [edx],ecx
 ret
endp

proc SkinUpload uses ebx esi edi,model
 locals
  boundary rb 128
  header rw 256
  boundaryW rw 128
  data dd ?
  used dd ?
  response dd ?
  success dd ?
  tries dd ?
 endl
 inc [skinSerial]
 mov [skinResult],0
 mov [skinPreviewError],0
 mov [skinHttpStatus],0
 mov [data],0
 mov [response],0
 mov [success],0
 cmp [model],1
 ja .invalid
 cmp [skinLocalReady],1
 jne .invalid
 cmp [skLocalData],0
 je .invalid
 cmp [skLocalBytes],SK_PNG_MAX
 ja .invalid
 ; The 64x32 legacy atlas has only classic four-pixel arms.
 cmp [skinLocalHeight],32
 jne @f
 mov [model],0
 mov [skinLocalModel],0
@@:
 stdcall AuthEnsureSession
 test eax,eax
 jz .session
 stdcall SkCancelled
 test eax,eax
 jnz .done
 mov [tries],8
.boundary:
 inc [skTempSerial]
 invoke GetCurrentProcessId
 mov ebx,eax
 invoke GetTickCount
 cinvoke wsprintfA,addr boundary,skBoundaryFormatA,ebx,eax,[skTempSerial]
 ; Reject a boundary present anywhere in the frozen PNG before constructing it.
 invoke lstrlenA,addr boundary
 mov ebx,eax
 mov esi,[skLocalData]
 mov ecx,[skLocalBytes]
 sub ecx,ebx
 jc .body
 inc ecx
.scan:
 push ecx esi
 lea edi,[boundary]
 mov ecx,ebx
 repe cmpsb
 pop esi ecx
 je .collision
 inc esi
 loop .scan
 jmp .body
.collision:
 dec [tries]
 jnz .boundary
 jmp .done
.body:
 mov ebx,[skLocalBytes]
 add ebx,2048
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 test eax,eax
 jz .done
 mov [data],eax
 mov [used],0
 stdcall SkAppend,eax,addr used,skDashA
 stdcall SkAppend,[data],addr used,addr boundary
 stdcall SkAppend,[data],addr used,skPartOneA
 mov eax,skClassicA
 cmp [model],0
 je .variant
 mov eax,skSlimA
.variant:
 stdcall SkAppend,[data],addr used,eax
 stdcall SkAppend,[data],addr used,skCrLfA
 stdcall SkAppend,[data],addr used,skDashA
 stdcall SkAppend,[data],addr used,addr boundary
 stdcall SkAppend,[data],addr used,skPartTwoA
 mov edi,[data]
 add edi,[used]
 mov esi,[skLocalData]
 mov ecx,[skLocalBytes]
 rep movsb
 mov eax,[skLocalBytes]
 add [used],eax
 stdcall SkAppend,[data],addr used,skCrLfA
 stdcall SkAppend,[data],addr used,skDashA
 stdcall SkAppend,[data],addr used,addr boundary
 stdcall SkAppend,[data],addr used,skDashA
 stdcall SkAppend,[data],addr used,skCrLfA
 invoke MultiByteToWideChar,65001,8,addr boundary,-1,addr boundaryW,128
 test eax,eax
 jz .done
 invoke lstrcpyW,addr header,skMultipartW
 invoke lstrcatW,addr header,addr boundaryW
 invoke lstrcatW,addr header,skCrLfW
 stdcall SetStatusA,skMsgUploadA
 stdcall SkApi,skPostW,skUploadW,[data],[used],addr header
 mov [response],eax
 test eax,eax
 jz .done
 cmp [skinHttpStatus],200
 je .changed
 cmp [skinHttpStatus],204
 jne .done
.changed:
 mov [success],1
 mov eax,[model]
 mov [skinLocalModel],eax
 stdcall SkLoadCurrent
 test eax,eax
 jnz .done
 mov [skinPreviewError],1
.done:
 stdcall FreeMem,[data]
 stdcall SkFreeText,[response]
 mov eax,[success]
 mov [skinResult],eax
 test eax,eax
 jz .failed
 cmp [skinPreviewError],0
 jne .preview
 stdcall SetStatusA,skMsgUploadedA
 mov eax,1
 ret
.preview:
 stdcall SetStatusA,skMsgPreviewA
 mov eax,1
 ret
.failed:
 stdcall SkFailureStatus
 ret
.invalid:
 stdcall SetStatusA,skMsgInvalidA
 xor eax,eax
 ret
.session:
 stdcall SkClearCurrent
 stdcall SkCancelled
 test eax,eax
 jnz .failed
 stdcall SetStatusA,skMsgSessionA
 xor eax,eax
 ret
endp

proc SkinReset
 local response:DWORD,success:DWORD
 inc [skinSerial]
 mov [skinResult],0
 mov [skinPreviewError],0
 mov [skinHttpStatus],0
 mov [success],0
 stdcall AuthEnsureSession
 test eax,eax
 jz .session
 stdcall SkApi,skDeleteW,skActiveW,0,0,0
 mov [response],eax
 test eax,eax
 jz .done
 cmp [skinHttpStatus],200
 je .changed
 cmp [skinHttpStatus],204
 jne .done
.changed:
 mov [success],1
 stdcall SkLoadCurrent
 test eax,eax
 jnz .done
 mov [skinPreviewError],1
.done:
 stdcall SkFreeText,[response]
 mov eax,[success]
 mov [skinResult],eax
 test eax,eax
 jz .failed
 cmp [skinPreviewError],0
 jne .preview
 stdcall SetStatusA,skMsgResetA
 mov eax,1
 ret
.preview:
 stdcall SetStatusA,skMsgPreviewA
 mov eax,1
 ret
.failed:
 stdcall SkFailureStatus
 ret
.session:
 stdcall SkClearCurrent
 stdcall SkCancelled
 test eax,eax
 jnz .failed
 stdcall SetStatusA,skMsgSessionA
 xor eax,eax
 ret
endp

; One export filename component. Reject streams, device aliases and invalid
; Windows characters before creating a temporary output beside the target.
proc SkExportLeaf uses ebx esi edi,nameW
 local stem[128]:WORD
 invoke lstrlenW,[nameW]
 test eax,eax
 jz .bad
 cmp eax,120
 ja .bad
 mov esi,[nameW]
 lea edi,[stem]
 xor ebx,ebx
.next:
 lodsw
 test ax,ax
 jz .tail
 cmp ax,32
 jb .bad
 cmp ax,127
 je .bad
 cmp ax,':'
 je .bad
 cmp ax,'*'
 je .bad
 cmp ax,'?'
 je .bad
 cmp ax,'"'
 je .bad
 cmp ax,'<'
 je .bad
 cmp ax,'>'
 je .bad
 cmp ax,'|'
 je .bad
 cmp ax,92
 je .bad
 cmp ax,'/'
 je .bad
 cmp ax,'.'
 jne .copy
 test ebx,ebx
 jnz .copy
 mov word [edi],0
 mov ebx,1
.copy:
 test ebx,ebx
 jnz .next
 stosw
 jmp .next
.tail:
 cmp word [esi-4],'.'
 je .bad
 cmp word [esi-4],' '
 je .bad
 test ebx,ebx
 jnz .devices
 mov word [edi],0
.devices:
 invoke lstrcmpiW,addr stem,skConW
 test eax,eax
 jz .bad
 invoke lstrcmpiW,addr stem,skPrnW
 test eax,eax
 jz .bad
 invoke lstrcmpiW,addr stem,skAuxW
 test eax,eax
 jz .bad
 invoke lstrcmpiW,addr stem,skNulW
 test eax,eax
 jz .bad
 invoke lstrcmpiW,addr stem,skConInW
 test eax,eax
 jz .bad
 invoke lstrcmpiW,addr stem,skConOutW
 test eax,eax
 jz .bad
 invoke lstrlenW,addr stem
 cmp eax,4
 jne .good
 mov ax,[stem+6]
 cmp ax,'1'
 jb .superscript
 cmp ax,'9'
 jbe .port
.superscript:
 cmp ax,00B9h
 je .port
 cmp ax,00B2h
 je .port
 cmp ax,00B3h
 jne .good
.port:
 mov ax,[stem]
 or ax,32
 cmp ax,'c'
 jne .lpt
 mov ax,[stem+2]
 or ax,32
 cmp ax,'o'
 jne .good
 mov ax,[stem+4]
 or ax,32
 cmp ax,'m'
 je .bad
.lpt:
 mov ax,[stem]
 or ax,32
 cmp ax,'l'
 jne .good
 mov ax,[stem+2]
 or ax,32
 cmp ax,'p'
 jne .good
 mov ax,[stem+4]
 or ax,32
 cmp ax,'t'
 je .bad
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc SkinExportCurrent uses ebx esi edi,destW,replaceAllowed
 locals
  destination rw 2048
  temporary rw 2048
  suffix rw 128
  file dd ?
  owned dd ?
  written dd ?
  result dd ?
 endl
 mov [file],-1
 mov [owned],0
 mov [result],0
 mov [skinResult],0
 mov [skinPreviewError],0
 mov [skinHttpStatus],0
 cmp [replaceAllowed],1
 ja .done
 cmp [skinCurrentReady],1
 jne .done
 cmp [skCurrentData],0
 je .done
 invoke GetFullPathNameW,[destW],2000,addr destination,0
 test eax,eax
 jz .done
 cmp eax,1950
 ja .done
 cmp word [destination+2],':'
 jne .done
 cmp word [destination+4],92
 jne .done
 lea edi,[destination+eax*2]
 cmp eax,7
 jb .done
 lea esi,[edi-8]
 invoke lstrcmpiW,esi,skPngExtW
 test eax,eax
 jnz .done
 invoke lstrlenW,addr destination
 lea esi,[destination+eax*2]
.leaf:
 sub esi,2
 cmp word [esi],92
 jne .leaf
 add esi,2
 stdcall SkExportLeaf,esi
 test eax,eax
 jz .done
 invoke lstrcmpiW,addr destination,skinCurrentPathW
 test eax,eax
 jz .done
 cmp word [skinLocalPathW],0
 je .attributes
 invoke lstrcmpiW,addr destination,skinLocalPathW
 test eax,eax
 jz .done
.attributes:
 invoke GetFileAttributesW,addr destination
 cmp eax,-1
 je .parent
 test eax,FILE_ATTRIBUTE_DIRECTORY+400h
 jnz .done
 cmp [replaceAllowed],1
 jne .done
.parent:
 invoke lstrcpyW,addr temporary,addr destination
 invoke lstrlenW,addr temporary
 lea esi,[temporary+eax*2]
.slash:
 sub esi,2
 cmp word [esi],92
 jne .slash
 mov word [esi],0
 lea eax,[temporary+4]
 cmp esi,eax
 jne .chain
 mov word [esi],92
 mov word [esi+2],0
.chain:
 stdcall VsDirectoryChain,addr temporary,0
 test eax,eax
 jz .done
 invoke lstrcpyW,addr temporary,addr destination
 inc [skTempSerial]
 invoke GetTickCount
 cinvoke wsprintfW,addr suffix,skExportFormatW,eax,[skTempSerial]
 invoke lstrcatW,addr temporary,addr suffix
 stdcall SkCancelled
 test eax,eax
 jnz .done
 invoke CreateFileW,addr temporary,GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .done
 mov [file],eax
 mov [owned],1
 mov esi,[skCurrentData]
 mov ebx,[skCurrentBytes]
.write:
 stdcall SkCancelled
 test eax,eax
 jnz .done
 invoke WriteFile,[file],esi,ebx,addr written,0
 test eax,eax
 jz .done
 mov eax,[written]
 test eax,eax
 jz .done
 cmp eax,ebx
 ja .done
 add esi,eax
 sub ebx,eax
 jnz .write
 invoke CloseHandle,[file]
 mov [file],-1
 test eax,eax
 jz .done
 stdcall SkCancelled
 test eax,eax
 jnz .done
 mov eax,[replaceAllowed]
 or eax,8
 invoke MoveFileExW,addr temporary,addr destination,eax
 test eax,eax
 jz .done
 mov [owned],0
 mov [result],1
.done:
 cmp [file],-1
 je .cleanup
 invoke CloseHandle,[file]
.cleanup:
 cmp [owned],0
 je .status
 invoke DeleteFileW,addr temporary
.status:
 mov eax,[result]
 mov [skinResult],eax
 test eax,eax
 jz .failed
 stdcall SetStatusA,skMsgExportA
 mov eax,1
 ret
.failed:
 stdcall SkFailureStatus
 ret
endp
