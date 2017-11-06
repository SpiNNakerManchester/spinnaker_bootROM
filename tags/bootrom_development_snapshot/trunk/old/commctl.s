;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; commctlr.s file 
; 
; used in the device drivers' coding
; created by mukaram khan 
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
       GET peripherals.s       
 		
 		;IMPORT  procChipStatus
; 		EXPORT	initCommCtl
 ;		EXPORT  testCommCtl
 ;		EXPORT  SetupSourceID
 		EXPORT  writePacketWOPL
        EXPORT  writePacketWPL
       ; EXPORT	writePacketASM
 ;       EXPORT	sendSpike
 		
 		AREA 	commctl, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; COMMCTLR init. Commented out by TS. See peripheral_inits.c
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;initCommCtl
;		LDR		R2, =COMMCTLR_BASE
;		LDR		R3, =0x07000000    			          								;default route =7 i.e. internal port to comm. noc.
;		STR		R3, [R2, #COMMCTLR_R0]
;		BX		LR  			;LDMFD	SP!, {R4-R7, PC}						;LDMFD	sp!, {r4-r12, pc}
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; COMMCTLR setupSourceID, new id in R0 commented out by TS. Never called
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;SetupSourceID
;		LDR		R2, =COMMCTLR_BASE
;		STRH	R0, [R2, #COMMCTLR_R0]
;		BX		LR				;LDMFD	SP!, {R4, PC}
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; commented out by TS. See peripheral_tests.c
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;testCommCtl
;		LDR		R2,	 =COMMCTLR_BASE
;    	MOV		R0,	#0x1		
;    	STR		R0, [R2, #COMMCTLR_R7]		;enabling test writing in registers by setting the test register high
;    	MOV		R3,	#0x6				;0x4000/0x100=0x40  for 16K
;looptCommCtl
;	  	LDR		R1, =0xABCDEF37
;		STR		R1, [R2]			;start with R0
;	 	LDR		R0, [R2]
;	 	CMP		R0, R1
;	    MOVNE	R0,	#0x0			;return false i.e. 0
;	    BNE		outtcc
;	 	ADD		R2,	R2,	#0x4        ;next register
;	    SUB		R3, R3, #0x1
;	    CMP		R3, #0x0
;	    BGT		looptCommCtl
;	    MOV		R0,	#0x1	
;	    	
;outtcc 	LDR		R2, =(COMMCTLR_BASE+COMMCTLR_R7)
;		MOV		R1, #0x0
 ;   	STR		R1, [R2]		;disabling test writing in ro registers by setting the test register high
 ;   	BX		LR							;LDMFD	sp!, {r4-r12, pc}


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;writePacketASM to be called from void writePacketWithoutPayload(unsigned long r0, unsigned long routingKey)
;parameters r0= Tx control, r1=routingKey
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
writePacketWOPL
 		LDR		R3,	  =COMMCTLR_BASE
 		STR		R0,  [R3, #COMMCTLR_R0]	    ;writing Tx control to comm ctlr R1
 		STR		R1,  [R3, #COMMCTLR_R3]	    ;writing routing key to comm ctlr  R3
		BX		LR					;return
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;writePacketASM to be called from void writePacketWithPayload(unsigned long r0, unsigned long routingKey, unsigned long payload)
;parameters r0= Tx control, r1=routingKey, r2 = payload
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
writePacketWPL
 		LDR		R3,	  =COMMCTLR_BASE
 		STR		R0,  [R3, #COMMCTLR_R0]	    ;writing Tx control to comm ctlr R1
 		STR		R2,  [R3, #COMMCTLR_R2]	    ;writing payload to comm ctlr R2
		STR		R1,  [R3, #COMMCTLR_R3]	    ;writing routing key to comm ctlr  R3
		BX		LR					;return
		

      
      	END                     

    	
