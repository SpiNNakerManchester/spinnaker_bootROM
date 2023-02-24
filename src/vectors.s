;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; vectors.s - Contains 'low' exception vectors i.e. those placed at memory address 0. High
; vectors are used at power-on (see start.s); these vectors are used once the core has
; completed its boot_processor() function in main.c
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
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



		GET		../inc/globals.s

		IMPORT	boot_fail
		IMPORT  reset

        PRESERVE8
        AREA vectors, CODE, READONLY

		ENTRY



; Low exception vectors
		LDR     PC, =reset					; Reset
		B		unexpected_exception_low	; Undefined instruction
		B		unexpected_exception_low	; SVC
		B		unexpected_exception_low	; Prefetch abort
		B		unexpected_exception_low	; Data abort
		B		unexpected_exception_low	; Reserved...
		LDR	    PC, [PC, #-VIC_OFFSET]		; IRQ
		B		unexpected_exception_low	; FIQ



; Called in the case of a non-reset or non-IRQ exception. Sends proc to permanent sleep.
unexpected_exception_low
		MOV		r0, #UNEXPECTED_EXCEPTION_LOW
		B		boot_fail



		END
