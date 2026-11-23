; Local download receipts support offline, same-platform re-export.
; Receipt names are content hashes. The relative game path must also match.
section '.psrcd' data readable writeable
psFolder du '\.offset-pack-sources',0
psMrSuffix du '.mr.json',0
psCfSuffix du '.cf.json',0
psCfReceipt db '{"path":"%s","projectID":%u,"fileID":%u}',0
psCfReference db '{"projectID":%u,"fileID":%u,"required":true}',0
psBbsFile db '{"type":"addon","path":"%s","hash":"%s","force":true}',0
psMrReceipt db '{"path":"%s","hashes":{"sha1":"%s","sha512":"%s"},"fileSize":%u,"downloads":[',0
psEnd db ']}',0
psSha512 db 'sha512',0
psBytes db 'fileSize',0
psDirectory rw 2048
psPath rw 2048
psHash rb 41
psRecord rb 65536

section '.psrcc' code readable executable
 nop
proc PsDirectory,root
 invoke lstrlenW,[root]
 cmp eax,1950
 ja .bad
 invoke lstrcpyW,psDirectory,[root]
 invoke lstrcatW,psDirectory,psFolder
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PsPath uses esi,hash,kind
 local nameW[41]:WORD
 stdcall ModIsSha1,[hash]
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,[hash],-1,addr nameW,41
 invoke lstrcpyW,psPath,psDirectory
 invoke lstrcatW,psPath,pkSlash
 invoke lstrcatW,psPath,addr nameW
 mov esi,psCfSuffix
 cmp [kind],1
 jne .suffix
 mov esi,psMrSuffix
.suffix:
 invoke lstrcatW,psPath,esi
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PsSave,root,hash,kind
 stdcall PsDirectory,[root]
 test eax,eax
 jz .done
 invoke CreateDirectoryW,psDirectory,0
 stdcall VsDirectoryChain,psDirectory,0
 test eax,eax
 jz .done
 stdcall PsPath,[hash],[kind]
 test eax,eax
 jz .done
 stdcall PkWriteNew,psPath,psRecord
.done:
 ret
endp

proc PkRememberSource uses ebx,record
 mov ebx,[record]
 cmp [pkFormat],1
 je .mr
 cinvoke wsprintfA,psRecord,psCfReceipt,ebx,dword [ebx+PK_FPROJECT],dword [ebx+PK_FID]
 jmp .save
.mr:
 stdcall JsonRaw,[pkContext],dword [ebx+PK_FITEM],psRecord,65536
 test eax,eax
 jz .done
 cmp eax,65536
 jae .done
.save:
 lea eax,[ebx+PK_FHASH]
 stdcall PsSave,pkStage,eax,[pkFormat]
.done:
 ret
endp

; Remember a verified file installed from the Mods page as well.
proc PsRememberMod uses ebx esi edi,record,target
 locals
  root rw 2048
  relative rb 512
  digest rb 129
  quoted rb 4098
  file dd ?
  bytes dd ?
  high dd ?
 endl
 stdcall ModsDataRoot,addr root,2048
 test eax,eax
 jz .done
 mov ebx,[record]
 invoke lstrcpyA,addr relative,pkCfModsPrefix
 lea eax,[ebx+MF_NAME]
 stdcall ModAppend,addr relative,512,eax
 test eax,eax
 jz .done
 cmp [modProvider],0
 je .modrinth
 mov eax,[modSelectedProject]
 cmp eax,[modProjectCount]
 jae .done
 imul eax,MP_SIZE
 add eax,modProjects
 stdcall ModNumber,eax
 test eax,eax
 jz .done
 mov esi,eax
 stdcall ModNumber,ebx
 test eax,eax
 jz .done
 cinvoke wsprintfA,psRecord,psCfReceipt,addr relative,esi,eax
 mov esi,2
 jmp .save
.modrinth:
 invoke CreateFileW,[target],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,00200000h,0
 cmp eax,-1
 je .done
 mov [file],eax
 stdcall VsRegularFile,[target]
 test eax,eax
 jz .close
 invoke GetFileSize,[file],addr high
 cmp [high],0
 jne .close
 cmp eax,PK_ENTRY_MAX
 ja .close
 mov [bytes],eax
 stdcall DigestFile,[target],addr digest,800Eh,64
 test eax,eax
 jz .close
 lea esi,[ebx+MF_SHA1]
 cinvoke wsprintfA,psRecord,psMrReceipt,addr relative,esi,addr digest,[bytes]
 lea eax,[ebx+MF_URL]
 stdcall PeQuote,eax,addr quoted,4098
 test eax,eax
 jz .close
 stdcall ModAppend,psRecord,65536,addr quoted
 stdcall ModAppend,psRecord,65536,psEnd
 invoke CloseHandle,[file]
 mov esi,1
.save:
 lea eax,[ebx+MF_SHA1]
 stdcall PsSave,addr root,eax,esi
 ret
.close:
 invoke CloseHandle,[file]
.done:
 ret
endp

proc PePrepareSources
 mov [peSourceAvailable],0
 cmp [peFormat],2
 ja .done
 stdcall PsDirectory,peDataDirW
 test eax,eax
 jz .done
 stdcall VsDirectoryChain,psDirectory,0
 mov [peSourceAvailable],eax
.done:
 ret
endp

; Append at a cursor so writing a large manifest takes linear time.
proc PeListAppend uses ebx esi edi,entryText
 invoke lstrlenA,[entryText]
 mov ecx,eax
 mov ebx,[peManifestBytes]
 lea edx,[ebx+ecx+4] ; comma, closing brackets and terminator
 cmp edx,PK_MANIFEST_MAX
 ja .bad
 lea edi,[peManifestA+ebx]
 cmp [peListCount],0
 je .copy
 mov al,','
 stosb
.copy:
 mov esi,[entryText]
 rep movsb
 mov byte [edi],0
 sub edi,peManifestA
 mov [peManifestBytes],edi
 inc [peListCount]
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Return 1 for a reference, 0 for an embedded file, -1 for manifest overflow.
proc PeReference uses ebx esi edi,record
 local ctx:DWORD,pinned:DWORD,result:DWORD,got:DWORD
 mov [ctx],0
 mov [result],0
 stdcall PsPath,psHash,[peFormat]
 test eax,eax
 jz .done
 invoke CreateFileW,psPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,00200000h,0
 cmp eax,-1
 je .done
 mov [pinned],eax
 stdcall VsRegularFile,psPath
 test eax,eax
 jz .close
 invoke GetFileSize,[pinned],addr got
 cmp [got],0
 jne .close
 cmp eax,65535
 ja .close
 mov ebx,eax
 invoke ReadFile,[pinned],psRecord,ebx,addr got,0
 test eax,eax
 jz .close
 cmp ebx,[got]
 jne .close
 mov byte [psRecord+ebx],0
 invoke lstrlenA,psRecord
 cmp eax,ebx
 jne .close
 stdcall JsonParse,psRecord
 test eax,eax
 jz .close
 mov [ctx],eax
 stdcall JsonRoot,eax
 mov esi,eax
 stdcall JsonGet,[ctx],esi,pkKeyPath
 stdcall PkString,[ctx],eax,pkValue,512
 test eax,eax
 jz .close
 ; Preserve filename spelling, including on case-insensitive Windows volumes.
 mov edi,[record]
 invoke lstrlenA,edi
 inc eax
 mov ecx,eax
 push esi
 mov esi,pkValue
 repe cmpsb
 pop esi
 jne .close
 cmp [peFormat],2
 je .curse
 stdcall JsonGet,[ctx],esi,psBytes
 stdcall JsonInt,[ctx],eax
 mov ebx,[record]
 cmp eax,[ebx+PE_BYTES]
 jne .close
 stdcall JsonGet,[ctx],esi,pkKeyHash
 mov edi,eax
 stdcall JsonGet,[ctx],edi,pkKeySha
 stdcall PkString,[ctx],eax,pkValue,41
 test eax,eax
 jz .close
 invoke lstrcmpiA,pkValue,psHash
 test eax,eax
 jnz .close
 stdcall JsonGet,[ctx],edi,psSha512
 stdcall PkString,[ctx],eax,pkValue,129
 test eax,eax
 jz .close
 lea eax,[ebx+PE_PATH]
 stdcall DigestFile,eax,pkValue2,800Eh,64
 test eax,eax
 jz .close
 invoke lstrcmpiA,pkValue,pkValue2
 test eax,eax
 jnz .close
 jmp .append
.curse:
 stdcall JsonGet,[ctx],esi,pkKeyProject
 stdcall JsonInt,[ctx],eax
 test eax,eax
 jle .close
 mov ebx,eax
 stdcall JsonGet,[ctx],esi,pkKeyFile
 stdcall JsonInt,[ctx],eax
 test eax,eax
 jle .close
 cinvoke wsprintfA,psRecord,psCfReference,ebx,eax
.append:
 stdcall PeListAppend,psRecord
 test eax,eax
 jnz .accepted
 or eax,-1
.accepted:
 mov [result],eax
.close:
 invoke CloseHandle,[pinned]
 stdcall FreeMem,[ctx]
.done:
 mov eax,[result]
 ret
endp

proc PeWritePayload uses ebx esi edi,record
 local lock:DWORD,result:DWORD,info:BY_HANDLE_FILE_INFORMATION
 mov ebx,[record]
 lea edi,[ebx+PE_PATH]
 cmp [peFormat],4
 je .pin
 cmp [peSourceAvailable],0
 jne .pin
 stdcall ExZipAddFile,peArchiveNameA,edi
 ret
.pin:
 mov [result],0
 invoke CreateFileW,edi,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,00200000h,0
 cmp eax,-1
 je .bad
 mov [lock],eax
 invoke GetFileInformationByHandle,eax,addr info
 test eax,eax
 jz .done
 test [info.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY+400h
 jnz .done
 cmp [info.nFileSizeHigh],0
 jne .done
 mov eax,[info.nFileSizeLow]
 cmp eax,EZ_FILE_MAX
 ja .done
 mov [ebx+PE_BYTES],eax
 stdcall Sha1File,edi,psHash
 test eax,eax
 jz .done
 cmp [peFormat],4
 jne .reference
 cinvoke wsprintfA,psRecord,psBbsFile,ebx,psHash
 stdcall PeListAppend,psRecord
 test eax,eax
 jz .done
 jmp .embed
.reference:
 stdcall PeReference,ebx
 cmp eax,-1
 je .done
 test eax,eax
 jnz .result
.embed:
 stdcall ExZipAddFile,peArchiveNameA,edi
.result:
 mov [result],eax
.done:
 invoke CloseHandle,[lock]
 mov eax,[result]
 ret
.bad:
 xor eax,eax
 ret
endp
