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

		EXPORT	execute_image 
		
		AREA 	copier, CODE, READONLY



; Copies flood-filled image to specified location and begins executing it.
; r0 = image source, r1 = image size (in words), r2 = target address, r3 = execute_address
execute_image
		MSR		CPSR_c, #SVC_MODE | IRQ_OFF | FIQ_OFF
multiple_copy
		CMP		r1, #3
		BLS		single_copy
		LDM		r0!, {r8-r11}
		STM		r2!, {r8-r11}
		SUB		r1, #4
		B		multiple_copy
single_copy
		CMP		r1, #0
		BXEQ	r3
		LDR		r4, [r0], #4
		STR		r4, [r2], #4
		SUB		r1, #1
		B		single_copy



		END
