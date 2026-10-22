; Real hidden edit controls; default requests are captured without networking.
format PE console 6.0
entry start
include 'win32ax.inc'
section '.data' data readable writeable
busy dd 0
detailActive dd 0
modTab dd 0
packTab dd 0
modOffset dd 60
packOffset dd 60
hModQuery dd 0
hPackQuery dd 0
modCalls dd 0
packCalls dd 0
cls du 'EDIT',0
empty du 0
typed du 'user draft',0
section '.text' code readable executable
start:
 invoke CreateWindowExW,0,cls,empty,WS_POPUP,0,0,100,24,0,0,0,0
 mov [hModQuery],eax
 test eax,eax
 jz failed
 invoke CreateWindowExW,0,cls,empty,WS_POPUP,0,0,100,24,0,0,0,0
 mov [hPackQuery],eax
 test eax,eax
 jz failed
 mov [busy],1
 stdcall ModBrowseDefault
 stdcall PackBrowseDefault
 cmp [modCalls],0
 jne failed
 cmp [packCalls],0
 jne failed
 mov [busy],0
 stdcall ModBrowseDefault
 stdcall ModBrowseDefault
 cmp [modCalls],1
 jne failed
 cmp [modOffset],0
 jne failed
 mov [packTab],1
 stdcall PackBrowseDefault
 cmp [packCalls],0
 jne failed
 mov [packTab],0
 mov [detailActive],1
 stdcall PackBrowseDefault
 cmp [packCalls],0
 jne failed
 mov [detailActive],0
 invoke SetWindowTextW,[hPackQuery],typed
 stdcall PackBrowseDefault
 cmp [packCalls],0
 jne failed
 invoke SetWindowTextW,[hPackQuery],empty
 stdcall PackBrowseDefault
 stdcall PackBrowseDefault
 cmp [packCalls],1
 jne failed
 cmp [packOffset],0
 jne failed
 invoke DestroyWindow,[hModQuery]
 invoke DestroyWindow,[hPackQuery]
 invoke ExitProcess,0
failed:
 invoke ExitProcess,1
proc BeginModSearch
 inc [modCalls]
 ret
endp
proc PackBeginSearch
 inc [packCalls]
 ret
endp
include '..\src\browse_defaults.inc'
include '..\src\imports.inc'
