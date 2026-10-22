; Native modpack catalogue. Reuses bounded shared mod records and transport.
section '.pcatd' data readable writeable
packProvider dd 0
packOffset dd 0
packTotal dd 0
packProjectCount dd 0
packFileCount dd 0
packSelectedProject dd -1
packSelectedFile dd -1
pcContext dd 0
pcResponse dd 0
pcRoot dd 0
pcMrSearch db 'https://api.modrinth.com/v2/search?limit=20&query=',0
pcCfSearch db 'https://api.curseforge.com/v1/mods/search?gameId=432&classId=4471&sortField=2&sortOrder=desc&pageSize=20&index=',0
pcMrProject db 'https://api.modrinth.com/v2/project/',0
pcCfProject db 'https://api.curseforge.com/v1/mods/',0
pcFacetParam db '&facets=',0
pcFacets db '[["project_type:modpack"]',0
pcFacetGame db ',["versions:',0
pcFacetLoader db ',["categories:',0
pcFacetEnd db '"]',0
pcClose db ']',0
pcCfQuery db '&searchFilter=',0
pcCfGame db '&gameVersion=',0
pcCfLoader db '&modLoaderType=',0
pcOffsetParam db '&offset=',0
pcMrVersion db '/version?include_changelog=false',0
pcCfVersion db '/files?pageSize=50&index=0',0
pcMrWebsite db 'https://modrinth.com/modpack/',0
pcCfWebsite db 'https://www.curseforge.com/minecraft/modpacks/',0
pcCfBare db 'https://curseforge.com/minecraft/modpacks/',0
pcType db 'modpack',0
pcFmt db '%u',0
pcMrpack db '.mrpack',0
pcZip db '.zip',0
pcSearchStatus db '正在搜索整合包…',0
pcVersionStatus db '正在读取整合包版本…',0
pcReadyStatus db '整合包列表已更新。',0
pcFailureStatus db '整合包请求失败，请检查网络或接口密钥。',0
pcSelectStatus db '请选择整合包及可下载版本。',0
pcDownloadStatus db '正在下载整合包…',0
pcCancelStatus db '整合包任务已取消。',0
packQuery rb 512
packGameVersion rb 64
packLoader rb 16
packProjects rb MP_SIZE*MOD_MAX_PROJECTS
packFiles rb MF_SIZE*MOD_MAX_FILES
pcUrl rb 8192
pcUrlW rw 8192
pcEncoded rb 3072
pcFilter rb 1024
pcValue rb 2048
pcNumber rb 32
pcFilename rb 260

section '.pcatc' code readable executable
 nop
proc PcClose
 stdcall JsonFree,[pcContext]
 mov [pcContext],0
 stdcall FreeMem,[pcResponse]
 mov [pcResponse],0
 ret
endp

proc PcCancelled
 mov eax,[modCancel]
 or eax,[engineCancel]
 ret
endp

proc PcRequest
 stdcall PcClose
 stdcall PcCancelled
 test eax,eax
 jnz .cancel
 invoke MultiByteToWideChar,65001,8,pcUrl,-1,pcUrlW,8192
 test eax,eax
 jz .bad
 cmp [packProvider],0
 jne .curse
 stdcall HttpRequest,pcUrlW,0,mrHeader,0
 jmp .response
.curse:
 stdcall CfMakeHeaders
 test eax,eax
 jz .bad
 stdcall HttpRequest,pcUrlW,0,cfHeadersW,0
 mov [pcResponse],eax
 stdcall CfWipeHeaders
 mov eax,[pcResponse]
.response:
 mov [pcResponse],eax
 test eax,eax
 jz .bad
 cmp [httpStatus],200
 jne .bad
 stdcall PcCancelled
 test eax,eax
 jnz .cancel
 stdcall JsonParse,[pcResponse]
 test eax,eax
 jz .bad
 mov [pcContext],eax
 stdcall JsonRoot,eax
 mov [pcRoot],eax
 mov eax,1
 ret
.cancel:
 stdcall SetStatusA,pcCancelStatus
 jmp .fail
.bad:
 stdcall SetStatusA,pcFailureStatus
.fail:
 stdcall PcClose
 xor eax,eax
 ret
endp

; Optional filters are bounded ASCII components; an empty value means all.
proc PcFilterValid uses esi,source,maximum
 mov esi,[source]
 xor ecx,ecx
.loop:
 mov al,[esi+ecx]
 test al,al
 jz .yes
 inc ecx
 cmp ecx,[maximum]
 jae .no
 cmp al,'0'
 jb .punct
 cmp al,'9'
 jbe .loop
 cmp al,'A'
 jb .punct
 cmp al,'Z'
 jbe .loop
 cmp al,'a'
 jb .punct
 cmp al,'z'
 jbe .loop
.punct:
 cmp al,'.'
 je .loop
 cmp al,'-'
 je .loop
 cmp al,'_'
 je .loop
.no:
 xor eax,eax
 ret
.yes:
 mov eax,1
 ret
endp

proc PcLoaderCode
 cmp byte [packLoader],0
 je .empty
 invoke lstrcmpA,packLoader,mrFabric
 test eax,eax
 jz .fabric
 invoke lstrcmpA,packLoader,mrForge
 test eax,eax
 jz .forge
 invoke lstrcmpA,packLoader,mrNeoForge
 test eax,eax
 jz .neo
 invoke lstrcmpA,packLoader,mrQuilt
 test eax,eax
 jz .quilt
 mov eax,-1
 ret
.empty: xor eax,eax
 ret
.fabric: mov eax,4
 ret
.forge: mov eax,1
 ret
.neo: mov eax,6
 ret
.quilt: mov eax,5
 ret
endp

proc PcAppendFilter uses ebx,base,suffix,value
 cmp byte [packGameVersion],0 ; caller value decides whether to add
 mov ebx,[value]
 cmp byte [ebx],0
 je .yes
 stdcall ModAppend,[base],8192,[suffix]
 test eax,eax
 jz .done
 stdcall ModUrlEncode,[value],pcEncoded,3072
 cmp eax,-1
 je .no
 stdcall ModAppend,[base],8192,pcEncoded
.done: ret
.yes: mov eax,1
 ret
.no: xor eax,eax
 ret
endp

proc PackSearch uses ebx esi edi
 local result:DWORD, list:DWORD, count:DWORD, index:DWORD, item:DWORD, record:DWORD
 mov [packProvider],0
 mov [result],0
 mov [packProjectCount],0
 mov [packFileCount],0
 mov [packSelectedProject],-1
 mov [packSelectedFile],-1
 mov [packTotal],0
 mov edi,packProjects
 xor eax,eax
 mov ecx,MP_SIZE*MOD_MAX_PROJECTS/4
 rep stosd
 stdcall PcFilterValid,packGameVersion,64
 test eax,eax
 jz .bad
 stdcall PcLoaderCode
 cmp eax,-1
 je .bad
 cmp [packOffset],9980
 ja .bad
 cmp [packProvider],1
 ja .bad
 stdcall SetStatusA,pcSearchStatus
 cmp [packProvider],1
 je .curse
 invoke lstrcpyA,pcUrl,pcMrSearch
 stdcall ModUrlEncode,packQuery,pcEncoded,3072
 cmp eax,-1
 je .bad
 stdcall ModAppend,pcUrl,8192,pcEncoded
 invoke lstrcpyA,pcFilter,pcFacets
 cmp byte [packGameVersion],0
 je .loaderFacet
 stdcall ModAppend,pcFilter,1024,pcFacetGame
 stdcall ModAppend,pcFilter,1024,packGameVersion
 stdcall ModAppend,pcFilter,1024,pcFacetEnd
.loaderFacet:
 cmp byte [packLoader],0
 je .facetDone
 stdcall ModAppend,pcFilter,1024,pcFacetLoader
 stdcall ModAppend,pcFilter,1024,packLoader
 stdcall ModAppend,pcFilter,1024,pcFacetEnd
.facetDone:
 stdcall ModAppend,pcFilter,1024,pcClose
 stdcall ModUrlEncode,pcFilter,pcEncoded,3072
 cmp eax,-1
 je .bad
 stdcall ModAppend,pcUrl,8192,pcFacetParam
 stdcall ModAppend,pcUrl,8192,pcEncoded
 stdcall ModAppend,pcUrl,8192,pcOffsetParam
 cinvoke wsprintfA,pcNumber,pcFmt,[packOffset]
 stdcall ModAppend,pcUrl,8192,pcNumber
 jmp .request
.curse:
 invoke lstrcpyA,pcUrl,pcCfSearch
 cinvoke wsprintfA,pcNumber,pcFmt,[packOffset]
 stdcall ModAppend,pcUrl,8192,pcNumber
 stdcall PcAppendFilter,pcUrl,pcCfQuery,packQuery
 test eax,eax
 jz .bad
 stdcall PcAppendFilter,pcUrl,pcCfGame,packGameVersion
 stdcall PcLoaderCode
 test eax,eax
 jz .request
 cinvoke wsprintfA,pcNumber,pcFmt,eax
 stdcall ModAppend,pcUrl,8192,pcCfLoader
 stdcall ModAppend,pcUrl,8192,pcNumber
.request:
 stdcall PcRequest
 test eax,eax
 jz .done
 cmp [packProvider],0
 jne .cfList
 stdcall JsonGet,[pcContext],[pcRoot],mrKTotal
 stdcall JsonInt,[pcContext],eax
 mov [packTotal],eax
 stdcall JsonGet,[pcContext],[pcRoot],mrKHits
 jmp .list
.cfList:
 stdcall JsonGet,[pcContext],[pcRoot],cfKPagination
 stdcall JsonGet,[pcContext],eax,cfKTotal
 stdcall JsonInt,[pcContext],eax
 mov [packTotal],eax
 stdcall JsonGet,[pcContext],[pcRoot],cfKData
.list:
 mov [list],eax
 test eax,eax
 jz .bad
 cmp [packTotal],0
 jge .totalPositive
 mov [packTotal],0
.totalPositive:
 cmp [packTotal],10000
 jbe .count
 mov [packTotal],10000
.count:
 stdcall JsonCount,[pcContext],[list]
 cmp eax,MOD_MAX_PROJECTS
 jbe .bounded
 mov eax,MOD_MAX_PROJECTS
.bounded:
 mov [count],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .success
 stdcall JsonAt,[pcContext],[list],eax
 mov [item],eax
 imul eax,[packProjectCount],MP_SIZE
 add eax,packProjects
 mov [record],eax
 cmp [packProvider],0
 jne .cfItem
 stdcall JsonGet,[pcContext],[item],mrKProjectType
 stdcall MrCopy,[pcContext],eax,pcValue,2048
 invoke lstrcmpA,pcValue,pcType
 test eax,eax
 jnz .next
 stdcall JsonGet,[pcContext],[item],mrKProject
 stdcall MrCopy,[pcContext],eax,[record],64
 stdcall MrValidId,[record]
 test eax,eax
 jz .next
 mov ebx,mrKTitle
 jmp .title
.cfItem:
 stdcall JsonGet,[pcContext],[item],cfKGame
 stdcall JsonInt,[pcContext],eax
 cmp eax,432
 jne .next
 stdcall JsonGet,[pcContext],[item],cfKClass
 stdcall JsonInt,[pcContext],eax
 cmp eax,4471
 jne .next
 stdcall JsonGet,[pcContext],[item],cfKAvailable
 stdcall JsonBool,[pcContext],eax
 cmp eax,1
 jne .next
 stdcall JsonGet,[pcContext],[item],cfKId
 stdcall JsonInt,[pcContext],eax
 test eax,eax
 jle .next
 cinvoke wsprintfA,[record],pcFmt,eax
 mov ebx,cfKName
.title:
 stdcall JsonGet,[pcContext],[item],ebx
 mov ebx,[record]
 add ebx,MP_TITLE
 stdcall MrCopy,[pcContext],eax,ebx,320
 test eax,eax
 jle .next
 mov ebx,mrKDescription
 cmp [packProvider],0
 je .summary
 mov ebx,cfKSummary
.summary:
 stdcall JsonGet,[pcContext],[item],ebx
 mov ebx,[record]
 add ebx,MP_DESC
 stdcall MrCopy,[pcContext],eax,ebx,512
 cmp [packProvider],0
 jne .cfWebsite
 mov ebx,[record]
 lea edi,[ebx+MP_URL]
 invoke lstrcpyA,edi,pcMrWebsite
 stdcall ModAppend,edi,512,[record]
 jmp .commit
.cfWebsite:
 stdcall JsonGet,[pcContext],[item],cfKLinks
 stdcall JsonGet,[pcContext],eax,cfKWebsite
 mov ebx,[record]
 lea edi,[ebx+MP_URL]
 stdcall MrCopy,[pcContext],eax,edi,512
 stdcall CfPrefix,edi,pcCfWebsite
 test eax,eax
 jnz .distribution
 stdcall CfPrefix,edi,pcCfBare
 test eax,eax
 jnz .distribution
 mov byte [edi],0
.distribution:
 stdcall JsonGet,[pcContext],[item],cfKDistribution
 stdcall JsonBool,[pcContext],eax
 test eax,eax
 jnz .commit
 mov ebx,[record]
 mov dword [ebx+MP_BLOCKED],1
.commit:
 inc [packProjectCount]
.next:
 inc [index]
 jmp .loop
.success:
 stdcall SetStatusA,pcReadyStatus
 mov [result],1
 jmp .done
.bad:
 mov [packProjectCount],0
 stdcall SetStatusA,pcFailureStatus
.done:
 stdcall PcClose
 mov eax,[result]
 ret
endp

; Treat archive basenames with the same Windows safety rules as mod JARs.
proc PcArchiveFilename uses ebx esi edi,name,provider
 invoke lstrlenA,[name]
 cmp eax,5
 jb .no
 cmp eax,250
 ja .no
 mov ebx,eax
 mov esi,[name]
 mov edi,pcFilename
 mov ecx,eax
 rep movsb
 mov byte [edi],0
 mov esi,pcZip
 mov ecx,4
 cmp [provider],0
 jne .suffix
 mov esi,pcMrpack
 mov ecx,7
.suffix:
 cmp ebx,ecx
 jbe .no
 mov edi,pcFilename
 add edi,ebx
 sub edi,ecx
 push edi
 stdcall CfEqualFold,edi,esi
 pop edi
 test eax,eax
 jz .no
 mov dword [edi],'.jar'
 mov byte [edi+4],0
 stdcall ModSafeFilename,pcFilename
 ret
.no:
 xor eax,eax
 ret
endp

proc PcCompatible uses ebx esi,object
 local list:DWORD,count:DWORD,index:DWORD,matchedGame:DWORD,matchedLoader:DWORD
 mov [matchedGame],1
 mov [matchedLoader],1
 cmp byte [packGameVersion],0
 je .loader
 mov [matchedGame],0
.loader:
 cmp byte [packLoader],0
 je .get
 mov [matchedLoader],0
.get:
 cmp [packProvider],0
 jne .curse
 stdcall JsonGet,[pcContext],[object],mrKVersions
 cmp byte [packGameVersion],0
 je .mrLoader
 stdcall MrHasValue,[pcContext],eax,packGameVersion
 mov [matchedGame],eax
.mrLoader:
 cmp byte [packLoader],0
 je .done
 stdcall JsonGet,[pcContext],[object],mrKLoaders
 stdcall MrHasValue,[pcContext],eax,packLoader
 mov [matchedLoader],eax
 jmp .done
.curse:
 stdcall JsonGet,[pcContext],[object],cfKGameVersions
 mov [list],eax
 stdcall JsonCount,[pcContext],eax
 cmp eax,1024
 ja .no
 mov [count],eax
 mov [index],0
.tags:
 mov eax,[index]
 cmp eax,[count]
 jae .done
 stdcall JsonAt,[pcContext],[list],eax
 stdcall MrCopy,[pcContext],eax,pcValue,2048
 invoke lstrcmpA,pcValue,packGameVersion
 test eax,eax
 jnz .tagLoader
 mov [matchedGame],1
.tagLoader:
 stdcall CfEqualFold,pcValue,packLoader
 test eax,eax
 jz .tagNext
 mov [matchedLoader],1
.tagNext:
 inc [index]
 jmp .tags
.done:
 mov eax,[matchedGame]
 and eax,[matchedLoader]
 ret
.no:
 xor eax,eax
 ret
endp

proc PackVersions uses ebx esi edi
 local result:DWORD,list:DWORD,count:DWORD,index:DWORD,item:DWORD,record:DWORD,project:DWORD,files:DWORD,fileCount:DWORD,fileIndex:DWORD,file:DWORD,hashes:DWORD,hashCount:DWORD,hashIndex:DWORD,hash:DWORD
 mov [result],0
 mov [packFileCount],0
 mov [packSelectedFile],-1
 mov edi,packFiles
 xor eax,eax
 mov ecx,MF_SIZE*MOD_MAX_FILES/4
 rep stosd
 mov eax,[packSelectedProject]
 cmp eax,[packProjectCount]
 jae .bad
 imul eax,MP_SIZE
 add eax,packProjects
 mov [project],eax
 stdcall PcFilterValid,packGameVersion,64
 test eax,eax
 jz .bad
 stdcall PcLoaderCode
 cmp eax,-1
 je .bad
 cmp [packProvider],0
 jne .cfUrl
 stdcall MrValidId,[project]
 test eax,eax
 jz .bad
 invoke lstrcpyA,pcUrl,pcMrProject
 stdcall ModAppend,pcUrl,8192,[project]
 stdcall ModAppend,pcUrl,8192,pcMrVersion
 jmp .request
.cfUrl:
 cmp [packProvider],1
 jne .bad
 stdcall ModNumber,[project]
 test eax,eax
 jz .bad
 invoke lstrcpyA,pcUrl,pcCfProject
 stdcall ModAppend,pcUrl,8192,[project]
 stdcall ModAppend,pcUrl,8192,pcCfVersion
 stdcall PcAppendFilter,pcUrl,pcCfGame,packGameVersion
 stdcall PcLoaderCode
 test eax,eax
 jz .request
 cinvoke wsprintfA,pcNumber,pcFmt,eax
 stdcall ModAppend,pcUrl,8192,pcCfLoader
 stdcall ModAppend,pcUrl,8192,pcNumber
.request:
 stdcall SetStatusA,pcVersionStatus
 stdcall PcRequest
 test eax,eax
 jz .done
 mov eax,[pcRoot]
 cmp [packProvider],0
 je .list
 stdcall JsonGet,[pcContext],eax,cfKData
.list:
 mov [list],eax
 stdcall JsonCount,[pcContext],eax
 cmp eax,20000
 ja .bad
 mov [count],eax
 mov [index],0
.loop:
 mov eax,[index]
 cmp eax,[count]
 jae .success
 cmp [packFileCount],MOD_MAX_FILES
 jae .success
 stdcall JsonAt,[pcContext],[list],eax
 mov [item],eax
 stdcall PcCompatible,eax
 test eax,eax
 jz .next
 imul eax,[packFileCount],MF_SIZE
 add eax,packFiles
 mov [record],eax
 cmp [packProvider],0
 jne .cfItem
 stdcall JsonGet,[pcContext],[item],mrKFiles
 mov [files],eax
 stdcall JsonCount,[pcContext],eax
 cmp eax,64
 ja .next
 mov [fileCount],eax
 mov [fileIndex],0
.fileLoop:
 mov eax,[fileIndex]
 cmp eax,[fileCount]
 jae .next
 stdcall JsonAt,[pcContext],[files],eax
 mov [file],eax
 stdcall JsonGet,[pcContext],eax,mrKPrimary
 stdcall JsonBool,[pcContext],eax
 cmp eax,1
 jne .fileNext
 stdcall JsonGet,[pcContext],[file],mrKFilename
 mov ebx,[record]
 add ebx,MF_NAME
 stdcall MrCopy,[pcContext],eax,ebx,256
 test eax,eax
 jle .fileNext
 stdcall PcArchiveFilename,ebx,0
 test eax,eax
 jz .fileNext
 stdcall JsonGet,[pcContext],[file],mrKUrl
 mov ebx,[record]
 add ebx,MF_URL
 stdcall MrCopy,[pcContext],eax,ebx,2048
 stdcall ModTrustedUrl,ebx,0
 test eax,eax
 jz .fileNext
 stdcall JsonGet,[pcContext],[file],mrKHashes
 stdcall JsonGet,[pcContext],eax,mrKSha1
 mov ebx,[record]
 add ebx,MF_SHA1
 stdcall MrCopy,[pcContext],eax,ebx,64
 stdcall ModIsSha1,ebx
 test eax,eax
 jz .fileNext
 stdcall JsonGet,[pcContext],[item],mrKId
 stdcall MrCopy,[pcContext],eax,[record],64
 stdcall MrValidId,[record]
 test eax,eax
 jz .next
 stdcall JsonGet,[pcContext],[item],mrKNumber
 mov ebx,[record]
 add ebx,MF_TITLE
 stdcall MrCopy,[pcContext],eax,ebx,320
 test eax,eax
 jle .next
 stdcall JsonGet,[pcContext],[item],mrKType
 stdcall MrCopy,[pcContext],eax,pcValue,2048
 mov ebx,[record]
 mov dword [ebx+MF_KIND],3
 invoke lstrcmpA,pcValue,mrRelease
 test eax,eax
 jnz .mrBeta
 mov dword [ebx+MF_KIND],1
 jmp .commit
.mrBeta:
 invoke lstrcmpA,pcValue,mrBeta
 test eax,eax
 jnz .commit
 mov dword [ebx+MF_KIND],2
 jmp .commit
.fileNext:
 inc [fileIndex]
 jmp .fileLoop
.cfItem:
 stdcall JsonGet,[pcContext],[item],cfKGame
 stdcall JsonInt,[pcContext],eax
 cmp eax,432
 jne .next
 stdcall JsonGet,[pcContext],[item],cfKMod
 stdcall JsonInt,[pcContext],eax
 mov ebx,eax
 stdcall ModNumber,[project]
 cmp eax,ebx
 jne .next
 stdcall JsonGet,[pcContext],[item],cfKAvailable
 stdcall JsonBool,[pcContext],eax
 cmp eax,1
 jne .next
 stdcall JsonGet,[pcContext],[item],cfKEarly
 stdcall JsonBool,[pcContext],eax
 cmp eax,1
 je .next
 stdcall JsonGet,[pcContext],[item],cfKServer
 stdcall JsonBool,[pcContext],eax
 cmp eax,1
 je .next
 stdcall JsonGet,[pcContext],[item],cfKFileName
 mov ebx,[record]
 add ebx,MF_NAME
 stdcall MrCopy,[pcContext],eax,ebx,256
 test eax,eax
 jle .next
 stdcall PcArchiveFilename,ebx,1
 test eax,eax
 jz .next
 stdcall JsonGet,[pcContext],[item],cfKId
 stdcall JsonInt,[pcContext],eax
 test eax,eax
 jle .next
 cinvoke wsprintfA,[record],pcFmt,eax
 stdcall JsonGet,[pcContext],[item],cfKDisplay
 mov ebx,[record]
 add ebx,MF_TITLE
 stdcall MrCopy,[pcContext],eax,ebx,320
 test eax,eax
 jle .next
 stdcall JsonGet,[pcContext],[item],cfKRelease
 stdcall JsonInt,[pcContext],eax
 cmp eax,1
 jb .next
 cmp eax,3
 ja .next
 mov ebx,[record]
 mov [ebx+MF_KIND],eax
 mov esi,[project]
 mov eax,[esi+MP_BLOCKED]
 mov [ebx+MF_BLOCKED],eax
 stdcall JsonGet,[pcContext],[item],cfKDownload
 lea ebx,[ebx+MF_URL]
 stdcall MrCopy,[pcContext],eax,ebx,2048
 stdcall ModTrustedUrl,ebx,1
 test eax,eax
 jz .blocked
 mov ebx,[record]
 cmp dword [ebx+MF_BLOCKED],0
 je .cfHashes
.blocked:
 mov ebx,[record]
 mov byte [ebx+MF_URL],0
 mov dword [ebx+MF_BLOCKED],1
.cfHashes:
 stdcall JsonGet,[pcContext],[item],cfKHashes
 mov [hashes],eax
 stdcall JsonCount,[pcContext],eax
 cmp eax,32
 ja .noHash
 mov [hashCount],eax
 mov [hashIndex],0
.hashLoop:
 mov eax,[hashIndex]
 cmp eax,[hashCount]
 jae .noHash
 stdcall JsonAt,[pcContext],[hashes],eax
 mov [hash],eax
 stdcall JsonGet,[pcContext],eax,cfKAlgo
 stdcall JsonInt,[pcContext],eax
 cmp eax,1
 jne .hashNext
 stdcall JsonGet,[pcContext],[hash],cfKValue
 mov ebx,[record]
 add ebx,MF_SHA1
 stdcall MrCopy,[pcContext],eax,ebx,64
 stdcall ModIsSha1,ebx
 test eax,eax
 jnz .commit
.hashNext:
 inc [hashIndex]
 jmp .hashLoop
.noHash:
 mov ebx,[record]
 mov byte [ebx+MF_SHA1],0
 mov dword [ebx+MF_BLOCKED],1
.commit:
 inc [packFileCount]
.next:
 inc [index]
 jmp .loop
.success:
 mov [result],1
 stdcall SetStatusA,pcReadyStatus
 jmp .done
.bad:
 mov [packFileCount],0
 stdcall SetStatusA,pcFailureStatus
.done:
 stdcall PcClose
 mov eax,[result]
 ret
endp

proc PackDownload uses ebx,index,destination
 mov eax,[index]
 cmp eax,[packFileCount]
 jae .bad
 imul eax,MF_SIZE
 lea ebx,[packFiles+eax]
 cmp dword [ebx+MF_BLOCKED],0
 jne .bad
 lea eax,[ebx+MF_URL]
 stdcall ModTrustedUrl,eax,[packProvider]
 test eax,eax
 jz .bad
 lea eax,[ebx+MF_SHA1]
 stdcall ModIsSha1,eax
 test eax,eax
 jz .bad
 stdcall PcCancelled
 test eax,eax
 jnz .bad
 stdcall SetStatusA,pcDownloadStatus
 lea eax,[ebx+MF_URL]
 lea edx,[ebx+MF_SHA1]
 stdcall HttpDownload,eax,[destination],edx
 ret
.bad:
 stdcall SetStatusA,pcSelectStatus
 xor eax,eax
 ret
endp
