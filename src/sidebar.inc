; Collapsible sidebar: full (220) <-> icon rail (68). The collapse amount is a
; spring (anim slot 8). Content re-centres by CONTENT_SHIFT when collapsed;
; drawing uses gOffX, clicks are translated, and native controls are moved
; once the spring settles (they stay hidden while it moves, like transitions).

SIDEBAR_RAIL = 68
CONTENT_SHIFT = -76
SIDE_SLOT = 8
A_SIDEBAR_CR = 0F4F0EEh         ; A_SIDEBAR as COLORREF, for fading labels

section '.sided' data readable writeable
sideCollapsed dd 0
sideAnim dd 0
sideT dd 0                      ; 0 = expanded .. 1024 = collapsed
contentX dd 0                   ; logical x offset of the content area
controlsX dd 0                  ; offset currently applied to native controls
navHotBase dd -1
folderHot dd -1
toggleHot dd -1
langHot dd -1
kSidebarCollapsed du 'SidebarCollapsed',0
sideRect RECT

section '.sidec' code readable executable

proc SidebarLoad
    invoke GetPrivateProfileIntW,sectionName,kSidebarCollapsed,0,iniPath
    and eax,1
    mov [sideCollapsed],eax
    shl eax,10
    mov [sideT],eax
    ret
endp

proc SidebarSave
    mov eax,[sideCollapsed]
    cinvoke wsprintfW,ramText,fmtNum,eax
    invoke WritePrivateProfileStringW,sectionName,kSidebarCollapsed,ramText,iniPath
    ret
endp

; Recompute sideT / contentX from the spring (call once per frame and on input).
proc SidebarSync
    mov eax,[sideCollapsed]
    shl eax,8
    stdcall AnimSet,SIDE_SLOT,eax,0
    stdcall AnimGet,SIDE_SLOT
    cmp eax,0
    jge @f
    xor eax,eax
@@:
    cmp eax,256
    jle @f
    mov eax,256
@@:
    shl eax,2
    mov [sideT],eax
    stdcall Lerp,0,CONTENT_SHIFT,eax
    mov [contentX],eax
    ret
endp

; Current sidebar width (logical).
proc SidebarWidth
    stdcall Lerp,SIDEBAR_W,SIDEBAR_RAIL,[sideT]
    ret
endp

; Label fade: 0 = fully visible .. 256 = gone (labels vanish in the first half).
proc SidebarLabelFade
    mov eax,[sideT]
    shr eax,1
    cmp eax,256
    jbe @f
    mov eax,256
@@:
    ret
endp

proc SidebarToggle
    xor [sideCollapsed],1
    stdcall SidebarSave
    stdcall HoverReset
    mov [sideAnim],1
    stdcall SidebarSync
    stdcall SyncPage
    stdcall AnimKick
    ; with motion off the spring snaps; finish immediately
    stdcall SidebarStep
    invoke InvalidateRect,[hWnd],0,0
    ret
endp

; Timer hook: once the spring rests, move native controls and reveal them.
proc SidebarStep
    cmp [sideAnim],0
    je .done
    stdcall SidebarSync
    mov eax,[sideCollapsed]
    shl eax,10
    cmp eax,[sideT]
    jne .done
    mov [sideAnim],0
    stdcall ShiftControls
    stdcall SyncPage
.done:
    ret
endp

proc ShiftChild child,param
    invoke GetParent,[child]
    cmp eax,[hWnd]
    jne .next
    invoke GetWindowRect,[child],sideRect
    invoke MapWindowPoints,0,[hWnd],sideRect,2
    mov eax,[sideRect.left]
    add eax,[param]
    invoke SetWindowPos,[child],0,eax,[sideRect.top],0,0,SWP_NOSIZE+SWP_NOZORDER+SWP_NOACTIVATE
.next:
    mov eax,1
    ret
endp

; Moves every page control so it matches contentX (no-op when already there).
proc ShiftControls
    mov eax,CONTENT_SHIFT
    cmp [sideCollapsed],0
    jne @f
    xor eax,eax
@@:
    mov ecx,eax
    sub eax,[controlsX]
    jz .done
    mov [controlsX],ecx
    invoke MulDiv,eax,[dpi],96
    invoke EnumChildWindows,[hWnd],ShiftChild,eax
.done:
    ret
endp

; Logical width of a single-line string in a font.
proc TextWidth uses ebx,str,font
    local rc RECT
    local hdc:DWORD
    mov eax,[gfxDC]
    mov [hdc],eax
    cmp [gfx],0
    je @f
    invoke GdipGetDC,[gfx],addr hdc
@@:
    invoke SelectObject,[hdc],[font]
    mov ebx,eax
    mov [rc.left],0
    mov [rc.top],0
    mov [rc.right],0
    mov [rc.bottom],0
    invoke DrawTextW,[hdc],[str],-1,addr rc,DT_CALCRECT+DT_SINGLELINE+DT_NOPREFIX
    invoke SelectObject,[hdc],ebx
    cmp [gfx],0
    je @f
    invoke GdipReleaseDC,[gfx],[hdc]
@@:
    invoke MulDiv,[rc.right],96,[dpi]
    ret
endp

; Hover label shown beside rail icons when the sidebar is collapsed.
proc PaintSideTooltip uses ebx esi edi
    local tipW:DWORD
    cmp [sideT],1024
    jne .done
    mov eax,[hoverIdx]
    cmp eax,-1
    je .done
    mov ecx,[navHotBase]
    cmp ecx,-1
    je .done
    sub eax,ecx
    jl .other
    cmp eax,NAV_COUNT
    jae .other
    mov ecx,[navLabels+eax*4]
    imul eax,NAV_STEP
    add eax,NAV_TOP
    jmp .draw
.other:
    mov eax,[hoverIdx]
    cmp eax,[folderHot]
    jne @f
    mov ecx,sFolder
    mov eax,608
    jmp .draw34
@@:
    cmp eax,[toggleHot]
    jne .done
    mov ecx,sSideExpand
    mov eax,78
    sub eax,6
    jmp .draw
.draw34:
    sub eax,1
.draw:
    mov esi,ecx
    mov edi,eax
    stdcall HoverLevel,[hoverIdx]
    test eax,eax
    jz .done
    mov ebx,eax
    stdcall TextWidth,esi,[fSmall]
    add eax,26
    mov [tipW],eax
    ; tooltip pill: ink at hover alpha, canvas-coloured text
    mov eax,ebx
    imul eax,0E6h
    shr eax,8
    stdcall WithAlpha,A_INK,eax
    stdcall FillRR,SIDEBAR_RAIL+6,edi,[tipW],NAV_H,9,eax
    stdcall Mix,T_INK,T_TIPTEXT,ebx
    stdcall TextC,0,esi,SIDEBAR_RAIL+6,edi,[tipW],NAV_H,[fSmall],eax
.done:
    ret
endp

; The rail/sidebar toggle glyph: a panel with a divider.
proc SideToggleGlyph x,y,argb
    mov eax,[x]
    add eax,4
    mov ecx,[y]
    add ecx,6
    stdcall StrokeRR,eax,ecx,16,12,3,[argb],14
    mov eax,[x]
    add eax,9
    mov ecx,[y]
    add ecx,7
    mov edx,[y]
    add edx,17
    stdcall LineA,eax,ecx,eax,edx,[argb],14
    ret
endp
