; OFFSET Launcher - original native x86 assembly, no managed runtime.
format PE GUI 6.0
stack 100000h,40000h
entry start
include 'win32ax.inc'
include 'mod_types.inc'
include 'version_types.inc'
include 'servers_types.inc'
; Sidebar entries are the primary pages. Personalization and logs remain
; available from Settings so the rail stays compact.
NAV_COUNT = 7
WM_APP = 8000h
OFFSET_I18N = 1
OFFSET_VERSION_SETTINGS = 1
OFFSET_LAUNCH_LOG = 1
OFFSET_ACCOUNT_SESSION = 1
OFFSET_LOADER_CHANGE = 1
OFFSET_BROWSE_DEFAULTS = 1

WM_STATUS = WM_APP+1
WM_FINISHED = WM_APP+2
WM_MOUSELEAVE = 02A3h
HTCLIENT = 1
HTCAPTION = 2
SWP_FRAMECHANGED = 0020h
WIN_STYLE = WS_OVERLAPPED+WS_CAPTION+WS_SYSMENU+WS_MINIMIZEBOX+WS_CLIPCHILDREN
; custom window buttons (logical, top-right)
WB_Y = 6
WB_H = 28
WB_W = 34
WB_MIN_X = 1040
WB_CLOSE_X = 1078
A_CLOSE = 0FFE5484Dh
; Legacy COLORREF names used by page modules.
C_INK = T_INK
C_MUTED = T_SECOND
C_BLUE = T_ACCENT
C_LINE = 0EBE6E3h

section '.data' data readable writeable
hWnd dd 0
hInstance dd 0
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
statusLock dd 0
currentPage dd 0
lastPage dd -1
suppressPageHeader dd 0
headerFadeAlpha dd 0
busy dd 0
workerHandle dd 0
workerCallback dd 0
dpi dd 96
paintDC dd 0
hVersion dd 0
hPlayer dd 0
hClient dd 0
hGameDir dd 0
hJava dd 0
hMemory dd 0
hJvmArgs dd 0
fSmall dd 0
fBody dd 0
fBodyStrong dd 0
fMedium dd 0
fHeading dd 0
fBrand dd 0
fInput dd 0
fHero dd 0
fTitle dd 0
fMono dd 0
fMonoSmall dd 0
inputBrush dd 0
hoverItem dd -1
mouseTrack dd 16,2,0,0
mouseTracking dd 0
tickCount dd 0
defaultPlayer db 'Player',0
defaultVersion db '1.21.1',0
defaultClientId db '54048348-72bb-4ed2-a8a4-c72423265e94',0
className du 'OffsetLauncherWindow',0
editClass du 'EDIT',0
comboClass du 'COMBOBOX',0
monoName du 'Consolas',0
emptyW dw 0
openVerb du 'open',0
sectionName du 'Launcher',0
kGame du 'GameDirectory',0
kJava du 'Java',0
kName du 'Player',0
kVersion du 'Version',0
kMemory du 'MemoryMiB',0
kJvmArgs du 'JvmArgs',0
kClient du 'MicrosoftClientID',0
kMode du 'Online',0
iniSuffix du '\offset.ini',0
fmtRam du '%u MiB',0
fmtNum du '%u',0
; Blueprint annotations (decorative, ASCII mono).
eyebrowHome du '01 / LAUNCH',0
eyebrowVersions du '02 / VERSIONS',0
eyebrowAccount du '03 / ACCOUNT',0
eyebrowSettings du '04 / SETTINGS',0
eyebrowMods du '05 / MODS',0
sheetFmt du 'SHEET %u / 09',0
monoWorkspace du 'WORKSPACE',0
monoTagline du 'MINECRAFT LAUNCHER',0
monoBuild du 'x86 / WIN32 / 0.9.0',0
monoDrawing du 'DWG OFFSET-001    SCALE 1:1    REV 0.9',0
monoFigure du 'FIG. 01',0
monoRadius du 'R 66',0
downloadUiStats rd 2
downloadUiText rw 128
monoAuth du 'AUTH / DEVICE CODE',0
monoSelected du 'SELECTED BUILD',0
monoProfile du 'A',0
monoSession du 'B',0
eyebrowPacks du '06 / MODPACKS',0
eyebrowPersonal du '07 / PERSONALIZE',0
eyebrowDetails du 'PROJECT / DETAILS',0
; Page ids behind the compact sidebar: Home, Versions, Account, Settings,
; Mods, Packs and Servers. Settings owns the secondary pages.
navPageIds dd 0,1,2,3,4,5,8
navKeys du '1',0,'2',0,'3',0,'4',0,'5',0,'6',0,'7',0
navLabels dd sHome,sVersions,sAccount,sSettings,sMods,sPacks,sServersHeading
sJvmArgsLabel du 'Java 虚拟机参数',0
segLabels dd 4 dup(0)
createControls dd 0
modTab dd 0
modAction dd 0
hModQuery dd 0
hModGame dd 0
hModLoader dd 0
hModProjects dd 0
hModFiles dd 0
hModLocal dd 0
hCurseKey dd 0
modControls dd 7 dup(0)
listClass du 'LISTBOX',0
modFabric db 'fabric',0
modForge db 'forge',0
modNeoForge db 'neoforge',0
modQuilt db 'quilt',0
modLoaderNames dd modFabric,modForge,modNeoForge,modQuilt
windowRect RECT 0,0,1120,760
wc WNDCLASSEX sizeof.WNDCLASSEX,CS_HREDRAW+CS_VREDRAW,WindowProc,0,0,0,0,0,0,0,className,0
msg MSG
icex dd 8,00000400h
roundCorners dd 2
ncPt POINT
minHot dd -1
closeHot dd -1
include 'strings.inc'
; All large zero-initialized buffers trail initialized data, so PE stores only
; their virtual size. This keeps the portable executable small.
gameDir rw 1024
javaPath rw 1024
jvmArgs rw 4096
playerName rb 64
selectedVersion rb 128
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
statusText rw 1024
statusIncoming rw 1024
iniPath rw 1024
textBuffer rw 2048
ramText rw 64
accountText rw 256
pathBuffer rw 1024
modUiText rw 2048
modUiDetail rw 2048
heroText rw 128
sheetText rw 32

section '.text' code readable executable
start:
    invoke SetProcessDPIAware
    invoke GetDpiForSystem
    mov [dpi],eax
    invoke GetModuleHandleW,0
    mov [hInstance],eax
    mov [wc.hInstance],eax
    invoke LoadCursorW,0,IDC_ARROW
    mov [wc.hCursor],eax
    invoke LoadIconW,[hInstance],1
    mov [wc.hIcon],eax
    mov [wc.hIconSm],eax
    invoke RegisterClassExW,wc
    test eax,eax
    jz .failed
    stdcall GfxStartup
    stdcall MemoryDetect
    cmp [memoryMb],eax
    jbe .memoryReady
    mov [memoryMb],eax
.memoryReady:
    stdcall EngineInit
    stdcall LoadSettings
    stdcall LangLoad
    stdcall LangApply
    stdcall ThemeLoad
    stdcall AuthLoad
    stdcall ModsInit
    stdcall ModsLoadKey
    stdcall SidebarLoad
    stdcall SplashInit
    invoke lstrcpyW,statusText,sReady
    invoke InitCommonControlsEx,icex
    ; Borderless: the caption frame is kept for DWM (shadow, rounded corners,
    ; taskbar minimize) but WM_NCCALCSIZE gives the whole window to the client.
    invoke SystemParametersInfoW,SPI_GETWORKAREA,0,windowRect,0
    stdcall Scale,1120
    mov esi,eax
    stdcall Scale,760
    mov edi,eax
    mov eax,[windowRect.right]
    sub eax,[windowRect.left]
    sub eax,esi
    sar eax,1
    add eax,[windowRect.left]
    mov ebx,eax
    mov eax,[windowRect.bottom]
    sub eax,[windowRect.top]
    sub eax,edi
    sar eax,1
    jns @f
    xor eax,eax
@@:
    add eax,[windowRect.top]
    invoke CreateWindowExW,0,className,sTitle,WIN_STYLE,ebx,eax,esi,edi,0,0,[hInstance],0
    test eax,eax
    jz .failed
    mov [hWnd],eax
    invoke SetWindowPos,eax,0,0,0,0,0,SWP_FRAMECHANGED+SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_NOACTIVATE
    cmp [splashActive],0
    je @f
    stdcall AnimKick
@@:
    invoke DwmSetWindowAttribute,eax,33,roundCorners,4
    invoke ShowWindow,[hWnd],SW_SHOWNORMAL
    invoke UpdateWindow,[hWnd]
    cmp [authHasRefresh],1
    jne .loop
    stdcall StartWorker,AccountRestoreWorker
.loop:
    ; While the interface animates, drain input and render one frame per
    ; display refresh (DwmFlush); otherwise block in GetMessage as before.
    cmp [animTimer],0
    je .wait
    cmp [animVsync],0
    je .wait
.pump:
    invoke PeekMessageW,msg,0,0,0,PM_REMOVE
    test eax,eax
    jz .frame
    cmp [msg.message],WM_QUIT
    je .exit
    stdcall PumpMessage
    jmp .pump
.frame:
    stdcall AnimFrame
    invoke UpdateWindow,[hWnd]
    stdcall FramePace
    jmp .loop
.wait:
    invoke GetMessageW,msg,0,0,0
    test eax,eax
    jle .exit
    stdcall PumpMessage
    jmp .loop
.exit:
    ; The bitmap is detached from its source file. Dispose it before GDI+.
    stdcall AccountUiReleaseImage
    cmp [workerHandle],0
    je .cleanAccount
    invoke WaitForSingleObject,[workerHandle],0
    test eax,eax
    jnz .processExit
.cleanAccount:
    stdcall SkinClear
    stdcall AuthInvalidateSession
    stdcall AuthWipeSecrets
    stdcall AuthWipeRefresh
    invoke GdiplusShutdown,[gdipToken]
.processExit:
    invoke ExitProcess,[msg.wParam]
.failed:
    invoke ExitProcess,1

proc PumpMessage
    cmp [msg.message],WM_KEYDOWN
    jne .dialog
    stdcall KeyInput,[msg.wParam]
    test eax,eax
    jnz .done
.dialog:
    invoke IsDialogMessageW,[hWnd],msg
    test eax,eax
    jnz .done
    invoke TranslateMessage,msg
    invoke DispatchMessageW,msg
.done:
    ret
endp

; Wait for the compositor's next frame; never spin faster than ~120 fps
; (DwmFlush returns at once when composition is unavailable).
proc FramePace
.pace:
    invoke DwmFlush
    stdcall NowMs
    mov ecx,eax
    sub ecx,[lastLoopFrame]
    cmp ecx,8
    jae .ok
    invoke Sleep,1
    jmp .pace
.ok:
    mov [lastLoopFrame],eax
    ret
endp

; One animation step: shared by the paced loop and the timer fallback.
proc AnimFrame
    stdcall SplashStep
    stdcall SidebarStep
    stdcall TransitionStep
    test eax,eax
    jz .tick
    stdcall SyncPage
.tick:
    stdcall AnimTick
    test eax,eax
    jnz .animating
    invoke KillTimer,[hWnd],1
    mov [animTimer],0
    mov [animVsync],0
    invoke InvalidateRect,[hWnd],0,0
    ret
.animating:
    ; Busy indicators use the same compositor-paced frames as navigation.
    inc [tickCount]
.moving:
    mov [animVsync],1
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp

proc Scale value
    invoke MulDiv,[value],[dpi],96
    ret
endp

; size is in logical pixels; the font is created at device size.
proc MakeFont size,weight,face
    stdcall Scale,[size]
    neg eax
    invoke CreateFontW,eax,0,0,0,[weight],0,0,0,1,0,0,5,0,[face]
    ret
endp

proc NewControl uses ebx esi edi,cls,style,id,x,y,w,h
    locals
        px dd ?
        py dd ?
        pw dd ?
        ph dd ?
    endl
    stdcall Scale,[x]
    mov [px],eax
    stdcall Scale,[y]
    mov [py],eax
    stdcall Scale,[w]
    mov [pw],eax
    stdcall Scale,[h]
    mov [ph],eax
    mov eax,[style]
    or eax,WS_CHILD+WS_TABSTOP
    invoke CreateWindowExW,0,[cls],emptyW,eax,[px],[py],[pw],[ph],[hWnd],[id],[hInstance],0
    mov ebx,eax
    invoke SendMessageW,ebx,WM_SETFONT,[fInput],1
    mov eax,ebx
    ret
endp

proc DeleteFonts uses esi
    mov esi,fSmall
.next:
    cmp dword [esi],0
    je .skip
    invoke DeleteObject,dword [esi]
    mov dword [esi],0
.skip:
    add esi,4
    cmp esi,fMonoSmall+4
    jb .next
    ret
endp

; Per-language type: YaHei UI (zh), Segoe UI (en), Yu Gothic UI (ja).
proc CreateFonts uses ebx esi
    stdcall DeleteFonts
    mov eax,[uiLang]
    mov ebx,[langFaces+eax*4]
    mov esi,[langStrongFaces+eax*4]
    stdcall MakeFont,12,400,ebx
    mov [fSmall],eax
    stdcall MakeFont,14,400,ebx
    mov [fBody],eax
    stdcall MakeFont,14,600,esi
    mov [fBodyStrong],eax
    stdcall MakeFont,15,600,esi
    mov [fMedium],eax
    stdcall MakeFont,28,700,esi
    mov [fHeading],eax
    stdcall MakeFont,18,700,esi
    mov [fBrand],eax
    stdcall MakeFont,15,400,ebx
    mov [fInput],eax
    stdcall MakeFont,38,600,esi
    mov [fHero],eax
    stdcall MakeFont,21,600,esi
    mov [fTitle],eax
    stdcall MakeFont,11,400,monoName
    mov [fMono],eax
    stdcall MakeFont,10,400,monoName
    mov [fMonoSmall],eax
    ret
endp

proc SetChildFont child,param
    invoke SendMessageW,[child],WM_SETFONT,[fInput],1
    mov eax,1
    ret
endp

proc ApplyControlFonts
    invoke EnumChildWindows,[hWnd],SetChildFont,0
    ret
endp

proc CreateControls
    stdcall CreateFonts
    invoke CreateSolidBrush,0FFFFFFh
    mov [inputBrush],eax
    stdcall NewControl,comboClass,CBS_DROPDOWN+CBS_AUTOHSCROLL+WS_VSCROLL,101,278,390,328,240
    mov [hVersion],eax
    invoke SendMessageA,eax,CB_LIMITTEXT,120,0
    stdcall EngineScanVersions,[hVersion]
    invoke SetWindowTextA,[hVersion],selectedVersion
    stdcall NewControl,editClass,ES_AUTOHSCROLL,102,288,480,416,26
    mov [hPlayer],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,16,0
    invoke SetWindowTextA,[hPlayer],playerName
    stdcall NewControl,editClass,ES_AUTOHSCROLL,103,288,428,744,26
    mov [hClient],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,120,0
    invoke SetWindowTextA,[hClient],clientId
    stdcall NewControl,editClass,ES_AUTOHSCROLL,104,288,212,744,26
    mov [hGameDir],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,1000,0
    invoke SetWindowTextW,[hGameDir],gameDir
    stdcall NewControl,editClass,ES_AUTOHSCROLL,105,288,311,744,26
    mov [hJava],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,1000,0
    invoke SetWindowTextW,[hJava],javaPath
    stdcall NewControl,editClass,ES_NUMBER+ES_AUTOHSCROLL,106,288,410,196,26
    mov [hMemory],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,10,0
    cinvoke wsprintfW,ramText,fmtNum,[memoryMb]
    invoke SetWindowTextW,[hMemory],ramText
    stdcall NewControl,editClass,ES_MULTILINE+ES_AUTOVSCROLL+WS_VSCROLL,107,278,498,768,68
    mov [hJvmArgs],eax
    invoke SendMessageW,eax,EM_SETLIMITTEXT,4000,0
    invoke SetWindowTextW,[hJvmArgs],jvmArgs
    stdcall CreateModControls
    stdcall CreateAccountControls
    stdcall ServersUiCreate
    stdcall CreateLogControls
    stdcall CreateVersionControls
    stdcall CreateVersionPageControls
    stdcall CreatePackControls
    stdcall CreateDetailControls
    stdcall CreatePersonalizationControls
    stdcall ThemeApply
    stdcall ModSyncSelectedProfile
    ; springs start at rest on the current state
    mov eax,[currentPage]
    shl eax,8
    stdcall AnimSet,0,eax,1
    mov eax,[onlineMode]
    shl eax,8
    stdcall AnimSet,1,eax,1
    mov eax,[onlineMode]
    shl eax,8
    stdcall AnimSet,6,eax,1
    mov eax,[uiLang]
    shl eax,8
    stdcall AnimSet,5,eax,1
    mov eax,[uiLang]
    shl eax,8
    stdcall AnimSet,7,eax,1
    mov eax,[sideCollapsed]
    shl eax,8
    stdcall AnimSet,SIDE_SLOT,eax,1
    stdcall SidebarSync
    stdcall ShiftControls
    stdcall SyncPage
    ret
endp

; Hides every page control, then shows the current page's controls unless a
; transition is still running (they appear when it settles).
proc SyncPage
    invoke ShowWindow,[hVersion],SW_HIDE
    invoke ShowWindow,[hPlayer],SW_HIDE
    invoke ShowWindow,[hClient],SW_HIDE
    invoke ShowWindow,[hGameDir],SW_HIDE
    invoke ShowWindow,[hJava],SW_HIDE
    invoke ShowWindow,[hMemory],SW_HIDE
    invoke ShowWindow,[hJvmArgs],SW_HIDE
    stdcall HideModControls
    stdcall HideVersionControls
    stdcall HideVersionPageControls
    stdcall HidePackControls
    stdcall HideDetailControls
    stdcall HidePersonalizationControls
    stdcall HideLogControls
    stdcall HideAccountControls
    stdcall ServersUiHide
    cmp [transActive],0
    jne .done
    cmp [splashActive],0
    jne .done
    cmp [sideAnim],0
    jne .done
    cmp [currentPage],4
    je .mods
    cmp [currentPage],5
    je .packs
    cmp [currentPage],6
    je .personal
    cmp [currentPage],7
    je .logs
    cmp [currentPage],8
    je .servers
    cmp [currentPage],0
    je .home
    cmp [currentPage],1
    je .versions
    cmp [currentPage],2
    je .account
    invoke ShowWindow,[hGameDir],SW_SHOW
    invoke ShowWindow,[hJava],SW_SHOW
    invoke ShowWindow,[hMemory],SW_SHOW
    invoke ShowWindow,[hJvmArgs],SW_SHOW
    jmp .done
.home:
    stdcall RefreshHomeVersionSettings
    invoke ShowWindow,[hVersion],SW_SHOW
    cmp [onlineMode],0
    jne .done
    invoke ShowWindow,[hPlayer],SW_SHOW
    jmp .done
.versions:
    cmp [vmActive],0
    jne .versionPage
    stdcall ShowVersionControls
    jmp .done
.versionPage:
    stdcall ShowVersionPageControls
    jmp .done
.account:
    stdcall ShowAccountControls
    jmp .done
.mods:
    cmp [detailActive],0
    jne .detail
    stdcall ShowModControls
    stdcall ModBrowseDefault
    jmp .done
.packs:
    stdcall ShowPackControls
    stdcall PackBrowseDefault
    jmp .done
.personal:
    stdcall ShowPersonalizationControls
    jmp .done
.logs:
    stdcall ShowLogControls
    jmp .done
.servers:
    stdcall ServersUiShow
    jmp .done
.detail:
    stdcall ShowDetailControls
.done:
    invoke InvalidateRect,[hWnd],0,0
    ret
endp

; Animated navigation between pages (or tabs when page is unchanged).
proc GoPage page
    ; Workers temporarily own selectedVersion/gameDir and engine scratch data.
    cmp [busy],0
    jne .navigate
    stdcall PullSettings
.navigate:
    cmp [page],8
    jne @f
    stdcall ServersUiInvalidate
@@:
    mov [detailActive],0
    cmp [page],1
    je .keepVersionPage
    mov [vmActive],0
.keepVersionPage:
    stdcall StartTransition
    mov eax,[page]
    mov [currentPage],eax
    stdcall SyncPage
    ret
endp

; Convert a compact sidebar slot (0..NAV_COUNT-1) to its backing page id.
proc NavPageFromIndex index
    mov eax,[index]
    mov eax,[navPageIds+eax*4]
    ret
endp

proc NavIndexFromPage page
    xor ecx,ecx
.loop:
    cmp ecx,NAV_COUNT
    jae .settings
    mov eax,[navPageIds+ecx*4]
    cmp eax,[page]
    je .found
    inc ecx
    jmp .loop
.found:
    mov eax,ecx
    ret
.settings:
    mov eax,3
    ret
endp

proc PullSettings uses ebx
    invoke GetWindowTextA,[hVersion],selectedVersion,128
    invoke GetWindowTextA,[hPlayer],playerName,64
    invoke GetWindowTextA,[hClient],clientId,128
    invoke GetWindowTextW,[hGameDir],textBuffer,1024
    test eax,eax
    jz .badPath
    invoke GetFullPathNameW,textBuffer,1024,pathBuffer,0
    test eax,eax
    jz .badPath
    cmp eax,1024
    jae .badPath
    invoke lstrcpyW,gameDir,pathBuffer
    invoke GetWindowTextW,[hJava],javaPath,1024
    invoke GetWindowTextW,[hJvmArgs],jvmArgs,2048
    invoke GetWindowTextW,[hMemory],textBuffer,64
    stdcall MemoryDetect
    stdcall MemoryParseW,textBuffer
    mov ebx,eax
    stdcall MemoryValid,eax
    test eax,eax
    jz .badMemory
    mov eax,ebx
    mov [memoryMb],eax
    mov eax,1
    ret
.badMemory:
    cinvoke wsprintfW,statusText,sBadMemory,[memoryLimitMb]
    xor eax,eax
    ret
.badPath:
    invoke lstrcpyW,statusText,sBadPath
    xor eax,eax
    ret
endp

; The page header stays on the current canvas while the body cross-fades.
; This prevents the eyebrow, title and sheet marker from appearing to jump.
proc PaintFixedHeader
    mov eax,[contentX]
    mov [gOffX],eax
    mov [gOffY],0
    stdcall GfxBegin,[bbDC]
    cmp [currentPage],0
    je .home
    cmp [currentPage],1
    je .versions
    cmp [currentPage],2
    je .account
    cmp [currentPage],4
    je .mods
    cmp [currentPage],5
    je .packs
    cmp [currentPage],6
    je .personal
    cmp [currentPage],7
    je .logs
    cmp [currentPage],8
    je .servers
    stdcall PageHeader,eyebrowSettings,sSettingsHeading,4
    jmp .done
.home:
    stdcall PageHeader,eyebrowHome,sHeading,1
    jmp .done
.versions:
    cmp [vmActive],0
    jne .versionPage
    stdcall PageHeader,eyebrowVersions,sVersions,2
    jmp .done
.versionPage:
    stdcall PageHeader,vmEyebrow,vmTitleW,2
    jmp .done
.account:
    stdcall PageHeader,eyebrowAccount,sAccountHeading,3
    jmp .done
.mods:
    stdcall PageHeader,eyebrowMods,sModHeading,5
    jmp .done
.packs:
    stdcall PageHeader,eyebrowPacks,sPacks,6
    jmp .done
.personal:
    stdcall PageHeader,eyebrowPersonal,sPersonalHeading,7
    jmp .done
.logs:
    stdcall PageHeader,logEyebrow,sLogsHeading,8
    jmp .done
.servers:
    stdcall PageHeader,serversUiEyebrow,sServersHeading,9
.done:
    stdcall GfxEnd
    mov [gOffX],0
    ret
endp

; Cross-fade the captured header over the destination header. The body uses
; the same transition curve, but this region is composited separately so the
; fixed header changes without a hard cut.
proc PaintHeaderFade uses ebx esi
    cmp [transActive],0
    je .done
    mov eax,[transEase]
    shl eax,1
    cmp eax,1024
    jb @f
    mov eax,1024
@@:
    neg eax
    add eax,1024
    imul eax,255
    shr eax,10
    test eax,eax
    jz .done
    shl eax,16
    mov [headerFadeAlpha],eax
    stdcall SidebarWidth
    stdcall Scale,eax
    mov ebx,eax
    mov ecx,[bufW]
    sub ecx,ebx
    stdcall Scale,124
    mov esi,eax
    invoke AlphaBlend,[bbDC],ebx,0,ecx,esi,[snapDC],ebx,0,ecx,esi,[headerFadeAlpha]
.done:
    ret
endp

proc GetIniPath uses edi
    invoke GetModuleFileNameW,0,iniPath,1024
    lea edi,[iniPath+eax*2]
.back:
    cmp edi,iniPath
    jbe .append
    sub edi,2
    cmp word [edi],'\'
    jne .back
.append:
    mov word [edi],0
    invoke lstrcatW,iniPath,iniSuffix
    ret
endp

proc LoadSettings
    stdcall GetIniPath
    invoke GetPrivateProfileStringW,sectionName,kGame,gameDir,gameDir,1024,iniPath
    invoke GetPrivateProfileStringW,sectionName,kJava,javaPath,javaPath,1024,iniPath
    invoke GetPrivateProfileStringW,sectionName,kJvmArgs,emptyW,jvmArgs,2048,iniPath
    invoke GetPrivateProfileStringW,sectionName,kName,emptyW,textBuffer,256,iniPath
    cmp word [textBuffer],0
    jne .name
    invoke lstrcpyA,playerName,defaultPlayer
    jmp .version
.name:
    invoke WideCharToMultiByte,65001,0,textBuffer,-1,playerName,64,0,0
.version:
    invoke GetPrivateProfileStringW,sectionName,kVersion,emptyW,textBuffer,256,iniPath
    cmp word [textBuffer],0
    jne .hasVersion
    invoke lstrcpyA,selectedVersion,defaultVersion
    jmp .client
.hasVersion:
    invoke WideCharToMultiByte,65001,0,textBuffer,-1,selectedVersion,128,0,0
.client:
    invoke GetPrivateProfileStringW,sectionName,kClient,emptyW,textBuffer,256,iniPath
    invoke WideCharToMultiByte,65001,0,textBuffer,-1,clientId,128,0,0
    cmp byte [clientId],0
    jne .clientReady
    invoke lstrcpyA,clientId,defaultClientId
.clientReady:
    invoke GetPrivateProfileIntW,sectionName,kMemory,4096,iniPath
    cmp eax,512
    jb .mode
    cmp eax,[memoryLimitMb]
    jbe .memorySaved
    mov eax,[memoryLimitMb]
.memorySaved:
    mov [memoryMb],eax
.mode:
    invoke GetPrivateProfileIntW,sectionName,kMode,0,iniPath
    and eax,1
    mov [onlineMode],eax
    ret
endp

proc SaveSettings uses ebx
    stdcall PullSettings
    test eax,eax
    jz .failed
    ; Leading BOM ensures Unicode settings also work on non-Chinese Windows.
    invoke GetFileAttributesW,iniPath
    cmp eax,-1
    jne .write
    invoke CreateFileW,iniPath,GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .write
    mov ebx,eax
    mov word [textBuffer],0FEFFh
    invoke WriteFile,ebx,textBuffer,2,ramText,0
    invoke CloseHandle,ebx
.write:
    invoke WritePrivateProfileStringW,sectionName,kGame,gameDir,iniPath
    test eax,eax
    jz .writeFailed
    invoke WritePrivateProfileStringW,sectionName,kJava,javaPath,iniPath
    test eax,eax
    jz .writeFailed
    invoke WritePrivateProfileStringW,sectionName,kJvmArgs,jvmArgs,iniPath
    test eax,eax
    jz .writeFailed
    invoke MultiByteToWideChar,65001,0,playerName,-1,textBuffer,2048
    invoke WritePrivateProfileStringW,sectionName,kName,textBuffer,iniPath
    test eax,eax
    jz .writeFailed
    invoke MultiByteToWideChar,65001,0,selectedVersion,-1,textBuffer,2048
    invoke WritePrivateProfileStringW,sectionName,kVersion,textBuffer,iniPath
    test eax,eax
    jz .writeFailed
    invoke MultiByteToWideChar,65001,0,clientId,-1,textBuffer,2048
    invoke WritePrivateProfileStringW,sectionName,kClient,textBuffer,iniPath
    test eax,eax
    jz .writeFailed
    cinvoke wsprintfW,ramText,fmtNum,[memoryMb]
    invoke WritePrivateProfileStringW,sectionName,kMemory,ramText,iniPath
    test eax,eax
    jz .writeFailed
    cinvoke wsprintfW,ramText,fmtNum,[onlineMode]
    invoke WritePrivateProfileStringW,sectionName,kMode,ramText,iniPath
    test eax,eax
    jz .writeFailed
    stdcall LangSave
    mov eax,1
    ret
.writeFailed:
    invoke lstrcpyW,statusText,sSaveFailed
.failed:
    invoke InvalidateRect,[hWnd],0,0
    xor eax,eax
    ret
endp

; Keyboard access works even while an edit field has focus.
proc KeyInput key
    cmp [splashActive],0
    je .noSplash
    stdcall SplashFinish
    mov eax,1
    ret
.noSplash:
    cmp [key],VK_ESCAPE
    jne .idle
    mov [authCancel],1
    mov [engineCancel],1
    mov [modCancel],1
    mov eax,1
    ret
.idle:
    cmp [busy],0
    je .idleActions
    ; Page/rail shortcuts remain available; task-changing shortcuts stay locked.
    invoke GetKeyState,VK_CONTROL
    test ax,8000h
    jz .unhandled
    cmp [key],'B'
    je .ctrlKeys
    mov eax,[key]
    sub eax,'1'
    cmp eax,NAV_COUNT-1
    ja .unhandled
    jmp .ctrlKeys
.idleActions:
    cmp [key],VK_F5
    je .launch
    cmp [key],VK_F6
    je .install
    cmp [key],VK_F7
    je .modsSearch
    cmp [key],VK_RETURN
    jne .ctrlKeys
    cmp [detailActive],0
    jne .unhandled
    cmp [currentPage],5
    je .packReturn
    cmp [currentPage],4
    jne .unhandled
    cmp [modTab],0
    jne .unhandled
    invoke GetFocus
    cmp eax,[hModQuery]
    jne .unhandled
.modsSearch:
    cmp [detailActive],0
    jne .unhandled
    cmp [currentPage],5
    je .packSearch
    cmp [currentPage],4
    jne .unhandled
    cmp [modTab],0
    jne .unhandled
    mov [modOffset],0
    stdcall BeginModSearch
    mov eax,1
    ret
.packReturn:
    invoke GetFocus
    cmp eax,[hPackQuery]
    je .packSearch
    cmp eax,[hPackGame]
    jne .unhandled
.packSearch:
    cmp [packTab],0
    jne .unhandled
    mov [packOffset],0
    stdcall PackBeginSearch
    mov eax,1
    ret
.ctrlKeys:
    invoke GetKeyState,VK_CONTROL
    test ax,8000h
    jz .unhandled
    cmp [key],'S'
    je .save
    cmp [key],'B'
    jne @f
    stdcall SidebarToggle
    mov eax,1
    ret
@@:
    mov eax,[key]
    sub eax,'1'
    cmp eax,NAV_COUNT-1
    ja .unhandled
    stdcall NavPageFromIndex,eax
    cmp eax,[currentPage]
    je .samePage
    stdcall GoPage,eax
.samePage:
    mov eax,1
    ret
.launch:
    cmp [currentPage],1
    jne .launchCurrent
    cmp [vmActive],0
    je .launchCurrent
    stdcall VersionPageLaunch
    mov eax,1
    ret
.launchCurrent:
    stdcall StartWorker,EngineLaunch
    mov eax,1
    ret
.install:
    cmp [currentPage],1
    jne .installCurrent
    cmp [vmActive],0
    je .installCatalogue
    stdcall VersionPageRepair
    jmp .installedKey
.installCatalogue:
    cmp [versionTab],0
    je .addSelectedKey
    cmp [versionTab],2
    je .installQueue
    jmp .installCurrent
.addSelectedKey:
    stdcall VersionAddSelected
    test eax,eax
    jz .installedKey
.installQueue:
    stdcall VersionStartQueue
    jmp .installedKey
.installCurrent:
    stdcall StartWorker,EngineInstall
.installedKey:
    mov eax,1
    ret
.save:
    cmp [currentPage],1
    jne .saveGlobal
    cmp [vmActive],0
    je .saveGlobal
    cmp [vmTab],1
    jne .saveGlobal
    stdcall VersionPageSave
    jmp .savedPaint
.saveGlobal:
    stdcall SaveSettings
    test eax,eax
    jz .savedPaint
    invoke lstrcpyW,statusText,sSaved
    stdcall StatusChanged
.savedPaint:
    invoke InvalidateRect,[hWnd],0,0
    mov eax,1
    ret
.unhandled:
    xor eax,eax
    ret
endp

; Callback used from workers. A small spin lock prevents torn status messages.
proc SetStatusA uses ebx esi edi,pUtf8
    stdcall LangTrA,[pUtf8]
    mov ebx,eax
.lock:
    mov eax,1
    xchg eax,[statusLock]
    test eax,eax
    jz .locked
    invoke Sleep,0
    jmp .lock
.locked:
    invoke MultiByteToWideChar,65001,0,ebx,-1,statusIncoming,1024
    test eax,eax
    jnz .copied
    mov word [statusIncoming],0
.copied:
    invoke PostMessageW,[hWnd],WM_STATUS,0,0
    mov [statusLock],0
    ret
endp

proc StatusChanged
    cmp [themeMotion],0
    jne .animated
    mov [statusStart],0
    ret
.animated:
    stdcall NowMs
    or eax,1
    mov [statusStart],eax
    stdcall AnimKick
    ret
endp

proc StartWorker uses ebx,callback
    cmp [busy],0
    jne .done
    cmp [callback],ModWorker
    je .versionReset
    mov [modAction],0
.versionReset:
    cmp [callback],VersionWorker
    je .packReset
    mov [versionUiAction],0
.packReset:
    cmp [callback],PackWorker
    je .vmModsReset
    mov [packUiAction],0
.vmModsReset:
    cmp [callback],VmModsWorker
    je .settings
    mov [vmModsAction],0
.settings:
    cmp [callback],ServersUiWorker
    je .workerReady
    cmp [callback],VmLoaderWorker
    je .workerReady
    cmp [callback],AccountRestoreWorker
    je .workerReady
    cmp [callback],AccountSkinWorker
    je .workerReady
    cmp [callback],AccountLoginWorker
    je .accountClient
    cmp [callback],PackWorker
    jne .saveWorkerSettings
    cmp [packUiAction],5
    je .workerReady
    cmp [packUiAction],6
    je .workerReady
    cmp [packUiAction],7
    je .workerReady
    jmp .saveWorkerSettings
.accountClient:
    invoke GetWindowTextA,[hClient],clientId,128
    cmp byte [clientId],0
    jne .accountClientReady
    invoke lstrcpyA,clientId,defaultClientId
    invoke SetWindowTextA,[hClient],clientId
.accountClientReady:
    stdcall AuthValidateClient
    test eax,eax
    jz .accountClientBad
    invoke MultiByteToWideChar,65001,0,clientId,-1,textBuffer,2048
    invoke WritePrivateProfileStringW,sectionName,kClient,textBuffer,iniPath
    jmp .workerReady
.accountClientBad:
    stdcall SetStatusA,authMsgInvalidId
    jmp .reject
.saveWorkerSettings:
    stdcall SaveSettings
    test eax,eax
    jz .reject
.workerReady:
    mov eax,[callback]
    mov [workerCallback],eax
    mov dword [cDownloadBytes],0
    mov dword [cDownloadBytes+4],0
    mov [cDownloadFiles],0
    mov [busy],1
    mov [engineCancel],0
    mov [authCancel],0
    mov [modCancel],0
    invoke EnableWindow,[hVersion],0
    invoke EnableWindow,[hPlayer],0
    invoke EnableWindow,[hClient],0
    invoke EnableWindow,[hGameDir],0
    invoke EnableWindow,[hJava],0
    invoke EnableWindow,[hMemory],0
    invoke EnableWindow,[hJvmArgs],0
    stdcall EnableModControls,0
    stdcall EnableVersionControls,0
    stdcall EnableVersionPageControls,0
    stdcall EnablePackControls,0
    stdcall EnablePersonalizationControls,0
    stdcall EnableAccountControls,0
    stdcall ServersUiEnable,0
    ; Browsing controls do not mutate engine inputs or start a second worker.
    ; Keep the active catalogue worker's own result controls locked.
    cmp [callback],ModWorker
    je .browsePacks
    stdcall EnableModControls,1
.browsePacks:
    cmp [callback],PackWorker
    je .browseCommon
    stdcall EnablePackControls,1
.browseCommon:
    invoke EnableWindow,[hVInstalled],1
    invoke EnableWindow,[hVQueue],1
    stdcall EnablePersonalizationControls,1
    stdcall ServersUiEnable,1
    invoke lstrcpyW,statusText,sWorking
    stdcall StatusChanged
    invoke CreateThread,0,0,[callback],0,0,0
    mov [workerHandle],eax
    test eax,eax
    jnz .paint
    invoke PostMessageW,[hWnd],WM_FINISHED,0,0
.paint:
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
.reject:
    mov [modAction],0
    mov [versionUiAction],0
    mov [packUiAction],0
    mov [vmModsAction],0
    ret
endp

; ---------------------------------------------------------------- drawing kit
; Account workers finish only after the profile/skin read has completed.
proc AccountLoginWorker,parameter
    stdcall SkinClear
    stdcall AuthLoginQuiet,[parameter]
    cmp [authReady],1
    jne .finish
    stdcall AccountRefreshSkin
.finish:
    invoke PostMessageW,[hWnd],WM_FINISHED,0,0
    mov eax,[authReady]
    ret
endp
proc AccountRestoreWorker,parameter
    stdcall AuthEnsureSession
    test eax,eax
    jz .finish
    stdcall AccountRefreshSkin
.finish:
    invoke PostMessageW,[hWnd],WM_FINISHED,0,0
    mov eax,[authReady]
    ret
endp
proc AccountRefreshSkin
    stdcall SkinRefresh
    cmp [authReady],1
    jne .done
    cmp [authCacheOkay],0
    jne .done
    stdcall SetStatusA,authMsgCacheSave
.done:
    ret
endp

proc CR2ARGB color
    mov eax,[color]
    bswap eax
    shr eax,8
    or eax,0FF000000h
    ret
endp

; Compatibility shims for page modules: rounded box / line with COLORREF.
proc Box dc,x,y,w,h,r,color
    stdcall CR2ARGB,[color]
    mov ecx,[r]
    shr ecx,1
    stdcall FillRR,[x],[y],[w],[h],ecx,eax
    ret
endp

proc Line dc,x1,y1,x2,y2,color,width
    stdcall CR2ARGB,[color]
    mov ecx,[width]
    imul ecx,10
    stdcall LineA,[x1],[y1],[x2],[y2],eax,ecx
    ret
endp

; Core text routine: GDI ClearType text on the active canvas.
proc TextDraw uses ebx esi,str,x,y,w,h,font,color,flags,extra
    local rect RECT
    local hdc:DWORD
    mov eax,[x]
    add eax,[gOffX]
    stdcall Scale,eax
    mov [rect.left],eax
    mov eax,[x]
    add eax,[gOffX]
    add eax,[w]
    stdcall Scale,eax
    mov [rect.right],eax
    mov eax,[y]
    add eax,[gOffY]
    stdcall Scale,eax
    mov [rect.top],eax
    mov eax,[y]
    add eax,[gOffY]
    add eax,[h]
    stdcall Scale,eax
    mov [rect.bottom],eax
    mov eax,[gfxDC]
    mov [hdc],eax
    cmp [gfx],0
    je .draw
    invoke GdipGetDC,[gfx],addr hdc
.draw:
    invoke SelectObject,[hdc],[font]
    mov esi,eax
    invoke SetTextColor,[hdc],[color]
    invoke SetBkMode,[hdc],TRANSPARENT
    stdcall Scale,[extra]
    invoke SetTextCharacterExtra,[hdc],eax
    mov eax,[flags]
    or eax,DT_VCENTER+DT_SINGLELINE+DT_END_ELLIPSIS+DT_NOPREFIX
    invoke DrawTextW,[hdc],[str],-1,addr rect,eax
    invoke SetTextCharacterExtra,[hdc],0
    invoke SelectObject,[hdc],esi
    cmp [gfx],0
    je .done
    invoke GdipReleaseDC,[gfx],[hdc]
.done:
    ret
endp

proc Text dc,str,x,y,w,h,font,color
    stdcall TextDraw,[str],[x],[y],[w],[h],[font],[color],DT_LEFT,0
    ret
endp

proc TextC dc,str,x,y,w,h,font,color
    stdcall TextDraw,[str],[x],[y],[w],[h],[font],[color],DT_CENTER,0
    ret
endp

proc TextR dc,str,x,y,w,h,font,color
    stdcall TextDraw,[str],[x],[y],[w],[h],[font],[color],DT_RIGHT,0
    ret
endp

proc Mono str,x,y,w,h,color
    stdcall TextDraw,[str],[x],[y],[w],[h],[fMono],[color],DT_LEFT,1
    ret
endp

; Pill / rounded button with hover glow and press feedback.
proc Button uses ebx esi edi,dc,str,x,y,w,h,primary
    locals
        base dd ?
        hi dd ?
        lo dd ?
        ink dd ?
        rad dd ?
        idx dd ?
    endl
    stdcall HotAdd,[x],[y],[w],[h]
    mov [idx],eax
    mov [base],A_SECOND
    mov [hi],A_SECOND_HI
    mov [lo],A_SECOND_LO
    mov [ink],T_INK
    cmp [primary],0
    je .colors
    mov eax,[themeAccentARGB]
    mov [base],eax
    mov eax,[themeAccentHi]
    mov [hi],eax
    mov eax,[themeAccentLo]
    mov [lo],eax
    mov [ink],T_WHITE
.colors:
    mov eax,[h]
    shr eax,1
    cmp [h],44
    jbe @f
    mov eax,14
@@:
    mov [rad],eax
    stdcall HoverLevel,[idx]
    mov ebx,eax
    stdcall Mix,[base],[hi],ebx
    mov esi,eax
    xor edi,edi
    mov eax,[idx]
    cmp eax,-1
    je .draw
    cmp eax,[pressIdx]
    jne .draw
    mov esi,[lo]
    mov edi,1
.draw:
    cmp [primary],0
    je .face
    ; soft accent glow grows with hover
    mov eax,ebx
    shr eax,3
    add eax,10h
    stdcall WithAlpha,[themeAccentARGB],eax
    mov ecx,[y]
    add ecx,3
    stdcall FillRR,[x],ecx,[w],[h],[rad],eax
.face:
    mov eax,[y]
    add eax,edi
    stdcall FillRR,[x],eax,[w],[h],[rad],esi
    mov eax,[y]
    add eax,edi
    stdcall TextC,0,[str],[x],eax,[w],[h],[fMedium],[ink]
    ret
endp

; Segmented control with a spring-animated thumb. labels -> dd array of strings.
proc Segmented uses ebx esi edi,x,y,w,h,count,slot,labels,selected,font
    locals
        segW dd ?
        tx dd ?
        idx dd ?
        i dd ?
    endl
    stdcall FillRR,[x],[y],[w],[h],10,A_TRACK
    mov eax,[w]
    sub eax,6
    xor edx,edx
    div dword [count]
    mov [segW],eax
    ; target follows state; position = selected * segW
    mov eax,[selected]
    shl eax,8
    stdcall AnimSet,[slot],eax,0
    stdcall AnimGet,[slot]
    imul eax,[segW]
    sar eax,8
    add eax,[x]
    add eax,3
    mov [tx],eax
    mov eax,[y]
    add eax,3
    mov ecx,[h]
    sub ecx,6
    stdcall Shadow,[tx],eax,[segW],ecx,8
    mov eax,[y]
    add eax,3
    mov ecx,[h]
    sub ecx,6
    stdcall FillRR,[tx],eax,[segW],ecx,8,A_CARD
    mov [i],0
.label:
    mov eax,[i]
    cmp eax,[count]
    jae .done
    imul eax,[segW]
    add eax,[x]
    add eax,3
    mov ebx,eax
    stdcall HotAdd,ebx,[y],[segW],[h]
    mov [idx],eax
    mov esi,T_SECOND
    mov eax,[i]
    cmp eax,[selected]
    jne @f
    mov esi,T_INK
@@:
    stdcall HoverLevel,[idx]
    test eax,eax
    jz @f
    stdcall Mix,esi,T_INK,eax
    mov esi,eax
@@:
    mov eax,[i]
    mov ecx,[labels]
    mov edi,[ecx+eax*4]
    stdcall TextC,0,edi,ebx,[y],[segW],[h],[font],esi
    inc [i]
    jmp .label
.done:
    ret
endp

; Page header: mono eyebrow, title, sheet number and a dimension rule.
proc PageHeader eyebrow,title,sheet
    cmp [suppressPageHeader],0
    jne .done
    stdcall Mono,[eyebrow],253,34,300,16,T_MUTED
    stdcall Text,0,[title],252,52,640,44,[fHeading],T_INK
    cinvoke wsprintfW,sheetText,sheetFmt,[sheet]
    stdcall TextDraw,sheetText,888,34,200,16,[fMono],T_MUTED,DT_RIGHT,1
    stdcall LineA,252,108,1088,108,A_HAIRLINE,10
    stdcall LineA,252,104,252,112,A_CROSS,10
    stdcall LineA,1088,104,1088,112,A_CROSS,10
.done:
    ret
endp

; Rotating arc spinner (busy) or a calm dot (idle) followed by status text.
proc StatusLine uses ebx esi,x,y,w,h
    locals
        cyv dd ?
        col dd ?
    endl
    mov eax,[h]
    shr eax,1
    add eax,[y]
    mov [cyv],eax
    cmp [busy],0
    je .idle
    stdcall NowMs
    xor edx,edx
    mov ecx,3
    div ecx
    xor edx,edx
    mov ecx,360
    div ecx
    mov ebx,edx
    mov eax,[x]
    add eax,7
    stdcall ArcA,eax,[cyv],6,0,360,A_TRACK,20,0
    mov eax,[x]
    add eax,7
    stdcall ArcA,eax,[cyv],6,ebx,110,[themeAccentARGB],20,0
    jmp .text
.idle:
    invoke lstrcmpiW,statusText,sReady
    mov ecx,A_GREEN
    test eax,eax
    jz @f
    mov ecx,A_MUTED
@@:
    mov eax,[x]
    add eax,7
    stdcall Dot,eax,[cyv],ecx
.text:
    ; fade new status text in from the canvas colour
    mov esi,T_SECOND
    cmp [statusStart],0
    je .draw
    stdcall NowMs
    sub eax,[statusStart]
    cmp eax,260
    jb .fading
    mov [statusStart],0
    jmp .draw
.fading:
    shl eax,8
    xor edx,edx
    mov ecx,260
    div ecx
    stdcall Mix,T_CANVAS,T_SECOND,eax
    mov esi,eax
.draw:
    mov eax,[x]
    add eax,22
    mov ecx,[w]
    sub ecx,22
    stdcall Text,0,statusText,eax,[y],ecx,[h],[fSmall],esi
    ret
endp

; Indeterminate progress hairline while a worker runs.
proc BusyBar uses ebx esi,x,y,w
    cmp [busy],0
    je .done
    stdcall DownloadSnapshot,downloadUiStats
    cmp [downloadUiStats],0
    je .bar
    mov eax,[downloadUiStats]
    xor edx,edx
    mov ecx,1024
    div ecx
    mov ebx,eax
    imul eax,edx,10
    xor edx,edx
    div ecx
    cinvoke wsprintfW,downloadUiText,sDownloadBytes,ebx,eax,[downloadUiStats+4]
    mov eax,[y]
    add eax,48
    cmp [y],628
    jne @f
    mov eax,[y]
    add eax,7
@@:
    stdcall Text,0,downloadUiText,[x],eax,[w],20,[fSmall],C_MUTED
.bar:
    stdcall FillRR,[x],[y],[w],3,1,A_TRACK
    stdcall NowMs
    xor edx,edx
    mov ecx,1400
    div ecx
    mov eax,edx
    mov ecx,[w]
    add ecx,160
    imul eax,ecx
    xor edx,edx
    mov ecx,1400
    div ecx
    sub eax,160
    add eax,[x]
    mov ebx,eax
    mov esi,ebx
    add esi,160
    cmp ebx,[x]
    jge @f
    mov ebx,[x]
@@:
    mov eax,[x]
    add eax,[w]
    cmp esi,eax
    jle @f
    mov esi,eax
@@:
    sub esi,ebx
    jle .done
    stdcall FillRR,ebx,[y],esi,3,1,[themeAccentARGB]
.done:
    ret
endp

; ---------------------------------------------------------------- nav icons
proc NavIcon uses ebx esi edi,kind,x,y,argb
    mov esi,[x]
    mov edi,[y]
    mov eax,[kind]
    cmp eax,1
    je .versions
    cmp eax,2
    je .account
    cmp eax,3
    je .settings
    cmp eax,4
    je .mods
    cmp eax,5
    je .packs
    cmp eax,6
    je .servers
    cmp eax,7
    je .logs
    cmp eax,8
    je .servers
    ; play: ring + triangle
    stdcall ArcA,esi,edi,8,0,360,[argb],14,0
    lea eax,[esi-2]
    lea ecx,[edi-4]
    lea edx,[esi+4]
    stdcall LineA,eax,ecx,edx,edi,[argb],14
    lea eax,[esi+4]
    lea ecx,[esi-2]
    lea edx,[edi+4]
    stdcall LineA,eax,edi,ecx,edx,[argb],14
    lea eax,[esi-2]
    lea ecx,[edi-4]
    lea edx,[edi+4]
    stdcall LineA,eax,ecx,eax,edx,[argb],14
    ret
.servers:
    lea eax,[esi-8]
    lea ecx,[edi-8]
    stdcall StrokeRR,eax,ecx,16,7,2,[argb],14
    lea eax,[esi-8]
    lea ecx,[edi+2]
    stdcall StrokeRR,eax,ecx,16,7,2,[argb],14
    lea eax,[esi-4]
    lea ecx,[edi-4]
    stdcall FillCircle,eax,ecx,1,[argb]
    lea eax,[esi-4]
    lea ecx,[edi+5]
    stdcall FillCircle,eax,ecx,1,[argb]
    ret
.logs:
    lea eax,[esi-6]
    lea ecx,[edi-9]
    stdcall StrokeRR,eax,ecx,13,18,2,[argb],14
    lea eax,[esi-3]
    lea ecx,[esi+4]
    lea edx,[edi-3]
    stdcall LineA,eax,edx,ecx,edx,[argb],12
    lea eax,[esi-3]
    lea ecx,[esi+4]
    lea edx,[edi+2]
    stdcall LineA,eax,edx,ecx,edx,[argb],12
    ret
.versions:
    ; stacked layers
    lea eax,[esi-8]
    lea ecx,[edi-3]
    lea edx,[edi-7]
    stdcall LineA,eax,ecx,esi,edx,[argb],14
    lea eax,[esi+8]
    lea ecx,[edi-3]
    lea edx,[edi-7]
    stdcall LineA,esi,edx,eax,ecx,[argb],14
    lea eax,[esi+8]
    lea ecx,[edi-3]
    lea edx,[edi+1]
    stdcall LineA,eax,ecx,esi,edx,[argb],14
    lea eax,[esi-8]
    lea ecx,[edi-3]
    lea edx,[edi+1]
    stdcall LineA,esi,edx,eax,ecx,[argb],14
    lea eax,[esi-8]
    lea ecx,[edi+2]
    lea edx,[edi+6]
    stdcall LineA,eax,ecx,esi,edx,[argb],14
    lea eax,[esi+8]
    lea ecx,[edi+2]
    lea edx,[edi+6]
    stdcall LineA,esi,edx,eax,ecx,[argb],14
    ret
.account:
    lea eax,[edi-3]
    stdcall ArcA,esi,eax,4,0,360,[argb],14,0
    lea eax,[edi+10]
    stdcall ArcA,esi,eax,8,205,130,[argb],14,0
    ret
.settings:
    lea eax,[esi-8]
    lea ecx,[esi+8]
    lea edx,[edi-4]
    stdcall LineA,eax,edx,ecx,edx,[argb],14
    lea eax,[esi-8]
    lea ecx,[esi+8]
    lea edx,[edi+4]
    stdcall LineA,eax,edx,ecx,edx,[argb],14
    lea eax,[esi-3]
    lea edx,[edi-4]
    stdcall FillCircle,eax,edx,3,[argb]
    lea eax,[esi+3]
    lea edx,[edi+4]
    stdcall FillCircle,eax,edx,3,[argb]
    ret
.mods:
    lea eax,[esi-8]
    lea ecx,[edi-8]
    stdcall StrokeRR,eax,ecx,7,7,2,[argb],14
    lea eax,[esi+1]
    lea ecx,[edi-8]
    stdcall StrokeRR,eax,ecx,7,7,2,[argb],14
    lea eax,[esi-8]
    lea ecx,[edi+1]
    stdcall StrokeRR,eax,ecx,7,7,2,[argb],14
    lea eax,[esi+1]
    lea ecx,[edi+1]
    stdcall StrokeRR,eax,ecx,7,7,2,[argb],14
    ret
.packs:
    lea eax,[esi-8]
    lea ecx,[edi-5]
    lea edx,[edi-9]
    stdcall LineA,eax,ecx,esi,edx,[argb],14
    lea eax,[esi+8]
    lea ecx,[edi-5]
    lea edx,[edi-9]
    stdcall LineA,esi,edx,eax,ecx,[argb],14
    lea eax,[esi-8]
    lea ecx,[edi+1]
    lea edx,[edi+5]
    stdcall LineA,eax,ecx,esi,edx,[argb],14
    lea eax,[esi+8]
    lea ecx,[edi+1]
    lea edx,[edi+5]
    stdcall LineA,esi,edx,eax,ecx,[argb],14
    ret
endp

; ---------------------------------------------------------------- sidebar
proc PaintSidebar uses ebx esi edi
    locals
        i dd ?
        iy dd ?
        idx dd ?
        sw dd ?
        fade dd ?
        pillW dd ?
        iconX dd ?
        tx dd ?
        ty dd ?
        selected dd ?
    endl
    stdcall SidebarWidth
    mov [sw],eax
    stdcall SidebarLabelFade
    mov [fade],eax
    stdcall FillRR,0,0,[sw],760,0,A_SIDEBAR
    mov eax,[sw]
    dec eax
    stdcall LineA,eax,0,eax,760,A_SIDELINE,10
    ; app icon: graphite squircle with the OFFSET mark (re-centres on the rail)
    stdcall Lerp,22,15,[sideT]
    mov ebx,eax
    stdcall Shadow,ebx,26,38,38,11
    stdcall FillRRGrad,ebx,26,38,38,11,A_ICONBG1,A_ICONBG2
    lea eax,[ebx+4]
    stdcall DrawLogo,eax,30,30,A_MARK,0,360
    ; labels fade out as the sidebar narrows (and are clipped to it)
    cmp [fade],256
    jae .toggle
    stdcall Mix,T_INK,A_SIDEBAR_CR,[fade]
    mov esi,eax
    stdcall ThemeName
    stdcall TextDraw,eax,72,26,106,22,[fBrand],esi,DT_LEFT,2
    stdcall Mix,T_MUTED,A_SIDEBAR_CR,[fade]
    mov esi,eax
    stdcall TextDraw,monoTagline,73,47,140,14,[fMonoSmall],esi,DT_LEFT,1
    stdcall TextDraw,monoWorkspace,25,98,180,14,[fMonoSmall],esi,DT_LEFT,1
.toggle:
    ; collapse / expand button
    stdcall Lerp,184,22,[sideT]
    mov [tx],eax
    stdcall Lerp,92,78,[sideT]
    mov [ty],eax
    stdcall HotAdd,[tx],[ty],24,24
    mov [toggleHot],eax
    stdcall HoverLevel,eax
    test eax,eax
    jz @f
    shr eax,4
    stdcall WithAlpha,0,eax
    stdcall FillRR,[tx],[ty],24,24,7,eax
@@:
    stdcall SideToggleGlyph,[tx],[ty],A_MUTED
    ; nav geometry: pill 196 -> 44 wide, icon follows the pill centre
    stdcall Lerp,196,44,[sideT]
    mov [pillW],eax
    stdcall Lerp,31,34,[sideT]
    mov [iconX],eax
    ; selection pill rides a spring between items
    stdcall NavIndexFromPage,[currentPage]
    mov [selected],eax
    shl eax,8
    stdcall AnimSet,0,eax,0
    stdcall AnimGet,0
    imul eax,NAV_STEP
    sar eax,8
    add eax,NAV_TOP
    mov ebx,eax
    stdcall Shadow,12,ebx,[pillW],NAV_H,10
    stdcall FillRR,12,ebx,[pillW],NAV_H,10,A_CARD
    stdcall StrokeRR,12,ebx,[pillW],NAV_H,10,A_HAIRLINE,10
    mov [i],0
.nav:
    mov eax,[i]
    imul eax,NAV_STEP
    add eax,NAV_TOP
    mov [iy],eax
    stdcall HotAdd,12,eax,[pillW],NAV_H
    mov [idx],eax
    cmp [i],0
    jne @f
    mov [navHotBase],eax
@@:
    mov eax,[i]
    cmp eax,[selected]
    je .selected
    stdcall HoverLevel,[idx]
    test eax,eax
    jz .plain
    shr eax,4
    stdcall WithAlpha,0,eax
    stdcall FillRR,12,[iy],[pillW],NAV_H,10,eax
.plain:
    mov esi,A_MUTED
    mov edi,T_SECOND
    mov ebx,[fBody]
    jmp .item
.selected:
    mov esi,[themeAccentARGB]
    mov edi,T_INK
    mov ebx,[fBodyStrong]
.item:
    mov eax,[iy]
    add eax,NAV_H/2
    stdcall NavIcon,[i],[iconX],eax,esi
    cmp [fade],256
    jae .nextItem
    stdcall Mix,edi,A_SIDEBAR_CR,[fade]
    mov edi,eax
    mov eax,[i]
    mov ecx,[navLabels+eax*4]
    stdcall Text,0,ecx,52,[iy],140,NAV_H,ebx,edi
.nextItem:
    inc [i]
    cmp [i],NAV_COUNT
    jb .nav
    ; footer: game folder, language, build tag
    mov eax,[sw]
    sub eax,20
    stdcall LineA,20,600,eax,600,A_SIDELINE,10
    stdcall HotAdd,12,608,[pillW],34
    mov [folderHot],eax
    stdcall HoverLevel,eax
    test eax,eax
    jz @f
    shr eax,4
    stdcall WithAlpha,0,eax
    stdcall FillRR,12,608,[pillW],34,10,eax
@@:
    stdcall Lerp,24,27,[sideT]
    mov ebx,eax
    stdcall StrokeRR,ebx,619,14,11,2,A_MUTED,14
    lea ecx,[ebx+13]
    stdcall LineA,ebx,622,ecx,622,A_MUTED,14
    cmp [fade],256
    jae .rail
    stdcall Mix,T_SECOND,A_SIDEBAR_CR,[fade]
    stdcall Text,0,sFolder,48,608,156,34,[fSmall],eax
    mov eax,[uiLang]
    stdcall Segmented,16,654,188,30,3,5,langNames,eax,[fSmall]
    stdcall Mix,T_MUTED,A_SIDEBAR_CR,[fade]
    stdcall TextDraw,monoBuild,25,700,180,14,[fMonoSmall],eax,DT_LEFT,1
    mov [langHot],-1
    ret
.rail:
    ; collapsed: one pill that cycles the language
    stdcall HotAdd,12,654,44,30
    mov [langHot],eax
    stdcall HoverLevel,eax
    mov ebx,eax
    stdcall Mix,A_TRACK,A_SECOND_HI,ebx
    stdcall FillRR,12,654,44,30,10,eax
    mov eax,[uiLang]
    mov ecx,[langNames+eax*4]
    stdcall TextC,0,ecx,12,654,44,30,[fSmall],T_INK
    ret
endp

; ---------------------------------------------------------------- home
; Blueprint hero: grid, construction circle and the mark drawing itself.
proc PaintHero uses ebx esi edi
    locals
        t dd ?
        sweep dd ?
        fillA dd ?
    endl
    ; hero progress 0..1024 (eased)
    mov eax,1024
    cmp [heroActive],0
    je .static
    stdcall NowMs
    sub eax,[heroStart]
    shl eax,10
    xor edx,edx
    mov ecx,HERO_MS
    div ecx
    cmp eax,1024
    jb .running
    mov [heroActive],0
    mov eax,1024
    jmp .static
.running:
    stdcall EaseOut,eax
.static:
    mov [t],eax
    stdcall Shadow,252,124,836,172,22
    stdcall FillRRGrad,252,124,836,172,22,[themeBlueprint1],[themeBlueprint2]
    stdcall ClipRR,252,124,836,172,22
    ; grid: 24-unit minor lines, every 4th brighter (cached paths)
    stdcall DrawBlueprintGrid
    ; construction axes and circle
    stdcall DashLine,850,210,1070,210,40FFFFFFh,10
    stdcall DashLine,960,130,960,290,40FFFFFFh,10
    mov eax,[t]
    imul eax,360
    shr eax,10
    mov [sweep],eax
    stdcall ArcA,960,210,76,-90,eax,66FFFFFFh,10,1
    mov eax,[t]
    imul eax,360
    shr eax,10
    stdcall ArcA,960,210,58,90,eax,30FFFFFFh,10,0
    ; the mark: outline sweeps in, then a faint fill settles
    stdcall DrawLogo,905,155,110,0F2FFFFFFh,12,[sweep]
    mov eax,[t]
    sub eax,600
    jle .noFill
    imul eax,46h
    xor edx,edx
    mov ecx,424
    div ecx
    stdcall WithAlpha,0FFFFFFh,eax
    stdcall DrawLogo,905,155,110,eax,0,360
.noFill:
    ; tick marks on the circle at 0/90/180/270
    stdcall LineA,960,128,960,138,99FFFFFFh,10
    stdcall LineA,960,282,960,292,99FFFFFFh,10
    stdcall LineA,878,210,888,210,99FFFFFFh,10
    stdcall LineA,1032,210,1042,210,99FFFFFFh,10
    stdcall GfxUnclip
    stdcall HeroTextColor,0D9C2A6h
    mov ebx,eax
    stdcall TextDraw,monoFigure,1000,136,68,18,[fMonoSmall],ebx,DT_LEFT,0
    stdcall TextDraw,monoRadius,1000,258,68,18,[fMonoSmall],ebx,DT_LEFT,0
    ; left: current version
    stdcall HeroTextColor,0E8D6C2h
    stdcall Text,0,sHeroVersion,284,146,520,20,[fSmall],eax
    invoke GetWindowTextW,[hVersion],heroText,120
    test eax,eax
    jnz @f
    invoke MultiByteToWideChar,65001,0,selectedVersion,-1,heroText,120
@@:
    stdcall HeroTextColor,T_WHITE
    stdcall Text,0,heroText,282,164,560,50,[fHero],eax
    stdcall HeroTextColor,0EEDCCAh
    stdcall Text,0,sHeroTag,284,214,560,22,[fBody],eax
    stdcall HeroTextColor,0C59B7Ch
    stdcall TextDraw,monoDrawing,285,262,520,14,[fMonoSmall],eax,DT_LEFT,1
    ret
endp

proc PaintHome uses ebx esi edi
    stdcall PageHeader,eyebrowHome,sHeading,1
    stdcall PaintHero
    ; profile card
    stdcall Card,252,316,490,236,18
    stdcall CornerTicks,252,316,490,236,A_CROSS
    stdcall Text,0,sProfileCaption,278,330,300,24,[fMedium],T_INK
    stdcall Mono,monoProfile,700,334,20,16,T_MUTED
    stdcall Text,0,sVersionLabel,278,364,420,20,[fSmall],T_SECOND
    stdcall Button,0,sVmManage,620,384,94,38,0
    cmp [onlineMode],0
    jne .onlineName
    stdcall Text,0,sPlayerLabel,278,448,420,20,[fSmall],T_SECOND
    stdcall Field,278,472,436,38,[hPlayer]
    jmp .session
.onlineName:
    stdcall Text,0,sOnline,278,448,420,20,[fSmall],T_SECOND
    stdcall FillRR,278,472,436,38,10,A_CANVAS
    cmp [authReady],0
    je .notLogged
    stdcall Dot,294,491,A_GREEN
    invoke MultiByteToWideChar,65001,0,authName,-1,accountText,256
    stdcall Text,0,accountText,306,472,400,38,[fBody],T_INK
    jmp .session
.notLogged:
    stdcall Dot,294,491,A_ORANGE
    stdcall Text,0,sNoAccount,306,472,400,38,[fBody],T_SECOND
.session:
    stdcall Card,762,316,326,236,18
    stdcall CornerTicks,762,316,326,236,A_CROSS
    stdcall Text,0,sSessionCaption,788,330,240,24,[fMedium],T_INK
    stdcall Mono,monoSession,1054,334,20,16,T_MUTED
    mov [segLabels],sOffline
    mov [segLabels+4],sOnline
    stdcall Segmented,786,366,278,36,2,1,segLabels,[onlineMode],[fSmall]
    stdcall Text,0,sRamLabel,788,418,140,24,[fSmall],T_SECOND
    cinvoke wsprintfW,ramText,fmtRam,[homeVersionMemory]
    stdcall TextDraw,ramText,900,418,164,24,[fMono],T_INK,DT_RIGHT,0
    stdcall LineA,788,454,1064,454,A_DIVIDER,10
    stdcall Text,0,sJavaLabel,788,464,140,22,[fSmall],T_SECOND
    mov eax,sJavaAuto
    mov ebx,A_GREEN
    cmp word [homeVersionJavaW],0
    jne .java
    mov eax,sJavaMissing
    mov ebx,A_ORANGE
.java:
    mov esi,eax
    stdcall Dot,793,500,ebx
    stdcall Text,0,esi,806,488,250,24,[fBody],T_INK
    ; launch row
    stdcall StatusLine,254,580,540,40
    stdcall BusyBar,254,628,540
    mov eax,sLaunch
    cmp [busy],0
    je .start
    mov eax,sWorking
.start:
    stdcall Button,0,eax,828,576,260,56,1
    ret
endp

proc PaintVersions dc
    stdcall PaintVersionManager,[dc]
    ret
endp



; ---------------------------------------------------------------- settings
proc PaintSettings
    stdcall PageHeader,eyebrowSettings,sSettingsHeading,4
    stdcall Card,252,124,836,468,18
    stdcall CornerTicks,252,124,836,468,A_CROSS
    stdcall Text,0,sGameDirLabel,280,166,760,28,[fMedium],T_INK
    stdcall Field,278,204,768,42,[hGameDir]
    stdcall Text,0,sJavaPathLabel,280,265,760,28,[fMedium],T_INK
    stdcall Field,278,303,768,42,[hJava]
    cinvoke wsprintfW,textBuffer,sMemoryLabel,[memoryLimitMb]
    stdcall Text,0,textBuffer,280,364,340,28,[fMedium],T_INK
    stdcall Field,278,402,216,42,[hMemory]
    stdcall Text,0,sLanguageLabel,642,364,300,28,[fMedium],T_INK
    mov eax,[uiLang]
    stdcall Segmented,640,402,300,42,3,7,langNames,eax,[fBody]
    stdcall LineA,278,470,1060,470,A_DIVIDER,10
    stdcall Text,0,sJvmArgsLabel,280,482,300,26,[fMedium],T_INK
    stdcall Field,278,514,768,64,[hJvmArgs]
    stdcall Button,0,sSave,252,600,190,42,1
    stdcall Button,0,sPersonalHeading,458,600,190,42,0
    stdcall Button,0,sLogsHeading,664,600,190,42,0
    stdcall StatusLine,254,676,830,32
    ret
endp

; ---------------------------------------------------------------- frame
; Custom minimize / close, drawn above everything (also over the splash).
proc PaintWindowButtons uses ebx esi edi
    mov edi,A_MUTED
    cmp [splashActive],0
    je @f
    mov edi,0D8FFFFFFh
@@:
    ; minimize
    stdcall HotAdd,WB_MIN_X,WB_Y,WB_W,WB_H
    mov [minHot],eax
    stdcall HoverLevel,eax
    mov ebx,eax
    test ebx,ebx
    jz .minGlyph
    mov eax,ebx
    shr eax,4
    cmp [splashActive],0
    je @f
    stdcall WithAlpha,0FFFFFFh,eax
    jmp .minFill
@@:
    stdcall WithAlpha,0,eax
.minFill:
    stdcall FillRR,WB_MIN_X,WB_Y,WB_W,WB_H,8,eax
.minGlyph:
    stdcall LineA,WB_MIN_X+12,WB_Y+14,WB_MIN_X+22,WB_Y+14,edi,13
    ; close: fills red on hover, glyph turns white
    stdcall HotAdd,WB_CLOSE_X,WB_Y,WB_W,WB_H
    mov [closeHot],eax
    stdcall HoverLevel,eax
    mov ebx,eax
    mov esi,edi
    test ebx,ebx
    jz .closeGlyph
    stdcall WithAlpha,A_CLOSE,ebx
    cmp ebx,255
    jbe @f
    mov eax,A_CLOSE
@@:
    stdcall FillRR,WB_CLOSE_X,WB_Y,WB_W,WB_H,8,eax
    stdcall Mix,edi,0FFFFFFFFh,ebx
    mov esi,eax
.closeGlyph:
    stdcall LineA,WB_CLOSE_X+12,WB_Y+9,WB_CLOSE_X+22,WB_Y+19,esi,13
    stdcall LineA,WB_CLOSE_X+22,WB_Y+9,WB_CLOSE_X+12,WB_Y+19,esi,13
    ret
endp

; 1 = minimize, 2 = close, 0 = neither (logical window coordinates).
proc WindowButtonHit x,y
    xor eax,eax
    mov ecx,[y]
    sub ecx,WB_Y
    cmp ecx,WB_H
    jae .done
    mov ecx,[x]
    sub ecx,WB_MIN_X
    cmp ecx,WB_W
    jae @f
    mov eax,1
    ret
@@:
    mov ecx,[x]
    sub ecx,WB_CLOSE_X
    cmp ecx,WB_W
    jae .done
    mov eax,2
.done:
    ret
endp

; Background, page content and sidebar into the back buffer.
proc PaintInterface uses ebx esi edi,w,h
    locals
        alpha dd ?
        cx0 dd ?
    endl
    invoke BitBlt,[bbDC],0,0,[w],[h],[bgDC],0,0,SRCCOPY
    ; restart the hero drawing whenever Home comes into view
    cmp [splashActive],0
    jne .samePage
    mov eax,[currentPage]
    cmp eax,[lastPage]
    je .samePage
    mov [lastPage],eax
    test eax,eax
    jnz .samePage
    cmp [themeMotion],0
    je .samePage
    stdcall NowMs
    mov [heroStart],eax
    mov [heroActive],1
    stdcall AnimKick
.samePage:
    ; content rises 14 units while it fades in
    mov eax,1024
    sub eax,[transEase]
    imul eax,14
    sar eax,10
    mov [gOffY],eax
    stdcall SidebarSync
    mov eax,[contentX]
    mov [gOffX],eax
    ; Draw the moving page body without its header. The header is composited
    ; once below, at a fixed offset, to avoid doubled anti-aliased glyphs.
    mov [suppressPageHeader],1
    stdcall GfxBegin,[bbDC]
    stdcall HotReset
    cmp [currentPage],0
    je .home
    cmp [currentPage],1
    je .versions
    cmp [currentPage],2
    je .account
    cmp [currentPage],4
    je .mods
    cmp [currentPage],5
    je .packs
    cmp [currentPage],6
    je .personal
    cmp [currentPage],7
    je .logs
    cmp [currentPage],8
    je .servers
    stdcall PaintSettings
    jmp .content
.home:
    stdcall PaintHome
    jmp .content
.versions:
    cmp [vmActive],0
    jne .versionPage
    stdcall PaintVersions,0
    jmp .content
.versionPage:
    stdcall PaintVersionPage,0
    jmp .content
.account:
    stdcall PaintAccount,0
    jmp .content
.mods:
    stdcall PaintMods,0
    jmp .content
.packs:
    stdcall PaintPacks,0
    jmp .content
.personal:
    stdcall PaintPersonalization,0
    jmp .content
.logs:
    stdcall PaintLogs,0
    jmp .content
.servers:
    stdcall ServersUiPaint,0
.content:
    stdcall GfxEnd
    mov [suppressPageHeader],0
    mov [gOffY],0
    mov [gOffX],0
    ; cross-fade the previous page out
    cmp [transActive],0
    je .fixedHeader
    ; old page clears in the first half so the new one reads cleanly
    mov eax,[transEase]
    shl eax,1
    cmp eax,1024
    jb @f
    mov eax,1024
@@:
    neg eax
    add eax,1024
    imul eax,255
    shr eax,10
    jz .fixedHeader
    shl eax,16
    mov [alpha],eax
    stdcall SidebarWidth
    stdcall Scale,eax
    mov [cx0],eax
    mov ecx,[w]
    sub ecx,eax
    invoke AlphaBlend,[bbDC],eax,0,ecx,[h],[snapDC],eax,0,ecx,[h],[alpha]
.fixedHeader:
    ; Keep the page heading on the destination canvas while the body fades.
    stdcall PaintFixedHeader
    stdcall PaintHeaderFade
.sidebar:
    ; sidebar is clipped to its current width so labels slide under the edge
    invoke SaveDC,[bbDC]
    mov ebx,eax
    stdcall SidebarWidth
    stdcall Scale,eax
    invoke IntersectClipRect,[bbDC],0,0,eax,[h]
    stdcall GfxBegin,[bbDC]
    stdcall PaintSidebar
    stdcall GfxEnd
    invoke RestoreDC,[bbDC],ebx
    ret
endp

proc Paint uses ebx esi edi,window
    locals
        ps PAINTSTRUCT
        rect RECT
        screenDC dd ?
    endl
    invoke BeginPaint,[window],addr ps
    mov [screenDC],eax
    invoke GetClientRect,[window],addr rect
    cmp [rect.right],0
    jle .empty
    cmp [rect.bottom],0
    jle .empty
    stdcall EnsureBuffers,[screenDC],[rect.right],[rect.bottom]
    mov [splashCovering],0
    cmp [splashActive],0
    je .interface
    stdcall SplashCoversWindow
    test eax,eax
    jz .reveal
    ; sheet covers everything. While the mark holds still, render the
    ; (static) interface once so the contraction frames only blit it.
    cmp [splashUiReady],0
    jne .cover
    cmp [splashStart],0
    je .cover
    stdcall SplashElapsed
    cmp eax,SPLASH_SNAP_AT
    jb .cover
    stdcall PaintInterface,[rect.right],[rect.bottom]
    invoke BitBlt,[snapDC],0,0,[rect.right],[rect.bottom],[bbDC],0,0,SRCCOPY
    mov [splashUiReady],1
.cover:
    stdcall SplashEnsureCache,[screenDC],[rect.right],[rect.bottom]
    invoke BitBlt,[bbDC],0,0,[rect.right],[rect.bottom],[splashDC],0,0,SRCCOPY
    mov [splashCovering],1
    jmp .overlay
.reveal:
    ; contraction: the interface underneath is the cached snapshot
    cmp [splashUiReady],0
    jne .snapshot
    stdcall PaintInterface,[rect.right],[rect.bottom]
    invoke BitBlt,[snapDC],0,0,[rect.right],[rect.bottom],[bbDC],0,0,SRCCOPY
    mov [splashUiReady],1
    jmp .overlay
.snapshot:
    invoke BitBlt,[bbDC],0,0,[rect.right],[rect.bottom],[snapDC],0,0,SRCCOPY
    jmp .overlay
.interface:
    stdcall PaintInterface,[rect.right],[rect.bottom]
.overlay:
    stdcall GfxBegin,[bbDC]
    cmp [splashActive],0
    je @f
    stdcall HotReset
@@:
    stdcall PaintSideTooltip
    stdcall PaintSplash
    stdcall PaintWindowButtons
    stdcall GfxEnd
    invoke BitBlt,[screenDC],0,0,[rect.right],[rect.bottom],[bbDC],0,0,SRCCOPY
.empty:
    invoke EndPaint,[window],addr ps
    ret
endp

; ---------------------------------------------------------------- input
proc Click uses ebx esi edi,x,y
    stdcall SidebarSync
    stdcall SidebarWidth
    cmp [x],eax
    jl .sidebar
    ; content works in expanded-layout coordinates
    mov eax,[contentX]
    sub [x],eax
    jmp .content
.sidebar:
    ; collapse / expand toggle
    stdcall Lerp,184,22,[sideT]
    mov ecx,[x]
    sub ecx,eax
    cmp ecx,24
    jae .notToggle
    stdcall Lerp,92,78,[sideT]
    mov ecx,[y]
    sub ecx,eax
    cmp ecx,24
    jae .notToggle
    stdcall SidebarToggle
    jmp .done
.notToggle:
    ; language switch (sidebar footer)
    cmp [y],654
    jb .navArea
    cmp [y],684
    ja .done
    cmp [sideT],512
    jb .langSegments
    mov eax,[uiLang]
    inc eax
    cmp eax,3
    jb @f
    xor eax,eax
@@:
    stdcall LangSet,eax
    jmp .done
.langSegments:
    mov eax,[x]
    sub eax,19
    jl .done
    xor edx,edx
    mov ecx,60
    div ecx
    cmp eax,2
    ja .done
    stdcall LangSet,eax
    jmp .done
.navArea:
    cmp [y],608
    jae .folder
    mov eax,[y]
    sub eax,NAV_TOP
    jl .done
    xor edx,edx
    mov ecx,NAV_STEP
    div ecx
    cmp edx,NAV_H
    jae .done
    cmp eax,NAV_COUNT-1
    ja .done
    stdcall NavPageFromIndex,eax
    cmp eax,[currentPage]
    je .done
    stdcall GoPage,eax
    jmp .done
.folder:
    cmp [busy],0
    jne .done
    cmp [y],642
    ja .done
    stdcall EnsureDirectory,gameDir
    invoke ShellExecuteW,[hWnd],openVerb,gameDir,0,0,SW_SHOWNORMAL
    jmp .done
.content:
    cmp [currentPage],1
    jne .modContent
    cmp [vmActive],0
    jne .versionPage
    stdcall ClickVersionManager,[x],[y]
    jmp .done
.versionPage:
    stdcall ClickVersionPage,[x],[y]
    jmp .done
.modContent:
    cmp [currentPage],4
    jne .packContent
    stdcall ClickMods,[x],[y]
    jmp .done
.packContent:
    cmp [currentPage],5
    jne .personalContent
    stdcall ClickPacks,[x],[y]
    jmp .done
.personalContent:
    cmp [currentPage],7
    je .logs
    cmp [currentPage],8
    je .servers
    cmp [currentPage],6
    jne .regularContent
    stdcall ClickPersonalization,[x],[y]
    jmp .done
.logs:
    stdcall ClickLogs,[x],[y]
    jmp .done
.servers:
    stdcall ServersUiClick,[x],[y]
    jmp .done
.regularContent:
    cmp [currentPage],2
    jne .regularBusy
    stdcall ClickAccount,[x],[y]
    jmp .done
.regularBusy:
    cmp [busy],0
    jne .done
.idle:
    cmp [currentPage],0
    je .home
    cmp [currentPage],3
    jne .settingsControls
    ; Secondary tools now live inside Settings.
    cmp [y],600
    jb .settingsControls
    cmp [y],642
    ja .settingsControls
    cmp [x],458
    jb .settingsControls
    cmp [x],648
    jbe .openPersonal
    cmp [x],664
    jb .settingsControls
    cmp [x],854
    ja .settingsControls
    stdcall GoPage,7
    jmp .done
.openPersonal:
    stdcall GoPage,6
    jmp .done
.settingsControls:
    ; settings: language segmented control
    cmp [y],402
    jb .saveButton
    cmp [y],444
    ja .saveButton
    cmp [x],643
    jb .done
    mov eax,[x]
    sub eax,643
    xor edx,edx
    mov ecx,98
    div ecx
    cmp eax,2
    ja .done
    stdcall LangSet,eax
    jmp .done
.saveButton:
    cmp [y],600
    jb .done
    cmp [y],653
    ja .done
    cmp [x],252
    jb .done
    cmp [x],442
    ja .done
    stdcall SaveSettings
    test eax,eax
    jz .redraw
    invoke lstrcpyW,statusText,sSaved
    stdcall StatusChanged
    jmp .redraw
.home:
    cmp [x],620
    jb .homeSession
    cmp [x],714
    ja .homeSession
    cmp [y],384
    jb .homeSession
    cmp [y],422
    ja .homeSession
    stdcall OpenCurrentVersionPage
    jmp .done
.homeSession:
    cmp [y],366
    jb .launch
    cmp [y],402
    ja .launch
    cmp [x],786
    jb .done
    cmp [x],1064
    ja .done
    xor eax,eax
    cmp [x],925
    setae al
    mov [onlineMode],eax
    stdcall SyncPage
    jmp .done
.launch:
    cmp [y],576
    jb .done
    cmp [y],632
    ja .done
    cmp [x],828
    jb .done
    cmp [x],1088
    ja .done
    stdcall StartWorker,EngineLaunch
    jmp .done
.redraw:
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp

proc ClientToLogical lparam
    movsx eax,word [lparam]
    invoke MulDiv,eax,96,[dpi]
    mov ecx,eax
    movsx eax,word [lparam+2]
    push ecx
    invoke MulDiv,eax,96,[dpi]
    pop ecx
    mov edx,eax
    mov eax,ecx
    ret
endp

proc WindowProc uses ebx esi edi,window,message,wparam,lparam
    cmp [message],WM_CREATE
    je .create
    cmp [message],WM_PAINT
    je .paint
    cmp [message],WM_ERASEBKGND
    je .handled
    cmp [message],WM_NCCALCSIZE
    je .ncCalc
    cmp [message],WM_NCHITTEST
    je .ncHit
    cmp [message],WM_NCACTIVATE
    je .ncActivate
    cmp [message],WM_MOUSEMOVE
    je .mouseMove
    cmp [message],WM_MOUSELEAVE
    je .mouseLeave
    cmp [message],WM_LBUTTONDOWN
    je .press
    cmp [message],WM_LBUTTONUP
    je .click
    cmp [message],WM_SETCURSOR
    je .cursor
    cmp [message],WM_TIMER
    je .timer
    cmp [message],WM_ACTIVATE
    je .activate
    cmp [message],WM_COMMAND
    je .command
    cmp [message],WM_DRAWITEM
    je .drawItem
    cmp [message],WM_MEASUREITEM
    je .measureItem
    cmp [message],WM_CTLCOLOREDIT
    je .editColor
    cmp [message],WM_CTLCOLORLISTBOX
    je .editColor
    cmp [message],WM_CTLCOLORSTATIC
    je .editColor
    cmp [message],WM_CTLCOLORSTATIC
    je .staticColor
    cmp [message],WM_STATUS
    je .status
    cmp [message],WM_FINISHED
    je .finished
    cmp [message],WM_CLOSE
    je .close
    cmp [message],WM_DESTROY
    je .destroy
.default:
    invoke DefWindowProcW,[window],[message],[wparam],[lparam]
    ret
.create:
    mov eax,[window]
    mov [hWnd],eax
    stdcall CreateControls
    xor eax,eax
    ret
.paint:
    stdcall Paint,[window]
    xor eax,eax
    ret
.handled:
    mov eax,1
    ret
.ncCalc:
    ; no non-client area: the client fills the window (borderless)
    xor eax,eax
    ret
.ncActivate:
    ; keep DWM from repainting a caption over the client
    invoke DefWindowProcW,[window],[message],[wparam],-1
    ret
.ncHit:
    movsx eax,word [lparam]
    mov [ncPt.x],eax
    movsx eax,word [lparam+2]
    mov [ncPt.y],eax
    invoke MapWindowPoints,0,[window],ncPt,1
    invoke MulDiv,[ncPt.x],96,[dpi]
    mov ebx,eax
    invoke MulDiv,[ncPt.y],96,[dpi]
    mov esi,eax
    ; the top band drags the window wherever nothing clickable is drawn
    cmp esi,108
    jge .ncClient
    stdcall HotFind,ebx,esi
    cmp eax,-1
    jne .ncClient
    mov eax,HTCAPTION
    ret
.ncClient:
    mov eax,HTCLIENT
    ret
.mouseMove:
    cmp [mouseTracking],0
    jne .tracked
    mov eax,[window]
    mov [mouseTrack+8],eax
    invoke TrackMouseEvent,mouseTrack
    mov [mouseTracking],1
.tracked:
    stdcall ClientToLogical,[lparam]
    stdcall MouseMoved,eax,edx
    xor eax,eax
    ret
.mouseLeave:
    mov [mouseTracking],0
    stdcall MouseLeft
    xor eax,eax
    ret
.press:
    stdcall ClientToLogical,[lparam]
    stdcall WindowButtonHit,eax,edx
    test eax,eax
    jnz .pressAny
    cmp [splashActive],0
    jne .splashSkip
.pressAny:
    stdcall ClientToLogical,[lparam]
    stdcall HotFind,eax,edx
    mov [pressIdx],eax
    invoke InvalidateRect,[window],0,0
    xor eax,eax
    ret
.cursor:
    mov eax,[wparam]
    cmp eax,[window]
    jne .default
    mov eax,[lparam]
    and eax,0FFFFh
    cmp eax,HTCLIENT
    jne .default
    cmp [hoverIdx],-1
    je .default
    invoke SetCursor,[handCursor]
    mov eax,1
    ret
.timer:
    stdcall NowMs
    sub eax,[lastLoopFrame]
    cmp eax,40
    jb .timerDone
    stdcall AnimFrame
.timerDone:
    xor eax,eax
    ret
.activate:
    stdcall AnimKick
    jmp .default
.command:
    stdcall HomeVersionCommand,[wparam]
    stdcall ModCommand,[wparam]
    stdcall VersionCommand,[wparam]
    stdcall VersionPageCommand,[wparam]
    stdcall PackCommand,[wparam]
    stdcall PersonalizationCommand,[wparam]
    stdcall AccountCommand,[wparam]
    stdcall ServersUiCommand,[wparam]
    ; focus rings and the hero version follow their controls
    mov eax,[wparam]
    shr eax,16
    cmp eax,EN_SETFOCUS
    je .refresh
    cmp eax,EN_KILLFOCUS
    je .refresh
    cmp eax,CBN_SETFOCUS
    je .refresh
    cmp eax,CBN_KILLFOCUS
    je .refresh
    mov eax,[wparam]
    and eax,0FFFFh
    cmp eax,101
    jne .commandDone
.refresh:
    invoke InvalidateRect,[window],0,0
.commandDone:
    xor eax,eax
    ret
.drawItem:
    stdcall DrawModProject,[lparam]
    mov eax,1
    ret
.measureItem:
    mov ebx,[lparam]
    cmp dword [ebx+4],207
    jne .handled
    stdcall Scale,54
    mov [ebx+16],eax
    mov eax,1
    ret
.editColor:
    invoke SetTextColor,[wparam],C_INK
    invoke SetBkMode,[wparam],TRANSPARENT
    mov eax,[inputBrush]
    ret
.staticColor:
    invoke SetTextColor,[wparam],T_MUTED
    invoke SetBkMode,[wparam],TRANSPARENT
    mov eax,[inputBrush]
    ret
.click:
    stdcall ClientToLogical,[lparam]
    mov ebx,eax
    mov esi,edx
    stdcall WindowButtonHit,ebx,esi
    cmp eax,1
    je .minimize
    cmp eax,2
    je .closeButton
    cmp [splashActive],0
    jne .splashSkip
    stdcall ClientToLogical,[lparam]
    mov ebx,eax
    mov esi,edx
    mov [pressIdx],-1
    stdcall Click,ebx,esi
    invoke InvalidateRect,[window],0,0
    xor eax,eax
    ret
.splashSkip:
    stdcall SplashFinish
    xor eax,eax
    ret
.minimize:
    mov [pressIdx],-1
    stdcall MouseLeft
    invoke ShowWindow,[window],SW_MINIMIZE
    xor eax,eax
    ret
.closeButton:
    invoke PostMessageW,[window],WM_CLOSE,0,0
    xor eax,eax
    ret
.status:
    mov eax,1
    xchg eax,[statusLock]
    test eax,eax
    jnz .retryStatus
    invoke lstrcpyW,statusText,statusIncoming
    mov [statusLock],0
    stdcall StatusChanged
    cmp [currentPage],1
    jne .statusPaint
    cmp [vmActive],0
    jne .statusPaint
    cmp [versionTab],2
    jne .statusPaint
    stdcall VersionFillQueue
.statusPaint:
    invoke InvalidateRect,[window],0,0
    xor eax,eax
    ret
.retryStatus:
    invoke PostMessageW,[window],WM_STATUS,0,0
    xor eax,eax
    ret
.finished:
    mov [busy],0
    mov [workerCallback],0
    invoke EnableWindow,[hVersion],1
    invoke EnableWindow,[hPlayer],1
    invoke EnableWindow,[hClient],1
    invoke EnableWindow,[hGameDir],1
    invoke EnableWindow,[hJava],1
    invoke EnableWindow,[hMemory],1
    invoke EnableWindow,[hJvmArgs],1
    stdcall EnableModControls,1
    stdcall EnableVersionControls,1
    stdcall EnableVersionPageControls,1
    stdcall EnablePackControls,1
    stdcall EnablePersonalizationControls,1
    stdcall EnableAccountControls,1
    stdcall ServersUiEnable,1
    cmp [workerHandle],0
    je .finishedPaint
    invoke CloseHandle,[workerHandle]
    mov [workerHandle],0
.finishedPaint:
    stdcall ModFinishUI
    stdcall PackFinishUI
    stdcall DetailFinishUI
    stdcall VersionPageFinishUI
    stdcall AccountFinishUI
    stdcall ServersUiFinish
    ; Disabling the focused edit/list during a worker clears keyboard focus.
    ; Restore it so shortcuts and Tab continue working after completion.
    invoke SetFocus,[window]
    stdcall EngineScanVersions,[hVersion]
    invoke SetWindowTextA,[hVersion],selectedVersion
    stdcall VersionFinishUI
    stdcall RefreshHomeVersionSettings
    stdcall LogFinishUI
    ; Refresh deferred page scans now that worker-owned buffers are released.
    stdcall SyncPage
    invoke InvalidateRect,[window],0,0
    xor eax,eax
    ret
.close:
    mov [authCancel],1
    mov [engineCancel],1
    mov [modCancel],1
    stdcall SaveSettings
    invoke DestroyWindow,[window]
    xor eax,eax
    ret
.destroy:
    invoke KillTimer,[window],1
    stdcall FreeBuffers
    invoke PostQuitMessage,0
    xor eax,eax
    ret
endp

include 'ui_gfx.inc'
include 'browse_defaults.inc'
include 'splash.inc'
include 'sidebar.inc'
include 'personalize.inc'
include 'i18n.inc'
include 'common.inc'
include 'memory.inc'
include 'engine.inc'
include 'launch_log.inc'
include 'logs_ui.inc'
include 'version_settings.inc'
include 'mc_sources.inc'
include 'profiles.inc'
include 'loaders.inc'
include 'loader_change.inc'
include 'versions.inc'
include 'versions_ui.inc'
include 'version_page.inc'
include 'loader_change_ui.inc'
include 'servers_nbt.inc'
include 'servers_net.inc'
include 'servers_ui.inc'
include 'version_actions.inc'
include 'version_mods.inc'
include 'auth.inc'
include 'skin.inc'
include 'account_ui.inc'
include 'mods_ui.inc'
include 'mod_profile.inc'
include 'mods.inc'
include 'modrinth.inc'
include 'curseforge.inc'
include 'catalog.inc'
include 'project_details.inc'
include 'packs.inc'
include 'pack_export.inc'
include 'packs_ui.inc'
include 'details_ui.inc'
include 'imports.inc'

section '.rsrc' resource data readable
directory RT_ICON,iconDirectory,RT_GROUP_ICON,groupDirectory,RT_VERSION,versionDirectory,RT_MANIFEST,manifestDirectory
resource iconDirectory,1,LANG_NEUTRAL,icon16,2,LANG_NEUTRAL,icon24,3,LANG_NEUTRAL,icon32,4,LANG_NEUTRAL,icon48,5,LANG_NEUTRAL,icon64,6,LANG_NEUTRAL,icon256
resource groupDirectory,1,LANG_NEUTRAL,mainIcon
resource versionDirectory,1,LANG_NEUTRAL,versionData
resource manifestDirectory,1,LANG_NEUTRAL,manifestData
icon mainIcon,icon16,'icons\offset-16.ico',icon24,'icons\offset-24.ico',icon32,'icons\offset-32.ico',icon48,'icons\offset-48.ico',icon64,'icons\offset-64.ico',icon256,'icons\offset-256.ico'
versioninfo versionData,VOS__WINDOWS32,VFT_APP,VFT2_UNKNOWN,LANG_ENGLISH+SUBLANG_DEFAULT,0,\
 'FileDescription','OFFSET Minecraft Launcher',\
 'FileVersion','0.9.0',\
 'ProductName','OFFSET',\
 'ProductVersion','0.9.0',\
 'OriginalFilename','OFFSET.exe'
resdata manifestData
file 'app.manifest'
endres
