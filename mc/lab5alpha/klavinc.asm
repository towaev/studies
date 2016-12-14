map_start:	equ 30h
	p4: 	equ E8h 
klav_num:  	equ 40h
		
	klav:	mov a, #EFh
		mov r0, #map_start
	cycle2: mov p4, a
		orl p4, #0Fh
		mov @r0, p4
		rl a
		inc r0
		cjne r0,#(map_start+4),cycle2
		mov r0, #map_start
	convert:	
		mov r2,#0
		mov a,@r0
	conv_cyc:	
		rrc  a	
		inc r2
		jnc end_conv
		cjne r2,#4,conv_cyc			
		inc r0
		cjne r0,#(map_start+4),convert
		mov klav_num,#20
		jmp end

	end_conv:
		mov a, r0
		subb a,#30h
		rl a
		rl a
		add a,r2		
		mov klav_num,a
	end:	ret
