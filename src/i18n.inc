; Runtime UI language (0 = 中文, 1 = English, 2 = 日本語).
; UI strings live in fixed slots (strings.inc) that are refilled on switch, so
; every existing reference such as sHome keeps working unchanged. Worker status
; text (UTF-8) is translated by pointer through statusMap (strings_status.inc).

section '.i18nd' data readable writeable
uiLang dd 0
kLanguage du 'Language',0
langCodeZh du 'zh',0
langCodeEn du 'en',0
langCodeJa du 'ja',0
langCodes dd langCodeZh,langCodeEn,langCodeJa
langNameZh dw 4E2Dh,6587h,0
langNameEn du 'EN',0
langNameJa dw 65E5h,672Ch,8A9Eh,0
langNames dd langNameZh,langNameEn,langNameJa
faceZh du 'Microsoft YaHei UI',0
faceEn du 'Segoe UI',0
faceJa du 'Yu Gothic UI',0
faceZhStrong du 'Microsoft YaHei UI',0
faceEnStrong du 'Segoe UI Semibold',0
faceJaStrong du 'Yu Gothic UI Semibold',0
langFaces dd faceZh,faceEn,faceJa
langStrongFaces dd faceZhStrong,faceEnStrong,faceJaStrong
; Format strings and prefixes that workers combine before SetStatusA.
trSources dd vFmtInstall,vFmtDone,eFmtProgress,mrDependencyPrefix,mrDependencyUnknown,authCodePrefix,authCodeMiddle,authSignedPrefix
TR_COUNT = 8
trVFmtInstall dd vFmtInstall
trVFmtDone dd vFmtDone
trEFmtProgress dd eFmtProgress
trMrDependencyPrefix dd mrDependencyPrefix
trMrDependencyUnknown dd mrDependencyUnknown
trAuthCodePrefix dd authCodePrefix
trAuthCodeMiddle dd authCodeMiddle
trAuthSignedPrefix dd authSignedPrefix
langCode rw 8
include 'strings_status.inc'

section '.i18nc' code readable executable

; Copy the selected language into every UI string slot.
proc LangApply uses ebx esi edi
    mov eax,[uiLang]
    cmp eax,LANG_COUNT
    jb @f
    xor eax,eax
    mov [uiLang],eax
@@:
    mov esi,[uiLangData+eax*4]
    xor ebx,ebx
.next:
    cmp ebx,UI_STRING_COUNT
    jae .done
    mov edi,[uiStringSlots+ebx*4]
.copy:
    lodsw
    stosw
    test ax,ax
    jnz .copy
    inc ebx
    jmp .next
.done:
    xor ebx,ebx
.pointers:
    stdcall LangTrA,dword [trSources+ebx*4]
    mov [trVFmtInstall+ebx*4],eax
    inc ebx
    cmp ebx,TR_COUNT
    jb .pointers
    ret
endp

; Translate a worker status string (UTF-8 pointer) to the current language.
proc LangTrA uses ebx,source
    mov eax,[source]
    xor ebx,ebx
.next:
    cmp ebx,STATUS_STRING_COUNT
    jae .done
    mov ecx,ebx
    shl ecx,4
    cmp eax,[statusMap+ecx]
    je .found
    inc ebx
    jmp .next
.found:
    mov edx,[uiLang]
    mov eax,[statusMap+ecx+4+edx*4]
.done:
    ret
endp

; Read the saved language, or follow the Windows display language on first run.
proc LangLoad
    invoke GetPrivateProfileStringW,sectionName,kLanguage,emptyW,langCode,8,iniPath
    xor ecx,ecx
.match:
    cmp ecx,LANG_COUNT
    jae .detect
    push ecx
    invoke lstrcmpiW,langCode,dword [langCodes+ecx*4]
    pop ecx
    test eax,eax
    jz .found
    inc ecx
    jmp .match
.found:
    mov [uiLang],ecx
    ret
.detect:
    invoke GetUserDefaultUILanguage
    and eax,3FFh
    mov ecx,1
    cmp eax,04h
    jne @f
    xor ecx,ecx
@@:
    cmp eax,11h
    jne @f
    mov ecx,2
@@:
    mov [uiLang],ecx
    ret
endp

proc LangSave
    mov eax,[uiLang]
    invoke WritePrivateProfileStringW,sectionName,kLanguage,dword [langCodes+eax*4],iniPath
    ret
endp

; Switch language at runtime: strings, fonts, list contents and cue banners.
proc LangSet uses ebx esi,index
    cmp [busy],0
    jne .done
    mov eax,[index]
    cmp eax,LANG_COUNT
    jae .done
    cmp eax,[uiLang]
    je .done
    mov [uiLang],eax
    stdcall LangApply
    stdcall CreateFonts
    stdcall ApplyControlFonts
    ; version filter
    invoke SendMessageW,[hVCFilter],CB_GETCURSEL,0,0
    mov ebx,eax
    invoke SendMessageW,[hVCFilter],CB_RESETCONTENT,0,0
    invoke SendMessageW,[hVCFilter],CB_ADDSTRING,0,sVRelease
    invoke SendMessageW,[hVCFilter],CB_ADDSTRING,0,sVSnapshot
    invoke SendMessageW,[hVCFilter],CB_ADDSTRING,0,sVAll
    invoke SendMessageW,[hVCFilter],CB_SETCURSEL,ebx,0
    ; loader kind (first entry is localized)
    invoke SendMessageW,[hVLoader],CB_GETCURSEL,0,0
    mov ebx,eax
    invoke SendMessageW,[hVLoader],CB_RESETCONTENT,0,0
    xor esi,esi
.kinds:
    invoke SendMessageW,[hVLoader],CB_ADDSTRING,0,dword [vUiNames+esi*4]
    inc esi
    cmp esi,5
    jb .kinds
    invoke SendMessageW,[hVLoader],CB_SETCURSEL,ebx,0
    invoke SendMessageW,[hVCQuery],1501h,0,sVSearchCue
    invoke SendMessageW,[hModQuery],1501h,0,sModQueryCue
    stdcall VersionFillQueue
    cmp [currentPage],4
    jne .title
    cmp [modTab],1
    jne .title
    stdcall RefreshLocalList
.title:
    invoke SendMessageW,[hPackQuery],1501h,0,sPackQueryCue
    invoke SendMessageW,[hPackGame],1501h,0,sPackGameCue
    stdcall ThemeApply
    ; A language change ends any transition and reveals the current controls.
    ; Sync also rebuilds version-mod list prefixes without changing drafts.
    mov [transActive],0
    mov [transEase],1024
    stdcall SyncPage
    invoke lstrcpyW,statusText,sReady
    stdcall LangSave
    invoke InvalidateRect,[hWnd],0,0
.done:
    ret
endp
