;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; processor.s - Contains functions for modifying processor state via MSR and coprocessor
; register operations.
;
;
; Created by Mukaram Khan and Xin Jin
; Modified by Thomas Sharp and Cameron Patterson
;
; Copyright (c) 2008 The University of Manchester
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     https://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



		GET		../inc/globals.s

		EXPORT  change_to_low_vectors
		EXPORT  disable_interrupts
		EXPORT  enable_interrupts
		EXPORT  wait_for_interrupt
		EXPORT  read_CPSR

		AREA 	proc, CODE, READONLY



; Sets processor to use low interrupt vectors.
change_to_low_vectors
		MRC		p15, 0, r0, c1, c0, 0
		BIC		r0, r0, #(0x1 << 13)
		MCR		p15, 0, r0, c1, c0, 0
		BX		LR



; Disables all interrupts via the Current Program Status Register.
disable_interrupts
		MRS		r0, CPSR
		ORR		r0, r0, #IRQ_OFF | FIQ_OFF
		MSR		CPSR_c, r0
		BX		lr



; Enables all interrupts via the Current Program Status Register.
enable_interrupts
		MRS		r0, CPSR
		BIC		r0, r0, #IRQ_OFF | FIQ_OFF
		MSR		CPSR_c, r0
		BX		lr



; Returns the CPSR in r0.
read_CPSR
		MRS		r0, CPSR
		BX		lr



; Puts processor into low-power wait-for-interrupt condition.
wait_for_interrupt
		PUSH	{lr}
		MCR 	p15, 0, r0, c7, c0, 4
		POP		{pc}



		END
