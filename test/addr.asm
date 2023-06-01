test:
  mov a,b
  mov b,#5
  ret
abc:
  inc r0
  ret
acall test
label: acall abc
lcall abc
lcall test
tkm: cjne a,#0x11,label
cam: mov p1,#0xff
sjmp tkm
ljmp tkm
ajmp label
ajmp tkm
acall adklj
lcall adklj
sjmp ahfn
ljmp ahfn
ajmp ahfn
acall 0213h
;lcall 0213h
ajmp 0213h
;ljmp 0213h
;sjmp 0004h
ahfn: setb p1.1
adklj:
  setb tcon.3 
  setb psw.5 
 	sjmp ahfn
  ret
