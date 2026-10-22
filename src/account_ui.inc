; Native account / skin workspace. Preview never changes the remote account.
section '.accuid' data readable writeable
accountUiTab dd 0
accountUiAdvanced dd 0
accountUiAuthPending dd 0
accountUiPreviewLocal dd 0
accountUiModel dd 0
accountUiBeforePreview dd 0
accountUiSkinResult dd 0
accountUiImage dd 0
accountUiMirrorImage dd 0
accountUiImageHeight dd 0
accountUiImageModel dd 0
hAccountUuid dd 0
accountUiUuidLabel du 'UUID',0
accountUiPngFilter du 'PNG (*.png)',0,'*.png',0,0
accountUiPngExt du 'png',0
accountUiPngName du 'Minecraft-skin.png',0
accountUiDialog OPENFILENAME
align 4
accountUiPickW rw 2048
accountUiExportW rw 2048
accountUiNameW rw 64
accountUiUuidW rw 64
accountUiCodeW rw 64

section '.accuic' code readable executable
 nop
proc CreateAccountControls
 stdcall NewControl,editClass,ES_AUTOHSCROLL+ES_READONLY,801,336,276,714,24
 mov [hAccountUuid],eax
 invoke SendMessageW,eax,EM_SETLIMITTEXT,63,0
 ret
endp
proc HideAccountControls
 invoke ShowWindow,[hAccountUuid],SW_HIDE
 invoke ShowWindow,[hClient],SW_HIDE
 ret
endp
proc EnableAccountControls,enabled
 invoke EnableWindow,[hAccountUuid],[enabled]
 invoke EnableWindow,[hClient],[enabled]
 ret
endp
proc AccountUiIdentity
 mov word [accountUiNameW],0
 mov word [accountUiUuidW],0
 cmp [authReady],1
 jne .show
 invoke MultiByteToWideChar,65001,8,authName,-1,accountUiNameW,64
 invoke MultiByteToWideChar,65001,8,authUuid,-1,accountUiUuidW,64
.show:
 invoke SetWindowTextW,[hAccountUuid],accountUiUuidW
 ret
endp
proc ShowAccountControls
 locals
  x dd ?
  y dd ?
  w dd ?
  h dd ?
 endl
 stdcall AccountUiIdentity
 cmp [accountUiTab],0
 jne .done
 cmp [authReady],1
 jne .advanced
 invoke ShowWindow,[hAccountUuid],SW_SHOW
.advanced:
 cmp [accountUiAdvanced],1
 jne .done
 mov eax,480
 add eax,[contentX]
 stdcall Scale,eax
 mov [x],eax
 stdcall Scale,554
 mov [y],eax
 stdcall Scale,574
 mov [w],eax
 stdcall Scale,24
 mov [h],eax
 invoke SetWindowPos,[hClient],0,[x],[y],[w],[h],SWP_NOZORDER+SWP_NOACTIVATE
 invoke ShowWindow,[hClient],SW_SHOW
.done:
 ret
endp
proc AccountCommand,wparam
 ; The UUID is read-only. Advanced client-ID edits commit through PullSettings.
 ret
endp
proc AccountUiReleaseImage
 cmp [accountUiMirrorImage],0
 je @f
 invoke GdipDisposeImage,[accountUiMirrorImage]
 mov [accountUiMirrorImage],0
@@:
 cmp [accountUiImage],0
 je .done
 invoke GdipDisposeImage,[accountUiImage]
 mov [accountUiImage],0
.done:
 mov [accountUiImageHeight],0
 ret
endp
; Copy into an independent memory bitmap; GDI+ clones can retain source streams.
proc AccountUiLoadImage uses ebx
 locals
  source dd ?
  clone dd ?
  width dd ?
  height dd ?
  copyGraphics dd ?
  points rd 6
 endl
 cmp [busy],0
 jne .done
 stdcall AccountUiReleaseImage
 mov [source],0
 mov [clone],0
 mov [copyGraphics],0
 mov eax,skinCurrentPathW
 mov ebx,[skinModel]
 cmp [accountUiPreviewLocal],1
 je .local
 cmp [skinCurrentReady],1
 jne .done
 jmp .open
.local:
 mov eax,skinLocalPathW
 mov ebx,[accountUiModel]
 cmp [skinLocalReady],1
 jne .done
.open:
 invoke GdipCreateBitmapFromFile,eax,addr source
 test eax,eax
 jnz .close
 cmp [source],0
 je .close
 invoke GdipGetImageWidth,[source],addr width
 test eax,eax
 jnz .close
 invoke GdipGetImageHeight,[source],addr height
 test eax,eax
 jnz .close
 cmp [width],64
 jne .close
 cmp [height],64
 je .clone
 cmp [height],32
 jne .close
.clone:
 cmp [height],32
 jne @f
 xor ebx,ebx
 cmp [accountUiPreviewLocal],1
 jne @f
 mov [accountUiModel],0
 mov [skinLocalModel],0
@@:
 invoke GdipCreateBitmapFromScan0,64,[height],0,26200Ah,0,addr clone
 test eax,eax
 jnz .close
 invoke GdipGetImageGraphicsContext,[clone],addr copyGraphics
 test eax,eax
 jnz .close
 mov [points],0
 mov [points+4],0
 mov [points+8],64
 mov [points+12],0
 mov [points+16],0
 mov eax,[height]
 mov [points+20],eax
 invoke GdipSetInterpolationMode,[copyGraphics],5
 invoke GdipDrawImagePointsRectI,[copyGraphics],[source],addr points,3,0,0,64,[height],2,0,0,0
 test eax,eax
 jnz .close
 mov eax,[clone]
 mov [accountUiImage],eax
 mov [clone],0
 mov eax,[height]
 mov [accountUiImageHeight],eax
 mov [accountUiImageModel],ebx
 cmp [height],32
 jne .close
 invoke GdipCloneBitmapAreaI,0,0,64,32,26200Ah,[accountUiImage],accountUiMirrorImage
 test eax,eax
 jnz .mirrorFailed
 invoke GdipImageRotateFlip,[accountUiMirrorImage],4
 test eax,eax
 jz .close
.mirrorFailed:
 stdcall AccountUiReleaseImage
.close:
 cmp [copyGraphics],0
 je @f
 invoke GdipDeleteGraphics,[copyGraphics]
@@:
 cmp [clone],0
 je @f
 invoke GdipDisposeImage,[clone]
@@:
 cmp [source],0
 je .done
 invoke GdipDisposeImage,[source]
.done:
 ret
endp
proc AccountFinishUI
 cmp [busy],0
 jne .done
 mov [accountUiAuthPending],0
 cmp [skinAction],2
 jne .remote
 cmp [accountUiSkinResult],1
 jne .selectionFailed
 cmp [skinLocalReady],1
 jne .selectionFailed
 cmp [skinLocalHeight],32
 jne @f
 mov [accountUiModel],0
 mov [skinLocalModel],0
@@:
 mov [accountUiPreviewLocal],1
 jmp .finished
.selectionFailed:
 mov eax,[accountUiBeforePreview]
 mov [accountUiPreviewLocal],eax
 jmp .finished
.remote:
 cmp [skinAction],0
 je .available
 cmp [accountUiSkinResult],1
 jne .available
 mov [accountUiPreviewLocal],0
 cmp [skinCurrentReady],1
 je .finished
 cmp [skinLocalReady],1
 jne .finished
 mov [accountUiPreviewLocal],1
 jmp .finished
.available:
 cmp [skinLocalReady],1
 je .finished
 mov [accountUiPreviewLocal],0
.finished:
 mov [skinAction],0
 mov [accountUiSkinResult],0
 stdcall AccountUiIdentity
 stdcall AccountUiLoadImage
 cmp [currentPage],2
 jne .done
 stdcall SyncPage
.done:
 ret
endp
proc AccountSkinWorker,param
 mov [accountUiSkinResult],0
 cmp [skinAction],1
 je .refresh
 cmp [skinAction],2
 je .select
 cmp [skinAction],3
 je .upload
 cmp [skinAction],4
 jne .finish
 stdcall SkinReset
 jmp .result
.refresh:
 stdcall SkinRefresh
 jmp .result
.select:
 stdcall SkinSelectLocal,accountUiPickW
 jmp .result
.upload:
 stdcall SkinUpload,[accountUiModel]
.result:
 mov [accountUiSkinResult],eax
.finish:
 invoke PostMessageW,[hWnd],WM_FINISHED,0,0
 ret
endp
proc AccountUiStartSkin,action
 cmp [busy],0
 jne .done
 mov eax,[accountUiPreviewLocal]
 mov [accountUiBeforePreview],eax
 stdcall AccountUiReleaseImage
 mov [accountUiSkinResult],0
 mov eax,[action]
 mov [skinAction],eax
 mov [accountUiAuthPending],0
 stdcall StartWorker,AccountSkinWorker
.done:
 ret
endp
proc AccountUiLogin
 cmp [busy],0
 jne .done
 stdcall AccountUiReleaseImage
 invoke SetWindowTextW,[hAccountUuid],emptyW
 invoke ShowWindow,[hAccountUuid],SW_HIDE
 mov byte [authUserCode],0
 mov [skinAction],0
 mov [accountUiAuthPending],1
 mov [onlineMode],1
 stdcall StartWorker,AccountLoginWorker
.done:
 ret
endp
proc AccountUiLogout
 cmp [busy],0
 jne .done
 stdcall AccountUiReleaseImage
 stdcall AuthLogout
 stdcall SkinClear
 mov [accountUiAuthPending],0
 mov [accountUiPreviewLocal],0
 stdcall AccountUiIdentity
 stdcall SyncPage
.done:
 ret
endp
proc AccountUiCode
 mov word [accountUiCodeW],0
 cmp [accountUiAuthPending],1
 jne .bad
 cmp [busy],1
 jne .bad
 cmp byte [authUserCode],0
 je .bad
 invoke MultiByteToWideChar,65001,8,authUserCode,-1,accountUiCodeW,64
 ret
.bad:
 xor eax,eax
 ret
endp
proc AccountUiCopyCode uses ebx esi
 local memory:DWORD
 stdcall AccountUiCode
 test eax,eax
 jz .done
 invoke lstrlenW,accountUiCodeW
 inc eax
 shl eax,1
 invoke GlobalAlloc,2,eax
 test eax,eax
 jz .failed
 mov [memory],eax
 invoke GlobalLock,eax
 test eax,eax
 jz .free
 invoke lstrcpyW,eax,accountUiCodeW
 invoke GlobalUnlock,[memory]
 invoke OpenClipboard,[hWnd]
 test eax,eax
 jz .free
 invoke EmptyClipboard
 test eax,eax
 jz .close
 invoke SetClipboardData,13,[memory]
 test eax,eax
 jz .close
 invoke CloseClipboard
 invoke lstrcpyW,statusText,sAccountCopied
 jmp .redraw
.close:
 invoke CloseClipboard
.free:
 invoke GlobalFree,[memory]
.failed:
 invoke lstrcpyW,statusText,sAccountCopyFailed
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc AccountUiVerify
 stdcall AccountUiCode
 test eax,eax
 jz .done
 stdcall AuthVerifyBrowser
 test eax,eax
 jz .done
 invoke ShellExecuteW,[hWnd],openVerb,authVerifyUri,0,0,SW_SHOWNORMAL
.done:
 ret
endp
proc AccountUiChooseSkin uses edi
 cmp [busy],0
 jne .done
 mov word [accountUiPickW],0
 mov edi,accountUiDialog
 xor eax,eax
 mov ecx,sizeof.OPENFILENAME/4
 rep stosd
 mov [accountUiDialog.lStructSize],sizeof.OPENFILENAME
 mov eax,[hWnd]
 mov [accountUiDialog.hwndOwner],eax
 mov [accountUiDialog.lpstrFilter],accountUiPngFilter
 mov [accountUiDialog.lpstrFile],accountUiPickW
 mov [accountUiDialog.nMaxFile],2048
 mov [accountUiDialog.Flags],OFN_FILEMUSTEXIST+OFN_PATHMUSTEXIST+OFN_NOCHANGEDIR+OFN_EXPLORER
 invoke GetOpenFileNameW,accountUiDialog
 test eax,eax
 jz .done
 stdcall AccountUiStartSkin,2
.done:
 ret
endp
proc AccountUiRemote,action
 cmp [busy],0
 jne .done
 cmp [authReady],1
 jne .login
 cmp [action],3
 jne .start
 cmp [skinLocalReady],1
 jne .local
.start:
 stdcall AccountUiStartSkin,[action]
 ret
.login:
 invoke lstrcpyW,statusText,sSkinNeedLogin
 jmp .redraw
.local:
 invoke lstrcpyW,statusText,sSkinNeedLocal
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc AccountUiExport uses edi
 cmp [busy],0
 jne .done
 cmp [skinCurrentReady],1
 jne .empty
 invoke lstrcpyW,accountUiExportW,accountUiPngName
 mov edi,accountUiDialog
 xor eax,eax
 mov ecx,sizeof.OPENFILENAME/4
 rep stosd
 mov [accountUiDialog.lStructSize],sizeof.OPENFILENAME
 mov eax,[hWnd]
 mov [accountUiDialog.hwndOwner],eax
 mov [accountUiDialog.lpstrFilter],accountUiPngFilter
 mov [accountUiDialog.lpstrFile],accountUiExportW
 mov [accountUiDialog.nMaxFile],2048
 mov [accountUiDialog.lpstrDefExt],accountUiPngExt
 mov [accountUiDialog.Flags],OFN_PATHMUSTEXIST+OFN_NOCHANGEDIR+OFN_EXPLORER+OFN_OVERWRITEPROMPT
 invoke GetSaveFileNameW,accountUiDialog
 test eax,eax
 jz .done
 stdcall SkinExportCurrent,accountUiExportW,1
 test eax,eax
 jz .failed
 invoke lstrcpyW,statusText,sSkinExported
 jmp .redraw
.failed:
 invoke lstrcpyW,statusText,sSkinExportFailed
 jmp .redraw
.empty:
 invoke lstrcpyW,statusText,sSkinEmpty
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
; Integer nearest-neighbor atlas rectangles. Mirroring handles old 64x32 limbs.
proc AccountUiSkinRect uses ebx,x,y,w,h,sx,sy,sw,sh,mirror
 locals
  points rd 6
  image dd ?
  sourceX dd ?
 endl
 mov eax,[accountUiImage]
 mov [image],eax
 mov eax,[sx]
 mov [sourceX],eax
 cmp [mirror],0
 je @f
 mov eax,[accountUiMirrorImage]
 mov [image],eax
 mov eax,64
 sub eax,[sx]
 sub eax,[sw]
 mov [sourceX],eax
@@:
 mov eax,[x]
 add eax,[gOffX]
 mov ebx,[w]
 add ebx,eax
 mov [points],eax
 mov [points+8],ebx
 mov [points+16],eax
 mov eax,[y]
 add eax,[gOffY]
 mov [points+4],eax
 mov [points+12],eax
 add eax,[h]
 mov [points+20],eax
 invoke GdipDrawImagePointsRectI,[gfx],[image],addr points,3,[sourceX],[sy],[sw],[sh],2,0,0,0
 ret
endp
; Flat front / back projections of the original atlas, including alpha overlays.
proc AccountUiSkinPerson uses ebx esi edi,x,y,back
 locals
  head dd ?
  body dd ?
  arm dd ?
  armWidth dd ?
  rightX dd ?
  leftX dd ?
  rightLeg dd ?
  leftLeg dd ?
  leftSource dd ?
  leftY dd ?
  mirror dd ?
 endl
 mov [head],8
 mov [body],20
 mov [arm],44
 mov [armWidth],4
 mov [rightLeg],4
 mov [leftLeg],20
 cmp [accountUiImageHeight],32
 je @f
 cmp [accountUiImageModel],1
 jne @f
 mov [armWidth],3
@@:
 cmp [back],0
 je .front
 mov [head],24
 mov [body],32
 mov [rightLeg],12
 mov [leftLeg],28
 mov eax,[armWidth]
 add eax,48
 mov [arm],eax
.front:
 mov eax,[x]
 add eax,24
 stdcall AccountUiSkinRect,eax,[y],48,48,[head],8,8,8,0
 mov eax,[y]
 add eax,48
 mov ebx,[x]
 add ebx,24
 stdcall AccountUiSkinRect,ebx,eax,48,72,[body],20,8,12,0
 mov ebx,[x]
 mov edi,[x]
 add edi,72
 cmp [armWidth],4
 je @f
 add ebx,6
@@:
 cmp [back],0
 je @f
 xchg ebx,edi
@@:
 mov [rightX],ebx
 mov [leftX],edi
 mov eax,[armWidth]
 imul eax,6
 mov esi,[y]
 add esi,48
 stdcall AccountUiSkinRect,[rightX],esi,eax,72,[arm],20,[armWidth],12,0
 mov [leftSource],36
 mov [leftY],52
 mov [mirror],0
 cmp [back],0
 je @f
 mov eax,[armWidth]
 add eax,40
 mov [leftSource],eax
@@:
 cmp [accountUiImageHeight],64
 je @f
 mov eax,[arm]
 mov [leftSource],eax
 mov [leftY],20
 mov [mirror],1
@@:
 mov eax,[armWidth]
 imul eax,6
 stdcall AccountUiSkinRect,[leftX],esi,eax,72,[leftSource],[leftY],[armWidth],12,[mirror]
 mov ebx,[x]
 add ebx,24
 mov edi,ebx
 add edi,24
 cmp [back],0
 je @f
 xchg ebx,edi
@@:
 mov [rightX],ebx
 mov [leftX],edi
 mov esi,[y]
 add esi,120
 stdcall AccountUiSkinRect,ebx,esi,24,72,[rightLeg],20,4,12,0
 mov eax,[leftLeg]
 mov edx,52
 cmp [accountUiImageHeight],64
 je @f
 mov eax,[rightLeg]
 mov edx,20
@@:
 stdcall AccountUiSkinRect,[leftX],esi,24,72,eax,edx,4,12,[mirror]
 ; Hat layer is also present in legacy skins.
 mov eax,[head]
 add eax,32
 mov ebx,[x]
 add ebx,24
 stdcall AccountUiSkinRect,ebx,[y],48,48,eax,8,8,8,0
 cmp [accountUiImageHeight],64
 jne .done
 mov ebx,[x]
 add ebx,24
 mov esi,[y]
 add esi,48
 stdcall AccountUiSkinRect,ebx,esi,48,72,[body],36,8,12,0
 mov eax,[x]
 mov edi,[x]
 add edi,72
 cmp [armWidth],4
 je @f
 add eax,6
@@:
 cmp [back],0
 je @f
 xchg eax,edi
@@:
 mov [rightX],eax
 mov [leftX],edi
 mov ebx,[armWidth]
 imul ebx,6
 stdcall AccountUiSkinRect,[rightX],esi,ebx,72,[arm],36,[armWidth],12,0
 mov eax,[leftSource]
 add eax,16
 stdcall AccountUiSkinRect,[leftX],esi,ebx,72,eax,52,[armWidth],12,0
 mov ebx,[x]
 add ebx,24
 mov edi,ebx
 add edi,24
 cmp [back],0
 je @f
 xchg ebx,edi
@@:
 mov esi,[y]
 add esi,120
 stdcall AccountUiSkinRect,ebx,esi,24,72,[rightLeg],36,4,12,0
 mov eax,[leftLeg]
 sub eax,16
 stdcall AccountUiSkinRect,edi,esi,24,72,eax,52,4,12,0
.done:
 ret
endp
proc PaintAccount,dc
 local state:DWORD
 stdcall PageHeader,eyebrowAccount,sAccountHeading,3
 mov [segLabels],sAccount
 mov [segLabels+4],sAccountSkin
 stdcall Segmented,252,124,450,42,2,2,segLabels,[accountUiTab],[fBody]
 cmp [accountUiTab],1
 je .skin
 stdcall Card,252,185,836,428,18
 mov eax,sNoAccount
 cmp [authReady],1
 jne @f
 mov eax,accountUiNameW
@@:
 stdcall Text,[dc],eax,280,216,440,40,[fHeading],T_INK
 mov [segLabels],sOffline
 mov [segLabels+4],sOnline
 stdcall Segmented,744,210,320,40,2,6,segLabels,[onlineMode],[fBody]
 cmp [authReady],1
 jne .accountButtons
 stdcall Text,[dc],accountUiUuidLabel,280,276,56,24,[fSmall],T_MUTED
.accountButtons:
 mov eax,sLogin
 cmp [accountUiAuthPending],1
 jne @f
 cmp [busy],0
 je @f
 mov eax,sCancelLogin
@@:
 stdcall Button,[dc],eax,280,326,200,44,1
 stdcall Button,[dc],sLogout,500,326,160,44,0
 stdcall AccountUiCode
 test eax,eax
 jz .advanced
 stdcall Text,[dc],sAccountCode,280,397,340,24,[fSmall],T_MUTED
 stdcall Text,[dc],accountUiCodeW,280,426,340,40,[fHeading],T_INK
 stdcall Button,[dc],sAccountCopy,660,424,160,40,0
 stdcall Button,[dc],sAccountVerify,840,424,224,40,0
.advanced:
 stdcall Button,[dc],sAccountAdvanced,280,546,180,40,[accountUiAdvanced]
 cmp [accountUiAdvanced],0
 je .footer
 stdcall Field,468,546,596,40,[hClient]
 jmp .footer
.skin:
 stdcall Card,252,185,400,428,18
 stdcall Card,672,185,416,428,18
 mov [segLabels],sSkinCurrent
 mov [segLabels+4],sSkinLocal
 stdcall Segmented,280,206,344,36,2,4,segLabels,[accountUiPreviewLocal],[fSmall]
 cmp [accountUiImage],0
 je .empty
 stdcall TextC,[dc],sSkinFront,296,260,104,24,[fSmall],T_MUTED
 stdcall TextC,[dc],sSkinBack,488,260,104,24,[fSmall],T_MUTED
 invoke GdipSaveGraphics,[gfx],addr state
 invoke GdipSetInterpolationMode,[gfx],5
 stdcall AccountUiSkinPerson,300,298,0
 stdcall AccountUiSkinPerson,488,298,1
 invoke GdipRestoreGraphics,[gfx],[state]
 jmp .skinActions
.empty:
 stdcall TextC,[dc],sSkinEmpty,280,358,344,44,[fMedium],T_MUTED
.skinActions:
 stdcall Button,[dc],sSkinChoose,696,210,368,44,1
 mov [segLabels],sSkinClassic
 mov [segLabels+4],sSkinSlim
 stdcall Segmented,696,282,368,40,2,7,segLabels,[accountUiModel],[fBody]
 stdcall TextDraw,sSkinLocalNote,696,344,368,50,[fSmall],T_MUTED,DT_LEFT+DT_WORDBREAK,0
 mov eax,sSkinApply
 cmp [busy],0
 je @f
 mov eax,sModCancel
@@:
 stdcall Button,[dc],eax,696,414,368,44,1
 stdcall Button,[dc],sRefresh,696,480,172,40,0
 stdcall Button,[dc],sSkinReset,888,480,176,40,0
 stdcall Button,[dc],sSkinExport,696,546,368,40,0
.footer:
 stdcall StatusLine,254,676,830,32
 stdcall BusyBar,254,664,834
 ret
endp
proc ClickAccount uses ebx,x,y
 ; Device-code copy / verification remain usable during this sign-in worker.
 cmp [accountUiTab],0
 jne .busy
 cmp [accountUiAuthPending],1
 jne .busy
 cmp [busy],1
 jne .busy
 cmp [y],424
 jb .cancel
 cmp [y],464
 ja .cancel
 cmp [x],660
 jb .done
 cmp [x],820
 jbe .copy
 cmp [x],840
 jb .done
 cmp [x],1064
 ja .done
 stdcall AccountUiVerify
 ret
.copy:
 stdcall AccountUiCopyCode
 ret
.cancel:
 cmp [y],326
 jb .done
 cmp [y],370
 ja .done
 cmp [x],280
 jb .done
 cmp [x],480
 ja .done
 mov [authCancel],1
 ret
.busy:
 cmp [busy],0
 je .tabs
 cmp [skinAction],0
 jne .busySkin
 cmp [y],124
 jb .done
 cmp [y],166
 ja .busySkin
 jmp .tabs
.busySkin:
 cmp [skinAction],0
 je .done
 ; Skin network work uses the common cancellation flags, never logout.
 cmp [accountUiTab],1
 jne .done
 cmp [y],414
 jb .done
 cmp [y],458
 ja .done
 cmp [x],696
 jb .done
 cmp [x],1064
 ja .done
 mov [modCancel],1
 mov [engineCancel],1
 mov [authCancel],1
 ret
.tabs:
 cmp [y],124
 jb .done
 cmp [y],166
 ja .body
 cmp [x],252
 jb .done
 cmp [x],702
 jae .done
 xor eax,eax
 cmp [x],477
 setae al
 cmp eax,[accountUiTab]
 je .done
 mov [accountUiTab],eax
 cmp eax,1
 jne @f
 stdcall AccountUiLoadImage
@@:
 stdcall StartTransition
 stdcall SyncPage
 ret
.body:
 cmp [busy],0
 jne .done
 cmp [accountUiTab],1
 je .skin
 cmp [y],210
 jb .accountButtons
 cmp [y],250
 ja .accountButtons
 cmp [x],744
 jb .done
 cmp [x],1064
 ja .done
 xor eax,eax
 cmp [x],904
 setae al
 mov [onlineMode],eax
 jmp .redraw
.accountButtons:
 cmp [y],326
 jb .advanced
 cmp [y],370
 ja .advanced
 cmp [x],280
 jb .done
 cmp [x],480
 jbe .login
 cmp [x],500
 jb .done
 cmp [x],660
 ja .done
 stdcall AccountUiLogout
 ret
.login:
 stdcall AccountUiLogin
 ret
.advanced:
 cmp [y],546
 jb .done
 cmp [y],586
 ja .done
 cmp [x],280
 jb .done
 cmp [x],460
 ja .done
 xor [accountUiAdvanced],1
 stdcall SyncPage
 ret
.skin:
 cmp [y],206
 jb .done
 cmp [y],242
 ja .skinActions
 cmp [x],280
 jb .done
 cmp [x],624
 jae .skinActions
 xor eax,eax
 cmp [x],452
 setae al
 mov [accountUiPreviewLocal],eax
 stdcall AccountUiLoadImage
 jmp .redraw
.skinActions:
 cmp [x],696
 jb .done
 cmp [x],1064
 ja .done
 cmp [y],210
 jb .done
 cmp [y],254
 jbe .choose
 cmp [y],282
 jb .done
 cmp [y],322
 jbe .model
 cmp [y],414
 jb .done
 cmp [y],458
 jbe .upload
 cmp [y],480
 jb .done
 cmp [y],520
 jbe .remote
 cmp [y],546
 jb .done
 cmp [y],586
 ja .done
 stdcall AccountUiExport
 ret
.choose:
 stdcall AccountUiChooseSkin
 ret
.model:
 xor eax,eax
 cmp [x],880
 setae al
 cmp [skinLocalReady],1
 jne @f
 cmp [skinLocalHeight],32
 jne @f
 xor eax,eax
@@:
 mov [accountUiModel],eax
 mov [skinLocalModel],eax
 cmp [accountUiPreviewLocal],1
 jne .redraw
 mov [accountUiImageModel],eax
 jmp .redraw
.upload:
 stdcall AccountUiRemote,3
 ret
.remote:
 cmp [x],868
 jbe .refresh
 cmp [x],888
 jb .done
 stdcall AccountUiRemote,4
 ret
.refresh:
 stdcall AccountUiRemote,1
 ret
.redraw:
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
