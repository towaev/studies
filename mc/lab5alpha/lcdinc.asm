
indic:	clr P1.4		; Подготовка к вводу КОМАНД: RS = 0
	mov r4, #38h	; 8-битовый режим обмена с выводом обеих строк
	lcall ind_wr	; Запись команды в ЖКИ
	mov r4, #0Ch	; Активизация всех знакомест дисплея без курсора
	lcall ind_wr
	mov r4, #80h	; Адрес нулевой ячейки 1-ой строки
	lcall ind_wr
			 
	mov dptr, #FFD0h
	setb P1.4 	; Подготовка к вводу ДАННЫХ: RS = 1
	
; Выводим 1-ую строку
wr_str1:	movx a, @dptr ; Читаем символ из внешней памяти
	mov r4, a
	lcall ind_wr  ; Запись данных в ЖКИ
	inc dptr	  ; Формируем сл. адрес видеобуфера
	mov a, dpl	; Мл. часть dptr
	cjne a, #E4h, wr_str1; Проверка окончания вывода символов 1 строки
			
	clr P1.4
	mov r4, #C0h
	lcall ind_wr
	setb P1.4
			
; Выводим 2-ую строку
wr_str2:	movx a, @dptr ; Читаем символ из внешней памяти
	mov r4, a	  ; Запись данных в ЖКИ
	lcall ind_wr  ; Формируем сл. адрес видеобуфера
	inc dptr 
	mov a, dpl 
	cjne a, #F7h, wr_str2; Проверка окончания вывода символов 2 строки
	ret
			
ind_wr: 	mov F8h, r4 ; Грузим в порт Р5 передаваемую посылку
	setb p1.7	; Установка сигнала Е
	clr p1.6	; Сигнал R/W=0 (запись)
	lcall delay
	clr p1.7	; Сброс сигнала Е
	lcall delay
	setb p1.7	
	ret

delay:	mov r3, #7
m2:	djnz r3, m2
	ret
	
	org FFD0h

data:	db 	'                    '
data2:	db 	'                    '