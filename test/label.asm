a1: mov a,b
abc: mov a,b
verylonglabel: mov a,b
_test: mov a,b
test: mov a,b
ad_ba: mov a,b
ad: mov a,b
sjmp a1
sjmp ad_ba
sjmp _test
sjmp verylonglabel
here: sjmp here
jnc ad
djnz r0, test
