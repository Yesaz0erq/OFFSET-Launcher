; Runs the real authentication state machine + real JSON parser against an
; in-process fixture transport. Never opens a browser or sends credentials.
format PE console 4.0
entry start
include 'win32ax.inc'

section '.data' data readable writeable
hWnd dd 0
authReady dd 0
caseIndex dd 0
requestCounter dd 0
lastStatus dd 0
transportFailure dd 0
fakeClock dd 0
lastHttpSize dd 0
lastHttpPointer dd 0
FakeSleep dd FakeSleepProc
FakeTick dd FakeTickProc
FakeShell dd FakeShellProc
fixtureGuid db '01234567-89ab-cdef-0123-456789abcdef',0
expectedName db 'TestPlayer',0
expectedUuid db '0123456789abcdef0123456789abcdef',0
expectedToken db 'minecraft-token',0
expectedRefresh db 'device-refresh',0
testDirectory du 'tests\out',0
testCachePath du 'tests\out\auth-device.account',0
fixtureDevice db '{"device_code":"device+/?","user_code":"TEST-CODE","verification_uri":"https://microsoft.com/devicelogin","interval":1,"expires_in":120}',0
fixtureBadVerify db '{"device_code":"device+/?","user_code":"TEST-CODE","verification_uri":"https://attacker.invalid/login","interval":1,"expires_in":120}',0
fixtureExpiredDevice db '{"device_code":"device+/?","user_code":"TEST-CODE","verification_uri":"https://microsoft.com/devicelogin","interval":1,"expires_in":1}',0
fixturePending db '{"error":"authorization_pending"}',0
fixtureSlow db '{"error":"slow_down"}',0
fixtureMicrosoft db '{"access_token":"ms-token","refresh_token":"device-refresh"}',0
fixtureXbox db '{"Token":"xbl-token"}',0
fixtureXsts db '{"Token":"xsts-token","DisplayClaims":{"xui":[{"uhs":"12345","xid":"1234567890123456"}]}}',0
fixtureXstsNoXuid db '{"Token":"xsts-token","DisplayClaims":{"xui":[{"uhs":"12345"}]}}',0
fixtureMinecraft db '{"access_token":"minecraft-token","expires_in":3600}',0
fixtureEntitlement db '{"items":[{"name":"game_minecraft"}]}',0
fixtureProfile db '{"name":"TestPlayer","id":"0123456789abcdef0123456789abcdef"}',0
fixtureDeclined db '{"error":"authorization_declined"}',0
fixtureNoOwnership db '{"items":[]}',0
fixtureInvalidProfile db '{"name":"Bad Player","id":"0123456789abcdef0123456789abcdef"}',0
fixtureUnsafeBearer db '{"access_token":"unsafe\r\nInjected","expires_in":3600}',0
fixtureNulBearer db '{"access_token":"minecraft-token\u0000trailing","expires_in":3600}',0
fixtureXstsDenied db '{"XErr":2148916238}',0
expectedDeviceBody db 'client_id=01234567-89ab-cdef-0123-456789abcdef&scope=XboxLive.signin%20offline_access',0
expectedPollBody db 'grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code&client_id=01234567-89ab-cdef-0123-456789abcdef&device_code=device%2B%2F%3F',0
expectedXboxBody db '{"Properties":{"AuthMethod":"RPS","SiteName":"user.auth.xboxlive.com","RpsTicket":"d=ms-token"},"RelyingParty":"http://auth.xboxlive.com","TokenType":"JWT"}',0
expectedXstsBody db '{"Properties":{"SandboxId":"RETAIL","UserTokens":["xbl-token"]},"RelyingParty":"rp://api.minecraftservices.com/","TokenType":"JWT"}',0
expectedMinecraftBody db '{"identityToken":"XBL3.0 x=12345;xsts-token"}',0
expectedBearer du 'Authorization: Bearer minecraft-token',13,10,0
expectedXuid db '1234567890123456',0
authName rb 64
authUuid rb 64
authToken rb 8192
clientId rb 128

section '.text' code readable executable
start:
    invoke CreateDirectoryW,testDirectory,0
    invoke lstrcpyW,authCachePath,testCachePath
    stdcall AuthDeleteCache
    invoke lstrcpyA,clientId,fixtureGuid
  .next_case:
    mov [requestCounter],0
    mov [transportFailure],0
    mov [fakeClock],0
    mov [authCancel],0
    cmp [caseIndex],10
    jne .login
    mov [authCancel],1
  .login:
    stdcall AuthLogin,0
    cmp [transportFailure],0
    jne failed
    cmp [caseIndex],0
    je .expect_success
    cmp [caseIndex],7
    jne .expect_failure
  .expect_success:
    cmp eax,1
    jne failed
    cmp [authReady],1
    jne failed
    cmp [authHasRefresh],1
    jne failed
    cmp [authCacheOkay],1
    jne failed
    stdcall AuthEqual,authRefreshToken,expectedRefresh
    cmp eax,1
    jne failed
    cmp [requestCounter],9
    jne failed
    stdcall AuthEqual,authName,expectedName
    cmp eax,1
    jne failed
    stdcall AuthEqual,authUuid,expectedUuid
    cmp eax,1
    jne failed
    stdcall AuthEqual,authToken,expectedToken
    cmp eax,1
    jne failed
    cmp [caseIndex],7
    jne .xuid_present
    cmp byte [authXuid],0
    jne failed
    jmp .xuid_done
  .xuid_present:
    stdcall AuthEqual,authXuid,expectedXuid
    cmp eax,1
    jne failed
  .xuid_done:
    cmp [fakeClock],8000 ; 1 second pending + 1 slow_down + 6 success
    jne failed
    jmp .check_wipe
  .expect_failure:
    test eax,eax
    jnz failed
    cmp [authReady],0
    jne failed
    cmp byte [authToken],0
    jne failed
    mov eax,[caseIndex]
    cmp eax,1
    jne .not_declined
    cmp [lastStatus],authMsgToken
    jne failed
    jmp .check_wipe
  .not_declined:
    cmp eax,2
    jne .not_ownership
    cmp [lastStatus],authMsgOwnFail
    jne failed
    jmp .check_wipe
  .not_ownership:
    cmp eax,3
    jne .not_profile
    cmp [lastStatus],authMsgProfile
    jne failed
    jmp .check_wipe
  .not_profile:
    cmp eax,4
    jne .not_header
    cmp [lastStatus],authMsgFormat
    jne failed
    jmp .check_wipe
  .not_header:
    cmp eax,5
    jne .not_cancel
    cmp [lastStatus],authMsgCancel
    jne failed
    cmp [requestCounter],1
    jne failed
    jmp .check_wipe
  .not_cancel:
    cmp eax,6
    jne .not_xsts
    cmp [lastStatus],authMsgXstsFail
    jne failed
    jmp .check_wipe
  .not_xsts:
    cmp eax,8
    jne .not_browser
    cmp [lastStatus],authMsgFormat
    jne failed
    cmp [requestCounter],1
    jne failed
    jmp .check_wipe
  .not_browser:
    cmp eax,9
    jne .not_expiry
    cmp [lastStatus],authMsgExpired
    jne failed
    cmp [requestCounter],1
    jne failed
    jmp .check_wipe
  .not_expiry:
    cmp eax,11
    jne .not_nul
    cmp [lastStatus],authMsgFormat
    jne failed
    jmp .check_wipe
  .not_nul:
    cmp [lastStatus],authMsgCancel
    jne failed
    cmp [requestCounter],0
    jne failed
  .check_wipe:
    mov esi,authMsToken
    mov ecx,authSecretsEnd-authMsToken
  .wipe_byte:
    cmp byte [esi],0
    jne failed
    inc esi
    loop .wipe_byte
    inc [caseIndex]
    cmp [caseIndex],12
    jb .next_case
    stdcall AuthLogout
    invoke ExitProcess,0
failed:
    stdcall AuthDeleteCache
    mov eax,[caseIndex]
    inc eax
    invoke ExitProcess,eax

proc SetStatusA,pText
    mov eax,[pText]
    mov [lastStatus],eax
    ret
endp
proc FakeTickProc
    mov eax,[fakeClock]
    ret
endp
proc FakeSleepProc,milliseconds
    mov eax,[milliseconds]
    add [fakeClock],eax
    cmp [caseIndex],5
    jne .done
    mov [authCancel],1
  .done:
    ret
endp
proc FakeShellProc,a,b,c,d,e,f
    mov eax,33
    ret
endp
proc FixtureCheck,pBody,pExpected
    stdcall AuthEqual,[pBody],[pExpected]
    test eax,eax
    jnz .done
    mov [transportFailure],1
  .done:
    ret
endp

proc FixtureHeader uses esi edi, pHeader
    mov esi,[pHeader]
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

; Ensure every released OAuth response was cleared before returning to the heap.
proc AuthTestFree uses esi ecx, pMemory
    mov esi,[pMemory]
    cmp esi,[lastHttpPointer]
    jne .release
    mov [lastHttpPointer],0
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
    stdcall FreeMem,[pMemory]
    ret
endp

proc AuthTestRequest uses ebx esi edi, pUrl,pMethod,pHeaders,pBody
    mov ebx,[requestCounter]
    inc [requestCounter]
    mov [httpStatus],200
    cmp ebx,0
    je .device
    cmp ebx,3
    jbe .poll
    cmp ebx,4
    je .xbox
    cmp ebx,5
    je .xsts
    cmp ebx,6
    je .minecraft
    cmp ebx,7
    je .ownership
    cmp ebx,8
    je .profile
    mov [transportFailure],1
    xor eax,eax
    ret
  .device:
    cmp [pUrl],authDeviceUrl
    jne .bad_transport
    stdcall FixtureCheck,[pBody],expectedDeviceBody
    mov esi,fixtureDevice
    cmp [caseIndex],8
    jne .not_bad_verify
    mov esi,fixtureBadVerify
  .not_bad_verify:
    cmp [caseIndex],9
    jne .allocate
    mov esi,fixtureExpiredDevice
    jmp .allocate
  .poll:
    cmp [pUrl],authTokenUrl
    jne .bad_transport
    stdcall FixtureCheck,[pBody],expectedPollBody
    mov [httpStatus],400
    cmp [caseIndex],1
    jne .poll_continue
    mov esi,fixtureDeclined
    jmp .allocate
  .poll_continue:
    mov esi,fixturePending
    cmp ebx,1
    je .allocate
    mov esi,fixtureSlow
    cmp ebx,2
    je .allocate
    mov [httpStatus],200
    mov esi,fixtureMicrosoft
    jmp .allocate
  .xbox:
    cmp [pUrl],authXblUrl
    jne .bad_transport
    stdcall FixtureCheck,[pBody],expectedXboxBody
    mov esi,fixtureXbox
    jmp .allocate
  .xsts:
    cmp [pUrl],authXstsUrl
    jne .bad_transport
    stdcall FixtureCheck,[pBody],expectedXstsBody
    mov esi,fixtureXsts
    cmp [caseIndex],7
    jne .not_no_xuid
    mov esi,fixtureXstsNoXuid
  .not_no_xuid:
    cmp [caseIndex],6
    jne .allocate
    mov [httpStatus],401
    mov esi,fixtureXstsDenied
    jmp .allocate
  .minecraft:
    cmp [pUrl],authMinecraftUrl
    jne .bad_transport
    stdcall FixtureCheck,[pBody],expectedMinecraftBody
    mov esi,fixtureMinecraft
    cmp [caseIndex],11
    jne .not_nul_token
    mov esi,fixtureNulBearer
  .not_nul_token:
    cmp [caseIndex],4
    jne .allocate
    mov esi,fixtureUnsafeBearer
    jmp .allocate
  .ownership:
    cmp [pUrl],authEntitleUrl
    jne .bad_transport
    stdcall FixtureHeader,[pHeaders]
    mov esi,fixtureEntitlement
    cmp [caseIndex],2
    jne .allocate
    mov esi,fixtureNoOwnership
    jmp .allocate
  .profile:
    cmp [pUrl],authProfileUrl
    jne .bad_transport
    stdcall FixtureHeader,[pHeaders]
    mov esi,fixtureProfile
    cmp [caseIndex],3
    jne .allocate
    mov esi,fixtureInvalidProfile
    jmp .allocate
  .bad_transport:
    mov [transportFailure],1
    xor eax,eax
    ret
  .allocate:
    invoke lstrlenA,esi
    inc eax
    mov [lastHttpSize],eax
    mov ebx,eax
    invoke GetProcessHeap
    invoke HeapAlloc,eax,8,ebx
    test eax,eax
    jz .done
    mov edi,eax
    mov [lastHttpPointer],eax
    invoke lstrcpyA,edi,esi
    mov eax,edi
  .done:
    ret
endp

include '../src/common.inc'
define HttpRequest AuthTestRequest
define Sleep FakeSleep
define GetTickCount FakeTick
define ShellExecuteW FakeShell
define FreeMem AuthTestFree
include '../src/auth.inc'
restore HttpRequest,Sleep,GetTickCount,ShellExecuteW,FreeMem
include '../src/imports.inc'
