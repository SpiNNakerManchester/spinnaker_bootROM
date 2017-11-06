;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; timer.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET ../inc/option.s     
 		
 		;IMPORT  procChipStatus
 		
 		EXPORT	initTimer
 		;EXPORT  initTimer2
 		EXPORT  testTimer
 		AREA 	timer, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TIMER INIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
initTimer
										;STMFD	SP!,  {R0, R1, LR}								;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2,	  =TIMER_BASE
    	LDR		R1,	  =TIMER1_OPT_VALUE
    	STR		R1,	  [R2, #TIMER1Load]
    	STR		R1,	  [R2, #TIMER1BGLoad]    		
    	LDR		R1,	  =TIMER1_OPT_CONTROL
    	STR		R1,	  [R2, #TIMER1Control]
    	MOV		PC,   LR    			;LDMFD	SP!,  {R0, R1, PC}						;LDMFD	SP!,  {R0, R1, PC} 	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TIMER INIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
;initTimer2
;										;STMFD	SP!,  {R4, R5, LR}								;STMFD	sp!,  {r0, r1, lr}
;    	LDR		R2,	  =TIMER_BASE
;    	LDR		R1,	  =TIMER2_OPT_VALUE
;    	STR		R1,	  [R2, #TIMER2Load]
;    	STR		R1,	  [R2, #TIMER2BGLoad]    		
;    	LDR		R1,	  =TIMER2_OPT_CONTROL
;    	STR		R1,	  [R2, #TIMER2Control]
;    	MOV		PC,	  LR				;LDMFD	SP!,  {R4, R5, PC}						;LDMFD	SP!,  {R4, R1, PC} 	


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TIMER TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;testTimerIntegratedTest				;new test as per Timer's programer's model for test Timer technical reference manual
 ;   	LDR		R2,	=TIMER_BASE
  ;  	LDR		R1, =0xFFFFFFFF
   ; 	STR		R1, [R2, #TIMER1Load]
    ;	LDR		R1,	=TIMER1_OPT_CONTROL    ;enabling the timer1 interrupt
   ; 	STR		R1,	[R2, #TIMER1Control]
   ; 	MOV		R0, #0x1
   ; 	STR		R0,	[R2, #TIMERITCR]  ;enable the integrated test by setting bit 0=1
   ; 	MOV		R0, #0x1       ;setting the timer1 interrupt bit to high to get an interrupt in int ctlr
   ; 	STR		R0, [R2, #TIMERITOP] 
   ; 	LDR		R3, =VIC_BASE
   ; 	LDR		R0, [R3, #VICRAWINTR] ;read raw interrupt register to make out if the watchdog sent an interrupt
   ; 	TST		R0, #(0x1 << 4)		  ;if int line 0 (for watchdog) is high
   ; 	MOVNE	R0, #0x0			  ;return 0 i.e. fail
   ; 	BNE		outtt
    	;LDR		R3, =procChipStatus
    	;LDR		R0, [R3]
    	;ORR		R0, R0, #(0x1 << 12) ;saving status of watchdog at bit 12 
    	;STR		R0, [R3]
    	;ORR		R11, R11, #(0x1 << 12) ;saving status of watchdog at bit 12 
;outtt 	MOV		R3, #0
;		MOV		R1,	#0x0   ;enabling the timer1 interrupt
;    	STR		R1,	[R2, #TIMER1Control]
    	
;		STR		R3, [R2, #TIMERITOP]  ;clearing the interrupt
;		STR		R3,	[R2, #TIMERITCR]  ;disabling the intergrated test register
;		MOV		PC,  LR		


;old test									;STMFD	sp!,  {r0, r1, lr}
testTimer
    	LDR		R2,	  	=TIMER_BASE
    	LDR		R1,		=0x1
    	STR		R1,		[R2, #TIMERITCR]        ; timer integrated test register
    	LDR		R0, 	[R2, #TIMERITCR]
    	CMP		R1, 	R0
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtt
    	MOV		R0,		#0x1
    	;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, R1, #(0x1 << 3)				;status of timer at bit 3
    	;STR		R1, [R2]
    	;ORR		R11, R11, #(0x1 << 3)				;status of timer at bit 3
outtt  
		MOV		R1,		#0x0
		STR		R1,		[R2, #TIMERITCR]	
		MOV		PC,   LR				;LDMFD	sp!,  {r0, r1, pc} 	
    	

		END                     
 	    	