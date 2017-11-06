;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; dmac.s file 
; 
; used in the device drivers' coding
; created by mukaram khan 
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
       GET peripherals.s       
 		
 			
 		IMPORT  	TLMMODEL     ;used for model specific compilation, TLM will be declared in globals.h if TLM has been
 								 ;defined or it will generate a linker error, comment it to support soc designer model
  		IMPORT  dmaInProcess
  		
  		EXPORT  startDMARead
  		EXPORT  startDMAWrite
  		EXPORT	initDMAC
 		EXPORT  testDMAC
 	 	
 		AREA 	dmac, CODE, READONLY
        ENTRY  

   

   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DMAC INIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
initDMAC
	IF :DEF:TLMMODEL   ;*******************************TLM specific code here**********************
    	LDR		R2,	  =DMAC_BASE
    	LDR		R1,    =(0x1 << DMAC_GCTL_ENINT_DONE):OR:(0x1 << DMAC_GCTL_ENINT_DONE2):OR:\
						(0x1 << DMAC_GCTL_ENINT_TOUT):OR:\
						(0x1 << DMAC_GCTL_ENINT_CRCERROR):OR:\
						(0x1 << DMAC_GCTL_ENINT_TCMERROR):OR:\
						(0x1 << DMAC_GCTL_ENINT_AXIERROR):OR:\
						(0x1 << DMAC_GCTL_ENINT_USERABORT):OR:\
						(0x1 << DMAC_GCTL_ENINT_SOFTRST):OR:\
						(0x1 << DMAC_GCTL_ENINT_WB)         ;:OR:
						;(0x1 << DMAC_GCTL_EN_BR_BUF)  ;not enabling the write buffer in dma in the start
    	STR		R1,  [R2, #TLM_DMAC_GCTL]    ;enabling inerrupts for the sources listed above
    	BX		LR
    ELSE     ;*****************************************SoC model code here************************************
    	LDR		R2,	  =DMAC_BASE
    	BX		LR
    ENDIF
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DMAC TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testDMAC
	IF :DEF:TLMMODEL     ;*******************************TLM specific code here**********************
		LDR		R2,	    =DMAC_BASE	
	    LDR		R1,		=0xFF0
	    STR		R1,		[R2, #TLM_DMAC_ADRS]
	    STR		R1,		[R2, #TLM_DMAC_ADRT]
	    LDR		R0, 	[R2, #TLM_DMAC_ADRS]
	   	CMP		R1, 	R0
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtdmacTLM	
	    LDR		R0, 	[R2, #TLM_DMAC_ADRT]
	   	CMP		R1, 	R0
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtdmacTLM	
    	MOV		R0,		#0x1					;return 1 i.e. true
    	;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, #(0x1 << 2)					;status of comm ctl at bit 0
    	;STR		R1, [R2]
    	;ORR		R11, R11, #(0x1 << 2)			;status of dma ctl at bit 2
outtdmacTLM    							
		BX		LR					
	   	
   ELSE    ;*****************************************SoC model code here************************************
   		LDR		R2,	    =DMAC_BASE	
	    MOV		R3,		#0x1
	    STR		R3,		[R2, #DMACC0SrcAddr]
	    STR		R3,		[R2, #DMACC0DestAddr]
	    LDR		R1, 	[R2, #DMACC0SrcAddr]
    	CMP		R1, 	R3
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtdmac	
	    LDR		R1, 	[R2, #DMACC0DestAddr]
    	CMP		R1, 	R3
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtdmac	
    	MOV		R0,		#0x1					;return 1 i.e. true
    	;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, R1, #(0x1 << 2)					;status of comm ctl at bit 2
    	;STR		R1, [R2]
    	;ORR		R11, R11, #(0x1 << 2)			;status of dma ctl at bit 2
outtdmac    							;LDMFD   SP!, {R0, R1, PC}
		BX		LR

	ENDIF
;****VERY IMPORTANT***********
;SET THE FLAG FOR DMA COMPLETED BEFORE STARTING DMA OPERATION COMMAND, AS THE DMA COMPLETE INTERRUPT MAY ARRIVE
;EARLIER THAN THE SETTING OF FLAG AND CAUSE AN UNDESIREABLE OPERATION                      
;;;;;;;;;;;;;;;;;;;;DMA Read Operation;;;;;;;;;;;;;;;;;;;; 
;;parameters r0 = dmaFrom, r1 = dmaTo, r2= noofBytes
;;parameters r0 = sysNoC address, r1 = TCM address, r2= noofBytes, r3=trf id 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
startDMARead
	IF :DEF:TLMMODEL     ;*******************************TLM specific code here**********************
		;;;;;;;;;;;;;;;;;;;;DMA Read Operation;;;;;;;;;;;;;;;;;;;; 
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;startTLM_DMARead
		STMFD	SP!, {R4-R6, LR}
;		MOV		R5,   #1
;		LDR		R6,   =dmaInProcess
;		STR		R5,   [R6, #0]					;DMA completed flag=false
        LDR  	R4,  =DMAC_BASE		             ;dma register start addr
        STR	 	R0,  [R4, #TLM_DMAC_ADRS]		;dma src addr
        STR	 	R1,  [R4, #TLM_DMAC_ADRT]		;dma dest addr
        LDR		R5,  =0x1FFFF 					;17 bits for len of transfer
        AND     R2,  R2, R5						;to ensure the length is not more than 17 bits
      	LSL		R3,  R3, #24					;transfer id in bit 21-24
      	ORR		R2,  R2, R3
      	STR	 	R2,  [R4, #TLM_DMAC_DESC]		;dma channel 0 control
        LDMFD	SP!, {R4-R6, PC}       
	ELSE    ;*****************************************SoC model code here************************************
   		STMFD	SP!, {R3-R6, LR}
;        MOV		R5,   #1
;		LDR		R6,   =dmaInProcess
;		STR		R5,   [R6, #0]					;DMA completed flag=false
        LDR  	R5,  =DMAC_BASE                 ;dma register start addr
        MOV  	R6,  #0x1
        STR	 	R6,  [R5, #DMACConfiguration]	;enable dma
        STR	 	R0,  [R5, #DMACC0SrcAddr]			;dma src addr
        STR	 	R1,  [R5, #DMACC0DestAddr]		;dma dest addr
       	LDR		R3,  =DMA_NOOFBYTES_MASK
        AND		R2,  R2, R3
        LDR		R3,	 =DMA_READ_CONTROL 			;control with the noofBytes in its first 12 bits
		ORR		R3,  R3, R2
		STR	 	R3,  [R5, #DMACC0Control]		;dma channel 0 control
        LDR		R3,  =DMA_OPT_CONFIG
        STR	 	R3,  [R5, #DMACC0Configuration]		;dma channel 0 control
        LDMFD	SP!, {R3-R6, PC}        
   ENDIF
 
;;;;;;;;;;;;;;;;;;;;DMA Write Operation;;;;;;;;;;;;;;;;;;;; 
;;parameters r0 = dmaFrom, r1 = dmaTo, r2= noofBytes
;;parameters r0 = sysNoC address, r1 = TCM address, r2= noofBytes, r3=trf id
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;   
startDMAWrite
	IF :DEF:TLMMODEL     ;*******************************TLM specific code here**********************
		;startTLM_DMAWrite
		STMFD	SP!, {R4-R6, LR}
		MOV		R5,   #1
		LDR		R6,   =dmaInProcess
		STR		R5,   [R6, #0]					;DMA completed flag=false
        LDR  	R5,  =DMAC_BASE             	;dma register start addr
        STR	 	R0,  [R5, #TLM_DMAC_ADRS]		;dma src addr
        STR	 	R1,  [R5, #TLM_DMAC_ADRT]		;dma dest addr
        LDR		R4,  =0x1FFFF 					;17 bits for len of transfer
        AND     R2,  R2, R4						;to ensure the length is not more than 17 bits
      	MOV		R4,  #1 
      	LSL		R4, R4, #19
      	ORR		R2, R2,	R4			;bit 19 to be 1 for write operation
      	LSL		R3,  R3, #24					;transfer id in bit 21-24
      	ORR		R2, R2, R3
      	STR	 	R2,  [R5, #TLM_DMAC_DESC]		;dma channel 0 control
      	LDMFD	SP!, {R4-R6, PC}        
 
	ELSE    ;*****************************************SoC model code here************************************
   		STMFD	SP!, {R3-R6, LR}
		MOV		R5,   #1
		LDR		R6,   =dmaInProcess
		STR		R5,   [R6, #0]					;DMA completed flag=false
        LDR  	r5,  =DMAC_BASE                 ;dma register start addr
        MOV  	r6,  #0x1
        STR	 	r6,  [r5, #DMACConfiguration]	;enable dma
        STR	 	r0,  [r5, #DMACC0SrcAddr]			;dma src addr
        STR	 	r1,  [r5, #DMACC0DestAddr]		;dma dest addr
		LDR		r3, =DMA_NOOFBYTES_MASK
        AND		r2,  r2, r3
        LDR		r3,	 =DMA_WRITE_CONTROL 			;control with the noofBytes in its first 12 bits
		ORR		r3,  r3, r2						;control with the noofBytes in its first 12 bits
		STR	 	r3,  [r5, #DMACC0Control]		;dma channel 0 control
        LDR		r3,  =DMA_OPT_CONFIG
        STR	 	r3,  [r5, #DMACC0Configuration]		;dma channel 0 control
        LDMFD	sp!, {r3-r6, pc}    
    ENDIF
;;;;;;;;;;;;;;;;;;;;TLM_DMA Write Operation;;;;;;;;;;;;;;;;;;;; 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;   

		
	
		END                     

		
