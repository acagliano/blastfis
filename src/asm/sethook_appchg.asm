include "equates.inc"
.assume adl = 1
XDEF _InstallAppChangeHook
XDEF _RemoveAppChangeHook

_InstallAppChangeHook:
    ld hl, AppHookAppV
    call _Mov9ToOP1       ; mov9toOP1
    call _ChkFindSym       ; chkfindsym
    jr nc, varexists
    ld hl, AppChgHook_End-AppChgHook
    call _CreateAppVar       ; CreateAppVar
    ex de, hl
    inc hl
    inc hl
    ld de, AppChgHook
    ld bc, AppChgHook_End-AppChgHook
    ldir
    ld b, 0
    ld hl, AppHookAppV
    call _Mov9ToOP1       ; mov9toOP1
    call _Arc_Unarc      ; should move to archive
    call _ChkFindSym       ; chkfindsym
    varexists:
    ex de, hl
    inc hl
    inc hl
    ld a, b
    call _SetAppChangeHook
    ret

_RemoveAppChangeHook:
    call _ClrAppChangeHook       ; unset parser hook
    ld hl, AppHookAppV
    call _Mov9ToOP1       ; mov9toOP1
    call _ChkFindSym       ; chkfindsym
    ret c
    call _DelVarArc       ; delvararc
    ret

AppHookAppV:
db 15h, "AVEditH",0


AppChgHook:
    db 83h
    push hl
    push af
    call process
    pop af
    pop hl
    ret
process:
    cp 46h
    jp z,openingeditor
    ld c,a      ;need to preserve A on newer models
    ld a,b
    cp 46h
    ld a,c      ;need to preserve A on newer models
    ret nz
closingeditor:
    ld hl,56*256
    ld (penCol),hl
    ld hl,str0
    call _VPutS
    jp show
openingeditor:
    ld hl,56*256
    ld (penCol),hl
    ld hl,str1
    call _VPutS
show:
    ld hl,84BFh
    ld b,8
charputloop:
    ld a,(hl)
    inc hl
    or a
    jr z,wait
    call _VPutMap
    djnz charputloop
wait:
    di
    xor a
    out (1),a
waitkeypress:
    in a,(1)
    inc a
    jr z,waitkeypress
waitkeyrelease:
    in a,(1)
    inc a
    jr nz,waitkeyrelease
    ei
    ret
str0: db "Closing ",0
str1: db "Opening ",0

AttrFile:
db 15h, "AVData", 0
AppChgHook_End:
