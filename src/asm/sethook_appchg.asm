.assume adl = 1
XDEF _SetAppChgHook
XDEF _RemoveAppChgHook

_SetAppChgHook:
    ld hl, AppHookAppV
    call 0020320h       ; mov9toOP1
    call 002050Ch       ; chkfindsym
    jr nc, varexists
    ld hl, AppChgHook_End-AppChgHook
    call 0021330h       ; CreateAppVar
    ex de, hl
    inc hl
    inc hl
    ld de, AppChgHook
    ld bc, AppChgHook_End-AppChgHook
    ldir
    ld b, 0
    ld hl, AppHookAppV
    call 0020320h       ; mov9toOP1
    call 0021448h       ; should move to archive
    call 002050Ch       ; chkfindsym
    varexists:
    ex de, hl
    inc hl
    inc hl
    ld a, b
    call 002149Ch       ; setparserhook
    ret

_RemoveAppChgHook:
    call 00214A0h       ; unset parser hook
    ld hl, AppHookAppV
    call 0020320h       ; mov9toOP1
    call 002050Ch       ; chkfindsym
    ret c
    call 0021434h       ; delvararc
    ret

AppHookAppV:
db 15h, "AVEditP",0


AppChgHook:
    db 83h             ; Required for all hooks
    or a, a
    ret z
    push bc
    ld b, a
    ld a, (0D007E0h)    ; cxCurApp
    cp 046h             ; cxPrgmEdit
    pop bc
    jr nz, notopeningeditor
    ld de, 0D0EA1Fh     ; SaveSScreen
    ld hl, 0D0065Bh     ; progToEdit
    call 00202FCh       ; Mov9b
notopeningeditor:
    push af
    ld a, b
    cp 046h
    jr z, closingeditor
    pop af
    ret
closingeditor:
    ld hl, 0D0EA1Fh
    push hl
    ld hl, AttrFile
    call 0020320h
    call 002050Ch
    pop hl
    jr nc, exists
    pop af
    ret
exists:
    ex de, hl
    ld c, (hl)
    inc hl
    ld b, (hl)
    inc hl
    add hl, bc
    push hl             ; address to end at on stack
    sbc hl, bc
; write routine to look for program name
; write routine to checksum/size

    pop af
    ret

AttrFile:
db 15h, "AVData", 0
AppChgHook_End:
