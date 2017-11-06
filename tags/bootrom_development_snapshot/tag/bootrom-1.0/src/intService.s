;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; systemLib.s file 
; 
; used in the device drivers' coding
; created by mmk
; modified by
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET ../inc/option.s       
       
		IMPORT  TLMMODEL         ;used for model specific compilation, TLM will be declared in globals.h if TLM has been
 								 ;defined or it will generate a linker error, comment it to support soc designer model
  		IMPORT	 procID							;from main
        IMPORT	 procType						;from main
        IMPORT   RxRoutingKey
        IMPORT   RxPayload
        IMPORT	 RxStatus
        IMPORT	 dmaInProcess
        IMPORT   dmaError
        IMPORT 	 dmaTimeout
        IMPORT   parityError
		IMPORT   frammingError
        IMPORT   txFramePending
        IMPORT   currentTime
        IMPORT   txEmpty
        IMPORT   txFull
        IMPORT   txOverrun
        IMPORT   handleReceivedPacket
        IMPORT   handleFrammingErrorInterrupt
        IMPORT   handleParityErrorInterrupt
        IMPORT   handlePhyInterrupt
        IMPORT   handleWatchdogInterrupt
        IMPORT   handleIncommingFrame
        IMPORT   handleTxFullInterrupt
        IMPORT   handleTxOverrunInterrupt
        ;IMPORT   handleWatchdogTimer
        IMPORT   handleRouterErrorInterrupt
        IMPORT   handleRouterDumpInterrupt
        IMPORT   handleRouterDiagInterrupt
        IMPORT   handleTimerInterrupt
        IMPORT   rtrDiagCount
        IMPORT   rtrParityError
 		IMPORT   rtrTimephaseError
 		IMPORT   rtrFrammingError
 		IMPORT   rtrUndefPacketError
 		IMPORT   rtrErrorCount
 		IMPORT   rtrErrorOverflow
 		IMPORT   rtrTxLinkStatus
 		IMPORT   rtrDumpOverflow
 ;       IMPORT	 clearCPUSleep
  		IMPORT   clearProcInterrupt
  		IMPORT   handleDMACompleted
  		IMPORT   clearDMAErrorInt
  		IMPORT   clearDMADoneInt
  		IMPORT   clearDMATimeoutInt
  		IMPORT   handleDMAErrorInt
  		IMPORT   handleDMATimeoutInt
  		IMPORT   receiveMessage
  		IMPORT   rxBuf
  		
		IMPORT	phy_read	; !! ST

        EXPORT   servTimer
        EXPORT   servDMAComplete
        EXPORT   servPktRcvd
   		EXPORT	 servDMATimeout
        EXPORT	 servDMAError
        EXPORT	 servParityError
        EXPORT	 servFrammingError
        EXPORT	 servTxFull
        EXPORT	 servTxOverrun
        EXPORT	 servTxEmpty
        EXPORT	 servSystemCtl
        EXPORT	 servWatchdog 
        EXPORT	 servEthernetTx
        EXPORT	 servEthernetRx
        EXPORT	 servEthernetPhy
        EXPORT	 servSoftware
        EXPORT	 servRouterErrorInt
        EXPORT   servRouterDumpInt
        EXPORT   servRouterDiagCntInt
               
        PRESERVE8
        
        AREA 	intService, CODE, READONLY
        ENTRY   

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DMA Completion Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servDMAComplete
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		LDR		R2,	  =DMAC_BASE
		;writeMemory(0x30000010, (1 << DMAC_CTRL_CLINT_DONE)); //writing at bit 3 of dma ctrl register to clear dma done interrupt
		;#else
		;writeMemory(0x30000008, 1);       //writing any value (1) to DMACIntTCClear register to clear dma done interrupt
	IF :DEF:TLMMODEL   ;*******************************TLM specific code here**********************
    	MOV		R1,  #(1 << DMAC_CTRL_CLINT_DONE)
    	STR		R1,  [R2, #TLM_DMAC_CTRL]
    ELSE     ;*****************************************SoC model code here************************************
  		MOV		R1,  #0x1
    	STR		R1,  [R2, #DMACIntTCClear]
  	ENDIF
	 	;clear DMA int bit	
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;BL		clearDMADoneInt					;clearing the interrupt basing on the type of model being used
   		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
   		;clearing the dmaInProcess flag while the interrupts are disabled so that some other interrupt should not mess with this in between
   		MOV		R0,   #0
		LDR		R1,   =dmaInProcess
		STR		R0,   [R1, #0]					;DMA completed flag
		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleDMACompleted
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DMA error Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servDMAError
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear DMA int bit	
		;BL		 clearCPUSleep              ;now done automatically by the processor with the help of STANDBYWFI signal
		;dmaErrorstatus=readMemory(0x30000014); //status register
		;intclear=(0x1 << DMAC_CTRL_RESTART) | (0x1 << DMAC_CTRL_CLINT_WB);
		;writeMemory(0x30000010, intclear); 
		LDR     R2,  =DMAC_BASE
	IF :DEF:TLMMODEL   ;*******************************TLM specific code here**********************
    	LDR		R1,	 =dmaError
    	LDR		R0,  [R2, #TLM_DMAC_STAT]
    	STR		R0,  [R1]					;update dmastatus
    	MOV		R1,  #(0x1 << DMAC_CTRL_RESTART) | (0x1 << DMAC_CTRL_CLINT_WB)
    	STR		R1,  [R2, #TLM_DMAC_CTRL]
    ELSE     ;*****************************************SoC model code here************************************
  		MOV		R1,  #0x1
    	STR		R1,  [R2, #DMACIntTCClear]
  	ENDIF
	   ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDR		R1,   =dmaError
		STR		R0,   [R1,#0x0]					;DMA completed flag
		
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DMA timeout Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servDMATimeout
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr		
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear DMA int bit	
		;BL		 clearCPUSleep              ;now done automatically by the processor with the help of STANDBYWFI signal
		;writeMemory(0x30000010, (1 << DMAC_CTRL_CLINT_TIMEOUT))
		LDR     R2,  =DMAC_BASE
	IF :DEF:TLMMODEL   ;*******************************TLM specific code here**********************
    	LDR		R1,	 =dmaError
    	LDR		R0,  [R2, #TLM_DMAC_STAT]
    	STR		R0,  [R1]					;update dmastatus
    	MOV		R1,  #(1 << DMAC_CTRL_CLINT_TIMEOUT)
    	STR		R1,  [R2, #TLM_DMAC_CTRL]
    ELSE     ;*****************************************SoC model code here************************************
  		MOV		R1,  #0x1
    	STR		R1,  [R2, #DMACIntTCClear]
  	ENDIF
       
        ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDR		R1,   =dmaTimeout
		STR		R0,   [R1, #0x0]					;DMA completed flag
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Timer Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
servTimer  
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear timer 1 interrupt
        LDR     R2,  =TIMER_BASE
        STR     R2,  [R2, #TIMER1IntClr]     
  		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDR		R1, =currentTime
		LDR		R2, [R1]					;increment currentTime
		ADD		R2, #0x1						
		STR		R2, [R1]
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleTimerInterrupt
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; packet received ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servPktRcvd
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr	
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		LDR      R0,  =COMMCTLR_BASE
        LDR      R1,  [R0, #COMMCTLR_R1]  		;read control byte of the rx packet
       	LDR      R3,  =RxStatus    
 		STR		 R1,  [R3, #0]					;RxStatus
        ;store r1 at some location
      	TST  	 R1,  #COMMCTLR_R1_DATA      	;data bit in the control byte of rx packet is set?
       	LDRNE	 R2,  [R0, #COMMCTLR_R4]		;if databit is 1 get the payload
      	LDR      R3,  =RxPayload    
 		STR		 R2,  [R3, #0]					;RxPayload
      	LDR		 R2,  [R0, #COMMCTLR_R5]        ;received routing key
        LDR      R3,  =RxRoutingKey    
 		STR		 R2,  [R3, #0]					;RxRoutingkey
     	;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		 handleReceivedPacket
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parity error Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servParityError
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear int by setting the parity bit to 0 in r1	
		LDR      R0,  =COMMCTLR_BASE
        LDR      R1,  [R0, #COMMCTLR_R1]  		;read r1 register
        BIC		 R1, #0x1 << 30                 ;clearing parity sticky bit, bit 30
        STR		 R1,  [r0, #COMMCTLR_R1]        ;load the new value after clearing parity error bit
        ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleParityErrorInterrupt
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; framming error Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servFrammingError
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear int by setting the framming error bit to 0 in r1	
		LDR      R0,  =COMMCTLR_BASE
        LDR      R1,  [R0, #COMMCTLR_R1]  		;read r1 register
		BIC	 	 R1,  #0x1 << 29   				;clear the bit 29 i.e. framming error sticky bit to clear the interrupt
        STR		 R1,  [R0, #COMMCTLR_R1]        ;load the new value after clearing framming error bit
  		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleFrammingErrorInterrupt
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
				
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; tx full Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servTxFull
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear int by setting the tx full bit to 0 in r0	
		LDR      R0,  =COMMCTLR_BASE
        LDR      R1,  [R0, #COMMCTLR_R0]  		;read r0 register
        BIC		 R1,  R1, #0x1 << 30     			;clear the bit 30 i.e. txfull sticky bit to clear the interrupt
        STR		 R1,  [R0, #COMMCTLR_R0]        ;load the new value after clearing txfull sticky bit
        ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 	;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleTxFullInterrupt		
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
				
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; tx overrun Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servTxOverrun
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear int by setting the tx overrun bit to 0 in r0	
		;BL		 clearCPUSleep              ;now done automatically by the processor with the help of STANDBYWFI signal
		LDR      R0,  =COMMCTLR_BASE
        LDR      R1,  [R0, #COMMCTLR_R0]  		;read r0 register
        BIC		 R1,  R1, #0x1 << 29     		;clear the bit 29 i.e. txOverrun sticky bit to clear the interrupt
        STR		 R1,  [R0, #COMMCTLR_R0]        ;load the new value after clearing txoverrun sticky bit
        ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleTxOverrunInterrupt
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; tx empty Int service
; the interrupt is not enabled normally, only done by the processor in case of 
; congestion to send only once comm. is free
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servTxEmpty
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;this interrupt can not be cleared, only way to clear it is to send a packet or from int clear register in the int ctlr	
        ;BL		 clearCPUSleep              ;now done automatically by the processor with the help of STANDBYWFI signal
        ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
;		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;		BL		handleTxEmptyInterrupt
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
				
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; router error int ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servRouterErrorInt
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr	
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		; reading the error status register
		LDR      R1,  =RTR_BASE
		LDR      R0,  [R1, #RTR_R5_ERROR_STATUS] ; reading the error flag register  
 		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 	;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleRouterErrorInterrupt	;calling function from main() with parameters in r0, r1, r2
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; router error int ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servRouterDumpInt
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		; reading the dump status register
		;BL		 clearCPUSleep              ;now done automatically by the processor with the help of STANDBYWFI signal
		LDR      R1,  =RTR_BASE
		LDR      R0,  [R1, #RTR_R10_DUMP_STATUS] ; reading the dump status register  
  		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 	;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleRouterDumpInterrupt				;calling the method with 4 parameters in r0, r1, r2, r3
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
				
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; router diagnostics counter int ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servRouterDiagCntInt
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr	
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		; reading the diag status register
		;BL		 clearCPUSleep              ;now done automatically by the processor with the help of STANDBYWFI signal
		LDR      R3,  =RTR_BASE
		LDR      R2,  [R3, #RTR_R1_STATUS]      ; first check which counter generated the int  
  		AND		 R2,  R2, #0xFF        			 ;reading the diag status
  		;now check each bit and then clear the interrupt by reading that diag count control register
  		TST		 R2, #0x1  << 0						;check if 0 at bit 0
  		BNE		 diag1
  		LDR		 R0,  [R3, #0x200]				;diag control 0  clear the diag control 0 interrupt
  		
diag1
		TST		 R2, #0x1 << 1						;check if 0 at bit 0
  		BNE		 diag2
  		LDR		 R0,  [R3, #0x204]				;diag control 0  clear the diag control 0 interrupt
  		
diag2
		TST		 R2, #0x1 << 2						;check if 0 at bit 0
  		BNE		 diag3
  		LDR		 R0,  [R3, #0x208]				;diag control 0  clear the diag control 0 interrupt
  		
diag3
		TST		 R2, #0x1 << 3						;check if 0 at bit 0
  		BNE		 diag4
  		LDR		 R0,  [R3, #0x20C]				;diag control 0  clear the diag control 0 interrupt
  		
diag4 
		TST		 R2, #0x1 << 4						;check if 0 at bit 0
  		BNE		 diag5
  		LDR		 R0,  [R3, #0x210]				;diag control 0  clear the diag control 0 interrupt
  		
diag5  
		TST		 R2, #0x1 << 5						;check if 0 at bit 0
  		BNE		 diag6
  		LDR		 R0,  [R3, #0x214]				;diag control 0  clear the diag control 0 interrupt
  		
diag6
		TST		 R2, #0x1 << 6						;check if 0 at bit 0
  		BNE		 diag7
  		LDR		 R0,  [R3, #0x218]				;diag control 0  clear the diag control 0 interrupt
  		
diag7 
		TST		 R2, #0x1 << 7						;check if 0 at bit 0
  		BNE		 diagout
  		LDR		 R0,  [R3, #0x21C]				;diag control 0  clear the diag control 0 interrupt
diagout 
		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 	;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleRouterDiagInterrupt				;from main to handle diag count
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; system ctl ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servSystemCtl  
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear system ctl interrupt
		;BL		 clearCPUSleep              ;now done automatically by the processor with the help of STANDBYWFI signal
		LDR		R0, =procID
		LDR		R0, [R0]
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		clearProcInterrupt
        ;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 	;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDR		R0, =rxBuf
		BL		receiveMessage
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
			
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Watchdog Timer Int service
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
servWatchdog  
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		;clear watchdog timer interrupt
		LDR     R3,  =WDOG_BASE
        LDR		R1,	=0x1ACCE551		  ;unlock value to enable writing the registers
    	STR		R1,	[R3, #WDOG_LOCK]
        STR     R1, [R3, #WDOG_INTCLR]  
        MOV	    R1,	#0x4   			  ;lock the registers
    	STR		R1,	[R3, #WDOG_LOCK] 
  		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleWatchdogInterrupt
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
				
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ethernet Tx int ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servEthernetTx
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		LDR	 	R3, =ETH_BASE   
		MOV		R2, #0x1 ;<< 0   			;clear bit at 0th location
		LDR		R1, =ETH_IRQ_CLEAR
		STR		R2, [R3, +R1]
        ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		MOV		R0, #0x0
		LDR		R1, =txFramePending
		STR		R0, [R1]					;TX frame has been sent, no pending 
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
				
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ethernet Rx int ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servEthernetRx
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr	
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		LDR	 	R3, =ETH_BASE   
		MOV		R2, #0x1 << 4               ;clear bit at 4th location
		LDR		R1, =ETH_IRQ_CLEAR
		STR		R2, [R3, +R1]
	    ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		BL		handleIncommingFrame
		;LDMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		B		restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ethernet Phy int ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; !! ST

servEthernetPhy
		SUB	lr, lr, #0x4			; subtracting 4 from lr
		STMFD 	sp!, {r0-r3, lr} 		; saving context
		MRS	r0, spsr			; copy spsr
		STMFD	sp!, {r0}			; save spsr

;!!		LDR	R0, =PHY_BASE	     		; clear the interrupt source
;!!		LDR	R0, [R0, #PHY_INT_STATUS]	; reading PHY_INT_STATUS to clear the interupt

		MOV	r0, #29	 			; PHY interrupt status register
		BL	phy_read			; Interrupt status to r0

		MSR	CPSR_c, #INTON|Mode_SYS 	; renabling interrupts, changing to system mode

; first probe the interrupt i.e. if it is connection interrupt, 
; assuming it is the link up interrupt
; enable the ehternet if not done so far

		BL	handlePhyInterrupt		; calling the method to handle the phy interrupt
;
		B	restoreSPSRandIRQ		
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; software int ISR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
servSoftware
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,		SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr	
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		LDR		R3, =VIC_BASE
		LDR		R1, =0xFFFFFFFF
		STR		R1, [R3, #VICSOFTINTCLEAR]  ;writing 0xFFFFFFFF to clear all the software interrupts, can be improved by the application later
	    ;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;
		;
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
restoreSPSRandIRQ
		MSR		CPSR_c, #I_Bit|Mode_IRQ		;disabling the IRQ
		LDMFD	SP!,  {R0}					;restore spsr_irq
		MSR		SPSR_cxsf, R0				;restore spsr
		;;;;;;;;;;;;;;;;;;;;;;;;; Acknowledge VIRQ serviced;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDR		R0, 	=VIC_BASE
		STR 	R0, 	[R0, #VICVECTADDR] 
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDMFD 	SP!, 	{R0-R3, PC}^ 		; unstack registers and return
		
		
		

		END
		
		
