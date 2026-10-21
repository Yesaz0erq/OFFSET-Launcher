; OFFSET Launcher - original Win32/x86 assembly Microsoft authentication.
; Device grant -> Xbox Live -> XSTS -> Minecraft entitlement/profile.
; Credentials are session-only. No borrowed client IDs or plaintext token files.

section '.authd' data readable writeable
authCancel       dd 0
authDeadline     dd 0
authInterval     dd 5000
authContext      dd 0
authResponse     dd 0
authRootToken    dd 0
authObject       dd 0
authHttpCode     dd 0
authResult       dd 0
authVerifyAllowed1 db 'https://microsoft.com/devicelogin',0
authVerifyAllowed2 db 'https://www.microsoft.com/link',0
authVerifyAllowed3 db 'https://microsoft.com/link',0
authVerifyAllowed4 db 'https://www.microsoft.com/devicelogin',0
authOpen         du 'open',0
authPost         du 'POST',0
authFormHeader   du 'Content-Type: application/x-www-form-urlencoded',13,10,0
authJsonHeader   du 'Content-Type: application/json',13,10,0
authDeviceUrl    du 'https://login.microsoftonline.com/consumers/oauth2/v2.0/devicecode',0
authTokenUrl     du 'https://login.microsoftonline.com/consumers/oauth2/v2.0/token',0
authXblUrl       du 'https://user.auth.xboxlive.com/user/authenticate',0
authXstsUrl      du 'https://xsts.auth.xboxlive.com/xsts/authorize',0
authMinecraftUrl du 'https://api.minecraftservices.com/authentication/login_with_xbox',0
authEntitleUrl   du 'https://api.minecraftservices.com/entitlements/mcstore',0
authProfileUrl   du 'https://api.minecraftservices.com/minecraft/profile',0
authKDevice      db 'device_code',0
authKUser        db 'user_code',0
authKVerify      db 'verification_uri',0
authKExpires     db 'expires_in',0
authKInterval    db 'interval',0
authKAccess      db 'access_token',0
authKToken       db 'Token',0
authKClaims      db 'DisplayClaims',0
authKXui         db 'xui',0
authKUhs         db 'uhs',0
authKXid         db 'xid',0
authKError       db 'error',0
authKItems       db 'items',0
authKName        db 'name',0
authKId          db 'id',0
authKXerr        db 'XErr',0
authPending      db 'authorization_pending',0
authSlowDown     db 'slow_down',0
authDeclined     db 'authorization_declined',0
authExpired      db 'expired_token',0
authItemProduct  db 'product_minecraft',0
authItemGame     db 'game_minecraft',0
authHex          db '0123456789ABCDEF',0
authDevicePrefix db 'client_id=',0
authScope        db '&scope=XboxLive.signin',0
authPollPrefix   db 'grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code&client_id=',0
authPollDevice   db '&device_code=',0
authXblPrefix    db '{"Properties":{"AuthMethod":"RPS","SiteName":"user.auth.xboxlive.com","RpsTicket":"d=',0
authXblSuffix    db '"},"RelyingParty":"http://auth.xboxlive.com","TokenType":"JWT"}',0
authXstsPrefix   db '{"Properties":{"SandboxId":"RETAIL","UserTokens":["',0
authXstsSuffix   db '"]},"RelyingParty":"rp://api.minecraftservices.com/","TokenType":"JWT"}',0
authMcPrefix     db '{"identityToken":"XBL3.0 x=',0
authSemicolon    db ';',0
authJsonClose    db '"}',0
authBearer       db 'Authorization: Bearer ',0
authCrLf         db 13,10,0
authCodePrefix   db 'Open ',0
authCodeMiddle   db ' | Code: ',0
authSignedPrefix db 'Signed in as ',0
authMsgClient    db 'Microsoft sign-in needs your registered public-client Application ID on the Account page.',0
authMsgInvalidId db 'Application ID must be a GUID from your own Microsoft public-client registration.',0
authMsgStart     db 'Requesting a Microsoft device code...',0
authMsgDevice    db 'Microsoft device authorization failed. Check Application ID and public-client permissions.',0
authMsgToken     db 'Microsoft authorization failed or was declined. Start sign-in again to retry.',0
authMsgExpired   db 'Microsoft sign-in expired. Start sign-in again for a new device code.',0
authMsgCancel    db 'Microsoft sign-in cancelled.',0
authMsgNetwork   db 'Sign-in request failed. Check your network connection and try again.',0
authMsgFormat    db 'Sign-in server returned an invalid or oversized response.',0
authMsgXbox      db 'Microsoft authorized. Connecting to Xbox Live...',0
authMsgXboxFail  db 'Xbox Live sign-in failed. Ensure this Microsoft account has an Xbox profile.',0
authMsgXsts      db 'Verifying Xbox account with XSTS...',0
authMsgXstsFail  db 'XSTS denied sign-in. Check Xbox profile, family permissions, and account region.',0
authMsgMinecraft db 'Xbox authorized. Checking Minecraft ownership...',0
authMsgMcFail    db 'Minecraft authentication failed. Your application may require Minecraft API approval.',0
authMsgOwnFail   db 'This account has no active Minecraft Java entitlement. Check ownership or Game Pass status.',0
authMsgProfile   db 'This account has no usable Minecraft Java profile. Complete profile setup first.',0
authMsgLogout    db 'Signed out. Session credentials have been cleared from memory.',0

; Trailing uninitialized storage adds virtual memory, never zero-filled file bytes.
authDisplay      rb 256
authXuid         rb 64
authMsToken      rb 16384
authXblToken     rb 16384
authXstsToken    rb 16384
authMcToken      rb 8192
authDeviceCode   rb 8192
authUserCode     rb 64
authUhs          rb 128
authProfileName  rb 64
authProfileUuid  rb 64
authError        rb 256
authBody         rb 65536
authEscaped      rb 49152
authFormDevice   rb 24576
authFormClient   rb 512
authHeaders      rw 8300
authHeaderA      rb 8300
authVerification rb 512
authVerifyUri    rw 512
authXuidCandidate rb 64
authSecretsEnd:

section '.authc' code readable executable

; Append to a caller-sized buffer. All call sites use statically bounded pieces.
proc AuthAppend uses esi edi, pDestination,pSource
    mov edi,[pDestination]
    mov esi,[pSource]
  .end_find:
    cmp byte [edi],0
    je .copy
    inc edi
    jmp .end_find
  .copy:
    lodsb
    stosb
    test al,al
    jnz .copy
    ret
endp

proc AuthEqual uses esi edi, pLeft,pRight
    mov esi,[pLeft]
    mov edi,[pRight]
  .next:
    mov al,[esi]
    cmp al,[edi]
    jne .no
    inc esi
    inc edi
    test al,al
    jnz .next
    mov eax,1
    ret
  .no:
    xor eax,eax
    ret
endp

; application/x-www-form-urlencoded encoding, checked destination capacity.
proc AuthFormEncode uses ebx esi edi, pSource,pDestination,capacity
    mov esi,[pSource]
    mov edi,[pDestination]
    mov ecx,[capacity]
    test ecx,ecx
    jz .bad
    dec ecx
  .next:
    movzx eax,byte [esi]
    test al,al
    jz .ok
    inc esi
    cmp al,'a'
    jb .upper
    cmp al,'z'
    jbe .literal
  .upper:
    cmp al,'A'
    jb .digit
    cmp al,'Z'
    jbe .literal
  .digit:
    cmp al,'0'
    jb .punctuation
    cmp al,'9'
    jbe .literal
  .punctuation:
    cmp al,'-'
    je .literal
    cmp al,'_'
    je .literal
    cmp al,'.'
    je .literal
    cmp al,'~'
    je .literal
    cmp ecx,3
    jb .bad
    sub ecx,3
    mov byte [edi],'%'
    inc edi
    mov ebx,eax
    shr eax,4
    mov al,[authHex+eax]
    stosb
    and ebx,15
    mov al,[authHex+ebx]
    stosb
    jmp .next
  .literal:
    test ecx,ecx
    jz .bad
    dec ecx
    stosb
    jmp .next
  .ok:
    mov byte [edi],0
    mov eax,1
    ret
  .bad:
    mov edi,[pDestination]
    cmp [capacity],0
    je .return_bad
    mov byte [edi],0
  .return_bad:
    xor eax,eax
    ret
endp

; JSON-escape opaque service tokens rather than assuming JWT contents.
proc AuthJsonEscape uses ebx esi edi, pSource,pDestination,capacity
    mov esi,[pSource]
    mov edi,[pDestination]
    mov ecx,[capacity]
    test ecx,ecx
    jz .bad
    dec ecx
  .next:
    movzx eax,byte [esi]
    inc esi
    test al,al
    jz .ok
    cmp al,32
    jb .control
    cmp al,'"'
    je .escape
    cmp al,92
    je .escape
    test ecx,ecx
    jz .bad
    dec ecx
    stosb
    jmp .next
  .escape:
    cmp ecx,2
    jb .bad
    sub ecx,2
    mov byte [edi],92
    inc edi
    stosb
    jmp .next
  .control:
    cmp ecx,6
    jb .bad
    sub ecx,6
    mov dword [edi],3030755Ch ; \u00
    add edi,4
    mov ebx,eax
    shr eax,4
    mov al,[authHex+eax]
    stosb
    and ebx,15
    mov al,[authHex+ebx]
    stosb
    jmp .next
  .ok:
    mov byte [edi],0
    mov eax,1
    ret
  .bad:
    mov edi,[pDestination]
    cmp [capacity],0
    je .return_bad
    mov byte [edi],0
  .return_bad:
    xor eax,eax
    ret
endp

proc AuthReadString uses ebx, context,object,key,destination,capacity
    stdcall JsonGet,[context],[object],[key]
    test eax,eax
    jz .bad
    stdcall JsonString,[context],eax,[destination],[capacity]
    test eax,eax
    jz .bad
    cmp eax,[capacity]
    jae .bad
    mov eax,1
    ret
  .bad:
    mov ebx,[destination]
    mov byte [ebx],0
    xor eax,eax
    ret
endp

; The bearer value is sent in an HTTP header: forbid whitespace/control bytes.
proc AuthSafeBearer uses esi, value
    mov esi,[value]
  .next:
    lodsb
    test al,al
    jz .ok
    cmp al,33
    jb .bad
    cmp al,126
    ja .bad
    jmp .next
  .ok:
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

proc AuthVerifyBrowser
    stdcall AuthEqual,authVerification,authVerifyAllowed1
    test eax,eax
    jnz .allowed
    stdcall AuthEqual,authVerification,authVerifyAllowed2
    test eax,eax
    jnz .allowed
    stdcall AuthEqual,authVerification,authVerifyAllowed3
    test eax,eax
    jnz .allowed
    stdcall AuthEqual,authVerification,authVerifyAllowed4
    test eax,eax
    jz .bad
  .allowed:
    invoke MultiByteToWideChar,65001,8,authVerification,-1,authVerifyUri,512
    ret
  .bad:
    xor eax,eax
    ret
endp

; Xbox XUID is optional in Minecraft-scoped claims. Retain a supplied decimal ID
; for modern --xuid arguments, while allowing responses containing only `uhs`.
proc AuthReadXuid uses esi ecx, context,root
    stdcall JsonGet,[context],[root],authKClaims
    test eax,eax
    jz .done
    stdcall JsonGet,[context],eax,authKXui
    test eax,eax
    jz .done
    stdcall JsonAt,[context],eax,0
    test eax,eax
    jz .done
    stdcall AuthReadString,[context],eax,authKXid,authXuidCandidate,64
    test eax,eax
    jz .done
    invoke lstrlenA,authXuidCandidate
    cmp eax,20
    ja .done
    mov ecx,eax
    mov esi,authXuidCandidate
  .digit:
    lodsb
    cmp al,'0'
    jb .done
    cmp al,'9'
    ja .done
    loop .digit
    invoke lstrcpyA,authXuid,authXuidCandidate
  .done:
    ret
endp

proc AuthValidateProfile uses esi ecx
    invoke lstrlenA,authProfileName
    ; Historical authenticated profiles may have one- or two-character names.
    cmp eax,1
    jb .bad
    cmp eax,16
    ja .bad
    mov esi,authProfileName
  .name_next:
    lodsb
    test al,al
    jz .uuid
    cmp al,'_'
    je .name_next
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .name_next
    or al,32
    cmp al,'a'
    jb .bad
    cmp al,'z'
    jbe .name_next
    jmp .bad
  .uuid:
    invoke lstrlenA,authProfileUuid
    cmp eax,32
    jne .bad
    mov esi,authProfileUuid
    mov ecx,32
  .uuid_next:
    lodsb
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .uuid_advance
    or al,32
    cmp al,'a'
    jb .bad
    cmp al,'f'
    ja .bad
  .uuid_advance:
    loop .uuid_next
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

; Owns response + parser context until the next request or cleanup.
proc AuthRelease uses edi ecx
    cmp [authContext],0
    je .response
    stdcall JsonFree,[authContext]
    mov [authContext],0
  .response:
    cmp [authResponse],0
    je .done
    invoke lstrlenA,[authResponse]
    mov ecx,eax
    mov edi,[authResponse]
    xor eax,eax
    rep stosb
    stdcall FreeMem,[authResponse]
    mov [authResponse],0
  .done:
    ret
endp

proc AuthRequest, url,method,headers,body
    stdcall AuthRelease
    cmp [authCancel],0
    jne .bad
    stdcall HttpRequest,[url],[method],[headers],[body]
    mov [authResponse],eax
    test eax,eax
    jz .bad
    mov eax,[httpStatus]
    mov [authHttpCode],eax
    stdcall JsonParse,[authResponse]
    mov [authContext],eax
    test eax,eax
    jz .bad
    stdcall JsonRoot,eax
    mov [authRootToken],eax
    test eax,eax
    jz .bad
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

proc AuthValidateClient uses esi ecx
    mov esi,clientId
    xor ecx,ecx
  .next:
    mov al,[esi+ecx]
    cmp ecx,36
    je .validation_end
    test al,al
    jz .bad
    cmp ecx,8
    je .hyphen
    cmp ecx,13
    je .hyphen
    cmp ecx,18
    je .hyphen
    cmp ecx,23
    je .hyphen
    cmp al,'0'
    jb .bad
    cmp al,'9'
    jbe .advance
    or al,32
    cmp al,'a'
    jb .bad
    cmp al,'f'
    ja .bad
    jmp .advance
  .hyphen:
    cmp al,'-'
    jne .bad
  .advance:
    inc ecx
    jmp .next
  .validation_end:
    test al,al
    jnz .bad
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

; Poll sleeps in 250ms slices so UI cancellation does not wait a full interval.
proc AuthWait uses ebx, milliseconds
    mov ebx,[milliseconds]
  .next:
    cmp [authCancel],0
    jne .bad
    invoke GetTickCount
    sub eax,[authDeadline]
    jns .bad
    test ebx,ebx
    jz .ok
    mov eax,250
    cmp ebx,eax
    jae .sleep
    mov eax,ebx
  .sleep:
    sub ebx,eax
    invoke Sleep,eax
    jmp .next
  .ok:
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

proc AuthWipeSecrets uses edi ecx eax
    xor eax,eax
    mov edi,authMsToken
    ; These buffers are contiguous up to authHeaderA.
    mov ecx,authSecretsEnd-authMsToken
    rep stosb
    ret
endp

proc AuthClearIdentity uses edi ecx eax
    mov [authReady],0
    xor eax,eax
    mov edi,authToken
    mov ecx,8192
    rep stosb
    mov edi,authName
    mov ecx,64
    rep stosb
    mov edi,authUuid
    mov ecx,64
    rep stosb
    mov edi,authXuid
    mov ecx,64
    rep stosb
    ret
endp

proc AuthLoad
    stdcall AuthClearIdentity
    mov byte [authDisplay],0
    ret
endp

proc AuthLogout
    mov [authCancel],1
    stdcall AuthClearIdentity
    mov byte [authDisplay],0
    stdcall SetStatusA,authMsgLogout
    ret
endp

proc AuthLogin uses ebx esi edi, parameter
    mov [authResult],0
    stdcall AuthClearIdentity
    stdcall AuthWipeSecrets
    mov byte [authDisplay],0
    cmp [authCancel],0
    jne .cancel_failure
    cmp byte [clientId],0
    je .need_client
    stdcall AuthValidateClient
    test eax,eax
    jz .invalid_client
    stdcall SetStatusA,authMsgStart
    stdcall AuthFormEncode,clientId,authFormClient,512
    test eax,eax
    jz .format_failure
    mov byte [authBody],0
    stdcall AuthAppend,authBody,authDevicePrefix
    stdcall AuthAppend,authBody,authFormClient
    stdcall AuthAppend,authBody,authScope
    stdcall AuthRequest,authDeviceUrl,authPost,authFormHeader,authBody
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    jne .device_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKDevice,authDeviceCode,8192
    test eax,eax
    jz .format_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKUser,authUserCode,64
    test eax,eax
    jz .format_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKVerify,authVerification,512
    test eax,eax
    jz .format_failure
    stdcall AuthVerifyBrowser
    test eax,eax
    jz .format_failure
    stdcall JsonGet,[authContext],[authRootToken],authKExpires
    test eax,eax
    jz .format_failure
    stdcall JsonInt,[authContext],eax
    cmp eax,1
    jl .format_failure
    cmp eax,1800
    ja .format_failure
    imul eax,1000
    mov ebx,eax
    invoke GetTickCount
    add eax,ebx
    mov [authDeadline],eax
    mov [authInterval],5000
    stdcall JsonGet,[authContext],[authRootToken],authKInterval
    test eax,eax
    jz .interval_ready
    stdcall JsonInt,[authContext],eax
    cmp eax,1
    jl .format_failure
    cmp eax,60
    ja .format_failure
    imul eax,1000
    mov [authInterval],eax
  .interval_ready:
    mov byte [authDisplay],0
if defined OFFSET_I18N
    stdcall AuthAppend,authDisplay,[trAuthCodePrefix]
else
    stdcall AuthAppend,authDisplay,authCodePrefix
end if
    stdcall AuthAppend,authDisplay,authVerification
if defined OFFSET_I18N
    stdcall AuthAppend,authDisplay,[trAuthCodeMiddle]
else
    stdcall AuthAppend,authDisplay,authCodeMiddle
end if
    stdcall AuthAppend,authDisplay,authUserCode
    stdcall SetStatusA,authDisplay
    invoke ShellExecuteW,[hWnd],authOpen,authVerifyUri,0,0,1
    stdcall AuthFormEncode,authDeviceCode,authFormDevice,24576
    test eax,eax
    jz .format_failure
  .poll:
    stdcall AuthWait,[authInterval]
    test eax,eax
    jz .wait_failure
    mov byte [authBody],0
    stdcall AuthAppend,authBody,authPollPrefix
    stdcall AuthAppend,authBody,authFormClient
    stdcall AuthAppend,authBody,authPollDevice
    stdcall AuthAppend,authBody,authFormDevice
    stdcall AuthRequest,authTokenUrl,authPost,authFormHeader,authBody
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    je .microsoft_authorized
    stdcall AuthReadString,[authContext],[authRootToken],authKError,authError,256
    test eax,eax
    jz .token_failure
    stdcall AuthEqual,authError,authPending
    test eax,eax
    jnz .poll
    stdcall AuthEqual,authError,authSlowDown
    test eax,eax
    jz .not_slow
    add [authInterval],5000
    jmp .poll
  .not_slow:
    stdcall AuthEqual,authError,authExpired
    test eax,eax
    jnz .expired_failure
    jmp .token_failure
  .microsoft_authorized:
    stdcall AuthReadString,[authContext],[authRootToken],authKAccess,authMsToken,16384
    test eax,eax
    jz .format_failure
    stdcall SetStatusA,authMsgXbox
    stdcall AuthJsonEscape,authMsToken,authEscaped,49152
    test eax,eax
    jz .format_failure
    mov byte [authBody],0
    stdcall AuthAppend,authBody,authXblPrefix
    stdcall AuthAppend,authBody,authEscaped
    stdcall AuthAppend,authBody,authXblSuffix
    stdcall AuthRequest,authXblUrl,authPost,authJsonHeader,authBody
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    jne .xbox_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKToken,authXblToken,16384
    test eax,eax
    jz .format_failure
    stdcall AuthReadXuid,[authContext],[authRootToken]
    stdcall SetStatusA,authMsgXsts
    stdcall AuthJsonEscape,authXblToken,authEscaped,49152
    test eax,eax
    jz .format_failure
    mov byte [authBody],0
    stdcall AuthAppend,authBody,authXstsPrefix
    stdcall AuthAppend,authBody,authEscaped
    stdcall AuthAppend,authBody,authXstsSuffix
    stdcall AuthRequest,authXstsUrl,authPost,authJsonHeader,authBody
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    jne .xsts_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKToken,authXstsToken,16384
    test eax,eax
    jz .format_failure
    stdcall AuthReadXuid,[authContext],[authRootToken]
    stdcall JsonGet,[authContext],[authRootToken],authKClaims
    test eax,eax
    jz .format_failure
    stdcall JsonGet,[authContext],eax,authKXui
    test eax,eax
    jz .format_failure
    stdcall JsonAt,[authContext],eax,0
    test eax,eax
    jz .format_failure
    stdcall AuthReadString,[authContext],eax,authKUhs,authUhs,128
    test eax,eax
    jz .format_failure
    stdcall SetStatusA,authMsgMinecraft
    mov byte [authBody],0
    stdcall AuthAppend,authBody,authMcPrefix
    stdcall AuthJsonEscape,authUhs,authEscaped,49152
    test eax,eax
    jz .format_failure
    stdcall AuthAppend,authBody,authEscaped
    stdcall AuthAppend,authBody,authSemicolon
    stdcall AuthJsonEscape,authXstsToken,authEscaped,49152
    test eax,eax
    jz .format_failure
    stdcall AuthAppend,authBody,authEscaped
    stdcall AuthAppend,authBody,authJsonClose
    stdcall AuthRequest,authMinecraftUrl,authPost,authJsonHeader,authBody
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    jne .minecraft_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKAccess,authMcToken,8192
    test eax,eax
    jz .format_failure
    stdcall AuthSafeBearer,authMcToken
    test eax,eax
    jz .format_failure
    mov byte [authHeaderA],0
    stdcall AuthAppend,authHeaderA,authBearer
    stdcall AuthAppend,authHeaderA,authMcToken
    stdcall AuthAppend,authHeaderA,authCrLf
    invoke MultiByteToWideChar,65001,8,authHeaderA,-1,authHeaders,8300
    test eax,eax
    jz .format_failure
    stdcall AuthRequest,authEntitleUrl,0,authHeaders,0
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    jne .ownership_failure
    stdcall JsonGet,[authContext],[authRootToken],authKItems
    test eax,eax
    jz .ownership_failure
    mov [authObject],eax
    stdcall JsonCount,[authContext],eax
    test eax,eax
    jz .ownership_failure
    mov ebx,eax
    xor esi,esi
  .entitlement_loop:
    cmp esi,ebx
    jae .ownership_failure
    stdcall JsonAt,[authContext],[authObject],esi
    test eax,eax
    jz .ownership_failure
    stdcall AuthReadString,[authContext],eax,authKName,authError,256
    test eax,eax
    jz .next_entitlement
    stdcall AuthEqual,authError,authItemProduct
    test eax,eax
    jnz .ownership_verified
    stdcall AuthEqual,authError,authItemGame
    test eax,eax
    jnz .ownership_verified
  .next_entitlement:
    inc esi
    jmp .entitlement_loop
  .ownership_verified:
    stdcall AuthRequest,authProfileUrl,0,authHeaders,0
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    jne .profile_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKName,authProfileName,64
    test eax,eax
    jz .profile_failure
    stdcall AuthReadString,[authContext],[authRootToken],authKId,authProfileUuid,64
    test eax,eax
    jz .profile_failure
    stdcall AuthValidateProfile
    test eax,eax
    jz .profile_failure
    cmp [authCancel],0
    jne .cancel_failure
    invoke lstrcpyA,authToken,authMcToken
    invoke lstrcpyA,authName,authProfileName
    invoke lstrcpyA,authUuid,authProfileUuid
    mov [authReady],1
    mov [authResult],1
    mov byte [authDisplay],0
if defined OFFSET_I18N
    stdcall AuthAppend,authDisplay,[trAuthSignedPrefix]
else
    stdcall AuthAppend,authDisplay,authSignedPrefix
end if
    stdcall AuthAppend,authDisplay,authName
    stdcall SetStatusA,authDisplay
    jmp .done
  .need_client:
    stdcall SetStatusA,authMsgClient
    jmp .done
  .invalid_client:
    stdcall SetStatusA,authMsgInvalidId
    jmp .done
  .device_failure:
    stdcall SetStatusA,authMsgDevice
    jmp .done
  .token_failure:
    stdcall SetStatusA,authMsgToken
    jmp .done
  .wait_failure:
    cmp [authCancel],0
    jne .cancel_failure
  .expired_failure:
    stdcall SetStatusA,authMsgExpired
    jmp .done
  .network_failure:
    cmp [authCancel],0
    jne .cancel_failure
    stdcall SetStatusA,authMsgNetwork
    jmp .done
  .format_failure:
    stdcall SetStatusA,authMsgFormat
    jmp .done
  .xbox_failure:
    stdcall SetStatusA,authMsgXboxFail
    jmp .done
  .xsts_failure:
    stdcall SetStatusA,authMsgXstsFail
    jmp .done
  .minecraft_failure:
    stdcall SetStatusA,authMsgMcFail
    jmp .done
  .ownership_failure:
    stdcall SetStatusA,authMsgOwnFail
    jmp .done
  .profile_failure:
    stdcall SetStatusA,authMsgProfile
    jmp .done
  .cancel_failure:
    stdcall SetStatusA,authMsgCancel
  .done:
    stdcall AuthRelease
    stdcall AuthWipeSecrets
    cmp [authResult],0
    jne .notify
    stdcall AuthClearIdentity
  .notify:
    invoke PostMessageW,[hWnd],08002h,0,0
    mov eax,[authResult]
    ret
endp
