mov a,b

org 11
test: mov a,b
acall ham

; to-do - sort org pages
; to-do - check overlap between pages after sort

org 54
mov a,b
mov b, r0
sjmp test
lcall ham

org 0x0300
ham: mov a,r1
nop

