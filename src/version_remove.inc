; Keep removed profiles outside versions/ so normal scans no longer list them.
; A same-volume rename preserves the complete directory, including isolated data.
section '.vrtrash' code readable executable
proc VersionTrash uses ebx esi edi,rootW,idW
 locals
  source rw 1536
  target rw 1536
 endl
 mov esi,[idW]
 test esi,esi
 jz .bad
 cmp word [esi],'.'                    ; EngineScanVersions skips hidden entries
 je .bad
 invoke lstrlenW,esi
 stdcall PkComponentSafe,esi,eax
 test eax,eax
 jz .bad
.leaf:
 lodsw
 cmp ax,'/'                            ; PkComponentSafe expects a single component
 je .bad
 test ax,ax
 jnz .leaf
 mov esi,[rootW]
 test esi,esi
 jz .bad
 cmp word [esi],0
 je .bad
 invoke GetFullPathNameW,esi,1024,addr source,0
 test eax,eax
 jz .bad
 cmp eax,1024
 jae .bad
 lea edi,[source+eax*2-2]
 cmp word [edi],92
 jne @f
 mov word [edi],0                      ; both suffixes begin with a backslash
@@:
 invoke lstrcpyW,addr target,addr source
 invoke lstrcatW,addr source,eVersions
 invoke lstrcatW,addr source,[idW]
 stdcall VsDirectoryChain,addr source,0
 test eax,eax
 jz .bad
 invoke lstrcatW,addr target,.trash
 invoke CreateDirectoryW,addr target,0
 ; Whether new or already present, the recovery folder must be a real directory.
 stdcall VsDirectoryChain,addr target,0
 test eax,eax
 jz .bad
 invoke lstrcatW,addr target,eSlash
 invoke lstrcatW,addr target,[idW]
 invoke lstrlenW,addr target
 lea edi,[target+eax*2]                ; original name, then name (2), name (3), ...
 mov ebx,1
.move:
 invoke MoveFileExW,addr source,addr target,0
 test eax,eax
 jnz .done
 invoke GetFileAttributesW,addr target
 cmp eax,-1
 je .bad
 inc ebx
 jz .bad
 cinvoke wsprintfW,edi,.number,ebx
 jmp .move
.bad:
 xor eax,eax
.done:
 ret
.trash du '\.offset-version-trash',0
.number du ' (%u)',0
endp
