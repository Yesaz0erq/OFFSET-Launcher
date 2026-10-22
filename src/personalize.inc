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
themePresets dd 00071E3h,07A3FBDh,0007D80h,0227546h,0B94D00h,0B83262h
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

proc ThemeApply uses ebx
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
    cmp [hWnd],0
    je .done
    stdcall ThemeWindowTitle
    invoke SetWindowTextW,[hWnd],eax
    stdcall ThemeMotionSync
    mov [transActive],0
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
    mov [themeControlLock],0
    ret
endp

proc RefreshPersonalizationControls
    stdcall RefreshPersonalizationColor
    invoke SetWindowTextW,[hThemeName],themeName
    ret
endp

proc CreatePersonalizationControls
    stdcall NewControl,editClass,ES_AUTOHSCROLL,501,452,270,116,24
    mov [hThemeColor],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,7,0
    stdcall NewControl,editClass,ES_AUTOHSCROLL,502,452,424,564,24
    mov [hThemeName],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,36,0
    stdcall RefreshPersonalizationControls
    ret
endp

proc ShowPersonalizationControls
    stdcall RefreshPersonalizationControls
    invoke ShowWindow,[hThemeColor],SW_SHOW
    invoke ShowWindow,[hThemeName],SW_SHOW
    ret
endp

proc HidePersonalizationControls
    invoke ShowWindow,[hThemeColor],SW_HIDE
    invoke ShowWindow,[hThemeName],SW_HIDE
    ret
endp

proc EnablePersonalizationControls,enabled
    invoke EnableWindow,[hThemeColor],[enabled]
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
    cmp eax,501
    jne .done
    cmp edx,EN_CHANGE
    jne .done
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

proc PaintPersonalization uses ebx esi,dc
    stdcall PageHeader,themeEyebrow,sPersonalHeading,7
    stdcall Card,252,124,836,236,18
    stdcall Text,[dc],sPersonalColor,280,144,730,26,[fMedium],T_INK
    xor esi,esi
.swatch:
    imul ebx,esi,90
    add ebx,306
    mov eax,[themePresets+esi*4]
    or eax,0FF000000h
    stdcall FillCircle,ebx,212,22,eax
    mov eax,[themePresets+esi*4]
    cmp eax,[themeRGB]
    jne .register
    stdcall ArcA,ebx,212,28,0,360,[themeAccentARGB],15,0
.register:
    mov eax,ebx
    sub eax,30
    stdcall HotAdd,eax,182,60,60
    inc esi
    cmp esi,6
    jb .swatch
    stdcall Text,[dc],sPersonalCustom,280,268,155,26,[fBody],T_SECOND
    stdcall Field,440,264,140,36,[hThemeColor]
    stdcall Text,[dc],sPersonalInvalid,280,314,760,26,[fSmall],T_MUTED
    stdcall Card,252,380,836,212,18
    stdcall Text,[dc],sPersonalName,280,424,150,26,[fBody],T_SECOND
    stdcall Field,440,418,588,36,[hThemeName]
    stdcall Text,[dc],sPersonalMotion,280,492,150,26,[fBody],T_SECOND
    mov eax,0
    cmp [themeMotion],0
    je .on
    inc eax
.on:
    stdcall Button,[dc],sPersonalOn,440,482,140,42,eax
    mov eax,0
    cmp [themeMotion],0
    jne .off
    inc eax
.off:
    stdcall Button,[dc],sPersonalOff,598,482,140,42,eax
    stdcall Button,[dc],sPersonalApply,252,620,200,42,1
    stdcall Button,[dc],sPersonalReset,470,620,200,42,0
    stdcall StatusLine,254,676,830,32
    ret
endp

proc ClickPersonalization uses ebx esi,x,y
    local parsed:DWORD
    cmp [y],182
    jb .actions
    cmp [y],242
    ja .actions
    xor esi,esi
.swatch:
    imul ebx,esi,90
    add ebx,276
    cmp [x],ebx
    jb .next
    add ebx,60
    cmp [x],ebx
    ja .next
    mov eax,[themePresets+esi*4]
    mov [themeRGB],eax
    stdcall ThemeApply
    stdcall RefreshPersonalizationColor
    jmp .done
.next:
    inc esi
    cmp esi,6
    jb .swatch
.actions:
    cmp [y],482
    jb .bottom
    cmp [y],524
    ja .bottom
    cmp [x],440
    jb .done
    cmp [x],580
    jbe .motionOn
    cmp [x],598
    jb .done
    cmp [x],738
    ja .done
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
    mov [themeMotion],1
    mov word [themeName],0
    jmp .save
.apply:
    invoke GetWindowTextW,[hThemeColor],themeHex,16
    stdcall ThemeParseHex,themeHex,addr parsed
    test eax,eax
    jz .invalid
    invoke GetWindowTextW,[hThemeName],themeInput,40
    stdcall ThemeValidName,themeInput
    test eax,eax
    jz .invalid
    invoke lstrcpyW,themeName,themeInput
    mov eax,[parsed]
    mov [themeRGB],eax
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
