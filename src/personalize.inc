; Appearance, native control colors and the optional custom palette.
section '.thdata' data readable writeable
themeStyle dd STYLE_BLUEPRINT
themeKeyStyle du 'ThemeStyle',0
themeStyleNames dd sThemeBlueprint,sThemeMinecraft
themeRGB dd 00071E3h
themeMotion dd 1
themeAccentARGB dd 0FF0071E3h
themeAccentHi dd 0FF1A84F0h
themeAccentLo dd 0FF0062C6h
themeAccentSoft dd 0FFE5F0FDh
themeAccentText dd 0E37100h
themeFocusARGB dd 400071E3h
themeHeroARGB dd 0FF0D3168h
themeBlueprintEnd dd 0FF071B3Eh
themeDotARGB dd 0FFD3D8E0h
themeCrossARGB dd 0FFC9CFD9h
sideInkCR dd 0FFFFFFh
sideMutedCR dd 0C0C5D0h
sideIconARGB dd 0FFD0C5C0h
sideSelectedIcon dd 0FF86D562h
themeControlLock dd 0
hThemeColor dd 0
hThemeName dd 0
hThemeWindow dd 0
themePresets dd 00071E3h,07A3FBDh,0007D80h,0227546h,0B94D00h,0B83262h
; Blueprint custom colors are retained while the Minecraft theme is active.
themeWindowRGB dd 0F5F6F8h
themeCanvasARGB dd 0FFF5F6F8h
themeSidebarARGB dd 0FFEEF0F4h
themeSidelineARGB dd 0FFDFE3E9h
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
; Blueprint paired color presets.
schemeAccent dd 00071E3h,0227546h,0B94D00h,0007D80h,0B83262h,07A3FBDh
schemeWindow dd 0F5F6F8h,0EEF5EEh,0F7F1E6h,0EEF3FAh,0F8EEF1h,0F2EFF8h
schemeNames dd sSchemeBlueprint,sSchemeMeadow,sSchemeDesert,sSchemeOcean,sSchemeCherry,sSchemeAmethyst
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
    ; Windows 11 frame corners follow the geometry of the selected theme.
    mov eax,1                              ; DWMWCP_DONOTROUND
    cmp [themeStyle],STYLE_MINECRAFT
    je @f
    inc eax                                ; DWMWCP_ROUND
@@:
    mov [flag],eax
    invoke DwmSetWindowAttribute,[hWnd],33,addr flag,4
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
    cmp [themeStyle],STYLE_MINECRAFT
    je .minecraft
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
    mov [themeHeroARGB],eax
    stdcall Mix,0FF07111Bh,ebx,42
    mov [themeBlueprintEnd],eax
    cmp [themeRGB],00071E3h
    jne .window
    ; Keep the existing design exactly when the default theme is selected.
    mov [themeAccentHi],0FF1A84F0h
    mov [themeAccentLo],0FF0062C6h
    mov [themeAccentSoft],0FFE5F0FDh
    mov [themeHeroARGB],0FF0D3168h
    mov [themeBlueprintEnd],0FF071B3Eh
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
    cmp [themeWindowRGB],0F5F6F8h
    jne .classicSide
    ; the default window keeps the original palette exactly
    mov [themeSidebarARGB],0FFEEF0F4h
    mov [themeSidelineARGB],0FFDFE3E9h
    mov [themeDotARGB],0FFD3D8E0h
    mov [themeCrossARGB],0FFC9CFD9h
    jmp .classicSide
.dark:
    ; Dark surfaces step up in lightness from a tinted near-black canvas.
    stdcall ThemeDarkCanvas,[themeWindowRGB]
    mov [themeCanvasARGB],eax
    mov ebx,eax
    tmix themeSidebarARGB,ebx,0FF000000h,64
    tmix themeSidelineARGB,ebx,0FFFFFFFFh,22
    tmix themeDotARGB,ebx,0FFFFFFFFh,34
    tmix themeCrossARGB,ebx,0FFFFFFFFh,50
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
    mov [palTInk],0F5F2F2h
    mov [palTSecond],0B5ADA9h
    mov [palTMuted],08B827Dh
    ; accent text needs to be lighter on dark; selection tint sits on cards
    stdcall Mix,[themeAccentARGB],0FFFFFFFFh,96
    stdcall ThemeCR,eax
    mov [themeAccentText],eax
    tmix themeAccentSoft,[palCard],[themeAccentARGB],60
.classicSide:
    mov eax,[palTInk]
    mov [sideInkCR],eax
    mov eax,[palTSecond]
    mov [sideMutedCR],eax
    mov eax,[palMuted]
    mov [sideIconARGB],eax
    mov eax,[themeAccentARGB]
    mov [sideSelectedIcon],eax
    jmp .colorRefs
.minecraft:
    ; Minecraft core colors. The navigation stays charcoal in either mode.
    mov [themeHeroARGB],0FF192C23h
    mov [themeSidebarARGB],0FF171615h
    mov [themeSidelineARGB],0FF3D3938h
    mov [sideInkCR],0FFFFFFh
    mov [sideMutedCR],0C0C5D0h
    mov [sideIconARGB],0FFD0C5C0h
    mov [sideSelectedIcon],0FF86D562h
    mov [themeAccentARGB],0FF347323h
    mov [themeAccentHi],0FF397B26h
    mov [themeAccentLo],0FF224D16h
    mov [themeFocusARGB],806CC349h
    cmp [themeDark],0
    jne .minecraftDark
    mov [themeCanvasARGB],0FFFCF5F1h
    mov [palCard],0FFFFFFFFh
    mov [palHairline],0FFD0C5C0h
    mov [palDivider],0FFE2D9D5h
    mov [palField],0FFFCF9F7h
    mov [palFieldLine],0FF9C938Fh
    mov [palTrack],0FFEDE5E2h
    mov [palSecond],0FFEDE5E2h
    mov [palSecondHi],0FFE0D6D1h
    mov [palSecondLo],0FFB7ACA6h
    mov [palInk],0FF262423h
    mov [palTInk],0232426h
    mov [palMuted],0FF6B6562h
    mov [palTMuted],062656Bh
    mov [palTSecond],04E5156h
    mov [palHover],0
    mov [themeAccentText],01C642Ah
    mov [themeAccentSoft],0FFE0F0D9h
    jmp .colorRefs
.minecraftDark:
    mov [themeCanvasARGB],0FF171615h
    mov [palCard],0FF262423h
    mov [palHairline],0FF514D4Ah
    mov [palDivider],0FF3D3938h
    mov [palField],0FF1E1D1Ch
    mov [palFieldLine],0FF79716Dh
    mov [palTrack],0FF171615h
    mov [palSecond],0FF494543h
    mov [palSecondHi],0FF595552h
    mov [palSecondLo],0FF302E2Ch
    mov [palInk],0FFFCF5F1h
    mov [palTInk],0F1F5FCh
    mov [palMuted],0FFABA09Ch
    mov [palTMuted],09CA0ABh
    mov [palTSecond],0C0C5D0h
    mov [palHover],0FFFFFFh
    mov [themeAccentText],062D586h
    mov [themeAccentSoft],0FF2B4227h
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
    jnz .style
    mov [themeWindowRGB],0F5F6F8h
.style:
    invoke GetPrivateProfileIntW,sectionName,themeKeyStyle,STYLE_BLUEPRINT,iniPath
    cmp eax,STYLE_MINECRAFT
    je @f
    mov eax,STYLE_BLUEPRINT
@@:
    mov [themeStyle],eax
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
    ; Discard the obsolete INI entry when saving an upgraded profile.
    invoke WritePrivateProfileStringW,sectionName,themeKeyTexture,0,iniPath
    cinvoke wsprintfW,themeHex,themeDecimalFormat,[themeStyle]
    invoke WritePrivateProfileStringW,sectionName,themeKeyStyle,themeHex,iniPath
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
    stdcall ShowCustomColorInputs
    invoke ShowWindow,[hThemeName],SW_SHOW
    ret
endp

proc ShowCustomColorInputs uses ebx
    mov ebx,SW_HIDE
    cmp [themeStyle],STYLE_BLUEPRINT
    jne @f
    mov ebx,SW_SHOW
@@:
    invoke ShowWindow,[hThemeColor],ebx
    invoke ShowWindow,[hThemeWindow],ebx
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
    cmp [themeStyle],STYLE_BLUEPRINT
    jne .done
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

; Blueprint palette shortcuts below the overall theme selector.
SCHEME_X = 276
SCHEME_Y = 278
SCHEME_W = 112
SCHEME_H = 48
SCHEME_DX = 124
SCHEME_DY = 60

; Returns 1 when the current theme equals scheme [index].
proc ThemeSchemeActive,index
    mov ecx,[index]
    mov eax,[schemeAccent+ecx*4]
    cmp eax,[themeRGB]
    jne .no
    mov eax,[schemeWindow+ecx*4]
    cmp eax,[themeWindowRGB]
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
    stdcall ThemeTilePos,[index]
    mov ebx,eax
    mov edi,edx
    stdcall HotAdd,ebx,edi,SCHEME_W,SCHEME_H
    mov esi,[index]
    mov eax,[schemeWindow+esi*4]
    or eax,0FF000000h
    cmp [themeDark],0
    je @f
    stdcall ThemeDarkCanvas,eax
@@:
    stdcall FillRR,ebx,edi,SCHEME_W,SCHEME_H,6,eax
    mov eax,[schemeAccent+esi*4]
    or eax,0FF000000h
    lea ecx,[ebx+8]
    lea edx,[edi+10]
    stdcall FillRR,ecx,edx,5,28,2,eax
    lea ecx,[ebx+20]
    stdcall Text,0,dword [schemeNames+esi*4],ecx,edi,88,SCHEME_H,[fSmall],T_INK
    stdcall ThemeSchemeActive,esi
    test eax,eax
    jz .plain
    stdcall StrokeRR,ebx,edi,SCHEME_W,SCHEME_H,6,[themeAccentARGB],20
    ret
.plain:
    stdcall StrokeRR,ebx,edi,SCHEME_W,SCHEME_H,6,A_HAIRLINE,10
    ret
endp

; A miniature client shares the live surfaces and the home illustration.
proc PaintWorldPreview,dc
    stdcall Text,[dc],sThemePreview,700,141,355,26,[fMedium],T_INK
    stdcall FillRR,700,184,360,200,0,A_CANVAS
    stdcall FillRR,700,184,69,200,0,[themeSidebarARGB]
    stdcall DrawLogo,719,200,30,0FFFCF5F1h,0,360
    stdcall FillRR,709,244,50,14,0,0FF3D3938h
    stdcall FillRR,709,244,3,14,0,0FF86D562h
    stdcall FillRR,719,269,32,3,0,0FF6B6562h
    stdcall FillRR,719,283,23,3,0,0FF6B6562h
    stdcall FillRR,782,199,265,95,0,[themeHeroARGB]
    stdcall DrawWorldScene,1047,294,3,255
    stdcall Text,[dc],sBrand,794,210,120,26,[fBrand],T_WHITE
    stdcall FillRR,782,304,265,63,0,A_CARD
    stdcall StrokeRR,782,304,265,63,0,A_HAIRLINE,10
    stdcall Text,[dc],sHeroTag,791,312,140,44,[fSmall],T_INK
    stdcall FillRR,938,318,99,34,0,[themeAccentLo]
    stdcall FillRR,938,318,99,31,0,[themeAccentARGB]
    stdcall TextC,[dc],sLaunch,938,318,99,31,[fSmall],T_WHITE
    stdcall StrokeRR,700,184,360,200,0,A_HAIRLINE,10
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
    stdcall Card,252,124,404,300,18
    stdcall Card,672,124,416,300,18
    stdcall Text,[dc],sThemeOverall,280,140,360,26,[fMedium],T_INK
    mov eax,[themeStyle]
    xor eax,1                              ; Blueprint first, preserved INI values
    stdcall Segmented,276,182,368,44,2,9,themeStyleNames,eax,[fBodyStrong]
    cmp [themeStyle],STYLE_BLUEPRINT
    je .classic
    stdcall Text,[dc],sThemeColors,280,249,360,24,[fMedium],T_INK
    stdcall FillRR,280,292,110,32,0,[themeAccentARGB]
    stdcall FillRR,402,292,110,32,0,[themeSidebarARGB]
    stdcall FillRR,524,292,110,32,0,A_CARD
    stdcall StrokeRR,524,292,110,32,0,A_HAIRLINE,10
    stdcall Text,[dc],sPersonalColor,280,332,116,28,[fSmall],T_SECOND
    stdcall Text,[dc],sThemeNav,402,332,116,28,[fSmall],T_SECOND
    stdcall Text,[dc],sThemeSurface,524,332,116,28,[fSmall],T_SECOND
    stdcall PaintWorldPreview,[dc]
    jmp .preferences
.classic:
    stdcall Text,[dc],sPersonalScheme,280,242,360,26,[fMedium],T_INK
    xor esi,esi
.tile:
    stdcall PaintSchemeTile,esi
    inc esi
    cmp esi,6
    jb .tile
    stdcall Text,[dc],sPersonalColor,700,140,270,26,[fMedium],T_INK
    stdcall PaintSwatchRow,188,themePresets,[themeRGB],0
    stdcall Field,980,170,92,36,[hThemeColor]
    stdcall Text,[dc],sPersonalWindow,700,220,270,26,[fMedium],T_INK
    stdcall PaintSwatchRow,268,windowPresets,[themeWindowRGB],1
    stdcall Field,980,250,92,36,[hThemeWindow]
    stdcall Text,[dc],sPersonalHelp,700,330,370,48,[fSmall],T_MUTED
.preferences:
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
    cmp [y],182
    jb .palette
    cmp [y],226
    jae .palette
    mov eax,[x]
    sub eax,279
    jb .palette
    cmp eax,362
    jae .palette
    cmp eax,181
    setb al
    movzx eax,al
    cmp eax,[themeStyle]
    je .done
    mov [themeStyle],eax
    stdcall ThemeApply
    stdcall RefreshPersonalizationColor
    stdcall ShowCustomColorInputs
    jmp .done
.palette:
    cmp [themeStyle],STYLE_BLUEPRINT
    jne .actions
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
    ja .actions
    stdcall SwatchAt,[x]
    cmp eax,-1
    je .done
    mov eax,[windowPresets+eax*4]
    mov [themeWindowRGB],eax
    jmp .applied
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
    mov [themeStyle],STYLE_BLUEPRINT
    mov [themeMotion],1
    mov [themeMode],2
    mov word [themeName],0
    stdcall ShowCustomColorInputs
    jmp .save
.apply:
    cmp [themeStyle],STYLE_BLUEPRINT
    jne .name
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
    mov [themeRGB],ebx
    mov eax,[parsed]
    mov [themeWindowRGB],eax
.name:
    invoke GetWindowTextW,[hThemeName],themeInput,40
    stdcall ThemeValidName,themeInput
    test eax,eax
    jz .invalid
    invoke lstrcpyW,themeName,themeInput
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
