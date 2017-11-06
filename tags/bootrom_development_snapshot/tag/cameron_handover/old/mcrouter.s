;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; mcrouter.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET option.s    
         
 		;IMPORT  procChipStatus
 		IMPORT  procID
 		
 		EXPORT	initRouter
 		EXPORT  testRouter
 		EXPORT  setupMCTable
 		EXPORT  setupP2PTable
 		AREA 	router, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ROUTER INIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
initRouter
						;STMFD	SP!, {R0-R3, LR}
    	LDR		R2,	=RTR_BASE
    	LDR		R1, =procID
    	LDR		R1, [R1]						;setting mon proc. id in the mon. proc. id register
    	LSL		R1, #0x8						;moving the mon proc. id to bits 8-12
    	MOV		R3, #0x1F						;ensuring it to be 5 bit wide	
    	LSL		R3, #8
    	AND		R1, R1, R3						;to ensure the id is within the bits assigned
    	LDR		R3,	=0x0F0F0007					;setting wait=0F, wait2=0F, dump enable=1, error enabled=1, router enabled=1
    	ORR		R1, R1, R3
    	STR		R1,	[R2, #RTR_R0_CONTROL]
    	LDR		R1, =0x07000000					;default route 7 i.e. internal processor
    	STR		R1, [R2, #RTR_R2_ERROR_HEADER]  ;disabling the diagnostics counters
    	LDR		R1, =0x07000000					;default route 7 i.e. internal processor
    	STR		R1, [R2, #RTR_R6_DUMP_HEADER]   ;disabling the diagnostics counters
    	MOV		R1, #0
    	STR		R1, [R2, #RTR_R11_DIAG_ENABLES] ;disabling the diagnostics counters
    	BX		LR
    	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TIMER TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testRouter
						;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2, =RTR_BASE
    	LDR		R1,	=0xABCDEF37
    	STR		R1,	[R2, #RTR_R3N_DIAG_COUNT]
    	LDR		R3, [R2, #RTR_R3N_DIAG_COUNT]
    	CMP		R1, R3
    	MOVNE	R0, #0x0					;return 0 i.e. fail
    	BNE		outtr
    	MOV		R0,	#0x1
    	;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, #(0x1 << 8)					;saving status of router at bit 8 
    	;STR		R1, [R2]
    	;ORR	 	R11, R11, #(0x1 << 8) 		;saving status of router at bit 8 
outtr 
		MOV		R1,	#0x0					;restoring to value 0
    	STR		R1,	[R2, #RTR_R3N_DIAG_COUNT]
    	BX		LR		;LDMFD	sp!,  {r0, r1, pc} 

		END                     
    	
