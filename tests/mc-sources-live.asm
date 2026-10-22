; Real vanilla source smoke test. All downloaded files stay in tests/out.
format PE console 4.0
entry start
stack 100000h,40000h
include '..\tools\INCLUDE\win32ax.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
gameDir du 'tests\out\mc-sources-live',0
        rw 1024-(($-gameDir)/2)
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion db '1.21.1',0,121 dup(0)
memoryMb dd 4096
onlineMode dd 0
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authReady dd 0
authXuid rb 64
liveLibrary du 'tests\out\mc-sources-live\library.jar',0
liveIndex du 'tests\out\mc-sources-live\index.json',0
liveAsset du 'tests\out\mc-sources-live\asset',0
livePrefix db 0,0,'/',0
liveNewline db 13,10,0
liveClientStatus db 'Downloading client through vanilla sources...',0
liveLibraryStatus db 'Downloading library through vanilla sources...',0
liveIndexStatus db 'Downloading asset index through vanilla sources...',0
liveAssetStatus db 'Downloading asset object through vanilla sources...',0
liveSuccess db 'PASS native mirror/official vanilla downloads and SHA1: metadata, client, library, asset index, asset object.',0
section '.text' code readable executable
start:
 stdcall EngineInit
 stdcall EValidateVersion
 test eax,eax
 jz .fail
 stdcall EBuildPaths
 stdcall EFetchMetadata
 test eax,eax
 jz .fail
 stdcall ELoadMetadata
 test eax,eax
 jz .fail
 stdcall SetStatusA,liveClientStatus
 stdcall JsonGet,[eMeta],[eRoot],eKeyDownloads
 stdcall JsonGet,[eMeta],eax,eKeyClient
 stdcall LiveDownload,[eMeta],eax,eClientPath,0
 test eax,eax
 jz .fail
 stdcall SetStatusA,liveLibraryStatus
 stdcall JsonGet,[eMeta],[eRoot],eKeyLibraries
 stdcall JsonAt,[eMeta],eax,0
 stdcall JsonGet,[eMeta],eax,eKeyDownloads
 stdcall JsonGet,[eMeta],eax,eKeyArtifact
 stdcall LiveDownload,[eMeta],eax,liveLibrary,1
 test eax,eax
 jz .fail
 stdcall SetStatusA,liveIndexStatus
 stdcall JsonGet,[eMeta],[eRoot],eKeyAssets
 stdcall LiveDownload,[eMeta],eax,liveIndex,0
 test eax,eax
 jz .fail
 stdcall EReadFile,liveIndex
 test eax,eax
 jz .fail
 mov [eIndexText],eax
 stdcall JsonParse,eax
 test eax,eax
 jz .fail
 mov [eIndex],eax
 stdcall JsonRoot,eax
 stdcall JsonGet,[eIndex],eax,eKeyObjects
 stdcall JsonValueAt,[eIndex],eax,0
 stdcall JsonGet,[eIndex],eax,eKeyHash
 stdcall JsonString,[eIndex],eax,eHash,64
 cmp eax,40
 jne .fail
 stdcall SetStatusA,liveAssetStatus
 mov ax,word [eHash]
 mov word [livePrefix],ax
 invoke lstrcpyA,eUrl,eAssetBase
 invoke lstrcatA,eUrl,livePrefix
 invoke lstrcatA,eUrl,eHash
 stdcall McDownload,eUrl,liveAsset,eHash,2
 test eax,eax
 jz .fail
 stdcall ECleanup
 stdcall SetStatusA,liveSuccess
 invoke ExitProcess,0
.fail:
 stdcall ECleanup
 invoke ExitProcess,1

proc LiveDownload uses ebx,ctx,token,path,kind
 mov ebx,[token]
 test ebx,ebx
 jz .bad
 stdcall JsonGet,[ctx],ebx,eKeyUrl
 stdcall JsonString,[ctx],eax,eUrl,4096
 test eax,eax
 jz .bad
 cmp eax,4096
 jae .bad
 stdcall JsonGet,[ctx],ebx,eKeySha
 stdcall JsonString,[ctx],eax,eHash,64
 cmp eax,40
 jne .bad
 stdcall McDownload,eUrl,[path],eHash,[kind]
 ret
.bad:
 xor eax,eax
 ret
endp

proc SetStatusA uses ebx,text
 local written:DWORD
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 mov ebx,eax
 invoke lstrlenA,[text]
 invoke WriteFile,ebx,[text],eax,addr written,0
 invoke WriteFile,ebx,liveNewline,2,addr written,0
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\imports.inc'
