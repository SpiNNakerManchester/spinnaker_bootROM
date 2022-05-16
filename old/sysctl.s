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
		GET		../inc/globals.s

 		EXPORT  testSysCtl
 		EXPORT	sendProcInt
 		EXPORT  clearProcInterrupt
 		EXPORT  clearCPUStatus

 		AREA 	SYS_CTRL, CODE, READONLY
 	
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
		BX		LR		;LDMFD	sp!,  {r0, r1, pc} 

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
    	BX		LR		;LDMFD	SP!, {R4, R5, PC}		;LDMFD	sp!,  {r0, r1, pc} 
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
    	BX		LR				;LDMFD	SP!, {R4, R5, PC}		;LDMFD	sp!,  {r0, r1, pc}



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
    	BX		LR					;LDMFD	SP!, {R4, R5, PC}	;LDMFD	sp!,  {r0, r1, pc}    	
   		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; printChar     //to print a char on the console
;				//char is defined by address in R0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 

sendCharASM
		MOV	R2, #SYSCTLR_BASE_U		; r13 -> Sys Ctlr
		STR	R0, [R2, #SYSCTLR_TUBE]
		BX		LR

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

