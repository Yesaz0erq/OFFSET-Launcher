; Native personalization. Palette values are derived from one RGB accent.
section '.thdata' data readable writeable
themeRGB dd 00071E3h
themeMotion dd 1
themeAccentARGB dd 0FF0071E3h
themeAccentHi dd 0FF1A84F0h
themeAccentLo dd 0FF0062C6h
themeAccentSoft dd 0FFE5F0FDh
themeAccentText dd 0E37100h
themeFocusARGB dd 400071E3h
themeBlueprint1 dd 0FF0D3168h
themeBlueprint2 dd 0FF071B3Eh
themeControlLock dd 0
hThemeColor dd 0
hThemeName dd 0
hThemeWindow dd 0
themePresets dd 00071E3h,07A3FBDh,0007D80h,0227546h,0B94D00h,0B83262h
; Window (canvas) colour and background texture. Window colours stay light so
; every ink colour keeps its contrast; the sidebar and grid marks derive from it.
themeWindowRGB dd 0F5F6F8h
themeTexture dd 0                       ; 0 dots, 1 grid, 2 blocks, 3 plain
themeCanvasARGB dd 0FFF5F6F8h
themeSidebarARGB dd 0FFEEF0F4h
themeSidelineARGB dd 0FFDFE3E9h
themeDotARGB dd 0FFD3D8E0h
themeCrossARGB dd 0FFC9CFD9h
themeGridARGB dd 0FFE6E9EEh
themeCanvasCR dd 0F8F6F5h
themeSidebarCR dd 0F4F0EEh
; Appearance: 0 light, 1 dark, 2 follow Windows. themeDark is the result.
themeMode dd 2
themeDark dd 0
themeNativeDark dd -1
themeBrushCR dd -1
themeRetheme dd 0
themeUxReady dd 0
themeUxMode dd 0                        ; uxtheme #135 SetPreferredAppMode (1809+)
themeUxFlush dd 0                       ; uxtheme #136 FlushMenuThemes
; Surface and ink colours; light values here, dark ones derived in ThemeApply.
palCard dd 0FFFFFFFFh
palHairline dd 0FFE3E6EBh
palDivider dd 0FFECEEF2h
palField dd 0FFFFFFFFh
palFieldLine dd 0FFE7EBF2h
palTrack dd 0FFE8EAEFh
palSecond dd 0FFE6E8ECh
palSecondHi dd 0FFDCDFE5h
palSecondLo dd 0FFD2D6DDh
palInk dd 0FF1D1D1Fh
palMuted dd 0FF8A8F98h
palHover dd 0
palTexInk dd 0FF000000h
palTInk dd 01F1D1Dh
palTSecond dd 0736E6Eh
palTMuted dd 0A09891h
palTipText dd 0FFFFFFh
palLineCR dd 0EBE6E3h
palFieldCR dd 0FFFFFFh
modeNames dd sModeLight,sModeDark,sModeSystem
themeKeyMode du 'Appearance',0
themeRegPersonalize du 'Software\Microsoft\Windows\CurrentVersion\Themes\Personalize',0
themeRegLight du 'AppsUseLightTheme',0
themeRegNt du 'SOFTWARE\Microsoft\Windows NT\CurrentVersion',0
themeRegBuild du 'CurrentBuildNumber',0
themeUxName du 'uxtheme.dll',0
themeDarkExplorer du 'DarkMode_Explorer',0
themeDarkCfd du 'DarkMode_CFD',0
themeDarkItems du 'DarkMode_ItemsView',0
themeClsCombo du 'ComboBox',0
themeClsHeader du 'SysHeader32',0
themeClsList du 'SysListView32',0
themeColorSet du 'ImmersiveColorSet',0
themeClass rw 32
windowPresets dd 0F5F6F8h,0EEF3FAh,0EEF5EEh,0F7F1E6h,0F8EEF1h,0F2EFF8h
; One-click schemes: accent + window colour + texture.
schemeAccent dd 00071E3h,0227546h,0B94D00h,0007D80h,0B83262h,07A3FBDh
schemeWindow dd 0F5F6F8h,0EEF5EEh,0F7F1E6h,0EEF3FAh,0F8EEF1h,0F2EFF8h
schemeTexture dd 0,2,1,1,0,2
schemeNames dd sSchemeBlueprint,sSchemeMeadow,sSchemeDesert,sSchemeOcean,sSchemeCherry,sSchemeAmethyst
textureNames dd sTexDots,sTexGrid,sTexBlocks,sTexPlain
themeKeyWindow du 'WindowColor',0
themeKeyTexture du 'Texture',0
themeDefaultWindow du 'F5F6F8',0
themeKeyColor du 'ThemeColor',0
themeKeyMotion du 'Animations',0
themeKeyName du 'LauncherName',0
themeKeyNameHex du 'LauncherNameHex',0
themeDefaultHex du '0071E3',0
themeHexFormat du '%06X',0
themeDecimalFormat du '%u',0
themeHexDigits du '0123456789ABCDEF',0
themeEyebrow du '07 / PERSONAL',0
themeName rw 40
themeHex rw 16
themeWinHex rw 16
themeInput rw 40
themeNameHex rw 160

section '.thcode' code readable executable
    nop

; Accept exactly six ASCII hexadecimal digits, optionally prefixed with '#'.
; An invalid input never writes the caller's output value.
proc ThemeParseHex uses ebx esi edi,text,result
    mov esi,[text]
    test esi,esi
    jz .bad
    cmp word [esi],'#'
    jne .start
    add esi,2
.start:
    xor ebx,ebx
    mov edi,6
.digit:
    movzx eax,word [esi]
    add esi,2
    cmp eax,'0'
    jb .bad
    cmp eax,'9'
    jbe .number
    or eax,20h
    cmp eax,'a'
    jb .bad
    cmp eax,'f'
    ja .bad
    sub eax,'a'-10
    jmp .append
.number:
    sub eax,'0'
.append:
    shl ebx,4
    or ebx,eax
    dec edi
    jnz .digit
    cmp word [esi],0
    jne .bad
    mov eax,[result]
    test eax,eax
    jz .bad
    mov [eax],ebx
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

; IEC sRGB relative luminance, scaled by 1,000,000,000.
; The lookup table is 256 small integers; no runtime math library is required.
proc ThemeLuminance uses ebx esi,rgb
    mov esi,[rgb]
    mov eax,esi
    shr eax,16
    and eax,255
    mov eax,[themeSrgb+eax*4]
    imul eax,2126
    mov ebx,eax
    mov eax,esi
    shr eax,8
    and eax,255
    mov eax,[themeSrgb+eax*4]
    imul eax,7152
    add ebx,eax
    mov eax,esi
    and eax,255
    mov eax,[themeSrgb+eax*4]
    imul eax,722
    add eax,ebx
    ret
endp

; Preserve hue while making white button text meet a contrast ratio of 4.5:1.
; The default blue is already readable and is preserved exactly.
proc ThemeNormalizeRGB uses ebx esi edi,rgb
    mov ebx,[rgb]
    and ebx,0FFFFFFh
.test:
    stdcall ThemeLuminance,ebx
    cmp eax,183333333
    jbe .done
    xor edi,edi
    xor esi,esi
.channel:
    mov eax,ebx
    mov ecx,esi
    shr eax,cl
    and eax,255
    imul eax,95
    xor edx,edx
    mov ecx,100
    div ecx
    mov ecx,esi
    shl eax,cl
    or edi,eax
    add esi,8
    cmp esi,24
    jb .channel
    mov ebx,edi
    jmp .test
.done:
    mov eax,ebx
    ret
endp

; Window colours are lightened (hue kept) until luminance >= 0.78, so the
; light-theme inks, cards and hairlines stay readable on any chosen colour.
proc ThemeLightRGB uses ebx esi edi,rgb
    mov ebx,[rgb]
    and ebx,0FFFFFFh
.test:
    stdcall ThemeLuminance,ebx
    cmp eax,780000000
    jae .done
    xor edi,edi
    xor esi,esi
.channel:
    mov eax,ebx
    mov ecx,esi
    shr eax,cl
    and eax,255
    mov edx,255
    sub edx,eax
    shr edx,3
    inc edx
    add eax,edx
    cmp eax,255
    jbe @f
    mov eax,255
@@:
    mov ecx,esi
    shl eax,cl
    or edi,eax
    add esi,8
    cmp esi,24
    jb .channel
    mov ebx,edi
    jmp .test
.done:
    mov eax,ebx
    ret
endp

; Unicode display name: 0..36 UTF-16 units, no controls or isolated surrogates.
proc ThemeValidName uses esi,text
    mov esi,[text]
    xor ecx,ecx
.next:
    movzx eax,word [esi+ecx*2]
    test eax,eax
    jz .good
    cmp ecx,36
    jae .bad
    cmp eax,32
    jb .bad
    cmp eax,127
    je .bad
    cmp eax,0D800h
    jb .advance
    cmp eax,0DFFFh
    ja .advance
    cmp eax,0DBFFh
    ja .bad
    inc ecx
    cmp ecx,36
    jae .bad
    movzx eax,word [esi+ecx*2]
    cmp eax,0DC00h
    jb .bad
    cmp eax,0DFFFh
    ja .bad
.advance:
    inc ecx
    jmp .next
.good:
    mov eax,1
    ret
.bad:
    xor eax,eax
    ret
endp

proc ThemeName
    mov eax,themeName
    cmp word [themeName],0
    jne .done
    mov eax,sBrand
.done:
    ret
endp

; ASCII hex storage survives both ANSI and Unicode INI files on any locale.
proc ThemeEncodeName uses ebx esi edi
    mov esi,themeName
    mov edi,themeNameHex
.unit:
    movzx ebx,word [esi]
    add esi,2
    test ebx,ebx
    jz .done
    mov ecx,12
.digit:
    mov eax,ebx
    shr eax,cl
    and eax,15
    mov ax,[themeHexDigits+eax*2]
    stosw
    sub ecx,4
    jns .digit
    jmp .unit
.done:
    mov word [edi],0
    ret
endp

proc ThemeDecodeName uses ebx esi edi
    local units:DWORD
    mov esi,themeNameHex
    mov edi,themeName
    mov [units],0
.unit:
    cmp word [esi],0
    je .good
    cmp [units],36
    jae .bad
    xor ebx,ebx
    mov edx,4
.digit:
    movzx eax,word [esi]
    add esi,2
    cmp eax,'0'
    jb .bad
    cmp eax,'9'
    jbe .number
    or eax,20h
    cmp eax,'a'
    jb .bad
    cmp eax,'f'
    ja .bad
    sub eax,'a'-10
    jmp .append
.number:
    sub eax,'0'
.append:
    shl ebx,4
    or ebx,eax
    dec edx
    jnz .digit
    test ebx,ebx
    jz .bad
    mov ax,bx
    stosw
    inc [units]
    jmp .unit
.good:
    mov word [edi],0
    stdcall ThemeValidName,themeName
    test eax,eax
    jz .bad
    mov eax,1
    ret
.bad:
    mov word [themeName],0
    xor eax,eax
    ret
endp

proc ThemeWindowTitle
    mov eax,themeName
    cmp word [themeName],0
    jne .done
    mov eax,sTitle
.done:
    ret
endp

proc ThemeMotionSync uses esi edi
    cmp [themeMotion],0
    jne .done
    mov esi,animTgt
    mov edi,animCur
    mov ecx,ANIM_SLOTS
    rep movsd
    mov [transActive],0
    mov [transEase],1024
    mov [heroActive],0
    mov [statusStart],0
    mov [fadeAmt],0
    mov [hoverAmt],256
    cmp [animTimer],0
    je .done
    invoke KillTimer,[hWnd],1
    mov [animTimer],0
.done:
    ret
endp

; ARGB -> COLORREF
proc ThemeCR,argb
    mov eax,[argb]
    bswap eax
    shr eax,8
    ret
endp

; Dark canvas that keeps the window colour's tint: 18h plus three quarters of
; each channel's distance above the smallest channel.
proc ThemeDarkCanvas uses esi,rgb
    mov edx,[rgb]
    movzx eax,dl
    movzx ecx,dh
    shr edx,16
    and edx,0FFh
    mov esi,eax
    cmp ecx,esi
    jae @f
    mov esi,ecx
@@:
    cmp edx,esi
    jae @f
    mov esi,edx
@@:
    sub eax,esi
    imul eax,3
    shr eax,2
    add eax,18h
    sub ecx,esi
    imul ecx,3
    shr ecx,2
    add ecx,18h
    sub edx,esi
    imul edx,3
    shr edx,2
    add edx,18h
    shl ecx,8
    shl edx,16
    or eax,ecx
    or eax,edx
    or eax,0FF000000h
    ret
endp

; 1 when Windows apps are set to dark (AppsUseLightTheme = 0).
proc ThemeSystemDark
    locals
        key dd ?
        value dd ?
        cb dd ?
        kind dd ?
    endl
    mov [value],1
    invoke RegOpenKeyExW,HKEY_CURRENT_USER,themeRegPersonalize,0,KEY_READ,addr key
    test eax,eax
    jnz .result
    mov [cb],4
    invoke RegQueryValueExW,[key],themeRegLight,0,addr kind,addr value,addr cb
    test eax,eax
    jz @f
    mov [value],1
@@:
    invoke RegCloseKey,[key]
.result:
    xor eax,eax
    cmp [value],0
    sete al
    ret
endp

; Dark context menus need uxtheme's unnamed exports, present from Windows 10
; 1809 (build 17763). Older systems keep light menus.
proc ThemeUxSetup uses ebx esi
    locals
        key dd ?
        cb dd ?
        kind dd ?
        build rw 16
    endl
    cmp [themeUxReady],0
    jne .done
    mov [themeUxReady],1
    invoke RegOpenKeyExW,HKEY_LOCAL_MACHINE,themeRegNt,0,KEY_READ,addr key
    test eax,eax
    jnz .done
    mov [cb],30
    mov word [build],0
    invoke RegQueryValueExW,[key],themeRegBuild,0,addr kind,addr build,addr cb
    mov ebx,eax
    invoke RegCloseKey,[key]
    test ebx,ebx
    jnz .done
    mov word [build+30],0
    xor ebx,ebx
    lea esi,[build]
.digit:
    movzx eax,word [esi]
    sub eax,'0'
    cmp eax,9
    ja .parsed
    imul ebx,10
    add ebx,eax
    add esi,2
    jmp .digit
.parsed:
    cmp ebx,17763
    jb .done
    invoke GetModuleHandleW,themeUxName
    test eax,eax
    jz .done
    mov ebx,eax
    invoke GetProcAddress,ebx,135
    mov esi,eax
    invoke GetProcAddress,ebx,136
    test esi,esi
    jz .done
    test eax,eax
    jz .done
    mov [themeUxMode],esi
    mov [themeUxFlush],eax
.done:
    ret
endp

; EnumChildWindows callback (also called for each new control with force=1):
; native dark scrollbars/combo faces, list view colours, repaint.
proc ThemeChild uses ebx,hwnd,force
    invoke GetClassNameW,[hwnd],themeClass,32
    cmp [force],0
    jne .theme
    cmp [themeRetheme],0
    je .colors
.theme:
    xor ebx,ebx
    cmp [themeDark],0
    je .set
    mov ebx,themeDarkExplorer
    invoke lstrcmpiW,themeClass,themeClsCombo
    test eax,eax
    jnz @f
    mov ebx,themeDarkCfd
@@:
    invoke lstrcmpiW,themeClass,themeClsHeader
    test eax,eax
    jnz .set
    mov ebx,themeDarkItems
.set:
    invoke SetWindowTheme,[hwnd],ebx,0
.colors:
    invoke lstrcmpiW,themeClass,themeClsList
    test eax,eax
    jnz .redraw
    invoke SendMessageW,[hwnd],1001h,0,[palFieldCR]     ; LVM_SETBKCOLOR
    invoke SendMessageW,[hwnd],1026h,0,[palFieldCR]     ; LVM_SETTEXTBKCOLOR
    invoke SendMessageW,[hwnd],1024h,0,[palTInk]        ; LVM_SETTEXTCOLOR
.redraw:
    invoke InvalidateRect,[hwnd],0,1
    mov eax,1
    ret
endp

; Bring native pieces in line with the palette: the edit/list brush, child
; control themes, context menus and the DWM frame. Cheap when nothing changed.
proc ThemeNative
    local flag:DWORD
    mov [themeRetheme],0
    xor ecx,ecx
    mov eax,[palFieldCR]
    cmp eax,[themeBrushCR]
    je .mode
    mov [themeBrushCR],eax
    mov eax,[inputBrush]
    test eax,eax
    jz @f
    invoke DeleteObject,eax
@@:
    invoke CreateSolidBrush,[palFieldCR]
    mov [inputBrush],eax
    mov ecx,1
.mode:
    mov eax,[themeDark]
    cmp eax,[themeNativeDark]
    je .children
    mov [themeNativeDark],eax
    mov [themeRetheme],1
    mov ecx,1
    push ecx
    stdcall ThemeUxSetup
    mov eax,[themeUxMode]
    test eax,eax
    jz .dwm
    mov ecx,3                               ; ForceLight
    cmp [themeDark],0
    je @f
    mov ecx,2                               ; ForceDark
@@:
    push ecx
    call eax
    call [themeUxFlush]
.dwm:
    mov eax,[themeDark]
    mov [flag],eax
    invoke DwmSetWindowAttribute,[hWnd],20,addr flag,4
    pop ecx
.children:
    test ecx,ecx
    jz .done
    invoke EnumChildWindows,[hWnd],ThemeChild,0
.done:
    mov [themeRetheme],0
    ret
endp

; WM_SETTINGCHANGE: follow a Windows light/dark switch while in System mode.
proc ThemeSettingChange,lparam
    cmp [themeMode],2
    jne .done
    mov eax,[lparam]
    test eax,eax
    jz .done
    invoke lstrcmpiW,eax,themeColorSet
    test eax,eax
    jnz .done
    stdcall ThemeSystemDark
    cmp eax,[themeDark]
    je .done
    stdcall ThemeApply
.done:
    ret
endp

macro tmix dst,a,b,t
{
    stdcall Mix,a,b,t
    mov [dst],eax
}

proc ThemeApply uses ebx
    mov eax,[themeMode]
    cmp eax,2
    jb @f
    stdcall ThemeSystemDark
@@:
    mov [themeDark],eax
    stdcall ThemeNormalizeRGB,[themeRGB]
    mov [themeRGB],eax
    or eax,0FF000000h
    mov [themeAccentARGB],eax
    mov ebx,eax
    stdcall Mix,ebx,0FFFFFFFFh,24
    mov [themeAccentHi],eax
    stdcall Mix,ebx,0FF000000h,30
    mov [themeAccentLo],eax
    stdcall Mix,ebx,0FFFFFFFFh,230
    mov [themeAccentSoft],eax
    mov eax,[themeRGB]
    bswap eax
    shr eax,8
    mov [themeAccentText],eax
    mov eax,[themeRGB]
    or eax,40000000h
    mov [themeFocusARGB],eax
    ; Blueprints remain dark, with a restrained tint from the chosen accent.
    stdcall Mix,0FF102030h,ebx,80
    mov [themeBlueprint1],eax
    stdcall Mix,0FF07111Bh,ebx,42
    mov [themeBlueprint2],eax
    cmp [themeRGB],00071E3h
    jne .window
    ; Keep the existing design exactly when the default theme is selected.
    mov [themeAccentHi],0FF1A84F0h
    mov [themeAccentLo],0FF0062C6h
    mov [themeAccentSoft],0FFE5F0FDh
    mov [themeBlueprint1],0FF0D3168h
    mov [themeBlueprint2],0FF071B3Eh
.window:
    stdcall ThemeLightRGB,[themeWindowRGB]
    mov [themeWindowRGB],eax
    cmp [themeDark],0
    jne .dark
    mov [palCard],0FFFFFFFFh
    mov [palHairline],0FFE3E6EBh
    mov [palDivider],0FFECEEF2h
    mov [palField],0FFFFFFFFh
    mov [palFieldLine],0FFE7EBF2h
    mov [palTrack],0FFE8EAEFh
    mov [palSecond],0FFE6E8ECh
    mov [palSecondHi],0FFDCDFE5h
    mov [palSecondLo],0FFD2D6DDh
    mov [palInk],0FF1D1D1Fh
    mov [palMuted],0FF8A8F98h
    mov [palHover],0
    mov [palTexInk],0FF000000h
    mov [palTInk],01F1D1Dh
    mov [palTSecond],0736E6Eh
    mov [palTMuted],0A09891h
    mov eax,[themeWindowRGB]
    or eax,0FF000000h
    mov [themeCanvasARGB],eax
    mov ebx,eax
    stdcall Mix,ebx,0FF000000h,7
    mov [themeSidebarARGB],eax
    stdcall Mix,ebx,0FF000000h,22
    mov [themeSidelineARGB],eax
    stdcall Mix,ebx,0FF000000h,34
    mov [themeDotARGB],eax
    stdcall Mix,ebx,0FF000000h,43
    mov [themeCrossARGB],eax
    stdcall Mix,ebx,0FF000000h,14
    mov [themeGridARGB],eax
    cmp [themeWindowRGB],0F5F6F8h
    jne .colorRefs
    ; the default window keeps the original palette exactly
    mov [themeSidebarARGB],0FFEEF0F4h
    mov [themeSidelineARGB],0FFDFE3E9h
    mov [themeDotARGB],0FFD3D8E0h
    mov [themeCrossARGB],0FFC9CFD9h
    jmp .colorRefs
.dark:
    ; Dark surfaces step up in lightness from a tinted near-black canvas.
    stdcall ThemeDarkCanvas,[themeWindowRGB]
    mov [themeCanvasARGB],eax
    mov ebx,eax
    tmix themeSidebarARGB,ebx,0FF000000h,64
    tmix themeSidelineARGB,ebx,0FFFFFFFFh,22
    tmix themeDotARGB,ebx,0FFFFFFFFh,34
    tmix themeCrossARGB,ebx,0FFFFFFFFh,50
    tmix themeGridARGB,ebx,0FFFFFFFFh,14
    tmix palCard,ebx,0FFFFFFFFh,18
    tmix palHairline,ebx,0FFFFFFFFh,34
    tmix palDivider,ebx,0FFFFFFFFh,26
    tmix palField,ebx,0FFFFFFFFh,8
    tmix palFieldLine,ebx,0FFFFFFFFh,40
    tmix palTrack,ebx,0FF000000h,72
    tmix palSecond,ebx,0FFFFFFFFh,42
    tmix palSecondHi,ebx,0FFFFFFFFh,54
    tmix palSecondLo,ebx,0FFFFFFFFh,32
    mov [palInk],0FFF2F2F5h
    mov [palMuted],0FF8E939Ch
    mov [palHover],0FFFFFFh
    mov [palTexInk],0FFFFFFFFh
    mov [palTInk],0F5F2F2h
    mov [palTSecond],0B5ADA9h
    mov [palTMuted],08B827Dh
    ; accent text needs to be lighter on dark; selection tint sits on cards
    stdcall Mix,[themeAccentARGB],0FFFFFFFFh,96
    stdcall ThemeCR,eax
    mov [themeAccentText],eax
    tmix themeAccentSoft,[palCard],[themeAccentARGB],60
.colorRefs:
    stdcall ThemeCR,[palField]
    mov [palFieldCR],eax
    stdcall ThemeCR,[palHairline]
    mov [palLineCR],eax
    mov eax,[themeCanvasARGB]
    bswap eax
    shr eax,8
    mov [themeCanvasCR],eax
    mov eax,[themeSidebarARGB]
    bswap eax
    shr eax,8
    mov [themeSidebarCR],eax
    mov eax,0FFFFFFh
    cmp [themeDark],0
    je @f
    mov eax,[themeCanvasCR]
@@:
    mov [palTipText],eax
    cmp [themeTexture],3
    jbe .ready
    mov [themeTexture],0
.ready:
    cmp [hWnd],0
    je .done
    stdcall ThemeWindowTitle
    invoke SetWindowTextW,[hWnd],eax
    stdcall ThemeMotionSync
    mov [transActive],0
    cmp [inputBrush],0
    je @f
    stdcall ThemeNative
@@:
    stdcall FreeBuffers
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp

proc ThemeLoad
    invoke GetPrivateProfileStringW,sectionName,themeKeyColor,themeDefaultHex,themeHex,16,iniPath
    stdcall ThemeParseHex,themeHex,themeRGB
    test eax,eax
    jnz .motion
    mov [themeRGB],00071E3h
.motion:
    invoke GetPrivateProfileIntW,sectionName,themeKeyMotion,1,iniPath
    cmp eax,1
    jbe .setMotion
    mov eax,1
.setMotion:
    mov [themeMotion],eax
    invoke GetPrivateProfileStringW,sectionName,themeKeyWindow,themeDefaultWindow,themeWinHex,16,iniPath
    stdcall ThemeParseHex,themeWinHex,themeWindowRGB
    test eax,eax
    jnz .texture
    mov [themeWindowRGB],0F5F6F8h
.texture:
    invoke GetPrivateProfileIntW,sectionName,themeKeyTexture,0,iniPath
    cmp eax,3
    jbe .setTexture
    xor eax,eax
.setTexture:
    mov [themeTexture],eax
    invoke GetPrivateProfileIntW,sectionName,themeKeyMode,2,iniPath
    cmp eax,2
    jbe @f
    mov eax,2
@@:
    mov [themeMode],eax
    invoke GetPrivateProfileStringW,sectionName,themeKeyNameHex,emptyW,themeNameHex,160,iniPath
    test eax,eax
    jz .legacyName
    stdcall ThemeDecodeName
    jmp .apply
.legacyName:
    invoke GetPrivateProfileStringW,sectionName,themeKeyName,emptyW,themeName,40,iniPath
    stdcall ThemeValidName,themeName
    test eax,eax
    jnz .apply
    mov word [themeName],0
.apply:
    stdcall ThemeApply
    ret
endp

proc ThemeSave uses ebx
    stdcall ThemeValidName,themeName
    test eax,eax
    jnz .valid
    ret
.valid:
    mov ebx,1
    cinvoke wsprintfW,themeHex,themeHexFormat,[themeRGB]
    invoke WritePrivateProfileStringW,sectionName,themeKeyColor,themeHex,iniPath
    and ebx,eax
    cinvoke wsprintfW,themeHex,themeDecimalFormat,[themeMotion]
    invoke WritePrivateProfileStringW,sectionName,themeKeyMotion,themeHex,iniPath
    and ebx,eax
    cinvoke wsprintfW,themeHex,themeHexFormat,[themeWindowRGB]
    invoke WritePrivateProfileStringW,sectionName,themeKeyWindow,themeHex,iniPath
    and ebx,eax
    cinvoke wsprintfW,themeHex,themeDecimalFormat,[themeTexture]
    invoke WritePrivateProfileStringW,sectionName,themeKeyTexture,themeHex,iniPath
    and ebx,eax
    cinvoke wsprintfW,themeHex,themeDecimalFormat,[themeMode]
    invoke WritePrivateProfileStringW,sectionName,themeKeyMode,themeHex,iniPath
    and ebx,eax
    stdcall ThemeEncodeName
    invoke WritePrivateProfileStringW,sectionName,themeKeyNameHex,themeNameHex,iniPath
    and ebx,eax
    ; Remove a legacy literal field so Reset cannot resurrect an old name.
    invoke WritePrivateProfileStringW,sectionName,themeKeyName,0,iniPath
    and eax,ebx
    ret
endp

proc RefreshPersonalizationColor
    mov [themeControlLock],1
    cinvoke wsprintfW,themeHex,themeHexFormat,[themeRGB]
    invoke SetWindowTextW,[hThemeColor],themeHex
    cinvoke wsprintfW,themeWinHex,themeHexFormat,[themeWindowRGB]
    invoke SetWindowTextW,[hThemeWindow],themeWinHex
    mov [themeControlLock],0
    ret
endp

proc RefreshPersonalizationControls
    stdcall RefreshPersonalizationColor
    invoke SetWindowTextW,[hThemeName],themeName
    ret
endp

proc CreatePersonalizationControls
    stdcall NewControl,editClass,ES_AUTOHSCROLL,501,990,176,72,24
    mov [hThemeColor],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,7,0
    stdcall NewControl,editClass,ES_AUTOHSCROLL,503,990,256,72,24
    mov [hThemeWindow],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,7,0
    stdcall NewControl,editClass,ES_AUTOHSCROLL,502,452,476,564,24
    mov [hThemeName],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,36,0
    stdcall RefreshPersonalizationControls
    ret
endp

proc ShowPersonalizationControls
    stdcall RefreshPersonalizationControls
    invoke ShowWindow,[hThemeColor],SW_SHOW
    invoke ShowWindow,[hThemeWindow],SW_SHOW
    invoke ShowWindow,[hThemeName],SW_SHOW
    ret
endp

proc HidePersonalizationControls
    invoke ShowWindow,[hThemeColor],SW_HIDE
    invoke ShowWindow,[hThemeWindow],SW_HIDE
    invoke ShowWindow,[hThemeName],SW_HIDE
    ret
endp

proc EnablePersonalizationControls,enabled
    invoke EnableWindow,[hThemeColor],[enabled]
    invoke EnableWindow,[hThemeWindow],[enabled]
    invoke EnableWindow,[hThemeName],[enabled]
    ret
endp

proc PersonalizationCommand,wparam
    local parsed:DWORD
    cmp [themeControlLock],0
    jne .done
    mov eax,[wparam]
    mov edx,eax
    shr edx,16
    and eax,0FFFFh
    cmp edx,EN_CHANGE
    jne .done
    cmp eax,501
    je .accent
    cmp eax,503
    jne .done
    invoke GetWindowTextW,[hThemeWindow],themeWinHex,16
    stdcall ThemeParseHex,themeWinHex,addr parsed
    test eax,eax
    jz .done
    mov eax,[parsed]
    mov [themeWindowRGB],eax
    stdcall ThemeApply
    jmp .done
.accent:
    invoke GetWindowTextW,[hThemeColor],themeHex,16
    stdcall ThemeParseHex,themeHex,addr parsed
    test eax,eax
    jz .done
    mov eax,[parsed]
    mov [themeRGB],eax
    stdcall ThemeApply
.done:
    ret
endp

; ---- scheme tiles: a small window swatch with its texture, accent bar and name
SCHEME_X = 276
SCHEME_Y = 180
SCHEME_W = 112
SCHEME_H = 104
SCHEME_DX = 124
SCHEME_DY = 116

; Returns 1 when the current theme equals scheme [index].
proc ThemeSchemeActive,index
    mov ecx,[index]
    mov eax,[schemeAccent+ecx*4]
    cmp eax,[themeRGB]
    jne .no
    mov eax,[schemeWindow+ecx*4]
    cmp eax,[themeWindowRGB]
    jne .no
    mov eax,[schemeTexture+ecx*4]
    cmp eax,[themeTexture]
    jne .no
    mov eax,1
    ret
.no:
    xor eax,eax
    ret
endp

proc ThemeTilePos,index
    ; eax = x, edx = y
    mov eax,[index]
    xor edx,edx
    mov ecx,3
    div ecx
    imul edx,SCHEME_DX
    add edx,SCHEME_X
    imul eax,SCHEME_DY
    add eax,SCHEME_Y
    xchg eax,edx
    ret
endp

proc PaintSchemeTile uses ebx esi edi,index
    locals
        tx dd ?
        ty dd ?
        base dd ?
        mark dd ?
        i dd ?
        j dd ?
    endl
    stdcall ThemeTilePos,[index]
    mov [tx],eax
    mov [ty],edx
    stdcall HotAdd,[tx],[ty],SCHEME_W,SCHEME_H
    mov esi,[index]
    mov eax,[schemeWindow+esi*4]
    or eax,0FF000000h
    cmp [themeDark],0
    je @f
    stdcall ThemeDarkCanvas,eax
@@:
    mov [base],eax
    stdcall FillRR,[tx],[ty],SCHEME_W,SCHEME_H,12,[base]
    stdcall Mix,[base],A_TEXINK,40
    mov [mark],eax
    mov eax,[schemeTexture+esi*4]
    cmp eax,1
    je .grid
    cmp eax,2
    je .blocks
    cmp eax,3
    je .accent
    ; dots
    mov [j],0
.dotRow:
    mov [i],0
.dotCol:
    mov eax,[i]
    imul eax,15
    add eax,[tx]
    add eax,18
    mov ecx,[j]
    imul ecx,13
    add ecx,[ty]
    add ecx,16
    stdcall FillCircle,eax,ecx,1,[mark]
    inc [i]
    cmp [i],6
    jb .dotCol
    inc [j]
    cmp [j],3
    jb .dotRow
    jmp .accent
.grid:
    mov [i],0
.gridV:
    mov eax,[i]
    imul eax,22
    add eax,[tx]
    add eax,12
    mov ecx,[ty]
    add ecx,10
    mov edx,[ty]
    add edx,52
    stdcall LineA,eax,ecx,eax,edx,[mark],10
    inc [i]
    cmp [i],5
    jb .gridV
    mov [i],0
.gridH:
    mov eax,[i]
    imul eax,14
    add eax,[ty]
    add eax,10
    mov ecx,[tx]
    add ecx,12
    mov edx,[tx]
    add edx,100
    stdcall LineA,ecx,eax,edx,eax,[mark],10
    inc [i]
    cmp [i],4
    jb .gridH
    jmp .accent
.blocks:
    mov [j],0
.blockRow:
    mov [i],0
.blockCol:
    ; a fixed, varied shade pattern of 11-unit pixels
    mov eax,[i]
    imul eax,3
    mov ecx,[j]
    imul ecx,5
    add eax,ecx
    and eax,3
    jz .blockNext
    imul eax,9
    stdcall Mix,[base],A_TEXINK,eax
    mov ebx,eax
    mov eax,[i]
    imul eax,11
    add eax,[tx]
    add eax,12
    mov ecx,[j]
    imul ecx,11
    add ecx,[ty]
    add ecx,10
    stdcall FillRR,eax,ecx,11,11,0,ebx
.blockNext:
    inc [i]
    cmp [i],8
    jb .blockCol
    inc [j]
    cmp [j],4
    jb .blockRow
.accent:
    mov eax,[schemeAccent+esi*4]
    or eax,0FF000000h
    mov ecx,[tx]
    add ecx,12
    mov edx,[ty]
    add edx,60
    stdcall FillRR,ecx,edx,88,8,4,eax
    mov ecx,[tx]
    add ecx,12
    mov edx,[ty]
    add edx,74
    mov eax,[schemeNames+esi*4]
    stdcall Text,0,eax,ecx,edx,92,24,[fSmall],T_INK
    stdcall ThemeSchemeActive,esi
    test eax,eax
    jz .plainEdge
    stdcall StrokeRR,[tx],[ty],SCHEME_W,SCHEME_H,12,[themeAccentARGB],22
    ret
.plainEdge:
    stdcall StrokeRR,[tx],[ty],SCHEME_W,SCHEME_H,12,A_HAIRLINE,10
    ret
endp

; Six colour circles in a row: accent (filled) or window (filled + outline).
PS_X = 716
PS_STEP = 46

proc PaintSwatchRow uses ebx esi,cy_,list,current,outlined
    xor esi,esi
.swatch:
    imul ebx,esi,PS_STEP
    add ebx,PS_X
    mov eax,[list]
    mov eax,[eax+esi*4]
    or eax,0FF000000h
    cmp [outlined],0
    je @f
    cmp [themeDark],0
    je @f
    stdcall ThemeDarkCanvas,eax
@@:
    stdcall FillCircle,ebx,[cy_],14,eax
    cmp [outlined],0
    je @f
    stdcall ArcA,ebx,[cy_],14,0,360,A_SIDELINE,10,0
@@:
    mov eax,[list]
    mov eax,[eax+esi*4]
    cmp eax,[current]
    jne .register
    stdcall ArcA,ebx,[cy_],19,0,360,[themeAccentARGB],15,0
.register:
    mov eax,ebx
    sub eax,20
    mov ecx,[cy_]
    sub ecx,20
    stdcall HotAdd,eax,ecx,40,40
    inc esi
    cmp esi,6
    jb .swatch
    ret
endp

proc PaintPersonalization uses ebx esi,dc
    stdcall PageHeader,themeEyebrow,sPersonalHeading,7
    ; left: colour schemes
    stdcall Card,252,124,404,300,18
    stdcall Text,[dc],sPersonalScheme,280,140,360,26,[fMedium],T_INK
    xor esi,esi
.tile:
    stdcall PaintSchemeTile,esi
    inc esi
    cmp esi,6
    jb .tile
    ; right: accent, window colour, texture
    stdcall Card,672,124,416,300,18
    stdcall Text,[dc],sPersonalColor,700,140,270,26,[fMedium],T_INK
    stdcall PaintSwatchRow,188,themePresets,[themeRGB],0
    stdcall Field,980,170,92,36,[hThemeColor]
    stdcall Text,[dc],sPersonalWindow,700,220,270,26,[fMedium],T_INK
    stdcall PaintSwatchRow,268,windowPresets,[themeWindowRGB],1
    stdcall Field,980,250,92,36,[hThemeWindow]
    stdcall Text,[dc],sPersonalTexture,700,300,370,26,[fMedium],T_INK
    stdcall Segmented,700,332,372,36,4,9,textureNames,[themeTexture],[fSmall]
    stdcall Text,[dc],sPersonalHelp,700,380,380,30,[fSmall],T_MUTED
    ; bottom: name and motion
    stdcall Card,252,440,836,152,18
    stdcall Text,[dc],sPersonalName,280,476,150,26,[fBody],T_SECOND
    stdcall Field,440,470,588,36,[hThemeName]
    stdcall Text,[dc],sPersonalMotion,280,540,150,26,[fBody],T_SECOND
    mov eax,0
    cmp [themeMotion],0
    je .on
    inc eax
.on:
    stdcall Button,[dc],sPersonalOn,440,530,120,42,eax
    mov eax,0
    cmp [themeMotion],0
    jne .off
    inc eax
.off:
    stdcall Button,[dc],sPersonalOff,572,530,120,42,eax
    stdcall Text,[dc],sPersonalAppearance,716,540,116,26,[fBody],T_SECOND
    stdcall Segmented,836,532,192,38,3,10,modeNames,[themeMode],[fSmall]
    stdcall Button,[dc],sPersonalApply,252,620,200,42,1
    stdcall Button,[dc],sPersonalReset,470,620,200,42,0
    stdcall StatusLine,254,676,830,32
    ret
endp

; Index of the swatch under x (0..5), or -1.
proc SwatchAt,x
    mov eax,[x]
    sub eax,PS_X-20
    jb .none
    xor edx,edx
    mov ecx,PS_STEP
    div ecx
    cmp eax,6
    jae .none
    cmp edx,40
    jae .none
    ret
.none:
    mov eax,-1
    ret
endp

proc ClickPersonalization uses ebx esi,x,y
    local parsed:DWORD
    ; back to Settings
    cmp [y],68
    jb .edit
    cmp [y],100
    ja .edit
    cmp [x],888
    jb .edit
    cmp [x],1088
    ja .edit
    if defined GoPage
    stdcall GoPage,3
    end if
    ret
.edit:
    ; scheme tiles
    cmp [x],656
    jae .right
    xor esi,esi
.tile:
    stdcall ThemeTilePos,esi
    cmp [x],eax
    jb .nextTile
    add eax,SCHEME_W
    cmp [x],eax
    ja .nextTile
    cmp [y],edx
    jb .nextTile
    add edx,SCHEME_H
    cmp [y],edx
    ja .nextTile
    mov eax,[schemeAccent+esi*4]
    mov [themeRGB],eax
    mov eax,[schemeWindow+esi*4]
    mov [themeWindowRGB],eax
    mov eax,[schemeTexture+esi*4]
    mov [themeTexture],eax
    jmp .applied
.nextTile:
    inc esi
    cmp esi,6
    jb .tile
    jmp .actions
.right:
    ; accent swatches
    cmp [y],168
    jb .actions
    cmp [y],208
    ja .windowRow
    stdcall SwatchAt,[x]
    cmp eax,-1
    je .done
    mov eax,[themePresets+eax*4]
    mov [themeRGB],eax
    jmp .applied
.windowRow:
    cmp [y],248
    jb .done
    cmp [y],288
    ja .textureRow
    stdcall SwatchAt,[x]
    cmp eax,-1
    je .done
    mov eax,[windowPresets+eax*4]
    mov [themeWindowRGB],eax
    jmp .applied
.textureRow:
    cmp [y],332
    jb .done
    cmp [y],368
    ja .actions
    cmp [x],703
    jb .done
    mov eax,[x]
    sub eax,703
    xor edx,edx
    mov ecx,91
    div ecx
    cmp eax,3
    ja .done
    mov [themeTexture],eax
.applied:
    stdcall ThemeApply
    stdcall RefreshPersonalizationColor
    jmp .done
.actions:
    cmp [y],530
    jb .bottom
    cmp [y],572
    ja .bottom
    cmp [x],440
    jb .done
    cmp [x],560
    jbe .motionOn
    cmp [x],572
    jb .done
    cmp [x],692
    jbe .motionOff
    ; appearance: Light / Dark / System
    cmp [y],532
    jb .done
    cmp [y],570
    ja .done
    mov eax,[x]
    sub eax,839
    jb .done
    xor edx,edx
    mov ecx,62
    div ecx
    cmp eax,2
    ja .done
    mov [themeMode],eax
    stdcall ThemeApply
    ; the mode is saved at once; it needs no Apply
    cinvoke wsprintfW,themeHex,themeDecimalFormat,[themeMode]
    invoke WritePrivateProfileStringW,sectionName,themeKeyMode,themeHex,iniPath
    jmp .done
.motionOff:
    mov [themeMotion],0
    jmp .motion
.motionOn:
    mov [themeMotion],1
.motion:
    stdcall ThemeApply
    jmp .done
.bottom:
    cmp [y],620
    jb .done
    cmp [y],662
    ja .done
    cmp [x],252
    jb .done
    cmp [x],452
    jbe .apply
    cmp [x],470
    jb .done
    cmp [x],670
    ja .done
    mov [themeRGB],00071E3h
    mov [themeWindowRGB],0F5F6F8h
    mov [themeTexture],0
    mov [themeMotion],1
    mov [themeMode],2
    mov word [themeName],0
    jmp .save
.apply:
    invoke GetWindowTextW,[hThemeColor],themeHex,16
    stdcall ThemeParseHex,themeHex,addr parsed
    test eax,eax
    jz .invalid
    mov eax,[parsed]
    mov ebx,eax
    invoke GetWindowTextW,[hThemeWindow],themeWinHex,16
    stdcall ThemeParseHex,themeWinHex,addr parsed
    test eax,eax
    jz .invalid
    invoke GetWindowTextW,[hThemeName],themeInput,40
    stdcall ThemeValidName,themeInput
    test eax,eax
    jz .invalid
    invoke lstrcpyW,themeName,themeInput
    mov [themeRGB],ebx
    mov eax,[parsed]
    mov [themeWindowRGB],eax
.save:
    stdcall ThemeApply
    stdcall RefreshPersonalizationControls
    stdcall ThemeSave
    test eax,eax
    jz .saveFailed
    invoke lstrcpyW,statusText,sPersonalSaved
    jmp .redraw
.saveFailed:
    invoke lstrcpyW,statusText,sSaveFailed
    jmp .redraw
.invalid:
    invoke lstrcpyW,statusText,sPersonalInvalid
.redraw:
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp

section '.thsrgb' data readable
themeSrgb:
    dd 0,30,61,91,121,152,182,212,243,273,304,335,368,402,439,478
    dd 518,561,605,651,700,750,802,857,913,972,1033,1096,1161,1229,1298,1370
    dd 1444,1521,1600,1681,1764,1850,1938,2029,2122,2217,2315,2416,2519,2624,2732,2843
    dd 2956,3071,3190,3310,3434,3560,3689,3820,3955,4092,4231,4374,4519,4667,4817,4971
    dd 5127,5286,5448,5613,5781,5951,6125,6301,6480,6663,6848,7036,7227,7421,7619,7819
    dd 8022,8228,8438,8650,8866,9084,9306,9531,9759,9990,10224,10462,10702,10946,11193,11444
    dd 11697,11954,12214,12477,12744,13014,13287,13563,13843,14126,14413,14703,14996,15293,15593,15896
    dd 16203,16513,16827,17144,17465,17789,18116,18447,18782,19120,19462,19807,20156,20508,20864,21223
    dd 21586,21953,22323,22697,23074,23455,23840,24228,24620,25016,25415,25818,26225,26636,27050,27468
    dd 27889,28315,28744,29177,29614,30054,30499,30947,31399,31855,32314,32778,33245,33716,34191,34670
    dd 35153,35640,36131,36625,37124,37626,38133,38643,39157,39676,40198,40724,41254,41789,42327,42869
    dd 43415,43966,44520,45079,45641,46208,46778,47353,47932,48515,49102,49693,50289,50888,51492,52100
    dd 52712,53328,53948,54572,55201,55834,56471,57112,57758,58408,59062,59720,60383,61050,61721,62396
    dd 63076,63760,64448,65141,65837,66539,67244,67954,68669,69387,70110,70838,71569,72306,73046,73791
    dd 74540,75294,76052,76815,77582,78354,79130,79910,80695,81485,82279,83077,83880,84687,85499,86316
    dd 87137,87962,88792,89627,90466,91310,92158,93011,93869,94731,95597,96469,97345,98225,99110,100000
