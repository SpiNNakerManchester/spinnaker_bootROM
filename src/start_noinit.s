;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; start.s - program execution begins here at power-on. The code contains some basic
; initialisation and testing and then branches out to main() in main.c. Additional code 
; (ivb_checker) is included to assist reboot after any unexpected reset that does not
; destroy the code in the ITCM. 
;
;
; Created by Mukaram Khan and Xin Jin
; Modified by Thomas Sharp and Cameron Patterson
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



		GET ../inc/globals.s
		
		IMPORT  ||Image$$STACKS$$ZI$$Limit||
		IMPORT  ||Region$$Table$$Limit||
;		IMPORT	disable_interrupts				;BUGZILLA 59. CP 23/08/2010 
;		IMPORT	wait_for_interrupt				;   now both done inline as import was pre-scatter
		IMPORT 	__main
		IMPORT	ram_test
		
		EXPORT	boot_fail
		EXPORT  image_end
		EXPORT  reset
		EXPORT  stack_base
		
		PRESERVE8
		AREA start, CODE, READONLY
		ENTRY



; High exception vectors
		LDR		PC, =reset					; Reset
		B		unexpected_exception_high	; Undefined instruction
		B		unexpected_exception_high	; SVC
		B		unexpected_exception_high	; Prefetch abort
		B		unexpected_exception_high	; Data abort
		B		unexpected_exception_high	; Reserved...
		B		unexpected_exception_high	; IRQ
		B		unexpected_exception_high	; FIQ



; At reset, check GPIO to see if manufacturing tests should be performed
reset

		B stackinit	; for no init test - go straight to the stack setup, and go to the main code
		
	IF :DEF: SPINNAKER2	
		MOV		r7, #SYS_CTRL_BASE
		LDR		r12, [r7, #SYS_CTRL_SET_IO]
		TST		r12, #0x80
		LDRNE	pc, =MANUFACTURING_TESTS
	ENDIF

	IF :DEF: TEST_CHIP ; If working on test chip, make sure to boot second core
		LDR		r0, =DMA_CTRL_BASE
		LDR		r1, [r0, #DMA_CTRL_STAT]
		LSR		r1, r1, #24				; 24 bits rotate to get the processorID in lower 8 bits
		CMP		r1, #0x0
		LDR		r0, =SYS_CTRL_BASE
		MOV		r1, #0x1
		STREQ	r1, [r0, #SYS_CTRL_MISC_CONTROL] ; Switch system RAM and ROM
		LDR		r1, =SECURITY_CODE | 0x2
		STREQ	r1, [r0, #SYS_CTRL_CPU_RESET_PULSE] ; Reset core 1
	ENDIF
	
	IF :DEF: DEBUG ; If debugging in TLM, disable all but 2 processors
		LDR		r0, =DMA_CTRL_BASE
		LDR		r1, [r0, #DMA_CTRL_STAT]
		LSR		r1, r1, #24				; 24 bits rotate to get the processorID in lower 8 bits
		CMP		r1, #0x0
		MCRGT 	p15, 0, r0, c7, c0, 4
	ENDIF

; If watchdog boot, check IVB magic number. If equal, perform IVB routines
		LDR		r0, =SYS_CTRL_BASE
		LDR		r1, [r0, #SYS_CTRL_RESET_CODE]
		CMP		r1, #WATCHDOG_RESET_CODE
		BNE		normal_boot
		LDR		r0, =ITCM_VALIDATION_BLOCK
		LDR		r1, [r0, #IVB_MAGIC_NUMBER_PLACE]
		LDR		r0, =IVB_MAGIC_NUMBER
		CMP		r0, r1
		BEQ		ivb_checker

; Fetch data from serial ROM (if present) then set chip clocks
normal_boot
	IF :DEF: SPINNAKER2
		GET serial.s
	ENDIF
	

	
	
boot_introm
		BL		set_chip_clock

	MOV		r7, #SYS_CTRL_BASE
	LDR		r0, =65
	STR	r0, [r7, #SYS_CTRL_RESET_CODE]	; !!! TLM !!! ** DEBUG **
	
	
; Enable TCMs by writing to co-processor registers
        MRC     p15, 0, r0, c1, c0, 0
        ORR     r0, r0, #(0x1 << 12)
        ORR     r0, r0, #(0x1 << 2)
        MCR     p15, 0, r0, c1, c0, 0

; Initialise this core`s failure log to 0 to indicate no fault found yet
		MOV		r0, #0
		BL		write_failure_log

; Test TCMs using ram_test code, disable interrupts and go to sleep in case of failure
		MOV		r0, #ITCM_BASE
	IF :DEF: DEBUG
		LDR		r1, =(ITCM_BASE + 0x20)
	ELSE
		LDR		r1, =(ITCM_BASE + ITCM_SIZE)
	ENDIF
		LDR		r2, =0xFFFFFFFF
		BL		ram_test
		CMP		r0, #0
		MOVNE	r0, #ITCM_FAILURE
		BNE		boot_fail
		
		MOV		r0, #DTCM_BASE
	IF :DEF: DEBUG
		LDR		r1, =(DTCM_BASE + 0x20)
	ELSE
		LDR		r1, =(DTCM_BASE + DTCM_SIZE)
	ENDIF
		LDR		r2, =0xFFFFFFFF
		BL		ram_test
		CMP		r0, #0
		MOVNE	r0, #DTCM_FAILURE
		BNE		boot_fail

; Initialise stacks for each used mode, switching interrupts off in the process
stackinit
		LDR     r0, stack_base

		MSR     CPSR_c, #SVC_MODE | IRQ_OFF | FIQ_OFF
		MOV     sp, r0
		SUB		r0, r0, #SVC_STACK_SIZE
		
		MSR     CPSR_c, #IRQ_MODE | IRQ_OFF | FIQ_OFF
		MOV     sp, r0
		SUB		r0, r0, #IRQ_STACK_SIZE
		
        MSR     CPSR_c, #SVC_MODE | IRQ_OFF | FIQ_OFF
		
; Branch to ARM routines that initialise state, scatter, and branch to main()
		B		__main



; Record failure of peripheral, disable interrupts and go to sleep (takes error code in r0)
boot_fail
		BL		write_failure_log		; Record peripheral failure
;		BL		disable_interrupts		;BUGZILLA 59. CP 23/08/2010  
		MRS		r0, CPSR				;    	interrupts disabled in-line
		ORR		r0, r0, #IRQ_OFF | FIQ_OFF	;	as function is in scattered code
		MSR		CPSR_c, r0				;BUGZILLA 59. CP 23/08/2010
		
;BUGZILLA 59. CP 20/08/2010
	; Work out if we are the monitor (and have to turn on the RED LEDs of doom)
		MOV		r0, #DMA_CTRL_BASE		
		LDR		r1, [r0, #DMA_CTRL_STAT]
		LSR		r1, r1, #24							; 24 bits rotate to get the processorID in lower 8 bits

		MOV		r0, #SYS_CTRL_BASE		
		LDR		r2, [r0, #SYS_CTRL_MONITOR_ID]
		AND		r2, r2, #0x1F  						; Five bits covering Monitor ID 
		
		CMP		r1,	r2								; are we the monitor?
		BNE		vic_off								; if not the monitor then no LED messin'
		
		LDR     r1, [r0, #SYS_CTRL_IO_DIRECTION]  	; Get current status of GPIO pin directions
		BIC     r1, r1, #0xC3						; set all LED GPIO bits to outputs (zero them)
		STR     r1, [r0, #SYS_CTRL_IO_DIRECTION]  	; write back adjusted GPIO direction

		MOV		r1, #0xC3					
		STR     r1, [r0, #SYS_CTRL_SET_IO]	  		; turn off all LEDs.
		MOV		r1, #0x82					
		STR     r1, [r0, #SYS_CTRL_CLEAR_IO]		; light the red LEDs.
		
	; BUGZILLA 59. CP 24/08/2010. as we are the monitor, and we are now aborting and going to sleep, 
	; 					we need to allow the fascicle to exit its spin wait, and go on to sleep in main
		LDR		r1, [r0, #SYS_CTRL_MISC_CONTROL]	; read current misc control register
		ORR		r1, r1, #0x8  						; Set bit3 in misc reg, indicating monitor specific setup done
		STR		r1, [r0, #SYS_CTRL_MISC_CONTROL]	; write updated misc control register
		
	; Turn off interrupts in VIC for this core as we are going to turn it off (to sleep)
vic_off	LDR		r0, =VIC_BASE			 
		MOV		r1, #0xFFFFFFFF						; everything!
		STR		r1, [r0, #VIC_ENABLE_CLEAR]	 		;clears all interrupt enables
;BUGZILLA 59. CP 20/08/2010		

;guard	BL		wait_for_interrupt		; BUGZILLA 59. CP 23/08/2010,	
guard	MCR 	p15, 0, r0, c7, c0, 4	;	now inline as previously this code had not been scattered
		B		guard					; BUGZILLA 59. CP 20/08/2010 (if for some reason we exit, go back to sleep!)


; Write a value in r0 to failure log (either for initialisation to 0, or to record error code)
write_failure_log
	IF :DEF: TEST_CHIP
		LDR		r1, =TEST_CHIP_FAILURE_LOG_BASE
	ELSE
		LDR		r1, =SPINNAKER2_FAILURE_LOG_BASE
	ENDIF
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
	IF :DEF: TEST_CHIP
		LDR		r1, =TEST_CHIP_CLK_MUX_CTRL_SETUP
	ELSE
		LDR		r1, =SPINNAKER2_CLK_MUX_CTRL_SETUP
	ENDIF
		STR		r1, [r0, #SYS_CTRL_CLK_MUX_CTRL]
		BX		lr



unexpected_exception_high
		MOV		r0, #UNEXPECTED_EXCEPTION_HIGH
		B		boot_fail



ivb_checker

; we have got here because the magic number is correct in ITCM, so we have a chance to recover to code here

;; do DMA write of 35 ctrl+blockcrcs in ITCM to SysRAM with CRC ON
		LDR		r0, =DMA_CTRL_BASE			; load DMA controller base into R0

	IF :DEF: TEST_CHIP
    	LDR		r1, =TEST_CHIP_FF_SPACE_BASE			; load FF_SPACE_BASE position (sysram block used for flood filling) into R1
	ELSE
		LDR		r1, =SPINNAKER2_FF_SPACE_BASE			; load FF_SPACE_BASE position (sysram block used for flood filling) into R1
	ENDIF
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
	IF :DEF: TEST_CHIP
    	LDR		r1, =TEST_CHIP_FF_SPACE_BASE			; load FF_SPACE_BASE position (sysram block used for flood filling) into R1
	ELSE
		LDR		r1, =SPINNAKER2_FF_SPACE_BASE			; load FF_SPACE_BASE position (sysram block used for flood filling) into R1
	ENDIF
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
