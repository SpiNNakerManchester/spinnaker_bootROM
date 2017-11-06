;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Created by Mukaram Khan and Xin Jin
; Modified by Thomas Sharp and Cameron Patterson
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		GET ../inc/globals.s
		
		IMPORT  ||Image$$STACKS$$ZI$$Limit||
		IMPORT  ||Region$$Table$$Limit||
		IMPORT 	__main
;		IMPORT	disable_clock
		IMPORT	main
		IMPORT	ram_test
;		IMPORT  wait_for_interrupt
;		IMPORT  SYS_CTRL_MUX
;		IMPORT	disable_interrupts
				
		EXPORT	boot_fail
		EXPORT	dputc
		EXPORT  image_end
		EXPORT  reset
		EXPORT  stack_base
		
		PRESERVE8
		AREA start, CODE, READONLY
		ENTRY



; High exception vectors
		LDR     PC, =reset					; Reset
		B		unexpected_exception_high	; Undefined instruction
		B		unexpected_exception_high	; SVC
		B		unexpected_exception_high	; Prefetch abort
		B		unexpected_exception_high	; Data abort
		B		unexpected_exception_high	; Reserved...
		B		unexpected_exception_high	; IRQ
		B		unexpected_exception_high	; FIQ


reset

; CP for !!!TLM testing only - turns off/sleepy extra processors to allow the simulation to run faster!
		LDR		r2, =DMA_CTRL_BASE				;//!!! TLM !!!   CP
		LDR		r2, [r2, #DMA_CTRL_STAT]		;//!!! TLM !!!
		LSR		r2, r2, #24						;//!!! TLM !!!  ; 24 bits rotate to get the processorID in lower 8 bits
		LDR		r1, =0x2						;//!!! TLM !!!   processor IDs of 3 or greater
		CMP     r2, r1							;//!!! TLM !!!   will be turned off to
		MOVGT   r0, #0							;//!!! TLM !!!   save simulation time (and avoid scatters)
		BGT		boot_fail						;//!!! TLM !!!   not a real error! (so code R0=0)
			

; CP - testing app load - comment out below line for ROM !!!S2!!!
;		B		stackinit	; !!!APP!!! testing CP initialise stacks and scatter self  (note not for ROM - just Ph1 appl testing)

; CP. If watchdog boot, compare last word in ITCM with magic number. If eq, branch to ivb_checker (to see if can recover ITCM execution)
		LDR		r0, =SYS_CTRL_BASE
		LDR		r1, [r0, #SYS_CTRL_RESET_CODE]
		CMP		r1, #WATCHDOG_RESET_CODE
		BNE		normal_boot
		LDR		r0, =ITCM_VALIDATION_BLOCK
		LDR		r1, [r0, #IVB_MAGIC_NUMBER_PLACE]
		LDR		r0, =IVB_MAGIC_NUMBER
		CMP		r0, r1
		BEQ		ivb_checker


normal_boot
; CP. If not an IVB boot, then check for existence of Serial ROM  (uncomment for SpiNNaker2 BOOTROM Only) !!!S2!!!
		GET ../src/serial.s

boot_introm
		BL		set_chip_clock				;TODO - should this actually be 1 processor only? 

; Enable TCMs by writing to co-processor registers (not definitively required, as on by default)
        MRC     p15, 0, r0, c1, c0, 0
        ORR     r0, r0, #(0x1 << 12)
        ORR     r0, r0, #(0x1 << 2)
        MCR     p15, 0, r0, c1, c0, 0


		MOV		r0, #0
		BL		write_failure_log			; Initialise this core's failure log to 0 (no fault found... yet!)

; Test TCMs using SBF`s/ST`s ram_test code, disable interrupts and go to sleep in case of failure
		MOV		r0, #ITCM_BASE   			
		LDR		r1, =(ITCM_BASE + 16)  		; !!! TLM !!! quicker boot was "MOV		r1, #ITCM_BASE + ITCM_SIZE"
		LDR		r2, =0xFFFFFFFF
		BL		ram_test					; Comment these 4 lines if building for 0 (ITCM) rather than SYSRAM/BOOTROM
		CMP		r0, #0
		MOVNE	r0, #1 << ITCM_FAILURE
		BNE		boot_fail
		
		MOV		r0, #DTCM_BASE
		LDR		r1, =(DTCM_BASE + 16)			; !!! TLM !!! quicker boot was "MOV		r1, #DTCM_BASE + DTCM_SIZE"
		LDR		r2, =0xFFFFFFFF
		BL		ram_test
		CMP		r0, #0
		MOVNE	r0, #1 << DTCM_FAILURE
		BNE		boot_fail

stackinit
; Initialise stacks for each used mode, switching interrupts off in the process
		LDR     r0, stack_base

		MSR     CPSR_c, #SVC_MODE | IRQ_OFF | FIQ_OFF
		MOV     sp, r0
		SUB		r0, r0, #SVC_STACK_SIZE
		
		MSR     CPSR_c, #IRQ_MODE | IRQ_OFF | FIQ_OFF
		MOV     sp, r0
		SUB		r0, r0, #IRQ_STACK_SIZE
		
        MSR     CPSR_c, #SVC_MODE | IRQ_OFF | FIQ_OFF



; Point to sys ctl - bootup LED lighting for diagnostics
;       mov     r0, #SYS_CTRL_BASE
; Set all pins as outputs
;      mov     r1, #0
;      str     r1, [r0, #SYS_CTRL_IO_DIRECTION]
; Turn off all LEDs and then On LEDs 2 & 3 (bits 6 & 7)
;       mov	   r1, #0xC3
;      str     r1, [r0, #SYS_CTRL_CLEAR_IO]
;      mov     r1, #0xC0
;     str     r1, [r0, #SYS_CTRL_SET_IO]


	
		
; Branch to ARM routines that initialise state, scatter, and branch to main()
		B		__main



; Push character up the serial attached tube - possibly used by printf (see debugprintf.c)
dputc	PUSH	{lr}
		SVC		0
		POP		{pc}



; Record failure of TCM, disable interrupts and go to sleep (takes error code in r0)
boot_fail
		BL		write_failure_log		; Record peripheral failure
		MRS		r0, CPSR				; recover current status register
		ORR		r0, r0, #0xC0			; create mask for turning off FIQ and IRQ
		MSR		CPSR_c, r0				; turn off the interrupts for this core
		MCR 	p15, 0, r0, c7, c0, 4	; & goto sleep  - NB to recover will need to force the PC somewhere useful



; Write a value in r0 to failure log (either for initialisation to 0, or to record error code)
write_failure_log
		LDR		r1, =FAILURE_LOG_BASE
		LDR		r2, =DMA_CTRL_BASE
		LDR		r2, [r2, #DMA_CTRL_STAT]
		LSR		r2, r2, #24				; 24 bits rotate to get the processorID in lower 8 bits
		LSL		r2, r2, #2				; Multiply by 4 to get memory address byte offset for failure log (each log is 1 word)
		STR		r0, [r1, r2]			; Store failure report in this CPUs failure log
		BX		lr



; Setup PLLs, loop 200 times to wait for stabilised output then set clock muxes
set_chip_clock
		LDR		r0, =SYS_CTRL_BASE
    	LDR		r1, =PLL_1_SETUP
		STR		r1, [r0, #SYS_CTRL_PLL1]
		LDR		r1, =PLL_2_SETUP
		STR		r1, [r0, #SYS_CTRL_PLL2]
		MOV     r1, #200
dloop   SUBS    r1, r1, #1
        BNE     dloop
;		LDR		r1, =SYS_CTRL_MUX
		LDR		r1, =CLK_MUX_CTRL_SETUP
		STR		r1, [r0, #SYS_CTRL_CLK_MUX_CTRL]
		BX		lr


unexpected_exception_high

		MOV		r0, #1 << UNEXPECTED_EXCEPTION_HIGH
		B		boot_fail



ivb_checker

; we have got here because the magic number is correct in ITCM, so we have a chance to recover to code here

;; do DMA write of 35 ctrl+blockcrcs in ITCM to SysRAM with CRC ON
		LDR		r0, =DMA_CTRL_BASE			; load DMA controller base into R0

    	LDR		r1, =FF_SPACE_BASE			; load FF_SPACE_BASE position (sysram block used for flood filling) into R1
		STR		r1, [r0, #DMA_CTRL_ADRS]	; store R1 into the DMA System side address reg	
	   	LDR		r1, =ITCM_VALIDATION_BLOCK	; load ITCM_VALIDATION_BLOCK position into R1 (block of all the IVB data)
		STR		r1, [r0, #DMA_CTRL_ADRT]	; store R1 into the DMA TCM side address reg		
		LDR		r4, [r1, #IVB_GRAND_CRC]	; load IVB CRC for later checking
    	LDR		r1, =DMA_CMD_MASTER_CRC_CHK	; DMA control word:  direction=1 (from tcm to sys), CRC=1 (write), len= (35*4)= 140 bytes
		STR		r1, [r0, #DMA_CTRL_DESC]	; store contents of R1 into the control register to initiate DMA transfer

dmawait LDR		r1, [r0, #DMA_CTRL_STAT]	; read DMA status register
		TST		r1, #DMA_TRANSFER_ACTIVE	; test for transfer still being in progress (LSbit)
		BNE		dmawait						; if still transferring spin	
		TST		r1, #DMA_TRANSFER_PAUSED	; Any type of error will show as a Paused Transfer (2nd LSBit)
		BNE		normal_boot					; branch to the normal boot label

		LDR		r1, [r0, #DMA_CTRL_CRCC]	; load calculated CRC
		CMP		R1, R4						; compare calculated CRC with the CRC from the IVB
		BNE		normal_boot					; if they are not the same then this block does not have integrity. Discard and boot normally
		
;; if no errors then continue to check individual CRCs against blocks
; R0 and R1 are used for general purposes.  R2 = block number (1-32 being worked on). R3= size of that block in bytes
; R4 is the read in CRC value from the IVB, R5 is used as a pointer into the IVB block at relevant position
; R6 is used as the remaining ITCM image size to be checked (bytes), R7 is the number of 1KB blocks to be checked
; R8 is fixed as 1KB=1024bytes - the size of the block. R9 is the Block start position in ITCM. R10-> not used.

	   	MOV		R8, #1024							; load max block size into R8
	   	LDR		r5, =ITCM_VALIDATION_BLOCK			; load IVB Block base 
		LDR		r6, [r5, #IVB_IMAGE_LENGTH]			; read length of image to check from IVB data (check if <32k)
		CMP		r6, #ITCM_SIZE						; compare image size reported with ITCM size
		BGT		normal_boot							; if length was greater than 32k then boot normally
		CMP		r6, #0								; compare image size reported with null length
		BEQ		normal_boot							; if length was zero then boot_normally
		
		SUB		R6, R6, #1							; reduce length by 1 byte to help with block count arithmetic	
		MOV		R7, R6, LSR #10						; divide length by 1024 (for number of blocks required)
		ADD		R7, R7, #1							; add 1 to shift block range 0:31 -> 1:32			
		ADD		R6, R6, #1							; add 1 to length to get it back to correct value


		ADD		r5, r5, #12							; move pointer to first stored CRC in IVB
	   	LDR		r9, =ITCM_BASE						; load ITCM_BASE into R9 which is incremented by block size each loop
		MOV		R2, #1								; load start block number1 into R2

ivbloop	MOV		R3, R8								; start with expection of full 1024byte block
		CMP		R2, R7								; check to see if we are sending final block
		MOVEQ	R3, R6								; R3 becomes the value of R1 (1024 or less in last block)

		LDR		R4, [R5]							; loads the CRC stored in IVB CRC table into R4
		
;; now do the DMA over the required ITCM block
		LDR		r0, =DMA_CTRL_BASE			; load DMA controller base into R0
    	LDR		r1, =FF_SPACE_BASE			; load FF_SPACE_BASE position (sysram block used for flood filling) into R1
		STR		r1, [r0, #DMA_CTRL_ADRS]	; store R1 into the DMA System side address reg	
		STR		r9, [r0, #DMA_CTRL_ADRT]	; store R9 into the DMA TCM side address reg		
    	MOV		r1, #DMA_CMD_BLOCK_CRC_CHK	; DMA control word:  direction=1 (from tcm to sys), CRC=1 (generate)
    	ADD		r1, r1, R3					; adds the transfer block size in bytes to the register
		STR		r1, [r0, #DMA_CTRL_DESC]	; store contents of R1 into the control register to initiate DMA transfer

dmawait2
		LDR		r1, [r0, #DMA_CTRL_STAT]	; read DMA status register
		TST		r1, #DMA_TRANSFER_ACTIVE	; test for transfer still being in progress 
		BNE		dmawait2					; if still transferring, so spin	
		TST		r1, #DMA_TRANSFER_PAUSED	; Any type of error will show as a Paused Transfer 
		BNE		normal_boot					; branch to the normal boot label as ivb boot has failed

		LDR		r1, [r0, #DMA_CTRL_CRCC]	; load calculated CRC
		CMP		R1, R4						; compare calculated CRC with the CRC from the IVB
		BNE		normal_boot					; if they are not the same then this block does not have integrity. Discard & boot normally
		
		ADD		R9, R9, R3					; move to next position in ITCM to be checked
		ADD		R2, R2, #1					; add one to the block counter 
		ADD		R5, R5, #4					; move CRC position on to next CRC32 entry
		SUBS	R6, R6, R3					; reduce the number of bytes to send by the amount just sent and set flags
				
		BNE 	ivbloop						; if we still have tcm data to crc check then do another block
		
;;  if we've got this far then the magic number has worked, the grand CRC too, and all the block CRCs worked out
		LDR		r5, =ITCM_VALIDATION_BLOCK			; load IVB Block base 
		LDR		r0, [r5, #IVB_MAGIC_NUMBER_PLACE]	; load current value of magic number into R0
		MVN		r0, r0								; logical NOT this register
		STR		r0, [r5, #IVB_MAGIC_NUMBER_PLACE]	; write inverted magic number to ensure that we don't loop here for ever
		LDR		r0, [r5, #IVB_GRAND_CRC]			; load current value of grand crc into R0
		MVN		r0, r0								; logical NOT of R0 register
		STR		r0, [r5, #IVB_GRAND_CRC]			; similarly to the grand CRC ensuring we don't end up here again on another crash
		LDR		r1, [r5, #IVB_IMAGE_EXECUTE_ADDR]	; read in the execute address from IVB data
		
		BX 		R1							; send off to the listed execute address (will never come back here) 
				

stack_base			DCD   	||Image$$STACKS$$ZI$$Limit||
image_end	 		DCD		||Region$$Table$$Limit||



		END
