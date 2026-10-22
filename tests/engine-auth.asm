; Integration boundary: an expired/revoked online identity must never reach
; CreateProcess, even if authReady was previously true. Uses the real engine,
; real AuthEnsureSession and JSON parser; HTTP/process creation are fixtures.
format PE console 4.0
entry start
stack 100000h,40000h
include '..\tools\INCLUDE\win32ax.inc'
WM_APP=8000h
OFFSET_ACCOUNT_SESSION=1

section '.data' data readable writeable
hWnd dd 0
memoryMb dd 4096
onlineMode dd 1
authReady dd 0
phase dd 1
processCalls dd 0
httpCalls dd 0
completionCalls dd 0
transportFailure dd 0
networkFailure dd 0
lastStatus dd 0
fixtureHandle dd 0
written dd 0
FakeCreateProcess dd FixtureCreateProcess
FakePost dd FixturePost
playerName db 'Notch',0,58 dup(0)
selectedVersion db 'auth-probe',0,117 dup(0)
fixtureDirectory du 'tests\out\engine-auth',0
fixtureCache du 'tests\out\engine-auth.account',0
fixtureGuid db '01234567-89ab-cdef-0123-456789abcdef',0
fixtureName db 'TestPlayer',0
fixtureUuid db '0123456789abcdef0123456789abcdef',0
fixtureToken db 'former-session-token',0
fixtureRefresh db 'fixture-refresh',0
fixtureInvalidGrant db '{"error":"invalid_grant"}',0
fixtureMetadata db '{"id":"auth-probe","type":"release","mainClass":"Fixture","assetIndex":{"id":"auth-probe"},"libraries":[],"arguments":{"jvm":["-cp","${classpath}","-Djava.library.path=${natives_directory}"],"game":["--username","${auth_player_name}","--uuid","${auth_uuid}","--accessToken","${auth_access_token}","--userType","${user_type}"]}}',0
fixtureMetadataLength = $-fixtureMetadata-1
fixtureClient db 'PK',3,4
gameDir rw 1024
javaPath rw 1024
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
fixtureVersionPath rw 2048
fixtureClock dq 0

section '.text' code readable executable
start:
    invoke GetFullPathNameW,fixtureDirectory,1024,gameDir,0
    test eax,eax
    jz failed
    invoke GetModuleFileNameW,0,javaPath,1024
    test eax,eax
    jz failed
    stdcall EngineInit
    invoke lstrcpyA,clientId,fixtureGuid
    invoke lstrcpyW,authCachePath,fixtureCache
    stdcall AuthDeleteCache
    stdcall EValidateVersion
    test eax,eax
    jz failed
    stdcall EBuildPaths
    invoke lstrcpyW,fixtureVersionPath,gameDir
    invoke lstrcatW,fixtureVersionPath,eVersions
    invoke lstrcatW,fixtureVersionPath,eVersionW
    stdcall EnsureDirectory,fixtureVersionPath
    test eax,eax
    jz failed
    stdcall WriteFixture,eMetaPath,fixtureMetadata,fixtureMetadataLength
    test eax,eax
    jz failed
    stdcall WriteFixture,eClientPath,fixtureClient,4
    test eax,eax
    jz failed

    ; Formerly ready, now expired: refresh rejection must block process launch.
    stdcall SeedFormerSession
    stdcall EngineLaunch,0
    test eax,eax
    jnz failed
    cmp [httpCalls],1
    jne failed
    cmp [processCalls],0
    jne failed
    cmp [authReady],0
    jne failed
    cmp [lastStatus],authMsgReauth
    jne failed
    cmp [completionCalls],1
    jne failed
    cmp [transportFailure],0
    jne failed
    mov [phase],2

    ; A valid fresh online session may reach CreateProcess. The fixture returns
    ; failure there; no executable is actually started by this test.
    stdcall ResetCounters
    stdcall SeedFormerSession
    invoke GetSystemTimeAsFileTime,fixtureClock
    mov eax,dword [fixtureClock]
    mov edx,dword [fixtureClock+4]
    add eax,1640261632
    adc edx,8 ; 3600 seconds = 36,000,000,000 FILETIME ticks
    mov dword [authSessionExpiry],eax
    mov dword [authSessionExpiry+4],edx
    stdcall EngineLaunch,0
    test eax,eax
    jnz failed
    cmp [httpCalls],0
    jne failed
    cmp [processCalls],1
    jne failed
    cmp [lastStatus],eErrStart
    jne failed
    cmp [completionCalls],1
    jne failed
    mov [phase],3

    ; Offline launch never asks the online session helper for credentials.
    stdcall ResetCounters
    stdcall AuthClearIdentity
    stdcall AuthWipeRefresh
    mov [onlineMode],0
    stdcall EngineLaunch,0
    test eax,eax
    jnz failed
    cmp [httpCalls],0
    jne failed
    cmp [processCalls],1
    jne failed
    cmp [completionCalls],1
    jne failed
    mov [phase],4

    ; An online account with no refresh credential also blocks before Java.
    stdcall ResetCounters
    mov [onlineMode],1
    stdcall EngineLaunch,0
    test eax,eax
    jnz failed
    cmp [httpCalls],0
    jne failed
    cmp [processCalls],0
    jne failed
    cmp [lastStatus],authMsgNeedLogin
    jne failed
    cmp [completionCalls],1
    jne failed
    mov [phase],5

    ; Transport failure must not fall back to the old ready identity either.
    stdcall ResetCounters
    stdcall SeedFormerSession
    mov [networkFailure],1
    stdcall EngineLaunch,0
    test eax,eax
    jnz failed
    cmp [httpCalls],1
    jne failed
    cmp [processCalls],0
    jne failed
    cmp [authReady],0
    jne failed
    cmp [lastStatus],authMsgNetwork
    jne failed
    cmp [completionCalls],1
    jne failed
    stdcall AuthWipeRefresh
    stdcall AuthDeleteCache
    invoke ExitProcess,0
failed:
    stdcall AuthDeleteCache
    invoke ExitProcess,[phase]

proc ResetCounters
    mov [httpCalls],0
    mov [processCalls],0
    mov [completionCalls],0
    mov [transportFailure],0
    mov [authCancel],0
    mov [engineCancel],0
    ret
endp
proc SeedFormerSession
    stdcall AuthClearIdentity
    invoke lstrcpyA,authName,fixtureName
    invoke lstrcpyA,authUuid,fixtureUuid
    invoke lstrcpyA,authToken,fixtureToken
    invoke lstrcpyA,authSessionClient,fixtureGuid
    invoke lstrcpyA,authRefreshToken,fixtureRefresh
    invoke lstrcpyA,authRefreshClient,fixtureGuid
    mov [authHasRefresh],1
    mov [authReady],1
    ret
endp
proc WriteFixture uses ebx,path,data,count
    invoke CreateFileW,[path],GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
    cmp eax,-1
    je .bad
    mov [fixtureHandle],eax
    invoke WriteFile,eax,[data],[count],written,0
    mov ebx,eax
    invoke CloseHandle,[fixtureHandle]
    test ebx,ebx
    jz .bad
    mov eax,[written]
    cmp eax,[count]
    jne .bad
    mov eax,1
    ret
  .bad:
    xor eax,eax
    ret
endp
proc SetStatusA,text
    mov eax,[text]
    mov [lastStatus],eax
    ret
endp
proc FixtureCreateProcess,a,b,c,d,e,f,g,h,i,j
    inc [processCalls]
    xor eax,eax
    ret
endp
proc FixturePost,window,message,wparam,lparam
    cmp [message],WM_APP+2
    jne .done
    inc [completionCalls]
  .done:
    mov eax,1
    ret
endp
proc FixtureRequest uses ebx,url,method,headers,body
    inc [httpCalls]
    cmp [url],authTokenUrl
    jne .bad
    cmp [networkFailure],1
    je .network
    mov [httpStatus],400
    invoke lstrlenA,fixtureInvalidGrant
    inc eax
    mov ebx,eax
    invoke GetProcessHeap
    invoke HeapAlloc,eax,8,ebx
    test eax,eax
    jz .network
    mov ebx,eax
    invoke lstrcpyA,eax,fixtureInvalidGrant
    mov eax,ebx
    ret
  .bad:
    mov [transportFailure],1
  .network:
    xor eax,eax
    ret
endp

include '..\src\common.inc'
include '..\src\memory.inc'
define CreateProcessW FakeCreateProcess
define PostMessageW FakePost
include '..\src\engine.inc'
restore CreateProcessW
include '..\src\mc_sources.inc'
define HttpRequest FixtureRequest
include '..\src\auth.inc'
restore HttpRequest,PostMessageW
include '..\src\imports.inc'
