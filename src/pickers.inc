; Settings pickers: game folder (modern Windows folder dialog), Java executable
; (file dialog) and local Java detection. Pickers only fill the fields; Save
; applies them through the existing validated settings path.

JD_MAX = 16
JD_PATH = 520                   ; UTF-16 units per detected path

section '.pickd' data readable writeable
; {DC1C5A9C-E88A-4DDE-A5A1-60F82A20AEF7}
pkClsidFileOpen dd 0DC1C5A9Ch
 dw 0E88Ah,4DDEh
 db 0A5h,0A1h,60h,0F8h,2Ah,20h,0AEh,0F7h
; {D57C7288-D4AD-4768-BE02-9D969532D960}
pkIidFileOpen dd 0D57C7288h
 dw 0D4ADh,4768h
 db 0BEh,02h,9Dh,96h,95h,32h,0D9h,60h
; {43826D1E-E718-42EE-BC55-A1E261C37BFE}
pkIidShellItem dd 43826D1Eh
 dw 0E718h,42EEh
 db 0BCh,55h,0A1h,0E2h,61h,0C3h,7Bh,0FEh
pkDialog dd 0
pkItem dd 0
pkName dd 0
jdCount dd 0
jdEnvHome du 'JAVA_HOME',0
jdEnvPf64 du 'ProgramW6432',0
jdEnvAppData du 'APPDATA',0
jdEnvLocal du 'LOCALAPPDATA',0
jdProgramFiles du 'C:\Program Files',0
jdJavaw du 'javaw.exe',0
jdBinJavaw du '\bin\javaw.exe',0
jdBinJava du '\bin\java.exe',0
jdStar du '\*',0
jdSlash du '\',0
jdRelease du '\release',0
jdRuntimeMc du '\.minecraft\runtime',0
jdRuntimeStore du '\Packages\Microsoft.4297127D64EC6_8wekyb3d8bbwe\LocalCache\Local\runtime',0
jdWindowsX64 du '\windows-x64\',0
jdKeyVersion db 'JAVA_VERSION="',0
jdItemFmt du 'Java %u      %s',0
jdItemFmtUnknown du 'Java      %s',0
jdVendors dd jdVJava,jdVAdoptium,jdVMicrosoft,jdVZulu,jdVBellSoft,jdVCorretto,jdVSemeru,jdVEclipse,jdVAdopt,jdVOpen
jdVendorsEnd:
jdVJava du '\Java',0
jdVAdoptium du '\Eclipse Adoptium',0
jdVMicrosoft du '\Microsoft',0
jdVZulu du '\Zulu',0
jdVBellSoft du '\BellSoft',0
jdVCorretto du '\Amazon Corretto',0
jdVSemeru du '\Semeru',0
jdVEclipse du '\Eclipse Foundation',0
jdVAdopt du '\AdoptOpenJDK',0
jdVOpen du '\OpenJDK',0
jdFilter du 'Java (javaw.exe, java.exe)',0,'javaw.exe;java.exe',0,'*.exe',0,'*.exe',0,0
; ---- reserved (no file space)
jdMajor dd JD_MAX dup(0)
jdPaths rw JD_MAX*JD_PATH
jdItem rw 700
jdShort rw 80
jdHome rw 1024
jdRoot rw 1024
jdPf rw 1024
jdProbe rw 1024
jdFind WIN32_FIND_DATAW
pickPathW rw 1024
pickOpen OPENFILENAME

section '.pickc' code readable executable
 nop

; ---------------------------------------------------------------- folder
; Modern IFileOpenDialog in folder mode. Returns 1 with the folder in pickPathW.
proc PickFolder uses ebx esi edi,initial,title
 local coInit:DWORD,options:DWORD,result:DWORD
 mov [result],0
 mov [pkDialog],0
 mov [pkItem],0
 mov [pkName],0
 invoke CoInitializeEx,0,2
 mov [coInit],eax
 invoke CoCreateInstance,pkClsidFileOpen,0,1,pkIidFileOpen,pkDialog
 test eax,eax
 jnz .done
 mov ebx,[pkDialog]
 mov ecx,[ebx]
 lea eax,[options]
 stdcall dword [ecx+10*4],ebx,eax                  ; GetOptions
 mov eax,[options]
 or eax,20h+40h+800h                               ; PICKFOLDERS FORCEFILESYSTEM PATHMUSTEXIST
 mov ecx,[ebx]
 stdcall dword [ecx+9*4],ebx,eax                   ; SetOptions
 mov ecx,[ebx]
 stdcall dword [ecx+17*4],ebx,[title]              ; SetTitle
 mov esi,[initial]
 cmp word [esi],0
 je .show
 invoke GetFileAttributesW,esi
 cmp eax,-1
 je .show
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .show
 invoke SHCreateItemFromParsingName,esi,0,pkIidShellItem,pkItem
 test eax,eax
 jnz .show
 mov ecx,[ebx]
 stdcall dword [ecx+12*4],ebx,[pkItem]             ; SetFolder
 mov eax,[pkItem]
 mov ecx,[eax]
 stdcall dword [ecx+2*4],eax                       ; Release
 mov [pkItem],0
.show:
 mov ecx,[ebx]
 stdcall dword [ecx+3*4],ebx,[hWnd]                ; Show
 test eax,eax
 jnz .release
 mov ecx,[ebx]
 stdcall dword [ecx+20*4],ebx,pkItem               ; GetResult
 test eax,eax
 jnz .release
 mov eax,[pkItem]
 mov ecx,[eax]
 stdcall dword [ecx+5*4],eax,80058000h,pkName      ; GetDisplayName(FILESYSPATH)
 test eax,eax
 jnz .item
 invoke lstrlenW,[pkName]
 cmp eax,1000
 jae .free
 invoke lstrcpyW,pickPathW,[pkName]
 mov [result],1
.free:
 invoke CoTaskMemFree,[pkName]
.item:
 mov eax,[pkItem]
 mov ecx,[eax]
 stdcall dword [ecx+2*4],eax
.release:
 mov ebx,[pkDialog]
 mov ecx,[ebx]
 stdcall dword [ecx+2*4],ebx
.done:
 cmp [coInit],1
 ja @f
 invoke CoUninitialize
@@:
 mov eax,[result]
 ret
endp

proc BrowseGameDir
 invoke GetWindowTextW,[hGameDir],textBuffer,1024
 stdcall PickFolder,textBuffer,sGameDirLabel
 test eax,eax
 jz .done
 invoke SetWindowTextW,[hGameDir],pickPathW
 invoke lstrcpyW,statusText,sPickSaveHint
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp

; ---------------------------------------------------------------- Java file
proc BrowseJava uses edi
 invoke GetWindowTextW,[hJava],pickPathW,1024
 invoke GetFileAttributesW,pickPathW
 cmp eax,-1
 je .blank
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jz .dialog
.blank:
 mov word [pickPathW],0
.dialog:
 mov edi,pickOpen
 xor eax,eax
 mov ecx,sizeof.OPENFILENAME/4
 rep stosd
 mov [pickOpen.lStructSize],sizeof.OPENFILENAME
 mov eax,[hWnd]
 mov [pickOpen.hwndOwner],eax
 mov [pickOpen.lpstrFilter],jdFilter
 mov [pickOpen.lpstrFile],pickPathW
 mov [pickOpen.nMaxFile],1024
 mov [pickOpen.lpstrTitle],sJavaPickTitle
 mov [pickOpen.Flags],OFN_FILEMUSTEXIST+OFN_PATHMUSTEXIST+OFN_NOCHANGEDIR+OFN_EXPLORER
 invoke GetOpenFileNameW,pickOpen
 test eax,eax
 jz .done
 ; Prefer the console-free javaw.exe beside a chosen java.exe.
 stdcall JdPreferJavaw,pickPathW
 invoke SetWindowTextW,[hJava],pickPathW
 invoke lstrcpyW,statusText,sPickSaveHint
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp

proc JdPreferJavaw uses esi,path
 invoke lstrlenW,[path]
 cmp eax,9
 jb .done
 mov esi,[path]
 lea esi,[esi+eax*2-18]                            ; "\java.exe" is 9 units
 invoke lstrcmpiW,esi,.java
 test eax,eax
 jnz .done
 invoke lstrcpyW,jdProbe,[path]
 invoke lstrlenW,jdProbe
 mov word [jdProbe+eax*2-18],0
 invoke lstrcatW,jdProbe,.javaw
 invoke GetFileAttributesW,jdProbe
 cmp eax,-1
 je .done
 invoke lstrcpyW,[path],jdProbe
.done:
 ret
.java du '\java.exe',0
.javaw du '\javaw.exe',0
endp

; ---------------------------------------------------------------- detection
; Major version from <home>\release (JAVA_VERSION="21.0.2" or "1.8.0_x").
proc JdReleaseMajor uses ebx esi edi,home
 invoke lstrcpyW,jdProbe,[home]
 invoke lstrcatW,jdProbe,jdRelease
 stdcall EReadFile,jdProbe
 test eax,eax
 jz .none
 mov ebx,eax
 mov esi,eax
.find:
 cmp byte [esi],0
 je .free0
 mov edi,jdKeyVersion
 mov ecx,esi
.cmp:
 mov al,[edi]
 test al,al
 jz .found
 cmp al,[ecx]
 jne .next
 inc edi
 inc ecx
 jmp .cmp
.next:
 inc esi
 jmp .find
.found:
 mov esi,ecx
 stdcall JdNumber
 cmp eax,1
 jne .free
 cmp byte [esi],'.'
 jne .free
 inc esi
 stdcall JdNumber
 jmp .free
.free0:
 xor eax,eax
.free:
 push eax
 stdcall FreeMem,ebx
 pop eax
 ret
.none:
 xor eax,eax
 ret
endp

; Parse decimal digits at esi (advances esi). Returns 0 if none.
proc JdNumber
 xor eax,eax
 xor ecx,ecx
.digit:
 movzx edx,byte [esi]
 sub edx,'0'
 cmp edx,9
 ja .done
 imul eax,10
 add eax,edx
 inc esi
 inc ecx
 cmp ecx,4
 jb .digit
.done:
 ret
endp

; Record an existing javaw.exe once (case-insensitive), with its major version.
proc JdAdd uses ebx esi edi,path
 invoke GetFileAttributesW,[path]
 cmp eax,-1
 je .done
 test eax,FILE_ATTRIBUTE_DIRECTORY
 jnz .done
 invoke lstrlenW,[path]
 cmp eax,JD_PATH-1
 jae .done
 cmp [jdCount],JD_MAX
 jae .done
 xor ebx,ebx
.dupe:
 cmp ebx,[jdCount]
 jae .add
 imul eax,ebx,JD_PATH*2
 add eax,jdPaths
 invoke lstrcmpiW,eax,[path]
 test eax,eax
 jz .done
 inc ebx
 jmp .dupe
.add:
 mov ebx,[jdCount]
 imul edi,ebx,JD_PATH*2
 add edi,jdPaths
 invoke lstrcpyW,edi,[path]
 ; home = path without "\bin\javaw.exe"
 invoke lstrcpyW,jdHome,edi
 stdcall JdParent,jdHome
 stdcall JdParent,jdHome
 stdcall JdReleaseMajor,jdHome
 test eax,eax
 jnz .major
 stdcall EJavaMajorFromPath,edi
.major:
 mov [jdMajor+ebx*4],eax
 inc [jdCount]
.done:
 ret
endp

; Menu label: paths over 56 units keep their first 20 and last 34 units.
proc JdShortPath uses esi edi,path
 invoke lstrlenW,[path]
 cmp eax,56
 ja .long
 invoke lstrcpyW,jdShort,[path]
 ret
.long:
 mov esi,[path]
 mov edi,jdShort
 mov ecx,20
 rep movsw
 mov word [edi],2026h                ; …
 add edi,2
 mov esi,[path]
 lea esi,[esi+eax*2-68]
 invoke lstrcpyW,edi,esi
 ret
endp

proc JdParent uses esi,path
 invoke lstrlenW,[path]
 mov esi,[path]
 lea esi,[esi+eax*2]
.back:
 cmp esi,[path]
 jbe .done
 sub esi,2
 cmp word [esi],'\'
 jne .back
 mov word [esi],0
.done:
 ret
endp

; <root>\<each subfolder><suffix>
proc JdScanDir uses ebx,root,suffix
 local handle:DWORD
 invoke lstrlenW,[root]
 cmp eax,700
 jae .done
 invoke lstrcpyW,jdRoot,[root]
 invoke lstrcatW,jdRoot,jdStar
 invoke FindFirstFileW,jdRoot,jdFind
 cmp eax,-1
 je .done
 mov [handle],eax
.entry:
 test [jdFind.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY
 jz .next
 cmp word [jdFind.cFileName],'.'
 je .next
 invoke lstrlenW,jdFind.cFileName
 cmp eax,200
 ja .next
 invoke lstrcpyW,jdItem,[root]
 invoke lstrcatW,jdItem,jdSlash
 invoke lstrcatW,jdItem,jdFind.cFileName
 invoke lstrcatW,jdItem,[suffix]
 stdcall JdAdd,jdItem
.next:
 invoke FindNextFileW,[handle],jdFind
 test eax,eax
 jnz .entry
 invoke FindClose,[handle]
.done:
 ret
endp

; Mojang runtimes: <root>\<component>\windows-x64\<component>\bin\javaw.exe
proc JdScanRuntime uses ebx,root
 local handle:DWORD
 invoke lstrlenW,[root]
 cmp eax,600
 jae .done
 invoke lstrcpyW,jdRoot,[root]
 invoke lstrcatW,jdRoot,jdStar
 invoke FindFirstFileW,jdRoot,jdFind
 cmp eax,-1
 je .done
 mov [handle],eax
.entry:
 test [jdFind.dwFileAttributes],FILE_ATTRIBUTE_DIRECTORY
 jz .next
 cmp word [jdFind.cFileName],'.'
 je .next
 invoke lstrlenW,jdFind.cFileName
 cmp eax,120
 ja .next
 invoke lstrcpyW,jdItem,[root]
 invoke lstrcatW,jdItem,jdSlash
 invoke lstrcatW,jdItem,jdFind.cFileName
 invoke lstrcatW,jdItem,jdWindowsX64
 invoke lstrcatW,jdItem,jdFind.cFileName
 invoke lstrcatW,jdItem,jdBinJavaw
 stdcall JdAdd,jdItem
.next:
 invoke FindNextFileW,[handle],jdFind
 test eax,eax
 jnz .entry
 invoke FindClose,[handle]
.done:
 ret
endp

proc JavaDetectAll uses ebx esi
 mov [jdCount],0
 ; the current field, JAVA_HOME
 invoke GetWindowTextW,[hJava],jdProbe,1024
 invoke lstrcpyW,jdItem,jdProbe
 stdcall JdAdd,jdItem
 invoke GetEnvironmentVariableW,jdEnvHome,jdItem,600
 test eax,eax
 jz .vendors
 cmp eax,600
 jae .vendors
 invoke lstrcatW,jdItem,jdBinJavaw
 stdcall JdAdd,jdItem
.vendors:
 ; 64-bit Program Files, even from this 32-bit process
 invoke GetEnvironmentVariableW,jdEnvPf64,jdPf,600
 test eax,eax
 jz .pfDefault
 cmp eax,600
 jb .pf
.pfDefault:
 invoke lstrcpyW,jdPf,jdProgramFiles
.pf:
 mov esi,jdVendors
.vendor:
 cmp esi,jdVendorsEnd
 jae .runtimes
 invoke lstrcpyW,jdProbe,jdPf
 invoke lstrcatW,jdProbe,dword [esi]
 invoke lstrcpyW,pickPathW,jdProbe
 stdcall JdScanDir,pickPathW,jdBinJavaw
 add esi,4
 jmp .vendor
.runtimes:
 invoke GetEnvironmentVariableW,jdEnvAppData,pickPathW,600
 test eax,eax
 jz .store
 cmp eax,600
 jae .store
 invoke lstrcatW,pickPathW,jdRuntimeMc
 stdcall JdScanRuntime,pickPathW
.store:
 invoke GetEnvironmentVariableW,jdEnvLocal,pickPathW,500
 test eax,eax
 jz .path
 cmp eax,500
 jae .path
 invoke lstrcatW,pickPathW,jdRuntimeStore
 stdcall JdScanRuntime,pickPathW
.path:
 invoke SearchPathW,0,jdJavaw,0,1024,jdItem,0
 test eax,eax
 jz .done
 cmp eax,1024
 jae .done
 stdcall JdAdd,jdItem
.done:
 mov eax,[jdCount]
 ret
endp

; Detect, then let the player choose from a native menu at the cursor.
proc JavaDetect uses ebx esi
 local popup:DWORD,pt:POINT
 stdcall JavaDetectAll
 test eax,eax
 jnz .menu
 invoke lstrcpyW,statusText,sJavaNone
 jmp .redraw
.menu:
 invoke CreatePopupMenu
 test eax,eax
 jz .done
 mov [popup],eax
 xor ebx,ebx
.item:
 cmp ebx,[jdCount]
 jae .track
 imul esi,ebx,JD_PATH*2
 add esi,jdPaths
 stdcall JdShortPath,esi
 mov esi,jdShort
 mov eax,[jdMajor+ebx*4]
 test eax,eax
 jz .unknown
 cinvoke wsprintfW,jdItem,jdItemFmt,eax,esi
 jmp .append
.unknown:
 cinvoke wsprintfW,jdItem,jdItemFmtUnknown,esi
.append:
 lea eax,[ebx+1]
 invoke AppendMenuW,[popup],0,eax,jdItem
 inc ebx
 jmp .item
.track:
 ; drop down from the Detect button, right-aligned so it stays inside the window
 mov eax,918
 add eax,[contentX]
 stdcall Scale,eax
 mov [pt.x],eax
 stdcall Scale,349
 mov [pt.y],eax
 invoke ClientToScreen,[hWnd],addr pt
 invoke TrackPopupMenu,[popup],0100h+0080h+0008h,[pt.x],[pt.y],0,[hWnd],0
 mov ebx,eax
 invoke DestroyMenu,[popup]
 test ebx,ebx
 jz .done
 dec ebx
 cmp ebx,[jdCount]
 jae .done
 imul esi,ebx,JD_PATH*2
 add esi,jdPaths
 invoke SetWindowTextW,[hJava],esi
 invoke lstrcpyW,statusText,sPickSaveHint
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
