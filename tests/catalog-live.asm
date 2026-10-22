; Native live pack discovery/archive verification and rich project text fetch.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\mod_types.inc'
section '.data' data readable writeable
hWnd dd 0
engineCancel dd 0
gameDir rw 1024
stage dd 1
q db 'Fabulously Optimized',0
game db '1.21.1',0
loader db 'fabric',0
packId db '1KVo5zza',0
modId db 'P7dR8mSH',0
destination du 'tests\out\catalog-live\fabulously-optimized.mrpack',0
section '.text' code readable executable
start:
 stdcall ModsInit
 invoke lstrcpyA,packQuery,q
 invoke lstrcpyA,packGameVersion,game
 invoke lstrcpyA,packLoader,loader
 stdcall PackSearch
 cmp eax,1
 jne .fail
 cmp [packProjectCount],1
 jb .fail
 inc [stage]
 invoke lstrcpyA,packProjects,packId
 mov [packProjectCount],1
 mov [packSelectedProject],0
 stdcall PackVersions
 cmp eax,1
 jne .fail
 cmp [packFileCount],1
 jb .fail
 inc [stage]
 stdcall PackDownload,0,destination
 cmp eax,1
 jne .fail
 inc [stage]
 stdcall ProjectFetchDetails,0,packId,1
 cmp eax,1
 jne .fail
 cmp byte [detailBody],0
 je .fail
 inc [stage]
 stdcall ProjectFetchDetails,0,modId,0
 cmp eax,1
 jne .fail
 cmp byte [detailBody],0
 je .fail
 invoke ExitProcess,0
.fail:
 invoke ExitProcess,[stage]
proc SetStatusA,text
 ret
endp
include '..\src\common.inc'
include '..\src\mods.inc'
include '..\src\modrinth.inc'
include '..\src\curseforge.inc'
include '..\src\catalog.inc'
include '..\src\project_details.inc'
include '..\src\imports.inc'
