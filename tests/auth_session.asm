; Actual current-user DPAPI/file tests + real authentication/JSON state machine.
; HTTP and wall clock are fixtures; no real account, browser or OAuth request.
format PE console 4.0
entry start
include 'win32ax.inc'

section '.data' data readable writeable
hWnd dd 0
authReady dd 0
scenario dd 0
phase dd 1
requests dd 0
transportFailure dd 0
lastStatus dd 0
lastHttpPointer dd 0
lastHttpSize dd 0
browserCalls dd 0
completionCalls dd 0
caseResult dd 0
handle dd 0
bytesRead dd 0
cipherSize dd 0
fixedTime dd 010000000h,2
FakeTime dd FixtureTime
FakeShell dd FixtureShell
FakePost dd FixturePost
FakeCachePid dd FixtureCachePid
FakeCacheTick dd FixtureCacheTick
FakeInformation dd FixtureInformation
simulateReparse dd 0
testDirectory du 'tests\out',0
testPath du 'tests\out\auth-'
         dw 08D26h,06237h
         du '.account',0
sentinelText db 'KEEP-ME',0
sentinelSuffix du '.sentinel',0
testGuid db '01234567-89ab-cdef-0123-456789abcdef',0
otherGuid db '11234567-89ab-cdef-0123-456789abcdef',0
upperGuid db '01234567-89AB-CDEF-0123-456789ABCDEF',0
oldRefresh db 'refresh+old/=',0
newRefresh db 'refresh+new/=',0
mcToken db 'mc-session-token',0
profileName db 'TestPlayer',0
profileUuid db '0123456789abcdef0123456789abcdef',0
fixtureMicrosoft db '{"access_token":"ms-token","refresh_token":"refresh+new/="}',0
fixtureNoRotation db '{"access_token":"ms-token"}',0
fixtureXbox db '{"Token":"xbl-token"}',0
fixtureXsts db '{"Token":"xsts-token","DisplayClaims":{"xui":[{"uhs":"12345","xid":"1234567890123456"}]}}',0
fixtureMinecraft db '{"access_token":"mc-session-token","expires_in":3600}',0
fixtureNoExpiry db '{"access_token":"mc-session-token"}',0
fixtureEntitlement db '{"items":[{"name":"product_minecraft"}]}',0
fixtureNoOwnership db '{"items":[]}',0
fixtureProfile db '{"name":"TestPlayer","id":"0123456789abcdef0123456789abcdef"}',0
fixtureInvalidGrant db '{"error":"invalid_grant"}',0
fixtureDenied db '{"error":"ForbiddenOperationException"}',0
fixtureUnauthorized db '{"error":"UnauthorizedOperationException"}',0
fixtureNoProfile db '{"error":"NOT_FOUND"}',0
expectedOldBody db 'grant_type=refresh_token&client_id=01234567-89ab-cdef-0123-456789abcdef&refresh_token=refresh%2Bold%2F%3D&scope=XboxLive.signin%20offline_access',0
expectedNewBody db 'grant_type=refresh_token&client_id=01234567-89ab-cdef-0123-456789abcdef&refresh_token=refresh%2Bnew%2F%3D&scope=XboxLive.signin%20offline_access',0
expectedXboxBody db '{"Properties":{"AuthMethod":"RPS","SiteName":"user.auth.xboxlive.com","RpsTicket":"d=ms-token"},"RelyingParty":"http://auth.xboxlive.com","TokenType":"JWT"}',0
expectedXstsBody db '{"Properties":{"SandboxId":"RETAIL","UserTokens":["xbl-token"]},"RelyingParty":"rp://api.minecraftservices.com/","TokenType":"JWT"}',0
expectedMinecraftBody db '{"identityToken":"XBL3.0 x=12345;xsts-token"}',0
expectedBearer du 'Authorization: Bearer mc-session-token',13,10,0
authName rb 64
authUuid rb 64
authToken rb 8192
clientId rb 128
cipherBytes rb 65536
cacheHashBefore rb 41
cacheHashAfter rb 41
oldTemporaryPath rw 1100
collisionPath rw 1100
sentinelPath rw 1100
temporarySuffix rw 64
originalCachePath rw 1024
sentinelBytes rb 8

section '.text' code readable executable
start:
    invoke CreateDirectoryW,testDirectory,0
    invoke lstrcpyA,clientId,testGuid
    invoke lstrcpyW,authCachePath,testPath
    stdcall AuthDeleteCache
    stdcall AuthLoad
    test eax,eax
    jnz failed
    cmp [authReady],0
    jne failed
    stdcall SeedRefresh
    test eax,eax
    jz failed

    ; Ciphertext exists, contains no literal refresh token, and can be restored.
    invoke CreateFileW,authCachePath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je failed
    mov [handle],eax
    invoke GetFileSize,eax,0
    cmp eax,1
    jb failed
    cmp eax,65536
    ja failed
    mov [cipherSize],eax
    invoke ReadFile,[handle],cipherBytes,eax,bytesRead,0
    test eax,eax
    jz failed
    invoke CloseHandle,[handle]
    stdcall ContainsBytes,cipherBytes,[cipherSize],oldRefresh
    test eax,eax
    jnz failed
    stdcall AuthLoad
    cmp eax,1
    jne failed
    cmp [authReady],0
    jne failed
    cmp [authHasRefresh],1
    jne failed
    stdcall AuthEqual,authRefreshToken,oldRefresh
    cmp eax,1
    jne failed
    mov [phase],2

    invoke lstrcpyA,clientId,upperGuid
    stdcall AuthLoad
    cmp eax,1
    jne failed
    cmp [authHasRefresh],1
    jne failed
    invoke lstrcpyA,clientId,testGuid

    ; Binding to a different app ID never yields a ready session or credential.
    invoke lstrcpyA,clientId,otherGuid
    stdcall AuthLoad
    test eax,eax
    jnz failed
    cmp [authHasRefresh],0
    jne failed
    invoke lstrcpyA,clientId,testGuid

    ; Cancelling an interactive switch preserves the previous ciphertext and
    ; its recoverable credential without marking the failed switch ready.
    stdcall SeedRefresh
    stdcall Sha1File,authCachePath,cacheHashBefore
    cmp eax,1
    jne failed
    mov [authCancel],1
    mov [requests],0
    stdcall AuthLoginQuiet,0
    test eax,eax
    jnz failed
    cmp [authReady],0
    jne failed
    cmp [authHasRefresh],1
    jne failed
    cmp [requests],0
    jne failed
    stdcall AuthEqual,authRefreshToken,oldRefresh
    cmp eax,1
    jne failed
    stdcall Sha1File,authCachePath,cacheHashAfter
    cmp eax,1
    jne failed
    stdcall AuthEqual,cacheHashBefore,cacheHashAfter
    cmp eax,1
    jne failed
    mov [authCancel],0

    mov [scenario],12
    mov [requests],0
    mov [transportFailure],0
    stdcall AuthLoginQuiet,0
    test eax,eax
    jnz failed
    cmp [authReady],0
    jne failed
    cmp [authHasRefresh],1
    jne failed
    cmp [transportFailure],0
    jne failed
    cmp [lastStatus],authMsgNetwork
    jne failed
    stdcall Sha1File,authCachePath,cacheHashAfter
    cmp eax,1
    jne failed
    stdcall AuthEqual,cacheHashBefore,cacheHashAfter
    cmp eax,1
    jne failed
    mov [scenario],0

    ; DPAPI rejects a damaged record. No plaintext fallback is attempted.
    mov eax,[cipherSize]
    dec eax
    xor byte [cipherBytes+eax],1
    invoke CreateFileW,authCachePath,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je failed
    mov [handle],eax
    invoke WriteFile,eax,cipherBytes,[cipherSize],bytesRead,0
    test eax,eax
    jz failed
    invoke CloseHandle,[handle]
    stdcall AuthLoad
    test eax,eax
    jnz failed
    cmp [authHasRefresh],0
    jne failed
    mov [phase],3

    stdcall SeedRefresh
    test eax,eax
    jz failed
    stdcall RunEnsure
    cmp eax,1
    jne failed
    cmp [requests],6
    jne failed
    cmp [authReady],1
    jne failed
    cmp [authCacheOkay],1
    jne failed
    stdcall AuthEqual,authToken,mcToken
    cmp eax,1
    jne failed
    stdcall AuthEqual,authName,profileName
    cmp eax,1
    jne failed
    stdcall AuthEqual,authUuid,profileUuid
    cmp eax,1
    jne failed
    stdcall AuthEqual,authRefreshToken,newRefresh
    cmp eax,1
    jne failed
    cmp [browserCalls],0
    jne failed
    cmp [completionCalls],0
    jne failed

    ; A fresh, matching session is reused without any network request.
    mov [phase],4
    stdcall RunEnsure
    cmp eax,1
    jne failed
    cmp [requests],0
    jne failed
    ; A token with 30 seconds left must refresh before use.
    mov eax,dword [fixedTime]
    mov edx,dword [fixedTime+4]
    add eax,300000000
    adc edx,0
    mov dword [authSessionExpiry],eax
    mov dword [authSessionExpiry+4],edx
    stdcall RunEnsure
    cmp eax,1
    jne failed
    cmp [requests],6
    jne failed
    mov [phase],5
    stdcall AuthLoad
    cmp eax,1
    jne failed
    cmp [authReady],0
    jne failed
    stdcall AuthEqual,authRefreshToken,newRefresh
    cmp eax,1
    jne failed

    ; A revoked Microsoft grant deletes its stale cache and invalidates ready.
    mov [scenario],2
    stdcall RunEnsure
    test eax,eax
    jnz failed
    cmp [requests],1
    jne failed
    cmp [authReady],0
    jne failed
    cmp [authHasRefresh],0
    jne failed
    cmp [lastStatus],authMsgReauth
    jne failed
    invoke GetFileAttributesW,authCachePath
    cmp eax,-1
    jne failed
    mov [phase],6

    ; A transport error leaves a retryable protected refresh credential, never
    ; the old ready flag or access token.
    stdcall SeedRefresh
    mov [scenario],3
    mov [authReady],1
    invoke lstrcpyA,authToken,mcToken
    stdcall RunEnsure
    test eax,eax
    jnz failed
    cmp [authReady],0
    jne failed
    cmp byte [authToken],0
    jne failed
    cmp [authHasRefresh],1
    jne failed
    cmp [lastStatus],authMsgNetwork
    jne failed
    mov [phase],7
    mov [scenario],4
  .api_failures:
    stdcall SeedRefresh
    stdcall RunEnsure
    test eax,eax
    jnz failed
    cmp [authReady],0
    jne failed
    cmp byte [authToken],0
    jne failed
    mov eax,[scenario]
    cmp eax,4
    jne .not_approval
    cmp [lastStatus],authMsgApproval
    jne failed
    jmp .next_api
  .not_approval:
    cmp eax,5
    jne .not_unauthorized
    cmp [lastStatus],authMsgUnauthorized
    jne failed
    jmp .next_api
  .not_unauthorized:
    cmp eax,6
    jne .not_ownership
    cmp [lastStatus],authMsgOwnFail
    jne failed
    jmp .next_api
  .not_ownership:
    cmp [lastStatus],authMsgProfile
    jne failed
  .next_api:
    inc [scenario]
    cmp [scenario],8
    jb .api_failures
    mov [phase],8

    ; Matching cache does not authorize a changed Client ID.
    stdcall SeedRefresh
    invoke lstrcpyA,clientId,otherGuid
    mov [authReady],1
    invoke lstrcpyA,authToken,mcToken
    stdcall RunEnsure
    test eax,eax
    jnz failed
    cmp [requests],0
    jne failed
    cmp [authReady],0
    jne failed
    cmp [lastStatus],authMsgNeedLogin
    jne failed
    invoke lstrcpyA,clientId,testGuid

    ; Cancellation does not reuse a formerly ready online identity.
    mov [authCancel],1
    stdcall RunEnsure
    test eax,eax
    jnz failed
    cmp [requests],0
    jne failed
    cmp [lastStatus],authMsgCancel
    jne failed
    mov [authCancel],0
    mov [phase],9

    ; Refresh responses without rotation retain the valid old refresh token.
    stdcall SeedRefresh
    mov [scenario],10
    stdcall RunEnsure
    cmp eax,1
    jne failed
    stdcall AuthEqual,authRefreshToken,oldRefresh
    cmp eax,1
    jne failed
    stdcall AuthLoad
    cmp eax,1
    jne failed
    stdcall AuthEqual,authRefreshToken,oldRefresh
    cmp eax,1
    jne failed
    mov [phase],10

    stdcall SeedRefresh
    mov [scenario],11
    stdcall RunEnsure
    test eax,eax
    jnz failed
    cmp [lastStatus],authMsgFormat
    jne failed
    cmp [authReady],0
    jne failed

    ; Logout removes the cache and wipes memory. File locking is reported.
    mov [phase],11
    stdcall SeedRefresh
    invoke CreateFileW,authCachePath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je failed
    mov [handle],eax
    stdcall AuthLogout
    cmp [lastStatus],authMsgCacheDelete
    jne failed
    cmp [authReady],0
    jne failed
    cmp [authHasRefresh],0
    jne failed
    cmp byte [authRefreshToken],0
    jne failed
    invoke CloseHandle,[handle]
    stdcall AuthLogout
    cmp [lastStatus],authMsgLogout
    jne failed
    invoke GetFileAttributesW,authCachePath
    cmp eax,-1
    jne failed
    cmp [browserCalls],0
    jne failed
    cmp [completionCalls],0
    jne failed

    ; Regression: a pre-existing fixed .part hard link must not be touched or
    ; deleted by saving/logout. It points at an unrelated seven-byte sentinel.
    mov [phase],12
    invoke lstrcpyW,oldTemporaryPath,authCachePath
    invoke lstrcatW,oldTemporaryPath,authCacheTempSuffix
    invoke lstrcpyW,sentinelPath,authCachePath
    invoke lstrcatW,sentinelPath,sentinelSuffix
    invoke CreateFileW,sentinelPath,GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je failed
    mov [handle],eax
    invoke WriteFile,eax,sentinelText,7,bytesRead,0
    test eax,eax
    jz failed
    invoke CloseHandle,[handle]
    invoke CreateHardLinkW,oldTemporaryPath,sentinelPath,0
    test eax,eax
    jz failed
    stdcall SeedRefresh
    cmp eax,1
    jne failed
    stdcall CheckSentinel
    cmp eax,1
    jne failed
    stdcall AuthDeleteCache
    cmp eax,1
    jne failed
    invoke GetFileAttributesW,oldTemporaryPath
    cmp eax,-1
    je failed

    ; CREATE_NEW collision: an existing hard link using the first unique
    ; candidate is preserved, and save retries with a distinct owned name.
    mov eax,[authCacheSerial]
    inc eax
    cinvoke wsprintfW,temporarySuffix,authCacheUniqueFormat,01357h,02468h,eax
    invoke lstrcpyW,collisionPath,authCachePath
    invoke lstrcatW,collisionPath,temporarySuffix
    invoke CreateHardLinkW,collisionPath,sentinelPath,0
    test eax,eax
    jz failed
    stdcall SeedRefresh
    cmp eax,1
    jne failed
    stdcall CheckSentinel
    cmp eax,1
    jne failed
    invoke GetFileAttributesW,collisionPath
    cmp eax,-1
    je failed
    stdcall AuthDeleteCache
    cmp eax,1
    jne failed
    invoke DeleteFileW,collisionPath
    test eax,eax
    jz failed
    invoke DeleteFileW,oldTemporaryPath
    test eax,eax
    jz failed

    ; A final cache path that is itself a hard link is rejected for save/read/
    ; logout. Its target bytes and the link both remain intact.
    mov [phase],13
    invoke CreateHardLinkW,authCachePath,sentinelPath,0
    test eax,eax
    jz failed
    stdcall SeedRefresh
    test eax,eax
    jnz failed
    stdcall AuthLoad
    test eax,eax
    jnz failed
    cmp [authHasRefresh],0
    jne failed
    stdcall AuthDeleteCache
    test eax,eax
    jnz failed
    invoke GetFileAttributesW,authCachePath
    cmp eax,-1
    je failed
    stdcall CheckSentinel
    cmp eax,1
    jne failed
    invoke DeleteFileW,authCachePath
    test eax,eax
    jz failed
    invoke DeleteFileW,sentinelPath
    test eax,eax
    jz failed

    ; A reparse flag in handle information blocks all cache operations. The
    ; flag fixture exercises refusal independently of symlink creation rights.
    mov [phase],14
    stdcall SeedRefresh
    cmp eax,1
    jne failed
    stdcall Sha1File,authCachePath,cacheHashBefore
    cmp eax,1
    jne failed
    mov [simulateReparse],1
    stdcall AuthSaveCache
    test eax,eax
    jnz failed
    stdcall AuthLoad
    test eax,eax
    jnz failed
    stdcall AuthDeleteCache
    test eax,eax
    jnz failed
    mov [simulateReparse],0
    stdcall Sha1File,authCachePath,cacheHashAfter
    cmp eax,1
    jne failed
    stdcall AuthEqual,cacheHashBefore,cacheHashAfter
    cmp eax,1
    jne failed
    stdcall AuthDeleteCache
    cmp eax,1
    jne failed

    ; An existing directory is equally rejected without modifying it.
    invoke lstrcpyW,originalCachePath,authCachePath
    invoke lstrcpyW,authCachePath,testDirectory
    stdcall SeedRefresh
    test eax,eax
    jnz failed
    stdcall AuthLoad
    test eax,eax
    jnz failed
    stdcall AuthDeleteCache
    test eax,eax
    jnz failed
    invoke lstrcpyW,authCachePath,originalCachePath
    invoke ExitProcess,0
failed:
    stdcall AuthDeleteCache
    invoke ExitProcess,[phase]

proc SeedRefresh
    mov [authCancel],0
    stdcall AuthClearIdentity
    stdcall AuthWipeRefresh
    invoke lstrcpyA,authRefreshToken,oldRefresh
    invoke lstrcpyA,authRefreshClient,testGuid
    mov [authHasRefresh],1
    stdcall AuthSaveCache
    ret
endp
proc RunEnsure
    mov [requests],0
    mov [transportFailure],0
    stdcall AuthEnsureSession
    mov [caseResult],eax
    cmp [transportFailure],0
    jne failed
    mov eax,[caseResult]
    ret
endp
proc ContainsBytes uses ebx esi edi, bytes,count,needle
    invoke lstrlenA,[needle]
    mov ebx,eax
    mov esi,[bytes]
    mov edx,[count]
  .scan:
    cmp edx,ebx
    jb .no
    mov edi,[needle]
    push esi
    mov ecx,ebx
    repe cmpsb
    pop esi
    je .yes
    inc esi
    dec edx
    jmp .scan
  .yes:
    mov eax,1
    ret
  .no:
    xor eax,eax
    ret
endp
proc SetStatusA,pText
    mov eax,[pText]
    mov [lastStatus],eax
    ret
endp
proc FixtureTime,pTime
    mov edx,[pTime]
    mov eax,dword [fixedTime]
    mov [edx],eax
    mov eax,dword [fixedTime+4]
    mov [edx+4],eax
    ret
endp
proc FixtureCachePid
    mov eax,01357h
    ret
endp
proc FixtureCacheTick
    mov eax,02468h
    ret
endp
proc FixtureInformation,file,information
    invoke GetFileInformationByHandle,[file],[information]
    test eax,eax
    jz .done
    cmp [simulateReparse],0
    je .done
    mov edx,[information]
    or dword [edx],0400h
  .done:
    ret
endp
proc CheckSentinel uses ebx
    invoke CreateFileW,sentinelPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .bad
    mov [handle],eax
    invoke GetFileSize,eax,0
    cmp eax,7
    jne .close_bad
    invoke ReadFile,[handle],sentinelBytes,7,bytesRead,0
    mov ebx,eax
    invoke CloseHandle,[handle]
    test ebx,ebx
    jz .bad
    mov byte [sentinelBytes+7],0
    stdcall AuthEqual,sentinelBytes,sentinelText
    ret
  .close_bad:
    invoke CloseHandle,[handle]
  .bad:
    xor eax,eax
    ret
endp
proc FixtureShell,a,b,c,d,e,f
    inc [browserCalls]
    mov eax,33
    ret
endp
proc FixturePost,a,b,c,d
    inc [completionCalls]
    mov eax,1
    ret
endp
proc CheckRequest,pBody,pExpected
    stdcall AuthEqual,[pBody],[pExpected]
    test eax,eax
    jnz .done
    mov [transportFailure],1
  .done:
    ret
endp
proc CheckHeader uses esi edi,header
    mov esi,[header]
    mov edi,expectedBearer
  .next:
    mov ax,[esi]
    cmp ax,[edi]
    jne .bad
    add esi,2
    add edi,2
    test ax,ax
    jnz .next
    ret
  .bad:
    mov [transportFailure],1
    ret
endp
proc SessionFree uses esi ecx, memory
    mov esi,[memory]
    cmp esi,[lastHttpPointer]
    jne .release
    mov ecx,[lastHttpSize]
  .next:
    test ecx,ecx
    jz .release
    cmp byte [esi],0
    jne .bad
    inc esi
    dec ecx
    jmp .next
  .bad:
    mov [transportFailure],1
  .release:
    stdcall FreeMem,[memory]
    ret
endp
proc SessionRequest uses ebx esi edi,url,method,headers,body
    mov ebx,[requests]
    inc [requests]
    mov [httpStatus],200
    cmp [scenario],12
    jne .normal_request
    cmp [url],authDeviceUrl
    jne .bad
    jmp .network
  .normal_request:
    cmp ebx,0
    je .refresh
    cmp ebx,1
    je .xbox
    cmp ebx,2
    je .xsts
    cmp ebx,3
    je .minecraft
    cmp ebx,4
    je .entitlement
    cmp ebx,5
    je .profile
    jmp .bad
  .refresh:
    cmp [url],authTokenUrl
    jne .bad
    stdcall AuthEqual,authRefreshToken,newRefresh
    test eax,eax
    jz .old_token
    stdcall CheckRequest,[body],expectedNewBody
    jmp .body_checked
  .old_token:
    stdcall CheckRequest,[body],expectedOldBody
  .body_checked:
    cmp [scenario],3
    je .network
    mov esi,fixtureMicrosoft
    cmp [scenario],10
    jne .not_no_rotation
    mov esi,fixtureNoRotation
  .not_no_rotation:
    cmp [scenario],2
    jne .allocate
    mov [httpStatus],400
    mov esi,fixtureInvalidGrant
    jmp .allocate
  .xbox:
    cmp [url],authXblUrl
    jne .bad
    stdcall CheckRequest,[body],expectedXboxBody
    mov esi,fixtureXbox
    jmp .allocate
  .xsts:
    cmp [url],authXstsUrl
    jne .bad
    stdcall CheckRequest,[body],expectedXstsBody
    mov esi,fixtureXsts
    jmp .allocate
  .minecraft:
    cmp [url],authMinecraftUrl
    jne .bad
    stdcall CheckRequest,[body],expectedMinecraftBody
    mov esi,fixtureMinecraft
    cmp [scenario],11
    jne .not_no_expiry
    mov esi,fixtureNoExpiry
  .not_no_expiry:
    cmp [scenario],4
    jne .not_denied
    mov [httpStatus],403
    mov esi,fixtureDenied
  .not_denied:
    cmp [scenario],5
    jne .allocate
    mov [httpStatus],401
    mov esi,fixtureUnauthorized
    jmp .allocate
  .entitlement:
    cmp [url],authEntitleUrl
    jne .bad
    stdcall CheckHeader,[headers]
    mov esi,fixtureEntitlement
    cmp [scenario],6
    jne .allocate
    mov esi,fixtureNoOwnership
    jmp .allocate
  .profile:
    cmp [url],authProfileUrl
    jne .bad
    stdcall CheckHeader,[headers]
    mov esi,fixtureProfile
    cmp [scenario],7
    jne .allocate
    mov [httpStatus],404
    mov esi,fixtureNoProfile
  .allocate:
    invoke lstrlenA,esi
    inc eax
    mov [lastHttpSize],eax
    mov ebx,eax
    invoke GetProcessHeap
    invoke HeapAlloc,eax,8,ebx
    test eax,eax
    jz .network
    mov [lastHttpPointer],eax
    mov edi,eax
    invoke lstrcpyA,edi,esi
    mov eax,edi
    ret
  .bad:
    mov [transportFailure],1
  .network:
    xor eax,eax
    ret
endp

include '../src/common.inc'
define HttpRequest SessionRequest
define GetSystemTimeAsFileTime FakeTime
define ShellExecuteW FakeShell
define PostMessageW FakePost
define FreeMem SessionFree
define GetCurrentProcessId FakeCachePid
define GetTickCount FakeCacheTick
define GetFileInformationByHandle FakeInformation
include '../src/auth.inc'
restore HttpRequest,GetSystemTimeAsFileTime,ShellExecuteW,PostMessageW,FreeMem,GetCurrentProcessId,GetTickCount,GetFileInformationByHandle
include '../src/imports.inc'
