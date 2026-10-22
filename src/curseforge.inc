; Original OFFSET x86 assembly provider for the official CurseForge REST API.
; The API key is sent only to api.curseforge.com through nonredirecting requests.
section '.cfd' data readable writeable
cfResponse dd 0
cfContext dd 0
cfRoot dd 0
cfList dd 0
cfLoaderCode dd 0
cfLoaderTag dd 0
cfBase db 'https://api.curseforge.com/v1/mods/',0
cfSearchPath db 'search?gameId=432&classId=6&sortField=2&sortOrder=desc&pageSize=20&index=',0
cfSearchFilter db '&searchFilter=',0
cfGameFilter db '&gameVersion=',0
cfFilePath db '/files?gameVersion=',0
cfLoaderFilter db '&modLoaderType=',0
cfFileLimit db '&pageSize=50&index=0',0
cfHeaderPrefix db 'Accept: application/json',13,10,'User-Agent: OFFSET/0.9.0 (native Minecraft mod browser)',13,10,'x-api-key: ',0
cfCrLf db 13,10,0
cfFormat db '%u',0
cfWebsitePrefix db 'https://www.curseforge.com/minecraft/mc-mods/',0
cfWebsiteBare db 'https://curseforge.com/minecraft/mc-mods/',0
cfFabric db 'fabric',0
cfForge db 'forge',0
cfNeoForge db 'neoforge',0
cfQuilt db 'quilt',0
cfFabricTag db 'Fabric',0
cfForgeTag db 'Forge',0
cfNeoForgeTag db 'NeoForge',0
cfQuiltTag db 'Quilt',0
cfKData db 'data',0
cfKId db 'id',0
cfKGame db 'gameId',0
cfKClass db 'classId',0
cfKName db 'name',0
cfKSummary db 'summary',0
cfKLinks db 'links',0
cfKWebsite db 'websiteUrl',0
cfKDistribution db 'allowModDistribution',0
cfKAvailable db 'isAvailable',0
cfKPagination db 'pagination',0
cfKTotal db 'totalCount',0
cfKMod db 'modId',0
cfKFileName db 'fileName',0
cfKDisplay db 'displayName',0
cfKRelease db 'releaseType',0
cfKDownload db 'downloadUrl',0
cfKHashes db 'hashes',0
cfKAlgo db 'algo',0
cfKValue db 'value',0
cfKGameVersions db 'gameVersions',0
cfKDeps db 'dependencies',0
cfKRelation db 'relationType',0
cfKEarly db 'isEarlyAccessContent',0
cfKServer db 'isServerPack',0
cfDepSeparator db ', ',0
cfMsgKey db '请先在模组页面填写并保存你自己的 CurseForge API Key。',0
cfMsgBadKey db 'CurseForge API Key 格式无效，请勿包含空格、换行或控制字符。',0
cfMsgAuth db 'CurseForge 拒绝了 API Key，请检查密钥是否有效及应用权限。',0
cfMsgRate db 'CurseForge 请求过于频繁，请稍后重试。',0
cfMsgNetwork db 'CurseForge 请求失败，请检查网络或稍后重试。',0
cfMsgFormat db 'CurseForge 返回的数据无效或超过支持范围。',0
cfMsgFilter db '请填写 Minecraft 版本并选择支持的模组加载器。',0
cfMsgSearch db '正在搜索 CurseForge 模组…',0
cfMsgVersions db '正在读取 CurseForge 的兼容文件…',0
cfMsgFound db 'CurseForge 搜索完成，选择项目查看兼容版本。',0
cfMsgNoProjects db 'CurseForge 没有找到符合条件的模组。',0
cfMsgFiles db '已读取兼容文件。下载受限的文件请使用项目官网入口。',0
cfMsgNoFiles db '该项目没有符合当前游戏版本与加载器的可用文件。',0
cfMsgSelect db '请先选择一个 CurseForge 项目。',0
cfMsgCancel db '模组请求已取消。',0

; Trailing BSS does not add zero bytes to the portable executable.
cfUrl rb 8192
cfWideUrl rw 8192
cfEncodedQuery rb 1536
cfEncodedGame rb 256
cfHeadersA rb 1024
cfHeadersW rw 1024
cfNumberText rb 32
cfValue rb 2048

section '.cfc' code readable executable

proc CfPrefix uses esi edi,source,prefix
 mov esi,[source]
 mov edi,[prefix]
 .loop:
 mov al,[edi]
 test al,al
 jz .yes
 cmp al,[esi]
 jne .no
 inc esi
 inc edi
 jmp .loop
 .yes: mov eax,1
 ret
 .no: xor eax,eax
 ret
endp

proc CfEqualFold uses esi edi,first,second
 mov esi,[first]
 mov edi,[second]
 .loop:
 mov al,[esi]
 mov dl,[edi]
 cmp al,'A'
 jb .first
 cmp al,'Z'
 ja .first
 add al,32
 .first:
 cmp dl,'A'
 jb .second
 cmp dl,'Z'
 ja .second
 add dl,32
 .second:
 cmp al,dl
 jne .no
 test al,al
 jz .yes
 inc esi
 inc edi
 jmp .loop
 .yes: mov eax,1
 ret
 .no: xor eax,eax
 ret
endp

proc CfCopy ctx,token,dest,capacity
 local required:DWORD
 stdcall JsonString,[ctx],[token],[dest],[capacity]
 cmp eax,[capacity]
 jae .bad
 test eax,eax
 jz .bad
 mov [required],eax
 invoke lstrlenA,[dest]
 cmp eax,[required]
 jne .bad
 mov eax,1
 ret
 .bad:
 mov edx,[dest]
 mov byte [edx],0
 xor eax,eax
 ret
endp

proc CfWipeHeaders uses edi
 mov edi,cfHeadersA
 xor eax,eax
 mov ecx,1024/4
 rep stosd
 mov edi,cfHeadersW
 mov ecx,2048/4
 rep stosd
 ret
endp

proc CfCleanup uses edi ebx
 cmp [cfContext],0
 je .response
 stdcall JsonFree,[cfContext]
 mov [cfContext],0
 .response:
 cmp [cfResponse],0
 je .headers
 invoke lstrlenA,[cfResponse]
 mov ecx,eax
 inc ecx
 mov edi,[cfResponse]
 xor eax,eax
 rep stosb
 stdcall FreeMem,[cfResponse]
 mov [cfResponse],0
 .headers:
 stdcall CfWipeHeaders
 mov [cfRoot],0
 ret
endp

; Validate every byte before building a sensitive request header.
proc CfMakeHeaders uses esi
 stdcall CfWipeHeaders
 mov esi,curseApiKey
 xor ecx,ecx
 .check:
 mov al,[esi+ecx]
 test al,al
 jz .length
 cmp ecx,255
 jae .invalid
 cmp al,33
 jb .invalid
 cmp al,126
 ja .invalid
 inc ecx
 jmp .check
 .length:
 test ecx,ecx
 jz .empty
 invoke lstrcpyA,cfHeadersA,cfHeaderPrefix
 stdcall ModAppend,cfHeadersA,1024,curseApiKey
 test eax,eax
 jz .invalid
 stdcall ModAppend,cfHeadersA,1024,cfCrLf
 test eax,eax
 jz .invalid
 invoke MultiByteToWideChar,65001,8,cfHeadersA,-1,cfHeadersW,1024
 test eax,eax
 jz .invalid
 mov eax,1
 ret
 .empty:
 stdcall SetStatusA,cfMsgKey
 xor eax,eax
 ret
 .invalid:
 stdcall CfWipeHeaders
 stdcall SetStatusA,cfMsgBadKey
 xor eax,eax
 ret
endp

proc CfFilters
 mov [cfLoaderCode],0
 mov [cfLoaderTag],0
 invoke lstrcmpA,modLoader,cfFabric
 test eax,eax
 jz .fabric
 invoke lstrcmpA,modLoader,cfForge
 test eax,eax
 jz .forge
 invoke lstrcmpA,modLoader,cfNeoForge
 test eax,eax
 jz .neo
 invoke lstrcmpA,modLoader,cfQuilt
 test eax,eax
 jz .quilt
 jmp .bad
 .fabric:
 mov [cfLoaderCode],4
 mov [cfLoaderTag],cfFabricTag
 jmp .game
 .forge:
 mov [cfLoaderCode],1
 mov [cfLoaderTag],cfForgeTag
 jmp .game
 .neo:
 mov [cfLoaderCode],6
 mov [cfLoaderTag],cfNeoForgeTag
 jmp .game
 .quilt:
 mov [cfLoaderCode],5
 mov [cfLoaderTag],cfQuiltTag
 .game:
 cmp byte [modGameVersion],0
 je .bad
 invoke lstrlenA,modGameVersion
 cmp eax,63
 ja .bad
 stdcall ModUrlEncode,modGameVersion,cfEncodedGame,256
 cmp eax,-1
 je .bad
 mov eax,1
 ret
 .bad:
 stdcall SetStatusA,cfMsgFilter
 xor eax,eax
 ret
endp

proc CfRequest
 stdcall CfCleanup
 cmp [modCancel],0
 jne .cancel
 stdcall CfMakeHeaders
 test eax,eax
 jz .bad
 invoke MultiByteToWideChar,65001,8,cfUrl,-1,cfWideUrl,8192
 test eax,eax
 jz .format
 stdcall HttpRequest,cfWideUrl,0,cfHeadersW,0
 mov [cfResponse],eax
 stdcall CfWipeHeaders
 cmp [modCancel],0
 jne .cancel
 cmp [httpStatus],401
 je .auth
 cmp [httpStatus],403
 je .auth
 cmp [httpStatus],429
 je .rate
 cmp [httpStatus],200
 jne .network
 cmp [cfResponse],0
 je .network
 stdcall JsonParse,[cfResponse]
 test eax,eax
 jz .format
 mov [cfContext],eax
 stdcall JsonRoot,eax
 mov [cfRoot],eax
 mov eax,1
 ret
 .auth: stdcall SetStatusA,cfMsgAuth
 jmp .bad
 .rate: stdcall SetStatusA,cfMsgRate
 jmp .bad
 .network: stdcall SetStatusA,cfMsgNetwork
 jmp .bad
 .format: stdcall SetStatusA,cfMsgFormat
 jmp .bad
 .cancel: stdcall SetStatusA,cfMsgCancel
 .bad:
 stdcall CfCleanup
 xor eax,eax
 ret
endp

proc CurseSearch uses ebx esi edi
 locals
  idx dd ?
  count dd ?
  object dd ?
  record dd ?
  result dd ?
 endl
 mov [result],0
 mov [modProjectCount],0
 mov [modFileCount],0
 mov [modTotal],0
 mov [modSelectedProject],-1
 mov [modSelectedFile],-1
 mov edi,modProjects
 xor eax,eax
 mov ecx,MP_SIZE*MOD_MAX_PROJECTS/4
 rep stosd
 mov edi,modFiles
 mov ecx,MF_SIZE*MOD_MAX_FILES/4
 rep stosd
 stdcall CfMakeHeaders
 test eax,eax
 jz .cfEnd
 stdcall CfWipeHeaders
 stdcall CfFilters
 test eax,eax
 jz .cfEnd
 cmp [modOffset],9980
 ja .format
 stdcall ModUrlEncode,modQuery,cfEncodedQuery,1536
 cmp eax,-1
 je .format
 invoke lstrcpyA,cfUrl,cfBase
 stdcall ModAppend,cfUrl,8192,cfSearchPath
 test eax,eax
 jz .format
 cinvoke wsprintfA,cfNumberText,cfFormat,[modOffset]
 stdcall ModAppend,cfUrl,8192,cfNumberText
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfSearchFilter
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfEncodedQuery
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfGameFilter
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfEncodedGame
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfLoaderFilter
 test eax,eax
 jz .format
 cinvoke wsprintfA,cfNumberText,cfFormat,[cfLoaderCode]
 stdcall ModAppend,cfUrl,8192,cfNumberText
 test eax,eax
 jz .format
 stdcall SetStatusA,cfMsgSearch
 stdcall CfRequest
 test eax,eax
 jz .cfEnd
 stdcall JsonGet,[cfContext],[cfRoot],cfKPagination
 stdcall JsonGet,[cfContext],eax,cfKTotal
 stdcall JsonInt,[cfContext],eax
 cmp eax,0
 jle .list
 cmp eax,10000
 jbe .total
 mov eax,10000
 .total: mov [modTotal],eax
 .list:
 stdcall JsonGet,[cfContext],[cfRoot],cfKData
 mov [cfList],eax
 test eax,eax
 jz .format
 stdcall JsonCount,[cfContext],eax
 cmp eax,MOD_MAX_PROJECTS
 jbe .count
 mov eax,MOD_MAX_PROJECTS
 .count: mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .okay
 cmp [modCancel],0
 jne .cancel
 stdcall JsonAt,[cfContext],[cfList],eax
 mov [object],eax
 mov eax,[modProjectCount]
 imul eax,MP_SIZE
 add eax,modProjects
 mov [record],eax
 mov edi,eax
 xor eax,eax
 mov ecx,MP_SIZE/4
 rep stosd
 stdcall JsonGet,[cfContext],[object],cfKGame
 stdcall JsonInt,[cfContext],eax
 cmp eax,432
 jne .next
 stdcall JsonGet,[cfContext],[object],cfKClass
 stdcall JsonInt,[cfContext],eax
 cmp eax,6
 jne .next
 stdcall JsonGet,[cfContext],[object],cfKAvailable
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 jne .next
 stdcall JsonGet,[cfContext],[object],cfKId
 stdcall JsonInt,[cfContext],eax
 test eax,eax
 jle .next
 mov ebx,[record]
 cinvoke wsprintfA,ebx,cfFormat,eax
 stdcall JsonGet,[cfContext],[object],cfKName
 mov edx,[record]
 add edx,MP_TITLE
 stdcall CfCopy,[cfContext],eax,edx,320
 test eax,eax
 jz .next
 stdcall JsonGet,[cfContext],[object],cfKSummary
 mov edx,[record]
 add edx,MP_DESC
 stdcall CfCopy,[cfContext],eax,edx,512
 stdcall JsonGet,[cfContext],[object],cfKLinks
 stdcall JsonGet,[cfContext],eax,cfKWebsite
 mov edx,[record]
 add edx,MP_URL
 stdcall CfCopy,[cfContext],eax,edx,512
 mov ebx,[record]
 lea edx,[ebx+MP_URL]
 stdcall CfPrefix,edx,cfWebsitePrefix
 test eax,eax
 jnz .website
 lea edx,[ebx+MP_URL]
 stdcall CfPrefix,edx,cfWebsiteBare
 test eax,eax
 jnz .website
 mov byte [ebx+MP_URL],0
 .website:
 stdcall JsonGet,[cfContext],[object],cfKDistribution
 stdcall JsonBool,[cfContext],eax
 test eax,eax
 jnz .commit
 mov ebx,[record]
 mov dword [ebx+MP_BLOCKED],1
 .commit: inc [modProjectCount]
 .next:
 inc [idx]
 jmp .loop
 .okay:
 mov [result],1
 mov eax,cfMsgFound
 cmp [modProjectCount],0
 jne .status
 mov eax,cfMsgNoProjects
 .status: stdcall SetStatusA,eax
 jmp .cfEnd
 .cancel: stdcall SetStatusA,cfMsgCancel
 mov [modProjectCount],0
 jmp .cfEnd
 .format: stdcall SetStatusA,cfMsgFormat
 mov [modProjectCount],0
 .cfEnd:
 stdcall CfCleanup
 mov eax,[result]
 ret
endp

; File compatibility requires exact game-version and loader tags from API data.
proc CfCompatible uses ebx esi edi,object
 locals
  list dd ?
  count dd ?
  idx dd ?
  matchedGame dd ?
  matchedLoader dd ?
 endl
 mov [matchedGame],0
 mov [matchedLoader],0
 stdcall JsonGet,[cfContext],[object],cfKGameVersions
 mov [list],eax
 stdcall JsonCount,[cfContext],eax
 cmp eax,1024
 ja .no
 mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .done
 stdcall JsonAt,[cfContext],[list],eax
 stdcall CfCopy,[cfContext],eax,cfValue,2048
 test eax,eax
 jz .next
 invoke lstrcmpA,cfValue,modGameVersion
 test eax,eax
 jnz .loader
 mov [matchedGame],1
 .loader:
 stdcall CfEqualFold,cfValue,[cfLoaderTag]
 test eax,eax
 jz .next
 mov [matchedLoader],1
 .next: inc [idx]
 jmp .loop
 .done:
 mov eax,[matchedGame]
 and eax,[matchedLoader]
 ret
 .no: xor eax,eax
 ret
endp

proc CfDependencies uses ebx esi edi,object,record
 locals
  list dd ?
  count dd ?
  idx dd ?
  dependency dd ?
 endl
 stdcall JsonGet,[cfContext],[object],cfKDeps
 mov [list],eax
 stdcall JsonCount,[cfContext],eax
 cmp eax,4096
 jbe .count
 mov eax,4096
 .count: mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .done
 stdcall JsonAt,[cfContext],[list],eax
 mov [dependency],eax
 stdcall JsonGet,[cfContext],eax,cfKRelation
 stdcall JsonInt,[cfContext],eax
 cmp eax,3
 jne .next
 stdcall JsonGet,[cfContext],[dependency],cfKMod
 stdcall JsonInt,[cfContext],eax
 test eax,eax
 jle .next
 mov ebx,[record]
 inc dword [ebx+MF_REQUIRED]
 cinvoke wsprintfA,cfNumberText,cfFormat,eax
 cmp byte [ebx+MF_DEPS],0
 je .number
 lea edx,[ebx+MF_DEPS]
 stdcall ModAppend,edx,512,cfDepSeparator
 test eax,eax
 jz .next
 .number:
 lea edx,[ebx+MF_DEPS]
 stdcall ModAppend,edx,512,cfNumberText
 .next: inc [idx]
 jmp .loop
 .done: ret
endp

proc CurseVersions uses ebx esi edi
 locals
  idx dd ?
  count dd ?
  object dd ?
  record dd ?
  project dd ?
  projectId dd ?
  hashes dd ?
  hashCount dd ?
  hashIndex dd ?
  hashObject dd ?
  result dd ?
 endl
 mov [result],0
 mov [modFileCount],0
 mov [modSelectedFile],-1
 mov edi,modFiles
 xor eax,eax
 mov ecx,MF_SIZE*MOD_MAX_FILES/4
 rep stosd
 mov eax,[modSelectedProject]
 cmp eax,[modProjectCount]
 jae .select
 imul eax,MP_SIZE
 add eax,modProjects
 mov [project],eax
 stdcall ModNumber,eax
 test eax,eax
 jz .select
 mov [projectId],eax
 stdcall CfMakeHeaders
 test eax,eax
 jz .cfEnd
 stdcall CfWipeHeaders
 stdcall CfFilters
 test eax,eax
 jz .cfEnd
 invoke lstrcpyA,cfUrl,cfBase
 stdcall ModAppend,cfUrl,8192,[project]
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfFilePath
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfEncodedGame
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfLoaderFilter
 test eax,eax
 jz .format
 cinvoke wsprintfA,cfNumberText,cfFormat,[cfLoaderCode]
 stdcall ModAppend,cfUrl,8192,cfNumberText
 test eax,eax
 jz .format
 stdcall ModAppend,cfUrl,8192,cfFileLimit
 test eax,eax
 jz .format
 stdcall SetStatusA,cfMsgVersions
 stdcall CfRequest
 test eax,eax
 jz .cfEnd
 stdcall JsonGet,[cfContext],[cfRoot],cfKData
 mov [cfList],eax
 test eax,eax
 jz .format
 stdcall JsonCount,[cfContext],eax
 cmp eax,50
 jbe .count
 mov eax,50
 .count: mov [count],eax
 mov [idx],0
 .loop:
 mov eax,[idx]
 cmp eax,[count]
 jae .okay
 cmp [modFileCount],MOD_MAX_FILES
 jae .okay
 cmp [modCancel],0
 jne .cancel
 stdcall JsonAt,[cfContext],[cfList],eax
 mov [object],eax
 stdcall JsonGet,[cfContext],eax,cfKGame
 stdcall JsonInt,[cfContext],eax
 cmp eax,432
 jne .next
 stdcall JsonGet,[cfContext],[object],cfKMod
 stdcall JsonInt,[cfContext],eax
 cmp eax,[projectId]
 jne .next
 stdcall JsonGet,[cfContext],[object],cfKAvailable
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 jne .next
 stdcall JsonGet,[cfContext],[object],cfKEarly
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 je .next
 stdcall JsonGet,[cfContext],[object],cfKServer
 stdcall JsonBool,[cfContext],eax
 cmp eax,1
 je .next
 stdcall CfCompatible,[object]
 test eax,eax
 jz .next
 mov eax,[modFileCount]
 imul eax,MF_SIZE
 add eax,modFiles
 mov [record],eax
 mov edi,eax
 xor eax,eax
 mov ecx,MF_SIZE/4
 rep stosd
 stdcall JsonGet,[cfContext],[object],cfKId
 stdcall JsonInt,[cfContext],eax
 test eax,eax
 jle .next
 cinvoke wsprintfA,[record],cfFormat,eax
 stdcall JsonGet,[cfContext],[object],cfKFileName
 mov edx,[record]
 add edx,MF_NAME
 stdcall CfCopy,[cfContext],eax,edx,256
 test eax,eax
 jz .next
 mov edx,[record]
 add edx,MF_NAME
 stdcall ModSafeFilename,edx
 test eax,eax
 jz .next
 stdcall JsonGet,[cfContext],[object],cfKDisplay
 mov edx,[record]
 add edx,MF_TITLE
 stdcall CfCopy,[cfContext],eax,edx,320
 test eax,eax
 jnz .title
 mov ebx,[record]
 lea eax,[ebx+MF_NAME]
 lea edx,[ebx+MF_TITLE]
 invoke lstrcpyA,edx,eax
 .title:
 stdcall JsonGet,[cfContext],[object],cfKRelease
 stdcall JsonInt,[cfContext],eax
 cmp eax,1
 jb .next
 cmp eax,3
 ja .next
 mov ebx,[record]
 mov [ebx+MF_KIND],eax
 mov esi,[project]
 mov eax,[esi+MP_BLOCKED]
 mov [ebx+MF_BLOCKED],eax
 stdcall JsonGet,[cfContext],[object],cfKDownload
 lea edx,[ebx+MF_URL]
 stdcall CfCopy,[cfContext],eax,edx,2048
 test eax,eax
 jz .blockedUrl
 mov edx,[record]
 add edx,MF_URL
 stdcall ModTrustedUrl,edx,1
 test eax,eax
 jz .blockedUrl
 mov ebx,[record]
 cmp dword [ebx+MF_BLOCKED],0
 je .hashes
 .blockedUrl:
 mov ebx,[record]
 mov byte [ebx+MF_URL],0
 mov dword [ebx+MF_BLOCKED],1
 .hashes:
 stdcall JsonGet,[cfContext],[object],cfKHashes
 mov [hashes],eax
 stdcall JsonCount,[cfContext],eax
 cmp eax,32
 jbe .hashCount
 mov eax,32
 .hashCount: mov [hashCount],eax
 mov [hashIndex],0
 .hashLoop:
 mov eax,[hashIndex]
 cmp eax,[hashCount]
 jae .hashDone
 stdcall JsonAt,[cfContext],[hashes],eax
 mov [hashObject],eax
 stdcall JsonGet,[cfContext],eax,cfKAlgo
 stdcall JsonInt,[cfContext],eax
 cmp eax,1
 jne .hashNext
 stdcall JsonGet,[cfContext],[hashObject],cfKValue
 mov edx,[record]
 add edx,MF_SHA1
 stdcall CfCopy,[cfContext],eax,edx,64
 mov edx,[record]
 add edx,MF_SHA1
 stdcall ModIsSha1,edx
 test eax,eax
 jnz .hashDone
 mov ebx,[record]
 mov byte [ebx+MF_SHA1],0
 .hashNext: inc [hashIndex]
 jmp .hashLoop
 .hashDone:
 mov ebx,[record]
 cmp byte [ebx+MF_SHA1],0
 jne .dependencies
 mov dword [ebx+MF_BLOCKED],1
 .dependencies:
 stdcall CfDependencies,[object],[record]
 inc [modFileCount]
 .next: inc [idx]
 jmp .loop
 .okay:
 mov [result],1
 mov eax,cfMsgFiles
 cmp [modFileCount],0
 jne .status
 mov eax,cfMsgNoFiles
 .status: stdcall SetStatusA,eax
 jmp .cfEnd
 .select: stdcall SetStatusA,cfMsgSelect
 jmp .cfEnd
 .cancel: stdcall SetStatusA,cfMsgCancel
 mov [modFileCount],0
 jmp .cfEnd
 .format: stdcall SetStatusA,cfMsgFormat
 mov [modFileCount],0
 .cfEnd:
 stdcall CfCleanup
 mov eax,[result]
 ret
endp
