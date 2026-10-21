; Readable project descriptions. External HTML/Markdown is never executed.
section '.detaild' data readable writeable
detailReady dd 0
dqContext dd 0
dqResponse dd 0
dqRoot dd 0
dqProvider dd 0
dqIsPack dd 0
dqMrProject db 'https://api.modrinth.com/v2/project/',0
dqCfProject db 'https://api.curseforge.com/v1/mods/',0
dqDescriptionSuffix db '/description',0
dqKBody db 'body',0
dqKLicense db 'license',0
dqKLicenseId db 'id',0
dqKDownloads db 'downloads',0
dqKDownloadCount db 'downloadCount',0
dqKAuthors db 'authors',0
dqMetaMr db 'Modrinth',13,10,'Downloads: %u',13,10,'License: %s',0
dqMetaCf db 'CurseForge',13,10,'Downloads: %u',13,10,'Author: %s',0
dqUnknown db '-',0
dqFormat db '%u',0
dqMrType db 'mod',0
dqMrPack db 'modpack',0
dqSearchBase db 'https://search.mcmod.cn/s?key=',0
dqHome db 'https://www.mcmod.cn/',0
dqLoadingStatus db '正在读取项目介绍…',0
dqReadyStatus db '项目介绍已加载。',0
dqFailureStatus db '项目介绍暂不可用，可访问项目官网。',0
dqEntityAmp db '&amp;',0
dqEntityLt db '&lt;',0
dqEntityGt db '&gt;',0
dqEntityQuot db '&quot;',0
dqEntityApos db '&apos;',0
dqEntityNbsp db '&nbsp;',0
dqScript db 'script',0
dqStyle db 'style',0
dqIframe db 'iframe',0
dqObject db 'object',0
dqBr db 'br',0
dqP db 'p',0
dqDiv db 'div',0
dqLi db 'li',0
dqTr db 'tr',0
dqH1 db 'h1',0
dqH2 db 'h2',0
dqH3 db 'h3',0
detailTitle rb 512
detailSummary rb 1024
detailBody rb 131072
detailMeta rb 2048
dqRaw rb 131072
dqUrl rb 8192
dqUrlW rw 8192
dqId rb 64
dqValue rb 512
dqSearch rb 4096
dqEncoded rb 3072

section '.detailc' code readable executable
 nop
proc DqClose
 stdcall JsonFree,[dqContext]
 mov [dqContext],0
 stdcall FreeMem,[dqResponse]
 mov [dqResponse],0
 ret
endp

proc DqRequest
 stdcall DqClose
 mov eax,[modCancel]
 or eax,[engineCancel]
 jnz .bad
 invoke MultiByteToWideChar,65001,8,dqUrl,-1,dqUrlW,8192
 test eax,eax
 jz .bad
 cmp [dqProvider],0
 jne .curse
 stdcall HttpRequest,dqUrlW,0,mrHeader,0
 jmp .response
.curse:
 stdcall CfMakeHeaders
 test eax,eax
 jz .bad
 stdcall HttpRequest,dqUrlW,0,cfHeadersW,0
 mov [dqResponse],eax
 stdcall CfWipeHeaders
 mov eax,[dqResponse]
.response:
 mov [dqResponse],eax
 test eax,eax
 jz .bad
 cmp [httpStatus],200
 jne .bad
 mov eax,[modCancel]
 or eax,[engineCancel]
 jnz .bad
 stdcall JsonParse,[dqResponse]
 test eax,eax
 jz .bad
 mov [dqContext],eax
 stdcall JsonRoot,eax
 mov [dqRoot],eax
 mov eax,1
 ret
.bad:
 stdcall DqClose
 xor eax,eax
 ret
endp

; Case-folded bounded tag name match; returns byte after the whole tag name.
proc DqTag uses esi edi,source,expected
 mov esi,[source]
 mov edi,[expected]
.loop:
 mov al,[edi]
 test al,al
 jz .boundary
 mov dl,[esi]
 cmp dl,'A'
 jb .lowered
 cmp dl,'Z'
 ja .lowered
 add dl,32
.lowered:
 cmp al,dl
 jne .no
 inc esi
 inc edi
 jmp .loop
.boundary:
 mov al,[esi]
 cmp al,'>'
 je .yes
 cmp al,'/'
 je .yes
 cmp al,32
 jbe .yes
.no:
 xor eax,eax
 ret
.yes:
 mov eax,esi
 ret
endp

; Convert to plain UTF-8 with CRLF paragraphs, no images/scripts/link targets.
; Formatting is deliberately native text. Original rich page stays on website.
proc DetailPlain uses ebx esi edi,source,destination,capacity
 locals
  blocked dd ?
  tag dd ?
  closing dd ?
  paragraph dd ?
  quote dd ?
  limit dd ?
  after dd ?
  unicode rw 2
  utf8 rb 8
 endl
 mov esi,[source]
 mov edi,[destination]
 mov eax,[capacity]
 cmp eax,4
 jb .bad
 add eax,edi
 sub eax,4
 mov [limit],eax
 mov [blocked],0
.loop:
 movzx ebx,byte [esi]
 test ebx,ebx
 jz .done
 cmp edi,[limit]
 jae .bad
 cmp bl,'<'
 je .tagStart
 cmp [blocked],0
 jne .skip
 cmp bl,'&'
 je .entity
 cmp bl,13
 je .skip
 cmp bl,10
 je .newline
 cmp bl,9
 je .space
 cmp bl,32
 jb .skip
 cmp bl,'!'
 jne .markdown
 cmp byte [esi+1],'['
 je .skip
.markdown:
 cmp bl,'*'
 je .skip
 cmp bl,'`'
 je .skip
 cmp bl,'#'
 je .heading
 cmp bl,'['
 je .skip
 cmp bl,']'
 je .linkEnd
 mov al,bl
 stosb
 inc esi
 jmp .loop
.heading:
 cmp byte [esi+1],' '
 je .skip
 cmp byte [esi+1],'#'
 je .skip
 mov al,bl
 stosb
 jmp .skip
.linkEnd:
 inc esi
 cmp byte [esi],'('
 jne .loop
 mov ebx,1
 inc esi
.linkTarget:
 mov al,[esi]
 test al,al
 jz .done
 inc esi
 cmp al,'('
 jne .closeTarget
 inc ebx
.closeTarget:
 cmp al,')'
 jne .linkTarget
 dec ebx
 jnz .linkTarget
 jmp .loop
.space:
 mov al,' '
 stosb
 jmp .skip
.newline:
 cmp edi,[destination]
 je .skip
 cmp byte [edi-1],10
 je .skip
 mov ax,0A0Dh
 stosw
 jmp .skip
.tagStart:
 inc esi
 mov [closing],0
 cmp byte [esi],'/'
 jne .tagName
 mov [closing],1
 inc esi
.tagName:
 cmp byte [esi],' '
 jne .name
 inc esi
 jmp .tagName
.name:
 mov [tag],esi
 mov [paragraph],0
 mov ebx,dqScript
 stdcall DqTag,esi,dqScript
 test eax,eax
 jnz .suppress
 mov ebx,dqStyle
 stdcall DqTag,esi,dqStyle
 test eax,eax
 jnz .suppress
 mov ebx,dqIframe
 stdcall DqTag,esi,dqIframe
 test eax,eax
 jnz .suppress
 mov ebx,dqObject
 stdcall DqTag,esi,dqObject
 test eax,eax
 jnz .suppress
 stdcall DqTag,esi,dqBr
 test eax,eax
 jnz .paragraph
 stdcall DqTag,esi,dqP
 test eax,eax
 jnz .paragraph
 stdcall DqTag,esi,dqDiv
 test eax,eax
 jnz .paragraph
 stdcall DqTag,esi,dqLi
 test eax,eax
 jnz .paragraph
 stdcall DqTag,esi,dqTr
 test eax,eax
 jnz .paragraph
 stdcall DqTag,esi,dqH1
 test eax,eax
 jnz .paragraph
 stdcall DqTag,esi,dqH2
 test eax,eax
 jnz .paragraph
 stdcall DqTag,esi,dqH3
 test eax,eax
 jz .scanTag
.paragraph:
 mov [paragraph],1
 jmp .scanTag
.suppress:
 cmp [closing],0
 jne .unsuppress
 cmp [blocked],0
 jne .scanTag
 mov [blocked],ebx
 jmp .scanTag
.unsuppress:
 cmp [blocked],ebx
 jne .scanTag
 mov [blocked],0
.scanTag:
 mov [quote],0
.tagByte:
 mov al,[esi]
 test al,al
 jz .done
 inc esi
 cmp [quote],0
 jne .inQuote
 cmp al,34
 je .openQuote
 cmp al,39
 je .openQuote
 cmp al,'>'
 jne .tagByte
 cmp [blocked],0
 jne .loop
 cmp [paragraph],0
 je .loop
 cmp edi,[destination]
 je .loop
 cmp byte [edi-1],10
 je .loop
 mov ax,0A0Dh
 stosw
 jmp .loop
.openQuote:
 movzx eax,al
 mov [quote],eax
 jmp .tagByte
.inQuote:
 cmp al,byte [quote]
 jne .tagByte
 mov [quote],0
 jmp .tagByte
.entity:
 ; Fixed named entities and numeric scalar references only.
 stdcall CfPrefix,esi,dqEntityAmp
 test eax,eax
 jz .entityLt
 mov al,'&'
 mov ebx,5
 jmp .entityWrite
.entityLt:
 stdcall CfPrefix,esi,dqEntityLt
 test eax,eax
 jz .entityGt
 mov al,'<'
 mov ebx,4
 jmp .entityWrite
.entityGt:
 stdcall CfPrefix,esi,dqEntityGt
 test eax,eax
 jz .entityQuot
 mov al,'>'
 mov ebx,4
 jmp .entityWrite
.entityQuot:
 stdcall CfPrefix,esi,dqEntityQuot
 test eax,eax
 jz .entityApos
 mov al,34
 mov ebx,6
 jmp .entityWrite
.entityApos:
 stdcall CfPrefix,esi,dqEntityApos
 test eax,eax
 jz .entityNbsp
 mov al,39
 mov ebx,6
 jmp .entityWrite
.entityNbsp:
 stdcall CfPrefix,esi,dqEntityNbsp
 test eax,eax
 jz .numeric
 mov al,' '
 mov ebx,6
.entityWrite:
 stosb
 add esi,ebx
 jmp .loop
.numeric:
 cmp byte [esi+1],'#'
 jne .literalAmp
 lea edx,[esi+2]
 mov ecx,10
 cmp byte [edx],'x'
 je .hex
 cmp byte [edx],'X'
 jne .number
.hex:
 inc edx
 mov ecx,16
.number:
 xor eax,eax
 xor ebx,ebx
.digit:
 movzx ebx,byte [edx]
 cmp bl,';'
 je .numberEnd
 sub ebx,'0'
 cmp ebx,9
 jbe .value
 cmp ecx,16
 jne .literalAmp
 movzx ebx,byte [edx]
 or bl,32
 sub ebx,'a'-10
 cmp ebx,10
 jb .literalAmp
 cmp ebx,15
 ja .literalAmp
.value:
 imul eax,ecx
 add eax,ebx
 cmp eax,10FFFFh
 ja .literalAmp
 inc edx
 jmp .digit
.numberEnd:
 cmp eax,32
 jb .literalAmp
 cmp eax,0D800h
 jb .scalar
 cmp eax,0DFFFh
 jbe .literalAmp
.scalar:
 inc edx
 mov [after],edx
 cmp eax,0FFFFh
 ja .pair
 mov word [unicode],ax
 mov ebx,1
 jmp .encode
.pair:
 sub eax,10000h
 mov edx,eax
 shr eax,10
 add eax,0D800h
 mov word [unicode],ax
 and edx,3FFh
 add edx,0DC00h
 mov word [unicode+2],dx
 mov ebx,2
.encode:
 invoke WideCharToMultiByte,65001,0,addr unicode,ebx,addr utf8,8,0,0
 test eax,eax
 jz .literalAmp
 mov edx,[limit]
 sub edx,edi
 cmp eax,edx
 jae .bad
 lea edx,[utf8]
 mov ecx,eax
.copyScalar:
 mov al,[edx]
 stosb
 inc edx
 loop .copyScalar
 mov esi,[after]
 jmp .loop
.literalAmp:
 mov al,'&'
 stosb
.skip:
 inc esi
 jmp .loop
.done:
 mov byte [edi],0
 mov eax,edi
 sub eax,[destination]
 ret
.bad:
 mov eax,[destination]
 mov byte [eax],0
 mov eax,-1
 ret
endp

proc ProjectFetchDetails uses ebx esi edi,provider,projectId,isPack
 local result:DWORD,root:DWORD,downloads:DWORD
 mov [result],0
 mov [detailReady],0
 mov byte [detailTitle],0
 mov byte [detailSummary],0
 mov byte [detailBody],0
 mov byte [detailMeta],0
 mov eax,[provider]
 cmp eax,1
 ja .bad
 mov [dqProvider],eax
 mov eax,[isPack]
 cmp eax,1
 ja .bad
 mov [dqIsPack],eax
 stdcall MrValidId,[projectId]
 test eax,eax
 jz .bad
 invoke lstrcpyA,dqId,[projectId]
 cmp [dqProvider],0
 jne .cfUrl
 invoke lstrcpyA,dqUrl,dqMrProject
 jmp .id
.cfUrl:
 stdcall ModNumber,dqId
 test eax,eax
 jz .bad
 invoke lstrcpyA,dqUrl,dqCfProject
.id:
 stdcall ModAppend,dqUrl,8192,dqId
 stdcall SetStatusA,dqLoadingStatus
 stdcall DqRequest
 test eax,eax
 jz .bad
 mov eax,[dqRoot]
 cmp [dqProvider],0
 je .root
 stdcall JsonGet,[dqContext],eax,cfKData
.root:
 mov [root],eax
 cmp [dqProvider],0
 jne .cfValidate
 stdcall JsonGet,[dqContext],[root],mrKId
 stdcall MrCopy,[dqContext],eax,dqValue,512
 invoke lstrcmpA,dqValue,dqId
 test eax,eax
 jnz .bad
 stdcall JsonGet,[dqContext],[root],mrKProjectType
 stdcall MrCopy,[dqContext],eax,dqValue,512
 mov ebx,dqMrType
 cmp [dqIsPack],0
 je .type
 mov ebx,dqMrPack
.type:
 invoke lstrcmpA,dqValue,ebx
 test eax,eax
 jnz .bad
 mov ebx,mrKTitle
 jmp .title
.cfValidate:
 stdcall JsonGet,[dqContext],[root],cfKId
 stdcall JsonInt,[dqContext],eax
 mov ebx,eax
 stdcall ModNumber,dqId
 cmp eax,ebx
 jne .bad
 stdcall JsonGet,[dqContext],[root],cfKGame
 stdcall JsonInt,[dqContext],eax
 cmp eax,432
 jne .bad
 stdcall JsonGet,[dqContext],[root],cfKClass
 stdcall JsonInt,[dqContext],eax
 mov ebx,6
 cmp [dqIsPack],0
 je .class
 mov ebx,4471
.class:
 cmp eax,ebx
 jne .bad
 mov ebx,cfKName
.title:
 stdcall JsonGet,[dqContext],[root],ebx
 stdcall MrCopy,[dqContext],eax,detailTitle,512
 test eax,eax
 jle .bad
 mov ebx,mrKDescription
 cmp [dqProvider],0
 je .summary
 mov ebx,cfKSummary
.summary:
 stdcall JsonGet,[dqContext],[root],ebx
 stdcall MrCopy,[dqContext],eax,detailSummary,1024
 cmp eax,-1
 je .bad
 mov ebx,dqKDownloads
 cmp [dqProvider],0
 je .downloads
 mov ebx,dqKDownloadCount
.downloads:
 stdcall JsonGet,[dqContext],[root],ebx
 stdcall JsonInt,[dqContext],eax
 test eax,eax
 jns .downloadsValid
 xor eax,eax
.downloadsValid:
 mov [downloads],eax
 invoke lstrcpyA,dqValue,dqUnknown
 cmp [dqProvider],0
 jne .cfAuthor
 stdcall JsonGet,[dqContext],[root],dqKLicense
 stdcall JsonGet,[dqContext],eax,dqKLicenseId
 stdcall MrCopy,[dqContext],eax,dqValue,512
 cinvoke wsprintfA,detailMeta,dqMetaMr,[downloads],dqValue
 stdcall JsonGet,[dqContext],[root],dqKBody
 stdcall MrCopy,[dqContext],eax,dqRaw,131072
 cmp eax,-1
 je .bad
 jmp .plain
.cfAuthor:
 stdcall JsonGet,[dqContext],[root],dqKAuthors
 stdcall JsonAt,[dqContext],eax,0
 stdcall JsonGet,[dqContext],eax,cfKName
 stdcall MrCopy,[dqContext],eax,dqValue,512
 cinvoke wsprintfA,detailMeta,dqMetaCf,[downloads],dqValue
 invoke lstrcpyA,dqUrl,dqCfProject
 stdcall ModAppend,dqUrl,8192,dqId
 stdcall ModAppend,dqUrl,8192,dqDescriptionSuffix
 stdcall DqRequest
 test eax,eax
 jz .bad
 stdcall JsonGet,[dqContext],[dqRoot],cfKData
 stdcall MrCopy,[dqContext],eax,dqRaw,131072
 cmp eax,-1
 je .bad
.plain:
 stdcall DetailPlain,dqRaw,detailBody,131072
 cmp eax,-1
 je .bad
 cmp byte [detailBody],0
 jne .success
 invoke lstrcpyA,detailBody,detailSummary
.success:
 mov [detailReady],1
 mov [result],1
 stdcall SetStatusA,dqReadyStatus
 jmp .done
.bad:
 mov byte [detailBody],0
 stdcall SetStatusA,dqFailureStatus
.done:
 stdcall DqClose
 mov eax,[result]
 ret
endp

; Official home page advertises //search.mcmod.cn/s with GET key in UTF-8.
proc McmodSearchUrl uses ebx,query,destination,capacity
 cmp [capacity],0
 je .zero
 mov edx,[destination]
 mov word [edx],0
 mov ebx,[query]
 cmp byte [ebx],0
 jne .search
 invoke MultiByteToWideChar,65001,8,dqHome,-1,[destination],[capacity]
 test eax,eax
 jz .bad
 mov eax,1
 ret
.search:
 invoke lstrlenA,[query]
 cmp eax,1000
 ja .bad
 stdcall ModUrlEncode,[query],dqEncoded,3072
 cmp eax,-1
 je .bad
 invoke lstrcpyA,dqSearch,dqSearchBase
 stdcall ModAppend,dqSearch,4096,dqEncoded
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,dqSearch,-1,[destination],[capacity]
 test eax,eax
 jz .bad
 mov eax,1
 ret
.bad:
 cmp [capacity],0
 je .zero
 mov edx,[destination]
 mov word [edx],0
.zero:
 xor eax,eax
 ret
endp
