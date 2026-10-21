; OFFSET: original modern profile normalization. One installer worker at a time.
; Public: ProfileNormalize(childIdA, baseIdA) -> 1/0.
; ProfileMerge(parentCtx, childCtx, childIdA) -> owned UTF-8 JSON / 0.
P_CAP = 8388608
section '.prdata' data readable writeable
pOutput dd 0
pLength dd 0
pFailed dd 0
pParent dd 0
pChild dd 0
pParentRoot dd 0
pChildRoot dd 0
pVersionKey db 'id',0
pInheritKey db 'inheritsFrom',0
pArgsKey db 'arguments',0
pLibsKey db 'libraries',0
pJvmKey db 'jvm',0
pGameKey db 'game',0
pMainKey db 'mainClass',0
pAssetsKey db 'assetIndex',0
pDownloadsKey db 'downloads',0
pArtifactKey db 'artifact',0
pClassifiersKey db 'classifiers',0
pNameKey db 'name',0
pUrlKey db 'url',0
pPathKey db 'path',0
pShaKey db 'sha1',0
pClientKey db 'client',0
pNormalizedKey db 'offsetNormalized',0
pBaseKey db 'offsetBase',0
pDefaultRepo db 'https://libraries.minecraft.net/',0
pHttps db 'https://',0
pOpen db '{"id":',0
pObjectOpen db '{',0
pNormalizedOpen db ',"offsetNormalized":true,"offsetBase":',0
pComma db ',',0
pColon db ':',0
pQuote db '"',0
pClose db '}',0
pArgsOpen db ',"arguments":{"jvm":[',0
pGameOpen db '],"game":[',0
pArgsClose db ']}',0
pLibsOpen db ',"libraries":[',0
pArrayClose db ']',0
pDownloadOpen db '"downloads":{"artifact":{"path":',0
pUrlOpen db ',"url":',0
pShaOpen db ',"sha1":',0
pClassifiersOpen db ',"classifiers":',0
pSlashA db '/',0
pColonA db ':',0
pDashA db '-',0
pDotA db '.',0
pAtA db '@',0
pJarA db 'jar',0
pShaSuffix db '.sha1',0
pHex db '0123456789abcdef',0
pVersionsW du '\versions\',0
pLibrariesW du '\libraries\',0
pSlashW du '\',0
pJsonW du '.json',0
pJarW du '.jar',0
pOriginalW du '\offset-original.json',0
pTempW du '.offset-profile.tmp',0
pErr db 'Loader profile normalization failed. Existing profile and backups were preserved.',0
pKey rb 256
pValue rb 4096
pName rb 512
pRepo rb 4096
pRel rb 4096
pUrl rb 4096
pHash rb 64
pGroup rb 512
pArtifact rb 512
pVersion rb 512
pClassifier rb 512
pExtension rb 64
pChildPath rw 2048
pBasePath rw 2048
pBackupPath rw 2048
pTempPath rw 2048
pChildJar rw 2048
pBaseJar rw 2048
pFilePath rw 2048
pWide rw 8192

section '.prcode' code readable executable
 nop

proc PAddN uses esi edi,source,count
 cmp [pFailed],0
 jne .bad
 mov eax,[pLength]
 add eax,[count]
 jc .fail
 cmp eax,P_CAP
 jae .fail
 mov edi,[pOutput]
 add edi,[pLength]
 mov esi,[source]
 mov ecx,[count]
 rep movsb
 mov byte [edi],0
 mov [pLength],eax
 mov eax,1
 ret
.fail:
 mov [pFailed],1
.bad:
 xor eax,eax
 ret
endp

proc PAdd,source
 invoke lstrlenA,[source]
 stdcall PAddN,[source],eax
 ret
endp

proc PToken,ctx,token
 cmp [pFailed],0
 jne .bad
 cmp [token],0
 je .fail
 mov eax,[pOutput]
 add eax,[pLength]
 mov ecx,P_CAP
 sub ecx,[pLength]
 stdcall JsonRaw,[ctx],[token],eax,ecx
 test eax,eax
 jz .fail
 mov ecx,[pLength]
 add ecx,eax
 jc .fail
 cmp ecx,P_CAP
 jae .fail
 mov [pLength],ecx
 mov eax,1
 ret
.fail:
 mov [pFailed],1
.bad:
 xor eax,eax
 ret
endp

; Quote decoded UTF-8 without allowing a path/URL to inject JSON syntax.
proc PQuoted uses esi,source
 local quotedBytes[6]:BYTE
 stdcall PAdd,pQuote
 mov esi,[source]
.next:
 lodsb
 test al,al
 jz .close
 cmp al,'"'
 je .escape
 cmp al,92
 je .escape
 cmp al,32
 jb .control
 mov byte [quotedBytes],al
 stdcall PAddN,addr quotedBytes,1
 jmp .next
.escape:
 mov byte [quotedBytes],92
 mov byte [quotedBytes+1],al
 stdcall PAddN,addr quotedBytes,2
 jmp .next
.control:
 mov dword [quotedBytes],'\u00'
 mov dl,al
 shr al,4
 movzx eax,al
 mov al,[pHex+eax]
 mov byte [quotedBytes+4],al
 movzx eax,dl
 and eax,15
 mov al,[pHex+eax]
 mov byte [quotedBytes+5],al
 stdcall PAddN,addr quotedBytes,6
 jmp .next
.close:
 stdcall PAdd,pQuote
 ret
endp

proc PString,ctx,token,dest,capacity
 stdcall JsonString,[ctx],[token],[dest],[capacity]
 test eax,eax
 jz .bad
 cmp eax,[capacity]
 jae .bad
 push eax
 invoke lstrlenA,[dest]
 pop ecx
 cmp eax,ecx
 jne .bad
 ret
.bad:
 xor eax,eax
 ret
endp

proc PIsArray,ctx,token
 local arrayPrefix[2]:BYTE
 stdcall JsonRaw,[ctx],[token],addr arrayPrefix,2
 test eax,eax
 jz .bad
 cmp byte [arrayPrefix],'['
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PSafeId uses esi,source
 mov esi,[source]
 xor ecx,ecx
.next:
 mov al,[esi+ecx]
 test al,al
 jz .finishedValue
 cmp ecx,120
 jae .bad
 cmp al,'A'
 jb .number
 cmp al,'Z'
 jbe .advance
 cmp al,'a'
 jb .number
 cmp al,'z'
 jbe .advance
.number:
 cmp al,'0'
 jb .punct
 cmp al,'9'
 jbe .advance
.punct:
 cmp al,'-'
 je .advance
 cmp al,'_'
 je .advance
 cmp al,'.'
 jne .bad
 test ecx,ecx
 jz .bad
 cmp byte [esi+ecx+1],'.'
 je .bad
.advance:
 inc ecx
 jmp .next
.finishedValue:
 test ecx,ecx
 jz .bad
 cmp byte [esi+ecx-1],'.'
 je .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; A metadata artifact path is relative, ASCII and cannot escape libraries/.
proc PSafePath uses esi,source
 mov esi,[source]
 xor ecx,ecx
.next:
 mov al,[esi+ecx]
 test al,al
 jz .finishedValue
 cmp ecx,1000
 jae .bad
 cmp al,33
 jb .bad
 cmp al,127
 jae .bad
 cmp al,92
 je .bad
 cmp al,':'
 je .bad
 cmp al,'"'
 je .bad
 cmp al,'<'
 je .bad
 cmp al,'>'
 je .bad
 cmp al,'|'
 je .bad
 cmp al,'?'
 je .bad
 cmp al,'*'
 je .bad
 cmp al,'/'
 jne .dot
 test ecx,ecx
 jz .bad
 cmp byte [esi+ecx+1],'/'
 je .bad
 cmp byte [esi+ecx+1],0
 je .bad
.dot:
 cmp al,'.'
 jne .advance
 cmp byte [esi+ecx+1],'.'
 je .bad
.advance:
 inc ecx
 jmp .next
.finishedValue:
 test ecx,ecx
 jz .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PIsHash uses esi,source
 mov esi,[source]
 mov ecx,40
.next:
 lodsb
 cmp al,'0'
 jb .bad
 cmp al,'9'
 jbe .valid
 or al,32
 cmp al,'a'
 jb .bad
 cmp al,'f'
 ja .bad
.valid:
 loop .next
 cmp byte [esi],0
 jne .bad
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Decode a bounded Maven coordinate. Empty components and unsafe characters fail.
proc PMavenParse uses ebx esi edi,nameA
 locals
  field dd ?
  length dd ?
  last dd ?
 endl
 mov byte [pGroup],0
 mov byte [pArtifact],0
 mov byte [pVersion],0
 mov byte [pClassifier],0
 invoke lstrcpyA,pExtension,pJarA
 mov [field],0
 mov [length],0
 mov [last],0
 mov esi,[nameA]
 mov edi,pGroup
.next:
 lodsb
 test al,al
 jz .finishedValue
 cmp al,':'
 je .separator
 cmp al,'@'
 je .extension
 cmp al,'0'
 jb .punct
 cmp al,'9'
 jbe .character
 cmp al,'A'
 jb .punct
 cmp al,'Z'
 jbe .character
 cmp al,'a'
 jb .punct
 cmp al,'z'
 jbe .character
.punct:
 cmp al,'-'
 je .character
 cmp al,'_'
 je .character
 cmp al,'+'
 je .character
 cmp al,'.'
 jne .bad
 cmp [length],0
 je .bad
 cmp [last],'.'
 je .bad
.character:
 movzx edx,al
 mov [last],edx
 mov ecx,[length]
 cmp ecx,500
 jae .bad
 cmp [field],4
 jne .store
 cmp ecx,31
 jae .bad
.store:
 stosb
 inc [length]
 jmp .next
.separator:
 cmp [length],0
 je .bad
 cmp [last],'.'
 je .bad
 cmp [field],2
 jae .maybeClassifier
 jmp .advanceField
.maybeClassifier:
 cmp [field],2
 jne .bad
.advanceField:
 mov byte [edi],0
 inc [field]
 mov eax,[field]
 mov edi,pArtifact
 cmp eax,1
 je .reset
 mov edi,pVersion
 cmp eax,2
 je .reset
 mov edi,pClassifier
.reset:
 mov [length],0
 mov [last],0
 jmp .next
.extension:
 cmp [field],2
 jb .bad
 cmp [field],3
 ja .bad
 cmp [length],0
 je .bad
 mov byte [edi],0
 mov edi,pExtension
 mov [field],4
 jmp .reset
.finishedValue:
 cmp [field],2
 jb .bad
 cmp [length],0
 je .bad
 cmp [last],'.'
 je .bad
 mov byte [edi],0
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

; Return group:artifact:classifier@extension, ignoring version for overrides.
proc PLibraryKey uses ebx,ctx,token,dest
 stdcall JsonGet,[ctx],[token],pNameKey
 stdcall PString,[ctx],eax,pName,512
 test eax,eax
 jz .bad
 stdcall PMavenParse,pName
 test eax,eax
 jz .bad
 invoke lstrcpyA,[dest],pGroup
 invoke lstrcatA,[dest],pColonA
 invoke lstrcatA,[dest],pArtifact
 invoke lstrcatA,[dest],pColonA
 invoke lstrcatA,[dest],pClassifier
 invoke lstrcatA,[dest],pAtA
 invoke lstrcatA,[dest],pExtension
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc PMavenPath uses esi edi
 mov esi,pGroup
 mov edi,pRel
.group:
 lodsb
 test al,al
 jz .rest
 cmp al,'.'
 jne .char
 mov al,'/'
.char:
 stosb
 jmp .group
.rest:
 mov byte [edi],0
 invoke lstrcatA,pRel,pSlashA
 invoke lstrcatA,pRel,pArtifact
 invoke lstrcatA,pRel,pSlashA
 invoke lstrcatA,pRel,pVersion
 invoke lstrcatA,pRel,pSlashA
 invoke lstrcatA,pRel,pArtifact
 invoke lstrcatA,pRel,pDashA
 invoke lstrcatA,pRel,pVersion
 cmp byte [pClassifier],0
 je .extension
 invoke lstrcatA,pRel,pDashA
 invoke lstrcatA,pRel,pClassifier
.extension:
 invoke lstrcatA,pRel,pDotA
 invoke lstrcatA,pRel,pExtension
 stdcall PSafePath,pRel
 ret
endp

proc PHashFile uses ebx esi edi,pathW,dest
 locals
  provider dd ?
  hash dd ?
  file dd ?
  got dd ?
  size dd ?
  success dd ?
  digest rb 20
  buffer rb 16384
 endl
 mov [provider],0
 mov [hash],0
 mov [file],-1
 mov [success],0
 invoke CryptAcquireContextW,addr provider,0,0,1,0F0000000h
 test eax,eax
 jz .done
 invoke CryptCreateHash,[provider],08004h,0,0,addr hash
 test eax,eax
 jz .done
 invoke CreateFileW,[pathW],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .done
 mov [file],eax
.read:
 cmp [engineCancel],0
 jne .done
 invoke ReadFile,[file],addr buffer,16384,addr got,0
 test eax,eax
 jz .done
 cmp [got],0
 je .finish
 invoke CryptHashData,[hash],addr buffer,[got],0
 test eax,eax
 jz .done
 jmp .read
.finish:
 mov [size],20
 invoke CryptGetHashParam,[hash],2,addr digest,addr size,0
 test eax,eax
 jz .done
 lea esi,[digest]
 mov edi,[dest]
 mov ecx,20
.hex:
 lodsb
 mov dl,al
 shr al,4
 movzx eax,al
 mov al,[pHex+eax]
 stosb
 movzx eax,dl
 and eax,15
 mov al,[pHex+eax]
 stosb
 loop .hex
 mov byte [edi],0
 mov [success],1
.done:
 cmp [file],-1
 je .destroy
 invoke CloseHandle,[file]
.destroy:
 cmp [hash],0
 je .release
 invoke CryptDestroyHash,[hash]
.release:
 cmp [provider],0
 je .return
 invoke CryptReleaseContext,[provider],0
.return:
 mov eax,[success]
 ret
endp

; Prepare and download one child library before serializing its artifact.
proc PPrepareLibrary uses ebx esi edi,ctx,token
 locals
  downloads dd ?
  artifact dd ?
  response dd ?
 endl
 mov [downloads],0
 mov [artifact],0
 cmp [engineCancel],0
 jne .bad
 stdcall JsonGet,[ctx],[token],pNameKey
 stdcall PString,[ctx],eax,pName,512
 test eax,eax
 jz .bad
 stdcall PMavenParse,pName
 test eax,eax
 jz .bad
 stdcall PMavenPath
 test eax,eax
 jz .bad
 invoke lstrcpyA,pRepo,pDefaultRepo
 stdcall JsonGet,[ctx],[token],pUrlKey
 test eax,eax
 jz .downloads
 stdcall PString,[ctx],eax,pRepo,4096
 test eax,eax
 jz .bad
.downloads:
 stdcall JsonGet,[ctx],[token],pDownloadsKey
 mov [downloads],eax
 stdcall JsonGet,[ctx],eax,pArtifactKey
 mov [artifact],eax
 stdcall JsonGet,[ctx],eax,pPathKey
 test eax,eax
 jz .url
 stdcall PString,[ctx],eax,pRel,4096
 test eax,eax
 jz .bad
 stdcall PSafePath,pRel
 test eax,eax
 jz .bad
.url:
 invoke lstrlenA,pRepo
 cmp eax,3000
 jae .bad
 cmp eax,8
 jbe .bad
 cmp dword [pRepo],'http'
 jne .bad
 cmp dword [pRepo+4],'s://'
 jne .bad
 cmp byte [pRepo+eax-1],'/'
 je .join
 invoke lstrcatA,pRepo,pSlashA
.join:
 invoke lstrlenA,pRel
 mov ebx,eax
 invoke lstrlenA,pRepo
 add eax,ebx
 cmp eax,4080
 jae .bad
 invoke lstrcpyA,pUrl,pRepo
 invoke lstrcatA,pUrl,pRel
 stdcall JsonGet,[ctx],[artifact],pUrlKey
 test eax,eax
 jz .path
 stdcall JsonString,[ctx],eax,pValue,4096
 test eax,eax
 jz .path
 cmp eax,4080
 jae .bad
 invoke lstrcpyA,pUrl,pValue
.path:
 cmp dword [pUrl],'http'
 jne .bad
 cmp dword [pUrl+4],'s://'
 jne .bad
 invoke lstrlenW,gameDir
 cmp eax,1000
 ja .bad
 invoke lstrcpyW,pFilePath,gameDir
 invoke lstrcatW,pFilePath,pLibrariesW
 invoke MultiByteToWideChar,65001,8,pRel,-1,pWide,2048
 test eax,eax
 jz .bad
 mov esi,pWide
.slashes:
 mov ax,[esi]
 test ax,ax
 jz .appendPath
 cmp ax,'/'
 jne .advance
 mov word [esi],92
.advance:
 add esi,2
 jmp .slashes
.appendPath:
 invoke lstrcatW,pFilePath,pWide
 mov byte [pHash],0
 stdcall JsonGet,[ctx],[artifact],pShaKey
 stdcall JsonString,[ctx],eax,pHash,64
 test eax,eax
 jz .missingHash
 cmp eax,40
 jne .bad
 stdcall PIsHash,pHash
 test eax,eax
 jz .bad
 jmp .download
.missingHash:
 ; Name-only Fabric/Quilt entries must obtain a remote checksum even when a
 ; same-named file already exists. Only installer artifact outputs may use a
 ; locally generated checksum when the artifact supplied none.
 cmp [artifact],0
 je .remoteHash
 ; Forge processors create local output jars whose hashes are not in the input.
 invoke GetFileAttributesW,pFilePath
 cmp eax,-1
 je .remoteHash
 test eax,FILE_ATTRIBUTE_DIRECTORY+400h
 jnz .bad
 stdcall PHashFile,pFilePath,pHash
 test eax,eax
 jz .bad
 jmp .download
.remoteHash:
 invoke lstrcpyA,pValue,pUrl
 invoke lstrcatA,pValue,pShaSuffix
 invoke MultiByteToWideChar,65001,8,pValue,-1,pWide,8192
 test eax,eax
 jz .bad
 stdcall HttpRequest,pWide,0,0,0
 test eax,eax
 jz .bad
 mov [response],eax
 cmp [httpStatus],200
 jne .freeBad
 mov esi,eax
 mov edi,pHash
 mov ecx,40
.copyHash:
 lodsb
 test al,al
 jz .freeBad
 stosb
 loop .copyHash
 mov byte [edi],0
 mov al,[esi]
 test al,al
 jz .hashEnd
 cmp al,32
 ja .freeBad
.hashEnd:
 stdcall FreeMem,[response]
 stdcall PIsHash,pHash
 test eax,eax
 jz .bad
.download:
 stdcall McDownload,pUrl,pFilePath,pHash,1
 ret
.freeBad:
 stdcall FreeMem,[response]
.bad:
 xor eax,eax
 ret
endp

proc PWriteLibrary uses ebx esi edi,ctx,token
 locals
  index dd ?
  count dd ?
  key dd ?
  value dd ?
  fields dd ?
  classifiers dd ?
 endl
 stdcall PPrepareLibrary,[ctx],[token]
 test eax,eax
 jz .bad
 stdcall PAdd,pObjectOpen
 mov [fields],0
 stdcall JsonCount,[ctx],[token]
 mov [count],eax
 mov [index],0
.fields:
 mov eax,[index]
 cmp eax,[count]
 jae .artifact
 stdcall JsonKeyAt,[ctx],[token],eax
 mov [key],eax
 stdcall PString,[ctx],eax,pKey,256
 test eax,eax
 jz .bad
 invoke lstrcmpA,pKey,pDownloadsKey
 test eax,eax
 jz .next
 cmp [fields],0
 je .key
 stdcall PAdd,pComma
.key:
 stdcall PToken,[ctx],[key]
 stdcall PAdd,pColon
 stdcall JsonValueAt,[ctx],[token],[index]
 stdcall PToken,[ctx],eax
 inc [fields]
.next:
 inc [index]
 jmp .fields
.artifact:
 cmp [fields],0
 je .artifactStart
 stdcall PAdd,pComma
.artifactStart:
 stdcall PAdd,pDownloadOpen
 stdcall PQuoted,pRel
 stdcall PAdd,pUrlOpen
 stdcall PQuoted,pUrl
 stdcall PAdd,pShaOpen
 stdcall PQuoted,pHash
 stdcall PAdd,pClose
 stdcall JsonGet,[ctx],[token],pDownloadsKey
 stdcall JsonGet,[ctx],eax,pClassifiersKey
 mov [classifiers],eax
 test eax,eax
 jz .close
 stdcall PAdd,pClassifiersOpen
 stdcall PToken,[ctx],[classifiers]
.close:
 stdcall PAdd,pClose
 stdcall PAdd,pClose
 cmp [pFailed],0
 jne .bad
 mov eax,1
 ret
.bad:
 mov [pFailed],1
 xor eax,eax
 ret
endp

proc PArgumentItems uses ebx esi edi,ctx,token,wrote
 locals
  index dd ?
  count dd ?
 endl
 cmp [token],0
 je .okay
 stdcall PIsArray,[ctx],[token]
 test eax,eax
 jz .bad
 stdcall JsonCount,[ctx],[token]
 mov [count],eax
 mov [index],0
.next:
 mov eax,[index]
 cmp eax,[count]
 jae .okay
 mov ebx,[wrote]
 cmp dword [ebx],0
 je .item
 stdcall PAdd,pComma
.item:
 stdcall JsonAt,[ctx],[token],[index]
 stdcall PToken,[ctx],eax
 mov ebx,[wrote]
 inc dword [ebx]
 inc [index]
 jmp .next
.okay:
 mov eax,1
 ret
.bad:
 mov [pFailed],1
 xor eax,eax
 ret
endp

proc PArguments uses ebx esi edi
 locals
  parent dd ?
  child dd ?
  wrote dd ?
 endl
 stdcall JsonGet,[pParent],[pParentRoot],pArgsKey
 mov [parent],eax
 test eax,eax
 jz .bad
 stdcall JsonGet,[pChild],[pChildRoot],pArgsKey
 mov [child],eax
 stdcall PAdd,pArgsOpen
 mov [wrote],0
 stdcall JsonGet,[pParent],[parent],pJvmKey
 stdcall PArgumentItems,[pParent],eax,addr wrote
 test eax,eax
 jz .bad
 stdcall JsonGet,[pChild],[child],pJvmKey
 stdcall PArgumentItems,[pChild],eax,addr wrote
 test eax,eax
 jz .bad
 stdcall PAdd,pGameOpen
 mov [wrote],0
 stdcall JsonGet,[pParent],[parent],pGameKey
 stdcall PArgumentItems,[pParent],eax,addr wrote
 test eax,eax
 jz .bad
 stdcall JsonGet,[pChild],[child],pGameKey
 stdcall PArgumentItems,[pChild],eax,addr wrote
 test eax,eax
 jz .bad
 stdcall PAdd,pArgsClose
 mov eax,1
 ret
.bad:
 mov [pFailed],1
 xor eax,eax
 ret
endp

proc PLibraries uses ebx esi edi
 locals
  parent dd ?
  child dd ?
  parentCount dd ?
  childCount dd ?
  index dd ?
  childIndex dd ?
  token dd ?
  wrote dd ?
  parentKey rb 1536
  childKey rb 1536
 endl
 stdcall JsonGet,[pParent],[pParentRoot],pLibsKey
 mov [parent],eax
 stdcall PIsArray,[pParent],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[pParent],[parent]
 cmp eax,2048
 ja .bad
 mov [parentCount],eax
 stdcall JsonGet,[pChild],[pChildRoot],pLibsKey
 mov [child],eax
 mov [childCount],0
 test eax,eax
 jz .start
 stdcall PIsArray,[pChild],eax
 test eax,eax
 jz .bad
 stdcall JsonCount,[pChild],[child]
 cmp eax,2048
 ja .bad
 mov [childCount],eax
.start:
 stdcall PAdd,pLibsOpen
 mov [wrote],0
 mov [index],0
.parent:
 mov eax,[index]
 cmp eax,[parentCount]
 jae .children
 stdcall JsonAt,[pParent],[parent],eax
 mov [token],eax
 stdcall PLibraryKey,[pParent],eax,addr parentKey
 test eax,eax
 jz .bad
 mov [childIndex],0
.findOverride:
 mov eax,[childIndex]
 cmp eax,[childCount]
 jae .parentItem
 stdcall JsonAt,[pChild],[child],eax
 stdcall PLibraryKey,[pChild],eax,addr childKey
 test eax,eax
 jz .bad
 invoke lstrcmpA,addr parentKey,addr childKey
 test eax,eax
 jz .nextParent
 inc [childIndex]
 jmp .findOverride
.parentItem:
 cmp [wrote],0
 je .emitParent
 stdcall PAdd,pComma
.emitParent:
 stdcall PToken,[pParent],[token]
 inc [wrote]
.nextParent:
 inc [index]
 jmp .parent
.children:
 mov [index],0
.child:
 mov eax,[index]
 cmp eax,[childCount]
 jae .done
 stdcall JsonAt,[pChild],[child],eax
 mov [token],eax
 cmp [wrote],0
 je .emitChild
 stdcall PAdd,pComma
.emitChild:
 stdcall PWriteLibrary,[pChild],[token]
 test eax,eax
 jz .bad
 inc [wrote]
 inc [index]
 jmp .child
.done:
 stdcall PAdd,pArrayClose
 mov eax,1
 ret
.bad:
 mov [pFailed],1
 xor eax,eax
 ret
endp

proc PSpecial,key
 invoke lstrcmpA,[key],pVersionKey
 test eax,eax
 jz .yes
 invoke lstrcmpA,[key],pInheritKey
 test eax,eax
 jz .yes
 invoke lstrcmpA,[key],pArgsKey
 test eax,eax
 jz .yes
 invoke lstrcmpA,[key],pLibsKey
 test eax,eax
 jz .yes
 invoke lstrcmpA,[key],pNormalizedKey
 test eax,eax
 jz .yes
 invoke lstrcmpA,[key],pBaseKey
 test eax,eax
 jz .yes
 xor eax,eax
 ret
.yes:
 mov eax,1
 ret
endp

; Merge root fields, preferring child fields. The three composite fields above
; are emitted separately; no inheritance remains in the serialized result.
proc ProfileMerge uses ebx esi edi,parentCtx,childCtx,childIdA
 locals
  count dd ?
  index dd ?
  key dd ?
  value dd ?
  context dd ?
  result dd ?
 endl
 mov [pOutput],0
 mov [pLength],0
 mov [pFailed],0
 mov eax,[parentCtx]
 mov [pParent],eax
 mov eax,[childCtx]
 mov [pChild],eax
 stdcall PSafeId,[childIdA]
 test eax,eax
 jz .bad
 stdcall JsonRoot,[pParent]
 mov [pParentRoot],eax
 stdcall JsonRoot,[pChild]
 mov [pChildRoot],eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,HEAP_ZERO_MEMORY,P_CAP
 test eax,eax
 jz .bad
 mov [pOutput],eax
 stdcall PAdd,pOpen
 stdcall PQuoted,[childIdA]
 stdcall PAdd,pNormalizedOpen
 stdcall JsonGet,[pParent],[pParentRoot],pVersionKey
 stdcall PString,[pParent],eax,pValue,4096
 test eax,eax
 jz .bad
 stdcall PSafeId,pValue
 test eax,eax
 jz .bad
 stdcall PQuoted,pValue
 stdcall JsonCount,[pParent],[pParentRoot]
 mov [count],eax
 mov [index],0
.parent:
 mov eax,[index]
 cmp eax,[count]
 jae .children
 stdcall JsonKeyAt,[pParent],[pParentRoot],eax
 mov [key],eax
 stdcall PString,[pParent],eax,pKey,256
 test eax,eax
 jz .bad
 stdcall PSpecial,pKey
 test eax,eax
 jnz .nextParent
 mov eax,[pParent]
 mov [context],eax
 stdcall JsonValueAt,[pParent],[pParentRoot],[index]
 mov [value],eax
 stdcall JsonGet,[pChild],[pChildRoot],pKey
 test eax,eax
 jz .parentField
 mov [value],eax
 mov eax,[pChild]
 mov [context],eax
.parentField:
 stdcall PAdd,pComma
 stdcall PToken,[pParent],[key]
 stdcall PAdd,pColon
 stdcall PToken,[context],[value]
.nextParent:
 inc [index]
 jmp .parent
.children:
 stdcall JsonCount,[pChild],[pChildRoot]
 mov [count],eax
 mov [index],0
.child:
 mov eax,[index]
 cmp eax,[count]
 jae .composites
 stdcall JsonKeyAt,[pChild],[pChildRoot],eax
 mov [key],eax
 stdcall PString,[pChild],eax,pKey,256
 test eax,eax
 jz .bad
 stdcall PSpecial,pKey
 test eax,eax
 jnz .nextChild
 stdcall JsonGet,[pParent],[pParentRoot],pKey
 test eax,eax
 jnz .nextChild
 stdcall PAdd,pComma
 stdcall PToken,[pChild],[key]
 stdcall PAdd,pColon
 stdcall JsonValueAt,[pChild],[pChildRoot],[index]
 stdcall PToken,[pChild],eax
.nextChild:
 inc [index]
 jmp .child
.composites:
 stdcall PArguments
 test eax,eax
 jz .bad
 stdcall PLibraries
 test eax,eax
 jz .bad
 stdcall PAdd,pClose
 cmp [pFailed],0
 jne .bad
 stdcall JsonParse,[pOutput]
 test eax,eax
 jz .bad
 mov [result],eax
 stdcall JsonRoot,eax
 mov ebx,eax
 stdcall JsonGet,[result],ebx,pMainKey
 test eax,eax
 jz .invalid
 stdcall JsonGet,[result],ebx,pInheritKey
 test eax,eax
 jnz .invalid
 stdcall JsonFree,[result]
 mov eax,[pOutput]
 mov [pOutput],0
 ret
.invalid:
 stdcall JsonFree,[result]
.bad:
 stdcall FreeMem,[pOutput]
 mov [pOutput],0
 xor eax,eax
 ret
endp

proc PVersionPath uses esi edi,idA,dest,extension
 stdcall PSafeId,[idA]
 test eax,eax
 jz .bad
 invoke lstrlenW,gameDir
 cmp eax,1000
 ja .bad
 invoke lstrcpyW,[dest],gameDir
 invoke lstrcatW,[dest],pVersionsW
 invoke MultiByteToWideChar,65001,8,[idA],-1,pWide,256
 test eax,eax
 jz .bad
 invoke lstrcatW,[dest],pWide
 invoke lstrcatW,[dest],pSlashW
 invoke lstrcatW,[dest],pWide
 invoke lstrcatW,[dest],[extension]
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp

proc ProfileNormalize uses ebx esi edi,childIdA,baseIdA
 locals
  parentText dd ?
  childText dd ?
  parentCtx dd ?
  childCtx dd ?
  merged dd ?
  root dd ?
  file dd ?
  wrote dd ?
  result dd ?
 endl
 mov [parentText],0
 mov [childText],0
 mov [parentCtx],0
 mov [childCtx],0
 mov [merged],0
 mov [file],-1
 mov [result],0
 cmp [engineCancel],0
 jne .done
 invoke lstrcmpA,[childIdA],[baseIdA]
 test eax,eax
 jz .done
 stdcall PVersionPath,[childIdA],pChildPath,pJsonW
 test eax,eax
 jz .done
 stdcall PVersionPath,[baseIdA],pBasePath,pJsonW
 test eax,eax
 jz .done
 stdcall PVersionPath,[childIdA],pChildJar,pJarW
 test eax,eax
 jz .done
 stdcall PVersionPath,[baseIdA],pBaseJar,pJarW
 test eax,eax
 jz .done
 stdcall EReadFile,pChildPath
 test eax,eax
 jz .done
 mov [childText],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [childCtx],eax
 stdcall JsonRoot,eax
 mov [root],eax
 stdcall JsonGet,[childCtx],eax,pVersionKey
 stdcall PString,[childCtx],eax,pValue,4096
 invoke lstrcmpA,pValue,[childIdA]
 test eax,eax
 jnz .done
 stdcall JsonGet,[childCtx],[root],pInheritKey
 test eax,eax
 jnz .inherit
 ; An already-normalized profile is reusable without replacing its backup.
 stdcall JsonGet,[childCtx],[root],pNormalizedKey
 stdcall JsonBool,[childCtx],eax
 cmp eax,1
 jne .done
 stdcall JsonGet,[childCtx],[root],pBaseKey
 stdcall PString,[childCtx],eax,pValue,4096
 test eax,eax
 jz .done
 invoke lstrcmpA,pValue,[baseIdA]
 test eax,eax
 jnz .done
 stdcall JsonGet,[childCtx],[root],pArgsKey
 test eax,eax
 jz .done
 mov ebx,eax
 stdcall JsonGet,[childCtx],ebx,pJvmKey
 stdcall PIsArray,[childCtx],eax
 test eax,eax
 jz .done
 stdcall JsonGet,[childCtx],ebx,pGameKey
 stdcall PIsArray,[childCtx],eax
 test eax,eax
 jz .done
 stdcall JsonGet,[childCtx],[root],pMainKey
 stdcall PString,[childCtx],eax,pValue,4096
 test eax,eax
 jz .done
 stdcall JsonGet,[childCtx],[root],pLibsKey
 stdcall PIsArray,[childCtx],eax
 test eax,eax
 jz .done
 stdcall JsonGet,[childCtx],[root],pDownloadsKey
 stdcall JsonGet,[childCtx],eax,pClientKey
 stdcall JsonGet,[childCtx],eax,pShaKey
 stdcall PString,[childCtx],eax,pHash,64
 stdcall PIsHash,pHash
 test eax,eax
 jz .done
 stdcall VerifySha1,pChildJar,pHash
 test eax,eax
 jz .done
 mov [result],1
 jmp .done
.inherit:
 stdcall PString,[childCtx],eax,pValue,4096
 test eax,eax
 jz .done
 invoke lstrcmpA,pValue,[baseIdA]
 test eax,eax
 jnz .done
 stdcall EReadFile,pBasePath
 test eax,eax
 jz .done
 mov [parentText],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .done
 mov [parentCtx],eax
 stdcall JsonRoot,eax
 mov [root],eax
 stdcall JsonGet,[parentCtx],eax,pInheritKey
 test eax,eax
 jnz .done
 stdcall JsonGet,[parentCtx],[root],pVersionKey
 stdcall PString,[parentCtx],eax,pValue,4096
 test eax,eax
 jz .done
 invoke lstrcmpA,pValue,[baseIdA]
 test eax,eax
 jnz .done
 stdcall JsonGet,[parentCtx],[root],pDownloadsKey
 stdcall JsonGet,[parentCtx],eax,pClientKey
 stdcall JsonGet,[parentCtx],eax,pShaKey
 stdcall PString,[parentCtx],eax,pHash,64
 stdcall PIsHash,pHash
 test eax,eax
 jz .done
 stdcall VerifySha1,pBaseJar,pHash
 test eax,eax
 jz .done
 stdcall ProfileMerge,[parentCtx],[childCtx],[childIdA]
 test eax,eax
 jz .done
 mov [merged],eax
 cmp [engineCancel],0
 jne .done
 ; Re-read the parent hash: child library preparation uses pHash.
 stdcall JsonGet,[parentCtx],[root],pDownloadsKey
 stdcall JsonGet,[parentCtx],eax,pClientKey
 stdcall JsonGet,[parentCtx],eax,pShaKey
 stdcall PString,[parentCtx],eax,pHash,64
 invoke GetFileAttributesW,pChildJar
 cmp eax,-1
 je .link
 stdcall VerifySha1,pChildJar,pHash
 test eax,eax
 jz .done
 jmp .backup
.link:
 invoke CreateHardLinkW,pChildJar,pBaseJar,0
 test eax,eax
 jnz .backup
 invoke CopyFileW,pBaseJar,pChildJar,1
 test eax,eax
 jz .done
.backup:
 invoke lstrcpyW,pBackupPath,gameDir
 invoke lstrcatW,pBackupPath,pVersionsW
 invoke MultiByteToWideChar,65001,8,[childIdA],-1,pWide,256
 invoke lstrcatW,pBackupPath,pWide
 invoke lstrcatW,pBackupPath,pOriginalW
 invoke GetFileAttributesW,pBackupPath
 cmp eax,-1
 je .copyBackup
 test eax,FILE_ATTRIBUTE_DIRECTORY+400h
 jnz .done
 jmp .temp
.copyBackup:
 invoke CopyFileW,pChildPath,pBackupPath,1
 test eax,eax
 jz .done
.temp:
 invoke lstrcpyW,pTempPath,pChildPath
 invoke lstrcatW,pTempPath,pTempW
 invoke CreateFileW,pTempPath,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .done
 mov [file],eax
 invoke lstrlenA,[merged]
 mov ebx,eax
 invoke WriteFile,[file],[merged],ebx,addr wrote,0
 test eax,eax
 jz .done
 cmp ebx,[wrote]
 jne .done
 invoke CloseHandle,[file]
 mov [file],-1
 cmp [engineCancel],0
 jne .done
 invoke MoveFileExW,pTempPath,pChildPath,9
 mov [result],eax
.done:
 cmp [file],-1
 je .free
 invoke CloseHandle,[file]
.free:
 stdcall FreeMem,[merged]
 stdcall JsonFree,[parentCtx]
 stdcall JsonFree,[childCtx]
 stdcall FreeMem,[parentText]
 stdcall FreeMem,[childText]
 cmp [result],0
 jne .return
 stdcall SetStatusA,pErr
.return:
 mov eax,[result]
 ret
endp
