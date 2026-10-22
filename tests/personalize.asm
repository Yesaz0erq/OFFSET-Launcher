; Native theme parsing, contrast, Unicode naming and isolated INI roundtrip.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
section '.data' data readable writeable
hWnd dd 0
dpi dd 96
busy dd 0
caseIndex dd 1
emptyW dw 0
sectionName du 'Launcher',0
iniPath du 'tests\out\personalize-native.ini',0
sBrand du 'OFFSET',0
sTitle du 'OFFSET Launcher',0
sampleHex du '0071E3',0
sampleUpper du '#7A3FBD',0
sampleLower du '0f85ac',0
sampleShort du '#12345',0
sampleLong du '0011223',0
sampleInvalid du '00GG11',0
sampleSign du '-001122',0
sampleSpace du ' 001122',0
sampleName du 'OFFSET ',0
sampleUnicode dw 504Fh,79FBh,20h,65E5h,672Ch,0
sampleControl dw 'A',13,'B',0
sampleSurrogate dw 0D83Dh,0DE00h,0
sampleBroken dw 0D83Dh,0
sampleLow dw 0DE00h,0
sampleLongName dw 37 dup('A'),0
keyForeign du 'Unrelated',0
valueForeign du 'preserve',0
keyInvalid du 'LauncherName',0
state dd 0
temp rb 256
section '.text' code readable executable
start:
    stdcall ThemeParseHex,sampleHex,state
    cmp eax,1
    jne failed
    cmp [state],00071E3h
    jne failed
    stdcall ThemeParseHex,sampleUpper,state
    cmp eax,1
    jne failed
    cmp [state],07A3FBDh
    jne failed
    stdcall ThemeParseHex,sampleLower,state
    cmp eax,1
    jne failed
    cmp [state],00F85ACh
    jne failed
    inc [caseIndex]
    mov [state],0123456h
    stdcall ThemeParseHex,sampleShort,state
    test eax,eax
    jnz failed
    stdcall ThemeParseHex,sampleLong,state
    test eax,eax
    jnz failed
    stdcall ThemeParseHex,sampleInvalid,state
    test eax,eax
    jnz failed
    stdcall ThemeParseHex,sampleSign,state
    test eax,eax
    jnz failed
    stdcall ThemeParseHex,sampleSpace,state
    test eax,eax
    jnz failed
    cmp [state],0123456h
    jne failed
    inc [caseIndex]
    stdcall ThemeNormalizeRGB,00071E3h
    cmp eax,00071E3h
    jne failed
    stdcall ThemeNormalizeRGB,0FFFFFFh
    cmp eax,0FFFFFFh
    je failed
    stdcall ThemeLuminance,eax
    cmp eax,183333333
    ja failed
    stdcall ThemeNormalizeRGB,0FFFF00h
    stdcall ThemeLuminance,eax
    cmp eax,183333333
    ja failed
    stdcall ThemeNormalizeRGB,0FFFFFF00h
    stdcall ThemeLuminance,eax
    cmp eax,183333333
    ja failed
    stdcall ThemeNormalizeRGB,0
    test eax,eax
    jnz failed
    inc [caseIndex]
    stdcall ThemeValidName,sampleUnicode
    cmp eax,1
    jne failed
    stdcall ThemeValidName,sampleSurrogate
    cmp eax,1
    jne failed
    stdcall ThemeValidName,emptyW
    cmp eax,1
    jne failed
    stdcall ThemeValidName,sampleControl
    test eax,eax
    jnz failed
    stdcall ThemeValidName,sampleBroken
    test eax,eax
    jnz failed
    stdcall ThemeValidName,sampleLow
    test eax,eax
    jnz failed
    stdcall ThemeValidName,sampleLongName
    test eax,eax
    jnz failed
    inc [caseIndex]
    mov [themeRGB],07A3FBDh
    stdcall ThemeApply
    cmp [themeAccentARGB],0FF7A3FBDh
    jne failed
    cmp [themeAccentText],0BD3F7Ah
    jne failed
    cmp [themeFocusARGB],407A3FBDh
    jne failed
    mov eax,[themeAccentLo]
    cmp eax,[themeAccentARGB]
    jae failed
    mov [themeRGB],00071E3h
    stdcall ThemeApply
    cmp [themeBlueprint1],0FF0D3168h
    jne failed
    cmp [themeAccentHi],0FF1A84F0h
    jne failed
    inc [caseIndex]
    invoke CreateDirectoryW,testOut,0
    invoke DeleteFileW,iniPath
    invoke WritePrivateProfileStringW,sectionName,keyForeign,valueForeign,iniPath
    test eax,eax
    jz failed
    invoke lstrcpyW,themeName,sampleUnicode
    mov [themeRGB],07A3FBDh
    mov [themeMotion],0
    stdcall ThemeSave
    cmp eax,1
    jne failed
    mov [themeRGB],0
    mov [themeMotion],1
    mov word [themeName],0
    stdcall ThemeLoad
    cmp [themeRGB],07A3FBDh
    jne failed
    cmp [themeMotion],0
    jne failed
    invoke lstrcmpiW,themeName,sampleUnicode
    test eax,eax
    jnz failed
    ; Surrogate pairs roundtrip even when the INI file itself is ANSI.
    invoke lstrcpyW,themeName,sampleSurrogate
    stdcall ThemeSave
    test eax,eax
    jz failed
    mov word [themeName],0
    stdcall ThemeLoad
    invoke lstrcmpiW,themeName,sampleSurrogate
    test eax,eax
    jnz failed
    invoke GetPrivateProfileStringW,sectionName,keyForeign,emptyW,temp,128,iniPath
    invoke lstrcmpiW,temp,valueForeign
    test eax,eax
    jnz failed
    inc [caseIndex]
    ; Invalid persisted values fall back, preserving unrelated launcher fields.
    invoke WritePrivateProfileStringW,sectionName,themeKeyColor,sampleInvalid,iniPath
    invoke WritePrivateProfileStringW,sectionName,themeKeyMotion,sampleHex,iniPath
    invoke WritePrivateProfileStringW,sectionName,themeKeyName,sampleLongName,iniPath
    invoke WritePrivateProfileStringW,sectionName,themeKeyNameHex,sampleInvalid,iniPath
    stdcall ThemeLoad
    cmp [themeRGB],00071E3h
    jne failed
    cmp [themeMotion],1
    jne failed
    cmp word [themeName],0
    jne failed
    stdcall ThemeName
    cmp eax,sBrand
    jne failed
    invoke lstrcpyW,themeName,sampleUnicode
    stdcall ThemeName
    cmp eax,themeName
    jne failed
    inc [caseIndex]
    ; Motion snap uses the real shared spring state even without a window.
    mov [themeMotion],0
    mov [animTgt],4096
    mov [animCur],0
    mov [heroActive],1
    mov [transActive],1
    mov [statusStart],1
    stdcall ThemeMotionSync
    cmp [animCur],4096
    jne failed
    cmp [heroActive],0
    jne failed
    cmp [transActive],0
    jne failed
    cmp [statusStart],0
    jne failed
    stdcall AnimSet,1,512,0
    cmp [animCur+4],8192
    jne failed
    invoke DeleteFileW,iniPath
    invoke ExitProcess,0
failed:
    invoke ExitProcess,[caseIndex]
testOut du 'tests\out',0
include '..\src\ui_gfx.inc'
include '..\src\personalize.inc'
include '..\src\imports.inc'
