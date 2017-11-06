;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; copier.s - Contains routine for moving and executing received flood-fill image.
;
;
; Written by Cameron Patterson and Thomas Sharp
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



		GET		../inc/globals.s

		IMPORT	disable_interrupts

		EXPORT	execute_image
		
		PRESERVE8
		AREA 	copier, CODE, READONLY



; Copies flood-filled image to specified location and begins executing it.
; r0 = image source, r1 = image size (in words), r2 = target address, r3 = execute_address
execute_image
		MSR		CPSR_c, #SVC_MODE | IRQ_OFF | FIQ_OFF	; Switch into SVC and disable IRQs
multiple_copy											; Copy image using LDM/STM
		CMP		r1, #3
		BLS		single_copy
		LDM		r0!, {r8-r11}
		STM		r2!, {r8-r11}
		SUB		r1, #4
		B		multiple_copy
single_copy												; Copy remaining single words
		CMP		r1, #0
		BXEQ	r3										; When done, execute!!!
		LDR		r4, [r0], #4
		STR		r4, [r2], #4
		SUB		r1, #1
		B		single_copy



		END