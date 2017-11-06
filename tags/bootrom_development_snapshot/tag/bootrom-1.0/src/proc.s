;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; proc.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET ../inc/option.s     
 	
 		IMPORT  	TLMMODEL     ;used for model specific compilation, TLM will be declared in globals.h if TLM has been
 								 ;defined or it will generate a linker error, comment it to support soc designer model
  		IMPORT	procID
       	IMPORT  procType
       	IMPORT  chipID
        ;IMPORT  procChipStatus
              	
     	EXPORT	initProcID
     	EXPORT  initChipID
 		EXPORT 	arbitForMonProc
 		EXPORT  gotoSleep
 		EXPORT  changeToLowVectors
 		;EXPORT  enableIRQ
 		;EXPORT  enableFIQ
 		EXPORT  enableInterrupts
 		;EXPORT  disableIRQ
 		;EXPORT  disableFIQ
 		EXPORT  disableInterrupts
 		EXPORT 	readMemory
 		EXPORT  writeMemory
 		
 		AREA 	start, CODE, READONLY
 		
        ENTRY  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; procID INIT, used to initialise the proc id in each processor 
; a register in each DMAC is kept to hold a unique proc ID for 
; the core. the processor is going to read this id to later use it in 
; configuration process.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
initProcID
	IF :DEF:TLMMODEL     ;*******************************TLM specific code here**********************
		LDR		R2,	  =DMAC_BASE	
    	LDR		R3,   =procID
    	LDR		R1,   [R2, #TLM_DMAC_STAT]	   ;reading the value of procID from procID register
    	LSR     R1,   R1,  #24                 ; Get CPUID to 7:0
    	MOV		R0,   R1
    	STR		R1,   [R3, #0]				   ;writing to the variable procID	
		MOV		PC,    LR	
	ELSE                 ;*********************************SoC model code here***********************
 		LDR		R2,	  =PROCID_REGISTER	
    	LDR		R3,   =procID
	    LDR		R1,   [R2, #0]					;reading the value of procID from procID register
    	STR		R1,   [R3, #0]					;writing to the variable procID	
	    MOV		PC,    LR	
 
    ENDIF
 


	    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; chipID INIT, used to initialise the chip id in the absence of the bootup code
; a temporary register in each chip is kept to hold a unique chip ID for 
; the chip. the processor is going to read this id to later use it in 
; configuration process.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
initChipID
		LDR		R0,	  =CHIPID_REGISTER	
    	LDR		R2,   =chipID
    	LDR		R1,   [R0, #0x0]				;reading the value of chipID from chipID register
    	STR		R1,   [R2, #0x0]					;writing to the variable chipID	
	    MOV		PC,   LR  			;LDMFD	SP!, {R4-R7, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; go to sleep, to put the processor to sleep mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
gotoSleep
		;set the bit in the system ctlr before sleeping
		STMFD	SP!,  {R0-R12, LR}
		MCR 	p15, 0, R0, c7, c0, 4
		LDMFD   SP!, {R0-R12, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arbitForMonProc to be used in setup monitor processor
; input mon proc id in R0, return procType in R0, 1 if mon proc or 0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
arbitForMonProc
		LDR	     R2,  =SYSCTLR_BASE_U	
		LSL		 R0,  R0, #0x2       ;multiply 4
		ADD		 R0,  R0, #0x80
		LDR		 R1,  [R2, R0]		; reading at 0x80 + 4*N where N=procID
		LDR		 R0,  =0x80000000		;value if selected as mon proc
		LDR		 R3,  =procType
		CMP		 R0,  R1			;see if read 1 i.e. it is the mon proc
		MOVEQ	 R0,  #0x1		;MON PROC
		MOVNE	 R0,  #0x0		;forcing to be fascicle for testing
		STR		 R0,  [R3]
		MOV		 PC,  LR				;LDMFD	 		SP!, 	{R0-R3, PC}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; enable IRQ
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;enableIRQ
;		MRS		R0,		CPSR
;		BIC		R0,		R0, #0x80					;bit 7 to 0
;		MSR		CPSR_c,	R0
;		MOV		PC,     LR		;LDMFD	SP!, 	{R0, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; enable FIQ
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;enableFIQ
;		MRS		R0, 	CPSR
;		BIC		R0, 	R0, #0x40					;bit 6 to 0
;		MSR		CPSR_c, R0
;		MOV		PC, 	LR		;LDMFD	 		SP!, 	{R0, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; enable interrupts
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enableInterrupts
		MRS		R0, 	CPSR
		BIC		R0,		R0,		#0xC0				;bit 7 and 6 to 0      
		MSR		CPSR_c, R0
		MOV		PC, 	LR		;LDMFD	 		SP!, 	{R0, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; disable IRQ
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;disableIRQ
;		MRS		R0,		CPSR
;		ORR		R0, 	R0, 	#0x80				;bit 7 to 1
;		MSR		CPSR_c, R0
;		MOV		PC,		LR		;LDMFD	 		SP!, 	{R0, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; disable FIQ
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;disableFIQ
;		MRS		R0,		CPSR
;		ORR		R0, 	R0, 	#0x40				;bit 6 to 1
;		MSR		CPSR_c, R0
;		MOV		PC,		LR		;LDMFD	 		SP!, 	{R0, PC} 		 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; disable interrupts
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
disableInterrupts
								;STMFD	 		SP!, 	{R0, LR}
		MRS		R0,		CPSR
		ORR		R0, 	R0, 	#0xC0				;bit 7 and 6 to 1
		MSR		CPSR_c, R0
		MOV		PC, 	LR		;LDMFD	 		SP!, 	{R0, PC} 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; disable high vector, change to low vectors
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
changeToLowVectors
		MRC 			p15, 0, R0, c1, c0, 0       ; read CP15 register 0 into R0
        BIC     		R0, R0, #(0x1  <<13)        ; disable high vectors
        MCR     		p15, 0, R0, c1, c0, 0       ; write CP15 register 0
		MOV				PC, LR	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; readMemory, read to R0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
readMemory
		LDR		R0, [R0, #0x0]
		MOV		PC, LR
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; writeMemory, addr in R0, value in R1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
writeMemory
		STR		R1, [R0, #0x0]
		MOV		PC, LR              

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; readMemByte, read to R0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
readMemByte
		LDRB	R0, [R0, #0x0]
		MOV		PC, LR
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; writeMemByte, addr in R0, value in R1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
writeMemByte
		STRB	R1, [R0, #0x0]
		MOV		PC, LR              
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; readMemHWord, read to R0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
readMemHWord
		LDRH	R0, [R0, #0x0]
		MOV		PC, LR
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; writeMemHWord, addr in R0, value in R1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
writeMemHWord
		STRH	R1, [R0, #0x0]
		MOV		PC, LR               
 
    	END  
	
		