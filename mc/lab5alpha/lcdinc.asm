
indic:	clr P1.4		; ���������� � ����� ������: RS = 0
	mov r4, #38h	; 8-������� ����� ������ � ������� ����� �����
	lcall ind_wr	; ������ ������� � ���
	mov r4, #0Ch	; ����������� ���� ��������� ������� ��� �������
	lcall ind_wr
	mov r4, #80h	; ����� ������� ������ 1-�� ������
	lcall ind_wr
			 
	mov dptr, #FFD0h
	setb P1.4 	; ���������� � ����� ������: RS = 1
	
; ������� 1-�� ������
wr_str1:	movx a, @dptr ; ������ ������ �� ������� ������
	mov r4, a
	lcall ind_wr  ; ������ ������ � ���
	inc dptr	  ; ��������� ��. ����� �����������
	mov a, dpl	; ��. ����� dptr
	cjne a, #E4h, wr_str1; �������� ��������� ������ �������� 1 ������
			
	clr P1.4
	mov r4, #C0h
	lcall ind_wr
	setb P1.4
			
; ������� 2-�� ������
wr_str2:	movx a, @dptr ; ������ ������ �� ������� ������
	mov r4, a	  ; ������ ������ � ���
	lcall ind_wr  ; ��������� ��. ����� �����������
	inc dptr 
	mov a, dpl 
	cjne a, #F7h, wr_str2; �������� ��������� ������ �������� 2 ������
	ret
			
ind_wr: 	mov F8h, r4 ; ������ � ���� �5 ������������ �������
	setb p1.7	; ��������� ������� �
	clr p1.6	; ������ R/W=0 (������)
	lcall delay
	clr p1.7	; ����� ������� �
	lcall delay
	setb p1.7	
	ret

delay:	mov r3, #7
m2:	djnz r3, m2
	ret
	
	org FFD0h

data:	db 	'                    '
data2:	db 	'                    '