; HMCL/PCL local MCBBS archives. Embedded addons are verified before install.
section '.bbsd' data readable writeable
pkKeyAddons db 'addons',0
pkKeyType db 'type',0
pkAddon db 'addon',0
pkCurse db 'curse',0
pkAddonHash db 'hash',0
pkBbsGame db 'game',0
pkBbsFabric db 'fabric',0
pkBbsQuilt db 'quilt',0
pkBbsIds dd pkBbsGame,pkBbsFabric,pkKeyLoaderForge,pkKeyLoaderNeo,pkBbsQuilt
pkAddonHashes rd PK_ZMAX

section '.bbsc' code readable executable
 nop
proc PkReadMcbbs uses ebx esi edi,root
 local list:DWORD,count:DWORD,item:DWORD
 stdcall JsonGet,[pkContext],[root],pkKeyManifestType
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .bad
 invoke lstrcmpA,pkValue,pkModpack
 test eax,eax
 jnz .bad
 stdcall JsonGet,[pkContext],[root],pkKeyManifestVersion
 stdcall JsonInt,[pkContext],eax
 dec eax
 cmp eax,1
 ja .bad
 stdcall JsonGet,[pkContext],[root],peKeyLibraries
 stdcall JsonCount,[pkContext],eax
 test eax,eax
 jnz .bad
 invoke lstrcpyA,pkOverrides,pkDefaultOverrides
 mov byte [pkBase],0
 mov byte [pkLoader],0
 mov [pkLoaderKind],0
 mov [pkFileCount],0
 mov edi,pkAddonHashes
 mov ecx,[pkZipCount]
 xor eax,eax
 rep stosd
 stdcall JsonGet,[pkContext],[root],pkKeyAddons
 mov [list],eax
 stdcall JsonCount,[pkContext],eax
 cmp eax,2
 ja .bad
 mov [count],eax
 xor esi,esi
.component:
 cmp esi,[count]
 jae .files
 stdcall JsonAt,[pkContext],[list],esi
 mov [item],eax
 stdcall JsonGet,[pkContext],eax,pkKeyId
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .bad
 xor ebx,ebx
.id:
 invoke lstrcmpA,pkValue,dword [pkBbsIds+ebx*4]
 test eax,eax
 jz .version
 inc ebx
 cmp ebx,5
 jb .id
 jmp .bad
.version:
 mov edi,pkBase
 test ebx,ebx
 jz .read
 mov edi,pkLoader
 mov [pkLoaderKind],ebx
.read:
 cmp byte [edi],0
 jne .bad
 stdcall JsonGet,[pkContext],[item],pkKeyVersion
 stdcall PkString,[pkContext],eax,edi,128
 test eax,eax
 jz .bad
 stdcall LoaderSafeId,edi
 test eax,eax
 jz .bad
 inc esi
 jmp .component
.files:
 cmp byte [pkBase],0
 je .bad
 stdcall JsonGet,[pkContext],[root],pkKeyFiles
 test eax,eax
 jz .good
 mov [list],eax
 stdcall PIsArray,[pkContext],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[pkContext],[list]
 cmp eax,PK_ZMAX+PK_FMAX
 ja .bad
 mov [count],eax
 stdcall JsonAt,[pkContext],[list],0
 mov [item],eax
.file:
 cmp [count],0
 je .good
 stdcall JsonGet,[pkContext],[item],pkKeyType
 stdcall PkString,[pkContext],eax,pkValue,4096
 test eax,eax
 jz .bad
 invoke lstrcmpA,pkValue,pkCurse
 test eax,eax
 jnz .embedded
 cmp [pkFileCount],PK_FMAX
 jae .bad
 stdcall PkAddCfFile,[item]
 test eax,eax
 jz .bad
 jmp .next
.embedded:
 invoke lstrcmpA,pkValue,pkAddon
 test eax,eax
 jnz .bad
 stdcall JsonGet,[pkContext],[item],pkKeyPath
 stdcall PkString,[pkContext],eax,pkValue2,512
 test eax,eax
 jz .bad
 stdcall PkPayloadSafe,pkValue2
 test eax,eax
 jz .bad
 invoke lstrcpyA,pkValue,pkArchivePrefix
 stdcall ModAppend,pkValue,4096,peOverridePrefix
 stdcall ModAppend,pkValue,4096,pkValue2
 stdcall PkFindEntry,pkValue
 cmp eax,-1
 je .bad
 mov ebx,eax
 ; Require the same spelling used by the indexed override copier.
 invoke lstrlenW,pkWide
 lea ecx,[eax+1]
 imul esi,ebx,PK_ZSIZE
 add esi,pkZipRecords
 mov edi,pkWide
 repe cmpsw
 jne .bad
 cmp dword [pkAddonHashes+ebx*4],0
 jne .bad
 stdcall JsonGet,[pkContext],[item],pkAddonHash
 mov [pkAddonHashes+ebx*4],eax
 stdcall PkString,[pkContext],eax,pkValue,41
 test eax,eax
 jz .bad
 stdcall ModIsSha1,pkValue
 test eax,eax
 jz .bad
.next:
 stdcall JsonNext,[pkContext],[item]
 mov [item],eax
 dec [count]
 jmp .file
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PkVerifyAddons uses ebx esi
 xor ebx,ebx
.loop:
 cmp ebx,[pkZipCount]
 jae .good
 cmp [engineCancel],0
 jne .bad
 mov eax,[pkAddonHashes+ebx*4]
 test eax,eax
 jz .next
 stdcall PkString,[pkContext],eax,pkValue,41
 imul esi,ebx,PK_ZSIZE
 add esi,pkZipRecords
 invoke lstrcpyW,pkPath,pkExtract
 invoke lstrcatW,pkPath,pkSlash
 invoke lstrcatW,pkPath,esi
 stdcall VerifySha1,pkPath,pkValue
 test eax,eax
 jz .bad
.next:
 inc ebx
 jmp .loop
.good:
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
