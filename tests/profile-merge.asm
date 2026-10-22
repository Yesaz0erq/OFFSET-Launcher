format PE console 6.0
entry start
stack 100000h,40000h
include '..\tools\INCLUDE\win32ax.inc'
WM_APP=8000h
section '.data' data readable writeable
hWnd dd 0
gameDir rw 1024
javaPath rw 1024
playerName db 'Notch',0,58 dup(0)
selectedVersion db 'base',0,123 dup(0)
memoryMb dd 4096
onlineMode dd 0
clientId rb 128
authName rb 64
authUuid rb 64
authToken rb 8192
authReady dd 0
authXuid rb 64
fixtureRoot du 'tests\out\profile-merge',0
fixtureBaseDir du 'tests\out\profile-merge\versions\base',0
fixtureChildDir du 'tests\out\profile-merge\versions\child',0
fixtureBaseJson du 'tests\out\profile-merge\versions\base\base.json',0
fixtureBaseJar du 'tests\out\profile-merge\versions\base\base.jar',0
fixtureChildJson du 'tests\out\profile-merge\versions\child\child.json',0
fixtureBackup du 'tests\out\profile-merge\versions\child\offset-original.json',0
fixtureProcessor du 'tests\out\profile-merge\libraries\org\example\processor\1\processor-1.jar',0
fixtureBaseId db 'base',0
fixtureChildId db 'child',0
fixtureWrong db 'wrong',0
fixtureBadId db '../escape',0
fixtureJar db 'abc',0
fixtureSha db 'a9993e364706816aba3e25717850c26c9cd0d89d',0
fixtureSentinel db 'preserve-this-backup',0
fixtureParent db '{"id":"base","type":"release","mainClass":"vanilla.Main","downloads":{"client":{"url":"https://example.invalid/client.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}},"assetIndex":{"id":"test"},"arguments":{"jvm":["-cp","${classpath}"],"game":["--parent"]},"libraries":[{"name":"org.example:keep:1","downloads":{"artifact":{"path":"org/example/keep/1/keep-1.jar","url":"https://example.invalid/keep.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}}},{"name":"org.example:override:1","downloads":{"artifact":{"path":"org/example/override/1/override-1.jar","url":"https://example.invalid/override.jar","sha1":"a9993e364706816aba3e25717850c26c9cd0d89d"}}}]}',0
fixtureChild db '{"id":"child","inheritsFrom":"base","mainClass":"loader.Main","arguments":{"jvm":["-Dloader=true"],"game":["--child"]},"libraries":[{"name":"org.example:override:2","url":"https://maven.fabricmc.net/"},{"name":"org.example:processor:1","downloads":{"artifact":{"path":"org/example/processor/1/processor-1.jar","url":"","sha1":""}}}]}',0
fixtureMavenBad db 'org.example:bad:../escape',0
fixtureBadHashChild db '{"id":"child","mainClass":"loader.Main","libraries":[{"name":"org.example:processor:1","downloads":{"artifact":{"path":"org/example/processor/1/processor-1.jar","url":"","sha1":"bad"}}}]}',0
fixtureMavenGood db 'net.fabricmc:sponge-mixin:0.17.0+mixin.0.8.7',0
fixtureClassified db 'org.example:artifact:2:natives-windows@jar',0
expectedMain db 'loader.Main',0
expectedNewName db 'org.example:override:2',0
expectedKeepName db 'org.example:keep:1',0
moduleBootstrap db 'cpw/mods/bootstraplauncher/2/bootstraplauncher-2.jar',0
moduleBootstrapNear db 'cpw/mods/bootstraplauncher/2/bootstraplauncher-2.jar.extra',0
moduleAsm db 'org/ow2/asm/asm/9/asm-9.jar',0
moduleAsmOther db 'org/ow2/asm/asm-analysis/9/asm-analysis-9.jar',0
moduleOld db 'old.jar',0
moduleNew db 'new.jar',0
moduleLinux db 'linux.jar',0
moduleJson db '{"arguments":{"jvm":["-p","${library_directory}/cpw/mods/bootstraplauncher/2/bootstraplauncher-2.jar${classpath_separator}${library_directory}\\org\\ow2\\asm\\asm\\9\\asm-9.jar"]}}',0
moduleLiteralJson db '{"arguments":{"jvm":["--module-path=./libraries/org/ow2/asm/asm/9/asm-9.jar;libraries/cpw/mods/bootstraplauncher/2/bootstraplauncher-2.jar"]}}',0
moduleRuleJson db '{"arguments":{"jvm":[{"rules":[{"action":"allow","os":{"name":"linux"}}],"value":["-p","${library_directory}/linux.jar"]},{"rules":[{"action":"allow","os":{"name":"windows"}}],"value":["--module-path","${library_directory}/org/ow2/asm/asm/9/asm-9.jar"]}]}}',0
moduleLastJson db '{"arguments":{"jvm":["--module-path","${library_directory}/old.jar","-p","${library_directory}/new.jar"]}}',0
moduleNoFlagJson db '{"arguments":{"jvm":["-DlibraryDirectory=${library_directory}/org/ow2/asm/asm/9/asm-9.jar"]}}',0
moduleBadPrefixJson db '{"arguments":{"jvm":["--module-path=notlibraries/org/ow2/asm/asm/9/asm-9.jar"]}}',0
moduleGamePrefixJson db '{"arguments":{"jvm":["--module-path","${game_directory}/libraries/ORG/OW2/ASM/asm/9/ASM-9.jar"]}}',0
moduleCases dd moduleJson,moduleBootstrap,1, moduleJson,moduleAsm,1
 dd moduleJson,moduleBootstrapNear,0, moduleJson,moduleAsmOther,0
 dd moduleLiteralJson,moduleBootstrap,1, moduleLiteralJson,moduleAsm,1
 dd moduleLiteralJson,moduleAsmOther,0, moduleRuleJson,moduleAsm,1
 dd moduleRuleJson,moduleLinux,0, moduleLastJson,moduleOld,0
 dd moduleLastJson,moduleNew,1, moduleNoFlagJson,moduleAsm,0
 dd moduleBadPrefixJson,moduleAsm,0, moduleGamePrefixJson,moduleAsm,1
moduleCaseCount = ($-moduleCases)/12
moduleClasspathJson db '{"arguments":{"jvm":["-p","${library_directory}/org/example/module/1/module-1.jar"]},"libraries":[{"downloads":{"artifact":{"path":"org/example/module/1/module-1.jar"}}},{"downloads":{"artifact":{"path":"org/example/keep/1/keep-1.jar"}}}]}',0
moduleJarPath du 'tests\out\profile-merge\libraries\org\example\module\1\module-1.jar',0
moduleKeepPath du 'tests\out\profile-merge\libraries\org\example\keep\1\keep-1.jar',0
moduleExpectedClasspath db 'libraries\org\example\keep\1\keep-1.jar;versions\base\base.jar',0
testParent dd 0
testChild dd 0
testMerged dd 0
testCtx dd 0
testRoot dd 0
testNo dd 0
requestCalls dd 0
downloadCalls dd 0
liveMode dd 0
liveGameEnv du 'OFFSET_PROFILE_GAME',0
liveChildEnv du 'OFFSET_PROFILE_CHILD',0
liveBaseEnv du 'OFFSET_PROFILE_BASE',0
livePrepareEnv du 'OFFSET_PROFILE_PREPARE',0
liveJavaEnv du 'OFFSET_TEST_JAVA',0
liveChild rb 128
liveBase rb 128
liveText rw 1024
testValue rb 4096
statusBuffer rb 512
section '.text' code readable executable
start:
 stdcall EngineInit
 invoke GetEnvironmentVariableW,liveGameEnv,liveText,1024
 test eax,eax
 jnz .live
 invoke GetFullPathNameW,fixtureRoot,1024,gameDir,0
 stdcall EnsureDirectory,fixtureBaseDir
 stdcall EnsureDirectory,fixtureChildDir
 stdcall FixtureWrite,fixtureBaseJson,fixtureParent
 stdcall FixtureWrite,fixtureBaseJar,fixtureJar
 stdcall FixtureWrite,fixtureChildJson,fixtureChild
 stdcall FixtureWrite,fixtureProcessor,fixtureJar
 inc [testNo]
 stdcall PMavenParse,fixtureMavenBad
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall PMavenParse,fixtureMavenGood
 test eax,eax
 jz failed
 inc [testNo]
 stdcall PMavenParse,fixtureClassified
 test eax,eax
 jz failed
 stdcall JsonParse,fixtureParent
 mov [testParent],eax
 stdcall JsonParse,fixtureChild
 mov [testChild],eax
 inc [testNo]
 stdcall ProfileMerge,[testParent],[testChild],fixtureBadId
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall ProfileMerge,[testParent],[testChild],fixtureChildId
 test eax,eax
 jz failed
 mov [testMerged],eax
 inc [testNo]
 cmp [requestCalls],1
 jne failed
 cmp [downloadCalls],2
 jne failed
 stdcall JsonParse,[testMerged]
 mov [testCtx],eax
 test eax,eax
 jz failed
 stdcall JsonRoot,eax
 mov [testRoot],eax
 inc [testNo]
 stdcall JsonGet,[testCtx],[testRoot],pInheritKey
 test eax,eax
 jnz failed
 stdcall JsonGet,[testCtx],[testRoot],pNormalizedKey
 stdcall JsonBool,[testCtx],eax
 cmp eax,1
 jne failed
 stdcall JsonGet,[testCtx],[testRoot],pBaseKey
 stdcall JsonString,[testCtx],eax,testValue,4096
 invoke lstrcmpA,testValue,fixtureBaseId
 test eax,eax
 jnz failed
 stdcall JsonGet,[testCtx],[testRoot],pMainKey
 stdcall JsonString,[testCtx],eax,testValue,4096
 invoke lstrcmpA,testValue,expectedMain
 test eax,eax
 jnz failed
 inc [testNo]
 stdcall JsonGet,[testCtx],[testRoot],pArgsKey
 stdcall JsonGet,[testCtx],eax,pJvmKey
 stdcall JsonCount,[testCtx],eax
 cmp eax,3
 jne failed
 inc [testNo]
 stdcall JsonGet,[testCtx],[testRoot],pArgsKey
 stdcall JsonGet,[testCtx],eax,pGameKey
 stdcall JsonCount,[testCtx],eax
 cmp eax,2
 jne failed
 inc [testNo]
 stdcall JsonGet,[testCtx],[testRoot],pLibsKey
 mov ebx,eax
 stdcall JsonCount,[testCtx],eax
 cmp eax,3
 jne failed
 stdcall JsonAt,[testCtx],ebx,0
 stdcall JsonGet,[testCtx],eax,pNameKey
 stdcall JsonString,[testCtx],eax,testValue,4096
 invoke lstrcmpA,testValue,expectedKeepName
 test eax,eax
 jnz failed
 stdcall JsonAt,[testCtx],ebx,1
 stdcall JsonGet,[testCtx],eax,pNameKey
 stdcall JsonString,[testCtx],eax,testValue,4096
 invoke lstrcmpA,testValue,expectedNewName
 test eax,eax
 jnz failed
 stdcall JsonAt,[testCtx],ebx,2
 stdcall JsonGet,[testCtx],eax,pDownloadsKey
 stdcall JsonGet,[testCtx],eax,pArtifactKey
 stdcall JsonGet,[testCtx],eax,pShaKey
 stdcall JsonString,[testCtx],eax,testValue,4096
 invoke lstrcmpA,testValue,fixtureSha
 test eax,eax
 jnz failed
 stdcall JsonFree,[testCtx]
 stdcall FreeMem,[testMerged]
 inc [testNo]
 stdcall JsonParse,fixtureBadHashChild
 mov [testCtx],eax
 stdcall ProfileMerge,[testParent],[testCtx],fixtureChildId
 test eax,eax
 jnz failed
 stdcall JsonFree,[testCtx]
 stdcall JsonFree,[testParent]
 stdcall JsonFree,[testChild]
 inc [testNo]
 stdcall ProfileNormalize,fixtureChildId,fixtureBaseId
 test eax,eax
 jz failed
 stdcall EReadFile,fixtureChildJson
 test eax,eax
 jz failed
 mov [testMerged],eax
 stdcall JsonParse,eax
 mov [testCtx],eax
 stdcall JsonRoot,eax
 stdcall JsonGet,[testCtx],eax,pInheritKey
 test eax,eax
 jnz failed
 stdcall JsonFree,[testCtx]
 stdcall FreeMem,[testMerged]
 inc [testNo]
 stdcall ProfileNormalize,fixtureChildId,fixtureBaseId
 test eax,eax
 jz failed
 inc [testNo]
 stdcall ProfileNormalize,fixtureChildId,fixtureWrong
 test eax,eax
 jnz failed
 inc [testNo]
 ; Preserve an existing backup even when replacing a new raw installer profile.
 stdcall FixtureWrite,fixtureBackup,fixtureSentinel
 stdcall FixtureWrite,fixtureChildJson,fixtureChild
 stdcall ProfileNormalize,fixtureChildId,fixtureBaseId
 test eax,eax
 jz failed
 stdcall EReadFile,fixtureBackup
 mov [testMerged],eax
 invoke lstrcmpA,eax,fixtureSentinel
 test eax,eax
 jnz failed
 stdcall FreeMem,[testMerged]
 inc [testNo]
 stdcall FixtureWrite,fixtureChildJson,fixtureChild
 stdcall ProfileNormalize,fixtureChildId,fixtureWrong
 test eax,eax
 jnz failed
 inc [testNo]
 mov [engineCancel],1
 stdcall ProfileNormalize,fixtureChildId,fixtureBaseId
 test eax,eax
 jnz failed
 mov [engineCancel],0
 ; Module declarations exclude only their exact files from the relative
 ; classpath. Conditions, both path separators and last-option semantics apply.
 mov esi,moduleCases
 mov edi,moduleCaseCount
.modules:
 inc [testNo]
 stdcall JsonParse,dword [esi]
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall EModuleOnly,dword [esi+4]
 cmp eax,[esi+8]
 jne failed
 stdcall JsonFree,[eMeta]
 mov [eMeta],0
 add esi,12
 dec edi
 jnz .modules
 inc [testNo]
 stdcall FixtureWrite,moduleJarPath,fixtureJar
 stdcall FixtureWrite,moduleKeepPath,fixtureJar
 stdcall JsonParse,moduleClasspathJson
 mov [eMeta],eax
 stdcall JsonRoot,eax
 mov [eRoot],eax
 stdcall EValidateVersion
 stdcall EBuildPaths
 stdcall EBuildClasspath
 test eax,eax
 jz failed
 invoke lstrcmpA,eClasspath,moduleExpectedClasspath
 test eax,eax
 jnz failed
 stdcall JsonFree,[eMeta]
 mov [eMeta],0
 invoke ExitProcess,0
.live:
 invoke lstrcpyW,gameDir,liveText
 invoke GetEnvironmentVariableW,liveChildEnv,liveText,1024
 test eax,eax
 jz .liveBad
 invoke WideCharToMultiByte,65001,0,liveText,-1,liveChild,128,0,0
 test eax,eax
 jz .liveBad
 invoke GetEnvironmentVariableW,liveBaseEnv,liveText,1024
 test eax,eax
 jz .liveBad
 invoke WideCharToMultiByte,65001,0,liveText,-1,liveBase,128,0,0
 test eax,eax
 jz .liveBad
 mov [liveMode],1
 stdcall ProfileNormalize,liveChild,liveBase
 test eax,eax
 jz .liveBadProfile
 invoke GetEnvironmentVariableW,livePrepareEnv,liveText,1024
 test eax,eax
 jz .liveGood
 invoke GetEnvironmentVariableW,liveJavaEnv,javaPath,1024
 invoke lstrcpyA,selectedVersion,liveChild
 stdcall EValidateVersion
 test eax,eax
 jz .liveBadProfile
 stdcall EBuildPaths
 stdcall ELoadMetadata
 test eax,eax
 jz .liveBadProfile
 stdcall EPrepareLaunch
 test eax,eax
 jz .livePrepareBad
.liveGood:
 stdcall ECleanup
 invoke ExitProcess,0
.liveBadProfile:
 stdcall ECleanup
 invoke ExitProcess,1
.livePrepareBad:
 stdcall ECleanup
 invoke ExitProcess,3
.liveBad:
 invoke ExitProcess,2
failed:
 mov eax,[testNo]
 add eax,40
 invoke ExitProcess,eax

proc FixtureWrite uses ebx esi edi,pathW,textA
 locals
  parent rw 2048
  f dd ?
  wrote dd ?
 endl
 invoke lstrcpyW,addr parent,[pathW]
 lea esi,[parent]
 xor edi,edi
.find:
 lodsw
 test ax,ax
 jz .found
 cmp ax,92
 jne .find
 lea edi,[esi-2]
 jmp .find
.found:
 test edi,edi
 jz .open
 mov word [edi],0
 stdcall EnsureDirectory,addr parent
.open:
 invoke CreateFileW,[pathW],GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0
 cmp eax,-1
 je .bad
 mov [f],eax
 invoke lstrlenA,[textA]
 mov ebx,eax
 invoke WriteFile,[f],[textA],ebx,addr wrote,0
 mov esi,eax
 invoke CloseHandle,[f]
 mov eax,esi
 ret
.bad:
 xor eax,eax
 ret
endp

proc FixtureRequest,urlW,method,headers,body
 cmp [liveMode],0
 je .fixture
 stdcall HttpRequest,[urlW],[method],[headers],[body]
 ret
.fixture:
 inc [requestCalls]
 mov [httpStatus],200
 invoke GetProcessHeap
 invoke HeapAlloc,eax,HEAP_ZERO_MEMORY,41
 push eax
 invoke lstrcpyA,eax,fixtureSha
 pop eax
 ret
endp

proc FixtureDownload,urlA,pathW,expected,kind
 cmp [liveMode],0
 je .fixture
 stdcall McDownload,[urlA],[pathW],[expected],[kind]
 ret
.fixture:
 inc [downloadCalls]
 stdcall FixtureWrite,[pathW],fixtureJar
 stdcall VerifySha1,[pathW],[expected]
 ret
endp
proc SetStatusA uses ebx,text
 local wrote:DWORD
 cmp [liveMode],0
 je .done
 invoke GetStdHandle,STD_OUTPUT_HANDLE
 mov ebx,eax
 invoke lstrlenA,[text]
 invoke WriteFile,ebx,[text],eax,addr wrote,0
.done:
 ret
endp
include '..\src\common.inc'
include '..\src\memory.inc'
include '..\src\engine.inc'
include '..\src\mc_sources.inc'
define HttpRequest FixtureRequest
define McDownload FixtureDownload
include '..\src\profiles.inc'
restore HttpRequest
restore McDownload
include '..\src\imports.inc'
