format PE console 4.0
stack 100000h,40000h
entry start
include '..\tools\INCLUDE\win32ax.inc'
include '..\src\mod_types.inc'
section '.data' data readable writeable
gameDir rw 1024
rootPattern du 'tests\out\mods-native-',0
tickPattern du '%08X ',0
unicodePart dw 08DEFh,05F84h,0
nameJar db 'Example.jar',0
nameBad db '..\outside.jar',0
nameCon db 'CON.any.jar',0
nameCom db 'COM1.jar',0
nameTrailing db 'bad .jar',0
nameUnicode db '模组.jar',0
nameDuplicate db 'Collision.jar',0
nameHtml db 'NotAJar.jar',0
badScheme db 'http://cdn.modrinth.com/data/a.jar',0
goodMr db 'https://cdn.modrinth.com/data/a.jar',0
evilMr db 'https://cdn.modrinth.com.evil.test/a.jar',0
goodCf db 'https://mediafilez.forgecdn.net/files/a.jar',0
evilCf db 'https://evilforgecdn.net/files/a.jar',0
credentials db 'https://a@cdn.modrinth.com/a.jar',0
query db 'a +',0E4h,0B8h,0ADh,0
queryExpected db 'a%20%2B%E4%B8%AD',0
numMax db '2147483647',0
numOverflow db '2147483648',0
seed db 'prefix',0
appendText db '-suffix',0
appendExpected db 'prefix-suffix',0
zipBytes db 'PK',3,4,'fixture native mod payload'
zipBytesEnd:
different db 'different old contents'
differentEnd:
htmlBytes db '<html>not a jar</html>'
htmlBytesEnd:
secret db 'test-only-local-key',0
zeroHash db '0000000000000000000000000000000000000000',0
statusPtr dd 0
rootW rw 2048
pathW rw 2048
nameW rw 280
tickW rw 64
digest rb 64
buffer rb 256
testFile dd 0
got dd 0
cacheW rw 2048
fixtureIndex dd 0

section '.text' code readable executable
start:
    stdcall ModsInit
    stdcall ModUrlEncode,query,buffer,256
    cmp eax,16
    jne fail1
    invoke lstrcmpA,buffer,queryExpected
    test eax,eax
    jnz fail1
    stdcall ModUrlEncode,query,buffer,3
    cmp eax,-1
    jne fail1
    cmp byte [buffer],0
    jne fail1
    invoke lstrcpyA,buffer,seed
    stdcall ModAppend,buffer,12,appendText
    test eax,eax
    jnz fail1
    invoke lstrcmpA,buffer,seed
    test eax,eax
    jnz fail1
    stdcall ModAppend,buffer,256,appendText
    cmp eax,1
    jne fail1
    invoke lstrcmpA,buffer,appendExpected
    test eax,eax
    jnz fail1
    stdcall ModSafeFilename,nameBad
    test eax,eax
    jnz fail2
    stdcall ModSafeFilename,nameCon
    test eax,eax
    jnz fail2
    stdcall ModSafeFilename,nameCom
    test eax,eax
    jnz fail2
    stdcall ModSafeFilename,nameTrailing
    test eax,eax
    jnz fail2
    stdcall ModSafeFilename,nameUnicode
    cmp eax,1
    jne fail2
    stdcall ModTrustedUrl,goodMr,0
    cmp eax,1
    jne fail3
    stdcall ModTrustedUrl,goodCf,1
    cmp eax,1
    jne fail3
    stdcall ModTrustedUrl,badScheme,0
    test eax,eax
    jnz fail3
    stdcall ModTrustedUrl,evilMr,0
    test eax,eax
    jnz fail3
    stdcall ModTrustedUrl,evilCf,1
    test eax,eax
    jnz fail3
    stdcall ModTrustedUrl,credentials,0
    test eax,eax
    jnz fail3
    stdcall ModNumber,numMax
    cmp eax,2147483647
    jne fail3
    stdcall ModNumber,numOverflow
    test eax,eax
    jnz fail3
    ; Create a fresh Unicode + spaces directory wholly below tests/out.
    invoke lstrcpyW,rootW,rootPattern
    invoke GetTickCount
    cinvoke wsprintfW,tickW,tickPattern,eax
    invoke lstrcatW,rootW,tickW
    invoke lstrcatW,rootW,unicodePart
    invoke GetFullPathNameW,rootW,1024,gameDir,0
    test eax,eax
    jz fail4
    stdcall EnsureDirectory,gameDir
    test eax,eax
    jz fail4
    invoke lstrcpyW,pathW,gameDir
    invoke lstrcatW,pathW,mlModsSuffix
    stdcall EnsureDirectory,pathW
    test eax,eax
    jz fail4
    invoke lstrcatW,pathW,mlSlash
    invoke MultiByteToWideChar,65001,8,nameUnicode,-1,nameW,280
    invoke lstrcatW,pathW,nameW
    stdcall WriteFixture,pathW,zipBytes,zipBytesEnd-zipBytes
    test eax,eax
    jz fail4
    stdcall ModsScan
    cmp eax,1
    jne fail4
    cmp [modLocals+ML_DISABLED],0
    jne fail4
    stdcall ModsToggle,0
    cmp eax,1
    jne fail5
    stdcall ModsScan
    cmp eax,1
    jne fail5
    cmp [modLocals+ML_DISABLED],1
    jne fail5
    stdcall ModsToggle,0
    cmp eax,1
    jne fail5
    stdcall ModsScan
    cmp eax,1
    jne fail5
    cmp [modLocals+ML_DISABLED],0
    jne fail5
    ; Collision must preserve the enabled and disabled copies.
    invoke lstrcpyW,pathW,gameDir
    invoke lstrcatW,pathW,mlModsSuffix
    invoke lstrcatW,pathW,mlSlash
    invoke lstrcatW,pathW,nameW
    invoke lstrcatW,pathW,mlDisabled
    stdcall WriteFixture,pathW,different,differentEnd-different
    test eax,eax
    jz fail5
    stdcall ModsToggle,0
    test eax,eax
    jnz fail5
    invoke GetFileAttributesW,pathW
    cmp eax,-1
    je fail5
    ; Trash is a recoverable move; the selected basename no longer exists.
    stdcall ModsTrash,0
    cmp eax,1
    jne fail6
    stdcall ModsScan
    cmp eax,1
    jne fail6
    cmp [modLocals+ML_DISABLED],1
    jne fail6
    ; Prepopulate SHA1 cache; real transport must reuse it without network.
    invoke lstrcpyW,pathW,gameDir
    invoke lstrcatW,pathW,mlSlash
    invoke lstrcatW,pathW,mlJar
    stdcall WriteFixture,pathW,zipBytes,zipBytesEnd-zipBytes
    test eax,eax
    jz fail7
    stdcall Sha1File,pathW,digest
    test eax,eax
    jz fail7
    stdcall PrepareFileRecord,nameJar
    stdcall PrepareCachedFixture,zipBytes,zipBytesEnd-zipBytes
    test eax,eax
    jz fail7
    stdcall ModsInstall,0
    cmp eax,1
    jne fail7
    ; Reinstall identical data reuses the existing file, no overwrite.
    stdcall ModsInstall,0
    cmp eax,1
    jne fail7
    ; A conflicting target never gets overwritten by download.
    stdcall PrepareFileRecord,nameDuplicate
    invoke lstrcpyW,pathW,gameDir
    invoke lstrcatW,pathW,mlModsSuffix
    invoke lstrcatW,pathW,mlSlash
    invoke MultiByteToWideChar,65001,8,nameDuplicate,-1,nameW,280
    invoke lstrcatW,pathW,nameW
    stdcall WriteFixture,pathW,different,differentEnd-different
    stdcall ModsInstall,0
    test eax,eax
    jnz fail8
    stdcall Sha1File,pathW,buffer
    invoke lstrcmpA,buffer,digest
    test eax,eax
    jz fail8
    mov [modCancel],1
    stdcall ModsInstall,0
    test eax,eax
    jnz fail8
    mov [modCancel],0
    ; An officially hashed but non-ZIP payload stays out of the mods folder.
    invoke lstrcpyW,pathW,gameDir
    invoke lstrcatW,pathW,mlSlash
    invoke lstrcatW,pathW,mlJar
    stdcall WriteFixture,pathW,htmlBytes,htmlBytesEnd-htmlBytes
    stdcall Sha1File,pathW,digest
    test eax,eax
    jz fail8
    stdcall PrepareFileRecord,nameHtml
    stdcall PrepareCachedFixture,htmlBytes,htmlBytesEnd-htmlBytes
    stdcall ModsInstall,0
    test eax,eax
    jnz fail8
    cmp [statusPtr],mlErrZip
    jne fail8
    mov dword [modFiles+MF_BLOCKED],1
    stdcall ModsInstall,0
    test eax,eax
    jnz fail8
    cmp [statusPtr],mlErrBlocked
    jne fail8
    mov dword [modFiles+MF_BLOCKED],0
    ; Actual user-scope DPAPI roundtrip only beside this test EXE (tests/out).
    invoke lstrcpyA,curseApiKey,secret
    stdcall ModsSaveKey
    cmp eax,1
    jne fail9
    mov byte [curseApiKey],0
    stdcall ModsLoadKey
    cmp eax,1
    jne fail9
    invoke lstrcmpA,curseApiKey,secret
    test eax,eax
    jnz fail9
    mov byte [curseApiKey],0
    stdcall ModsSaveKey
    cmp eax,1
    jne fail9
    stdcall ModsLoadKey
    test eax,eax
    jnz fail9
    invoke ExitProcess,0
fail1: invoke ExitProcess,11
fail2: invoke ExitProcess,12
fail3: invoke ExitProcess,13
fail4: invoke ExitProcess,14
fail5: invoke ExitProcess,15
fail6: invoke ExitProcess,16
fail7: invoke ExitProcess,17
fail8: invoke ExitProcess,18
fail9: invoke ExitProcess,19

proc SetStatusA,source
    mov eax,[source]
    mov [statusPtr],eax
    ret
endp
proc WriteFixture uses ebx,path,bytes,size
    invoke CreateFileW,[path],GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .bad
    mov ebx,eax
    invoke WriteFile,eax,[bytes],[size],got,0
    push eax
    invoke CloseHandle,ebx
    pop eax
    ret
.bad:
    xor eax,eax
    ret
endp
proc PrepareFileRecord,filename
    mov [modFileCount],1
    mov [modProvider],0
    invoke lstrcpyA,modFiles+MF_NAME,[filename]
    invoke lstrcpyA,modFiles+MF_SHA1,digest
    invoke lstrcpyA,modFiles+MF_URL,goodMr
    ret
endp
proc PrepareCachedFixture uses ebx,bytes,size
    invoke lstrcpyW,cacheW,gameDir
    invoke lstrcatW,cacheW,mlCacheSuffix
    stdcall EnsureDirectory,cacheW
    invoke MultiByteToWideChar,65001,8,digest,-1,nameW,280
    invoke lstrcatW,cacheW,nameW
    invoke lstrcatW,cacheW,mlJar
    stdcall WriteFixture,cacheW,[bytes],[size]
    ret
endp
include '..\src\common.inc'
include '..\src\mods.inc'
include '..\src\imports.inc'
