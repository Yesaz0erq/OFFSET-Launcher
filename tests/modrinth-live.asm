; Actual native HTTPS search and filtered version metadata (no API key needed).
format PE console 4.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '../src/mod_types.inc'
section '.data' data readable writeable
hWnd dd 0
query db 'fabric api',0
knownProject db 'P7dR8mSH',0
resultName du 'tests\out\modrinth-live-result.txt',0
fixturePath du 'tests\out\modrinth-live-game',0
fileHandle dd 0
written dd 0
gameDir rw 1024
section '.text' code readable executable
start:
    stdcall ModsInit
    invoke GetFullPathNameW,fixturePath,1024,gameDir,0
    test eax,eax
    jz failed
    invoke lstrcpyA,modQuery,query
    stdcall ModrinthSearch
    test eax,eax
    jz failed
    cmp [modProjectCount],0
    je failed
    ; Stable official Fabric API project; search is independently proven above.
    invoke lstrcpyA,modProjects,knownProject
    mov [modProjectCount],1
    mov [modSelectedProject],0
    stdcall ModrinthVersions
    test eax,eax
    jz failed
    cmp [modFileCount],0
    je failed
    stdcall ModSafeFilename,modFiles+MF_NAME
    test eax,eax
    jz failed
    stdcall ModTrustedUrl,modFiles+MF_URL,0
    test eax,eax
    jz failed
    stdcall ModIsSha1,modFiles+MF_SHA1
    test eax,eax
    jz failed
    ; Real artifact download, official hash, ZIP signature and local operations.
    stdcall ModsInstall,0
    test eax,eax
    jz failed
    stdcall ModsScan
    cmp [modLocalCount],0
    je failed
    stdcall ModsToggle,0
    test eax,eax
    jz failed
    stdcall ModsScan
    cmp dword [modLocals+ML_DISABLED],1
    jne failed
    stdcall ModsToggle,0
    test eax,eax
    jz failed
    invoke CreateFileW,resultName,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je failed
    mov [fileHandle],eax
    invoke lstrlenA,modFiles+MF_NAME
    invoke WriteFile,[fileHandle],modFiles+MF_NAME,eax,written,0
    invoke CloseHandle,[fileHandle]
    invoke ExitProcess,0
failed:
    invoke ExitProcess,1
proc SetStatusA,source
    ret
endp
include '../src/common.inc'
include '../src/mods.inc'
include '../src/modrinth.inc'
include '../src/imports.inc'
