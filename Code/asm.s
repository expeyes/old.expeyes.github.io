
.include "p24FV32KA302.inc"

;--------------- Time measurement on Digital input pins ------------
.global _wait_cmp_low
.global _wait_cmp_high
.global _wait_for_low
.global _wait_for_high

_wait_cmp_low:       ; wait till COMP2 output LOW
        btsc IFS1, #12          ; Skip if T5IF is LOW
        return                  ; Else timeout error return
        btst CM2CON, #8         ; Comparator output
        bra  nz, $-6
        return

_wait_cmp_high:       ; wait till COMP2 output HIGH
        btsc IFS1, #12          ; Skip if T5IF is LOW
        return                  ; Else timeout error return
        btst CM2CON, #8         ; Comparator output
        bra  z, $-6             ;
        return

_wait_for_low:       ; wait till PORTB & w1 is true, or timeout on T3
        btsc IFS1, #12          ; Skip if T5IF is LOW
        return                  ; Else timeout error return
        mov  PORTB, w0
        and  w0, w1, w0
        bra  nz, $-8             ;
        return

_wait_for_high:       ; wait till PORTB.w1 is false
        btsc IFS1, #12          ; Skip if T5IF is LOW
        return                  ; Else timeout error return
        mov  PORTB, w0
        and  w0, w1, w0
        bra  z, $-8             ;
        return

;---------------------- ADC & DAC related routines----------------------
.global _write_dac_fast
.global _read_adc
.global _usleep

_usleep:            ; value of w0 MUST be in 1 to 2000 range
    sl w0,#3,w0              ; 8 Tcy = 1 usec, Fcy = 8MHz
    sub w0, #8, w0           ; subtract overheads
    repeat w0
    nop                      ; repeat this for w0 times
    return

_write_dac_fast:            ; cmd = 0. w1 = 4MSB, w0 = 8LSB
    mov w1, I2C1TRN         ; cmd + 4 MSB
D1: btst I2C1STAT, #14      ; test TRSTAT
    bra nz, D1
    mov w0, I2C1TRN         ; 8 LSB
D2: btst I2C1STAT, #14      ; test TRSTAT
    bra nz, D2
    ;btg LATB, #11
    return


_read_adc:                  ; Called only after init_adc
        mov w0, AD1CHS
        bset AD1CON1, #1        ; Set SAMP
        mov #50, w0            ; Sample for 50 usecs
        call _usleep
        bclr AD1CON1, #1        ; Clear SAMP
        mov #10, w0
        call _usleep            ; conversion time
        mov  ADC1BUF0, w0       ; avoiding wait loop
        return

;------------------------- Capture functions & ISR ----------------
.global __T1Interrupt
.global _capture1
.global _capture1_hr
.global _capture2
.global _capture2_hr
.global _capture3
.global _capture4

.equ HR1BIT,    0       ; 12 bit single channel
.equ HR2BIT,    1       ; 12 bit two channels
.equ LR1BIT,    2       ;  8 bit single channel
.equ LR2BIT,    3       ;  8 bit two channels
.equ LR3BIT,    4       ;  8 bit 3 channels
.equ LR4BIT,    5       ;  8 bit 4 channels
.equ HRMODE,    7       ; HR1 or HR2 mode will set this also

;Our ISR should get called only from within the wait loop of capture functions.
;It does not preserve registers to meet the timing requirements.
__T1Interrupt:  ; For double channels, enter with ch1 sampling
        bclr AD1CON1, #1        ; start convert.
        mov  ADC1BUF0, w2       ; Collect last output, (last ch in list)
        bclr IFS0,#3            ; Clear T1 interrupt flag
        btst _cmode, #HR1BIT   ; 12 bit Single channel
        bra  nz, hrsing
        btst _cmode, #LR1BIT   ;  8 bit Single channel
        bra  nz, lrsing
        btst _cmode, #HR2BIT   ; 12 bit 2 channels
        bra  nz, hrdoub
        btst _cmode, #LR2BIT   ;  8 bit 2 channels
        bra  nz, lrdoub
        btst _cmode, #LR3BIT   ; 12 bit 3 channels
        bra  nz, lr3ch
       ; Otherwise it is LR4BIT, 8 bit 4 channels, just drop into it

lr4ch:  ;1 byte, 4 channels. w3=ch1, w4=ch2, w5=ch3, w6=ch4
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 4th chan data to buffer
        dec  w1,w1              ; decrement NS
        bra  z, finish          ; We are done, if NS is zero
        btst AD1CON1, #0        ; test DONE bit (for Ch1)
        bra  z, $-2             ; wait until DONE is 1
        mov  w4, AD1CHS         ; Start sampling  Ch2
        bset AD1CON1, #1        ;
        mov  ADC1BUF0, w2       ; Collect Data of Channel 1
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        nop                     ; More sampling time
        bclr AD1CON1, #1        ; Start Converting Ch2
        nop
        btst AD1CON1, #0        ; test DONE bit
        bra  z, $-2             ; wait until DONE is 1
        mov w5, AD1CHS          ; select  Channel 3 and
        bset AD1CON1, #1        ; Start Sampling  it
        mov  ADC1BUF0, w2       ; Collect output of Channel 2
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        nop                     ; More sampling time
        bclr AD1CON1, #1        ; Start Converting 1st channel
        nop
        btst AD1CON1, #0        ; test DONE bit
        bra  z, $-2             ; wait until DONE is 1
        mov w6, AD1CHS          ; select  Channel 3 and
        bset AD1CON1, #1        ; Start Sampling  it
        mov  ADC1BUF0, w2       ; Collect output of Channel 2
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        nop                     ; More sampling time
        bclr AD1CON1, #1        ; Start Converting 1st channel
        nop
        btst AD1CON1, #0        ; test DONE bit
        bra  z, $-2             ; wait until DONE is 1
        mov w3, AD1CHS          ; select  Channel 4 and
        bset AD1CON1, #1        ; Start Sampling  it
       ; btg LATB, #7            ; MARKER for CRO
        retfie                  ; Alternate return only

lr3ch:  ;1 byte, 3 channels. w3=ch1, w4=ch2, w5=ch3
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        bra  z, finish          ; We are done, if NS is zero
        btst AD1CON1, #0        ; test DONE bit (for Ch1)
        bra  z, $-2             ; wait until DONE is 1
        mov  w4, AD1CHS         ; Start sampling  Channel 2
        bset AD1CON1, #1        ;
        mov  ADC1BUF0, w2       ; Collect Data of Channel 1
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        nop                     ; More sampling time
        bclr AD1CON1, #1        ; Start Converting Ch2
        nop
        btst AD1CON1, #0        ; test DONE bit
        bra  z, $-2             ; wait until DONE is 1
        mov w5, AD1CHS          ; select  Channel 3 and
        bset AD1CON1, #1        ; Start Sampling  it
        mov  ADC1BUF0, w2       ; Collect output of Channel 2
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        nop                     ; More sampling time
        bclr AD1CON1, #1        ; Start Converting 1st channel
        nop
        btst AD1CON1, #0        ; test DONE bit
        bra  z, $-2             ; wait until DONE is 1
        mov w3, AD1CHS          ; select  Channel 1 and
        bset AD1CON1, #1        ; Start Sampling  it
       ; btg LATB, #7            ; MARKER for CRO
        retfie                  ; Alternate return only

lrdoub:   ; 1 byte, two channels
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b w2, [w0++]        ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        bra  z, finish          ; We are done, if NS is zero
wt1:    btst AD1CON1, #0        ; test DONE bit, (C1, for double)
        bra  z, wt1             ; wait until DONE is 1
        mov  w4, AD1CHS         ; Start sampling 2nd Channel
        bset AD1CON1, #1        ;
        mov  ADC1BUF0, w2       ; Collect output of Channel 1
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b  w2, [w0++]         ; store to buffer
        dec  w1,w1              ; decrement NS
        nop                     ; More sampling time
        bclr AD1CON1, #1        ; Start Converting 1st channel
        nop
wt2:    btst AD1CON1, #0        ; test DONE bit
        bra  z, wt2             ; wait until DONE is 1
        mov w3, AD1CHS          ; Start Sampling  Channel 1
        bset AD1CON1, #1        ;
        retfie

hrdoub:   ; 2 byte, two channels
        mov  w2, [w0++]         ; store 2nd chan data to buffer
        dec  w1,w1              ; decrement NS
        bra  z, finish          ; We are done, if NS is zero
wt3:    btst AD1CON1, #0        ; test DONE bit, (C1, for double)
        bra  z, wt3             ; wait until DONE is 1
        mov  w4, AD1CHS         ; Start sampling 2nd Channel
        bset AD1CON1, #1        ;
        mov  ADC1BUF0, w2       ; Collect output of Channel 1
        mov  w2, [w0++]         ; store to buffer
        dec  w1,w1              ; decrement NS
        nop
        nop                    ; More sampling time
        bclr AD1CON1, #1        ; Start Converting 1st channel
        nop
wt4:    btst AD1CON1, #0        ; test DONE bit
        bra  z, wt4             ; wait until DONE is 1
        mov w3, AD1CHS          ; Start Sampling  Channel 1
        bset AD1CON1, #1        ;
        retfie

lrsing:     ; low resolution (1 byte) single channel
        lsr  w2, #4, w2         ; Right Shift by 4 bits
        mov.b  w2, [w0++]       ; and store 1 byte to buffer, increment
        dec  w1,w1              ; decrement NS
        bra  z, finish          ; finish if NS is zero
lrsw:   btst AD1CON1, #0        ; test DONE bit
        bra  z, lrsw            ; wait until DONE is 1
        bset AD1CON1, #1        ; Set ADC SAMP
        ;btg LATB, #7            ; MARKER for CRO
        retfie

hrsing:     ; high resolution (2 byte) single channel
        bclr IFS0,#3            ; Clear T1 interrupt flag
        bclr AD1CON1, #1        ; Clear SAMP, start ADC
        mov  ADC1BUF0, w2       ; Collect output of previous run
        mov  w2, [w0++]         ; and store it to buffer, increment
        dec  w1,w1              ; decrement NS
        bra  z, finish          ; finish if NS is zero
hrsw:   btst AD1CON1, #0        ; test DONE bit
        bra  z, hrsw            ; wait until DONE is 1
        bset AD1CON1, #1        ; Set ADC SAMP
        ;btg LATB, #7            ; MARKER for CRO
        retfie
finish: bclr IEC0,#3            ; clear T1 int. enable flag
        bset AD1CON1, #1        ; Set ADC SAMP
        retfie
;------------------- ISR ends here --------------------------------

set_timer:  ; w2 contains period in microseconds, assumes 16MHz crystal
        clr  T1CON
        sl   w2,#3,w2        ; 8 x TG cycles, one cycle = 0.125 usec
        dec  w2,w2           ; Subtract 1
        mov  w2, PR1         ; set Timer1 PR
        sub  w2, #8, w2      ; subtract 1 usec, 8 cycles
        mov  w2, TMR1        ; Will interrupt after 8 cycles
        bset IEC0,#3         ; T1 int. enable flag
        return

fast_read_adc:     ; init_adc() assumed. Channel selected by caller
        bset AD1CON1, #1   ; Set SAMP
        repeat #30
        nop                ; 1.25 usec sampling time
        bclr AD1CON1, #1   ; Clear SAMP
        btst AD1CON1, #0   ; test DONE bit
        bra  z, $-2        ; wait until DONE is 1
        mov ADC1BUF0, w0
        bset AD1CON1, #1   ; Leave ADC in Sample mode
        return             ; result in w0
;======================================================================
wait_adctrig:   ; ADC channel in action mask or ch1 (w3)
        mov _actionmask, w0 ;  ADC channel for triggering
        cp0 w0
        bra nz, $+4         ; If actionmask is zero use the
        mov w3, w0          ; content of w3 (1st channel)
        mov w0, AD1CHS      ; Select the channel
        mov _triglevel, w3  ; store trigger level in w3
loop:   btsc IFS1, #12      ; Skip if T5IF is LOW
        return              ; Else timeout error return
        call fast_read_adc  ; read trigger channel
        mov w0, w1          ; save a copy in w1
        mov _tg, w0         ; wait TG/4 usecs
        lsr w0,#2,w0        ; and read voltage again
        call _usleep
        call fast_read_adc  ; now, w1 has 1st value, w0 has 2nd
        ; Test the trigger condition => w1 < triglevel < w0
        sub w0,w1,w2        ; second - first
        bra n, loop
        sub w0,w3,w2        ; second - triglevel
        bra n, loop
        sub w3,w1,w2        ; triglevel - first
        bra n, loop
        return
;=====================================================================
; Analog trigger on channel zero means self triggering,ong 1st captured channel
.equ AANATRIG,     0       ; Trigger on an analog channel
.equ AWAITHI,      1       ; wait for HIGH level
.equ AWAITLO,      2       ; wait for LOW level
.equ AWAITRISE,    3       ; wait for rising edge
.equ AWAITFALL,    4       ; wait for falling edge
.equ ASET,         5       ; Set Digital output
.equ ACLR,         6       ; Clear Digital output
.equ APULSEHT,     7       ; HIGH TRUE pulse on Digital output
.equ APULSELT,     8       ; LOW TRUE pulse on Digital output

actions:        ; Capture Modifiers
        push w0
        push w1
    ;Start testing the Trigger/Actions, SET/CLR type actions
        mov _actionmask, w0 ; For setting/clearing LATB
        btst  _action, #ASET      ; Set Dout BIT
        bra   nz, A1
        btst _action, #ACLR     ; Clear Dout BIT
        bra  nz, A2
        btst _action, #APULSEHT ; HIGH True Pulse
        bra  nz, A3
        btst _action, #APULSELT ; LOW True Pulse
        bra  nz, A4

    ; Now the Wait type actions. Setup Timer4/5

        mov _actionmask, w1 ; For wait functions, on PORTB
        clr TMR4
        mov #300, w0        ; 400 is the total timeout, PR5
        mov w0, TMR5        ; Reduce timeout period to 1/4th
        bclr IFS1, #12      ; Clear Timeout error, IFS1.T5
        bset T4CON, #15     ; Start Timer4/5
        btst _action, #AWAITHI  ; Wait for HIGH
        bra  nz, A5
        btst _action, #AWAITLO  ; Wait for LOW
        bra  nz, A6
        btst _action, #AWAITRISE  ; Wait for rising edge
        bra  nz, A7
        btst _action, #AWAITFALL  ; Wait for falling edge
        bra  nz, A8
        btst  _action, #AANATRIG  ; Analog trigger, wait type
        bra   nz, A0
        bra  afin

A0:     push w2
        push w3
        mov  #390, w0
        mov  w0, TMR5       ; (MAXWAIT - 390)*65536/8 usecs
        call wait_adctrig   ; Self trigger. w1 = NS, w2 = TG, w3 = 1st ch
        pop  w3
        pop  w2
        mov  w3, AD1CHS
        bra  afin
A1:     ior  LATB                ; Set the bit as per mask
        bra  afin
A2:     com  w0, w0              ; Clearing the action mask bit
        and  LATB
        bra  afin
A3:     mov _pulse_width, w1
        sl  w1,#3, w1
        sub w1, #3, w1
        ior  LATB               ; Go HIGH
        repeat w1               ; pulse_width*8 cycles
        nop
        xor  LATB                ; Go LOW
        bra  afin
A4:     mov _pulse_width, w1
        sl  w1,#3, w1
        sub w1, #3, w1
        xor  LATB                ; Go HIGH
        repeat w1               ; width is 12.5 usecs
        nop
        ior  LATB                ; Go LOW
        bra  afin

A5:     mov _actionmask, w0     ; Make pin digital
        push ANSB
        xor  ANSB
        call _wait_for_high
        pop  ANSB
        bra  afin

A6:     mov _actionmask, w0     ; Make pin digital
        push ANSB
        xor  ANSB
        call _wait_for_low
        pop  ANSB
        bra  afin

A7:     mov _actionmask, w0     ; Make pin digital
        push ANSB
        xor  ANSB
        call _wait_for_low
        call _wait_for_high
        pop  ANSB
        bra  afin

A8:     mov _actionmask, w0     ; Make pin digital
        push ANSB
        xor  ANSB
        call _wait_for_high
        call _wait_for_low
        pop  ANSB

afin:   bclr T4CON, #15     ;Disable Timer4
        pop  w1
        pop  w0
       ; btg  LATB, #7
        return

;-------------------------------------------------------------------
capture_all:   ; Common routine for all capture functions
        mov  w3, AD1CHS      ; Select ADC channel for ISR
        bset AD1CON1, #1     ; Set ADC SAMP
        mov  #_dbuf, w0      ; point to _dbuf
        btst _cmode, #HRMODE ; In 8 bit mode,
        bra  nz, doub        ; we need to point to the
        inc w0, w0           ; odd address, buf+1
doub:   inc  w1, w1          ; One extra word/byte filled
        call actions
        call set_timer       ; TG is in w2
        bset T1CON, #15      ; Start Timer1 and wait
busy1:  btst IEC0,#3         ; loop here until ISR disables the Interrupt
        bra  nz,busy1
        clr  T1CON           ; Disable Timer1
;Reset the Digital Output, only for SET/CLR type actions
        mov _actionmask, w0     ; For setting/clearing LATB
        btst  _action, #ASET    ; Is ASET action enabled ?
        bra   z, aclr
        com  w0, w0             ; Clear bit, ready for next
        and  LATB
aclr:   btst _action, #ACLR     ; Is ACLR action enabled ?
        bra  z, done
        ior  LATB               ; Set bit, ready for next
done:   return


_capture1_hr:
        clr _cmode
        bset _cmode, #HR1BIT   ; 12 bit 1 channel
        bset _cmode, #HRMODE   ; Indicate 12 bit mode, is a MUST
        call capture_all
        return

_capture2_hr: ;12 bit reads. w0=0, w1=NS, w2=TG, w3=A0, w4=A1
        clr _cmode
        bset _cmode, #HR2BIT   ;  12 bit Single channel
        bset _cmode, #HRMODE   ;  Indicate 12 bit mode, is a MUST
        call capture_all
        return

_capture1:
        clr _cmode
        bset _cmode, #LR1BIT   ;  8 bit 1 channel
        call capture_all
        return

_capture2:  ;8 bit read. w0=0, w1=NS, w2=TG, w3=A0, w4=A1
        clr _cmode
        bset _cmode, #LR2BIT   ; 8 bit 2 channels
        call capture_all
        return

_capture3:   ;8 bit reads. w0=0, w1=NS, w2=TG, w3=A0, w4=A1, w5=a2
        clr _cmode
        bset _cmode, #LR3BIT   ; 8 bit 3 channels
        call capture_all
        return

_capture4:   ;8 bit reads. w0=0, w1=NS, w2=TG, w3=A0, w4=A1, w5=a2, w6=a3
        clr _cmode
        bset _cmode, #LR4BIT   ; 8 bit 3 channels
        call capture_all
        return





