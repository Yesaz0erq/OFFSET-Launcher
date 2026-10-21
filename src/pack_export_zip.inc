; Original bounded ZIP/store writer. Caller owns the CREATE_NEW temporary path.
EZ_MAX = 4096
EZ_REC = 1552
EZ_NAME = 16
EZ_WNAME = 528
EZ_ARCHIVE_MAX = 536870912
EZ_FILE_MAX = 134217728
section '.ezd' data readable writeable
exZipOwned dd 0
ezHandle dd -1
ezFailed dd 0
ezOffset dd 0
ezCount dd 0
ezCentralBytes dd 0
ezNameLength dd 0
ezCrc dd 0
ezFileBytes dd 0
ezHeader rb 46
ezNameW rw 512
ezBuffer rb 65536
ezTable rd 256
ezRecords rb EZ_REC*EZ_MAX

section '.ezc' code readable executable
 nop
proc ExZipAbort
 cmp [ezHandle],-1
 je .done
 invoke CloseHandle,[ezHandle]
 mov [ezHandle],-1
.done:
 mov eax,1
 ret
endp
proc ExZipBegin uses ebx esi edi,temporaryW
 cmp [ezHandle],-1
 jne .bad
 mov [exZipOwned],0
 mov [ezFailed],0
 mov [ezOffset],0
 mov [ezCount],0
 mov [ezCentralBytes],0
 invoke CreateFileW,[temporaryW],GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [ezHandle],eax
 mov [exZipOwned],1
 xor ebx,ebx
.table:
 mov eax,ebx
 mov ecx,8
.bit:
 shr eax,1
 jnc .nextBit
 xor eax,0EDB88320h
.nextBit:
 loop .bit
 mov [ezTable+ebx*4],eax
 inc ebx
 cmp ebx,256
 jb .table
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc EzCrcUpdate uses ebx esi,initial,data,bytes
 mov eax,[initial]
 mov esi,[data]
 mov ecx,[bytes]
 test ecx,ecx
 jz .done
.next:
 movzx ebx,byte [esi]
 xor bl,al
 shr eax,8
 xor eax,[ezTable+ebx*4]
 inc esi
 dec ecx
 jnz .next
.done:
 ret
endp
proc EzWrite uses ebx esi edi,data,bytes
 local got:DWORD
 cmp [ezHandle],-1
 je .bad
 cmp [ezFailed],0
 jne .bad
 if defined engineCancel
 cmp [engineCancel],0
 jne .bad
 end if
 if defined modCancel
 cmp [modCancel],0
 jne .bad
 end if
 mov edi,[bytes]
 mov eax,EZ_ARCHIVE_MAX
 sub eax,[ezOffset]
 cmp edi,eax
 ja .bad
 mov esi,[data]
.write:
 test edi,edi
 jz .good
 invoke WriteFile,[ezHandle],esi,edi,addr got,0
 test eax,eax
 jz .bad
 mov eax,[got]
 test eax,eax
 jz .bad
 cmp eax,edi
 ja .bad
 add [ezOffset],eax
 add esi,eax
 sub edi,eax
 jmp .write
.good:
 mov eax,1
 ret
.bad:
 mov [ezFailed],1
 xor eax,eax
 ret
endp
; Names are UTF-8, slash-separated, and unique under Windows case folding.
proc EzValidate uses ebx esi edi,nameA,bytes
 cmp [ezHandle],-1
 je .bad
 cmp [ezFailed],0
 jne .bad
 cmp [ezCount],EZ_MAX
 jae .bad
 cmp [bytes],EZ_FILE_MAX
 ja .bad
 invoke lstrlenA,[nameA]
 test eax,eax
 jz .bad
 cmp eax,511
 ja .bad
 mov [ezNameLength],eax
 invoke MultiByteToWideChar,65001,8,[nameA],-1,ezNameW,512
 test eax,eax
 jz .bad
 mov esi,[nameA]
 mov edi,esi
.scan:
 mov al,[esi]
 test al,al
 jz .component
 cmp al,32
 jb .bad
 cmp al,127
 je .bad
 cmp al,92
 je .bad
 cmp al,':'
 je .bad
 cmp al,'/'
 je .component
 inc esi
 jmp .scan
.component:
 cmp esi,edi
 je .bad
 cmp byte [esi-1],'.'
 je .bad
 cmp byte [esi-1],' '
 je .bad
 cmp byte [esi],0
 je .duplicates
 lea edi,[esi+1]
 inc esi
 jmp .scan
.duplicates:
 xor ebx,ebx
.compare:
 cmp ebx,[ezCount]
 jae .space
 imul eax,ebx,EZ_REC
 add eax,ezRecords+EZ_WNAME
 invoke lstrcmpiW,eax,ezNameW
 test eax,eax
 jz .bad
 inc ebx
 jmp .compare
.space:
 ; Reserve all pending central headers and EOCD before writing this entry.
 mov eax,[ezNameLength]
 shl eax,1
 add eax,30+46+22
 add eax,[bytes]
 jc .bad
 add eax,[ezCentralBytes]
 jc .bad
 add eax,[ezOffset]
 jc .bad
 cmp eax,EZ_ARCHIVE_MAX
 ja .bad
 mov eax,1
 ret
.bad:
 mov [ezFailed],1
 xor eax,eax
 ret
endp
proc EzLocal uses ebx esi edi,nameA,bytes,crc
 mov edi,ezHeader
 xor eax,eax
 mov ecx,46/2
 rep stosw
 mov dword [ezHeader],04034B50h
 mov word [ezHeader+4],20
 mov word [ezHeader+6],0800h
 mov word [ezHeader+12],33 ; 1980-01-01, valid DOS date.
 mov eax,[crc]
 mov dword [ezHeader+14],eax
 mov eax,[bytes]
 mov dword [ezHeader+18],eax
 mov dword [ezHeader+22],eax
 mov eax,[ezNameLength]
 mov word [ezHeader+26],ax
 imul ebx,[ezCount],EZ_REC
 add ebx,ezRecords
 mov eax,[ezOffset]
 mov [ebx],eax
 mov eax,[crc]
 mov [ebx+4],eax
 mov eax,[bytes]
 mov [ebx+8],eax
 mov eax,[ezNameLength]
 mov [ebx+12],eax
 lea eax,[ebx+EZ_NAME]
 invoke lstrcpyA,eax,[nameA]
 lea eax,[ebx+EZ_WNAME]
 invoke lstrcpyW,eax,ezNameW
 stdcall EzWrite,ezHeader,30
 test eax,eax
 jz .bad
 stdcall EzWrite,[nameA],[ezNameLength]
 ret
.bad:
 xor eax,eax
 ret
endp
proc EzEntryDone
 inc [ezCount]
 mov eax,[ezNameLength]
 add eax,46
 add [ezCentralBytes],eax
 mov eax,1
 ret
endp
proc ExZipAddMemory,nameA,data,bytes
 stdcall EzValidate,[nameA],[bytes]
 test eax,eax
 jz .bad
 stdcall EzCrcUpdate,-1,[data],[bytes]
 not eax
 stdcall EzLocal,[nameA],[bytes],eax
 test eax,eax
 jz .bad
 stdcall EzWrite,[data],[bytes]
 test eax,eax
 jz .bad
 stdcall EzEntryDone
 ret
.bad:
 xor eax,eax
 ret
endp
proc ExZipAddFile uses ebx esi edi,nameA,sourceW
 local source:DWORD,remaining:DWORD,got:DWORD,info:BY_HANDLE_FILE_INFORMATION,result:DWORD
 mov [source],-1
 mov [result],0
 ; OPEN_REPARSE_POINT + handle attributes rejects swapped symbolic links.
 invoke CreateFileW,[sourceW],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL+00200000h,0
 cmp eax,-1
 je .bad
 mov [source],eax
 invoke GetFileInformationByHandle,eax,addr info
 test eax,eax
 jz .bad
 test [info.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY+400h
 jnz .bad
 cmp [info.nFileSizeHigh],0
 jne .bad
 mov eax,[info.nFileSizeLow]
 mov [ezFileBytes],eax
 stdcall EzValidate,[nameA],eax
 test eax,eax
 jz .bad
 mov eax,[ezFileBytes]
 mov [remaining],eax
 mov [ezCrc],-1
.hash:
 cmp [remaining],0
 je .rewind
 if defined engineCancel
 cmp [engineCancel],0
 jne .bad
 end if
 mov ebx,[remaining]
 cmp ebx,65536
 jbe @f
 mov ebx,65536
@@:
 invoke ReadFile,[source],ezBuffer,ebx,addr got,0
 test eax,eax
 jz .bad
 cmp [got],0
 je .bad
 cmp [got],ebx
 ja .bad
 stdcall EzCrcUpdate,[ezCrc],ezBuffer,[got]
 mov [ezCrc],eax
 mov eax,[got]
 sub [remaining],eax
 jmp .hash
.rewind:
 invoke SetFilePointer,[source],0,0,FILE_BEGIN
 test eax,eax
 jnz .bad
 mov eax,[ezCrc]
 not eax
 stdcall EzLocal,[nameA],[ezFileBytes],eax
 test eax,eax
 jz .bad
 mov eax,[ezFileBytes]
 mov [remaining],eax
.copy:
 cmp [remaining],0
 je .copied
 mov ebx,[remaining]
 cmp ebx,65536
 jbe @f
 mov ebx,65536
@@:
 invoke ReadFile,[source],ezBuffer,ebx,addr got,0
 test eax,eax
 jz .bad
 cmp [got],0
 je .bad
 cmp [got],ebx
 ja .bad
 stdcall EzWrite,ezBuffer,[got]
 test eax,eax
 jz .bad
 mov eax,[got]
 sub [remaining],eax
 jmp .copy
.copied:
 stdcall EzEntryDone
 mov [result],eax
 jmp .close
.bad:
 mov [ezFailed],1
.close:
 cmp [source],-1
 je .return
 invoke CloseHandle,[source]
.return:
 mov eax,[result]
 ret
endp
proc ExZipFinish uses ebx esi edi
 local startOffset:DWORD,result:DWORD
 mov [result],0
 cmp [ezHandle],-1
 je .close
 cmp [ezFailed],0
 jne .close
 mov eax,[ezOffset]
 mov [startOffset],eax
 xor ebx,ebx
.central:
 cmp ebx,[ezCount]
 jae .endRecord
 imul esi,ebx,EZ_REC
 add esi,ezRecords
 mov edi,ezHeader
 xor eax,eax
 mov ecx,46/2
 rep stosw
 mov dword [ezHeader],02014B50h
 mov dword [ezHeader+4],00140014h
 mov word [ezHeader+8],0800h
 mov word [ezHeader+14],33
 mov eax,[esi+4]
 mov dword [ezHeader+16],eax
 mov eax,[esi+8]
 mov dword [ezHeader+20],eax
 mov dword [ezHeader+24],eax
 mov eax,[esi+12]
 mov word [ezHeader+28],ax
 mov eax,[esi]
 mov dword [ezHeader+42],eax
 stdcall EzWrite,ezHeader,46
 test eax,eax
 jz .close
 lea eax,[esi+EZ_NAME]
 stdcall EzWrite,eax,dword [esi+12]
 test eax,eax
 jz .close
 inc ebx
 jmp .central
.endRecord:
 mov edi,ezHeader
 xor eax,eax
 mov ecx,46/2
 rep stosw
 mov dword [ezHeader],06054B50h
 mov eax,[ezCount]
 mov word [ezHeader+8],ax
 mov word [ezHeader+10],ax
 mov eax,[ezOffset]
 sub eax,[startOffset]
 mov dword [ezHeader+12],eax
 mov eax,[startOffset]
 mov dword [ezHeader+16],eax
 stdcall EzWrite,ezHeader,22
 test eax,eax
 jz .close
 mov [result],1
.close:
 stdcall ExZipAbort
 mov eax,[result]
 ret
endp
