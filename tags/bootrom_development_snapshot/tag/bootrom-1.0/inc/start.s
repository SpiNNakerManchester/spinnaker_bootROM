;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; start.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET ../inc/peripherals.s        
 		
 		IMPORT  ||Image$$STACKS$$ZI$$Limit||
 		IMPORT  ||Region$$Table$$Limit||
       	IMPORT 	__main
       	IMPORT  procID 
       
   		EXPORT	HiVectors
 		EXPORT  stack_base
 		EXPORT  image_end
 		EXPORT  Reset_HandlerHi
 		
 		AREA 	start, CODE, READONLY
        ENTRY
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; reset hiVectors for reset
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
HiVectors
        LDR     PC, =Reset_HandlerHi
        LDR     PC, =Undefined_HandlerHi
        LDR     PC, =SWI_HandlerHi
        LDR     PC, =Prefetch_HandlerHi
        LDR     PC, =Abort_HandlerHi
        LDR     PC, =Reserved_HandlerHi                             	;Reserved vector
		LDR	 	PC, =LoadHiIRQ						;VIC IRQ
        LDR     PC, =FSRoutineHi	        
FSRoutineHi
		;clear the interrupt source here
		MSR		CPSR_c, #Mode_SYS:OR:I_Bit:OR:F_Bit		;changing to system mode with insterrupts disable
		LDR		R0, =VIC_BASE
		LDR		R1, =0xFFFFFFFF				;clear all interrupts in the VIC as we don't expect any interrupts at this time
		STR		R1, [R0, #VICINTENCLEAR]
		SUBS	PC, LR, #0x4				;restore to the actual code
		   
; ************************
; Exception Handlers
; ************************

; The following dummy handlers do not do anything useful in this example.
; They are set up here for completeness.

Undefined_HandlerHi
        B       gotoSleepHi
SWI_HandlerHi
        B       gotoSleepHi     
Prefetch_HandlerHi
        B       gotoSleepHi
Abort_HandlerHi
        B       gotoSleepHi
Reserved_HandlerHi
        B       gotoSleepHi
LoadHiIRQ
		;clear the interrupt source here
		MSR		CPSR_c, #Mode_SYS:OR:I_Bit:OR:F_Bit		;changing to system mode with insterrupts disable
		LDR		R0, =VIC_BASE
		LDR		R1, =0xFFFFFFFF				;clear all interrupts in the VIC as we don't expect any interrupts at this time
		STR		R1, [R0, #VICINTENCLEAR]
		SUBS	PC, LR, #0x4				;restore to the actual code
VectorEndAddressHi
		MOV		R0, #0x0        

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; reset handler to start the code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;!! ST - added serial boot code

Reset_HandlerHi	
		GET	serial.s
boot_introm
;!!		BL	setChipClock   ;done temporarily to speed up the process, will be removed in the final code

		;****************************************************
        ;TCMs enable    							        	
        ;****************************************************
        MRC     p15, 0, R0, c1, c0, 0       ; read CP15 register 1 into R4
        ORR     R0, R0, #(0x1  <<12)        ; enable ITCM
        ORR     R0, R0, #(0x1  <<2)         ; enable DTCM
        MCR     p15, 0, R0, c1, c0, 0       ; write CP15 register 1
        ;****************************************************
        ;TCMs enable    							        	
        ;****************************************************
        ;the bootup process will use R11 to maintain the status of the core and chip peripherals
        MOV		R11, #0x0				;initialising r11 with all faulty peripherals value
        ;------ testing ITCM --------------------------------
        BL		testITCMHi
        ;------ testDTCM ------------------------------------
        BL		testDTCMHi
        ; branch to sleep if failed
        CMP		R11, #0x3     				;R11 be compared with 0x3 to see if TCM tests are fine
        BNE		goSleep
        ;********************************************************************    
        ; stack_base could be defined below, or located in a scatter file
        ;******************************************************************** 
        LDR     R0, stack_base 
        ; Enter each mode in turn and set up the stack pointer
		; --- Now change to SVC mode and set up SVC mode stack.
        MSR     CPSR_c, #Mode_SVC:OR:I_Bit:OR:F_Bit ; No interrupts
        MOV     SP, R0
	    ; --- Now change to FIQ mode and set up FIQ mode stack.
        MSR     CPSR_c, #Mode_FIQ:OR:I_Bit:OR:F_Bit
        SUB     R0, R0, #Len_SVC_Stack  
        MOV     SP, R0
		; --- Now change to IRQ mode and set up IRQ mode stack.
        MSR     CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
        SUB     R0, R0, #Len_FIQ_Stack
        MOV     SP, R0
        ; --- Now change to SYS mode and set up System/User mode stack.
        MSR     CPSR_c, #Mode_SYS:OR:I_Bit:OR:F_Bit
        SUB     R0, R0, #Len_IRQ_Stack
        MOV     SP, R0
        ; --- Now change to Abort mode and set up Abort mode stack.
        MSR     CPSR_c, #Mode_ABT:OR:I_Bit:OR:F_Bit
        SUB     R0, R0, #Len_USR_Stack
        MOV     SP, R0
        ; --- Now change to Undef mode and set up undef mode stack.
        MSR     CPSR_c, #Mode_UND:OR:I_Bit:OR:F_Bit
        SUB     R0, R0, #Len_ABT_Stack
        MOV     SP, R0
       	; --- Now change to SVC mode for rest of bootup process
        MSR     CPSR_c, #Mode_SVC:OR:I_Bit:OR:F_Bit ; No interrupts
        ; ----- Keep in SVC mode for the duration of boot up 
        ;LDR   R12, =__main                 ; save this in register for possible long jump
        B      __main   ; note use B not BL, because an application will never return this way
goSleep
        B		gotoSleepHi
          
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; gotoSleepHi
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
gotoSleepHi
		;set the bit in the system ctlr before sleeping
		STMFD	SP!,  {R0-R12, LR}
		MCR 	p15, 0, R4, c7, c0, 4
		LDMFD   SP!, {R0-R12, PC}             
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ITCM TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testITCMHi    ;using R11 to maintain the status of core peripherals
	  	LDR		R1,	  	=ITCM_BASE	
	    MOV		R3,	  	#0x80
looptitcmHi
	  	LDR		R0,   	=0xABCDEF37
		STR		R0,   	[R1]
	 	LDR		R2,  	[R1]
	 	ADD		R1,		R1,		#0x100
	    CMP		R0, 	R2
	    MOVNE	R0,		#0x0			;return false i.e. 0
	    BNE		outitcmHi
	    SUB		R3,   	R3, 	#0x1
	    CMP		R3, 	#0x0
	    BGT		looptitcmHi
	    MOV		R0,		#0x1			;test pass, return true i.e. 1
	    ORR		R11, R11, #(0x1 << 0)	;status of ITCM	at bit 0
outitcmHi
		MOV		PC, LR  						

		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DTCM TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 		
testDTCMHi
		LDR		R1,	  	=DTCM_BASE	
	    MOV		R3,	  	#0x100
looptdtcmHi
	  	LDR		R0,   	=0xABCDEF37
		STR		R0,   	[R1]
	 	LDR		R2,  	[R1]
	 	ADD		R1,		R1,		#0x100
	    CMP		R0, 	R2
	    MOVNE	R0,		#0x0			;return false i.e. 0
	    BNE		outdtcmHi
	    SUB		R3,   	R3, 	#0x1
	    CMP		R3, 	#0x0
	    BGT		looptdtcmHi
	    MOV		R0,		#0x1			;test pass, return true i.e. 1
	    ORR		R11,    R11, #(0x1 << 1)	;status of DTCM	at bit 0
outdtcmHi
		MOV		PC, LR  		

;setChipClock
;		LDR		R2,	=SYSCTLR_BASE_U
;    	LDR		R1, =0x00060114			;NS[5:0] 5:0 R/W input clock multiplier : 010100 = 200MHz
										;MS[5:0] 13:8 R/W output clock divider	: 000001
										;FR[1:0] 17:16 R/W frequency range (10 = 100 to 200MHz) : 10
										;P 18 R/W NOT power down				: 1
										;T 24 R/W test							: 0
										; 0000 0000 0000 0110 0000 0001 0001 0100 = 0x00050114
;		STR		R1, [R2, #SYSCTLR_PLL1]
;		LDR		R1, =0x00070121 	    ; PLL1 - 330 MHz
									    ; 0000 0000 0000 0111 0000 0001 0010 0001 = 0x0070121
;		STR		R1, [R2, #SYSCTLR_PLL2]
;		MOV     R1, #200                ; NEW CODE by ST to stabalize the PLL
;dloop   SUBS    R1, R1, #1              ; NEW CODE
;        BNE     dloop                   ; NEW CODE
;		LDR		R1, =0x50000165			;Pe[1:0] 1:0 R/W clock selector for even numbered CPUs  : 01
										;Po[1:0] 3:2 R/W clock selector for odd numbered CPUs   : 01
										;Mem[1:0] 5:4 R/W clock selector for SDRAM				: 10
										;Rtr[1:0] 7:6 R/W clock selector for Router				: 01
										;Sys[1:0] 9:8 R/W clock selector for System AHB components : 01
										;Toric[1:0] 17:16 R/W clock selector for Toric clock synthesizer : 00
										;E 24 R/W divide even CPU clock by E+1 (= 1-2)			: 0
										;D 25 R/W divide odd CPU clock by D+1 (= 1-2)			: 0
										;M 26 R/W divide SDRAM clock by M+1 (= 1-2)				: 0
										;R 27 R/W divide Router clock by R+1 (= 1-2)			: 0
										;V 28 R/W invert odd CPU clock							: 1
										;Sdiv[1:0] 31:30 R/W divide System AHB clock by Sdiv+1 (= 1-4) : 01
										;0101 0000 0000 0000 0000 0001 0110  0101  = 0x50000165
	    ; Set clock muxes
;		STR		R1, [R2, #SYSCTLR_CLK_MUX_CTL]   
;		MOV		PC, LR   		
   	
		
		;old code by ST
		;LDR		R2,	=SYSCTLR_BASE_U
    	;LDR		R1, =0x00070120		; PLL1 - 320 MHz
		;STR		R1, [R2, #SYSCTLR_PLL1]
		;LDR		R1, =0x00070118		; PLL2 - 240 MHz
		;STR		R1, [R2, #SYSCTLR_PLL2]
		;LDR		R1, =0x9b00029a		; Set clock muxes
		;STR		R1, [R2, #SYSCTLR_CLK_MUX_CTL]   
		;MOV		PC, LR     
		
     

stack_base    			DCD   	||Image$$STACKS$$ZI$$Limit||
image_end	 			DCD		||Region$$Table$$Limit||

		END
		
		
; --- use of precompiler construct #ifdef    #endif
;   IF :DEF: ROM_RAM_REMAP

; On reset, an aliased copy of ROM is at 0x0.
; Continue execution from 'real' ROM rather than aliased copy
;       LDR     pc, =Instruct_2

; Remap by writing to ClearResetMap in the RPS Remap and Pause Controller
;       MOV     r0, #0
;       LDR     r1, =ClearResetMap
;        STRB    r0, [r1]

; RAM is now at 0x0.
; The exception vectors (in vectors.s) must be copied from ROM to the RAM
; The copying is done later by the C library code inside __main

;    ENDIF