; OFFSET UI graphics layer: antialiased GDI+ shapes, motion engine and hit areas.
; All public coordinates are logical 96-DPI units on the 1120x760 design canvas.
; Text keeps GDI ClearType rendering; shapes use the GDI+ flat API.

; ---- Palette. A_* are GDI+ ARGB, T_* are GDI COLORREF (0x00BBGGRR).
A_CANVAS      = 0FFF5F6F8h
A_SIDEBAR     = 0FFEEF0F4h
A_SIDELINE    = 0FFDFE3E9h
A_CARD        = 0FFFFFFFFh
A_HAIRLINE    = 0FFE3E6EBh
A_DIVIDER     = 0FFECEEF2h
A_FIELD       = 0FFFFFFFFh
A_FIELDLINE   = 0FFE7EBF2h
A_TRACK       = 0FFE8EAEFh
A_ACCENT      = 0FF0071E3h
A_ACCENT_HI   = 0FF1A84F0h
A_ACCENT_LO   = 0FF0062C6h
A_ACCENT_SOFT = 0FFE5F0FDh
A_SECOND      = 0FFE6E8ECh
A_SECOND_HI   = 0FFDCDFE5h
A_SECOND_LO   = 0FFD2D6DDh
A_INK         = 0FF1D1D1Fh
A_MUTED       = 0FF8A8F98h
A_DOT         = 0FFD3D8E0h
A_CROSS       = 0FFC9CFD9h
A_GREEN       = 0FF30B455h
A_ORANGE      = 0FFF2A20Ah
A_BLUEPRINT1  = 0FF0D3168h
A_BLUEPRINT2  = 0FF071B3Eh
A_ICONBG1     = 0FF2A2C32h
A_ICONBG2     = 0FF060607h
A_MARK        = 0FFE3E8ECh

T_INK         = 01F1D1Dh
T_SECOND      = 0736E6Eh
T_MUTED       = 0A09891h
T_ACCENT      = 0E37100h
T_WHITE       = 0FFFFFFh
T_CANVAS      = 0F8F6F5h

SIDEBAR_W = 220
NAV_TOP = 128
NAV_STEP = 44
NAV_H = 36
HOT_CAPACITY = 64
ANIM_SLOTS = 10
TRANSITION_MS = 300
HERO_MS = 1100

section '.gfxd' data readable writeable
gdipToken dd 0
gdipInput dd 1,0,0,0
gfx dd 0
gfxDC dd 0
gOffX dd 0
gOffY dd 0
gScale dd 1.0
gTemp dd 0
gTemp2 dd 0
f0 dd 0.0
f90 dd 90.0
f180 dd 180.0
f270 dd 270.0
f360 dd 360.0
fHalf dd 0.5
fTen dd 10.0
fThousand dd 1000.0
qpcFreq dq 0
qpcNow dq 0
; Springs: current and target in 1/16 logical px.
animCur dd ANIM_SLOTS dup(0)
animTgt dd ANIM_SLOTS dup(0)
animTimer dd 0
animVsync dd 0                  ; frame loop paced by DWM while interface moves
lastLoopFrame dd 0
gridMinorPath dd 0
gridMajorPath dd 0
gridMinorPen dd 0
gridMajorPen dd 0
; Hover and press feedback for custom-drawn controls.
hotCount dd 0
hotRects dd HOT_CAPACITY*4 dup(0)
hoverIdx dd -1
hoverAmt dd 0
fadeIdx dd -1
fadeAmt dd 0
pressIdx dd -1
mouseX dd -1
mouseY dd -1
; Page transition and hero drawing.
transActive dd 0
transStart dd 0
transEase dd 1024
heroStart dd 0
heroActive dd 0
statusStart dd 0
; Back buffers (device pixels).
bufW dd 0
bufH dd 0
bbDC dd 0
bbBmp dd 0
bbOld dd 0
bgDC dd 0
bgBmp dd 0
bgOld dd 0
snapDC dd 0
snapBmp dd 0
snapOld dd 0
handCursor dd 0
arrowCursor dd 0
dashPattern dd 3.0,3.0
include 'logo_data.inc'
logoPointsF dd logoPointTotal*2 dup(?)

section '.gfxc' code readable executable

; ---------------------------------------------------------------- basics
proc GfxStartup
    invoke GdiplusStartup,gdipToken,gdipInput,0
    stdcall GridBuild
    invoke QueryPerformanceFrequency,qpcFreq
    ; scale = dpi / 96
    fild [dpi]
    mov [gTemp],96
    fidiv [gTemp]
    fstp [gScale]
    invoke LoadCursorW,0,IDC_HAND
    mov [handCursor],eax
    invoke LoadCursorW,0,IDC_ARROW
    mov [arrowCursor],eax
    ret
endp

; Milliseconds from the high-resolution counter (wraps harmlessly).
proc NowMs
    invoke QueryPerformanceCounter,qpcNow
    fild [qpcNow]
    fild [qpcFreq]
    fdivp st1,st0
    fmul [fThousand]
    fistp [qpcNow]
    mov eax,dword [qpcNow]
    ret
endp

; Ease-out cubic on 0..1024.
proc EaseOut t
    mov eax,[t]
    cmp eax,1024
    jge .full
    test eax,eax
    jle .zero
    mov ecx,1024
    sub ecx,eax
    mov eax,ecx
    imul eax,ecx
    shr eax,10
    imul eax,ecx
    shr eax,10
    mov ecx,1024
    sub ecx,eax
    mov eax,ecx
    ret
.full:
    mov eax,1024
    ret
.zero:
    xor eax,eax
    ret
endp

; Channel-wise blend for ARGB or COLORREF. t = 0..256 (0 = a, 256 = b).
proc Mix uses ebx esi edi,a,b,t
    xor edi,edi
    xor esi,esi
.channel:
    mov eax,[a]
    mov ecx,esi
    shr eax,cl
    and eax,0FFh
    mov ebx,[b]
    shr ebx,cl
    and ebx,0FFh
    sub ebx,eax
    imul ebx,[t]
    sar ebx,8
    add eax,ebx
    and eax,0FFh
    shl eax,cl
    or edi,eax
    add esi,8
    cmp esi,32
    jb .channel
    mov eax,edi
    ret
endp

; Replace the alpha channel of an ARGB color.
proc WithAlpha color,alpha
    mov eax,[color]
    and eax,0FFFFFFh
    mov ecx,[alpha]
    shl ecx,24
    or eax,ecx
    ret
endp

; Logical int (+ tenths) -> float bits, with the current draw offset applied.
proc FX value,tenths
    mov eax,[value]
    add eax,[gOffX]
    imul eax,10
    add eax,[tenths]
    mov [gTemp],eax
    fild [gTemp]
    fdiv [fTen]
    fstp [gTemp]
    mov eax,[gTemp]
    ret
endp

proc FY value,tenths
    mov eax,[value]
    add eax,[gOffY]
    imul eax,10
    add eax,[tenths]
    mov [gTemp],eax
    fild [gTemp]
    fdiv [fTen]
    fstp [gTemp]
    mov eax,[gTemp]
    ret
endp

; Plain length (no offset), in tenths -> float bits.
proc FL tenths
    fild [tenths]
    fdiv [fTen]
    fstp [tenths]
    mov eax,[tenths]
    ret
endp

proc FI value
    fild [value]
    fstp [value]
    mov eax,[value]
    ret
endp

; Begin GDI+ drawing on a DC with the DPI scale as world transform.
proc GfxBegin dc
    mov eax,[dc]
    mov [gfxDC],eax
    invoke GdipCreateFromHDC,[dc],gfx
    invoke GdipSetSmoothingMode,[gfx],4
    invoke GdipSetPixelOffsetMode,[gfx],4
    invoke GdipScaleWorldTransform,[gfx],[gScale],[gScale],0
    ret
endp

proc GfxEnd
    cmp [gfx],0
    je .done
    invoke GdipDeleteGraphics,[gfx]
    mov [gfx],0
.done:
    ret
endp

; Rounded rectangle path. inset10 shrinks every edge by tenths of a pixel.
proc RRPath uses ebx esi edi,x,y,w,h,r,inset10
    locals
        path dd ?
        fx dd ?
        fy dd ?
        fx2 dd ?
        fy2 dd ?
        fd dd ?
        fw dd ?
        fh dd ?
    endl
    invoke GdipCreatePath,0,addr path
    stdcall FX,[x],[inset10]
    mov [fx],eax
    stdcall FY,[y],[inset10]
    mov [fy],eax
    mov eax,[w]
    imul eax,10
    mov ecx,[inset10]
    shl ecx,1
    sub eax,ecx
    stdcall FL,eax
    mov [fw],eax
    mov eax,[h]
    imul eax,10
    mov ecx,[inset10]
    shl ecx,1
    sub eax,ecx
    stdcall FL,eax
    mov [fh],eax
    ; clamp radius to half the shorter side
    mov eax,[r]
    mov ecx,[w]
    shr ecx,1
    cmp eax,ecx
    jle @f
    mov eax,ecx
@@:
    mov ecx,[h]
    shr ecx,1
    cmp eax,ecx
    jle @f
    mov eax,ecx
@@:
    test eax,eax
    jle .rect
    imul eax,20
    sub eax,[inset10]
    jg @f
    mov eax,1
@@:
    stdcall FL,eax
    mov [fd],eax
    ; x2 = x + w - d, y2 = y + h - d
    fld [fx]
    fadd [fw]
    fsub [fd]
    fstp [fx2]
    fld [fy]
    fadd [fh]
    fsub [fd]
    fstp [fy2]
    invoke GdipAddPathArc,[path],[fx],[fy],[fd],[fd],[f180],[f90]
    invoke GdipAddPathArc,[path],[fx2],[fy],[fd],[fd],[f270],[f90]
    invoke GdipAddPathArc,[path],[fx2],[fy2],[fd],[fd],[f0],[f90]
    invoke GdipAddPathArc,[path],[fx],[fy2],[fd],[fd],[f90],[f90]
    invoke GdipClosePathFigure,[path]
    mov eax,[path]
    ret
.rect:
    invoke GdipAddPathRectangle,[path],[fx],[fy],[fw],[fh]
    mov eax,[path]
    ret
endp

proc FillRR uses ebx esi,x,y,w,h,r,argb
    local brush:DWORD
    cmp [gfx],0
    je .done
    stdcall RRPath,[x],[y],[w],[h],[r],0
    mov ebx,eax
    invoke GdipCreateSolidFill,[argb],addr brush
    invoke GdipFillPath,[gfx],[brush],ebx
    invoke GdipDeleteBrush,[brush]
    invoke GdipDeletePath,ebx
.done:
    ret
endp

; Vertical gradient rounded rectangle.
proc FillRRGrad uses ebx esi,x,y,w,h,r,top,bottom
    locals
        brush dd ?
        p1x dd ?
        p1y dd ?
        p2x dd ?
        p2y dd ?
    endl
    cmp [gfx],0
    je .done
    stdcall RRPath,[x],[y],[w],[h],[r],0
    mov ebx,eax
    mov eax,[x]
    add eax,[gOffX]
    mov [p1x],eax
    mov [p2x],eax
    mov eax,[y]
    add eax,[gOffY]
    dec eax
    mov [p1y],eax
    add eax,[h]
    add eax,2
    mov [p2y],eax
    lea eax,[p1x]
    lea ecx,[p2x]
    invoke GdipCreateLineBrushI,eax,ecx,[top],[bottom],3,addr brush
    invoke GdipFillPath,[gfx],[brush],ebx
    invoke GdipDeleteBrush,[brush]
    invoke GdipDeletePath,ebx
.done:
    ret
endp

proc MakePen argb,pw10
    local pen:DWORD
    stdcall FL,[pw10]
    invoke GdipCreatePen1,[argb],eax,0,addr pen
    invoke GdipSetPenLineCap197819,[pen],2,2,2
    invoke GdipSetPenLineJoin,[pen],2
    mov eax,[pen]
    ret
endp

proc StrokeRR uses ebx esi,x,y,w,h,r,argb,pw10
    cmp [gfx],0
    je .done
    mov eax,[pw10]
    shr eax,1
    stdcall RRPath,[x],[y],[w],[h],[r],eax
    mov ebx,eax
    stdcall MakePen,[argb],[pw10]
    mov esi,eax
    invoke GdipDrawPath,[gfx],esi,ebx
    invoke GdipDeletePen,esi
    invoke GdipDeletePath,ebx
.done:
    ret
endp

; Clip all following shapes to a rounded rectangle (GfxUnclip to reset).
proc ClipRR uses ebx,x,y,w,h,r
    stdcall RRPath,[x],[y],[w],[h],[r],0
    mov ebx,eax
    invoke GdipSetClipPath,[gfx],ebx,0
    invoke GdipDeletePath,ebx
    ret
endp

proc GfxUnclip
    invoke GdipResetClip,[gfx]
    ret
endp

; Straight line; 1px lines are nudged onto pixel centres for crisp hairlines.
proc LineA uses ebx esi,x1,y1,x2,y2,argb,pw10
    locals
        ax1 dd ?
        ay1 dd ?
        ax2 dd ?
        ay2 dd ?
    endl
    cmp [gfx],0
    je .done
    xor esi,esi
    cmp [pw10],10
    ja @f
    mov esi,5
@@:
    stdcall FX,[x1],esi
    mov [ax1],eax
    stdcall FY,[y1],esi
    mov [ay1],eax
    stdcall FX,[x2],esi
    mov [ax2],eax
    stdcall FY,[y2],esi
    mov [ay2],eax
    stdcall MakePen,[argb],[pw10]
    mov ebx,eax
    invoke GdipDrawLine,[gfx],ebx,[ax1],[ay1],[ax2],[ay2]
    invoke GdipDeletePen,ebx
.done:
    ret
endp

proc DashLine uses ebx,x1,y1,x2,y2,argb,pw10
    locals
        ax1 dd ?
        ay1 dd ?
        ax2 dd ?
        ay2 dd ?
    endl
    stdcall FX,[x1],5
    mov [ax1],eax
    stdcall FY,[y1],5
    mov [ay1],eax
    stdcall FX,[x2],5
    mov [ax2],eax
    stdcall FY,[y2],5
    mov [ay2],eax
    stdcall MakePen,[argb],[pw10]
    mov ebx,eax
    invoke GdipSetPenLineCap197819,ebx,0,0,0
    invoke GdipSetPenDashArray,ebx,dashPattern,2
    invoke GdipDrawLine,[gfx],ebx,[ax1],[ay1],[ax2],[ay2]
    invoke GdipDeletePen,ebx
    ret
endp

proc FillCircle uses ebx,cx_,cy_,r,argb
    locals
        brush dd ?
        fx dd ?
        fy dd ?
        fd dd ?
    endl
    cmp [gfx],0
    je .done
    mov eax,[cx_]
    sub eax,[r]
    stdcall FX,eax,0
    mov [fx],eax
    mov eax,[cy_]
    sub eax,[r]
    stdcall FY,eax,0
    mov [fy],eax
    mov eax,[r]
    imul eax,20
    stdcall FL,eax
    mov [fd],eax
    invoke GdipCreateSolidFill,[argb],addr brush
    invoke GdipFillEllipse,[gfx],[brush],[fx],[fy],[fd],[fd]
    invoke GdipDeleteBrush,[brush]
.done:
    ret
endp

; Arc / circle outline. start and sweep are degrees; dashed when dash<>0.
proc ArcA uses ebx,cx_,cy_,r,start,sweep,argb,pw10,dash
    locals
        fx dd ?
        fy dd ?
        fd dd ?
        fsc dd ?
        fw dd ?
    endl
    cmp [gfx],0
    je .done
    cmp [sweep],0
    je .done
    mov eax,[cx_]
    sub eax,[r]
    stdcall FX,eax,0
    mov [fx],eax
    mov eax,[cy_]
    sub eax,[r]
    stdcall FY,eax,0
    mov [fy],eax
    mov eax,[r]
    imul eax,20
    stdcall FL,eax
    mov [fd],eax
    stdcall FI,[start]
    mov [fsc],eax
    stdcall FI,[sweep]
    mov [fw],eax
    stdcall MakePen,[argb],[pw10]
    mov ebx,eax
    cmp [dash],0
    je .draw
    invoke GdipSetPenLineCap197819,ebx,0,0,0
    invoke GdipSetPenDashArray,ebx,dashPattern,2
.draw:
    invoke GdipDrawArc,[gfx],ebx,[fx],[fy],[fd],[fd],[fsc],[fw]
    invoke GdipDeletePen,ebx
.done:
    ret
endp

;-- Build a path for the OFFSET mark at (x,y) in a size x size box.
; Points are transformed here (no world-transform stacking).
proc LogoPath uses ebx esi edi,x,y,size
    locals
        path dd ?
        ox dd ?
        oy dd ?
        k dd ?
    endl
    stdcall FX,[x],0
    mov [ox],eax
    stdcall FY,[y],0
    mov [oy],eax
    fild [size]
    fdiv [fThousand]
    fstp [k]
    mov esi,logoPoints
    mov edi,logoPointsF
    mov ecx,logoPointTotal
.point:
    fild dword [esi]
    fmul [k]
    fadd [ox]
    fstp dword [edi]
    fild dword [esi+4]
    fmul [k]
    fadd [oy]
    fstp dword [edi+4]
    add esi,8
    add edi,8
    dec ecx
    jnz .point
    invoke GdipCreatePath,0,addr path
    mov esi,logoPointsF
    xor ebx,ebx
.ring:
    cmp ebx,logoRingCount
    jae .done
    mov edi,[logoRingSizes+ebx*4]
    invoke GdipAddPathPolygon,[path],esi,edi
    shl edi,3
    add esi,edi
    inc ebx
    jmp .ring
.done:
    mov eax,[path]
    ret
endp

; Draw the OFFSET mark inside a size x size box. Filled when pw10 = 0,
; outlined otherwise. sweep < 360 reveals it through a pie centred on the mark.
proc DrawLogo uses ebx esi,x,y,size,argb,pw10,sweep
    locals
        state dd ?
        brush dd ?
        fx dd ?
        fy dd ?
        clip dd ?
        fw dd ?
        fsw dd ?
        fr dd ?
    endl
    cmp [gfx],0
    je .done
    cmp [sweep],0
    jle .done
    invoke GdipSaveGraphics,[gfx],addr state
    cmp [sweep],360
    jge .noclip
    invoke GdipCreatePath,0,addr clip
    mov eax,[size]
    shr eax,1
    mov ecx,[x]
    sub ecx,eax
    stdcall FX,ecx,0
    mov [fx],eax
    mov eax,[size]
    shr eax,1
    mov ecx,[y]
    sub ecx,eax
    stdcall FY,ecx,0
    mov [fy],eax
    mov eax,[size]
    imul eax,20
    stdcall FL,eax
    mov [fw],eax
    stdcall FI,[sweep]
    mov [fsw],eax
    mov [gTemp2],-90
    stdcall FI,[gTemp2]
    invoke GdipAddPathPie,[clip],[fx],[fy],[fw],[fw],eax,[fsw]
    invoke GdipSetClipPath,[gfx],[clip],1
    invoke GdipDeletePath,[clip]
.noclip:
    stdcall LogoPath,[x],[y],[size]
    mov ebx,eax
    cmp [pw10],0
    jne .stroke
    invoke GdipCreateSolidFill,[argb],addr brush
    invoke GdipFillPath,[gfx],[brush],ebx
    invoke GdipDeleteBrush,[brush]
    jmp .release
.stroke:
    stdcall FL,[pw10]
    invoke GdipCreatePen1,[argb],eax,0,addr brush
    invoke GdipDrawPath,[gfx],[brush],ebx
    invoke GdipDeletePen,[brush]
.release:
    invoke GdipDeletePath,ebx
    invoke GdipRestoreGraphics,[gfx],[state]
.done:
    ret
endp

; ---------------------------------------------------------------- composites
proc Shadow uses ebx esi,x,y,w,h,r
    mov eax,[x]
    sub eax,2
    mov ecx,[y]
    add ecx,1
    mov edx,[w]
    add edx,4
    push edx
    mov edx,[h]
    add edx,5
    mov ebx,[r]
    add ebx,2
    pop esi
    stdcall FillRR,eax,ecx,esi,edx,ebx,05000000h
    stdcall FillRR,[x],[y],[w],[h],[r],03000000h
    mov eax,[y]
    inc eax
    stdcall FillRR,[x],eax,[w],[h],[r],06000000h
    ret
endp

; White card with soft shadow and hairline.
proc Card uses ebx esi,x,y,w,h,r
    stdcall Shadow,[x],[y],[w],[h],[r]
    stdcall FillRR,[x],[y],[w],[h],[r],A_CARD
    stdcall StrokeRR,[x],[y],[w],[h],[r],A_HAIRLINE,10
    ret
endp

; Blueprint registration ticks just outside the four corners of a box.
proc CornerTicks x,y,w,h,argb
    locals
        x2 dd ?
        y2 dd ?
    endl
    mov eax,[x]
    add eax,[w]
    mov [x2],eax
    mov eax,[y]
    add eax,[h]
    mov [y2],eax
    mov eax,[x]
    sub eax,10
    mov ecx,[x]
    sub ecx,4
    stdcall LineA,eax,[y],ecx,[y],[argb],10
    mov eax,[y]
    sub eax,10
    mov ecx,[y]
    sub ecx,4
    stdcall LineA,[x],eax,[x],ecx,[argb],10
    mov eax,[x2]
    add eax,4
    mov ecx,[x2]
    add ecx,10
    stdcall LineA,eax,[y],ecx,[y],[argb],10
    mov eax,[y]
    sub eax,10
    mov ecx,[y]
    sub ecx,4
    stdcall LineA,[x2],eax,[x2],ecx,[argb],10
    mov eax,[x]
    sub eax,10
    mov ecx,[x]
    sub ecx,4
    stdcall LineA,eax,[y2],ecx,[y2],[argb],10
    mov eax,[y2]
    add eax,4
    mov ecx,[y2]
    add ecx,10
    stdcall LineA,[x],eax,[x],ecx,[argb],10
    mov eax,[x2]
    add eax,4
    mov ecx,[x2]
    add ecx,10
    stdcall LineA,eax,[y2],ecx,[y2],[argb],10
    mov eax,[y2]
    add eax,4
    mov ecx,[y2]
    add ecx,10
    stdcall LineA,[x2],eax,[x2],ecx,[argb],10
    ret
endp

; Text input well drawn behind a native EDIT/COMBO. Shows a focus ring.
proc Field uses ebx,x,y,w,h,control
    invoke GetFocus
    mov ebx,eax
    cmp [control],0
    je .plain
if defined hJvmArgs
    ; The multiline JVM field owns a large canvas card; do not add a
    ; second focus halo outside that card.
    mov eax,[hJvmArgs]
    cmp [control],eax
    je .plain
end if
    cmp ebx,[control]
    je .focused
    ; a combo box's edit child also counts
    invoke GetParent,ebx
    cmp eax,[control]
    je .focused
.plain:
    stdcall FillRR,[x],[y],[w],[h],10,A_FIELD
    stdcall StrokeRR,[x],[y],[w],[h],10,A_FIELDLINE,10
    ret
.focused:
    mov eax,[x]
    sub eax,3
    mov ecx,[y]
    sub ecx,3
    mov edx,[w]
    add edx,6
    push edx
    mov edx,[h]
    add edx,6
    pop ebx
    stdcall FillRR,eax,ecx,ebx,edx,13,[themeFocusARGB]
    stdcall FillRR,[x],[y],[w],[h],10,A_FIELD
    stdcall StrokeRR,[x],[y],[w],[h],10,[themeAccentARGB],12
    ret
endp

; Small status dot.
proc Dot cx_,cy_,argb
    stdcall FillCircle,[cx_],[cy_],4,[argb]
    ret
endp

; ---------------------------------------------------------------- springs
proc AnimSet slot,target,immediate
    mov ecx,[slot]
    mov eax,[target]
    shl eax,4
    mov [animTgt+ecx*4],eax
    cmp [themeMotion],0
    je .immediate
    cmp [immediate],0
    je .kick
.immediate:
    mov [animCur+ecx*4],eax
    ret
.kick:
    cmp eax,[animCur+ecx*4]
    je .done
    stdcall AnimKick
.done:
    ret
endp

proc AnimGet slot
    mov ecx,[slot]
    mov eax,[animCur+ecx*4]
    sar eax,4
    ret
endp

proc AnimKick
    cmp [themeMotion],0
    jne .enabled
    cmp [hWnd],0
    je .done
    invoke InvalidateRect,[hWnd],0,0
    ret
.enabled:
    mov [animVsync],1
    cmp [animTimer],0
    jne .done
    cmp [hWnd],0
    je .done
    invoke SetTimer,[hWnd],1,15,0
    mov [animTimer],1
.done:
    ret
endp

; Returns non-zero while busy indicators should animate (window visible and active).
proc BusyAnimating
    cmp [themeMotion],0
    je .no
    cmp [busy],0
    je .no
    invoke IsIconic,[hWnd]
    test eax,eax
    jnz .no
    invoke GetForegroundWindow
    cmp eax,[hWnd]
    jne .no
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

; Advance every animation one frame. Returns non-zero while anything moves.
proc AnimTick uses ebx esi edi
    cmp [themeMotion],0
    jne .enabled
    stdcall ThemeMotionSync
    xor eax,eax
    ret
.enabled:
    xor edi,edi
    xor esi,esi
.spring:
    mov eax,[animTgt+esi*4]
    sub eax,[animCur+esi*4]
    jz .next
    mov ecx,eax
    cdq
    xor eax,edx
    sub eax,edx
    cmp eax,3
    jbe .snap
    mov eax,ecx
    imul eax,9
    sar eax,5
    test eax,eax
    jnz .step
    mov eax,1
    test ecx,ecx
    jns .step
    neg eax
.step:
    add [animCur+esi*4],eax
    or edi,1
    jmp .next
.snap:
    mov eax,[animTgt+esi*4]
    mov [animCur+esi*4],eax
    or edi,1
.next:
    inc esi
    cmp esi,ANIM_SLOTS
    jb .spring
    ; hover fade in / out
    cmp [hoverIdx],-1
    je .fade
    cmp [hoverAmt],256
    jae .fade
    add [hoverAmt],40
    cmp [hoverAmt],256
    jbe @f
    mov [hoverAmt],256
@@:
    or edi,1
.fade:
    cmp [fadeAmt],0
    je .trans
    sub [fadeAmt],32
    jns @f
    mov [fadeAmt],0
@@:
    or edi,1
.trans:
    cmp [transActive],0
    je .hero
    or edi,1
.hero:
    cmp [heroActive],0
    je .status
    or edi,1
.status:
    cmp [statusStart],0
    je .splash
    or edi,1
.splash:
    ; startup sequence and the hero text fade that follows it
    mov eax,[splashActive]
    or eax,[heroTextStart]
    jz .busy
    or edi,1
.busy:
    ; bit 0: interface motion, bit 1: busy indicators only
    stdcall BusyAnimating
    shl eax,1
    or edi,eax
    mov eax,edi
    ret
endp

; ---------------------------------------------------------------- hit areas
proc HotReset
    mov [hotCount],0
    ret
endp

; Registers a clickable rectangle; returns its index (or -1 when full).
proc HotAdd x,y,w,h
    mov eax,[hotCount]
    cmp eax,HOT_CAPACITY
    jae .full
    mov ecx,eax
    shl ecx,4
    mov edx,[x]
    add edx,[gOffX]
    mov [hotRects+ecx],edx
    mov edx,[y]
    add edx,[gOffY]
    mov [hotRects+ecx+4],edx
    mov edx,[w]
    mov [hotRects+ecx+8],edx
    mov edx,[h]
    mov [hotRects+ecx+12],edx
    inc [hotCount]
    ret
.full:
    mov eax,-1
    ret
endp

proc HotFind uses ebx,x,y
    xor ebx,ebx
.next:
    cmp ebx,[hotCount]
    jae .none
    mov ecx,ebx
    shl ecx,4
    mov eax,[x]
    sub eax,[hotRects+ecx]
    jl .skip
    cmp eax,[hotRects+ecx+8]
    jge .skip
    mov eax,[y]
    sub eax,[hotRects+ecx+4]
    jl .skip
    cmp eax,[hotRects+ecx+12]
    jge .skip
    mov eax,ebx
    ret
.skip:
    inc ebx
    jmp .next
.none:
    mov eax,-1
    ret
endp

; Hover level 0..256 for a registered index.
proc HoverLevel idx
    mov eax,[idx]
    cmp eax,-1
    je .none
    cmp [themeMotion],0
    jne .animated
    cmp eax,[hoverIdx]
    jne .none
    mov eax,256
    ret
.animated:
    cmp eax,[hoverIdx]
    jne .fading
    mov eax,[hoverAmt]
    ret
.fading:
    cmp eax,[fadeIdx]
    jne .none
    mov eax,[fadeAmt]
    ret
.none:
    xor eax,eax
    ret
endp

proc MouseMoved uses ebx,x,y
    mov eax,[x]
    mov [mouseX],eax
    mov eax,[y]
    mov [mouseY],eax
    stdcall HotFind,[x],[y]
    cmp eax,[hoverIdx]
    je .done
    mov ebx,eax
    mov eax,[hoverIdx]
    mov [fadeIdx],eax
    mov eax,[hoverAmt]
    mov [fadeAmt],eax
    mov [hoverIdx],ebx
    mov [hoverAmt],0
    stdcall AnimKick
.done:
    ret
endp

proc MouseLeft
    mov eax,[hoverIdx]
    cmp eax,-1
    je .done
    mov [fadeIdx],eax
    mov eax,[hoverAmt]
    mov [fadeAmt],eax
    mov [hoverIdx],-1
    mov [hoverAmt],0
    mov [pressIdx],-1
    stdcall AnimKick
.done:
    mov [mouseX],-1
    ret
endp

proc HoverReset
    mov [hoverIdx],-1
    mov [hoverAmt],0
    mov [fadeIdx],-1
    mov [fadeAmt],0
    mov [pressIdx],-1
    ret
endp

; ---------------------------------------------------------------- buffers
proc FreeBuffers
    cmp [bbDC],0
    je .bg
    invoke SelectObject,[bbDC],[bbOld]
    invoke DeleteObject,[bbBmp]
    invoke DeleteDC,[bbDC]
    mov [bbDC],0
.bg:
    cmp [bgDC],0
    je .snap
    invoke SelectObject,[bgDC],[bgOld]
    invoke DeleteObject,[bgBmp]
    invoke DeleteDC,[bgDC]
    mov [bgDC],0
.snap:
    cmp [snapDC],0
    je .done
    invoke SelectObject,[snapDC],[snapOld]
    invoke DeleteObject,[snapBmp]
    invoke DeleteDC,[snapDC]
    mov [snapDC],0
.done:
    ret
endp

proc EnsureBuffers uses ebx esi edi,screen,w,h
    mov eax,[w]
    cmp eax,[bufW]
    jne .rebuild
    mov eax,[h]
    cmp eax,[bufH]
    jne .rebuild
    cmp [bbDC],0
    jne .done
.rebuild:
    stdcall FreeBuffers
    mov [splashUiReady],0
    mov eax,[w]
    mov [bufW],eax
    mov eax,[h]
    mov [bufH],eax
    invoke CreateCompatibleDC,[screen]
    mov [bbDC],eax
    invoke CreateCompatibleBitmap,[screen],[w],[h]
    mov [bbBmp],eax
    invoke SelectObject,[bbDC],eax
    mov [bbOld],eax
    invoke CreateCompatibleDC,[screen]
    mov [bgDC],eax
    invoke CreateCompatibleBitmap,[screen],[w],[h]
    mov [bgBmp],eax
    invoke SelectObject,[bgDC],eax
    mov [bgOld],eax
    invoke CreateCompatibleDC,[screen]
    mov [snapDC],eax
    invoke CreateCompatibleBitmap,[screen],[w],[h]
    mov [snapBmp],eax
    invoke SelectObject,[snapDC],eax
    mov [snapOld],eax
    stdcall PaintBackdrop
.done:
    ret
endp

; Static canvas: sidebar wash and the blueprint dot grid. Rendered once.
proc PaintBackdrop uses ebx esi edi
    mov eax,[gOffX]
    push eax
    mov eax,[gOffY]
    push eax
    mov [gOffX],0
    mov [gOffY],0
    stdcall GfxBegin,[bgDC]
    invoke GdipSetPixelOffsetMode,[gfx],4
    stdcall FillRR,0,0,1120,760,0,A_CANVAS
    ; dot grid, every 24 units, with crosshair marks every 96
    mov esi,20
.col:
    mov edi,20
.row:
    lea eax,[esi+96*3-SIDEBAR_W-20]
    xor edx,edx
    mov ecx,96
    div ecx
    mov ebx,edx
    mov eax,edi
    sub eax,20
    xor edx,edx
    mov ecx,96
    div ecx
    or ebx,edx
    jz .cross
    stdcall FillCircle,esi,edi,1,A_DOT
    jmp .step
.cross:
    mov eax,esi
    sub eax,4
    mov ecx,esi
    add ecx,4
    stdcall LineA,eax,edi,ecx,edi,A_CROSS,10
    mov eax,edi
    sub eax,4
    mov ecx,edi
    add ecx,4
    stdcall LineA,esi,eax,esi,ecx,A_CROSS,10
.step:
    add edi,24
    cmp edi,760
    jb .row
    add esi,24
    cmp esi,1120
    jb .col
    stdcall GfxEnd
    pop eax
    mov [gOffY],eax
    pop eax
    mov [gOffX],eax
    ret
endp

; Freeze the last frame so the next page can cross-fade over it.
proc StartTransition
    cmp [themeMotion],0
    je .done
    cmp [bbDC],0
    je .done
    invoke BitBlt,[snapDC],0,0,[bufW],[bufH],[bbDC],0,0,SRCCOPY
    stdcall NowMs
    mov [transStart],eax
    mov [transActive],1
    mov [transEase],0
    stdcall HoverReset
    stdcall AnimKick
.done:
    ret
endp

; Updates transEase (0..1024). Returns 1 when the transition just finished.
proc TransitionStep
    cmp [transActive],0
    je .idle
    stdcall NowMs
    sub eax,[transStart]
    shl eax,10
    xor edx,edx
    mov ecx,TRANSITION_MS
    div ecx
    cmp eax,1024
    jb .running
    mov [transActive],0
    mov [transEase],1024
    mov eax,1
    ret
.running:
    stdcall EaseOut,eax
    mov [transEase],eax
.idle:
    xor eax,eax
    ret
endp

; ---------------------------------------------------------------- blueprint grid
; Full-canvas grid shared by the hero card and the startup sheet: 24-unit
; lines, every 4th brighter, aligned to the hero (x = 264, y = 136). Built once
; as two paths so a frame draws it with two calls instead of ~80 pens.
proc GridLine path,x1,y1,x2,y2
    locals
        a dd ?
        b dd ?
        c dd ?
        d dd ?
    endl
    ; +0.5 keeps 1px lines on pixel centres (as LineA does)
    mov eax,[x1]
    imul eax,10
    add eax,5
    stdcall FL,eax
    mov [a],eax
    mov eax,[y1]
    imul eax,10
    add eax,5
    stdcall FL,eax
    mov [b],eax
    mov eax,[x2]
    imul eax,10
    add eax,5
    stdcall FL,eax
    mov [c],eax
    mov eax,[y2]
    imul eax,10
    add eax,5
    stdcall FL,eax
    mov [d],eax
    invoke GdipStartPathFigure,[path]
    invoke GdipAddPathLine,[path],[a],[b],[c],[d]
    ret
endp

proc GridBuild uses ebx esi
    invoke GdipCreatePath,0,gridMinorPath
    invoke GdipCreatePath,0,gridMajorPath
    xor esi,esi
.gx:
    lea eax,[esi+960-264]
    xor edx,edx
    mov ecx,96
    div ecx
    mov ebx,[gridMinorPath]
    test edx,edx
    jnz @f
    mov ebx,[gridMajorPath]
@@:
    stdcall GridLine,ebx,esi,0,esi,760
    add esi,24
    cmp esi,1216
    jb .gx
    mov esi,16
.gy:
    lea eax,[esi+960-136]
    xor edx,edx
    mov ecx,96
    div ecx
    mov ebx,[gridMinorPath]
    test edx,edx
    jnz @f
    mov ebx,[gridMajorPath]
@@:
    stdcall GridLine,ebx,0,esi,1216,esi
    add esi,24
    cmp esi,760
    jb .gy
    stdcall FL,10
    invoke GdipCreatePen1,14FFFFFFh,eax,0,gridMinorPen
    stdcall FL,10
    invoke GdipCreatePen1,26FFFFFFh,eax,0,gridMajorPen
    ret
endp

; Draw the grid at the current draw offset (clip first to bound it).
proc DrawBlueprintGrid
    local state:DWORD
    cmp [gfx],0
    je .done
    invoke GdipSaveGraphics,[gfx],addr state
    stdcall FI,[gOffX]
    mov ecx,eax
    push ecx
    stdcall FI,[gOffY]
    pop ecx
    invoke GdipTranslateWorldTransform,[gfx],ecx,eax,0
    invoke GdipDrawPath,[gfx],[gridMinorPen],[gridMinorPath]
    invoke GdipDrawPath,[gfx],[gridMajorPen],[gridMajorPath]
    invoke GdipRestoreGraphics,[gfx],[state]
.done:
    ret
endp
