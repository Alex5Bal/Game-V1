	.file "_wdt_handler.c"
	.text

	.balign 2

	.global	WDT
	.section __interrupt_vector_11,"ax",@progbits
	.word	WDT
	.text

	.extern redrawScreen
	.extern wdt_c_handler
WDT:
	PUSH	R15
	PUSH	R14
	PUSH	R13
	PUSH	R12
	PUSH	R11
	PUSH	R10
	PUSH	R9
	PUSH	R8
	PUSH	R7
	PUSH	R6
	PUSH	R5
	PUSH	R4
	CALL	#wdt_c_handler
	POP	R4
	POP	R5
	POP	R6
	POP	R7
	POP	R8
	POP	R9
	POP	R10
	POP	R11
	POP	R12
	POP	R13
	POP	R14
	POP	R15
	cmp	#0, &redrawScreen
	jz	ball_no_move
	and	#0xffef, 0(r1)	; clear CPU off in saved SR

ball_no_move:
	RETI
	.size	WDT, .-WDT
	.local	count
	.comm	count,1,1
	.ident	"GCC: (GNU) 4.9.1 20140707 (prerelease (msp430-14r1-364)) (GNUPro 14r1) (Based on: GCC 4.8 GDB 7.7 Binutils 2.24 Newlib 2.1)"
