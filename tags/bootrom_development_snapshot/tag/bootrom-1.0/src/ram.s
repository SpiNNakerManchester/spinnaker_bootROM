;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ram.s file 
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
 		
 		EXPORT	testRAM
 		
 		AREA 	systemram, CODE, READONLY
        ENTRY  


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SYSTEM RAM TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testRAM
								;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2, =SYSTEM_RAM_BASE
    	MOV		R3,	#0x40				;0x4000/0x100=0x40  for 16K
looptram
	  	LDR		R1, =0xABCDEF37
		STR		R1, [R2]
	 	LDR		R0, [R2]
	 	CMP		R0, R1
	    MOVNE	R0,	#0x0			;return false i.e. 0
	    BNE		outram
	 	ADD		R2,	R2,	#0x100
	    SUB		R3, R3, #0x1
	    CMP		R3, #0x0
	    BGT		looptram
	    MOV		R0,	#0x1			;test pass, return true i.e. 1
	    ;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, R1, #(0x1 << 13)	;saving status of ram at bit 13
    	;STR		R1, [R2]
	    ;ORR		R11, R11, #(0x1 << 13)	;saving status of ram at bit 13 
outram
		MOV		PC,	LR 					;LDMFD   		sp!, 	{R1-R1, pc}

		END                     
    	
    	