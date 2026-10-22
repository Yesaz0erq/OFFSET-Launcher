; First idle entry loads the provider's ordinary empty-keyword result page.
; Mark attempts before starting work so layout sync cannot duplicate a request.
section '.brd' data readable writeable
modDefaultAttempted dd 0
packDefaultAttempted dd 0
section '.brc' code readable executable
proc ModBrowseDefault
 local query[2]:WORD
 cmp [busy],0
 jne .done
 cmp [modTab],0
 jne .done
 cmp [modDefaultAttempted],0
 jne .done
 invoke GetWindowTextW,[hModQuery],addr query,2
 test eax,eax
 jnz .done
 mov [modDefaultAttempted],1
 mov [modOffset],0
 stdcall BeginModSearch
.done:
 ret
endp
proc PackBrowseDefault
 local query[2]:WORD
 cmp [busy],0
 jne .done
 cmp [detailActive],0
 jne .done
 cmp [packTab],0
 jne .done
 cmp [packDefaultAttempted],0
 jne .done
 invoke GetWindowTextW,[hPackQuery],addr query,2
 test eax,eax
 jnz .done
 mov [packDefaultAttempted],1
 mov [packOffset],0
 stdcall PackBeginSearch
.done:
 ret
endp
