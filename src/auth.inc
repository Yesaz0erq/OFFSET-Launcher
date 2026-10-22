; OFFSET Launcher - original Win32/x86 assembly Microsoft authentication.
; Device grant -> Xbox Live -> XSTS -> Minecraft entitlement/profile.
; Only refresh credentials persist, protected by current-user Windows DPAPI.
; Minecraft tokens remain in memory and are refreshed before expiry.

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
authHasRefresh   dd 0
authCacheOkay    dd 0
authCacheSerial  dd 0
authSessionExpiry dq 0
authNow          dq 0
authExpiryPending dq 0
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
authKRefresh     db 'refresh_token',0
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
authInvalidGrant db 'invalid_grant',0
authInteraction  db 'interaction_required',0
authItemProduct  db 'product_minecraft',0
authItemGame     db 'game_minecraft',0
authHex          db '0123456789ABCDEF',0
authDevicePrefix db 'client_id=',0
authScope        db '&scope=XboxLive.signin%20offline_access',0
authPollPrefix   db 'grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code&client_id=',0
authPollDevice   db '&device_code=',0
authRefreshPrefix db 'grant_type=refresh_token&client_id=',0
authRefreshField db '&refresh_token=',0
authCacheSuffix  du '\offset.account',0
authCacheTempSuffix du '.part',0
authCacheUniqueFormat du '.part-%08X-%08X-%08X',0
authCacheEntropy db 'OFFSET-ACCOUNT-V1',0
authCacheEntropyBlob dd 17,authCacheEntropy
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
authMsgRestore   db 'Restoring your protected Microsoft account session...',0
authMsgNeedLogin db 'Sign in to your Microsoft account on the Account page before using online features.',0
authMsgReauth    db 'Microsoft authorization expired or was revoked. Sign in again on the Account page.',0
authMsgApproval  db 'Minecraft Services denied this Application ID (HTTP 403). Confirm Minecraft API approval for your own client ID.',0
authMsgUnauthorized db 'Your online session was rejected or expired (HTTP 401). Sign in again on the Account page.',0
authMsgCacheSave db 'Signed in. The account could not be saved securely; this session will last until the launcher closes.',0
authMsgCacheDelete db 'Signed out of memory, but the protected account file could not be removed. Check folder permissions before restarting.',0

; Trailing uninitialized storage adds virtual memory, never zero-filled file bytes.
authDisplay      rb 256
authXuid         rb 64
authRefreshToken rb 16384
authRefreshClient rb 128
authSessionClient rb 128
authCachePath    rw 1024
authCacheTemporary rw 1100
authCachePlain   rb 16429
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
authNewRefresh   rb 16384
authFormRefresh  rb 49152
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

; Application GUIDs are case-insensitive. Never apply this comparison to tokens.
proc AuthSameClient uses esi edi ecx, pLeft,pRight
    mov esi,[pLeft]
    mov edi,[pRight]
    mov ecx,36
  .next:
    mov al,[esi]
    mov dl,[edi]
    or al,32
    or dl,32
    cmp al,dl
    jne .bad
    inc esi
    inc edi
    loop .next
    cmp byte [esi],0
    jne .bad
    cmp byte [edi],0
    jne .bad
    mov eax,1
    ret
  .bad:
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
    mov ebx,eax
    invoke lstrlenA,[destination]
    cmp eax,ebx
    jne .bad ; embedded JSON \u0000 must not become a truncated credential
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

proc AuthWipeRefresh uses edi ecx eax
    mov [authHasRefresh],0
    xor eax,eax
    mov edi,authRefreshToken
    mov ecx,16384
    rep stosb
    mov edi,authRefreshClient
    mov ecx,128
    rep stosb
    ret
endp

proc AuthCachePaths uses esi edi
    cmp word [authCachePath],0
    jne .temporary
    invoke GetModuleFileNameW,0,authCachePath,1024
    test eax,eax
    jz .bad
    cmp eax,980
    jae .bad
    mov esi,authCachePath
    mov edi,esi
  .walk:
    mov ax,[esi]
    test ax,ax
    jz .cut
    cmp ax,92
    jne .advance
    mov edi,esi
  .advance:
    add esi,2
    jmp .walk
  .cut:
    cmp edi,authCachePath
    je .bad
    mov word [edi],0
    invoke lstrcatW,authCachePath,authCacheSuffix
  .temporary:
    invoke lstrlenW,authCachePath
    cmp eax,1023
    jae .bad
    invoke lstrcpyW,authCacheTemporary,authCachePath
    invoke lstrcatW,authCacheTemporary,authCacheTempSuffix
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

proc AuthDeleteCache
    stdcall AuthCachePaths
    test eax,eax
    jz .done
    ; A previous .part file is not owned by this operation. Never remove it.
    stdcall AuthCacheTargetSafe,1
    test eax,eax
    jz .done
    invoke GetFileAttributesW,authCachePath
    cmp eax,-1
    je .missing
    invoke DeleteFileW,authCachePath
    ret
  .missing:
    mov eax,1
  .done:
    ret
endp

; Inspect the final cache entry itself, without following a symbolic link.
; A cache must be a regular, singly-linked file; a missing entry is optional.
proc AuthCacheTargetSafe uses ebx,allowMissing
    locals
        information rd 13
        handle dd ?
    endl
    invoke CreateFileW,authCachePath,080h,7,0,OPEN_EXISTING,00200080h,0
    cmp eax,-1
    jne .opened
    cmp [allowMissing],0
    je .bad
    invoke GetLastError
    cmp eax,2
    je .okay
    cmp eax,3
    je .okay
    jmp .bad
  .opened:
    mov [handle],eax
    invoke GetFileInformationByHandle,eax,addr information
    mov ebx,eax
    invoke CloseHandle,[handle]
    test ebx,ebx
    jz .bad
    test dword [information],0410h ; directory or reparse point
    jnz .bad
    cmp dword [information+40],1
    jne .bad
  .okay:
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

; CREATE_NEW is the ownership boundary: collisions never overwrite/truncate an
; existing object, including hard links. A bounded retry gets a different name.
proc AuthCreateCacheTemporary uses ebx esi
    locals
        suffix rw 64
        pid dd ?
        tick dd ?
        attempts dd ?
    endl
    invoke GetCurrentProcessId
    mov [pid],eax
    invoke GetTickCount
    mov [tick],eax
    mov [attempts],32
  .attempt:
    inc [authCacheSerial]
    cinvoke wsprintfW,addr suffix,authCacheUniqueFormat,[pid],[tick],[authCacheSerial]
    invoke lstrcpyW,authCacheTemporary,authCachePath
    invoke lstrcatW,authCacheTemporary,addr suffix
    invoke CreateFileW,authCacheTemporary,GENERIC_WRITE,0,0,CREATE_NEW,00200080h,0
    cmp eax,-1
    jne .done
    invoke GetLastError
    cmp eax,80
    je .collision
    cmp eax,183
    jne .bad
  .collision:
    dec [attempts]
    jnz .attempt
  .bad:
    mov eax,-1
  .done:
    ret
endp

; Writes a short versioned record containing client ID + refresh token only.
; DPAPI uses the current Windows user (never CRYPTPROTECT_LOCAL_MACHINE).
proc AuthSaveCache uses ebx esi edi
    locals
        inputBlob rd 2
        outputBlob rd 2
        fileHandle dd ?
        written dd ?
        okay dd ?
        owned dd ?
    endl
    mov [okay],0
    mov [owned],0
    mov dword [outputBlob],0
    mov dword [outputBlob+4],0
    cmp [authHasRefresh],1
    jne .finish
    stdcall AuthCachePaths
    test eax,eax
    jz .finish
    stdcall AuthCacheTargetSafe,1
    test eax,eax
    jz .finish
    stdcall AuthSameClient,clientId,authRefreshClient
    test eax,eax
    jz .finish
    invoke lstrlenA,authRefreshToken
    cmp eax,1
    jb .finish
    cmp eax,16384
    jae .finish
    inc eax
    mov dword [authCachePlain],3141464Fh ; OFA1
    mov dword [authCachePlain+4],eax
    add eax,45
    mov [inputBlob],eax
    mov dword [inputBlob+4],authCachePlain
    invoke lstrcpyA,authCachePlain+8,authRefreshClient
    invoke lstrcpyA,authCachePlain+45,authRefreshToken
    invoke CryptProtectData,addr inputBlob,0,authCacheEntropyBlob,0,0,1,addr outputBlob
    test eax,eax
    jz .finish
    cmp dword [outputBlob],65536
    ja .release
    stdcall AuthCreateCacheTemporary
    cmp eax,-1
    je .release
    mov [fileHandle],eax
    mov [owned],1
    invoke WriteFile,eax,[outputBlob+4],[outputBlob],addr written,0
    mov ebx,eax
    invoke CloseHandle,[fileHandle]
    test ebx,ebx
    jz .remove_temporary
    mov eax,[written]
    cmp eax,[outputBlob]
    jne .remove_temporary
    stdcall AuthCacheTargetSafe,1
    test eax,eax
    jz .remove_temporary
    invoke MoveFileExW,authCacheTemporary,authCachePath,9
    mov [okay],eax
    test eax,eax
    jz .remove_temporary
    mov [owned],0
  .remove_temporary:
    cmp [owned],0
    je .release
    invoke DeleteFileW,authCacheTemporary
  .release:
    mov edi,[outputBlob+4]
    test edi,edi
    jz .finish
    mov ecx,[outputBlob]
    xor eax,eax
    rep stosb
    invoke LocalFree,[outputBlob+4]
  .finish:
    mov edi,authCachePlain
    mov ecx,16429
    xor eax,eax
    rep stosb
    mov eax,[okay]
    mov [authCacheOkay],eax
    ret
endp

proc AuthReadCache uses ebx esi edi
    locals
        inputBlob rd 2
        outputBlob rd 2
        fileHandle dd ?
        cipher dd ?
        size dd ?
        readCount dd ?
        okay dd ?
        information rd 13
    endl
    mov [okay],0
    mov [cipher],0
    mov dword [outputBlob],0
    mov dword [outputBlob+4],0
    stdcall AuthWipeRefresh
    stdcall AuthValidateClient
    test eax,eax
    jz .finish
    stdcall AuthCachePaths
    test eax,eax
    jz .finish
    invoke CreateFileW,authCachePath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,00200080h,0
    cmp eax,-1
    je .finish
    mov [fileHandle],eax
    invoke GetFileInformationByHandle,eax,addr information
    test eax,eax
    jz .close
    test dword [information],0410h
    jnz .close
    cmp dword [information+40],1
    jne .close
    mov eax,[fileHandle]
    invoke GetFileSize,eax,0
    cmp eax,1
    jb .close
    cmp eax,65536
    ja .close
    mov [size],eax
    invoke GetProcessHeap
    invoke HeapAlloc,eax,8,[size]
    test eax,eax
    jz .close
    mov [cipher],eax
    invoke ReadFile,[fileHandle],eax,[size],addr readCount,0
    test eax,eax
    jz .close
    mov eax,[readCount]
    cmp eax,[size]
    jne .close
    invoke CloseHandle,[fileHandle]
    mov [fileHandle],-1
    mov eax,[size]
    mov [inputBlob],eax
    mov eax,[cipher]
    mov [inputBlob+4],eax
    invoke CryptUnprotectData,addr inputBlob,0,authCacheEntropyBlob,0,0,1,addr outputBlob
    test eax,eax
    jz .release
    cmp dword [outputBlob],46
    jb .release
    cmp dword [outputBlob],16429
    ja .release
    mov esi,[outputBlob+4]
    cmp dword [esi],3141464Fh
    jne .release
    mov ebx,[esi+4]
    cmp ebx,2
    jb .release
    cmp ebx,16384
    ja .release
    lea eax,[ebx+45]
    cmp eax,[outputBlob]
    jne .release
    cmp byte [esi+44],0
    jne .release
    lea eax,[esi+8]
    stdcall AuthSameClient,eax,clientId
    test eax,eax
    jz .release
    cmp byte [esi+ebx+44],0
    jne .release
    lea eax,[esi+45]
    invoke lstrlenA,eax
    inc eax
    cmp eax,ebx
    jne .release
    lea eax,[esi+45]
    invoke lstrcpyA,authRefreshToken,eax
    invoke lstrcpyA,authRefreshClient,clientId
    mov [authHasRefresh],1
    mov [okay],1
    jmp .release
  .close:
    invoke CloseHandle,[fileHandle]
  .release:
    mov edi,[outputBlob+4]
    test edi,edi
    jz .cipher
    mov ecx,[outputBlob]
    xor eax,eax
    rep stosb
    invoke LocalFree,[outputBlob+4]
  .cipher:
    cmp [cipher],0
    je .finish
    stdcall FreeMem,[cipher]
  .finish:
    mov eax,[okay]
    ret
endp

; FILETIME deadline is based on expires_in, never by decoding opaque tokens.
proc AuthSetExpiry
    stdcall JsonGet,[authContext],[authRootToken],authKExpires
    test eax,eax
    jz .bad
    stdcall JsonInt,[authContext],eax
    cmp eax,1
    jb .bad
    cmp eax,604800
    ja .bad
    mov ecx,10000000
    mul ecx
    mov dword [authExpiryPending],eax
    mov dword [authExpiryPending+4],edx
    invoke GetSystemTimeAsFileTime,authNow
    mov eax,dword [authNow]
    mov edx,dword [authNow+4]
    add dword [authExpiryPending],eax
    adc dword [authExpiryPending+4],edx
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp

proc AuthClearIdentity uses edi ecx eax
    mov [authReady],0
    mov dword [authSessionExpiry],0
    mov dword [authSessionExpiry+4],0
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
    mov edi,authSessionClient
    mov ecx,128
    rep stosb
    ret
endp

proc AuthLoad
    stdcall AuthClearIdentity
    mov byte [authDisplay],0
    stdcall AuthReadCache
    ret
endp

proc AuthLogout
    mov [authCancel],1
    stdcall AuthClearIdentity
    stdcall AuthWipeRefresh
    stdcall AuthWipeSecrets
    mov byte [authDisplay],0
    stdcall AuthDeleteCache
    test eax,eax
    jz .delete_failed
    stdcall SetStatusA,authMsgLogout
    ret
  .delete_failed:
    stdcall SetStatusA,authMsgCacheDelete
    ret
endp

; Noninteractive entry used by launch and skin workers. No browser or completion
; message is emitted; the caller owns its worker lifecycle.
proc AuthEnsureSession uses ebx
    cmp [authCancel],0
    jne .cancel
    stdcall AuthValidateClient
    test eax,eax
    jz .client
    cmp [authReady],1
    jne .refresh
    cmp byte [authToken],0
    je .refresh
    cmp byte [authName],0
    je .refresh
    cmp byte [authUuid],0
    je .refresh
    stdcall AuthSameClient,authSessionClient,clientId
    test eax,eax
    jz .refresh
    invoke GetSystemTimeAsFileTime,authNow
    mov eax,dword [authNow]
    mov edx,dword [authNow+4]
    add eax,600000000 ; refresh at least 60 seconds before expiry
    adc edx,0
    cmp edx,dword [authSessionExpiry+4]
    ja .refresh
    jb .valid
    cmp eax,dword [authSessionExpiry]
    jae .refresh
  .valid:
    mov eax,1
    ret
  .refresh:
    stdcall AuthClearIdentity
    cmp [authHasRefresh],1
    jne .need_login
    cmp byte [authRefreshToken],0
    je .need_login
    stdcall AuthSameClient,authRefreshClient,clientId
    test eax,eax
    jz .need_login
    stdcall AuthRun,0,1
    ret
  .client:
    stdcall AuthClearIdentity
    stdcall SetStatusA,authMsgInvalidId
    xor eax,eax
    ret
  .cancel:
    stdcall AuthClearIdentity
    stdcall SetStatusA,authMsgCancel
    xor eax,eax
    ret
  .need_login:
    stdcall SetStatusA,authMsgNeedLogin
    xor eax,eax
    ret
endp

proc AuthInvalidateSession
    stdcall AuthClearIdentity
    ret
endp

proc AuthLoginQuiet, parameter
    stdcall AuthRun,[parameter],0
    test eax,eax
    jnz .done
    ; A cancelled/failed account switch never destroys the previous encrypted
    ; credential. Keep it recoverable without claiming the failed login worked.
    stdcall AuthReadCache
    xor eax,eax
  .done:
    ret
endp

proc AuthLogin, parameter
    stdcall AuthLoginQuiet,[parameter]
    push eax
    invoke PostMessageW,[hWnd],08002h,0,0
    pop eax
    ret
endp

; Shared authentication chain. Microsoft device and refresh grants converge
; before Xbox/XSTS/Minecraft; only successful profile+ownership commits ready=1.
proc AuthRun uses ebx esi edi, parameter,useRefresh
    mov [authResult],0
    stdcall AuthClearIdentity
    stdcall AuthWipeSecrets
    mov byte [authDisplay],0
    mov [authCacheOkay],1
    cmp [authCancel],0
    jne .cancel_failure
    cmp byte [clientId],0
    je .need_client
    stdcall AuthValidateClient
    test eax,eax
    jz .invalid_client
    cmp [useRefresh],0
    jne .refresh_start
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
  .refresh_start:
    stdcall SetStatusA,authMsgRestore
    stdcall AuthFormEncode,clientId,authFormClient,512
    test eax,eax
    jz .format_failure
    stdcall AuthFormEncode,authRefreshToken,authFormRefresh,49152
    test eax,eax
    jz .format_failure
    mov byte [authBody],0
    stdcall AuthAppend,authBody,authRefreshPrefix
    stdcall AuthAppend,authBody,authFormClient
    stdcall AuthAppend,authBody,authRefreshField
    stdcall AuthAppend,authBody,authFormRefresh
    stdcall AuthAppend,authBody,authScope
    stdcall AuthRequest,authTokenUrl,authPost,authFormHeader,authBody
    test eax,eax
    jz .network_failure
    cmp [authHttpCode],200
    je .microsoft_authorized
    cmp [authHttpCode],401
    je .refresh_invalid
    stdcall AuthReadString,[authContext],[authRootToken],authKError,authError,256
    test eax,eax
    jz .token_failure
    stdcall AuthEqual,authError,authInvalidGrant
    test eax,eax
    jnz .refresh_invalid
    stdcall AuthEqual,authError,authInteraction
    test eax,eax
    jnz .refresh_invalid
    jmp .token_failure
  .refresh_invalid:
    stdcall AuthWipeRefresh
    stdcall AuthDeleteCache
    stdcall SetStatusA,authMsgReauth
    jmp .done
  .microsoft_authorized:
    stdcall AuthReadString,[authContext],[authRootToken],authKAccess,authMsToken,16384
    test eax,eax
    jz .format_failure
    ; Refresh token is opaque and optional on rotation; keep the previous one
    ; when a successful refresh response does not issue a replacement.
    stdcall JsonGet,[authContext],[authRootToken],authKRefresh
    test eax,eax
    jz .refresh_field_done
    stdcall AuthReadString,[authContext],[authRootToken],authKRefresh,authNewRefresh,16384
    test eax,eax
    jz .format_failure
  .refresh_field_done:
    cmp [useRefresh],0
    je .rotation_done
    cmp byte [authNewRefresh],0
    je .rotation_done
    ; A valid refresh response rotates the same account's credential. Save it
    ; even if a later Xbox/Minecraft request temporarily fails.
    stdcall AuthWipeRefresh
    invoke lstrcpyA,authRefreshToken,authNewRefresh
    invoke lstrcpyA,authRefreshClient,clientId
    mov [authHasRefresh],1
    stdcall AuthSaveCache
  .rotation_done:
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
    stdcall AuthSetExpiry
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
    invoke lstrcpyA,authSessionClient,clientId
    mov eax,dword [authExpiryPending]
    mov edx,dword [authExpiryPending+4]
    mov dword [authSessionExpiry],eax
    mov dword [authSessionExpiry+4],edx
    cmp byte [authNewRefresh],0
    jne .new_refresh
    cmp [useRefresh],0
    jne .refresh_commit_done
    ; A different interactive account must never inherit an old account's
    ; refresh token when the server has not granted offline access.
    stdcall AuthWipeRefresh
    stdcall AuthDeleteCache
    mov [authCacheOkay],0
    jmp .refresh_commit_done
  .new_refresh:
    stdcall AuthWipeRefresh
    invoke lstrcpyA,authRefreshToken,authNewRefresh
    invoke lstrcpyA,authRefreshClient,clientId
    mov [authHasRefresh],1
  .refresh_commit_done:
    cmp [authHasRefresh],1
    jne .session_only
    stdcall AuthSaveCache
  .session_only:
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
    cmp [authCacheOkay],0
    jne .done
    stdcall SetStatusA,authMsgCacheSave
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
    cmp [authHttpCode],403
    je .approval_failure
    cmp [authHttpCode],401
    je .unauthorized_failure
    stdcall SetStatusA,authMsgMcFail
    jmp .done
  .ownership_failure:
    cmp [authHttpCode],401
    je .unauthorized_failure
    stdcall SetStatusA,authMsgOwnFail
    jmp .done
  .profile_failure:
    cmp [authHttpCode],401
    je .unauthorized_failure
    stdcall SetStatusA,authMsgProfile
    jmp .done
  .approval_failure:
    stdcall SetStatusA,authMsgApproval
    jmp .done
  .unauthorized_failure:
    stdcall SetStatusA,authMsgUnauthorized
    jmp .done
  .cancel_failure:
    stdcall SetStatusA,authMsgCancel
  .done:
    stdcall AuthRelease
    stdcall AuthWipeSecrets
    cmp [authResult],0
    jne .return_result
    stdcall AuthClearIdentity
  .return_result:
    mov eax,[authResult]
    ret
endp
