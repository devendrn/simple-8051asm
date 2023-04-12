; 8051 program to sort n bytes in ascending order
		org 4100h
		mov dptr,#4300h
		movx a,@dptr
		mov r0,a ;get count
		dec r0	;last = count - 1
		inc dptr
		mov r1,dpl ; store first addr
sort:	mov r2,00 ;loop size r2<-r0
		mov dpl,r1 ;goto first index
		movx a,@dptr
next:	mov r3,a ;first element
		inc dptr
		movx a,@dptr;second element
		cjne a,03,nequal
		sjmp skip ;jump if equal
nequal:	jnc skip ;if c=0 then a>b
		dec dpl
		movx @dptr,a ;put a in prev loc
		inc dptr
		mov a,r3
		movx @dptr,a ;put r3 in current loc
skip:	djnz r2,next ;next element
		djnz r0,sort ;sort again
here:	sjmp here
