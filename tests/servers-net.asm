; Pure address/count/SRV tests plus optional real loopback SLP fixtures.
format PE console 6.0
entry start
stack 200000h,100000h
include 'win32ax.inc'
include '..\src\servers_types.inc'
section '.data' data readable writeable
engineCancel dd 0
modCancel dd 0
serversCount dd 0
step dd 1
dnsCalls dd 0
dnsMode dd 0
fixturePort dd 25565
startedTick dd 0
fixtureEnvName du 'OFFSET_SERVERS_NET_FIXTURE',0
fixtureSection du 'Fixture',0
fixtureAddressFormat du '127.0.0.1:%u',0
fixtureHostFormat du 'localhost:%u',0
fixtureIpv6Format du '[::1]:%u',0
fixtureNormal du 'normal',0
fixtureFragment du 'fragment',0
fixtureHuge du 'huge',0
fixtureMalformed du 'malformed',0
fixtureBadCount du 'badcount',0
fixtureUnknown du 'unknown',0
fixtureTimeout du 'timeout',0
fixtureOffline du 'offline',0
fixtureIpv6 du 'ipv6',0
fixtureSrv du 'srv',0
fixtureCancel du 'cancel',0
fixtureKeys dd fixtureNormal,fixtureFragment,fixtureHuge,fixtureMalformed,fixtureBadCount,fixtureUnknown,fixtureTimeout,fixtureOffline,fixtureIpv6,fixtureSrv
expectedStates dd SV_ON,SV_ON,SV_UNSUPPORTED,SV_UNSUPPORTED,SV_UNSUPPORTED,SV_UNSUPPORTED,SV_UNSUPPORTED,SV_OFF,SV_ON,SV_ON
fixtureSrvHost du 'fixture-srv.test',0
fixtureSrvQuery du '_minecraft._tcp.fixture-srv.test',0
fixtureSrvTarget du '127.0.0.1.',0
fixtureSrvExpected du '127.0.0.1',0
fixtureSrvDisabled du '.',0
fixtureBracket du '[::1]:25566',0
fixtureBadPort du 'host:65536',0
fixtureBadZero du 'host:0',0
fixtureBadScheme du 'https://host',0
fixtureRawIpv6 du '::1',0
fixtureTrim du '  example.org:25567  ',0
fixtureTrimHost du 'example.org',0
fixtureCountGood db '{"players":{"online":0,"max":100}}',0
fixtureCountBad db '{"players":{"online":"0","max":100}}',0
fixtureNegative db '{"players":{"online":-1,"max":100}}',0
fixtureOverflow db '{"players":{"online":2147483648,"max":100}}',0
fixtureFloat db '{"players":{"online":1.5,"max":100}}',0
fixtureMissing db '{"players":{"max":100}}',0
fixtureEnv rw 1024
fixtureHost rw 256
fixtureResult rw 256
fixtureParsedPort dd 0
fixtureExplicit dd 0
fixtureNumeric dd 0
fixtureCancelThread dd 0
fixtureDebugFormat db 'row=%u state=%u online=%d max=%d latency=%d',13,10,0
fixtureDebug rb 256
fixtureDnsPtr dd FixtureDnsQuery
fixtureDnsFreePtr dd FixtureDnsFree
fixtureRecord dd 0,0
 dw 33,12
 dd 0,0,0,fixtureSrvTarget
 dw 0,0,25565,0
fixtureRecord2 dd 0,0
 dw 33,12
 dd 0,0,0,fixtureSrvTarget
 dw 1,0,25566,0
serversRows rb SV_SIZE*SV_CAPACITY

section '.text' code readable executable
start:
 stdcall SpParseAddress,fixtureBracket,fixtureHost,fixtureParsedPort,fixtureExplicit,fixtureNumeric
 cmp eax,1
 jne failed
 cmp [fixtureParsedPort],25566
 jne failed
 cmp [fixtureExplicit],1
 jne failed
 cmp [fixtureNumeric],1
 jne failed
 inc [step]
 stdcall SpParseAddress,fixtureTrim,fixtureHost,fixtureParsedPort,fixtureExplicit,fixtureNumeric
 cmp eax,1
 jne failed
 invoke lstrcmpiW,fixtureHost,fixtureTrimHost
 test eax,eax
 jnz failed
 cmp [fixtureParsedPort],25567
 jne failed
 inc [step]
 stdcall SpParseAddress,fixtureBadPort,fixtureHost,fixtureParsedPort,fixtureExplicit,fixtureNumeric
 test eax,eax
 jnz failed
 stdcall SpParseAddress,fixtureBadZero,fixtureHost,fixtureParsedPort,fixtureExplicit,fixtureNumeric
 test eax,eax
 jnz failed
 stdcall SpParseAddress,fixtureBadScheme,fixtureHost,fixtureParsedPort,fixtureExplicit,fixtureNumeric
 test eax,eax
 jnz failed
 stdcall SpParseAddress,fixtureRawIpv6,fixtureHost,fixtureParsedPort,fixtureExplicit,fixtureNumeric
 cmp eax,1
 jne failed
 cmp [fixtureParsedPort],25565
 jne failed
 cmp [fixtureExplicit],0
 jne failed
 cmp [fixtureNumeric],1
 jne failed
 inc [step]
 invoke lstrlenA,fixtureCountGood
 stdcall SpStatusJson,fixtureCountGood,eax,serversRows
 cmp eax,1
 jne failed
 cmp dword [serversRows+SV_ONLINE],0
 jne failed
 cmp dword [serversRows+SV_MAX],100
 jne failed
 inc [step]
 stdcall BadStatus,fixtureCountBad
 test eax,eax
 jz failed
 stdcall BadStatus,fixtureNegative
 test eax,eax
 jz failed
 stdcall BadStatus,fixtureOverflow
 test eax,eax
 jz failed
 stdcall BadStatus,fixtureFloat
 test eax,eax
 jz failed
 stdcall BadStatus,fixtureMissing
 test eax,eax
 jz failed
 inc [step]
 mov [dnsMode],1
 stdcall SpSrvLookup,fixtureSrvHost,fixtureResult,fixtureParsedPort
 cmp eax,1
 jne failed
 invoke lstrcmpiW,fixtureResult,fixtureSrvExpected
 test eax,eax
 jnz failed
 cmp [fixtureParsedPort],25565
 jne failed
 cmp [dnsCalls],1
 jne failed
 inc [step]
 mov dword [fixtureRecord],fixtureRecord2
 mov word [fixtureRecord+28],10
 stdcall SpSrvLookup,fixtureSrvHost,fixtureResult,fixtureParsedPort
 cmp eax,1
 jne failed
 cmp [fixtureParsedPort],25566
 jne failed
 mov dword [fixtureRecord],0
 mov word [fixtureRecord+28],0
 mov dword [fixtureRecord+24],fixtureSrvDisabled
 stdcall SpSrvLookup,fixtureSrvHost,fixtureResult,fixtureParsedPort
 cmp eax,-1
 jne failed
 mov dword [fixtureRecord+24],fixtureSrvTarget
 inc [step]
 invoke GetEnvironmentVariableW,fixtureEnvName,fixtureEnv,1024
 test eax,eax
 jz success
 cmp eax,1024
 jae failed
 mov [serversCount],10
 xor esi,esi
.setup:
 cmp esi,10
 jae .refresh
 invoke GetPrivateProfileIntW,fixtureSection,dword [fixtureKeys+esi*4],0,fixtureEnv
 test eax,eax
 jz failed
 imul ebx,esi,SV_SIZE
 add ebx,serversRows+SV_ADDRESS
 cmp esi,8
 je .ipv6
 cmp esi,9
 je .srv
 cmp esi,1
 je .hostname
 cinvoke wsprintfW,ebx,fixtureAddressFormat,eax
 jmp .next
.hostname:
 cinvoke wsprintfW,ebx,fixtureHostFormat,eax
 jmp .next
.ipv6:
 cinvoke wsprintfW,ebx,fixtureIpv6Format,eax
 jmp .next
.srv:
 mov word [fixtureRecord+32],ax
 invoke lstrcpyW,ebx,fixtureSrvHost
.next:
 inc esi
 jmp .setup
.refresh:
 mov [dnsCalls],0
 invoke GetTickCount
 mov [startedTick],eax
 stdcall ServersRefresh
 cmp eax,1
 jne failed
 inc [step]
 invoke GetTickCount
 sub eax,[startedTick]
 cmp eax,8500
 ja failed
 xor esi,esi
.verify:
 cmp esi,10
 jae .cancel
 imul ebx,esi,SV_SIZE
 add ebx,serversRows
 mov eax,[expectedStates+esi*4]
 cmp [ebx+SV_STATE],eax
 jne failed
 cmp eax,SV_ON
 jne .unknownCount
 cmp dword [ebx+SV_ONLINE],7
 jne failed
 cmp dword [ebx+SV_MAX],25
 jne failed
 cmp dword [ebx+SV_LATENCY],0
 jl failed
 jmp .verified
.unknownCount:
 cmp dword [ebx+SV_ONLINE],-1
 jne failed
 cmp dword [ebx+SV_MAX],-1
 jne failed
.verified:
 inc esi
 inc [step]
 jmp .verify
.cancel:
 cmp [dnsCalls],1
 jne failed
 mov [engineCancel],1
 stdcall ServersRefresh
 test eax,eax
 jnz failed
 cmp dword [serversRows+SV_STATE],SV_CANCELLED
 jne failed
 mov [engineCancel],0
 inc [step]
 ; Cancellation while a connected server withholds its response is prompt.
 mov [serversCount],1
 invoke GetPrivateProfileIntW,fixtureSection,fixtureCancel,0,fixtureEnv
 test eax,eax
 jz failed
 cinvoke wsprintfW,serversRows+SV_ADDRESS,fixtureAddressFormat,eax
 invoke CreateThread,0,0,FixtureCancelWorker,0,0,0
 test eax,eax
 jz failed
 mov [fixtureCancelThread],eax
 invoke GetTickCount
 mov [startedTick],eax
 stdcall ServersRefresh
 test eax,eax
 jnz failed
 invoke GetTickCount
 sub eax,[startedTick]
 cmp eax,1500
 ja failed
 cmp dword [serversRows+SV_STATE],SV_CANCELLED
 jne failed
 invoke WaitForSingleObject,[fixtureCancelThread],2000
 invoke CloseHandle,[fixtureCancelThread]
 mov [engineCancel],0
success:
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[step]
proc BadStatus,text
 invoke lstrlenA,[text]
 stdcall SpStatusJson,[text],eax,serversRows
 test eax,eax
 setz al
 movzx eax,al
 ret
endp
proc FixtureCancelWorker,param
 invoke Sleep,150
 mov [engineCancel],1
 xor eax,eax
 ret
endp
proc DumpRows uses ebx esi edi
 local written:DWORD
 xor esi,esi
.loop:
 cmp esi,[serversCount]
 jae .done
 imul edi,esi,SV_SIZE
 add edi,serversRows
 cinvoke wsprintfA,fixtureDebug,fixtureDebugFormat,esi,dword [edi+SV_STATE],dword [edi+SV_ONLINE],dword [edi+SV_MAX],dword [edi+SV_LATENCY]
 mov ebx,eax
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 invoke WriteFile,eax,fixtureDebug,ebx,addr written,0
 inc esi
 jmp .loop
.done:
 ret
endp
proc FixtureDnsQuery uses ebx,query,kind,options,server,records,reserved
 inc [dnsCalls]
 invoke lstrcmpiW,[query],fixtureSrvQuery
 test eax,eax
 jnz .bad
 cmp [dnsMode],1
 jne .bad
 mov eax,[records]
 mov dword [eax],fixtureRecord
 xor eax,eax
 ret
.bad:
 mov eax,9003
 ret
endp
proc FixtureDnsFree,records,kind
 ret
endp
include '..\src\common.inc'
define DnsQuery_W fixtureDnsPtr
define DnsRecordListFree fixtureDnsFreePtr
include '..\src\servers_net.inc'
restore DnsQuery_W
restore DnsRecordListFree
include '..\src\imports.inc'



