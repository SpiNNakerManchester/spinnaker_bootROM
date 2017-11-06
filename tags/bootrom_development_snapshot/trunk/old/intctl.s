;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; intctl.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET option.s     
 		IMPORT   procType
 		IMPORT   servTimer
        IMPORT   servDMAComplete
        IMPORT   servPktRcvd
   		IMPORT	 servDMATimeout
        IMPORT	 servDMAError
        IMPORT	 servParityError
        IMPORT	 servFrammingError
        IMPORT	 servTxFull
        IMPORT	 servTxOverrun
        IMPORT	 servTxEmpty
        IMPORT	 servRouterErrorInt
        IMPORT	 servRouterDumpInt
        IMPORT	 servRouterDiagCntInt
        IMPORT	 servSystemCtl
        IMPORT	 servWatchdog 
        IMPORT	 servEthernetTx
        IMPORT	 servEthernetRx
        IMPORT	 servEthernetPhy
        IMPORT	 servSoftware
        IMPORT   enableEthernet
        IMPORT	 poolVicStatus
 		;IMPORT   procChipStatus
 		
 		EXPORT	initInterrupt
 		EXPORT  testIntCtl
 		
 		PRESERVE8
 		AREA 	intctl, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; initInterrupt
; to change the priority, only need to change prio defination in the head file.
; VIC_PKT_RECEIVED_PRIO   	EQU		   0  ;
; VIC_DMA_PRIO 				EQU		   3  ;
; VIC_TIMER1_PRIO 			EQU		   4  ;
;     MII_PHY_Int,          // [21] Ethernet Controller PHY interrrupt
;     MII_Rx_Int,           // [20] Ethernet Controller Rx interrrupt
;     MII_Tx_Int,           // [19] Ethernet Controller Tx interrrupt
;     Sys_Ctl_Int,          // [18] System Controller interrupt
;     Router_Err_Int,       // [17] Router Error interrupt
;     Router_Dump_Int,      // [16] Router Dump interrupt
;     Router_Diag_Cntr_Int, // [15] Router Diagnostic Counter interrupt
;     dmac_timeout_irq,     // [14] DMA Timed-out interrupt
;     dmac_error_irq,       // [13] DMA Error interrupt
;     dmac_done_irq,        // [12] DMA completed interrupt
;     Tx_Empty_Int,         // [11] Tx pipeline empty interrupt
;     Tx_Ovfl_Int,          // [10] Tx pipeline overrun interrupt
;     Tx_Full_Int,          // [9]  Tx pipeline full interrupt
;     Framing_Error_Int,    // [8]  Framing Error interrupt
;     Parity_Error_Int,     // [7]  Parity Error interrupt
;     Pkt_Rx_Int,           // [6]  Packet Received interrupt
;     TIMINT2,              // [5]  Timer 2
;     TIMINT1,              // [4]  Timer 1
;     COMMTX,               // [3]  ARM Comms Tx
;     COMMRX,               // [2]  ARM Comms Rx
;     1'b0,                 // [1]  Software interrupt
;     WDOGINT};             // [0]  Watchdog interrupt from System Controller
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
initInterrupt
						;STMFD	SP!, {R0-R3, LR}
		LDR		R0,	=VIC_BASE
		MOV		R1, #0					  ;NO FIQ
		STR		R1, [R0, #VICINTSELECT]		
		;;;;;;;;;;;;;clr all Int bits;;;;;;;;;;;
		LDR		R1, =0xFFFFFFFF 			;CLEAR ALL INTERRUPTS #(0x1<<VIC_DMA_COMPLETE_INT):OR:(0x1<<VIC_TIMER1_INT):OR:(0x1<<VIC_CC_PKT_RECEIVED_INT)
		STR		R1, [R0, #VICINTENCLEAR]								
    	;;;;;;;;;;;;;;;VICVECTADDRESS;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    	LDR     R0, =(VIC_BASE+VICVECTADDR0)        ;load VICVECTADDR0 as base address
    	LDR		R2, =(VIC_BASE+VICVECTCNTL0)		;load VICVECTCNTL0 as base address
    	LDR		R3, =procType
    	LDR		R3, [R3]
    	CMP		R3, #0x1						;check if monitor processor
    	BNE		fasInit							
    	
monInit
		;setup watchdog
		LDR		R1, = servWatchdog 
		STR		R1, [R0, #(MON_WDOG_PRITY<<2)]	;initializing the vector Address register
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_WDOG_INT	
		STR		R1, [R2, #(MON_WDOG_PRITY<<2)]  ;initializing the vector control register
    	;setup router
    	LDR		R1, =servRouterErrorInt
		STR		R1, [R0, #(MON_ROUTER_ERROR_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_ROUTER_ERROR_INT	
		STR		R1, [R2, #(MON_ROUTER_ERROR_PRITY<<2)]  ;initializing the vector control register
    	
    	LDR		R1, =servRouterDumpInt
		STR		R1, [R0, #(MON_ROUTER_DUMP_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_ROUTER_DUMP_INT	
		STR		R1, [R2, #(MON_ROUTER_DUMP_PRITY<<2)]  ;initializing the vector control register
		
		;LDR		R5, =servRouterDiagInt
		;STR		R5, [R0, #(MON_ROUTER_DIAG_PRITY<<2)]	;initializing the vector Address register	
		;MOV		R5, #VIC_CNTL_EN_BIT:OR:VIC_ROUTER_DIAG_INT	
		;STR		R5, [R2, #(MON_ROUTER_DIAG_PRITY<<2)]  ;initializing the vector control register
    	
		;setup packet received int
		LDR		R1, =servPktRcvd
		STR		R1, [R0, #(MON_PKT_RECEIVED_PRITY<<2)]	;initializing the vector Address register
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_PKT_RECEIVED_INT	
		STR		R1, [R2, #(MON_PKT_RECEIVED_PRITY<<2)]  ;initializing the vector control register		
		;setup dma Complete interrupt
		LDR		R1, =servDMAComplete
		STR		R1, [R0, #(MON_DMA_COMPLETE_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_DMA_COMPLETE_INT	
		STR		R1, [R2, #(MON_DMA_COMPLETE_PRITY<<2)]  ;initializing the vector control register	
		;setup timer interrupt
		LDR		R1, =servTimer
		STR		R1, [R0, #(MON_TIMER1_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_TIMER1_INT	
		STR		R1, [R2, #(MON_TIMER1_PRITY<<2)]  ;initializing the vector control register	
		;setup tx full interrupt
		LDR		R1, =servTxFull
		STR		R1, [R0, #(MON_TX_FULL_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_TX_FULL_INT	
		STR		R1, [R2, #(MON_TX_FULL_PRITY<<2)]  ;initializing the vector control register	
		;setup tx Overrun interrupt
		LDR		R1, =servTxOverrun
		STR		R1, [R0, #(MON_TX_OVERRUN_PRITY<<2)]	;initializing the vector Address register
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_TX_OVERRUN_INT	
		STR		R1, [R2, #(MON_TX_OVERRUN_PRITY<<2)]  ;initializing the vector control register		
		;****************important**********************
		;txempty interrupt is not enabled in the start
		;the processor will enable it afterward to 
		;handle the congestion i.e. to send a packet only
		;if the comm. noc is free
		;setup txEmpty interrupt
		;LDR		R5, =servTxEmpty
		;STR		R5, [R0, #(MON_TX_EMPTY_PRITY<<2)]	;initializing the vector Address register	
		;MOV		R5, #VIC_CNTL_EN_BIT:OR:VIC_CC_TX_EMPTY_INT	
		;STR		R5, [R2, #(MON_TX_EMPTY_PRITY<<2)]  ;initializing the vector control register	
		;setup parity error interrupt
		LDR		R1, =servParityError
		STR		R1, [R0, #(MON_PARITY_ERROR_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_PARITY_ERROR_INT	
		STR		R1, [R2, #(MON_PARITY_ERROR_PRITY<<2)]  ;initializing the vector control register	
		;setup framming error interrupt
		LDR		R1, =servFrammingError
		STR		R1, [R0, #(MON_FRAMMING_ERROR_PRITY<<2)]	;initializing the vector Address register
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_FRAMMING_ERROR_INT	
		STR		R1, [R2, #(MON_FRAMMING_ERROR_PRITY<<2)]  ;initializing the vector control register		
		;setup dma error interrupt
		LDR		R1, =servDMAError
		STR		R1, [R0, #(MON_DMA_ERROR_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_DMA_ERROR_INT	
		STR		R1, [R2, #(MON_DMA_ERROR_PRITY<<2)]  ;initializing the vector control register	
		;setup dma timeout interrupt
		LDR		R1, =servDMATimeout
		STR		R1, [R0, #(MON_DMA_TIMEOUT_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_DMA_TIMEOUT_INT		
		STR		R1, [R2, #(MON_DMA_TIMEOUT_PRITY<<2)]  ;initializing the vector control register	
		;setup software interrupt
		LDR		R1, =servSoftware
		STR		R1, [R0, #(MON_SOFTWARE_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_SOFTWARE_INT	
		STR		R1, [R2, #(MON_SOFTWARE_PRITY<<2)]  ;initializing the vector control register	
		;setup system controller interrupt
		LDR		R1, =servSystemCtl
		STR		R1, [R0, #(MON_SYS_CTL_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_SYS_CTLR_INT	
		STR		R1, [R2, #(MON_SYS_CTL_PRITY<<2)]  ;initializing the vector control register
		;setup ethernet interrupts
		LDR		R3, =enableEthernet
		LDR		R3, [R3]
		CMP		R3, #0x0
		MOVEQ	R3, #0x0
		BEQ		outmoninit    						;skip initializing the ethernet interface
		LDR		R1, =servEthernetRx					;;;;;;;;;;;;;initializing ethernet rx interrupt
		STR		R1, [R0, #(MON_ETH_RX_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_ETH_RX_INT	
		STR		R1, [R2, #(MON_ETH_RX_PRITY<<2)]  ;initializing the vector control register
		LDR		R1, =servEthernetTx					;;;;;;;;;;;;;initializing ethernet tx interrupt
		STR		R1, [R0, #(MON_ETH_TX_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_ETH_TX_INT	
		STR		R1, [R2, #(MON_ETH_TX_PRITY<<2)]  ;initializing the vector control register
		LDR		R1, =servEthernetPhy				;;;;;;;;;;;;;initializing ethernet phy interrupt
		STR		R1, [R0, #(MON_ETH_PHY_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_ETH_PHY_INT	
		STR		R1, [R2, #(MON_ETH_PHY_PRITY<<2)]  ;initializing the vector control register
		LDR		R3, =(0x1<<VIC_ETH_RX_INT):OR:(0x1<<VIC_ETH_TX_INT):OR:(0x1<<VIC_ETH_PHY_INT) ; to be added up later
outmoninit
		;****************important**********************
		;txempty interrupt is not enabled in the start
		;the processor will enable it afterward to 
		;handle the congestion i.e. to send a packet only
		;if the comm. noc is free
		;enabling the interrupts in vic int enable register
		LDR		R1, =(0x1<<VIC_DMA_COMPLETE_INT):OR:(0x1<<VIC_TIMER1_INT):OR:(0x1<<VIC_CC_PKT_RECEIVED_INT):OR:\
				(0x1<<VIC_SYS_CTLR_INT):OR:(0x1<<VIC_CC_TX_OVERRUN_INT):OR:\
				(0x1<<VIC_CC_TX_FULL_INT):OR:(0x1<<VIC_CC_FRAMMING_ERROR_INT):OR:(0x1<<VIC_CC_PARITY_ERROR_INT)\
				:OR:(0x1<<VIC_WDOG_INT):OR:(0x1<<VIC_ROUTER_ERROR_INT):OR:(0x1<<VIC_ROUTER_DUMP_INT)
		ORR		R1, R1, R3							;to include the ethernet interrupts if any
		LDR		R0,	=VIC_BASE
		STR		R1, [R0, #VICINTENABLE]	
		LDR		R1, =0x0							;CLEAR ALL INTERRUPTS #(0x1<<VIC_DMA_COMPLETE_INT):OR:(0x1<<VIC_TIMER1_INT):OR:(0x1<<VIC_CC_PKT_RECEIVED_INT)
		STR		R1, [R0, #VICINTENCLEAR]								
    	
		B		outInitInt	
fasInit
		;set packet received Int service entry
		LDR		R1, =servPktRcvd
		STR		R1, [R0, #(FAS_PKT_RECEIVED_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_PKT_RECEIVED_INT	
		STR		R1, [R2, #(FAS_PKT_RECEIVED_PRITY<<2)]  ;initializing the vector control register	
		;set DMA Completion Int service entry
		LDR		R1, =servDMAComplete
		STR		R1, [R0, #(FAS_DMA_COMPLETE_PRITY<<2)]	;initializing the vector Address register
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_DMA_COMPLETE_INT	
		STR		R1, [R2, #(FAS_DMA_COMPLETE_PRITY<<2)]  ;initializing the vector control register	
		;set timer Int service entry
		LDR		R1, =servTimer
		STR		R1, [R0, #(FAS_TIMER1_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_TIMER1_INT	
		STR		R1, [R2, #(FAS_TIMER1_PRITY<<2)]  ;initializing the vector control register	
		;set parity error Int service entry
		LDR		R1, =servParityError
		STR		R1, [R0, #(FAS_PARITY_ERROR_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_PARITY_ERROR_INT	
		STR		R1, [R2, #(FAS_PARITY_ERROR_PRITY<<2)]  ;initializing the vector control register	
		;set framming error Int service entry
		LDR		R1, =servFrammingError
		STR		R1, [R0, #(FAS_FRAMMING_ERROR_PRITY<<2)]	;initializing the vector Address register
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_FRAMMING_ERROR_INT	
		STR		R1, [R2, #(FAS_FRAMMING_ERROR_PRITY<<2)]  ;initializing the vector control register		
		;set tx full Int service entry
		LDR		R1, =servTxFull
		STR		R1, [R0, #(FAS_TX_FULL_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_TX_FULL_INT	
		STR		R1, [R2, #(FAS_TX_FULL_PRITY<<2)]  ;initializing the vector control register	
		;set tx overrun Int service entry
		LDR		R1, =servTxOverrun
		STR		R1, [R0, #(FAS_TX_OVERRUN_PRITY<<2)]	;initializing the vector Address register
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_TX_OVERRUN_INT	
		STR		R1, [R2, #(FAS_TX_OVERRUN_PRITY<<2)]  ;initializing the vector control register		
		;set tx empty Int service entry
		;LDR		R1, =servTxEmpty
		;STR		R1, [R0, #(FAS_TX_EMPTY_PRITY<<2)]	;initializing the vector Address register	
		;MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_CC_TX_EMPTY_INT		
		;STR		R1, [R2, #(FAS_TX_EMPTY_PRITY<<2)]  ;initializing the vector control register	
		;set system ctl Int service entry
		LDR		R1, =servSystemCtl
		STR		R1, [R0, #(FAS_SYS_CTL_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_SYS_CTLR_INT	
		STR		R1, [R2, #(FAS_SYS_CTL_PRITY<<2)]  ;initializing the vector control register	
		;set dma error Int service entry
		LDR		R1, =servDMAError
		STR		R1, [R0, #(FAS_DMA_ERROR_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_DMA_ERROR_INT	
		STR		R1, [R2, #(FAS_DMA_ERROR_PRITY<<2)]  ;initializing the vector control register	
		;set dma timeout Int service entry
		LDR		R1, =servDMATimeout
		STR		R1, [R0, #(FAS_DMA_TIMEOUT_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_DMA_TIMEOUT_INT	
		STR		R1, [R2, #(FAS_DMA_TIMEOUT_PRITY<<2)]  ;initializing the vector control register	
		;set software Int service entry
		LDR		R1, =servSoftware
		STR		R1, [R0, #(FAS_SOFTWARE_PRITY<<2)]	;initializing the vector Address register	
		MOV		R1, #VIC_CNTL_EN_BIT:OR:VIC_SOFTWARE_INT	
		STR		R1, [R2, #(FAS_SOFTWARE_PRITY<<2)]  ;initializing the vector control register	
		;;;;;;;;;;;;Enable INT;;;;;;;;;;;;;;;;;
		LDR		R1, =(0x1<<VIC_DMA_COMPLETE_INT):OR:(0x1<<VIC_TIMER1_INT):OR:(0x1<<VIC_CC_PKT_RECEIVED_INT):OR:\
				(0x1<<VIC_SYS_CTLR_INT):OR:(0x1<<VIC_CC_TX_OVERRUN_INT):OR:\
				(0x1<<VIC_CC_TX_FULL_INT):OR:(0x1<<VIC_CC_FRAMMING_ERROR_INT):OR:(0x1<<VIC_CC_PARITY_ERROR_INT)
		LDR		R0,	=VIC_BASE
		STR		R1, [R0, #VICINTENABLE]	
		LDR		R1, =0x0						;CLEAR ALL INTERRUPTS #(0x1<<VIC_DMA_COMPLETE_INT):OR:(0x1<<VIC_TIMER1_INT):OR:(0x1<<VIC_CC_PKT_RECEIVED_INT)
		STR		R1, [R0, #VICINTENCLEAR]								
    	
outInitInt
		LDR		R1, =poolVicStatus
		STR     R1, [R0, #VICDEFVECTADDR]        ;to read this value and go to the routine to pool the int status register
												;to manually find the source of address, in case of non vectored interrupts
      
        BX		LR					;LDMFD	SP!, {R0-R3, PC}
        

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; VIC TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testIntCtl
		LDR		R2,	    =VIC_BASE
    	LDR		R1,   	=0xABCDEF37
    	STR		R1,		[R2, #VICVECTADDR]
    	STR		R1,		[R2, #VICDEFVECTADDR]
    	LDR		R0,		[R2, #VICVECTADDR]
    	CMP		R1, 	R0
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtic	
    	LDR		R0,		[R2, #VICDEFVECTADDR]
    	CMP		R1, 	R0
    	MOVNE	R0, 	#0x0					;return 0 i.e. fail
    	BNE		outtic	
    	MOV		R0,		#0x1					;return 1 i.e. true
outtic	
		MOV		R1,		#0x0
    	STR		R1,		[R2, #VICVECTADDR]
    	STR		R1,		[R2, #VICDEFVECTADDR]
    	BX		LR    	
    	
    	

		END                     

    	
