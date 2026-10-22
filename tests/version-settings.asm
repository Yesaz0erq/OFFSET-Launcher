; Native per-profile persistence, defaults, bounds and context separation.
format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
WM_APP=8000h
OFFSET_VERSION_SETTINGS=1
section '.data' data readable writeable
hWnd dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion db 'one',0,124 dup(0)
memoryMb dd 4096
onlineMode dd 0
authReady dd 0
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authXuid rb 64
step dd 1
fixtureFmt du 'tests\out\version-settings-%u',0
fixtureB du '\rootB',0
fixtureFuture du '\future-root',0
fixtureJavaDir dw '\','J','a','v','a',' ',8DEFh,5F84h,0
fixtureJavaSuffix du '\java.exe',0
fixtureForeign du 'Foreign',0
fixtureKeep du 'Keep',0
fixtureValue du 'preserved',0
fixtureBadMemory du '999999999',0
fixtureBadHex du 'D800',0
fixtureFive du '5',0
fixtureOne db 'one',0
fixtureNew db 'not-installed',0
fixtureBad db '../escape',0
fixtureCon db 'CON.txt',0
fixtureBase db '1.21.1',0
fixtureOneW du '\versions\one',0
fixtureJar du '\one.jar',0
fixtureJson du '\one.json',0
fixtureIni du '\.offset-profile.ini',0
fixtureMetadata db '{"id":"one","mainClass":"Probe","arguments":{"jvm":["-cp","${classpath}"],"game":[]},"libraries":[],"assetIndex":{"id":"probe"}}',0
fixtureLoaderMetadata db '{"id":"one","offsetNormalized":true,"offsetBase":"1.21.1","mainClass":"Probe","arguments":{"jvm":["-cp","${classpath}"],"game":[]},"libraries":[{"name":"net.neoforged:neoforge:21.1.0"}],"assetIndex":{"id":"probe"}}',0
fixtureName dw 504Fh,79FBh,20h,0D83Dh,0DE00h,0
fixtureDraft du 'Unsaved draft',0
fixtureRoot rw 1024
fixtureRootB rw 1024
fixturePath rw 2048
fixtureJava rw 1024
fixtureExe rw 1024
fixtureIniPath rw 2048
fixtureData rw 2048
fixtureExpected rw 2048
fixtureValueOut rw 64
fixtureHash rb 41
fixtureHash2 rb 41
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetTickCount
 cinvoke wsprintfW,fixturePath,fixtureFmt,eax
 invoke GetFullPathNameW,fixturePath,1024,fixtureRoot,0
 stdcall EnsureDirectory,fixtureRoot
 invoke lstrcpyW,fixtureRootB,fixtureRoot
 invoke lstrcatW,fixtureRootB,fixtureB
 stdcall EnsureDirectory,fixtureRootB
 stdcall MakeFixture,fixtureRoot
 stdcall MakeFixture,fixtureRootB
 invoke lstrcpyW,fixturePath,fixtureRootB
 invoke lstrcatW,fixturePath,fixtureOneW
 invoke lstrcatW,fixturePath,fixtureJson
 stdcall WriteFixture,fixturePath,fixtureLoaderMetadata
 invoke lstrcpyW,fixtureJava,fixtureRoot
 invoke lstrcatW,fixtureJava,fixtureJavaDir
 stdcall EnsureDirectory,fixtureJava
 invoke lstrcatW,fixtureJava,fixtureJavaSuffix
 invoke GetModuleFileNameW,0,fixtureExe,1024
 invoke CopyFileW,fixtureExe,fixtureJava,1
 test eax,eax
 jz failed
 stdcall VsSafeId,fixtureBad
 test eax,eax
 jnz failed
 stdcall VsSafeId,fixtureCon
 test eax,eax
 jnz failed
 stdcall VsLoad,fixtureRootB,fixtureOne
 cmp eax,1
 jne failed
 cmp [vsLoaderKind],3
 jne failed
 invoke lstrcmpA,vsBaseVersionA,fixtureBase
 test eax,eax
 jnz failed
 inc [step]
 stdcall VsLoad,fixtureRoot,fixtureOne
 cmp eax,1
 jne failed
 cmp [vsContextValid],1
 jne failed
 cmp [vsMemory],0
 jne failed
 cmp [vsIsolate],0
 jne failed
 invoke lstrcmpiW,vsDataDirW,fixtureRoot
 test eax,eax
 jnz failed
 inc [step]
 invoke lstrcpyW,fixtureIniPath,vsProfileDirW
 invoke lstrcatW,fixtureIniPath,fixtureIni
 invoke WritePrivateProfileStringW,fixtureForeign,fixtureKeep,fixtureValue,fixtureIniPath
 invoke lstrcpyW,vsNameW,fixtureName
 invoke lstrcpyW,vsJavaW,fixtureJava
 mov [vsMemory],1024
 mov [vsIsolate],1
 stdcall VsSave
 cmp eax,1
 jne failed
 invoke lstrcpyW,fixtureExpected,vsProfileDirW
 invoke lstrcatW,fixtureExpected,vsGameSuffix
 invoke lstrcmpiW,vsDataDirW,fixtureExpected
 test eax,eax
 jnz failed
 invoke GetFileAttributesW,fixtureExpected
 cmp eax,-1
 jne failed
 inc [step]
 stdcall VsLoad,fixtureRoot,fixtureOne
 cmp eax,1
 jne failed
 invoke lstrcmpiW,vsNameW,fixtureName
 test eax,eax
 jnz failed
 invoke lstrcmpiW,vsJavaW,fixtureJava
 test eax,eax
 jnz failed
 cmp [vsMemory],1024
 jne failed
 cmp [vsIsolate],1
 jne failed
 invoke GetPrivateProfileStringW,fixtureForeign,fixtureKeep,vsEmpty,fixtureValueOut,64,fixtureIniPath
 invoke lstrcmpiW,fixtureValueOut,fixtureValue
 test eax,eax
 jnz failed
 inc [step]
 invoke lstrcpyW,vsNameW,fixtureDraft
 stdcall VsResolveData,fixtureRootB,fixtureOne,fixtureData,2048
 cmp eax,1
 jne failed
 invoke lstrcmpiW,fixtureData,fixtureRootB
 test eax,eax
 jnz failed
 invoke lstrcmpiW,vsNameW,fixtureDraft
 test eax,eax
 jnz failed
 invoke lstrcmpiW,vsRootW,fixtureRoot
 test eax,eax
 jnz failed
 inc [step]
 stdcall VsResolveData,fixtureRoot,fixtureNew,fixtureData,2048
 cmp eax,1
 jne failed
 invoke lstrcmpiW,fixtureData,fixtureRoot
 test eax,eax
 jnz failed
 stdcall VsResolveData,fixtureRoot,fixtureBad,fixtureData,2048
 test eax,eax
 jnz failed
 invoke lstrcpyW,fixturePath,fixtureRootB
 invoke lstrcatW,fixturePath,fixtureFuture
 stdcall VsResolveData,fixturePath,fixtureNew,fixtureData,2048
 cmp eax,1
 jne failed
 invoke lstrcmpiW,fixtureData,fixturePath
 test eax,eax
 jnz failed
 invoke GetFileAttributesW,fixturePath
 cmp eax,-1
 jne failed
 inc [step]
 stdcall Sha1File,fixtureIniPath,fixtureHash
 mov [vsMemory],511
 stdcall VsSave
 test eax,eax
 jnz failed
 stdcall Sha1File,fixtureIniPath,fixtureHash2
 invoke lstrcmpA,fixtureHash,fixtureHash2
 test eax,eax
 jnz failed
 inc [step]
 invoke WritePrivateProfileStringW,vsSection,vsKeyMemory,fixtureBadMemory,fixtureIniPath
 invoke WritePrivateProfileStringW,vsSection,vsKeyIsolate,fixtureFive,fixtureIniPath
 invoke WritePrivateProfileStringW,vsSection,vsKeyName,fixtureBadHex,fixtureIniPath
 invoke WritePrivateProfileStringW,vsSection,vsKeyJava,fixtureBadHex,fixtureIniPath
 stdcall VsLoad,fixtureRoot,fixtureOne
 cmp eax,1
 jne failed
 cmp [vsMemory],0
 jne failed
 cmp [vsIsolate],0
 jne failed
 cmp word [vsNameW],0
 jne failed
 cmp word [vsJavaW],0
 jne failed
 inc [step]
 mov [vsMemory],1024
 mov [vsIsolate],1
 invoke lstrcpyW,vsJavaW,fixtureJava
 stdcall VsSave
 cmp eax,1
 jne failed
 invoke lstrcpyW,vsNameW,fixtureDraft
 stdcall VsLaunchSettings,fixtureRoot,fixtureOne
 cmp eax,1
 jne failed
 cmp [memoryMb],1024
 jne failed
 cmp [eRuntimeIsolated],1
 jne failed
 invoke lstrcmpiW,eRuntimeDirW,fixtureExpected
 test eax,eax
 jnz failed
 invoke lstrcmpiW,javaPath,fixtureJava
 test eax,eax
 jnz failed
 invoke lstrcmpiW,vsNameW,fixtureDraft
 test eax,eax
 jnz failed
 invoke ExitProcess,0
failed:
 invoke ExitProcess,[step]
proc MakeFixture uses ebx,rootPath
 invoke lstrcpyW,fixturePath,[rootPath]
 invoke lstrcatW,fixturePath,fixtureOneW
 stdcall EnsureDirectory,fixturePath
 invoke lstrcatW,fixturePath,fixtureJson
 stdcall WriteFixture,fixturePath,fixtureMetadata
 invoke lstrcpyW,fixturePath,[rootPath]
 invoke lstrcatW,fixturePath,fixtureOneW
 invoke lstrcatW,fixturePath,fixtureJar
 stdcall WriteFixture,fixturePath,fixtureMetadata
 ret
endp
proc WriteFixture uses ebx,path,text
 local written:DWORD
 invoke CreateFileW,[path],GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 mov ebx,eax
 invoke lstrlenA,[text]
 invoke WriteFile,ebx,[text],eax,addr written,0
 invoke CloseHandle,ebx
 ret
endp
proc SetStatusA,text
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
include '..\src\version_settings.inc'
include '..\src\imports.inc'

