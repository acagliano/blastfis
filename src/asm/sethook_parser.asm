include "equates.inc"
.assume adl = 1
XDEF _InstallParserHook
XDEF _RemoveParserHook

_InstallParserHook:
    ld hl, ParsHookAppV
    call _Mov9ToOP1
    call _ChkFindSym
    jr nc, varexists
    ld hl, ParserHook_End-ParserHook
    call _CreateAppVar
    ex de, hl
    inc hl
    inc hl
    ld de, ParserHook
    ld bc, ParserHook_End-ParserHook
    ldir
    ld b, 0
    ld hl, ParsHookAppV
    call _Mov9ToOP1
    call _Arc_Unarc
    call _ChkFindSym
varexists:
    ex de, hl
    inc hl
    inc hl
    ld a, b
    call _SetParserHook
    ret

_RemoveParserHook:
    call _ClrParserHook
    ld hl, ParsHookAppV
    call _Mov9ToOP1
    call _ChkFindSym
    ret c
    call _DelVarArc
    ret


ParsHookAppV:
db 15h, "AVParsH",0


ParserHook:
    db 83h             ; Required for all hooks
    or a                ; Which condition?
    jr nz,ReturnZ
    ld hl, AppVName
    call _Mov9ToOP1           ; mov9toop1
    call _ChkFindSym        ; check for virus defs
    jr c, ReturnZ           ; parse var if doesnt exist
    ex de, hl
    xor a
    cp b
    jr z, unarchived
    call _Arc_Unarc
    call _ChkFindSym
    ex de, hl
unarchived:
    ld c, (hl)
    inc hl
    ld b, (hl)    ; size into bc
    inc hl
    push hl
    add hl, bc
    push hl
    pop bc      ; save address vdefs should stop at
    pop hl
    push bc
    push bc
    ld bc, 4
    add hl, bc               ; jump past timestamp
loopvdef:
    ld de, curPC                       ; set de to current place in var (curPC)
    ld a, (hl)
    ld c, a
    ld b, 0      ; get size of opcode string
    push bc                             ; keep opcode size on stack
    add hl, bc
    inc hl                ; jump size of opcode string + 1 (for size)
    ld a, (hl)
    ld b, a
    inc hl      ; get size of opcode data, set to b, move to start of opcode
loopscanprog:
    ld a, (de)                          ; load byte at current parse spot into a
    push de
    push hl
    cp (hl)
    jr z, match               ; compare (de) to (hl), if match, parse forward
    pop bc
    pop hl
    pop de            ; restore b (size), start of opcode (de), and parse addr (hl)
    dec hl
    inc b
match:
    dec b
    ld a, b
    or a
    jr z, ReturnNZ
    push bc
    ld bc, endPC             ; endPC
    ex de, hl
    sbc hl, bc
    jr z, nextdef
    add hl, bc
    ex de, hl
    pop bc
    inc hl
    inc de
    jr loopscanprog
nextdef:
    inc hl
    ld a, (hl)         ; size of description
    ld c, a
    ld b, 0
    add hl, bc                  ; jump past description
    pop bc
    sbc hl, bc
    jr z, ReturnZ     ; definitions parsed, ok to run program
    add hl, bc
    push bc
    inc hl
    jr loopvdef
ReturnZ:
    ld hl, AppVName
    call 0020320h           ; mov9toop1
    call 0021448h
    cp a
    ret
ReturnNZ:
    ld hl, AppVName
    call _Mov9ToOP1           ; mov9toop1
    call _Arc_Unarc
    or 1
    ret
AppVName:
db 15h, "AVDEFS",0
ParserHook_End:
