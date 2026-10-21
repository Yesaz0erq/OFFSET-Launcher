; Per-profile settings. Filesystem paths are UTF-16; profiles remain original.
VS_NAME = 0
VS_JAVA = 80
VS_MEMORY = 2128
VS_ISOLATE = 2132
VS_ROOT = 2136
VS_ID = 4184
VS_PROFILE = 4312
VS_DATA = 8408
VS_BASE = 12504
VS_LOADER = 12632
VS_VALID = 12636
VS_SIZE = 12640

section '.vsdata' data readable writeable
vsSection du 'Profile',0
vsKeyName du 'NameHex',0
vsKeyJava du 'JavaHex',0
vsKeyMemory du 'MemoryMiB',0
vsKeyIsolate du 'Isolate',0
vsIniSuffix du '\.offset-profile.ini',0
vsTempFormat du '.tmp-%08X',0
vsVersions du '\versions\',0
vsSlash du '\',0
vsGameSuffix du '\game',0
vsJsonExt du '.json',0
vsJarExt du '.jar',0
vsEmpty dw 0
vsZero du '0',0
vsDecimal du '%u',0
vsHexDigits du '0123456789ABCDEF'
vsKeyIdA db 'id',0
vsKeyInheritsA db 'inheritsFrom',0
vsKeyArgumentsA db 'arguments',0
vsKeyMainA db 'mainClass',0
vsKeyBaseA db 'offsetBase',0
vsKeyNormalizedA db 'offsetNormalized',0
vsKeyLibrariesA db 'libraries',0
vsKeyNameA db 'name',0
vsFabricA db 'net.fabricmc:fabric-loader:',0
vsForgeA db 'net.minecraftforge:forge:',0
vsNeoA db 'net.neoforged:neoforge:',0
vsQuiltA db 'org.quiltmc:quilt-loader:',0
vsBom dw 0FEFFh
vsNameW rw 40
vsJavaW rw 1024
vsMemory dd ?
vsIsolate dd ?
vsRootW rw 1024
vsIdA rb 128
vsProfileDirW rw 2048
vsDataDirW rw 2048
vsBaseVersionA rb 128
vsLoaderKind dd ?
vsContextValid dd ?

section '.vscode' code readable executable
 nop
proc VsSafeId uses ebx esi edi,text
 local stem[128]:BYTE
 mov esi,[text]
 test esi,esi
 jz .bad
 lea edi,[stem]
 xor ecx,ecx
 xor ebx,ebx
.scan:
 mov al,[esi+ecx]
 test al,al
 jz .finish
 cmp ecx,127
 jae .bad
 cmp al,'.'
 je .dot
 cmp al,'-'
 je .char
 cmp al,'_'
 je .char
 cmp al,'0'
 jb .bad
 cmp al,'9'
 jbe .char
 cmp al,'A'
 jb .bad
 cmp al,'Z'
 jbe .char
 cmp al,'a'
 jb .bad
 cmp al,'z'
 ja .bad
 jmp .char
.dot:
 test ecx,ecx
 jz .bad
 cmp byte [esi+ecx+1],'.'
 je .bad
 cmp byte [esi+ecx+1],0
 je .bad
 mov ebx,1
.char:
 test ebx,ebx
 jnz .next
 cmp al,'A'
 jb .lower
 cmp al,'Z'
 ja .lower
 add al,32
.lower:
 stosb
.next:
 inc ecx
 jmp .scan
.finish:
 test ecx,ecx
 jz .bad
 mov byte [edi],0
 lea esi,[stem]
 sub edi,esi
 cmp edi,3
 jne .four
 cmp word [esi],'co'
 jne .prn
 cmp byte [esi+2],'n'
 je .bad
.prn:
 cmp word [esi],'pr'
 jne .aux
 cmp byte [esi+2],'n'
 je .bad
.aux:
 cmp word [esi],'au'
 jne .nul
 cmp byte [esi+2],'x'
 je .bad
.nul:
 cmp word [esi],'nu'
 jne .good
 cmp byte [esi+2],'l'
 je .bad
 jmp .good
.four:
 cmp edi,4
 jne .good
 cmp byte [esi+3],'1'
 jb .good
 cmp byte [esi+3],'9'
 ja .good
 cmp word [esi],'co'
 jne .lpt
 cmp byte [esi+2],'m'
 je .bad
.lpt:
 cmp word [esi],'lp'
 jne .good
 cmp byte [esi+2],'t'
 je .bad
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Every existing parent must be a real directory. Optional missing tails are
; allowed only when resolving a future data directory, never for installed UI.
proc VsDirectoryChain uses ebx esi edi,path,allowMissing
 local copy[2048]:WORD, saved:DWORD
 invoke lstrlenW,[path]
 cmp eax,2048
 jae .bad
 invoke lstrcpyW,addr copy,[path]
 lea esi,[copy]
 cmp word [esi+2],':'
 jne .unc
 cmp word [esi+4],92
 jne .bad
 add esi,6
 jmp .walk
.unc:
 cmp word [esi],92
 jne .bad
 cmp word [esi+2],92
 jne .bad
 cmp word [esi+4],'?'
 je .bad
 cmp word [esi+4],'.'
 je .bad
 add esi,4
.server:
 cmp word [esi],0
 je .bad
 cmp word [esi],92
 je .shareStart
 add esi,2
 jmp .server
.shareStart:
 add esi,2
 cmp word [esi],0
 je .bad
.share:
 cmp word [esi],0
 je .check
 cmp word [esi],92
 je .check
 add esi,2
 jmp .share
.walk:
 cmp word [esi],0
 je .check
 cmp word [esi],92
 je .check
 add esi,2
 jmp .walk
.check:
 movzx eax,word [esi]
 mov [saved],eax
 mov word [esi],0
 invoke GetFileAttributesW,addr copy
 mov edx,[saved]
 mov [esi],dx
 cmp eax,-1
 je .missing
 test eax,400h
 jnz .bad
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .bad
 cmp [saved],0
 je .good
 add esi,2
 jmp .walk
.missing:
 cmp [allowMissing],0
 je .bad
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc VsRegularFile,path
 invoke GetFileAttributesW,[path]
 cmp eax,-1
 je .bad
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc VsValidText uses esi,text,maximum
 mov esi,[text]
 xor ecx,ecx
.next:
 movzx eax,word [esi+ecx*2]
 test eax,eax
 jz .good
 cmp ecx,[maximum]
 jae .bad
 cmp eax,32
 jb .bad
 cmp eax,127
 je .bad
 cmp eax,0D800h
 jb .advance
 cmp eax,0DFFFh
 ja .advance
 cmp eax,0DBFFh
 ja .bad
 inc ecx
 cmp ecx,[maximum]
 jae .bad
 movzx eax,word [esi+ecx*2]
 cmp eax,0DC00h
 jb .bad
 cmp eax,0DFFFh
 ja .bad
.advance:
 inc ecx
 jmp .next
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc VsEncode uses ebx esi edi,source,destination
 mov esi,[source]
 mov edi,[destination]
.unit:
 movzx ebx,word [esi]
 add esi,2
 test ebx,ebx
 jz .done
 mov ecx,12
.digit:
 mov eax,ebx
 shr eax,cl
 and eax,15
 mov ax,[vsHexDigits+eax*2]
 stosw
 sub ecx,4
 jns .digit
 jmp .unit
.done:
 mov word [edi],0
 ret
endp
proc VsDecode uses ebx esi edi,source,destination,capacity
 mov esi,[source]
 mov edi,[destination]
 xor ecx,ecx
.unit:
 cmp word [esi],0
 je .good
 mov eax,[capacity]
 dec eax
 cmp ecx,eax
 jae .bad
 xor ebx,ebx
 mov edx,4
.digit:
 movzx eax,word [esi]
 add esi,2
 cmp eax,'0'
 jb .bad
 cmp eax,'9'
 jbe .number
 or eax,20h
 cmp eax,'a'
 jb .bad
 cmp eax,'f'
 ja .bad
 sub eax,'a'-10
 jmp .append
.number:
 sub eax,'0'
.append:
 shl ebx,4
 or ebx,eax
 dec edx
 jnz .digit
 test ebx,ebx
 jz .bad
 mov ax,bx
 stosw
 inc ecx
 jmp .unit
.good:
 mov word [edi],0
 mov eax,[capacity]
 dec eax
 stdcall VsValidText,[destination],eax
 test eax,eax
 jz .bad
 ret
.bad:
 mov edi,[destination]
 mov word [edi],0
 xor eax,eax
 ret
endp
proc VsString uses ebx,ctx,token,destination,capacity
 stdcall JsonString,[ctx],[token],[destination],[capacity]
 test eax,eax
 jz .bad
 cmp eax,[capacity]
 jae .bad
 mov ebx,eax
 invoke lstrlenA,[destination]
 cmp eax,ebx
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc VsPrefix uses esi edi,source,prefix
 mov esi,[source]
 mov edi,[prefix]
.next:
 mov al,[edi]
 test al,al
 jz .suffix
 cmp al,[esi]
 jne .bad
 inc esi
 inc edi
 jmp .next
.suffix:
 cmp byte [esi],0
 je .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc VsMetadata uses ebx esi edi,record
 locals
  path rw 2048
  value rb 512
  wideId rw 128
  response dd ?
  ctx dd ?
  root dd ?
  list dd ?
  index dd ?
  count dd ?
  flags dd ?
 endl
 mov [response],0
 mov [ctx],0
 mov ebx,[record]
 lea eax,[ebx+VS_ID]
 invoke MultiByteToWideChar,65001,8,eax,-1,addr wideId,128
 test eax,eax
 jz .bad
 lea eax,[ebx+VS_PROFILE]
 invoke lstrcpyW,addr path,eax
 invoke lstrcatW,addr path,vsSlash
 invoke lstrcatW,addr path,addr wideId
 invoke lstrcatW,addr path,vsJarExt
 stdcall VsRegularFile,addr path
 test eax,eax
 jz .bad
 invoke lstrlenW,addr path
 ; Replace .jar with .json without relying on an overlapping copy.
 mov word [path+eax*2-8],0
 invoke lstrcatW,addr path,vsJsonExt
 stdcall VsRegularFile,addr path
 test eax,eax
 jz .bad
 stdcall EReadFile,addr path
 test eax,eax
 jz .bad
 mov [response],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .bad
 mov [ctx],eax
 stdcall JsonRoot,eax
 mov [root],eax
 stdcall JsonGet,[ctx],eax,vsKeyIdA
 stdcall VsString,[ctx],eax,addr value,512
 test eax,eax
 jz .bad
 lea eax,[ebx+VS_ID]
 invoke lstrcmpA,addr value,eax
 test eax,eax
 jnz .bad
 stdcall JsonGet,[ctx],[root],vsKeyInheritsA
 test eax,eax
 jnz .bad
 stdcall JsonGet,[ctx],[root],vsKeyArgumentsA
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],[root],vsKeyMainA
 stdcall VsString,[ctx],eax,addr value,512
 test eax,eax
 jz .bad
 stdcall JsonGet,[ctx],[root],vsKeyNormalizedA
 stdcall JsonBool,[ctx],eax
 cmp eax,1
 jne .libraries
 stdcall JsonGet,[ctx],[root],vsKeyBaseA
 stdcall VsString,[ctx],eax,addr value,128
 test eax,eax
 jz .libraries
 stdcall VsSafeId,addr value
 test eax,eax
 jz .libraries
 lea eax,[ebx+VS_BASE]
 invoke lstrcpyA,eax,addr value
.libraries:
 mov [flags],0
 stdcall JsonGet,[ctx],[root],vsKeyLibrariesA
 mov [list],eax
 stdcall JsonCount,[ctx],eax
 cmp eax,4096
 ja .bad
 mov [count],eax
 mov [index],0
.library:
 mov eax,[index]
 cmp eax,[count]
 jae .choose
 stdcall JsonAt,[ctx],[list],eax
 stdcall JsonGet,[ctx],eax,vsKeyNameA
 stdcall VsString,[ctx],eax,addr value,512
 test eax,eax
 jz .nextLibrary
 stdcall VsPrefix,addr value,vsNeoA
 test eax,eax
 jnz .neo
 stdcall VsPrefix,addr value,vsForgeA
 test eax,eax
 jz .quilt
 or [flags],2
 jmp .nextLibrary
.quilt:
 stdcall VsPrefix,addr value,vsQuiltA
 test eax,eax
 jz .fabric
 or [flags],8
 jmp .nextLibrary
.fabric:
 stdcall VsPrefix,addr value,vsFabricA
 test eax,eax
 jz .nextLibrary
 or [flags],1
.nextLibrary:
 inc [index]
 jmp .library
.choose:
 mov eax,2
 test [flags],2
 jnz .loader
 mov eax,4
 test [flags],8
 jnz .loader
 mov eax,1
 test [flags],1
 jnz .loader
 xor eax,eax
 jmp .loader
.neo:
 mov eax,3
.loader:
 mov [ebx+VS_LOADER],eax
 stdcall JsonFree,[ctx]
 stdcall FreeMem,[response]
 mov eax,1
 ret
.bad:
 stdcall JsonFree,[ctx]
 stdcall FreeMem,[response]
 xor eax,eax
 ret
endp

proc VsRead uses ebx esi edi,rootPath,id,record,installed
 locals
  path rw 2048
  wideId rw 128
  value rw 4112
 endl
 mov edi,[record]
 xor eax,eax
 mov ecx,VS_SIZE/4
 rep stosd
 stdcall VsSafeId,[id]
 test eax,eax
 jz .bad
 mov eax,[rootPath]
 cmp word [eax],0
 je .bad
 mov ebx,[record]
 lea eax,[ebx+VS_ROOT]
 invoke GetFullPathNameW,[rootPath],1024,eax,0
 test eax,eax
 jz .bad
 cmp eax,1024
 jae .bad
 lea esi,[ebx+VS_ROOT]
 invoke lstrlenW,esi
.trim:
 cmp eax,3
 jbe .root
 cmp word [esi+eax*2-2],92
 jne .root
 dec eax
 mov word [esi+eax*2],0
 jmp .trim
.root:
 xor eax,eax
 cmp [installed],0
 setz al
 stdcall VsDirectoryChain,esi,eax
 test eax,eax
 jz .bad
 lea eax,[ebx+VS_ID]
 invoke lstrcpyA,eax,[id]
 lea eax,[ebx+VS_BASE]
 invoke lstrcpyA,eax,[id]
 invoke MultiByteToWideChar,65001,8,[id],-1,addr wideId,128
 test eax,eax
 jz .bad
 lea edi,[ebx+VS_PROFILE]
 invoke lstrcpyW,edi,esi
 invoke lstrcatW,edi,vsVersions
 invoke lstrcatW,edi,addr wideId
 xor eax,eax
 cmp [installed],0
 setz al
 stdcall VsDirectoryChain,edi,eax
 test eax,eax
 jz .bad
 cmp [installed],0
 je .ini
 stdcall VsMetadata,ebx
 test eax,eax
 jz .bad
.ini:
 invoke lstrcpyW,addr path,edi
 invoke lstrcatW,addr path,vsIniSuffix
 invoke GetFileAttributesW,addr path
 cmp eax,-1
 je .resolveData
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 invoke GetPrivateProfileStringW,vsSection,vsKeyName,vsEmpty,addr value,4112,addr path
 test eax,eax
 jz .java
 cmp eax,160
 jae .java
 lea eax,[ebx+VS_NAME]
 stdcall VsDecode,addr value,eax,40
 lea eax,[ebx+VS_NAME]
 stdcall VsValidText,eax,36
 test eax,eax
 jnz .java
 mov word [ebx+VS_NAME],0
.java:
 invoke GetPrivateProfileStringW,vsSection,vsKeyJava,vsEmpty,addr value,4112,addr path
 test eax,eax
 jz .memory
 cmp eax,4096
 jae .memory
 lea eax,[ebx+VS_JAVA]
 stdcall VsDecode,addr value,eax,1024
 test eax,eax
 jz .memory
 lea eax,[ebx+VS_JAVA]
 invoke GetFullPathNameW,eax,4112,addr value,0
 test eax,eax
 jz .clearJava
 cmp eax,1024
 jae .clearJava
 invoke GetFileAttributesW,addr value
 cmp eax,-1
 je .clearJava
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jnz .clearJava
 lea eax,[ebx+VS_JAVA]
 invoke lstrcpyW,eax,addr value
 jmp .memory
.clearJava:
 mov word [ebx+VS_JAVA],0
.memory:
 invoke GetPrivateProfileStringW,vsSection,vsKeyMemory,vsZero,addr value,32,addr path
 stdcall MemoryParseW,addr value
 test eax,eax
 jz .isolation
 stdcall MemoryValid,eax
 test eax,eax
 jz .isolation
 stdcall MemoryParseW,addr value
 mov [ebx+VS_MEMORY],eax
.isolation:
 invoke GetPrivateProfileIntW,vsSection,vsKeyIsolate,0,addr path
 cmp eax,1
 jne .resolveData
 mov [ebx+VS_ISOLATE],eax
.resolveData:
 lea edi,[ebx+VS_DATA]
 lea eax,[ebx+VS_ROOT]
 cmp dword [ebx+VS_ISOLATE],0
 je .copyData
 lea eax,[ebx+VS_PROFILE]
.copyData:
 invoke lstrcpyW,edi,eax
 cmp dword [ebx+VS_ISOLATE],0
 je .valid
 invoke lstrcatW,edi,vsGameSuffix
 stdcall VsDirectoryChain,edi,1
 test eax,eax
 jz .bad
.valid:
 mov dword [ebx+VS_VALID],1
 mov eax,1
 ret
.bad:
 mov ebx,[record]
 mov dword [ebx+VS_VALID],0
 xor eax,eax
 ret
endp
proc VsLoad,rootPath,id
 stdcall MemoryDetect
 stdcall VsRead,[rootPath],[id],vsNameW,1
 ret
endp
proc VsResolveData,rootPath,id,destination,capacity
 local record[VS_SIZE]:BYTE
 stdcall MemoryDetect
 stdcall VsRead,[rootPath],[id],addr record,0
 test eax,eax
 jz .bad
 invoke lstrlenW,addr record+VS_DATA
 cmp eax,[capacity]
 jae .bad
 invoke lstrcpyW,[destination],addr record+VS_DATA
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc VsSave uses ebx esi edi
 locals
  path rw 2048
  temp rw 2048
  canonical rw 1024
  value rw 4112
  fresh rb VS_SIZE
  owned dd ?
  h dd ?
  written dd ?
 endl
 mov [owned],0
 cmp [vsContextValid],1
 jne .bad
 stdcall VsValidText,vsNameW,36
 test eax,eax
 jz .bad
 stdcall VsValidText,vsJavaW,1023
 test eax,eax
 jz .bad
 stdcall MemoryDetect
 cmp [vsMemory],0
 je .memoryValid
 stdcall MemoryValid,[vsMemory]
 test eax,eax
 jz .bad
.memoryValid:
 cmp [vsIsolate],1
 ja .bad
 stdcall VsRead,vsRootW,vsIdA,addr fresh,1
 test eax,eax
 jz .bad
 mov eax,vsRootW
 cmp [vsIsolate],0
 je .dataCheck
 mov eax,vsProfileDirW
.dataCheck:
 ; The full prospective directory needs the larger private path buffer.
 invoke lstrcpyW,addr path,eax
 cmp [vsIsolate],0
 je .javaCheck
 invoke lstrcatW,addr path,vsGameSuffix
 stdcall VsDirectoryChain,addr path,1
 test eax,eax
 jz .bad
.javaCheck:
 cmp word [vsJavaW],0
 je .path
 invoke GetFullPathNameW,vsJavaW,1024,addr canonical,0
 test eax,eax
 jz .bad
 cmp eax,1024
 jae .bad
 invoke GetFileAttributesW,addr canonical
 cmp eax,-1
 je .bad
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 invoke lstrcpyW,vsJavaW,addr canonical
.path:
 invoke lstrcpyW,addr path,vsProfileDirW
 invoke lstrcatW,addr path,vsIniSuffix
 invoke lstrcpyW,addr temp,addr path
 invoke GetTickCount
 cinvoke wsprintfW,addr value,vsTempFormat,eax
 invoke lstrcatW,addr temp,addr value
 invoke GetFileAttributesW,addr path
 cmp eax,-1
 je .new
 test eax,400h or FILE_ATTRIBUTE_DIRECTORY
 jnz .bad
 invoke CopyFileW,addr path,addr temp,1
 test eax,eax
 jz .bad
 jmp .owned
.new:
 invoke CreateFileW,addr temp,GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [h],eax
 invoke WriteFile,eax,vsBom,2,addr written,0
 mov ebx,eax
 invoke CloseHandle,[h]
 test ebx,ebx
 jz .deleteNew
 cmp [written],2
 jne .deleteNew
.owned:
 mov [owned],1
 stdcall VsEncode,vsNameW,addr value
 invoke WritePrivateProfileStringW,vsSection,vsKeyName,addr value,addr temp
 test eax,eax
 jz .bad
 stdcall VsEncode,vsJavaW,addr value
 invoke WritePrivateProfileStringW,vsSection,vsKeyJava,addr value,addr temp
 test eax,eax
 jz .bad
 cinvoke wsprintfW,addr value,vsDecimal,[vsMemory]
 invoke WritePrivateProfileStringW,vsSection,vsKeyMemory,addr value,addr temp
 test eax,eax
 jz .bad
 cinvoke wsprintfW,addr value,vsDecimal,[vsIsolate]
 invoke WritePrivateProfileStringW,vsSection,vsKeyIsolate,addr value,addr temp
 test eax,eax
 jz .bad
 invoke WritePrivateProfileStringW,0,0,0,addr temp
 invoke MoveFileExW,addr temp,addr path,1 or 8
 test eax,eax
 jz .bad
 mov [owned],0
 ; No data migration or directory creation occurs while changing a setting.
 mov eax,vsRootW
 cmp [vsIsolate],0
 je .resolveData
 mov eax,vsProfileDirW
.resolveData:
 invoke lstrcpyW,vsDataDirW,eax
 cmp [vsIsolate],0
 je .good
 invoke lstrcatW,vsDataDirW,vsGameSuffix
.good:
 mov eax,1
 ret
.deleteNew:
 mov [owned],1
.bad:
 cmp [owned],0
 je .return
 invoke DeleteFileW,addr temp
.return:
 xor eax,eax
 ret
endp

; Engine worker helper. Never mutates the version page's vs* draft buffers.
proc VsLaunchSettings,rootPath,id
 local record[VS_SIZE]:BYTE
 stdcall MemoryDetect
 stdcall VsRead,[rootPath],[id],addr record,0
 test eax,eax
 jz .bad
 cmp word [record+VS_JAVA],0
 je .memory
 invoke lstrcpyW,javaPath,addr record+VS_JAVA
.memory:
 cmp dword [record+VS_MEMORY],0
 je .resolveData
 mov eax,dword [record+VS_MEMORY]
 mov [memoryMb],eax
.resolveData:
 invoke lstrcpyW,eRuntimeDirW,addr record+VS_DATA
 mov eax,dword [record+VS_ISOLATE]
 mov [eRuntimeIsolated],eax
 stdcall EnsureDirectory,eRuntimeDirW
 test eax,eax
 jz .bad
 stdcall VsDirectoryChain,eRuntimeDirW,0
 ret
.bad:
 xor eax,eax
 ret
endp
