;****h* Boot_ROM/copier.s
;
;	NAME
;		copier.s
;
;	DESCRIPTION
;		Contains copier routine for image copying (in 'protected' memory area).
;
;	AUTHOR
;		Written by Cameron Patterson and Thomas Sharp
;
;	COPYRIGHT
;		SpiNNaker Project, The University of Manchester
;		Copyright (C) SpiNNaker Project, 2008-10. All rights reserved.
;
;******

		GET		../inc/globals.s

		EXPORT	execute_image_protected 
		
		AREA 	copier, CODE, READONLY


;****f*	copier.s/execute_image_protected
;
;	NAME
;		execute_image_protected
;
;	DESCRIPTION
;		Takes R0 as source address of image, R1 as the size in words and copies it into fixed memory location 0
;                R2 is used to signify the target address (usually 0x0 in ITCM), and R3 where to execute from.
;
;******

; Called at the end of flood fill. Parameters r0 = image source, r1 = image size (words), r2 = target address, r3 = execute_address
execute_image_protected
;		PUSH 	{r4-r11}
		MSR		CPSR_c, #SVC_MODE | IRQ_OFF | FIQ_OFF
;		MOV		r2, #0x0000  ;// This was used before we began supplying target address (was always zero)
;		MOV		r5, #0x70000000		;; CP added for testing
multiple_copy
		CMP		r1, #3
		BLS		single_copy
		LDM		r0!, {r8-r11}
		STM		r2!, {r8-r11}
;		STM		r5!, {r8-r11}		;; CP added for testing
		SUB		r1, #4
		B		multiple_copy
single_copy
		CMP		r1, #0
;		POPEQ	{r4-r11}
;		BXEQ	LR
;//		MOVEQ	r0, #0x0000 // This was used before we began supplying execute address (was always zero)
		BXEQ	r3
;		BEQ		.					;; CP for testing
		LDR		r4, [r0], #4
		STR		r4, [r2], #4
		SUB		r1, #1
		B		single_copy

		END

