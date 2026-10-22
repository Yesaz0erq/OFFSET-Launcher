; Native account controls, backend-fixture actions and real GDI+ atlas pixels.
; No live account, Microsoft browser, clipboard write or remote skin mutation.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
WM_FINISHED=8002h
section '.data' data readable writeable
hWnd dd 0
hClient dd 0
busy dd 0
currentPage dd 2
contentX dd 0
onlineMode dd 0
authReady dd 0
authCancel dd 0
engineCancel dd 0
modCancel dd 0
authName rb 64
authUuid rb 64
authUserCode rb 64
authVerifyUri rw 512
skinReady dd 0
skinCurrentReady dd 0
skinLocalReady dd 0
skinModel dd 0
skinLocalModel dd 0
skinLocalHeight dd 0
skinAction dd 0
skinCurrentPathW rw 2048
skinLocalPathW rw 2048
skinPreviewPathW rw 2048
statusText rw 1024
gOffX dd 0
gOffY dd 0
gfx dd 0
gdipInput dd 1,0,0,0
gdipToken dd 0
canvas dd 0
pixel dd 0
editClass du 'EDIT',0
emptyW dw 0
openVerb du 'open',0
fixtureName db 'PreviewPlayer',0
fixtureUuid db '0123456789abcdef0123456789abcdef',0
fixtureUuidW du '0123456789abcdef0123456789abcdef',0
fixtureClient db '54048348-72bb-4ed2-a8a4-c72423265e94',0
fixtureCode db 'TEST-CODE',0
fixtureCodeW du 'TEST-CODE',0
fixtureVerify du 'https://microsoft.com/devicelogin',0
fixtureOld du 'prior status',0
fixtureOut du 'tests\out',0
fixtureBase du 'tests\out\account-ui-',0
fixtureSerial du '%08X',0
fixtureLocalSuffix du '\local64.png',0
fixtureCurrentSuffix du '\current.png',0
fixtureLegacySuffix du '\legacy32.png',0
fixtureExportSuffix du '\exported.png',0
fixtureRoot rw 1024
fixtureLocal rw 2048
fixtureCurrent rw 2048
fixtureLegacy rw 2048
fixtureExport rw 2048
fixtureChosen rw 2048
textW rw 256
textA rb 128
rect RECT
workerCalls dd 0
lastWorker dd 0
postCalls dd 0
selectCalls dd 0
refreshCalls dd 0
uploadCalls dd 0
resetCalls dd 0
exportCalls dd 0
clearCalls dd 0
logoutCalls dd 0
browserCalls dd 0
chooserCalls dd 0
selectFail dd 0
chooserResult dd 0
lastModel dd 0
imageHeldAtStart dd 0
step dd 1
checkpoint dd 0
expectedColor dd 0
failureFormat db 'step %u check %u pixel %08X expected %08X',13,10,0
openPointer dd FixtureChoose
savePointer dd FixtureSave
postPointer dd FixturePost
shellPointer dd FixtureBrowser
png64:
 file 'fixtures\account\skin-64.png'
png64Bytes=$-png64
png32:
 file 'fixtures\account\skin-32.png'
png32Bytes=$-png32
align 4
include '..\src\strings.inc'
section '.text' code readable executable
start:
 invoke GdiplusStartup,gdipToken,gdipInput,0
 test eax,eax
 jnz failed
 invoke CreateDirectoryW,fixtureOut,0
 invoke lstrcpyW,fixtureRoot,fixtureBase
 invoke GetTickCount
 cinvoke wsprintfW,textW,fixtureSerial,eax
 invoke lstrcatW,fixtureRoot,textW
 invoke CreateDirectoryW,fixtureRoot,0
 stdcall FixturePath,fixtureLocal,fixtureLocalSuffix
 stdcall FixturePath,fixtureCurrent,fixtureCurrentSuffix
 stdcall FixturePath,fixtureLegacy,fixtureLegacySuffix
 stdcall FixturePath,fixtureExport,fixtureExportSuffix
 stdcall Put,fixtureLocal,png64,png64Bytes
 stdcall Put,fixtureCurrent,png64,png64Bytes
 stdcall Put,fixtureLegacy,png32,png32Bytes
 invoke CreateWindowExW,0,editClass,emptyW,WS_POPUP,0,0,500,30,0,0,0,0
 mov [hClient],eax
 invoke SetWindowTextA,eax,fixtureClient
 stdcall CreateAccountControls
 test eax,eax
 jz failed
 stdcall SyncPage
 invoke GetWindowLongW,[hClient],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 invoke GetWindowTextW,[hAccountUuid],textW,256
 test eax,eax
 jnz failed
 inc [step]
 ; Identity is a read-only native control, and advanced client ID stays private.
 mov [authReady],1
 invoke lstrcpyA,authName,fixtureName
 invoke lstrcpyA,authUuid,fixtureUuid
 stdcall SyncPage
 invoke GetWindowTextW,[hAccountUuid],textW,256
 invoke lstrcmpiW,textW,fixtureUuidW
 test eax,eax
 jnz failed
 invoke SendMessageW,[hAccountUuid],WM_CHAR,'X',0
 invoke GetWindowTextW,[hAccountUuid],textW,256
 invoke lstrcmpiW,textW,fixtureUuidW
 test eax,eax
 jnz failed
 stdcall ClickAccount,320,560
 cmp [accountUiAdvanced],1
 jne failed
 invoke GetWindowLongW,[hClient],GWL_STYLE
 test eax,WS_VISIBLE
 jz failed
 invoke GetWindowRect,[hClient],rect
 cmp [rect.left],480
 jne failed
 invoke GetWindowTextA,[hClient],textA,128
 invoke lstrcmpA,textA,fixtureClient
 test eax,eax
 jnz failed
 stdcall ClickAccount,320,560
 invoke GetWindowLongW,[hClient],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 inc [step]
 ; Tab routing hides account edits and preserves the application's identity.
 stdcall ClickAccount,550,140
 cmp [accountUiTab],1
 jne failed
 invoke GetWindowLongW,[hAccountUuid],GWL_STYLE
 test eax,WS_VISIBLE
 jnz failed
 stdcall ClickAccount,320,140
 cmp [accountUiTab],0
 jne failed
 invoke lstrcmpA,authUuid,fixtureUuid
 test eax,eax
 jnz failed
 inc [step]
 ; Native file cancellation and anonymous remote actions do not queue work.
 stdcall AccountUiChooseSkin
 cmp [workerCalls],0
 jne failed
 cmp [chooserCalls],1
 jne failed
 mov [authReady],0
 invoke lstrcpyW,statusText,fixtureOld
 stdcall AccountUiRemote,3
 cmp [workerCalls],0
 jne failed
 invoke lstrcmpiW,statusText,sSkinNeedLogin
 test eax,eax
 jnz failed
 inc [step]
 ; Local PNG preview is read-only and available without a signed-in account.
 mov [accountUiTab],1
 mov [chooserResult],1
 invoke lstrcpyW,fixtureChosen,fixtureLocal
 stdcall ClickAccount,876,232
 mov [checkpoint],1
 cmp [workerCalls],1
 jne failed
 cmp [lastWorker],AccountSkinWorker
 jne failed
 cmp [skinAction],2
 jne failed
 cmp [accountUiSkinResult],0
 jne failed
 stdcall ClickAccount,950,300
 cmp [accountUiModel],0
 jne failed
 stdcall AccountSkinWorker,0
 mov [checkpoint],2
 cmp [postCalls],1
 jne failed
 cmp [selectCalls],1
 jne failed
 mov [busy],0
 stdcall AccountFinishUI
 mov [checkpoint],3
 cmp [accountUiImage],0
 je failed
 cmp [accountUiImageHeight],64
 jne failed
 cmp [accountUiPreviewLocal],1
 jne failed
 cmp [skinAction],0
 jne failed
 cmp [refreshCalls],0
 jne failed
 cmp [uploadCalls],0
 jne failed
 invoke GdipBitmapGetPixel,[accountUiImage],40,8,pixel
 mov [checkpoint],5
 mov [expectedColor],0FFFFCC00h
 cmp [pixel],0FFFFCC00h
 jne failed
 ; The native image clone holds no lock on the source PNG.
 invoke DeleteFileW,fixtureLocal
 mov [checkpoint],4
 test eax,eax
 jz failed
 inc [step]
 ; Front/back UVs, transparent pixels and hat alpha render from real PNG pixels.
 stdcall CanvasBegin
 stdcall AccountUiSkinPerson,0,0,0
 stdcall AccountUiSkinPerson,110,0,1
 stdcall CheckPixel,25,1,0FFFFCC00h
 stdcall CheckPixel,45,20,0FFFF0000h
 stdcall CheckPixel,35,70,0FF00FF00h
 stdcall CheckPixel,1,55,0FF800000h
 stdcall CheckPixel,73,55,0FFFF00FFh
 stdcall CheckPixel,135,20,0FF0000FFh
 stdcall CheckPixel,183,55,0FF800000h
 stdcall CheckPixel,0,0,0
 stdcall CanvasEnd
 inc [step]
 ; Slim changes the actual arm projection instead of stretching a classic body.
 stdcall ClickAccount,950,300
 cmp [accountUiModel],1
 jne failed
 cmp [skinLocalModel],1
 jne failed
 cmp [accountUiImageModel],1
 jne failed
 stdcall CanvasBegin
 stdcall AccountUiSkinPerson,0,0,0
 stdcall CheckPixel,1,55,0
 stdcall CheckPixel,7,55,0FF800000h
 stdcall CanvasEnd
 stdcall Put,fixtureLocal,png64,png64Bytes
 inc [step]
 ; Refresh changes the shown source only after the worker publishes a result.
 mov [authReady],1
 stdcall AccountUiRemote,1
 cmp [accountUiImage],0
 jne failed
 cmp [workerCalls],2
 jne failed
 stdcall AccountSkinWorker,0
 mov [busy],0
 stdcall AccountFinishUI
 cmp [accountUiPreviewLocal],0
 jne failed
 cmp [accountUiImage],0
 je failed
 cmp [refreshCalls],1
 jne failed
 mov [chooserResult],0
 stdcall AccountUiExport
 cmp [exportCalls],0
 jne failed
 mov [chooserResult],1
 stdcall AccountUiExport
 cmp [exportCalls],1
 jne failed
 invoke GetFileAttributesW,fixtureExport
 cmp eax,-1
 je failed
 inc [step]
 ; Upload requires a verified local draft and forwards its explicit model.
 mov [skinLocalReady],0
 stdcall AccountUiRemote,3
 cmp [workerCalls],2
 jne failed
 mov [skinLocalReady],1
 stdcall AccountUiRemote,3
 cmp [workerCalls],3
 jne failed
 stdcall AccountSkinWorker,0
 cmp [lastModel],1
 jne failed
 cmp [uploadCalls],1
 jne failed
 mov [busy],0
 stdcall AccountFinishUI
 cmp [accountUiImageModel],1
 jne failed
 inc [step]
 ; Busy skin actions cancel only the owned skin request and block other inputs.
 stdcall AccountUiRemote,4
 cmp [workerCalls],4
 jne failed
 stdcall AccountUiChooseSkin
 stdcall AccountUiExport
 stdcall AccountUiLogout
 stdcall ClickAccount,320,140
 cmp [accountUiTab],1
 jne failed
 cmp [logoutCalls],0
 jne failed
 stdcall ClickAccount,740,430
 cmp [engineCancel],1
 jne failed
 cmp [modCancel],1
 jne failed
 cmp [authCancel],1
 jne failed
 stdcall AccountSkinWorker,0
 cmp [resetCalls],1
 jne failed
 mov [busy],0
 stdcall AccountFinishUI
 mov [engineCancel],0
 mov [modCancel],0
 mov [authCancel],0
 inc [step]
 ; Legacy 64x32 left limbs are mirrored, with no nonexistent lower overlays.
 invoke lstrcpyW,fixtureChosen,fixtureLegacy
 stdcall AccountUiChooseSkin
 stdcall AccountSkinWorker,0
 mov [busy],0
 stdcall AccountFinishUI
 cmp [accountUiImageHeight],32
 jne failed
 cmp [accountUiModel],0
 jne failed
 cmp [skinLocalModel],0
 jne failed
 cmp [accountUiImageModel],0
 jne failed
 ; Slim is unavailable for the legacy atlas, including the clickable model tile.
 stdcall ClickAccount,950,300
 cmp [accountUiModel],0
 jne failed
 cmp [skinLocalModel],0
 jne failed
 cmp [accountUiImageModel],0
 jne failed
 invoke GdipBitmapGetPixel,[accountUiImage],47,21,pixel
 mov [expectedColor],0FF008080h
 cmp [pixel],0FF008080h
 jne failed
 stdcall CanvasBegin
 stdcall AccountUiSkinRect,110,48,24,72,44,20,4,12,1
 stdcall CheckPixel,111,55,0FF008080h
 ; The renderer also defends against stale model metadata on a legacy image.
 mov [accountUiImageModel],1
 stdcall AccountUiSkinPerson,0,0,0
 stdcall CheckPixel,1,55,0FF800000h
 stdcall CheckPixel,73,55,0FF008080h
 stdcall CheckPixel,35,70,0FF00FF00h
 stdcall CanvasEnd
 ; A remote legacy texture also renders classic even if its profile says Slim.
 invoke lstrcpyW,skinCurrentPathW,fixtureLegacy
 mov [skinCurrentReady],1
 mov [skinModel],1
 mov [accountUiPreviewLocal],0
 stdcall AccountUiLoadImage
 cmp [accountUiImageHeight],32
 jne failed
 cmp [accountUiImageModel],0
 jne failed
 stdcall CanvasBegin
 stdcall AccountUiSkinPerson,0,0,0
 stdcall CheckPixel,1,55,0FF800000h
 stdcall CheckPixel,73,55,0FF008080h
 stdcall CanvasEnd
 mov [accountUiPreviewLocal],1
 stdcall AccountUiLoadImage
 inc [step]
 ; Rejecting a new local file cannot replace the previous valid preview.
 mov [selectFail],1
 stdcall AccountUiChooseSkin
 stdcall AccountSkinWorker,0
 mov [busy],0
 stdcall AccountFinishUI
 cmp [accountUiPreviewLocal],1
 jne failed
 cmp [accountUiImageHeight],32
 jne failed
 cmp [accountUiImage],0
 je failed
 mov [selectFail],0
 inc [step]
 ; Sign-in owns its worker, clears old codes, and allows verified device navigation.
 mov [accountUiTab],0
 invoke lstrcpyA,authUserCode,fixtureCode
 stdcall AccountUiLogin
 cmp [lastWorker],AccountLoginWorker
 jne failed
 cmp [accountUiAuthPending],1
 jne failed
 cmp [onlineMode],1
 jne failed
 cmp byte [authUserCode],0
 jne failed
 stdcall AccountUiCopyCode
 stdcall AccountUiVerify
 cmp [browserCalls],0
 jne failed
 invoke lstrcpyA,authUserCode,fixtureCode
 stdcall AccountUiCode
 test eax,eax
 jz failed
 invoke lstrcmpiW,accountUiCodeW,fixtureCodeW
 test eax,eax
 jnz failed
 stdcall AccountUiVerify
 cmp [browserCalls],1
 jne failed
 stdcall ClickAccount,320,348
 cmp [authCancel],1
 jne failed
 mov [busy],0
 stdcall AccountFinishUI
 cmp [accountUiAuthPending],0
 jne failed
 inc [step]
 ; Sign-out disposes pixels before clearing owned backend files and identity.
 stdcall AccountUiLogout
 cmp [logoutCalls],1
 jne failed
 cmp [clearCalls],1
 jne failed
 cmp [authReady],0
 jne failed
 cmp [accountUiImage],0
 jne failed
 cmp [skinCurrentReady],0
 jne failed
 cmp [skinLocalReady],0
 jne failed
 invoke GetWindowTextW,[hAccountUuid],textW,256
 test eax,eax
 jnz failed
 cmp [imageHeldAtStart],0
 jne failed
 inc [step]
 ; Unknown actions still produce one completion and never reuse stale success.
 mov eax,[postCalls]
 inc eax
 mov [pixel],eax
 mov [skinAction],0
 mov [accountUiSkinResult],1
 stdcall AccountSkinWorker,0
 cmp [accountUiSkinResult],0
 jne failed
 mov eax,[pixel]
 cmp eax,[postCalls]
 jne failed
 stdcall AccountUiReleaseImage
 invoke GdiplusShutdown,[gdipToken]
 invoke ExitProcess,0
failed:
 cinvoke wsprintfA,textA,failureFormat,[step],[checkpoint],[pixel],[expectedColor]
 invoke GetStdHandle,-11
 mov ebx,eax
 invoke lstrlenA,textA
 invoke WriteFile,ebx,textA,eax,pixel,0
 mov eax,[step]
 imul eax,100
 add eax,[checkpoint]
 invoke ExitProcess,eax
proc FixturePath,dest,suffix
 invoke lstrcpyW,[dest],fixtureRoot
 invoke lstrcatW,[dest],[suffix]
 ret
endp
proc Put uses ebx,path,data,bytes
 local written:DWORD
 invoke CreateFileW,[path],GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je failed
 mov ebx,eax
 invoke WriteFile,ebx,[data],[bytes],addr written,0
 test eax,eax
 jz failed
 invoke CloseHandle,ebx
 ret
endp
proc CanvasBegin
 invoke GdipCreateBitmapFromScan0,220,210,0,26200Ah,0,canvas
 test eax,eax
 jnz failed
 invoke GdipGetImageGraphicsContext,[canvas],gfx
 test eax,eax
 jnz failed
 invoke GdipSetInterpolationMode,[gfx],5
 ret
endp
proc CanvasEnd
 invoke GdipDeleteGraphics,[gfx]
 mov [gfx],0
 invoke GdipDisposeImage,[canvas]
 mov [canvas],0
 ret
endp
proc CheckPixel,x,y,expected
 mov eax,[x]
 imul eax,1000
 add eax,[y]
 mov [checkpoint],eax
 mov eax,[expected]
 mov [expectedColor],eax
 invoke GdipBitmapGetPixel,[canvas],[x],[y],pixel
 test eax,eax
 jnz failed
 mov eax,[pixel]
 cmp eax,[expected]
 jne failed
 ret
endp
proc NewControl,cls,style,id,x,y,w,h
 mov eax,[style]
 or eax,WS_POPUP
 invoke CreateWindowExW,0,[cls],emptyW,eax,[x],[y],[w],[h],0,0,0,0
 ret
endp
proc Scale,value
 mov eax,[value]
 ret
endp
proc SyncPage
 stdcall HideAccountControls
 stdcall ShowAccountControls
 ret
endp
proc StartTransition
 ret
endp
proc StartWorker,callback
 inc [workerCalls]
 mov eax,[callback]
 mov [lastWorker],eax
 mov [busy],1
 cmp [accountUiImage],0
 je .done
 mov [imageHeldAtStart],1
.done:
 ret
endp
proc AccountLoginWorker,param
 ret
endp
proc AuthLogout
 inc [logoutCalls]
 mov [authReady],0
 mov byte [authName],0
 mov byte [authUuid],0
 ret
endp
proc AuthVerifyBrowser
 invoke lstrcpyW,authVerifyUri,fixtureVerify
 mov eax,1
 ret
endp
proc SkinClear
 inc [clearCalls]
 mov [skinCurrentReady],0
 mov [skinLocalReady],0
 mov [skinLocalHeight],0
 mov word [skinCurrentPathW],0
 mov word [skinLocalPathW],0
 ret
endp
proc SkinSelectLocal,path
 inc [selectCalls]
 cmp [selectFail],1
 je .bad
 invoke lstrcpyW,skinLocalPathW,[path]
 mov [skinLocalHeight],64
 invoke lstrcmpiW,[path],fixtureLegacy
 test eax,eax
 jnz @f
 mov [skinLocalHeight],32
@@:
 mov [skinLocalReady],1
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc SkinRefresh
 inc [refreshCalls]
 mov [skinCurrentReady],1
 mov [skinModel],0
 invoke lstrcpyW,skinCurrentPathW,fixtureCurrent
 mov eax,1
 ret
endp
proc SkinUpload,model
 inc [uploadCalls]
 mov eax,[model]
 mov [lastModel],eax
 mov [skinModel],eax
 mov [skinCurrentReady],1
 invoke lstrcpyW,skinCurrentPathW,fixtureCurrent
 mov eax,1
 ret
endp
proc SkinReset
 inc [resetCalls]
 xor eax,eax
 ret
endp
proc SkinExportCurrent,path,replaceAllowed
 inc [exportCalls]
 invoke CopyFileW,skinCurrentPathW,[path],0
 ret
endp
proc FixtureChoose uses esi,ofn
 inc [chooserCalls]
 cmp [chooserResult],0
 je .cancel
 mov esi,[ofn]
 invoke lstrcpyW,[esi+OPENFILENAME.lpstrFile],fixtureChosen
 mov eax,1
 ret
.cancel:
 xor eax,eax
 ret
endp
proc FixtureSave uses esi,ofn
 cmp [chooserResult],0
 je .cancel
 mov esi,[ofn]
 invoke lstrcpyW,[esi+OPENFILENAME.lpstrFile],fixtureExport
 mov eax,1
 ret
.cancel:
 xor eax,eax
 ret
endp
proc FixturePost,window,message,wparam,lparam
 inc [postCalls]
 mov eax,1
 ret
endp
proc FixtureBrowser,window,verb,path,params,dir,show
 inc [browserCalls]
 invoke lstrcmpiW,[path],fixtureVerify
 test eax,eax
 jnz failed
 mov eax,33
 ret
endp
define GetOpenFileNameW openPointer
define GetSaveFileNameW savePointer
define PostMessageW postPointer
define ShellExecuteW shellPointer
include '..\src\account_ui.inc'
restore GetOpenFileNameW
restore GetSaveFileNameW
restore PostMessageW
restore ShellExecuteW
section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL',user32,'USER32.DLL',gdiplus,'GDIPLUS.DLL'
import kernel32,ExitProcess,'ExitProcess',GetStdHandle,'GetStdHandle',lstrlenA,'lstrlenA',lstrcpyW,'lstrcpyW',lstrcpyA,'lstrcpyA',lstrcmpiW,'lstrcmpiW',lstrcmpA,'lstrcmpA',lstrcatW,'lstrcatW',lstrlenW,'lstrlenW',\
 CreateDirectoryW,'CreateDirectoryW',CreateFileW,'CreateFileW',WriteFile,'WriteFile',CloseHandle,'CloseHandle',CopyFileW,'CopyFileW',GetFileAttributesW,'GetFileAttributesW',DeleteFileW,'DeleteFileW',GetTickCount,'GetTickCount',\
 MultiByteToWideChar,'MultiByteToWideChar',GlobalAlloc,'GlobalAlloc',GlobalLock,'GlobalLock',GlobalUnlock,'GlobalUnlock',GlobalFree,'GlobalFree'
import user32,CreateWindowExW,'CreateWindowExW',ShowWindow,'ShowWindow',SetWindowPos,'SetWindowPos',GetWindowRect,'GetWindowRect',GetWindowLongW,'GetWindowLongW',SendMessageW,'SendMessageW',SetWindowTextW,'SetWindowTextW',SetWindowTextA,'SetWindowTextA',\
 GetWindowTextW,'GetWindowTextW',GetWindowTextA,'GetWindowTextA',InvalidateRect,'InvalidateRect',EnableWindow,'EnableWindow',wsprintfW,'wsprintfW',wsprintfA,'wsprintfA',OpenClipboard,'OpenClipboard',EmptyClipboard,'EmptyClipboard',SetClipboardData,'SetClipboardData',CloseClipboard,'CloseClipboard'
import gdiplus,GdiplusStartup,'GdiplusStartup',GdiplusShutdown,'GdiplusShutdown',GdipCreateBitmapFromFile,'GdipCreateBitmapFromFile',GdipGetImageWidth,'GdipGetImageWidth',GdipGetImageHeight,'GdipGetImageHeight',GdipCloneBitmapAreaI,'GdipCloneBitmapAreaI',GdipImageRotateFlip,'GdipImageRotateFlip',GdipDisposeImage,'GdipDisposeImage',\
 GdipCreateBitmapFromScan0,'GdipCreateBitmapFromScan0',GdipGetImageGraphicsContext,'GdipGetImageGraphicsContext',GdipSetInterpolationMode,'GdipSetInterpolationMode',GdipDeleteGraphics,'GdipDeleteGraphics',GdipBitmapGetPixel,'GdipBitmapGetPixel',GdipDrawImagePointsRectI,'GdipDrawImagePointsRectI'
