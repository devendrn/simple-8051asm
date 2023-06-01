;lcm of two bytes
;nums at ext mem 4300h and 4301h
;lcm at ext mem 4302h
;hcf at ext mem 4303h

  org 4100h


  mov dptr,#4300h
  movx a, @dptr
  mov r0, a  ;byte 1
  inc dptr
  movx a, @dptr
  mov r1, a  ;byte 2

  acall lcm2
  acall hcf2

  inc dptr
  mov a,r2
  movx @dptr, a

  inc dptr
  mov a,r1
  movx @dptr, a

here: sjmp here


;subroutine for lcm
;in r0, in r1, out r2
lcm2:
  mov a, r0
  mov r2, a
  _lcm2_loop: 
    mov a, r2 ;r0 multiple
    mov b, r1
    div ab
    mov a, b
    jz _lcm2_exit ;check if r2 divisible by r1
    mov a, r2
    add a, r0
    mov r2, a ;next r0 multiple
    sjmp _lcm2_loop
  _lcm2_exit: ret

;subroutine for hcf
;in r0, in r1, out r1
hcf2:
    mov a,r0
    mov b,r1
    div ab
    mov a,b
    jz _hcf2_exit
    mov a,r1
    mov r0,a
    mov r1,b
    sjmp hcf2
  _hcf2_exit: ret
