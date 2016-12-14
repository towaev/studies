	drebm1:	equ 82h
	drebm2:	equ 83h	


	Al:	equ 41h
	Ahi:	equ 42h
	Bl:	equ 43h
	Bhi:	equ 44h
	Chi:	equ 48h
	Cl: 	equ 49h	
	Dhi: 	equ 52h
	Dl:	equ 53h
	Oper:	equ 45h
	Func:	equ 46h
	LastKey:	equ 47h
	OVF:	equ 50h
	cycle:	equ 51h
	razr_num:	equ 54h
	signs: 	equ 70h
	count:	equ 71h
	org 8100h

	mov Al,#0
	mov Ahi,#0	
	mov Chi,#0
	mov Cl,#0
	mov Bhi,#0
	mov Bl,#0	
	lcall indicateA	
	mov Func,#0
	mov LastKey,#20
	mov OVF, #0
	mov count, #0
	
loop:	lcall klav
	lcall dreb_delay                    ;вроде как защита от дребезга, будем тестить
	mov a, klav_num
	cjne a, LastKey, processK
	ljmp loop

processK:	mov LastKey, klav_num
	cjne a, #20, process
	ljmp loop
process:    lcall ProcessKey
	ljmp loop
ProcessKey:
	mov a, Func
	cjne a, #1, NFKeyPress
	lcall FuncKeyPress
	ret
NFKeyPress:
	mov a, klav_num
	cjne a, #11	, NoFunction
	mov Func, #1
	ret
NoFunction:
	jc Number
	cjne a, #12, Operation
	;---------------------------
	inc count
	call check
	;---------------------------
	call Calc
	ret	
Number:	
	push Bl
	push Bhi
	mov Bhi, #0
	mov Bl, #10
	lcall Mult
	
	mov a, OVF
	cjne a, #1, next
	pop Bhi
	pop Bl
	ret
next:	
	push Ahi
	push Al
	mov Ahi, Chi
	mov Al, Cl
	mov Bhi, #0
	mov Bl, klav_num
	dec Bl
	lcall Addd	
	mov a,OVF
	cjne a, #1, next2
	pop Al
	pop Ahi
	pop Bhi
	pop Bl
	ret
next2:	mov Ahi, Chi
	mov Al, Cl
	pop a
	pop a
	pop Bhi
	pop Bl
	lcall indicateA
	ret
Operation:	mov Oper, klav_num
	mov Bhi, Ahi
	mov Bl, Al
	mov Ahi, #0
	mov Al, #0
	mov dptr, #data
	mov a, dpl
	clr c
	add a, #20
	mov dpl, a
	mov a, dph
	addc a, #0
	mov dph, a
	mov a, klav_num
	cjne a, #13, oper1
	mov a, #42
	movx @dptr, a
	ret
oper1:	cjne a, #14, oper2
	mov a, #47
	movx @dptr, a
	ret
oper2:	cjne a, #15, oper3
	mov a, #43
	movx @dptr, a
	ret
oper3:	mov a, #45
	movx @dptr, a
	ret

FuncKeyPress:
	; ---------------------
	mov count, #0
	
	
	; -----------------------
	mov Func, #0
	mov a, klav_num
	cjne a, #14, FKP1
	mov a, Ahi
	xrl a, #128
	mov Ahi, a
	lcall indicateA
FKP1:	mov Func, #0
	ret

Calc:	mov a, Oper
	cjne a, #13, NotMul
	lcall mul_sign
	mov a, OVF
	cjne a, #0, CalcError
	mov Ahi, Chi
	mov Al, Cl
	lcall indicateA
	ret
NotMul:	cjne a, #15, NotAdd
	lcall add_sign
	mov a, OVF
	cjne a, #0, CalcError
	mov Ahi, Chi
	mov Al, Cl
	lcall indicateA
	ret
NotAdd:	cjne a, #16, NotSubb_calc
	lcall Subb_calc
	mov a, OVF
	cjne a, #0, CalcError
	mov Ahi, Chi
	mov Al, Cl
	lcall indicateA
	ret
NotSubb_calc:
	cjne a, #14, NotDiv
	lcall div_sign
	mov a, OVF
	cjne a, #0, CalcError
	mov Ahi, Chi
	mov Al, Cl
	lcall indicateA
NotDiv:	ret

CalcError:	mov dptr, #data
	mov a, #101
	movx @dptr, a
	inc dptr
	mov a, #114
	movx @dptr,a
	inc dptr
	mov a, #114
	movx @dptr, a
	inc dptr
	mov a, #111
	movx @dptr, a
	inc dptr
	mov a, #114
	movx @dptr, a
	inc dptr
	mov a, #33
	movx @dptr, a
	inc dptr
	mov a, #33
	movx @dptr, a
	inc dptr
	mov a, #33
	movx @dptr, a
	inc dptr
	lcall indic
	mov Ahi, #0
	mov Al, #0
	mov Bhi, #0
	mov Bl, #0	
	ret
Mult:	mov a, Al
	mov b, Bl
	mul ab
	mov Chi, b
	mov Cl, a
	mov a, Ahi
	mov b, Bl
	mul ab
	clr c
	add a, Chi
	mov Chi, a 
	jc Error
	mov a, b
	cjne a, #0, Error

	mov a, Al
	mov b, Bhi
	mul ab
	clr c
	add a, Chi
	mov Chi, a 
	jc Error
	mov a, b
	cjne a, #0, Error
	
	mov a, Ahi
	mov b, Bhi
	mul ab
	cjne a, #0, Error
	mov a, b
	cjne a, #0, Error
	ret	
Error:	mov OVF, #1
	ret

Addd:	clr c
	mov a, Al
	add a, Bl
	mov Cl, a
	mov a, Ahi
	addc a, Bhi
	mov Chi, a
	jnc AddRet	
	mov OVF, #1
AddRet:	ret

mul_sign:	mov signs, #0
	mov a, Ahi
	anl a, #128
	cjne a, #128, mul_sign1
	mov signs, #1
mul_sign1:mov a, Ahi
	anl a, #127
	mov Ahi, a
	mov a, Bhi
	anl a, #128
	cjne a, #128, mul_sign2
	mov a, signs
	orl a, #2
	mov signs, a
mul_sign2:	mov a, Bhi
	anl a, #127
	mov Bhi, a
	lcall Mult
	mov a, OVF
	cjne a, #1, mul_sign3
	ret
mul_sign3:	mov a, Chi
	anl a, #128
	cjne a, #128, mul_sign4
	mov OVF, #1
	ret
mul_sign4:	mov a, signs
	cjne a, #0, mul_sign5
	ret
mul_sign5:	cjne a, #3, mul_sign6
	ret
mul_sign6 mov a, Chi
	orl a, #128
	mov Chi, a
	ret	

div_sign:	mov signs, #0
	mov a, Ahi
	anl a, #128
	cjne a, #128, div_sign1
	mov signs, #1
div_sign1:mov a, Ahi
	anl a, #127
	mov Ahi, a
	mov a, Bhi
	anl a, #128
	cjne a, #128, div_sign2
	mov a, signs
	orl a, #2
	mov signs, a
div_sign2:	mov a, Bhi
	anl a, #127
	mov Bhi, a
	lcall divide
	mov a, OVF
	cjne a, #1, div_sign3
	ret
div_sign3:	mov a, Chi
	anl a, #128
	cjne a, #128, div_sign4
	mov OVF, #1
	ret
div_sign4:	mov a, signs
	cjne a, #0, div_sign5
	ret
div_sign5:	cjne a, #3, div_sign6
	ret
div_sign6 mov a, Chi
	orl a, #128
	mov Chi, a
	ret	

divide:	mov Chi,#0
	mov Cl,#0
	mov cycle, #0
	mov razr_num, #0
	mov a, Bhi
	cjne a, Ahi, divide_cor
	mov a, Bl
	cjne a, Al, divide_cor
	mov Chi,#0
	mov Cl, #1
	ret
divide_cor:
	jnc normalisB
	ret
	
;----------------
check:	mov a, count
	cjne a, #3, go_norm
	mov dptr, #data
	mov a, '!'
	movx @dptr, a
	lcall indic
	; initilizto ZERo cond
    mov Al,#0
	mov Ahi,#0	
	mov Chi,#0
	mov Cl,#0
	mov Bhi,#0
	mov Bl,#0	
	lcall indicateA	
	mov Func,#0
	mov LastKey,#20
	mov OVF, #0
	lcall loop
go_norm:
	ret
	
;----------------------
	
	
normalisB:	mov a, Bhi
	anl a, #128
	cjne a, #0, normalis
	inc cycle
	mov a, Bl
	clr c
	rlc a
	mov Bl, a
	mov a, Bhi
	rlc a
	mov Bhi, a
	ljmp normalisB

normalis:	mov a, Ahi
	anl a, #128
	cjne a, #0, Norm_d
	mov a, Al
	clr c
	rlc a
	mov Al, a
	mov a, Ahi
	rlc a
	mov Ahi, a

	mov a,cycle
	cjne a,#0,normalis_next
	inc razr_num
	ljmp normalis
normalis_next:
	dec cycle
	ljmp normalis

Norm_d:	clr c
	mov a, Bl
	subb a, Al
	mov Bl, a
	mov a, Bhi
	subb a, Ahi
	mov Bhi, a	
	push psw
	clr c
	mov a, Cl
	rlc a
	mov Cl, a
	mov a, Chi
	rlc a
	mov Chi, a
	pop psw
	jc next3
	mov a, Cl
	add a, #1
	mov Cl, a
	ljmp next3_1
next3:	clr c
	mov a, Al
	add a, Bl
	mov Bl,a
	mov a, Ahi
	addc a, Bhi
	mov Bhi,a
next3_1:	clr c
	mov a, Ahi
	rrc a
	mov Ahi, a
	mov a, Al
	rrc a
	mov Al, a
	mov a,razr_num
	cjne a,#0, next4
divide_ret: ret
next4:
	dec razr_num
	ljmp Norm_d

indicateA:	mov dptr, #data
	mov signs, Ahi	
	mov a, Ahi
	anl a, #128
	cjne a, #128, ia1
	mov a, #45
	movx @dptr, a
	inc dptr
	mov a, Ahi
	anl a, #127
	mov Ahi, a
ia1:	push Ahi
	push Al
	push Bhi	
	push Bl

	mov Bhi, Ahi
	mov Bl, Al

	push Bhi	
	push Bl
	
	mov Ahi, #27h
	mov Al, #10h
	lcall divide
	
	mov a, Cl
	add a, #30h
	movx @dptr, a
	inc dptr
	mov Bhi,#0
	mov Bl,Cl
	mov Ahi, #27h
	mov Al, #10h
	lcall Mult
	mov Ahi, Chi
	mov Al,Cl	
	pop Bl
	pop Bhi
	lcall Subb_calc
	mov Bhi, Chi
	mov Bl, Cl
	push Bhi
	push Bl
; Tisiachi
	mov Ahi, #03h
	mov Al, #0E8h
	lcall divide
	
	mov a, Cl
	add a, #30h
	movx @dptr, a
	inc dptr
	mov Bhi,#0
	mov Bl,Cl
	mov Ahi, #03h
	mov Al, #0E8h
	lcall Mult
	mov Ahi, Chi
	mov Al,Cl	
	pop Bl
	pop Bhi
	lcall Subb_calc
	mov Bhi, Chi
	mov Bl, Cl
	push Bhi
	push Bl
; Sotni
	mov Ahi, #00h
	mov Al, #64h
	lcall divide
	
	mov a, Cl
	add a, #30h
	movx @dptr, a
	inc dptr
	mov Bhi,#0
	mov Bl,Cl
	mov Ahi, #00h
	mov Al, #64h
	lcall Mult
	mov Ahi, Chi
	mov Al,Cl	
	pop Bl
	pop Bhi
	lcall Subb_calc
	mov Bhi, Chi
	mov Bl, Cl
	push Bhi
	push Bl
; Desiatki
	mov Ahi, #00h
	mov Al, #10
	lcall divide
	
	mov a, Cl
	add a, #30h
	movx @dptr, a
	inc dptr
	mov Bhi,#0
	mov Bl,Cl
	mov Ahi, #00h
	mov Al, #10
	lcall Mult
	mov Ahi, Chi
	mov Al,Cl	
	pop Bl
	pop Bhi
	lcall Subb_calc
	mov Bhi, Chi
	mov Bl, Cl
; Edinici
	mov a, Bl
	add a, #30h
	movx @dptr, a
	inc dptr
	mov a, #32
	movx @dptr, a
	inc dptr
	movx @dptr, a
	inc dptr
	movx @dptr, a

	pop Bl
	pop Bhi
	pop Al
	pop Ahi
	mov Ahi, signs
	lcall indic
	ret
Subb_calc:
	clr c
	mov a, Bl
	subb a, Al
	mov Cl, a
	mov a, Bhi
	subb a, Ahi
	mov Chi, a	
	ret


sub_abs:	clr c
	mov a, Ahi
	cjne a, Bhi, NE1
	mov a, Al
	cjne a, Bl, NE1
	mov Chi, #0
	mov Cl, #0	
	ret
NE1:	jc ALessB
AMoreB:	clr c
	mov a, Al	
	subb a, Bl
	mov Cl, a
	mov a, Ahi
	subb a, Bhi
	mov Chi, a
	setb c
	ret
ALessB:	clr c
	mov a, Bl	
	subb a, Al
	mov Cl, a
	mov a, Bhi
	subb a, Ahi
	mov Chi, a
	clr c
	ret

add_sign:	mov signs, #0
	mov a, Ahi
	anl a, #128
	cjne a, #128, add_sign1
	mov signs, #1
add_sign1:mov a, Ahi
	anl a, #127
	mov Ahi, a
	mov a, Bhi
	anl a, #128
	cjne a, #128, add_sign2
	mov a, signs
	orl a, #2
	mov signs, a
add_sign2:mov a, Bhi
	anl a, #127
	mov Bhi, a
	mov a, signs
	cjne a, #0, add_sign3
	lcall Addd
	mov a, OVF
	cjne a, #1, add_s21
	ret
add_s21:	mov a, Chi
	anl a, #128
	cjne a, #128, add_s22
	mov OVF, #1
	ret
add_s22:	mov Ahi, Chi
	mov Al, Cl
	ret
add_sign3:cjne a, #3, add_sign4
	lcall Addd
	mov a, OVF
	cjne a, #1, add_s31
	ret
add_s31:	mov a, Chi
	anl a, #128
	cjne a, #128, add_s32
	mov OVF, #1
	ret
add_s32:	mov a, Chi
	orl a, #128
	mov Chi, a
	ret
add_sign4:lcall sub_abs
	jc AMB
ALB:	mov a, signs
	anl a, #2
	cjne a, #0, SetSign
	ret
AMB:	mov a, signs
	anl a, #1
	cjne a, #0, SetSign
	ret
SetSign:	mov a, Chi
	orl a, #128
	mov Chi, a
	ret

dreb_delay:
	mov	drebm1, #FFh
dreb_m1:
	mov 	drebm2, #FFh
dreb_m2: 	djnz	drebm2, dreb_m2
	djnz	drebm1, dreb_m1
	ret

	include C:\SHELL51\ASMS\43501_3\Koz\lab5-2\klavinc.asm
	include C:\SHELL51\ASMS\43501_3\Koz\lab5-2\lcdinc.asm
