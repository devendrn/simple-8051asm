; perfect squareroot of a byte
; logic: perfect squares are series sum of n odd numbers

  mov r0, #0x40 ;in addr
  mov r1, #0x50 ;out addr

  mov r2, #1 ;first odd number
  mov r3, #0 ;count
  mov a, @r0
loop:
  subb a, r2
  jc save ; ov flag on, not perfect square
  jz save ; ov flag off, perfect square
  inc r2
  inc r2 ;next odd number
  inc r3 ;inc count
  sjmp loop
save:
  mov a, r3
  mov @r1, a ; store ans

stop:
  sjmp stop

