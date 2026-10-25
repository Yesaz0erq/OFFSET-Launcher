; OFFSET 0.2 - original Modrinth v2 provider, native Win32 assembly.
; HTTP/JSON response ownership never escapes these synchronous worker calls.
section '.mrdata' data readable writeable
mrResponse dd 0
mrContext dd 0
mrRoot dd 0
mrResult dd 0
mrHeader du 'Accept: application/json',13,10,'User-Agent: OFFSET/1.0.0 (native Minecraft launcher; Modrinth integration)',13,10,0
mrSearchBase db 'https://api.modrinth.com/v2/search?limit=20&query=',0
mrFacetParam db '&facets=',0
mrOffsetParam db '&offset=',0
mrFacetPrefix db '[["project_type:mod"],["versions:',0
mrFacetMiddle db '"],["categories:',0
mrFacetClose db '"]]',0
mrProjectBase db 'https://api.modrinth.com/v2/project/',0
mrVersionSuffix db '/version?include_changelog=false&game_versions=',0
mrLoaderParam db '&loaders=',0
mrArrayOpen db '["',0
mrArrayClose db '"]',0
mrWebsiteBase db 'https://modrinth.com/mod/',0
mrFmtNumber db '%u',0
mrKHits db 'hits',0
mrKTotal db 'total_hits',0
mrKProject db 'project_id',0
mrKProjectType db 'project_type',0
mrKTitle db 'title',0
mrKDescription db 'description',0
mrKVersions db 'game_versions',0
mrKLoaders db 'loaders',0
mrKFiles db 'files',0
mrKPrimary db 'primary',0
mrKFilename db 'filename',0
mrKUrl db 'url',0
mrKHashes db 'hashes',0
mrKSha1 db 'sha1',0
mrKId db 'id',0
mrKNumber db 'version_number',0
mrKName db 'name',0
mrKType db 'version_type',0
mrKDependencies db 'dependencies',0
mrKDependencyType db 'dependency_type',0
mrKVersionId db 'version_id',0
mrMod db 'mod',0
mrRelease db 'release',0
mrBeta db 'beta',0
mrRequired db 'required',0
mrDependencyPrefix db '必需依赖: ',0
mrDependencyComma db ', ',0
mrDependencyUnknown db '(未提供项目ID)',0
mrFabric db 'fabric',0
mrForge db 'forge',0
mrNeoForge db 'neoforge',0
mrQuilt db 'quilt',0
mrMsgSearch db '正在 Modrinth 搜索模组…',0
mrMsgFound db 'Modrinth 搜索完成，选择项目查看兼容版本。',0
mrMsgEmpty db 'Modrinth 未找到匹配项目，可更换关键词、游戏版本或加载器。',0
mrMsgVersions db '正在查询 Modrinth 兼容版本…',0
mrMsgVersionsReady db '兼容版本已加载，选择版本后下载；必需依赖需分别安装。',0
mrMsgNoFiles db '没有符合当前游戏版本与加载器、且可下载的 JAR 文件。',0
mrMsgFilter db '请填写有效的游戏版本，并选择 Fabric、Forge、NeoForge 或 Quilt。',0
mrMsgProject db '请先选择一个有效的 Modrinth 项目。',0
mrMsgNetwork db 'Modrinth 请求失败，请检查网络后重试。',0
mrMsgRate db 'Modrinth 请求过于频繁，请稍候重试。',0
mrMsgFormat db 'Modrinth 返回的数据无效，已停止处理。',0
mrMsgCancel db '模组查询已取消。',0
; Buffers trail initialized values: no zero-filled padding in portable EXE.
mrUrl rb 8192
mrWideUrl rw 8192
mrFilter rb 1024
mrEncoded rb 3072
mrScalar rb 512
mrNumber rb 32

section '.mrcode' code readable executable

proc MrClose
    cmp [mrContext],0
    je .response
    stdcall JsonFree,[mrContext]
    mov [mrContext],0
.response:
    stdcall FreeMem,[mrResponse]
    mov [mrResponse],0
    ret
endp

; JSON strings must neither truncate nor contain embedded NUL aliases.
; Length -1 is a hard failure, zero is a valid empty/missing optional field.
proc MrCopy uses ebx,ctx,token,destination,capacity
    stdcall JsonString,[ctx],[token],[destination],[capacity]
    cmp eax,[capacity]
    jae .bad
    mov ebx,eax
    invoke lstrlenA,[destination]
    cmp eax,ebx
    jne .bad
    mov eax,ebx
    ret
.bad:
    mov eax,[destination]
    mov byte [eax],0
    mov eax,-1
    ret
endp

; IDs are emitted as URL path components, never trusted as a raw URL.
proc MrValidId uses esi,source
    mov esi,[source]
    xor ecx,ecx
.loop:
    mov al,[esi+ecx]
    test al,al
    jz .validEnd
    cmp ecx,63
    jae .bad
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .next
    cmp al,'A'
    jb .bad
    cmp al,'Z'
    jbe .next
    cmp al,'a'
    jb .bad
    cmp al,'z'
    ja .bad
.next:
    inc ecx
    jmp .loop
.validEnd:
    xor eax,eax
    test ecx,ecx
    setnz al
    ret
.bad:
    xor eax,eax
    ret
endp

proc MrValidFilter uses esi
    invoke lstrcmpA,modLoader,mrFabric
    test eax,eax
    jz .game
    invoke lstrcmpA,modLoader,mrForge
    test eax,eax
    jz .game
    invoke lstrcmpA,modLoader,mrNeoForge
    test eax,eax
    jz .game
    invoke lstrcmpA,modLoader,mrQuilt
    test eax,eax
    jnz .bad
.game:
    mov esi,modGameVersion
    xor ecx,ecx
.loop:
    lodsb
    test al,al
    jz .validEnd
    inc ecx
    cmp ecx,63
    ja .bad
    cmp al,'-'
    je .loop
    cmp al,'.'
    je .loop
    cmp al,'_'
    je .loop
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .loop
    cmp al,'A'
    jb .bad
    cmp al,'Z'
    jbe .loop
    cmp al,'a'
    jb .bad
    cmp al,'z'
    jbe .loop
.bad:
    xor eax,eax
    ret
.validEnd:
    xor eax,eax
    test ecx,ecx
    setnz al
    ret
endp

; Fetch an already assembled HTTPS API URL. Caller closes parsed context.
proc MrRequest
    cmp [modCancel],0
    jne .cancel
    invoke MultiByteToWideChar,65001,8,mrUrl,-1,mrWideUrl,8192
    test eax,eax
    jz .bad
    stdcall HttpRequest,mrWideUrl,0,mrHeader,0
    mov [mrResponse],eax
    test eax,eax
    jz .network
    cmp [modCancel],0
    jne .cancel
    cmp [httpStatus],429
    je .rate
    cmp [httpStatus],200
    jne .network
    stdcall JsonParse,[mrResponse]
    mov [mrContext],eax
    test eax,eax
    jz .bad
    stdcall JsonRoot,eax
    mov [mrRoot],eax
    mov eax,1
    ret
.cancel:
    stdcall SetStatusA,mrMsgCancel
    xor eax,eax
    ret
.rate:
    stdcall SetStatusA,mrMsgRate
    xor eax,eax
    ret
.network:
    stdcall SetStatusA,mrMsgNetwork
    xor eax,eax
    ret
.bad:
    stdcall SetStatusA,mrMsgFormat
    xor eax,eax
    ret
endp

proc ModrinthSearch uses ebx esi edi
    mov [modProjectCount],0
    mov [modFileCount],0
    mov [modSelectedProject],-1
    mov [modSelectedFile],-1
    mov [modTotal],0
    mov [mrResult],0
    mov edi,modProjects
    xor eax,eax
    mov ecx,MP_SIZE*MOD_MAX_PROJECTS/4
    rep stosd
    mov edi,modFiles
    mov ecx,MF_SIZE*MOD_MAX_FILES/4
    rep stosd
    stdcall MrValidFilter
    test eax,eax
    jz .filter
    cmp [modOffset],0
    jl .filter
    stdcall SetStatusA,mrMsgSearch
    invoke lstrcpyA,mrUrl,mrSearchBase
    stdcall CnResolve,modQuery
    stdcall ModUrlEncode,eax,mrEncoded,3072
    cmp eax,-1
    je .filter
    stdcall ModAppend,mrUrl,8192,mrEncoded
    test eax,eax
    jz .filter
    invoke lstrcpyA,mrFilter,mrFacetPrefix
    stdcall ModAppend,mrFilter,1024,modGameVersion
    stdcall ModAppend,mrFilter,1024,mrFacetMiddle
    stdcall ModAppend,mrFilter,1024,modLoader
    stdcall ModAppend,mrFilter,1024,mrFacetClose
    stdcall ModUrlEncode,mrFilter,mrEncoded,3072
    cmp eax,-1
    je .filter
    stdcall ModAppend,mrUrl,8192,mrFacetParam
    stdcall ModAppend,mrUrl,8192,mrEncoded
    stdcall ModAppend,mrUrl,8192,mrOffsetParam
    cinvoke wsprintfA,mrNumber,mrFmtNumber,[modOffset]
    stdcall ModAppend,mrUrl,8192,mrNumber
    test eax,eax
    jz .filter
    stdcall MrRequest
    test eax,eax
    jz .done
    stdcall MrParseSearch
    mov [mrResult],eax
    jmp .done
.filter:
    stdcall SetStatusA,mrMsgFilter
.done:
    stdcall MrClose
    mov eax,[mrResult]
    ret
endp

; Parse an owned current context. Metadata strings are bounded before exposure.
proc MrParseSearch uses ebx esi edi
    locals
        list dd ?
        count dd ?
        idx dd ?
        item dd ?
        record dd ?
    endl
    stdcall JsonGet,[mrContext],[mrRoot],mrKTotal
    stdcall JsonInt,[mrContext],eax
    test eax,eax
    js .bad
    mov [modTotal],eax
    stdcall JsonGet,[mrContext],[mrRoot],mrKHits
    test eax,eax
    jz .bad
    mov [list],eax
    stdcall JsonCount,[mrContext],eax
    cmp eax,MOD_MAX_PROJECTS
    jbe .count
    mov eax,MOD_MAX_PROJECTS
.count:
    mov [count],eax
    mov [idx],0
.loop:
    cmp [modCancel],0
    jne .cancel
    mov eax,[idx]
    cmp eax,[count]
    jae .ready
    stdcall JsonAt,[mrContext],[list],eax
    mov [item],eax
    mov eax,[modProjectCount]
    imul eax,MP_SIZE
    add eax,modProjects
    mov [record],eax
    stdcall JsonGet,[mrContext],[item],mrKProjectType
    test eax,eax
    jz .identity
    stdcall MrCopy,[mrContext],eax,mrScalar,512
    invoke lstrcmpA,mrScalar,mrMod
    test eax,eax
    jnz .next
.identity:
    stdcall JsonGet,[mrContext],[item],mrKProject
    stdcall MrCopy,[mrContext],eax,[record],64
    cmp eax,64
    jae .next
    stdcall MrValidId,[record]
    test eax,eax
    jz .next
    stdcall JsonGet,[mrContext],[item],mrKTitle
    mov ebx,[record]
    add ebx,MP_TITLE
    stdcall MrCopy,[mrContext],eax,ebx,320
    cmp eax,320
    jae .next
    test eax,eax
    jz .next
    stdcall JsonGet,[mrContext],[item],mrKDescription
    mov ebx,[record]
    add ebx,MP_DESC
    stdcall MrCopy,[mrContext],eax,ebx,512
    cmp eax,512
    jb .website
    mov byte [ebx],0
.website:
    mov ebx,[record]
    add ebx,MP_URL
    invoke lstrcpyA,ebx,mrWebsiteBase
    stdcall ModAppend,ebx,512,[record]
    test eax,eax
    jz .next
    stdcall CnDecorateTitle,[record]
    inc [modProjectCount]
.next:
    inc [idx]
    jmp .loop
.ready:
    mov eax,mrMsgFound
    cmp [modProjectCount],0
    jne .message
    mov eax,mrMsgEmpty
.message:
    stdcall SetStatusA,eax
    mov eax,1
    ret
.cancel:
    stdcall SetStatusA,mrMsgCancel
    jmp .failure
.bad:
    stdcall SetStatusA,mrMsgFormat
.failure:
    mov [modProjectCount],0
    xor eax,eax
    ret
endp

proc ModrinthVersions uses ebx esi edi
    mov [mrResult],0
    mov [modFileCount],0
    mov [modSelectedFile],-1
    mov edi,modFiles
    xor eax,eax
    mov ecx,MF_SIZE*MOD_MAX_FILES/4
    rep stosd
    stdcall MrValidFilter
    test eax,eax
    jz .filter
    mov eax,[modSelectedProject]
    cmp eax,[modProjectCount]
    jae .project
    imul eax,MP_SIZE
    add eax,modProjects
    mov ebx,eax
    stdcall MrValidId,ebx
    test eax,eax
    jz .project
    stdcall SetStatusA,mrMsgVersions
    invoke lstrcpyA,mrUrl,mrProjectBase
    stdcall ModAppend,mrUrl,8192,ebx
    stdcall ModAppend,mrUrl,8192,mrVersionSuffix
    invoke lstrcpyA,mrFilter,mrArrayOpen
    stdcall ModAppend,mrFilter,1024,modGameVersion
    stdcall ModAppend,mrFilter,1024,mrArrayClose
    stdcall ModUrlEncode,mrFilter,mrEncoded,3072
    stdcall ModAppend,mrUrl,8192,mrEncoded
    stdcall ModAppend,mrUrl,8192,mrLoaderParam
    invoke lstrcpyA,mrFilter,mrArrayOpen
    stdcall ModAppend,mrFilter,1024,modLoader
    stdcall ModAppend,mrFilter,1024,mrArrayClose
    stdcall ModUrlEncode,mrFilter,mrEncoded,3072
    stdcall ModAppend,mrUrl,8192,mrEncoded
    test eax,eax
    jz .filter
    stdcall MrRequest
    test eax,eax
    jz .done
    stdcall MrParseVersions
    mov [mrResult],eax
    jmp .done
.filter:
    stdcall SetStatusA,mrMsgFilter
    jmp .done
.project:
    stdcall SetStatusA,mrMsgProject
.done:
    stdcall MrClose
    mov eax,[mrResult]
    ret
endp

proc MrHasValue uses ebx esi edi, ctx,list,value
    locals
        count dd ?
        idx dd ?
        scalar rb 256
    endl
    stdcall JsonCount,[ctx],[list]
    cmp eax,4096
    ja .no
    mov [count],eax
    mov [idx],0
.loop:
    mov eax,[idx]
    cmp eax,[count]
    jae .no
    stdcall JsonAt,[ctx],[list],eax
    stdcall MrCopy,[ctx],eax,addr scalar,256
    cmp eax,256
    jae .next
    invoke lstrcmpA,addr scalar,[value]
    test eax,eax
    jz .yes
.next:
    inc [idx]
    jmp .loop
.no:
    xor eax,eax
    ret
.yes:
    mov eax,1
    ret
endp

proc MrParseVersions uses ebx esi edi
    locals
        count dd ?
        idx dd ?
        version dd ?
        record dd ?
        files dd ?
        fileCount dd ?
        fileIdx dd ?
        file dd ?
    endl
    stdcall JsonCount,[mrContext],[mrRoot]
    cmp eax,20000
    ja .bad
    mov [count],eax
    mov [idx],0
.version:
    cmp [modCancel],0
    jne .cancel
    mov eax,[idx]
    cmp eax,[count]
    jae .ready
    cmp [modFileCount],MOD_MAX_FILES
    jae .ready
    stdcall JsonAt,[mrContext],[mrRoot],eax
    mov [version],eax
    stdcall JsonGet,[mrContext],eax,mrKVersions
    stdcall MrHasValue,[mrContext],eax,modGameVersion
    test eax,eax
    jz .nextVersion
    stdcall JsonGet,[mrContext],[version],mrKLoaders
    stdcall MrHasValue,[mrContext],eax,modLoader
    test eax,eax
    jz .nextVersion
    mov eax,[modFileCount]
    imul eax,MF_SIZE
    add eax,modFiles
    mov [record],eax
    stdcall JsonGet,[mrContext],[version],mrKFiles
    mov [files],eax
    stdcall JsonCount,[mrContext],eax
    cmp eax,64
    ja .nextVersion
    mov [fileCount],eax
    mov [fileIdx],0
.file:
    mov eax,[fileIdx]
    cmp eax,[fileCount]
    jae .nextVersion
    stdcall JsonAt,[mrContext],[files],eax
    mov [file],eax
    stdcall JsonGet,[mrContext],eax,mrKPrimary
    stdcall JsonBool,[mrContext],eax
    cmp eax,1
    jne .nextFile
    stdcall JsonGet,[mrContext],[file],mrKFilename
    mov ebx,[record]
    add ebx,MF_NAME
    stdcall MrCopy,[mrContext],eax,ebx,256
    cmp eax,256
    jae .nextFile
    stdcall ModSafeFilename,ebx
    test eax,eax
    jz .nextFile
    stdcall JsonGet,[mrContext],[file],mrKUrl
    mov ebx,[record]
    add ebx,MF_URL
    stdcall MrCopy,[mrContext],eax,ebx,2048
    cmp eax,2048
    jae .nextFile
    stdcall ModTrustedUrl,ebx,0
    test eax,eax
    jz .nextFile
    stdcall JsonGet,[mrContext],[file],mrKHashes
    stdcall JsonGet,[mrContext],eax,mrKSha1
    mov ebx,[record]
    add ebx,MF_SHA1
    stdcall MrCopy,[mrContext],eax,ebx,64
    stdcall ModIsSha1,ebx
    test eax,eax
    jz .nextFile
    stdcall JsonGet,[mrContext],[version],mrKId
    stdcall MrCopy,[mrContext],eax,[record],64
    cmp eax,64
    jae .nextVersion
    stdcall MrValidId,[record]
    test eax,eax
    jz .nextVersion
    stdcall JsonGet,[mrContext],[version],mrKNumber
    mov ebx,[record]
    add ebx,MF_TITLE
    stdcall MrCopy,[mrContext],eax,ebx,320
    cmp eax,320
    jae .nextVersion
    test eax,eax
    jnz .kind
    stdcall JsonGet,[mrContext],[version],mrKName
    stdcall MrCopy,[mrContext],eax,ebx,320
    cmp eax,320
    jae .nextVersion
.kind:
    stdcall JsonGet,[mrContext],[version],mrKType
    stdcall MrCopy,[mrContext],eax,mrScalar,512
    mov ebx,[record]
    mov dword [ebx+MF_KIND],3
    invoke lstrcmpA,mrScalar,mrRelease
    test eax,eax
    jnz .beta
    mov dword [ebx+MF_KIND],1
    jmp .dependencies
.beta:
    invoke lstrcmpA,mrScalar,mrBeta
    test eax,eax
    jnz .dependencies
    mov dword [ebx+MF_KIND],2
.dependencies:
    stdcall MrDependencies,[version],[record]
    inc [modFileCount]
    jmp .nextVersion
.nextFile:
    inc [fileIdx]
    jmp .file
.nextVersion:
    inc [idx]
    jmp .version
.ready:
    mov eax,mrMsgVersionsReady
    cmp [modFileCount],0
    jne .message
    mov eax,mrMsgNoFiles
.message:
    stdcall SetStatusA,eax
    mov eax,1
    ret
.cancel:
    stdcall SetStatusA,mrMsgCancel
    jmp .failure
.bad:
    stdcall SetStatusA,mrMsgFormat
.failure:
    mov [modFileCount],0
    xor eax,eax
    ret
endp

proc MrDependencies uses ebx esi edi,version,record
    locals
        list dd ?
        count dd ?
        idx dd ?
        item dd ?
    endl
    mov ebx,[record]
    mov dword [ebx+MF_REQUIRED],0
    mov byte [ebx+MF_DEPS],0
    stdcall JsonGet,[mrContext],[version],mrKDependencies
    mov [list],eax
    stdcall JsonCount,[mrContext],eax
    cmp eax,4096
    ja .done
    mov [count],eax
    mov [idx],0
.loop:
    mov eax,[idx]
    cmp eax,[count]
    jae .done
    stdcall JsonAt,[mrContext],[list],eax
    mov [item],eax
    stdcall JsonGet,[mrContext],eax,mrKDependencyType
    stdcall MrCopy,[mrContext],eax,mrScalar,512
    invoke lstrcmpA,mrScalar,mrRequired
    test eax,eax
    jnz .next
    inc dword [ebx+MF_REQUIRED]
    cmp dword [ebx+MF_REQUIRED],1
    jne .comma
    lea edi,[ebx+MF_DEPS]
if defined OFFSET_I18N
    invoke lstrcpyA,edi,[trMrDependencyPrefix]
else
    invoke lstrcpyA,edi,mrDependencyPrefix
end if
    jmp .id
.comma:
    lea edi,[ebx+MF_DEPS]
    stdcall ModAppend,edi,512,mrDependencyComma
.id:
    stdcall JsonGet,[mrContext],[item],mrKProject
    stdcall MrCopy,[mrContext],eax,mrScalar,512
    test eax,eax
    jz .unknown
    cmp eax,64
    jae .unknown
    stdcall MrValidId,mrScalar
    test eax,eax
    jnz .append
.unknown:
if defined OFFSET_I18N
    invoke lstrcpyA,mrScalar,[trMrDependencyUnknown]
else
    invoke lstrcpyA,mrScalar,mrDependencyUnknown
end if
.append:
    stdcall ModAppend,edi,512,mrScalar
.next:
    inc [idx]
    jmp .loop
.done:
    ret
endp
