;;; Copyright ARM Ltd 2002. All rights reserved.
   		GET peripherals.s    
       
		IMPORT  Reset_HandlerHi ;Reset_Handler           ; In start.s
		IMPORT  undefHandler
		IMPORT	swiHandler
		IMPORT	prefetchHandler
		IMPORT	abortHandler
		IMPORT	resHandler
		IMPORT	poolVicStatus
		IMPORT  RxPayload
		IMPORT  RxRoutingKey
		IMPORT  RxStatus

		;EXPORT  HiVectors
		EXPORT  LowVectors
		EXPORT  VectorEndAddress

; !! ST removed unecessary exports below

;!!		EXPORT 	Undefined_Handler
;!!        	EXPORT  SWI_Handler
;!!        	EXPORT  Prefetch_Handler
;!!        	EXPORT  Abort_Handler
;!!        	EXPORT  Reserved_Handler                             	;Reserved vector

        EXPORT  FSRoutine	
        EXPORT  Pool_Vic_Status
        
        PRESERVE8
        AREA Vect, CODE, READONLY
; *****************
; Exception Vectors
; *****************

; Note: LDR PC instructions are used here, though branch (B) instructions
; could also be used, unless the ROM is at an address >32MB.

        ENTRY
LowVectors
        LDR     PC, =Reset_HandlerHi    ;Reset_Handler
        LDR     PC, =Undefined_Handler
; !!!        LDR     PC, =SWI_Handler
        LDR     PC, =0xF500007C
        LDR     PC, =Prefetch_Handler
        LDR     PC, =Abort_Handler
        LDR     PC, =Reserved_Handler                             	;Reserved vector
	    LDR	    PC, [PC, #-VIC_VECTADDR_OFFSET]						;VIC IRQ
        LDR     PC, =FSRoutine										;FIQ_Handler
FSRoutine
		;in FSR we don't have to save any context and use the dedicated register bank from R8-R12 for interrupt handing
		;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		;STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		;MRS		R0,		SPSR				;copy spsr
		;STMFD	SP!,	{R0}				;save spsr	
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		LDR      R8,  =COMMCTLR_BASE
        LDR      R9,  [R8, #COMMCTLR_R1]  		;read control byte of the rx packet
       	LDR      R11,  =RxStatus    
 		STR		 R9,  [R11, #0]					;RxStatus
        ;store r1 at some location
      	TST  	 R9,  #COMMCTLR_R1_DATA      	;data bit in the control byte of rx packet is set?
       	LDRNE	 R10,  [R8, #COMMCTLR_R4]		;if databit is 1 get the payload
      	LDR      R11,  =RxPayload    
 		STR		 R10,  [R11, #0]					;RxPayload
      	LDR		 R10,  [R8, #COMMCTLR_R5]        ;received routing key
        LDR      R11,  =RxRoutingKey    
 		STR		 R10,  [R11, #0]					;RxRoutingkey
     	;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		;MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;BL		 handleReceivedPacket
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;MSR		CPSR_c, #I_Bit|Mode_IRQ		;disabling the IRQ
		;LDMFD	SP!,  {R0}					;restore spsr_irq
		;MSR		SPSR_cxsf, R0				;restore spsr
		;;;;;;;;;;;;;;;;;;;;;;;;; Acknowledge VIRQ serviced;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;LDR		R0, 	=VIC_BASE
		;STR 	R0, 	[R0, #VICVECTADDR] 
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDMFD 	SP!, 	{R0-R3, PC}^ 		; unstack registers and return
		
		
; ************************
; Exception Handlers
; ************************

; The following handlers do not do anything useful 
; except to put the processor to sleep in order to isolate the problem 
; we avoid the error from propagating
Undefined_Handler     ;goes to undef mode and uses undef handler
		;subtraction not required as in undef the lr offset =0
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,	SPSR				;copy spsr

; !! ST changed next
;!!		STMFD	SP!,	{R10}				;save spsr
		STMFD	SP!,	{R0}				;save spsr

		;LDR		R0, =undefHandlemsg
		BL		undefHandler
		B		restore_spsr_irq
		
		
SWI_Handler			;goes to svc mode and uses svc stack
		;subtraction not required as in SWI the lr offset =0
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,	SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;LDR		R0, =swimsg
		BL		swiHandler					;C++ CODE  printStr
		B		restore_spsr_irq
		
Prefetch_Handler         ; goes to abt mode and uses abt stack
		;;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;subtract 4 in prefetch handler
		;SUB		LR, LR, #0x4                  ; not subtracting to execute the next interuction, to avoid going to the exception again
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,	SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;LDR		R0, =prefetchImsg
		BL		prefetchHandler				;C++ CODE  printStr
		B		restore_spsr_irq
		
Abort_Handler    ;goes to abt mode and uses abt stack
		;subtract 8 in abort handler
		SUB		LR, LR, #0x4					; to move to the next instruction, not to the one caused an error, subtracting 4 instread of 8
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,	SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		;LDR		R0, =abortmsg
		BL		abortHandler				;C++ CODE  printStr
		B		restore_spsr_irq
		
Reserved_Handler     ; undef handler goes to undef mode and uses undef stack
		STMFD 	SP!, 	{R0-R3, LR} 		;saving context
		MRS		R0,	SPSR				;copy spsr
		STMFD	SP!,	{R0}				;save spsr
		LDR		R0, =abortmsg

; !! ST - changed next line
;!!		BL		Reserved_Handler			;C++ CODE  printStr
		BL		resHandler			;C++ CODE  printStr

restore_spsr_irq
		LDMFD	SP!,  {R0}					;restore spsr_irq
		MSR		SPSR_cxsf, R0				;restore spsr
		LDMFD 	SP!, 	{R0-R3, PC}^ 	; unstack registers and return
		
;it is done in case of the non vectored interrupts, this address is stored in the default vector address register and is returned on a non vectored interrupt, 
;here we pool the status register and then handle the interrupts    
Pool_Vic_Status    
		;;;;;;;;;;;;;;;;;;;;;;;;;saving context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		SUB		LR, 	LR, #0x4			;subtracting 4 from lr
		STMFD 	SP!, 	{R0-R3, R12, LR} 		;saving context
		MRS		R12,	SPSR				;copy spsr
		STMFD	SP!,	{R12}				;save spsr
		;;;;;;;;;;;;;;;;;;;;;;;;;clear the interrupt source;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
		
  		;;;;;;;;;;;;;;;;;;;;;;;;;enabling interrupts;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		MSR		CPSR_c, #INTON|Mode_SYS 		;enabling interrupts, changing to system mode
		;;;;;;;;;;;;;;;;;;;;;;;;;interrupt service code;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		BL		poolVicStatus					;entry point for more suffisticated routine in C++
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring spsr_irq;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		MSR		CPSR_c, #I_Bit|Mode_IRQ		;disabling the IRQ
		LDMFD	SP!,  {R12}					;restore spsr_irq
		MSR		SPSR_cxsf, R12				;restore spsr
		
		;;;;;;;;;;;;;;;;;;;;;;;;;restoring context;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		LDMFD 	SP!, 	{R0-R3, R12, PC}^ 		; unstack registers and return
		
VectorEndAddress
		MOV		R0, #0x0      
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; print string
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
printStr
		LDR		R3,	=SYSCTLR_BASE_U
		MOV 	R1, R0
PRST	LDRB	R0, [R1], #1
		CMP		R0, #0
		STRNE	R0, [r3, #SYSCTLR_TUBE]
		BNE		PRST
		BX		LR  						
		
		
undefHandlemsg			DCB  "got udef handler int", 10, 0
swimsg					DCB  "got sw int", 10, 0
prefetchImsg			DCB  "got frefetch int", 10, 0
abortmsg				DCB  "got abort int", 10, 0
irgmsg 					DCB  "got irq int", 10, 0 
resHandlemsg     		DCB  "got res handler int", 10, 0
fsimsg					DCB  "got fiq int", 10, 0 
loadingITCMmsg			DCB  "going to load ITCM now  ", 10, 0 

		  

		  

		
		
		END
