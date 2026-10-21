; UI actions select a version explicitly; simply opening its page is read-only.
section '.vmactd' data readable writeable
vmActionResult dd 0
homeVersionMemory dd 4096
homeVersionJavaW rw 1024
homeVersionRecord rb VS_SIZE
homeVersionIdA rb 128

section '.vmactc' code readable executable
 nop
proc RefreshHomeVersionSettings
 cmp [busy],0
 jne .done
 mov eax,[memoryMb]
 mov [homeVersionMemory],eax
 invoke lstrcpyW,homeVersionJavaW,javaPath
 stdcall VsRead,gameDir,selectedVersion,homeVersionRecord,0
 test eax,eax
 jz .done
 mov eax,dword [homeVersionRecord+VS_MEMORY]
 test eax,eax
 jz .java
 mov [homeVersionMemory],eax
.java:
 cmp word [homeVersionRecord+VS_JAVA],0
 je .done
 invoke lstrcpyW,homeVersionJavaW,homeVersionRecord+VS_JAVA
.done:
 ret
endp
proc HomeVersionCommand,wparam
 cmp [currentPage],0
 jne .done
 cmp [busy],0
 jne .done
 mov eax,[wparam]
 mov edx,eax
 shr edx,16
 and eax,0FFFFh
 cmp eax,101
 jne .done
 cmp edx,CBN_SELCHANGE
 je .selection
 cmp edx,CBN_EDITCHANGE
 jne .done
 invoke GetWindowTextA,[hVersion],selectedVersion,128
 jmp .refresh
.selection:
 invoke SendMessageW,[hVersion],CB_GETCURSEL,0,0
 cmp eax,-1
 je .done
 invoke SendMessageA,[hVersion],CB_GETLBTEXT,eax,homeVersionIdA
 invoke lstrcpyA,selectedVersion,homeVersionIdA
.refresh:
 stdcall RefreshHomeVersionSettings
 invoke InvalidateRect,[hWnd],0,0
.done:
 ret
endp
proc VersionPageUse uses ebx esi edi
 locals
  oldRoot rw 1024
  oldId rb 128
 endl
 cmp [busy],0
 jne .bad
 stdcall PullSettings
 test eax,eax
 jz .bad
 stdcall VsLoad,vmRootW,vmIdA
 test eax,eax
 jz .bad
 invoke lstrcpyW,addr oldRoot,gameDir
 invoke lstrcpyA,addr oldId,selectedVersion
 invoke lstrcpyW,gameDir,vmRootW
 invoke SetWindowTextW,[hGameDir],gameDir
 invoke lstrcpyA,selectedVersion,vmIdA
 stdcall EngineScanVersions,[hVersion]
 invoke SetWindowTextA,[hVersion],selectedVersion
 stdcall SaveSettings
 test eax,eax
 jnz .selected
 invoke lstrcpyW,gameDir,addr oldRoot
 invoke lstrcpyA,selectedVersion,addr oldId
 invoke SetWindowTextW,[hGameDir],gameDir
 stdcall EngineScanVersions,[hVersion]
 invoke SetWindowTextA,[hVersion],selectedVersion
 jmp .bad
.selected:
 invoke lstrcpyW,statusText,sVCurrentSet
 invoke InvalidateRect,[hWnd],0,0
 mov eax,1
 ret
.bad:
 xor eax,eax
 ret
endp
proc VersionPageLaunch
 stdcall VersionPageUse
 test eax,eax
 jz .done
 stdcall StartWorker,EngineLaunch
.done:
 ret
endp
proc VersionPageBrowseMods
 stdcall VersionPageUse
 test eax,eax
 jz .done
 mov [modTab],0
 stdcall GoPage,4
.done:
 ret
endp
proc VersionPageRepair
 cmp [busy],0
 jne .done
 stdcall VsLoad,vmRootW,vmIdA
 test eax,eax
 jz .done
 stdcall StartWorker,VersionPageRepairWorker
.done:
 ret
endp
proc VersionPageRepairWorker uses ebx,param
 locals
  savedRoot rw 1024
  savedVersion rb 128
 endl
 mov [vmActionResult],0
 invoke lstrcpyW,addr savedRoot,gameDir
 invoke lstrcpyA,addr savedVersion,selectedVersion
 invoke lstrcpyW,gameDir,vmRootW
 invoke lstrcpyA,selectedVersion,vmIdA
 stdcall EInstallCore
 mov [vmActionResult],eax
 test eax,eax
 jz .cleanup
 stdcall SetStatusA,eStatusInstalled
.cleanup:
 stdcall ECleanup
.restore:
 invoke lstrcpyW,gameDir,addr savedRoot
 invoke lstrcpyA,selectedVersion,addr savedVersion
 invoke PostMessageW,[hWnd],WM_FINISHED,0,0
 ret
endp
proc OpenCurrentVersionPage
 cmp [busy],0
 jne .done
 stdcall PullSettings
 test eax,eax
 jz .done
 stdcall VersionPageOpen,gameDir,selectedVersion
.done:
 ret
endp
