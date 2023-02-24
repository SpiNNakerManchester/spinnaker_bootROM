;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; copier.s - Contains routine for moving and executing received flood-fill image.
;
;
; Written by Cameron Patterson and Thomas Sharp
;
; Copyright (c) 2008 The University of Manchester
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
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
