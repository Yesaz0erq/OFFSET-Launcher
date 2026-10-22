; Real file operations: profile isolation, explicit VM scope and cache routing.
format PE console 6.0
stack 100000h,40000h
entry start
include 'win32ax.inc'
include '..\src\mod_types.inc'
OFFSET_VERSION_SETTINGS=1
section '.data' data readable writeable
gameDir rw 1024
selectedVersion db 'shared',0,121 dup(0)
rootPrefix du 'tests\out\version-mods-',0
rootFormat du '%08X',0
isolatedSuffix dw '\','v','e','r','s','i','o','n','s','\',7248h,672Ch,'\','g','a','m','e',0
scopedSuffix dw '\',72ECh,7ACBh,76EEh,5F55h,0
sharedId db 'shared',0
isolatedId db 'isolated',0
fixtureNameA db 'Profile.jar',0
fixtureNameW du 'Profile.jar',0
fixtureScopeName du 'Scope.jar',0
fixtureUrl db 'https://cdn.modrinth.com/data/test/versions/test/Profile.jar',0
fixtureData db 'PK',3,4,'profile scope fixture'
fixtureDataEnd:
otherData db 'keep the original different file'
otherDataEnd:
step dd 1
rootW rw 2048
isolatedW rw 2048
scopedW rw 2048
pathW rw 2048
path2W rw 2048
serialW rw 64
savedGameW rw 1024
digest rb 64
copyHash rb 64
fileHandle dd 0
written dd 0
resolverCalls dd 0

section '.text' code readable executable
start:
    stdcall ModsInit
    invoke lstrcpyW,pathW,rootPrefix
    invoke GetTickCount
    cinvoke wsprintfW,serialW,rootFormat,eax
    invoke lstrcatW,pathW,serialW
    invoke GetFullPathNameW,pathW,2048,rootW,0
    test eax,eax
    jz failed
    invoke lstrcpyW,gameDir,rootW
    invoke lstrcpyW,savedGameW,gameDir
    invoke lstrcpyW,isolatedW,rootW
    invoke lstrcatW,isolatedW,isolatedSuffix
    invoke lstrcpyW,scopedW,rootW
    invoke lstrcatW,scopedW,scopedSuffix
    stdcall CreateNamedFixture,rootW,fixtureNameW,fixtureData,fixtureDataEnd-fixtureData
    test eax,eax
    jz failed
    stdcall CreateNamedFixture,isolatedW,fixtureNameW,fixtureData,fixtureDataEnd-fixtureData
    test eax,eax
    jz failed
    stdcall CreateNamedFixture,scopedW,fixtureScopeName,fixtureData,fixtureDataEnd-fixtureData
    test eax,eax
    jz failed
    ; Shared profile sees only the shared root. Isolated sibling is untouched.
    stdcall ModsScan
    cmp eax,1
    jne failed
    invoke lstrcmpiW,modLocals,fixtureNameW
    test eax,eax
    jnz failed
    inc [step]
    invoke lstrcpyA,selectedVersion,isolatedId
    stdcall ModsScan
    cmp eax,1
    jne failed
    stdcall ModsToggle,0
    cmp eax,1
    jne failed
    stdcall MakeModPath,rootW,fixtureNameW,pathW
    invoke GetFileAttributesW,pathW
    cmp eax,-1
    je failed
    stdcall MakeModPath,isolatedW,fixtureNameW,pathW
    invoke GetFileAttributesW,pathW
    cmp eax,-1
    jne failed
    invoke lstrcatW,pathW,mlDisabled
    invoke GetFileAttributesW,pathW
    cmp eax,-1
    je failed
    ; Toggle collisions never replace the enabled copy.
    inc [step]
    stdcall CreateNamedFixture,isolatedW,fixtureNameW,otherData,otherDataEnd-otherData
    test eax,eax
    jz failed
    stdcall ModsToggle,0
    test eax,eax
    jnz failed
    stdcall MakeModPath,isolatedW,fixtureNameW,pathW
    stdcall Sha1File,pathW,digest
    test eax,eax
    jz failed
    stdcall ModsScan
    cmp eax,2
    jne failed
    ; Explicit page scope takes priority, then clearing returns to selected root.
    inc [step]
    mov eax,[resolverCalls]
    mov [written],eax
    stdcall ModsSetDataRoot,scopedW
    cmp eax,1
    jne failed
    stdcall ModsScan
    cmp eax,1
    jne failed
    invoke lstrcmpiW,modLocals,fixtureScopeName
    test eax,eax
    jnz failed
    mov eax,[resolverCalls]
    cmp eax,[written]
    jne failed
    stdcall ModsTrash,0
    cmp eax,1
    jne failed
    stdcall MakeModPath,scopedW,fixtureScopeName,pathW
    invoke GetFileAttributesW,pathW
    cmp eax,-1
    jne failed
    stdcall ModsSetDataRoot,0
    stdcall ModsScan
    cmp eax,2
    jne failed
    stdcall MakeModPath,isolatedW,fixtureNameW,pathW
    stdcall VerifySha1,pathW,digest
    cmp eax,1
    jne failed
    ; Install cache and final JAR both use the selected profile's data directory.
    inc [step]
    stdcall WriteFixture,addr pathW,fixtureData,fixtureDataEnd-fixtureData
    stdcall Sha1File,pathW,digest
    test eax,eax
    jz failed
    mov [modFileCount],1
    mov [modProvider],0
    invoke lstrcpyA,modFiles+MF_NAME,fixtureNameA
    invoke lstrcpyA,modFiles+MF_URL,fixtureUrl
    invoke lstrcpyA,modFiles+MF_SHA1,digest
    ; Restore the chosen filename to different contents to test collision first.
    stdcall WriteFixture,pathW,otherData,otherDataEnd-otherData
    stdcall ModsInstall,0
    test eax,eax
    jnz failed
    invoke DeleteFileW,pathW
    test eax,eax
    jz failed
    invoke lstrcpyW,path2W,isolatedW
    invoke lstrcatW,path2W,mlCacheSuffix
    stdcall EnsureDirectory,path2W
    test eax,eax
    jz failed
    invoke MultiByteToWideChar,65001,0,digest,-1,serialW,64
    invoke lstrcatW,path2W,serialW
    invoke lstrcatW,path2W,mlJar
    stdcall WriteFixture,path2W,fixtureData,fixtureDataEnd-fixtureData
    test eax,eax
    jz failed
    stdcall ModsInstall,0
    cmp eax,1
    jne failed
    stdcall VerifySha1,pathW,digest
    cmp eax,1
    jne failed
    ; Every operation preserved global launch selection and game/cache root.
    inc [step]
    invoke lstrcmpiW,gameDir,savedGameW
    test eax,eax
    jnz failed
    invoke lstrcmpA,selectedVersion,isolatedId
    test eax,eax
    jnz failed
    stdcall MakeModPath,rootW,fixtureNameW,pathW
    stdcall VerifySha1,pathW,digest
    cmp eax,1
    jne failed
    stdcall ModsSetDataRoot,0
    stdcall ModsDataRoot,pathW,2
    test eax,eax
    jnz failed
    cmp word [pathW],0
    jne failed
    invoke ExitProcess,0
failed:
    invoke ExitProcess,[step]

proc SetStatusA,source
    ret
endp
proc VsResolveData uses ebx,root,id,dest,capacity
    inc [resolverCalls]
    invoke lstrcmpA,[id],isolatedId
    mov ebx,[root]
    test eax,eax
    jnz .copy
    mov ebx,isolatedW
.copy:
    invoke lstrlenW,ebx
    cmp eax,[capacity]
    jae .bad
    invoke lstrcpyW,[dest],ebx
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp
proc MakeModPath uses ebx,root,name,dest
    mov ebx,[dest]
    invoke lstrcpyW,ebx,[root]
    invoke lstrcatW,ebx,mlModsSuffix
    invoke lstrcatW,ebx,mlSlash
    invoke lstrcatW,ebx,[name]
    ret
endp
proc CreateNamedFixture uses ebx,root,name,bytes,size
    stdcall MakeModPath,[root],[name],pathW
    invoke lstrcpyW,path2W,[root]
    invoke lstrcatW,path2W,mlModsSuffix
    stdcall EnsureDirectory,path2W
    test eax,eax
    jz .done
    stdcall WriteFixture,pathW,[bytes],[size]
.done:
    ret
endp
proc WriteFixture uses ebx,path,bytes,size
    invoke CreateFileW,[path],GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .bad
    mov ebx,eax
    invoke WriteFile,ebx,[bytes],[size],written,0
    push eax
    invoke CloseHandle,ebx
    pop eax
    ret
.bad:
    xor eax,eax
    ret
endp
include '..\src\common.inc'
include '..\src\mods.inc'
include '..\src\imports.inc'
