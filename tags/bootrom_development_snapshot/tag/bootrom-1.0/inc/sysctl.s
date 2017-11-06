;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; sysctl.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET ../inc/option.s     
 		
 		IMPORT  enableEthernet
 		IMPORT	procID
 		;IMPORT  procChipStatus
 		
 		;EXPORT	initSysCtl
 		EXPORT  setChipClock
 		EXPORT  testSysCtl
 		EXPORT	sendProcInt
 		EXPORT  clearProcInterrupt
 		EXPORT  setCPUStatus
 		EXPORT  clearCPUStatus
 		EXPORT  setChipStatus

 		AREA 	sysctl, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SYSTEM CTRL INIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
;initSysCtl
;										;STMFD	SP!, {R4, R5, LR}					;STMFD	sp!,  {r0, r1, lr}
;   	LDR		R2,	=SYSCTLR_BASE_U
;    	MOV		R1,	#0x0					;arbitrary value, shall be replaced with some meaningful value
;    	STR		R1,	[R2, #SYSCTLR_MISC_CONTROL]
;    	MOV		PC, LR					;LDMFD	SP!, {R4, R5, PC}			;LDMFD	sp!,  {r0, r1, pc} 	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SYSTEM CTRL TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testSysCtl
							;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	=SYSCTLR_BASE_U
    	LDR		R1,	=0xABCDEF37
    	LDR		R3, [R2, #SYSCTLR_MISC_CONTROL]
    	STR		R1,	[R2, #SYSCTLR_MISC_CONTROL]
    	LDR		R0, [R2, #SYSCTLR_MISC_CONTROL]
    	CMP		R1, R0
    	MOVNE	R0, #0x0					;return 0 i.e. fail
    	BNE		outsct
    	MOV		R0,	#0x1
    	;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, R1, #(0x1 << 9)			;saving status of sys ctl at bit 9 
    	;STR		R1, [R2]
    	;ORR		R11, R11, #(0x1 << 9) ;saving status of sys ctl at bit 9 
outsct 	
		STR		R3,	[R2, #SYSCTLR_MISC_CONTROL]	 ;restoring old value
		MOV		PC, LR		;LDMFD	sp!,  {r0, r1, pc} 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; sendProcInt(int procID);   //procID to be passes in ro
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
sendProcInt
							;STMFD	SP!, {R4, R5, LR}					;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	=SYSCTLR_BASE_U
    	MOV		R1, #0x1
    	LSL		R0, R1, R0		   ;setting a bit at the relevant location defined by procID passed in R0
    	LDR		R1, =0x7FFFF
    	AND     R0, R1             ;to ensure it does not go beyond 20 bits
    	LDR		R1, =0x5EC
    	LSL		R1, R1, #20
    	ORR		R0, R0, R1
    	STR		R0,	[R2, #SYSCTLR_SET_CPU_IRQ]
    	MOV		PC, LR		;LDMFD	SP!, {R4, R5, PC}		;LDMFD	sp!,  {r0, r1, pc} 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; clearProcInterrupt(int procID);  //to clear the interrupt by 
; relavent processor, procID in ro
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
clearProcInterrupt
									;STMFD	SP!, {R4, R5, LR}					;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	=SYSCTLR_BASE_U
    	MOV		R1, #0x1
    	LSL		R0, R1, R0		   ;setting a bit at the relevant location defined by procID passed in R0
    	LDR		R1, =0x7FFFF
    	AND     R0, R1             ;to ensure it does not go beyond 20 bits
    	LDR		R1, =0x5EC
    	LSL		R1, R1, #20
    	ORR		R0, R0, R1
    	STR		R0,	[R2, #SYSCTLR_CLR_CPU_IRQ]
    	MOV		PC, LR				;LDMFD	SP!, {R4, R5, PC}		;LDMFD	sp!,  {r0, r1, pc}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; int setCPUStatus(int procID, unsigned int status);       //to set up cpu status in system ctl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
setCPUStatus
								;STMFD	SP!, {R4, R5, LR}				;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	=SYSCTLR_BASE_U
    	CMP		R1, #0xF						;4 bits should be high for all core peripherals
    	MOVNE	R0, #0x0						;fail return
    	BNE		outscpustatus
    	MOV		R1, #0x1
    	LSL		R1, R0
       	LDR		R0, =0x7FFFF
    	AND     R1, R0             ;to ensure it does not go beyond 20 bits
    	STR		R1,	[R2, #SYSCTLR_SET_CPU_OK]
    	MOV		R0, #0x1						;success return
outscpustatus
   		MOV     PC, LR			;LDMFD	SP!, {R4, R5, PC}		;LDMFD	sp!,  {r0, r1, pc}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; clearCPUStatus(int procID);       //to set up cpu status in system ctl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
clearCPUStatus
										;STMFD	SP!, {R4, R5, LR}				;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	=SYSCTLR_BASE_U
    	MOV		R1, #0x1
    	LSL		R0, R1, R0		   ;setting a bit at the relevant location defined by procID passed in R0
    	LDR		R1, =0x7FFFF
    	AND     R0, R1             ;to ensure it does not go beyond 20 bits
    	STR		R0,	[R2, #SYSCTLR_CLR_CPU_OK]
    	MOV		PC, LR					;LDMFD	SP!, {R4, R5, PC}	;LDMFD	sp!,  {r0, r1, pc}    	


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; int setChipStatus(unsigned int status);       //to set up cpu status in system ctl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
setChipStatus
											;STMFD	SP!, {R4-R7, LR}				;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	=SYSCTLR_BASE_U
    	;todo, check for all the 9 bits and then set the status in cpu status register bits 20 to 28
    	LDR		R1,  =enableEthernet				;if ethernet enabled
    	LDR		R1, [R1]
    	CMP		R1, #0x0						;if not mon proc then
    	LDREQ	R1, =0x3F						; test with 0x3F i.e. excluding ethernet and phy test	
    	LDRNE	R1, =0xFF						;else 8 bits should be high for all chip peripherals 
    	LSR		R0, R0, #0x8					;right shift 8 bits to clear first 8 bits related to the proc peripherals
    	CMP		R0, R1						    
    	MOVNE	R0, #0x0						;fail return
    	BNE		outchips
    	LSL		R0, R0, #20						; saving the chip status in the high bit (over 20) of SYSCTLR_SET_CPU_OK register
    	LDR		R1, [R2, #SYSCTLR_SET_CPU_OK]
    	ORR		R1, R1, R0
    	STR		R1, [R2, #SYSCTLR_SET_CPU_OK]  ;saving chip status to sys ctl, SYSCTLR_SET_CPU_OK bits over 20
    	MOV		R0, #0x1						;success return
    	
outchips
   		MOV 	PC, LR   					;LDMFD	SP!, {R4-R7, PC}		;LDMFD	sp!,  {r0, r1, pc}
   		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; printChar     //to print a char on the console
;				//char is defined by address in R0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 

sendCharASM
		MOV	R2, #SYSCTLR_BASE_U		; r13 -> Sys Ctlr
		STR	R0, [R2, #SYSCTLR_TUBE]
		MOV 	PC, LR

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; setChipClock   // to be refined to have even odd disparity
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  

; !! ST - set frequencies low to be on the safe side!

setChipClock
		LDR	R2, =SYSCTLR_BASE_U
;
;!!    		LDR	R1, =0x00060114			; NS[5:0] 5:0 R/W input clock multiplier : 010100 = 200MHz
;
    		LDR	R1, =0x0006010c			; NS[5:0] 5:0 R/W input clock multiplier                 : 001100 = 120MHz
							; MS[5:0] 13:8 R/W output clock divider	                 : 000001
							; FR[1:0] 17:16 R/W frequency range (10 = 100 to 200MHz) : 10
							; P 18 R/W NOT power down				 : 1
							; T 24 R/W test						 : 0
							; 0000 0000 0000 0110 0000 0001 0001 0100 = 0x00050114
		STR	R1, [R2, #SYSCTLR_PLL1]
;
;!!		LDR	R1, =0x00070121			; PLL1 - 330 MHz (!! PLL2?)
;
		LDR	R1, =0x00060114			; PLL2 - 200 MHz
							; 0000 0000 0000 0111 0000 0001 0010 0001 = 0x0070121
		STR	R1, [R2, #SYSCTLR_PLL2]
		
		MOV     R1, #200			; NEW CODE by ST to stabalize the PLL
dloop   	SUBS    R1, R1, #1              	; NEW CODE
        	BNE     dloop                   	; NEW CODE
        
		LDR	R1, =0x50000165			; Pe[1:0] 1:0 R/W clock selector for even numbered CPUs  : 01
							; Po[1:0] 3:2 R/W clock selector for odd numbered CPUs   : 01
							; Mem[1:0] 5:4 R/W clock selector for SDRAM		 : 10
							; Rtr[1:0] 7:6 R/W clock selector for Router		 : 01
							; Sys[1:0] 9:8 R/W clock selector for System AHB components : 01
							; Toric[1:0] 17:16 R/W clock selector for Toric clock synthesizer : 00
							; E 24 R/W divide even CPU clock by E+1 (= 1-2)			: 0
							; D 25 R/W divide odd CPU clock by D+1 (= 1-2)			: 0
							; M 26 R/W divide SDRAM clock by M+1 (= 1-2)			: 0
							; R 27 R/W divide Router clock by R+1 (= 1-2)			: 0
							; V 28 R/W invert odd CPU clock					: 1
							; Sdiv[1:0] 31:30 R/W divide System AHB clock by Sdiv+1 (= 1-4) : 01
							; 0101 0000 0000 0000 0000 0001 0110  0101  = 0x50000165
	    						; Set clock muxes
		STR	R1, [R2, #SYSCTLR_CLK_MUX_CTL]   
		MOV	PC, LR   		
;
		END                     
;old code by ST	
;		LDR		R2,	=SYSCTLR_BASE_U
;    		LDR		R1, =0x00070120		; PLL1 - 320 MHz
;		STR		R1, [R2, #SYSCTLR_PLL1]
;		LDR		R1, =0x00070118		; PLL2 - 240 MHz
;		STR		R1, [R2, #SYSCTLR_PLL2]
;		LDR		R1, =0x9b00029a		; Set clock muxes
;		STR		R1, [R2, #SYSCTLR_CLK_MUX_CTL]   
;		MOV		PC, LR     

