format PE console 6.0
entry start
stack 100000h,40000h
include 'win32ax.inc'
section '.data' data readable writeable
engineCancel dd 0
modCancel dd 0
step dd 1
prefix du 'tests\out\zip-writer-',0
suffix du '%08X.zip',0
badSuffix du '.bad',0
nameA db 'modrinth.index.json',0
fileNameA db 'overrides/',0E9h,085h,08Dh,0E7h,0BDh,0AEh,'/data.txt',0
duplicateA db 'MODRINTH.INDEX.JSON',0
badA db '../outside.txt',0
payload db '{"formatVersion":1,"game":"minecraft","name":"ZIP writer","dependencies":{"minecraft":"1.21.1"},"files":[]}'
payloadEnd:
sourceW du 'tests\fixtures\packs\payload.txt',0
pathW rw 1024
badW rw 1024
numberW rw 32
section '.text' code readable executable
start:
 invoke lstrcpyW,pathW,prefix
 invoke GetTickCount
 cinvoke wsprintfW,numberW,suffix,eax
 invoke lstrcatW,pathW,numberW
 stdcall ExZipBegin,pathW
 test eax,eax
 jz failed
 cmp [exZipOwned],1
 jne failed
 inc [step]
 stdcall ExZipAddMemory,nameA,payload,payloadEnd-payload
 test eax,eax
 jz failed
 stdcall ExZipAddFile,fileNameA,sourceW
 test eax,eax
 jz failed
 stdcall ExZipFinish
 test eax,eax
 jz failed
 cmp [ezHandle],-1
 jne failed
 inc [step]
 stdcall ExZipBegin,pathW
 test eax,eax
 jnz failed
 cmp [exZipOwned],0
 jne failed
 inc [step]
 invoke lstrcpyW,badW,pathW
 invoke lstrcatW,badW,badSuffix
 stdcall ExZipBegin,badW
 test eax,eax
 jz failed
 stdcall ExZipAddMemory,nameA,payload,payloadEnd-payload
 test eax,eax
 jz failed
 stdcall ExZipAddMemory,duplicateA,payload,payloadEnd-payload
 test eax,eax
 jnz failed
 stdcall ExZipFinish
 test eax,eax
 jnz failed
 cmp [ezHandle],-1
 jne failed
 invoke DeleteFileW,badW
 inc [step]
 stdcall ExZipBegin,badW
 test eax,eax
 jz failed
 stdcall ExZipAddMemory,badA,payload,payloadEnd-payload
 test eax,eax
 jnz failed
 stdcall ExZipAbort
 invoke DeleteFileW,badW
 inc [step]
 stdcall ExZipBegin,badW
 test eax,eax
 jz failed
 mov [engineCancel],1
 stdcall ExZipAddMemory,nameA,payload,payloadEnd-payload
 test eax,eax
 jnz failed
 stdcall ExZipAbort
 cmp [ezHandle],-1
 jne failed
 invoke DeleteFileW,badW
 invoke ExitProcess,0
failed:
 stdcall ExZipAbort
 invoke ExitProcess,[step]
include '..\src\pack_export_zip.inc'
include '..\src\imports.inc'
