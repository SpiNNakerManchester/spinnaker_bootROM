;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; watchdog.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET option.s     
 		
 			
 		EXPORT	initWatchdog
 		EXPORT  testWatchdog
 	
 		AREA 	watchdog, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; WATCHDOG TIMER INIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
initWatchdog
								;STMFD	SP!, {R4, R5, LR}					    	  ;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	=WDOG_BASE
    	LDR		R1,	=0x1ACCE551		  ;unlock value to enable writing the registers
    	STR		R1,	[R2, #WDOG_LOCK]
    	LDR		R0, =1000000          ;20 ms timer

    	MOV		R1, #WDOG_COUNTER_VALUE

; !! ST modified next (UNPREDICTABLE behaviour!)
;
;     	MUL		R0, R0, R1

		MOV		R3, R0
     	MUL		R0, R3, R1

    	STR		R0, [R2, #WDOG_LOAD]
    	MOV		R1,	#0x03			  ;enable interrupt and reset outputs
    	STR		R1, [R2, #WDOG_CONTROL]
    	STR		R1,	[R2, #WDOG_LOCK]  ;writing value other than 0x1ACCE551 to lock the watchdog registers against an accidental write
		BX		LR		;LDMFD	SP!, {R4, R5, PC}					  ;LDMFD	sp!,  {r0, r1, pc} 	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; WATCHDOG TIMER TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testWatchdog				;new test as per Watchdog's programer's model for test Watchdog technical reference manual
;   	LDR		R2,	=WDOG_BASE
;    	LDR		R1,	=0x1ACCE551		  ;unlock value to enable writing the registers
;    	STR		R1,	[R2, #WDOG_LOCK]
;    	MOV		R0, #0x1
;    	STR		R0,	[R2, #WDOG_ITCR]  ;enable the integrated test by setting bit 0=1
;    	MOV		R0, #(0x1 << 1)       ;setting the watchdog interrupt bit to high to get an interrupt in int ctlr
;    	STR		R0, [R2, #WDOG_ITOP] 
;    	LDR		R3, =VIC_BASE
;    	LDR		R0, [R3, #VICRAWINTR] ;read raw interrupt register to make out if the watchdog sent an interrupt
;    	TST		R0, #0x1			  ;IF int line 0 (for watchdog) is high
;    	MOVNE	R0, #0x0			  ;return 0 i.e. fail
;    	BNE		outtwd
;outtwd 	MOV		R3, #0
;		STR		R3, [R2, #WDOG_ITOP]  ;clearing the interrupt
;		STR		R3,	[R2, #WDOG_ITCR]  ;disabling the intergrated test register
;		STR		R1,	[R2, #WDOG_LOCK]  ;writing value other than 0x1ACCE551 to lock the watchdog registers against an accidental write
;		MOV		PC,  LR					  ;LDMFD	sp!,  {r0, r1, pc} 
   		LDR		R2,	=WDOG_BASE
    	LDR		R1,	=0x1ACCE551		  ;unlock value to enable writing the registers
	  	STR		R1,	[R2, #WDOG_LOCK]
    	MOV		R1, #0x1
    	STR		R1,	[R2, #WDOG_ITCR]  ;enable the integrated test by setting bit 0=1
    	LDR		R0,	[R2, #WDOG_ITCR]
    	CMP		R1, 	R0
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtwd
    	MOV		R0,		#0x1
    	
outtwd   
		MOV		R1,	#0x0
		STR		R1,	[R2, #WDOG_ITCR]	
		STR		R1,	[R2, #WDOG_LOCK]
		BX		LR				;LDMFD	sp!,  {r0, r1, pc} 	    	

		END                     
   	



