;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; pl340.s file 
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
 		
 		EXPORT	initPL340
 		EXPORT  testPL340
 		EXPORT  testSDRAM
 		AREA 	pl340, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
;initialize PL340
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
initPL340
												;STMFD	SP!, {R0, R1, LR}											;STMFD	SP!, {R0, R1, R2, LR}
		LDR		R0,	=PL340CFG_BASE
													;setting refresh period
		LDR		R1, =0x00000410						
		STR		R1, [R0, #0x010]					;BURST1_WRITE(0x80000010, 4, 0x00000410);	// cas_latency=3
		MOV		R1, #0x00000006						
		STR		R1, [R0, #0x014]					;BURST1_WRITE(0x80000014, 4, 0x00000006);	// cas_latency=3
		MOV		R1, #0x00000001						
		STR		R1, [R0, #0x018]					;BURST1_WRITE(0x80000018, 4, 0x00000001);
		MOV		R1, #0x00000006						
		STR		R1, [R0, #0x01c]					;BURST1_WRITE(0x8000001c, 4, 0x00000002);
		MOV		R1, #0x00000006						
		STR		R1, [R0, #0x020]					;BURST1_WRITE(0x80000020, 4, 0x00000006);
		MOV		R1, #0x00000009						
		STR		R1, [R0, #0x024]					;BURST1_WRITE(0x80000024, 4, 0x00000009);
		MOV		R1, #0x00000003						
		STR		R1, [R0, #0x028]					;BURST1_WRITE(0x80000028, 4, 0x00000003);
		LDR		R1, =0x0000010b					
		STR		R1, [R0, #0x02c]					;BURST1_WRITE(0x8000002c, 4, 0x0000010b);
		MOV		R1, #0x00000003						
		STR		R1, [R0, #0x030]					;BURST1_WRITE(0x80000030, 4, 0x00000003);
		MOV		R1, #0x00000002						
		STR		R1, [R0, #0x034]					;BURST1_WRITE(0x80000034, 4, 0x00000002);
		MOV		R1, #0x00000003						
		STR		R1, [R0, #0x038]					;BURST1_WRITE(0x80000038, 4, 0x00000003);
		MOV		R1, #0x00000001						
		STR		R1, [R0, #0x03c]					;BURST1_WRITE(0x8000003c, 4, 0x00000001);
		MOV		R1, #0x00000001						
		STR		R1, [R0, #0x040]					;BURST1_WRITE(0x80000040, 4, 0x00000001);
		MOV		R1, #0x00000010						
		STR		R1, [R0, #0x044]					;BURST1_WRITE(0x80000044, 4, 0x00000010);
		MOV		R1, #0x00000014						
		STR		R1, [R0, #0x048]					;BURST1_WRITE(0x80000048, 4, 0x00000014);
		LDR		R1, =0x00210012						
		STR		R1, [R0, #0x00c]					;BURST1_WRITE(0x8000000c, 4, 0x00210012);	// memory_cfg
		LDR		R1, =0x00000a60						
		STR		R1, [R0, #0x010]					;//BURST1_WRITE(0x80000010, 4, 0x00000a60);
		;LDR		R1, =0x000050ff						
		LDR		R1, =0x000060e0	
		STR		R1, [R0, #0x200]					;BURST1_WRITE(0x80000200, 4, 0x000000ff);
		LDR		R1, =0x000c0000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x000c0000);
		MOV		R1, #0x00000000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00000000);
		LDR		R1, =0x00040000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00040000);
		LDR		R1, =0x00040000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00040000);
		LDR		R1, =0x00080032						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00080032);
		LDR		R1, =0x001c0000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x001c0000);
		LDR		R1, =0x00100000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00100000);
		LDR		R1, =0x00140000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00140000);
		LDR		R1, =0x00140000						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00140000);
		LDR		R1, =0x00180032						
		STR		R1, [R0, #0x008]					;BURST1_WRITE(0x80000008, 4, 0x00180032);
		MOV		R1, #0x0						
		STR		R1, [R0, #0x004]					;BURST1_WRITE(0x80000004, 4, 0x0);
	    MOV		PC, LR          ;LDMFD	SP!, {R0, R1, PC}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PL340 TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testPL340
							;STMFD	sp!,  {R0, R1, lr}
    	LDR		R2,	=PL340CFG_BASE
    	LDR		R3, [R2, #PL340CFG_REFRESH_PRD]  ;copy current value
    	MOV		R1,	#0xAB
    	STR		R1,	[R2, #PL340CFG_REFRESH_PRD]
    	LDR		R0, [R2, #PL340CFG_REFRESH_PRD]
    	CMP		R1, R0
    	MOVNE	R0, #0x0					;return 0 i.e. fail
    	BNE		outtpl
    	MOV		R0,	#0x1
    	;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, R1, #(0x1 << 10)		;restoring the status of pl340 at bit 10
    	;STR		R1, [R2]
    	;ORR		R11, R11, #(0x1 << 10)		;restoring the status of pl340 at bit 10
outtpl 	
		STR		R3,	[R2, #PL340CFG_REFRESH_PRD]		;restore old value
		MOV		PC,	LR		;LDMFD	sp!,  {r0, r1, pc} 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  SDRAM TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testSDRAM
								;STMFD	sp!,  {r0, r1, lr}
    	LDR		R2, =SDRAM_BASE
    	MOV		R3,	#0x200
looptsdram
	  	LDR		R1, =0xABCDEF37
		STR		R1, [R2]
	 	LDR		R0, [R2]
	 	ADD		R2,	R2,	#0x100
	    CMP		R1, R0
	    MOVNE	R0,	#0x0			;return false i.e. 0
	    BNE		outsdram
	    SUB		R3, R3, #0x1
	    CMP		R3, #0x0
	    BGT		looptsdram
	    MOV		R0,	#0x1			;test pass, return true i.e. 1
	    ;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, #(0x1 << 11) 	;saving status of sdram at bit 11 
    	;STR		R1, [R2]
	    ;ORR		R11, R11, #(0x1 << 11) ;saving status of sdram at bit 11
outsdram
		MOV		PC,	LR						;LDMFD   		sp!, 	{R1-R1, pc}


		END                     
    	
    	
	