; A flat landscape silhouette. The same logical artwork scales to the hero,
; appearance preview and animated title sheet. Coordinates anchor at bottom right.
section '.world' data readable
macro sceneRect x,y,w,h,color {
    db x,y,w,h
    dd color
}
worldScene:
    ; Evening sun above the distant ridge.
    sceneRect -12,-18,5,5,0FFF1BB72h
    sceneRect -46,-2,46,2,0FF294235h
    sceneRect -39,-4,39,2,0FF294235h
    sceneRect -32,-6,32,2,0FF294235h
    sceneRect -28,-8,28,2,0FF294235h
    sceneRect -25,-11,15,3,0FF294235h
    sceneRect -22,-13,8,2,0FF294235h
    ; Near ridge and two square-canopied trees.
    sceneRect -33,-3,33,3,0FF38583Bh
    sceneRect -27,-5,27,2,0FF38583Bh
    sceneRect -18,-7,18,2,0FF38583Bh
    sceneRect -10,-9,10,2,0FF38583Bh
    sceneRect -5,-11,5,2,0FF38583Bh
    sceneRect -28,-7,1,4,0FF354936h
    sceneRect -31,-10,7,3,0FF59814Ah
    sceneRect -30,-11,5,1,0FF6F9657h
    sceneRect -15,-11,2,5,0FF344333h
    sceneRect -19,-15,10,4,0FF527545h
    sceneRect -18,-16,8,1,0FF6F9657h
    ; Grass ledge rising into the foreground.
    sceneRect -22,-2,22,2,0FF42633Bh
    sceneRect -22,-2,7,1,0FF7DA861h
    sceneRect -15,-3,15,2,0FF42633Bh
    sceneRect -15,-3,6,1,0FF7DA861h
    sceneRect -9,-5,9,3,0FF42633Bh
    sceneRect -9,-5,9,1,0FF7DA861h
worldSceneEnd:

section '.worldc' code readable executable
proc DrawWorldScene uses ebx esi edi,right,bottom,unit,opacity
    locals
        x dd ?
        y dd ?
        w dd ?
        h dd ?
    endl
    mov esi,worldScene
.next:
    movsx eax,byte [esi]
    imul eax,[unit]
    add eax,[right]
    mov [x],eax
    movsx eax,byte [esi+1]
    imul eax,[unit]
    add eax,[bottom]
    mov [y],eax
    movzx eax,byte [esi+2]
    imul eax,[unit]
    mov [w],eax
    movzx eax,byte [esi+3]
    imul eax,[unit]
    mov [h],eax
    stdcall WithAlpha,dword [esi+4],[opacity]
    stdcall FillRR,[x],[y],[w],[h],0,eax
    add esi,8
    cmp esi,worldSceneEnd
    jb .next
    ret
endp
