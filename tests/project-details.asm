; Real project parser + plain-text renderer, fixture HTTP only.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
include '..\src\mod_types.inc'
section '.data' data readable writeable
hWnd dd 0
engineCancel dd 0
gameDir rw 1024
step dd 1
fixtureCase dd 0
calls dd 0
requestBad dd 0
expectedId db 'Project1',0
cfId db '123',0
key db 'fixture-key',0
badKey db 'fixture',13,10,'X: bad',0
mrProject db '{"id":"Project1","project_type":"mod","title":"Fixture project","description":"Summary","body":"# Title\nHello **world**. [Read more](https://example.invalid/)\n<script>evil()</script>Safe","downloads":42,"license":{"id":"MIT"}}',0
mrWrong db '{"id":"WrongID","project_type":"mod","title":"Wrong","body":"Body"}',0
mrNul db '{"id":"Project1","project_type":"mod","title":"Alias\u0000tail","body":"Body"}',0
cfProject db '{"data":{"id":123,"gameId":432,"classId":6,"name":"CF project","summary":"Summary","downloadCount":8,"authors":[{"name":"Tester"}]}}',0
cfDescription db '{"data":"<h2>Heading</h2><p>One &amp; two.</p><script>bad()</script><style>x</style><p>Safe &#20013;&#x6587;.</p>"}',0
malformed db '{"data":BROKEN}',0
html db '<h1 class="x>y">Hello</h1><p>A &amp; B &lt; C</p><ScRiPt>private()</sCrIpT><STYLE>hidden</STYLE><p>&#20013;&#x6587; &#x1F600;</p>',0
htmlExpected db 'Hello',13,10,'A & B < C',13,10,'中文 😀',13,10,0
markdown db '# Title',10,'Hello **world**. [Read](https://invalid/) and ![image](https://invalid/a.png).',0
markdownExpected db ' Title',13,10,'Hello world. Read and image.',0
query db '机械 动力 &Sodium',0
emptyQuery db 0
mcmodExpected du 'https://search.mcmod.cn/s?key=%E6%9C%BA%E6%A2%B0%20%E5%8A%A8%E5%8A%9B%20%26Sodium',0
bodyExpected db ' Title',13,10,'Hello world. Read more',13,10,'Safe',0
cfExpected db 'Heading',13,10,'One & two.',13,10,'Safe 中文.',13,10,0
suppressed db '<script>A</style>B</script>Safe',0
safeExpected db 'Safe',0
largeStart db '{"id":"Project1","project_type":"mod","title":"Large","body":"',0
largeEnd db '"}',0
outText rb 131072
largeResponse rb 133000
outUrl rw 4096
requestUrl rb 8192

section '.text' code readable executable
start:
 stdcall ModsInit
 stdcall DetailPlain,html,outText,131072
 cmp eax,-1
 je fail
 invoke lstrcmpA,outText,htmlExpected
 test eax,eax
 jnz fail
 inc [step]
 stdcall DetailPlain,markdown,outText,131072
 invoke lstrcmpA,outText,markdownExpected
 test eax,eax
 jnz fail
 inc [step]
 stdcall DetailPlain,html,outText,8
 cmp eax,-1
 jne fail
 cmp byte [outText],0
 jne fail
 inc [step]
 stdcall McmodSearchUrl,query,outUrl,4096
 cmp eax,1
 jne fail
 invoke lstrcmpiW,outUrl,mcmodExpected
 test eax,eax
 jnz fail
 inc [step]
 stdcall McmodSearchUrl,query,outUrl,4
 test eax,eax
 jnz fail
 cmp word [outUrl],0
 jne fail
 inc [step]
 stdcall ProjectFetchDetails,0,expectedId,0
 cmp eax,1
 jne fail
 cmp [detailReady],1
 jne fail
 invoke lstrcmpA,detailBody,bodyExpected
 test eax,eax
 jnz fail
 inc [step]
 mov [fixtureCase],1
 stdcall ProjectFetchDetails,0,expectedId,0
 test eax,eax
 jnz fail
 cmp [detailReady],0
 jne fail
 inc [step]
 mov [fixtureCase],2
 stdcall ProjectFetchDetails,0,expectedId,0
 test eax,eax
 jnz fail
 inc [step]
 mov [fixtureCase],3
 invoke lstrcpyA,curseApiKey,key
 stdcall ProjectFetchDetails,1,cfId,0
 cmp eax,1
 jne fail
 invoke lstrcmpA,detailBody,cfExpected
 test eax,eax
 jnz fail
 inc [step]
 mov esi,cfHeadersA
 mov ecx,1024
.checkA:
 lodsb
 test al,al
 jnz fail
 loop .checkA
 mov esi,cfHeadersW
 mov ecx,2048
.checkW:
 lodsb
 test al,al
 jnz fail
 loop .checkW
 inc [step]
 mov ebx,[calls]
 invoke lstrcpyA,curseApiKey,badKey
 stdcall ProjectFetchDetails,1,cfId,0
 test eax,eax
 jnz fail
 cmp [calls],ebx
 jne fail
 inc [step]
 mov [engineCancel],1
 stdcall ProjectFetchDetails,0,expectedId,0
 test eax,eax
 jnz fail
 cmp [calls],ebx
 jne fail
 inc [step]
 mov [engineCancel],0
 mov [fixtureCase],4
 stdcall ProjectFetchDetails,0,expectedId,0
 test eax,eax
 jnz fail
 cmp [requestBad],0
 jne fail
 inc [step]
 stdcall DetailPlain,suppressed,outText,131072
 invoke lstrcmpA,outText,safeExpected
 test eax,eax
 jnz fail
 inc [step]
 invoke lstrcpyA,largeResponse,largeStart
 invoke lstrlenA,largeResponse
 lea edi,[largeResponse+eax]
 mov ecx,132000
 mov al,'a'
 rep stosb
 invoke lstrcpyA,edi,largeEnd
 mov [fixtureCase],5
 stdcall ProjectFetchDetails,0,expectedId,0
 test eax,eax
 jnz fail
 cmp [detailReady],0
 jne fail
 inc [step]
 mov edi,largeResponse
 mov ecx,131000
 mov al,'a'
 rep stosb
 mov byte [edi],0
 stdcall DetailPlain,largeResponse,outText,131072
 cmp eax,131000
 jne fail
 invoke ExitProcess,0
fail:
 invoke ExitProcess,[step]
proc SetStatusA,text
 ret
endp
proc FixtureRequest uses ebx esi edi,url,method,headers,body
 inc [calls]
 invoke WideCharToMultiByte,65001,0,[url],-1,requestUrl,8192,0,0
 mov [httpStatus],200
 mov esi,mrProject
 cmp [fixtureCase],1
 je .wrong
 cmp [fixtureCase],2
 je .nul
 cmp [fixtureCase],3
 je .cf
 cmp [fixtureCase],4
 je .bad
 cmp [fixtureCase],5
 je .large
 jmp .copy
.wrong: mov esi,mrWrong
 jmp .copy
.nul: mov esi,mrNul
 jmp .copy
.bad: mov esi,malformed
 jmp .copy
.large: mov esi,largeResponse
 jmp .copy
.cf:
 mov esi,cfProject
 invoke lstrlenA,requestUrl
 cmp eax,12
 jb .error
 lea eax,[requestUrl+eax-12]
 invoke lstrcmpA,eax,dqDescriptionSuffix
 test eax,eax
 jnz .copy
 mov esi,cfDescription
.copy:
 invoke lstrlenA,esi
 inc eax
 mov ebx,eax
 invoke GetProcessHeap
 invoke HeapAlloc,eax,0,ebx
 mov edi,eax
 invoke lstrcpyA,edi,esi
 mov eax,edi
 ret
.error:
 mov [requestBad],1
 xor eax,eax
 ret
endp
include '..\src\common.inc'
include '..\src\mods.inc'
define HttpRequest FixtureRequest
include '..\src\modrinth.inc'
include '..\src\curseforge.inc'
include '..\src\project_details.inc'
restore HttpRequest
include '..\src\imports.inc'
