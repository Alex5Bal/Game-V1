		.data
counter:	.word 0		;counter = 0

jt:		.text
		.word option1 	;jt[1]
		.word option2 	;jt[2]
		.word option3 	;jt[3]
		.word option4 	;jt[4]
		.word option5 	;jt[5]
		.word option6 	;jt[6]
		.word option7 	;jt[7]
		.word option8 	;jt[8]
		.word option9 	;jt[9]
		.word option10 	;jt[10]
		.word option11 	;jt[11]
		.word option12 	;jt[12]
		.word option13 	;jt[13]
		.word option14 	;jt[14]
		.word option15 	;jt[15]

		.global starWarsSong
starWarsSong:	mov &counter, r12 ;r12 = counter
		add r12, r12	  ;r12 = counter*2
		mov jt(r12), r0	  ;jmp jt[counter]

	;; switch table options
	;; same order as in source code

option0:
		mov #950, r12		;r12 = 950
		call #buzzer_set_period ;buzzer_set_period(950)
		add #1, &counter	;counter++
		jmp break		;break

option1:

option6:

option11:	mov #630, r12		;r12 = 630
		call #buzzer_set_period ;buzzer_set_period(630)
		add #1, &counter	;counter++
		jmp break		;break

option2:

option7:

option12:

option14:
		mov #710, r12		;r12 = 710
		call #buzzer_set_period ;buzzer_set_period(710)
		add #1, &counter	;couter++
		jmp break		;break

option3:
option8:
option13:
		mov #750, r12		;r12 = 750
		call #buzzer_set_period ;buzzer_set_period(750)
		add #1, &counter	;counter++
		jmp break		;break

option4:

option9:

option15:
		mov #840, r12		;r12 = 840
		call #buzzer_set_period ;buzzer_set_period(840)
		cmp #15, &counter	;counter-15
		JNZ else		;if(counter != 0) goto else
		mov #0, &counter	;counter = 0
	else:				;else
		add #1, &counter	;counter++
		jmp break		;break

option5:

option10:
		mov #475, r12		;r12 = 475
		call #buzzer_set_period ;buzzer_set_period(475)
		add #1, &counter	;counter++
		jmp break		;break

break:
		pop r0
