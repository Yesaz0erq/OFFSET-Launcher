; Startup animation. A full-window blueprint sheet appears with the OFFSET mark
; drawing itself in the centre; the sheet then contracts into the Play page's
; hero card while the mark glides into the hero's large logo on the right.
; Everything is vector (GDI+ paths); the final frame equals PaintHero exactly,
; so the hand-off is seamless. Disabled when Personalize > motion is off or
; offset.ini sets StartupAnimation=0. A click or key press skips it.

SPLASH_LEAD_MS = 140            ; let the window's own show animation finish
SPLASH_DRAW_MS = 900            ; mark outline + construction circle sweep
SPLASH_FILL_AT = 600            ; faint fill settles into the outline
SPLASH_FILL_MS = 400
SPLASH_WORD_AT = 650            ; wordmark fades in under the mark
SPLASH_MOVE_AT = 1450           ; sheet starts contracting toward the hero
SPLASH_MOVE_MS = 900
SPLASH_TOTAL = SPLASH_MOVE_AT+SPLASH_MOVE_MS
SPLASH_SNAP_AT = 1150           ; quiet moment to pre-render the interface
SPLASH_TEXT_MS = 380            ; hero text fade-in after the hand-off
; start (centre of window) and end (hero logo box from PaintHero)
SPLASH_LOGO0 = 236
SPLASH_CX0 = 560
SPLASH_CY0 = 352
SPLASH_LOGO1 = 110
SPLASH_CX1 = 960
SPLASH_CY1 = 210

section '.splashd' data readable writeable
splashActive dd 0
splashStart dd 0
splashNow dd 0
heroTextStart dd 0
splashDC dd 0
splashBmp dd 0
splashOld dd 0
splashCacheW dd 0
splashCacheH dd 0
splashCovering dd 0
splashUiReady dd 0
; contraction layers: a 1x256 gradient strip (stretched with GDI) and a
; premultiplied grid layer (AlphaBlend), so large sheets cost a blit, not a fill
splashColDC dd 0
splashColBmp dd 0
splashColOld dd 0
splashColBits dd 0
splashGridDC dd 0
splashGridBmp dd 0
splashGridOld dd 0
splashGridBits dd 0
splashGridW dd 0
splashBmi dd 40,0,0
         dw 1,32
         dd 0,0,0,0,0,0
kStartupAnimation du 'StartupAnimation',0

section '.splashc' code readable executable

; Decide before the window exists, so page controls start hidden.
proc SplashInit
    mov [splashActive],0
    cmp [themeMotion],0
    je .done
    cmp [currentPage],0
    jne .done
    invoke GetPrivateProfileIntW,sectionName,kStartupAnimation,1,iniPath
    test eax,eax
    jz .done
    mov [splashActive],1
    mov [splashStart],0
.done:
    ret
endp

; Milliseconds since the first visible frame.
proc SplashElapsed
    stdcall NowMs
    cmp [splashStart],0
    jne @f
    or eax,1
    mov [splashStart],eax
@@:
    sub eax,[splashStart]
    sub eax,SPLASH_LEAD_MS
    jns @f
    xor eax,eax
@@:
    ret
endp

; Static first-phase sheet (gradient + grid), rendered once at device size.
proc SplashEnsureCache uses ebx,screen,w,h
    cmp [splashDC],0
    je .build
    mov eax,[w]
    cmp eax,[splashCacheW]
    jne .rebuild
    mov eax,[h]
    cmp eax,[splashCacheH]
    je .done
.rebuild:
    stdcall SplashFreeCache
.build:
    mov eax,[w]
    mov [splashCacheW],eax
    mov eax,[h]
    mov [splashCacheH],eax
    invoke CreateCompatibleDC,[screen]
    mov [splashDC],eax
    invoke CreateCompatibleBitmap,[screen],[w],[h]
    mov [splashBmp],eax
    invoke SelectObject,[splashDC],eax
    mov [splashOld],eax
    stdcall GfxBegin,[splashDC]
    stdcall FillRRGrad,0,0,1120,760,0,[themeBlueprint1],[themeBlueprint2]
    stdcall DrawBlueprintGrid
    stdcall GfxEnd
    stdcall SplashBuildLayers,[screen],[w],[h]
.done:
    ret
endp

proc SplashBuildLayers uses ebx esi edi,screen,w,h
    locals
        gpBitmap dd ?
        g dd ?
    endl
    ; gradient strip: 1 x 256, top-down
    mov dword [splashBmi+4],1
    mov dword [splashBmi+8],-256
    invoke CreateDIBSection,[screen],splashBmi,0,splashColBits,0,0
    mov [splashColBmp],eax
    invoke CreateCompatibleDC,[screen]
    mov [splashColDC],eax
    invoke SelectObject,eax,[splashColBmp]
    mov [splashColOld],eax
    mov edi,[splashColBits]
    xor ebx,ebx
.strip:
    stdcall Mix,[themeBlueprint1],[themeBlueprint2],ebx
    mov [edi+ebx*4],eax
    inc ebx
    cmp ebx,256
    jb .strip
    ; grid layer: device size plus room for the content shift
    stdcall Scale,96
    add eax,[w]
    mov [splashGridW],eax
    mov dword [splashBmi+4],eax
    mov eax,[h]
    neg eax
    mov dword [splashBmi+8],eax
    invoke CreateDIBSection,[screen],splashBmi,0,splashGridBits,0,0
    mov [splashGridBmp],eax
    invoke CreateCompatibleDC,[screen]
    mov [splashGridDC],eax
    invoke SelectObject,eax,[splashGridBmp]
    mov [splashGridOld],eax
    mov eax,[splashGridW]
    shl eax,2
    invoke GdipCreateBitmapFromScan0,[splashGridW],[h],eax,0E200Bh,[splashGridBits],addr gpBitmap
    invoke GdipGetImageGraphicsContext,[gpBitmap],addr g
    invoke GdipSetSmoothingMode,[g],4
    invoke GdipSetPixelOffsetMode,[g],4
    invoke GdipScaleWorldTransform,[g],[gScale],[gScale],0
    invoke GdipDrawPath,[g],[gridMinorPen],[gridMinorPath]
    invoke GdipDrawPath,[g],[gridMajorPen],[gridMajorPath]
    invoke GdipDeleteGraphics,[g]
    invoke GdipDisposeImage,[gpBitmap]
    ret
endp

; Rounded sheet at device coordinates via the cached layers (GDI on gfx's DC).
proc SplashBlitSheet uses ebx esi edi,x,y,w,h,r
    locals
        hdc dd ?
        dx0 dd ?
        dy0 dd ?
        dx1 dd ?
        dy1 dd ?
        rgn dd ?
        shift dd ?
    endl
    mov eax,[x]
    add eax,[gOffX]
    stdcall Scale,eax
    mov [dx0],eax
    mov eax,[x]
    add eax,[gOffX]
    add eax,[w]
    stdcall Scale,eax
    mov [dx1],eax
    stdcall Scale,[y]
    mov [dy0],eax
    mov eax,[y]
    add eax,[h]
    stdcall Scale,eax
    mov [dy1],eax
    stdcall Scale,[gOffX]
    mov [shift],eax
    mov eax,[gfxDC]
    mov [hdc],eax
    invoke GdipGetDC,[gfx],addr hdc
    stdcall Scale,[r]
    shl eax,1
    mov ebx,eax
    mov eax,[dx1]
    inc eax
    mov ecx,[dy1]
    inc ecx
    invoke CreateRoundRectRgn,[dx0],[dy0],eax,ecx,ebx,ebx
    mov [rgn],eax
    invoke SelectClipRgn,[hdc],eax
    invoke SetStretchBltMode,[hdc],COLORONCOLOR
    mov esi,[dx1]
    sub esi,[dx0]
    mov edi,[dy1]
    sub edi,[dy0]
    invoke StretchBlt,[hdc],[dx0],[dy0],esi,edi,[splashColDC],0,0,1,256,SRCCOPY
    mov eax,[dx0]
    sub eax,[shift]
    invoke AlphaBlend,[hdc],[dx0],[dy0],esi,edi,[splashGridDC],eax,[dy0],esi,edi,01FF0000h
    invoke SelectClipRgn,[hdc],0
    invoke DeleteObject,[rgn]
    invoke GdipReleaseDC,[gfx],[hdc]
    ret
endp

proc SplashFreeCache
    cmp [splashColDC],0
    je @f
    invoke SelectObject,[splashColDC],[splashColOld]
    invoke DeleteObject,[splashColBmp]
    invoke DeleteDC,[splashColDC]
    mov [splashColDC],0
    invoke SelectObject,[splashGridDC],[splashGridOld]
    invoke DeleteObject,[splashGridBmp]
    invoke DeleteDC,[splashGridDC]
    mov [splashGridDC],0
@@:
    cmp [splashDC],0
    je .done
    invoke SelectObject,[splashDC],[splashOld]
    invoke DeleteObject,[splashBmp]
    invoke DeleteDC,[splashDC]
    mov [splashDC],0
.done:
    ret
endp

; Non-zero while the sheet still covers the whole window (nothing else to draw).
proc SplashCoversWindow
    xor eax,eax
    cmp [splashActive],0
    je .done
    cmp [splashStart],0
    je .yes
    stdcall SplashElapsed
    cmp eax,SPLASH_MOVE_AT
    jae .no
.yes:
    mov eax,1
    ret
.no:
    xor eax,eax
.done:
    ret
endp

; Called by the animation timer; ends the sequence on time.
proc SplashStep
    cmp [splashActive],0
    je .done
    cmp [splashStart],0
    je .done
    stdcall SplashElapsed
    cmp eax,SPLASH_TOTAL
    jb .done
    stdcall SplashFinish
.done:
    ret
endp

; Hand off to the live interface (also used to skip).
proc SplashFinish
    cmp [splashActive],0
    je .done
    mov [splashActive],0
    ; the hero already shows the finished drawing; only its text fades in
    mov [heroActive],0
    mov eax,[currentPage]
    mov [lastPage],eax
    stdcall NowMs
    or eax,1
    mov [heroTextStart],eax
    mov [splashUiReady],0
    stdcall SplashFreeCache
    stdcall SyncPage
    stdcall AnimKick
.done:
    ret
endp

; 0..1024 progress of a phase that starts at `start` and lasts `duration`.
proc SplashProg start,duration
    mov eax,[splashNow]
    sub eax,[start]
    jle .zero
    shl eax,10
    xor edx,edx
    div dword [duration]
    cmp eax,1024
    jbe .done
    mov eax,1024
.done:
    ret
.zero:
    xor eax,eax
    ret
endp

; Cubic ease-in-out on 0..1024.
proc EaseInOut t
    mov eax,[t]
    cmp eax,512
    jae .out
    mov ecx,eax
    imul eax,ecx
    shr eax,10
    imul eax,ecx
    shr eax,8
    ret
.out:
    mov ecx,1024
    sub ecx,eax
    shl ecx,1
    mov eax,ecx
    imul eax,ecx
    shr eax,10
    imul eax,ecx
    shr eax,11
    mov ecx,1024
    sub ecx,eax
    mov eax,ecx
    ret
endp

; a + (b - a) * t / 1024
proc Lerp a,b,t
    mov eax,[b]
    sub eax,[a]
    imul eax,[t]
    sar eax,10
    add eax,[a]
    ret
endp

; ARGB -> COLORREF
proc ArgbToCR color
    mov eax,[color]
    bswap eax
    shr eax,8
    ret
endp

; Hero text alpha 0..256 after the hand-off (256 when no fade is running).
proc HeroTextLevel
    cmp [heroTextStart],0
    je .full
    stdcall NowMs
    sub eax,[heroTextStart]
    cmp eax,SPLASH_TEXT_MS
    jae .finished
    shl eax,8
    xor edx,edx
    mov ecx,SPLASH_TEXT_MS
    div ecx
    ret
.finished:
    mov [heroTextStart],0
.full:
    mov eax,256
    ret
endp

; Colour for hero text: fades from the blueprint ink to its final colour.
proc HeroTextColor color
    stdcall HeroTextLevel
    cmp eax,256
    jae .plain
    push eax
    stdcall ArgbToCR,[themeBlueprint1]
    pop ecx
    stdcall Mix,eax,[color],ecx
    ret
.plain:
    mov eax,[color]
    ret
endp

; Drawn last in Paint, over sidebar and content.
proc PaintSplash uses ebx esi edi
    locals
        m dd ?
        px dd ?
        py dd ?
        pw dd ?
        ph dd ?
        pr dd ?
        size dd ?
        lcx dd ?
        lcy dd ?
        sweep dd ?
        draw dd ?
        k dd ?
        wordLvl dd ?
        bgCR dd ?
    endl
    cmp [splashActive],0
    je .done
    stdcall SplashElapsed
    mov [splashNow],eax
    ; phases
    stdcall SplashProg,0,SPLASH_DRAW_MS
    stdcall EaseOut,eax
    mov [draw],eax
    imul eax,360
    shr eax,10
    mov [sweep],eax
    stdcall SplashProg,SPLASH_MOVE_AT,SPLASH_MOVE_MS
    stdcall EaseInOut,eax
    mov [m],eax
    ; land on the hero wherever the (collapsible) sidebar has put it
    stdcall Lerp,0,[contentX],eax
    mov [gOffX],eax
    ; sheet: whole window -> hero card
    stdcall Lerp,0,252,[m]
    mov [px],eax
    stdcall Lerp,0,124,[m]
    mov [py],eax
    stdcall Lerp,1120,836,[m]
    mov [pw],eax
    stdcall Lerp,760,172,[m]
    mov [ph],eax
    stdcall Lerp,0,22,[m]
    mov [pr],eax
    ; mark: centre -> hero logo
    stdcall Lerp,SPLASH_LOGO0,SPLASH_LOGO1,[m]
    mov [size],eax
    stdcall Lerp,SPLASH_CX0,SPLASH_CX1,[m]
    mov [lcx],eax
    stdcall Lerp,SPLASH_CY0,SPLASH_CY1,[m]
    mov [lcy],eax
    ; first phase: Paint already blitted the cached sheet
    cmp [splashCovering],0
    jne .construction
    ; contraction: cached gradient + grid layers (the grid stays aligned with
    ; the hero's, so contraction only crops it)
    stdcall SplashBlitSheet,[px],[py],[pw],[ph],[pr]
    stdcall ClipRR,[px],[py],[pw],[ph],[pr]
.construction:
    ; k = size / 110 in 1/1024 units, for scaling the construction geometry
    mov eax,[size]
    shl eax,10
    xor edx,edx
    mov ecx,SPLASH_LOGO1
    div ecx
    mov [k],eax
    ; construction axes (fade in with the drawing)
    mov eax,[draw]
    imul eax,40h
    shr eax,10
    stdcall WithAlpha,0FFFFFFh,eax
    mov ebx,eax
    mov eax,[lcx]
    sub eax,[size]
    mov ecx,[lcx]
    add ecx,[size]
    stdcall DashLine,eax,[lcy],ecx,[lcy],ebx,10
    mov eax,80
    imul eax,[k]
    shr eax,10
    mov edi,eax
    mov eax,[lcy]
    sub eax,edi
    mov ecx,[lcy]
    add ecx,edi
    stdcall DashLine,[lcx],eax,[lcx],ecx,ebx,10
    ; construction circles
    mov eax,76
    imul eax,[k]
    shr eax,10
    mov edi,eax
    stdcall ArcA,[lcx],[lcy],edi,-90,[sweep],66FFFFFFh,10,1
    mov eax,58
    imul eax,[k]
    shr eax,10
    stdcall ArcA,[lcx],[lcy],eax,90,[sweep],30FFFFFFh,10,0
    ; quadrant ticks (hero: 72..82 from centre)
    mov eax,72
    imul eax,[k]
    shr eax,10
    mov esi,eax
    mov eax,82
    imul eax,[k]
    shr eax,10
    mov edi,eax
    cmp [draw],1024
    jb .mark
    mov eax,[lcy]
    sub eax,edi
    mov ecx,[lcy]
    sub ecx,esi
    stdcall LineA,[lcx],eax,[lcx],ecx,99FFFFFFh,10
    mov eax,[lcy]
    add eax,esi
    mov ecx,[lcy]
    add ecx,edi
    stdcall LineA,[lcx],eax,[lcx],ecx,99FFFFFFh,10
    mov eax,[lcx]
    sub eax,edi
    mov ecx,[lcx]
    sub ecx,esi
    stdcall LineA,eax,[lcy],ecx,[lcy],99FFFFFFh,10
    mov eax,[lcx]
    add eax,esi
    mov ecx,[lcx]
    add ecx,edi
    stdcall LineA,eax,[lcy],ecx,[lcy],99FFFFFFh,10
.mark:
    ; the mark: outline sweeps in, then the hero's faint fill settles
    mov eax,[size]
    shr eax,1
    mov esi,[lcx]
    sub esi,eax
    mov edi,[lcy]
    sub edi,eax
    stdcall Lerp,18,12,[m]
    stdcall DrawLogo,esi,edi,[size],0F2FFFFFFh,eax,[sweep]
    stdcall SplashProg,SPLASH_FILL_AT,SPLASH_FILL_MS
    test eax,eax
    jz .wordmark
    imul eax,46h
    shr eax,10
    stdcall WithAlpha,0FFFFFFh,eax
    stdcall DrawLogo,esi,edi,[size],eax,0,360
.wordmark:
    stdcall GfxUnclip
    ; wordmark under the mark: in after the drawing, out as the sheet moves
    stdcall SplashProg,SPLASH_WORD_AT,400
    mov ebx,eax
    stdcall SplashProg,SPLASH_MOVE_AT,260
    mov ecx,1024
    sub ecx,eax
    cmp ebx,ecx
    jbe @f
    mov ebx,ecx
@@:
    shr ebx,2
    test ebx,ebx
    jz .done
    mov [wordLvl],ebx
    stdcall Mix,[themeBlueprint1],[themeBlueprint2],128
    stdcall ArgbToCR,eax
    mov [bgCR],eax
    stdcall Mix,[bgCR],0ECE8E3h,[wordLvl]
    mov ebx,eax
    mov eax,[lcy]
    mov ecx,[size]
    shr ecx,1
    add eax,ecx
    add eax,30
    mov esi,eax
    stdcall TextDraw,sBrand,0,esi,1120,30,[fTitle],ebx,DT_CENTER,10
    stdcall Mix,[bgCR],0D9C2A6h,[wordLvl]
    mov ebx,eax
    add esi,34
    stdcall TextDraw,monoTagline,0,esi,1120,16,[fMonoSmall],ebx,DT_CENTER,3
.done:
    mov [gOffX],0
    ret
endp
