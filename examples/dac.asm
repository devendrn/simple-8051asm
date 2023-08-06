; triangular waveform using dac
; p1 used for dac

mov r0, p1

; start ascent
mov a, #0x00
ascent: 
  mov p1, a
  acall delay
  inc a
  jnz ascent

; ascent complete, start descend
mov a, #0xff
descend:
  mov p1, a
  acall delay
  dec a
  jnz descend

; descend complete, start ascent
sjmp ascent

; delay using timer
delay:
  mov tmod, #0x01 ; timer mode 1: 16-bit
  mov th0, #0xfc
  mov tl0, #0x66 ; 0xfc66 for 1ms delay (approx)
  setb tr0
  wait: jnb tf0, wait
  clr tr0
  clr tf0
  ret
