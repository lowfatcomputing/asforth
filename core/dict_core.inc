; dict_core.inc
; this part of the dictionay has to fit into the nrww flash
; most of these words are assembly and can be inlinned when required.


; ( xt -- ) 
; System
; execute XT
forthword_ EXEC, 0, "exec"
    movw ZL, tosl
    poptos
    ijmp

; (  -- )
; doesn't do anything - no operation
forthword_ NOOP, INLINE_OPT, "noop"
    nop
    ret


;;;;;;;;;;;; Parameter Stack ;;;;;;;;;;;;;
; ( n -- n n ) 
; Stack
; duplicate TOS
forthword_ DUP, INLINE_OPT, "dup"
    pushtos
    ret

; ( n1 -- [ n1 n1 ] | 0) 
; Stack
; duplicate TOS if non-zero
forthword_ QDUP, 0, "?dup"
    sbiw tosl, 0
    breq PFA_QDUP1
    pushtos
PFA_QDUP1:
    ret

; ( n1 n2 -- n2 n1) 
; Stack
; swaps the two top level stack cells
forthword_ SWAP, 0, "swap"
    movw temp0, tosl
    poptos
    st -Y, temp1
    st -Y, temp0
    ret

; ( x1 x2 -- x1 x2 x1 ) 
; Stack
; Place a copy of x1 on top of the stack
forthword_ OVER, 0, "over"
    pushtos
    ldd tosl, Y+2
    ldd tosh, Y+3
    ret

; ( x1 x2 -- x1 x2 x1 x2) 
; Stack
; Place a copy of x1 and x2 on top of the stack
forthword_ TWOOVER, 0, "2over"
    rcall OVER
    rjmp OVER

; ( n -- ) 
; Stack
; drop TOS
forthword_ DROP, INLINE_OPT, "drop"
    drop_
    ret

; ( n n -- ) 
; Stack
; drop TOS twice
forthword_ DDROP, 0, "2drop"
    nip_
    drop_
    ret

; ( n1 n2 -- n2) 
; Stack
; drop NOS, one before TOS.
; same as SWAP DROP
forthword_ NIP, INLINE_OPT, "nip"
    nip_
    ret

; ( n1 n2 n3 -- n3) 
; Stack
; drop NOS twice, two cells before TOS.
forthword_ DNIP, INLINE_OPT, "2nip"
    dnip_
    ret

; ( n1 n2 -- n2 n1 n2) 
; Stack
; tuck TOS before NOS, same as SWAP OVER
forthword_ TUCK, 0, "tuck"
    ld temp0, y+
    ld temp1, y+
    st -y, tosh 
    st -y, tosl
    st -y, temp1 
    st -y, temp0
    ret

; ( n1 n2 n3 -- n3 n2 n1) 
; Stack
; exchange TOS and stack cell before NOS, same as SWAP ROT
forthword_ FLIP, 0, "flip"
    ldd temp0, y+2
    ldd temp1, y+3
    std y+3, tosh 
    std y+2, tosl
    movw tosl, temp0 
    ret

; ( n1 n2 n3 -- n2 n3 n1) 
; Stack
; rotate the three top level cells
forthword_ ROT, 0, "rot"
    movw temp0, tosl
    ld temp2, Y+
    ld temp3, Y+ 
    poptos
    st -Y, temp3
    st -Y, temp2
    st -Y, temp1
    st -Y, temp0
    ret

; ( n1 n2 n3 -- n3 n1 n2) 
; Stack
; reverse rotate the three top level cells
; equivalent to rotate twice but faster
forthword_ RROT, 0, "-rot"
    ld temp0, Y+
    ld temp1, Y+
    ld temp2, Y+
    ld temp3, Y+
    st -Y, tosh
    st -Y, tosl
    st -Y, temp3
    st -Y, temp2
    movw tosl, temp0
    ret

;;;;;;;;;;;; Return Stack ;;;;;;;;;;;;;
; ( -- n ) (R: n --)
; Stack
; move TOR to TOS
; call only
forthword_ R_FROM, DIS_CALL_OPT, "r>"
    pop ZH
    pop ZL
    pushtos
    pop tosh
    pop tosl
    ijmp

; ( -- ) (R: n -- )
; Stack
; Drop TOR
forthword_ R_DROP, INLINE_OPT, "r>drop"
    r_drop_
    ret

; ( -- retaddr )
; Stack
; pop return onto data stack and exit caller
; must be called, do not use jmp
; used if data in program mem and need an address to it
forthword_ POPRET, DIS_CALL_OPT, "popret"
    pushtos
    pop tosh
    pop tosl
    ret


; ( n -- n ) (R: -- n )
; Stack
; copy TOS to TOR
forthword_ DUP_R, INLINE_OPT, "r>r"
    dup_r_
    ret

; ( n -- ) (R: -- n)
; Stack
; move TOS to TOR
; call only
forthword_ TO_R, DIS_CALL_OPT, ">r"
    pop ZH
    pop ZL
    dup_r_
    poptos
    ijmp

; ( -- n) (R: n -- n )
; Stack
; put a copy of TOR on TOS
forthword_ R_FETCH, DIS_CALL_OPT, "r"
    in ZL, SPL
    in ZH, SPH
    pushtos
    ldd tosh, Z+3
    ldd tosl, Z+4
    ret

; ( -- n) (R: n -- n+1 )
; Stack
; fetch content of TOR and then increment TOR by 1
; call only
forthword_ R_FETCHPLUS, DIS_CALL_OPT, "r>r+"
    pop ZH
    pop ZL
    pushtos
    pop tosh
    pop tosl
    adiw tosl, 0x01
    push tosl
    push tosh
    sbiw tosl, 0x01
    ijmp


;;;;;;;;;;;;; Operators ;;;;;;;;;;;
; ( n1 n2 -- flag) 
; Compare
; flag is true if n1 is not equal to n2
forthword_ NOTEQUAL, 0, "<>"
    rjmp XOR

; ( n -- flag ) 
; Compare
; flag is true if TOS equal to 0 (zero)
forthword_ EQUALZERO, 0, "0="
    or tosh, tosl
    rjmp PFA_EQUALDONE


; ( n2 n1 -- n1 )
; compare top of stack and next on top of stack and sets mcu status register flags
forthword_ NTCP, 0, "ntcp"
    ld temp0, Y+
    ld temp1, Y+
    cp temp0, tosl
    cpc temp1, tosh
    ret

; ( n1 n2 -- flag ) 
; Compare
; flag is true if TOS and NOS are equal

forthword_ EQUAL, 0, "="
    rcall NTCP
PFA_EQUALDONE:
    brne PFA_ZERO1
    rjmp PFA_TRUE1

; ( n1 n2 -- flag) 
; Compare
; flag is true if n1 is less than n2
forthword_ LESS, 0, "<"
    rcall NTCP
PFA_LESSDONE:
    brge PFA_ZERO1
    rjmp PFA_TRUE1

; ( n1 n2 -- flag ) 
; Compare
; flag is true if n1 is greater than n2
forthword_ GREATER, 0, ">"
    rcall NTCP
PFA_GREATERDONE:
    brlt PFA_ZERO1
    breq PFA_ZERO1
    rjmp PFA_TRUE1

; ( u1 u2 -- flag ) 
; Compare
; flag is true if u1 > u2 (unsigned)
forthword_ UGREATER, 0, "u>"
    rcall NTCP
    brlo PFA_ZERO1
    breq PFA_ZERO1
    rjmp PFA_TRUE1

; ( u1 u2 -- flag) 
; Compare
; flag is true if u1 < u2 (unsigned)
forthword_ ULESS, 0, "u<"
    rcall NTCP
    brlo PFA_TRUE1
    rjmp PFA_ZERO1

; ( n1 -- flag) 
; Compare
; flag is true if TOS is less than zero
forthword_ LESSZERO, 0, "0<"
    sbiw tosl, 0
    rjmp PFA_LESSDONE

; ( n1 -- flag ) 
; Compare
; flag is true if n1 is greater than 0
forthword_ GREATERZERO, 0, "0>"
    sbiw tosl, 0
    rjmp PFA_GREATERDONE

; ( -- 0 ) 
; Arithmetics
; set TOS to 0
forthword_ ZEROSET, INLINE_OPT, "0:"
    zerotos_
    ret

; ( -- 0 ) 
; Arithmetics
; place a value 0 on TOS
forthword_ ZERO, 0, "0"
    pushtos
PFA_ZERO1:
    zerotos_
    ret

; ( -- 1 ) 
; Arithmetics
; place a value 1 on TOS
forthword_ ONE, 0, "1"
    pushtos
    ldi tosh, 0
    ldi tosl, 1
    ret

; ( -- 2 ) 
; Arithmetics
; place a value 2 on TOS
forthword_ TWO, 0, "2"
    pushtos
    ldi tosh, 0
    ldi tosl, 2
    ret

; ( -- 10 ) 
; Arithmetics
; place a value 10 on TOS
forthword_ TEN, 0, "10"
    pushtos
    ldi tosh, 0
    ldi tosl, 10
    ret

; ( -- -1 ) 
; Arithmetics
; leaves the value -1 (true) on TOS
forthword_ TRUE, 0, "true"
    pushtos
PFA_TRUE1:
    ser tosl
    ser tosh
    ret

; ( n -- ) 
; Compare
; test TOS if zero
forthword_ ZEROSENSE, INLINE_OPT, "0?"
    zerosense_
    ret

; ( n -- n) 
; Compare
; test TOS if zero, does not consume TOS
forthword_ QZEROSENSE, INLINE_OPT, "?0?"
    dupzerosense_
    ret


; ( n1|u1 n2|u2 -- n3|u3 ) 
; Arithmetics
; subtract n2 from n1
forthword_ MINUS, 0, "-"
    ld temp0, Y+
    ld temp1, Y+
    sub temp0, tosl
    sbc temp1, tosh
    movw tosl, temp0
    ret

; ( n1 n2 -- n3) 
; Arithmetics
; add n1 and n2
forthword_ PLUS, 0, "+"
    ld temp0, Y+
    ld temp1, Y+
    add tosl, temp0
    adc tosh, temp1
    ret


; ( n1 -- n2) 
; Arithmetics
; 1-complement of TOS
forthword_ NOT, INLINE_OPT, "not"
    not_
    ret

; ( ddddkkkk -- kkkkdddd )
; Arithmetics
; swap nibbles in lower half of TOS
forthword_ SWAPNIB, INLINE_OPT, "swnib"
    swapnib_
    ret

; ( n1 -- n2 ) 
; Arithmetics
; arithmetic shift right
forthword_ TWOSLASH, INLINE_OPT, "2/"
    twoslash_
    ret

; ( n1 -- n2 ) 
; Arithmetics
; arithmetic shift left, filling with zero
forthword_ TWOSTAR, INLINE_OPT, "2*"
    twostar_
    ret


; ( n1 n2 -- n3 ) 
; Logic
; bitwise and

forthword_ AND, 0, "and"
    ld temp0, Y+
    ld temp1, Y+
    and tosl, temp0
    and tosh, temp1
    ret

; ( n1 n2 -- n3 ) 
; Logic
; logical or
forthword_ OR, 0, "or"
    ld temp0, Y+
    ld temp1, Y+
    or tosl, temp0
    or tosh, temp1
    ret

; ( n1 n2 -- n3) 
; Logic
; exclusive or
forthword_ XOR, 0, "xor"
    ld temp0, Y+
    ld temp1, Y+
    eor tosl, temp0
    eor tosh, temp1
    ret


; ( n1|u1 -- n2|u2 ) 
; Arithmetics
; optimized increment

forthword_ ONEPLUS, INLINE_OPT, "1+"
    oneplus_
    ret

; (S: n1 -- n2 )
; Arithmetics
; optimized decrement

forthword_ ONEMINUS, INLINE_OPT, "1-"
    oneminus_
    ret

; ( n1|u1 -- n2|u2 ) 
; Arithmetics
; optimized two increment
forthword_ TWOPLUS, INLINE_OPT, "2+"
    twoplus_
    ret

; (S: n1 -- n2 )
; Arithmetics
; optimized decrement
forthword_ TWOMINUS, INLINE_OPT, "2-"
    twominus_
    ret

; ( n1 -- n2/2 ) 
; Arithmetics
; convert byte length to word align length
; round number up to even value then right shift

forthword_ WALIGN, 0, "w_"
    oneplus_
	andi tosl, 0xfe
    twoslash_
    ret


; ( n1 n2 -- n3) 
; Arithmetics
; logically shift n1 left n2 times
forthword_ LSHIFT, 0, "<<"
    movw ZL, tosl
    poptos
PFA_LSHIFT1:
    sbiw ZL, 1
    brmi PFA_LSHIFT2
    lsl tosl
    rol tosh
    rjmp PFA_LSHIFT1
PFA_LSHIFT2:
    ret

; ( n1 n2 -- n3 ) 
; Arithmetics
; shift n1 n2-times logically right
forthword_ RSHIFT, 0, ">>"
    movw ZL, tosl
    poptos
PFA_RSHIFT1:
    sbiw ZL, 1
    brmi PFA_RSHIFT2
    lsr tosh
    ror tosl
    rjmp PFA_RSHIFT1
PFA_RSHIFT2:
    ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; (  -- n) 
; Stack
; current return stack pointer address
; must be called, don't use jmp to optimize
forthword_ RP_FETCH, DIS_CALL_OPT, "rp@"
    pushtos
    in tosl, SPL
    in tosh, SPH
    ; don't include return address
    twoplus_
    ret

; ( addr  -- ) (R: -- x*y)
; Stack
; set return stack pointer
; must be called
forthword_ RP_STORE, DIS_CALL_OPT, "rp!"
    in temp2, SREG
    cli
    ; keep the return address
    pop ZH
    pop ZL
    out SPL, tosl
    out SPH, tosh
    out SREG, temp2
    poptos
    ijmp

; (  -- addr ) 
; Stack
; current data stack pointer
forthword_ SP_FETCH, INLINE_OPT, "sp@"
    pushtos
    movw tosl, YL
    ret

; ( addr -- i*x) 
; Stack
; set data stack pointer to addr
forthword_ SP_STORE, INLINE_OPT, "sp!"
    movw YL, tosl
    poptos
    ret

; ( n1 -- n2 ) 
; Arithmetics
; exchange the bytes of the TOS

forthword_ BYTESWAP, INLINE_OPT, "><"
    mov temp0, tosh
    mov tosh, tosl
    mov tosl, temp0
    ret

; ( n1 -- n2 ) 
; Arithmetics
; clear the high byte of the TOS

forthword_ ZEROHIGH, INLINE_OPT, "0~"
    zerohigh_
    ret

; ( n1 -- n2 ) 
; Arithmetics
; clear the low byte of the TOS
forthword_ ZEROLOW, INLINE_OPT, "~0"
    zerolow_
    ret

; ( n1 -- n2 ) 
; Arithmetics
; put high byte of TOS in low byte
forthword_ SREIGHT, INLINE_OPT, "~h"
    highbyte_
    ret


; ( -- addr ) 
; System Variable
; get user area pointer
forthword_ UP_FETCH, INLINE_OPT, "up@"
    pushtos
    movw tosl, upl
    ret

; ( addr -- ) 
; System Variable
; set user area pointer
forthword_ UP_STORE, INLINE_OPT, "up!"
    movw upl, tosl
    poptos
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ( n e-addr -- ) 
; Memory
; write n (2bytes) to eeprom address
forthword_ STOREE, 0, "!e"
    movw ZL, tosl
    poptos
    in_ temp2, SREG
    cli
    rcall PFA_STOREE1
    adiw ZL,1

    mov tosl, tosh
    rcall PFA_STOREE1
    out_ SREG, temp2
    poptos
    ret
    
PFA_STOREE1:
    sbic EECR, EEPE
    rjmp PFA_STOREE1

PFA_STOREE2: ; estore_wait_low_spm:
    in_ temp0, SPMCSR
    sbrc temp0,SPMEN
    rjmp PFA_STOREE2

    out_ EEARH,ZH
    out_ EEARL,ZL
    out_ EEDR, tosl
    sbi EECR,EEMPE
    sbi EECR,EEPE

    ret

; ( e-addr - n) 
; Memory
; read 1 cell from eeprom
forthword_ FETCHE, 0, "@e"
    in_ temp2, SREG
    cli
    movw ZL, tosl
    rcall FETCHE_READ
    in_ tosl, EEDR

    adiw ZL, 1

    rcall FETCHE_READ
    in_  tosh, EEDR
    out_ SREG, temp2
    adiw ZL, 1
    ret

FETCHE_READ:
    sbic EECR, EEPE
    rjmp FETCHE_READ

    out_ EEARH,ZH
    out_ EEARL,ZL

    sbi EECR,EERE
    ret


; ( n f-addr -- ) 
; Memory
; writes n to flash memory using assembly code (code to be placed in boot loader section)
forthword_ STOREI, 0, "!i"
  ; store status register
  in temp1,SREG
  push temp1
  cli

  movw temp2, tosl ; save the (word) address
  poptos          ; get the new value for the flash cell
  ; save parameter stack
  push YL
  push YH
  rcall DO_STOREI_atmega
  ; restore parameter stack
  pop YH
  pop YL
  ; finally clear the stack
  poptos
  pop temp1
  ; restore status register (and interrupt enable flag)
  out SREG,temp1

  ret

; 
DO_STOREI_atmega:
  ; write data to temp page buffer
  ; use the values in tosl/tosh at the
  ; appropiate place
  rcall pageload

  ; erase page if needed
  ; it is needed if a bit goes from 0 to 1
  com temp4
  com temp5
  and tosl, temp4
  and tosh, temp5
  or tosh, tosl
  breq DO_STOREI_writepage 

    movw ZL, temp2
    ldi temp0,(1<<PGERS)
    rcall dospm

DO_STOREI_writepage:
  ; write page
  movw ZL, temp2
  ldi temp0,(1<<PGWRT)
  rcall dospm

  ; reenable RWW section
  movw ZL, temp2
  ldi temp0,(1<<RWWSRE)
  rcall dospm
  ; incase A+ gets used
  movw ZL, temp2
  adiw ZL, 1
  ret

; load the desired page
.equ pagemask, ~ ( PAGESIZE - 1 )
pageload:
  movw ZL, temp2
  ; get the beginning of page
  andi ZL,lo8(pagemask)
  andi ZH,hi8(pagemask)
  movw YL, ZL
  ; loop counter (in words)
  ldi XL,lo8(PAGESIZE)
  ldi XH,hi8(PAGESIZE)
pageload_loop:
  ; we need the current flash value anyway
  movw ZL, YL
  lsl ZL
  rol ZH
  lpm temp6, Z+
  lpm temp7, Z+
  ; now check: if Z points to the same cell as temp2/3, we want the new data
  movw ZL, YL
  cp ZL, temp2
  cpc ZH, temp3
  breq pageload_newdata
    movw r0, temp6
    rjmp pageload_cont
pageload_newdata:
    movw temp4, temp6
    movw r0, tosl
pageload_cont:
  clr temp0
  rcall dospm
  adiw YL, 1
  sbiw XL, 1
  brne pageload_loop

pageload_done:
  ret


;; dospm
;;
;; EXEC spm instruction
;;   temp0 holds the value for SPMCR

dospm:
dospm_wait_ee:
  sbic EECR, EEPE
  rjmp dospm_wait_ee
dospm_wait_spm:
  in_  temp1, SPMCSR
  sbrc temp1, SPMEN
  rjmp dospm_wait_spm

  ; turn the word addres into a byte address
  lsl ZL
  rol ZH

  ; EXEC spm
  ori temp0, (1<<SPMEN)
  out_ SPMCSR,temp0
  spm
  ret

; ( f-addr -- n1 ) 
; Memory
; read 1 cell from flash
forthword_ FETCHI, 0, "@i"
    movw ZL, tosl

ZDFETCHI:
    lsl ZL
    rol ZH

ZFETCHI:
	lpm tosl, Z+
	lpm tosh, Z+

    ret

; ( a-addr -- n ) 
; Memory
; read 1 cell from RAM address
forthword_ FETCH, 0, "@"
    movw ZL, tosl

ZFETCH:
    ; low byte is read before the high byte
    ld tosl, Z+
    ld tosh, Z+
    ret

; ( n addr -- ) 
; Memory
; write n to RAM memory at addr, low byte first
forthword_ STORE, 0, "!"
    movw ZL, tosl
    poptos

ZSTORE:
    st Z+, tosl
    st Z+, tosh
    poptos
    ret

; ( addr -- )
; write 0 to RAM memory at addr.    
forthword_ ZEROSTORE, 0, "0!"
    movw ZL, tosl
    zerotos_
    rjmp ZSTORE

; ( n addr -- ) 
; addr is high address
; Memory
; write n to RAM memory at addr, high byte first
forthword_ HSTORE, 0, "h!"
    movw ZL, tosl
    poptos

    in_ temp2, SREG
    cli
    st Z, tosh
    st -Z, tosl
    out_ SREG, temp2
    poptos
    ret

; ( c a-addr -- ) 
; Memory
; store a single byte to RAM address
forthword_ CSTORE, 0, "c!"
    movw ZL, tosl
    poptos

ZCSTORE:
    st Z+, tosl
    poptos
    ret
    
; ( a-addr -- ) 
; Memory
; store a 0 to single byte RAM address
forthword_ ZEROCSTORE, 0, "0c!"
    movw ZL, tosl
    clr tosl
    rjmp ZCSTORE

; ( a-addr - c1 ) 
; Memory
; fetch a single byte from memory mapped locations
forthword_ CFETCH, 0, "c@"
    movw ZL, tosl

ZCFETCH:
    clr tosh
    ld tosl, Z+
    ret

; ( n a-addr -- ) 
; Memory
; add n to content of RAM address a-addr
forthword_ PLUSSTORE, 0, "+!"
    movw ZL, tosl
    poptos
    ld temp2, Z
    ldd temp3, Z+1
    add tosl, temp2
    adc tosh, temp3
    rjmp ZSTORE
    
; ( a-addr -- ) 
; Memory
; add 1 to content of word RAM address a-addr
forthword_ ONEPLUSSTORE, 0, "1+!"
    movw ZL, tosl
    ld tosl, Z
    ldd tosh, Z+1
    adiw tosl, 1
    rjmp ZSTORE

; ( n a-addr -- ) 
; Memory
; add n to content of byte RAM address a-addr
forthword_ PLUSCSTORE, 0, "+c!"
    movw ZL, tosl
    poptos
    ld temp2, Z
    add tosl, temp2
    rjmp ZCSTORE
    

; ( a-addr -- ) 
; Memory
; add 1 to content of byte RAM address a-addr
forthword_ ONEPLUSCSTORE, 0, "1+c!"
    movw ZL, tosl
    ld tosl, Z
    subi tosl, -1
    rjmp ZCSTORE


; ( index -- n ) 
; Memory
; read 1 cell from user RAM address + index
forthword_ FETCHU, 0, "@u"
    rcall UP_FETCH
    rcall PLUS
    rjmp FETCH

; ( n index -- ) 
; Memory
; write n to user RAM memory at addr, low byte first
forthword_ STOREU, 0, "!u"
    rcall UP_FETCH
    rcall PLUS
    rjmp STORE


; ( -- n2 ) 
; Extended VM
; Read memory pointed to by register A (Extended VM)
forthword_ AFETCH, 0, "a@"
    movw ZL, al
    pushtos
    rjmp ZFETCH

; ( -- n2 ) 
; Extended VM
; Read program memory pointed to by register A (Extended VM)
forthword_ AFETCHI, 0, "a@i"
    movw ZL, al
    pushtos
    rjmp ZFETCHI


; ( n1 -- n2 )
; Extended VM
; Read memory pointed to by register A plus offset (Extended VM)
;forthword_ NAFETCH, 0, "na@"
;    movw ZL, al
;    add ZL, tosl
;    adc ZH, tosh
;    rjmp ZFETCH


; ( -- n ) 
; Extended VM
; Read a byte from ram pointed to by register A (Extended VM)
forthword_ ACFETCH, 0, "ac@"
    movw ZL, al
    pushtos
    rjmp ZCFETCH


; ( n -- ) A: n
; Extended VM
; Move TOS to A register (Extended VM)
forthword_ TO_A, INLINE_OPT, ">a"
    copytoa_
    poptos
    ret

; ( n -- ) B: n
; Extended VM
; Move TOS to B register (Extended VM)
forthword_ TO_B, INLINE_OPT, ">b"
    copytob_
    poptos
    ret

; ( n -- n ) A: n
; Extended VM
; copy TOS to A register (Extended VM)
forthword_ _A, INLINE_OPT, ":a"
    copytoa_
    ret

; ( n -- n ) B: n
; Extended VM
; copy TOS to B register (Extended VM)
forthword_ _B, INLINE_OPT, ":b"
    copytob_
    ret

; ( n1 n2 -- n2 ) A: n1
; Extended VM
; Remove next on stack and write to A register (Extended VM)
forthword_ NIPTOA, INLINE_OPT, "nip>a"
    niptoa_
    ret

; ( n1 n2 -- n2 ) B: n1
; Extended VM
; Remove next on stack and write to B register (Extended VM)
forthword_ NIPTOB, INLINE_OPT, "nip>b"
    niptob_
    ret

; ( -- n ) 
; Extended VM
; read the A register (Extended VM)
forthword_ A_, INLINE_OPT, "a"
    pushtos
    movw tosl, al
    ret

; ( -- n ) 
; Extended VM
; read the B register (Extended VM)
forthword_ B_, INLINE_OPT, "b"
    pushtos
    movw tosl, bl
    ret

; ( n -- )
; Extended VM
; Write memory pointed to by register A (Extended VM)
forthword_ ASTORE, 0, "a!"
    movw ZL, al
    rjmp ZSTORE

; ( n -- )
; Extended VM
; Write char memory pointed to by register A (Extended VM)
forthword_ ACSTORE, 0, "ac!"
    movw ZL, al
    rjmp ZCSTORE


; ( n offs -- )
; Extended VM
; Write TOS to memory pointed to by register A plus offset (Extended VM)
forthword_ NASTORE, 0, "na!"
    movw ZL, al
    add ZL, tosl
    adc ZH, tosh
    poptos
    rjmp ZSTORE

; ( -- )
; Extended VM
; update register A with post increment from last memory operation (Extended VM)
forthword_ APLUS, INLINE_OPT, "a+"
    aplus_
    ret
   

; ( len -- len/2 rem)
; Arithmetics
; convert byte length to word length plus remainder
forthword_ BTOW, 0, "btow"
    dup_        ; ( -- len len)
    twoslash_   ; ( -- len len/2
    rcall TUCK  ; ( -- len/2 len len/2
    twostar_    ; ( -- len/2 len len'
    rcall MINUS ; ( -- len/2 rem
    ret
        
    
; (start dest count -- )
; Memory
; copy flash memory.  Assumes no overlap.
forthword_ ITOI, 0, "itoi"
    niptoa_         ; ( start count ) A: dest
ITOI_B:
    dupzerosense_   ; ( start count )
    breq ITOI_END
    rcall TO_B      ; ( start ) B: count
    dup_            ; ( start start )
    rcall FETCHI    ; ( start val )
    rcall A_        ; ( start val dest )
    rcall STOREI    ; ( start )
    aplus_
    oneplus_        ; ( start+1 )
    rcall B_        ; ( start+1 count )
    oneminus_       ; ( start+1 count-1 )
    rjmp ITOI_B

ITOI_END:
    rjmp DDROP

.include "dict_interpret.inc"
.include "dict_compiler.inc"
.include "dict_defer.inc"


; must be last word in core    
forthword_ WIPE, 0, "WIPE"
    ; reset forth dictionary, here, edp, dp to original burn state
    ; set EE_DP to DPSTART
    doliteral_ pm(DPSTART)
    doliteral_ EE_DP
    rcall STOREE
    ; set EE_HERE to HERESTART
    doliteral_ HERESTART
    doliteral_ EE_HERE
    rcall STOREE
    ; set EE_EDP to EDPSTART
    doliteral_ EDPSTART
    doliteral_ EE_EDP
    rcall STOREE
    ; set EE_TURNKEY to NOOP
    doliteral_ pm(NOOP)
    doliteral_ EE_TURNKEY
    rcall STOREE
    ; set EE_FORTHWORDLIST to 0
    rcall ZERO
    doliteral_ EE_FORTHWORDLIST
    rcall STOREE
    jmp COLD
