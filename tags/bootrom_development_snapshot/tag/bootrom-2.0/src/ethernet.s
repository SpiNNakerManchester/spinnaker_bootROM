;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ethernet.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET peripherals.s        
 		
 		IMPORT  dmaInProcess
 		IMPORT  DMARead
 		IMPORT  DMAWrite
 		IMPORT  txFramePending
 		IMPORT	sourceMACAddressLS
 		IMPORT  sourceMACAddressHS
 		IMPORT  handleIncommingFrame
 		IMPORT  rxBufSize
 		;IMPORT  procChipStatus
 		
 		EXPORT	initEthernet
 		EXPORT  testEthernet
 		;EXPORT  pollTXActive
 		;EXPORT  pollRXCount
 		;EXPORT  sendFrameASM
 		;EXPORT  sendFrameWithDMA
 		;EXPORT  receiveFrameASM
 		PRESERVE8
 		AREA 	ethernet, CODE, READONLY
        ENTRY  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE INIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  

; !! ST - tidied code

initEthernet
		LDR	R2, =ETH_BASE
    		LDR	R1, =sourceMACAddressLS		; private mac address
    		LDR	R1, [R1]
    		LDR	R3, =ETH_MAC_ADDR_LS
    		ADD	R3, R3, R2
    		STR	R1, [R3]			; Set up MAC_ADDRESS registers
    		LDR	R1, =sourceMACAddressHS
    		LDR	R1, [R1]
    		LDR	R3, =ETH_MAC_ADDR_HS
    		ADD	R3, R3, R2
    		STR	R1, [R3]  			; Set up MAC_ADDRESS registers
    		LDR	R1, =0x000
    		LDR 	R3, =ETH_GEN_COMMAND
    		ADD	R3,  R3, R2
    		STR	R1, [R3]  			; Set up GEN_COMMAND register with TX_SYS and RX_SYS disabled
;
; Bring the PHY out of reset
; Set-up Ethernet PHY via SMI if required.
; Set-up may be done automatically via PCB-level configuration, (for example
; using pull-up resistors). See PHY datasheet for details
; If set-up via SMI not required, wait for a few microseconds to allow initialisation
;
; !! ST - Brendan says set up most control bits before RX_SYS_EN
; !! Also -> Check use of loopback bit - now fixed???
;
		MOV	R1, #0x79		; RX_BCAST_EN RX_MCAST_EN RX_UCAST_EN RX_DROP_EN TX_SYS_EN
    		STR	R1, [R3]  		; Set up GEN_COMMAND register with these options
;
		ORR	r1, r1, #2		; Set RX_SYS_EN
		STR	r1, [r3]
	   	BX		LR
;
;     	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE TEST
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
testEthernet
		LDR	 	R2, =(ETH_BASE+ETH_TX_LENGTH)
		MOV		R1,	#0xCD
		STR		R1,	[R2]
    	LDR		R0, [R2]
    	CMP		R0, R1
    	MOVNE	R0, #0x0			  ;return 0 i.e. fail
    	BNE		outet
     	MOV	 	R0, #0x1
     	;LDR		R2, =procChipStatus
    	;LDR		R1, [R2]
    	;ORR		R1, R1, #(0x1 << 14)					;status of comm ctl at bit 14
    	;STR		R1, [R2]
    	;ORR	 	R11, R11, #(0x1 << 14) ;saving status of ethernet interface at bit 14 
outet  	
		BX		LR
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE POLL TX_MII_ACTIVE, R0 to return true or false
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;pollTXActive
;		LDR	 	R2, =(ETH_BASE+ETH_GEN_STATUS)
;		LDR		R0, [R2]        ;to test the tx mii active
;		AND     R0, R0, #0x1							 ;r0 will BE false if inactive or true if still active
;		MOV		PC, LR
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE POLL RX FRAME CNT > 0, R0 to return true or false
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;pollRXCount
;		LDR	 	R2, =(ETH_BASE+ETH_GEN_STATUS)
;		LDR		R0, [R2]        ;to test the tx mii active
;		LDR		R1, =0x1FE	
;		AND     R0, R0, R1					     ;BIT 8-1 = rx frame count, r0 will contain 0 to return false
;		MOV		PC, LR										 ;or a value more than 0 to return true
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE SEND FRAME, returns false if TX_MII_ACTIVE  
; is high i.e. the transmission of a previous frame is in process
; R0 the starting address of the buffer, R1 contains length of frame to send
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;sendFrameWithDMA
;		STMFD	SP!, {R4-R9, LR}
;		LDR	 	R4, =ETH_BASE
;		LDR		R7, =ETH_GEN_STATUS
;		ADD		R8, R4, R7
;		LDR		R5, [R8]        					;to test the tx mii active
;		AND     R5, #0x1							 ;r0 will be false if inactive or true if still active
;		CMP		R5, #0x0
;		MOVNE	R0, #0x0							;if still high, i.e. active
;		BNE		outsendframeWDMA	
    	;DMA OPERATION TO LOAD THE DATA TO TX_FRAME_BUFFER
;    	MOV		R5, #0x1
;    	LDR		R8, =dmaInProcess
;    	STR		R5, [R8]				;setting dmaCompleted to low
 ;   	;R0 contains the buffer start address i.e. dma from 
    	;R1 contains the length of the the frame, copy it to R2 for dmaread operation
;    	MOV		R5, #0x1
;		LDR		R8, =txFramePending
;		STR		R5, [R8]				;TX frame pending 
;		LDR		R7, =ETH_TX_LENGTH
;		ADD		R8, R4, R7
;		STR		R1, [R8]		;writing length of frame to tx length register
 ;   	MOV		R2, R1							;no of bytes
;    	ADD		R1, R4, #ETH_TX_FRAME_BUFFER    ;address of tx frame buffer
;    	BL      DMAWrite
;    	LDR		R8, =dmaInProcess
;loopframeWDMA   
;		LDR     R5, [R8]
;		CMP		R5, #0x0						;see if dmaCompleted
;		BEQ		loopframeWDMA							;loop till dma not completed
	
;		LDR		R7, =ETH_TX_COMMAND
;		ADD		R8, R4, R7
;		STR     R5, [R4]		;start the frame transfer
;		MOV		R0, #0x1						;return true
;outsendframeWDMA
;		LDMFD	 SP!, {R4-R9, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE SEND FRAME, returns false if TX_MII_ACTIVE  
; is high i.e. the transmission of a previous frame is in process
; R0 the starting address of the buffer, R1 contains length of frame to send
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;sendFrameASM
;		STMFD	SP!, {R4-R9, LR}
;		LDR	 	R4, =ETH_BASE
;		LDR		R7, =ETH_GEN_STATUS
;		ADD		R8, R4, R7
;		LDR		R5, [R8]        					;to test the tx mii active
;		AND     R5, #0x1							 ;r0 will be false if inactive or true if still active
;		CMP		R5, #0x0
;		MOVNE	R0, #0x0							;if still high, i.e. active
;		BNE		outsendframe	
    	;DMA OPERATION TO LOAD THE DATA TO TX_FRAME_BUFFER
    	;MOV		R5, #0x1
    	;LDR		R8, =dmaInProcess
;    	;STR		R5, [R8]				;setting dmaCompleted to low
;    	;R0 contains the buffer start address i.e. dma from 
;    	;R1 contains the length of the the frame, copy it to R2 for dmaread operation
;    	MOV		R5, #0x1
 ;   	LDR		R8, =txFramePending
;		STR		R5, [R8]				;TX frame pending 
;		LDR		R7, =ETH_TX_LENGTH
;		ADD		R8, R4, R7
;		STR		R1, [R8]				;writing length of frame to tx length register
;		ADD		R2, R4, #ETH_TX_FRAME_BUFFER    ;address of tx frame buffer
;    	MOV		R3, #0
;loopsendframe
;	    LDR     R5, [R0, R3]		;copy from DTCM i.e. source
;		STR		R5, [R2, R3]
;		ADD		R3, #4				;increment offset
;		CMP		R3, R1				;see if dmaCompleted
;		BLT		loopsendframe		;loop till dma not completed
;	
;		LDR		R7, =ETH_TX_COMMAND
;		ADD		R8, R4, R7
;		MOV		R5, #1
;		STR     R5, [R8]		;start the frame transfer
;		MOV		R0, #0x1		;return true
;outsendframe
;		LDMFD	 SP!, {R4-R9, PC}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE RECEIVE FRAME  with DMA operation 
; better to use if the size of frame is larger
; R0 the starting address of the buffer
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;receiveFrameWithDMA
		;SW read of the frame
		;When SW detects that an unread frame is in RX_FRAME_BUFFER, the frame can be read by
		;use of the following:
		;• Start location indicated by RX_FRAME_BUF_RD_PTR
		;• Length of the frame indicated by currently-visible RX_DESC_WORD
		;After SW has dealt with the received frame, it indicates that it can be over-written by writing a
		;rising-edge to the RX_COMMAND register, executing READ_HANDSHAKE. This causes
		;RX_HOST_FSM to:
		;(1) increment the RX_FRAME_BUF_RD_PTR by RX_FRAME_LEN
		;(2) increment the RX_DESC_RAM_RD_PTR
		;****important****
		;CRC is stored with the data in the rx_frame_buffer, however, its 4 bytes are not included in the frame_lenght
		;stored in the rx_desc_ram (bits 10-0) and left to the application to hendle with this
		
;		STMFD	SP!, {R4-R9, LR}
;		LDR	 	R4, =ETH_BASE
;		;;;;;;;;;;;;;;;;;;; initializing DMA to i.e. R1
;		MOV		R1, R0						   ;dma to, the location at DTCM to store the frame --R1 initialized here
	    ;;;;;;;;;;;;;;;;;;; initializing DMA length i.e. R2
;	    LDR		R7, =ETH_RX_DSC_RD_PTR         ;pointer to the rx_desc_ram
;		ADD		R8, R4, R7
;		LDR		R5, [R8]
;		LDR		R9, =0x3F						;bit 5-0 contain the offset of rx_desc_ram which contains the start of rx frame to be read by processor
;		AND 	R2, R5, R9						;to get bits 5-0, the value refers to the whole word in rx_desc_ram i.e. 32bit word, we need to multiply with 4 to get the start address of that word
;		LSL     R2, R2, #0x2					;multiply with 4 to get address on AHB
;		LDR		R7, =ETH_RX_DESC_RAM
;		ADD		R2, R2, R7						;address in relation to rx_desc_ram
;		ADD		R8, R2, R4						;physical address of rx desc ram where we will find the address of frame in rx frame buffer
;		LDR		R5, [R8]						;
;		LDR 	R9, =0x7FF						;bit 10-0 i.e. 11 bits contain the length of frame
;		AND 	R2, R5, R9						;length of frame i.e. dma request size   --R2 is initialized here
;		LDR		R8, =rxBufSize
;		STR		R2, [R8]						;saving the size of frame in the rxBufSize
;		ADD		R2, R2, #0x4					;adding 4 to get CRC as well alongwith the data to do crc check once again
		
		;;;;;;;;;;;;;;;;;;; initializing DMA from i.e. R0
;		LDR 	R7, =ETH_RX_BUF_RD_PTR
;		ADD		R8, R4, R7
;		LDR		R5, [R8]
;		LDR		R9, =0x3FF						;bit 9-0 contain the offset of rx frame buffer which contains the start of rx frame to be read by processor
;		AND 	R0, R5, R9						;to get bits 9-0, the value contains the position of 32 bit word in rx_frame_buffer
;		LSL		R0, R0, #0x2					;multiply with 4 to get the AHB address
;		LDR		R7, =ETH_RX_FRAME_BUFFER
;		ADD		R0, R0, R7						;address in relation to rx_desc_ram
;		ADD		R0, R0, R4						;physical address of rx desc ram where we will find the address of frame in rx frame buffer
		;;;;;;;;;;;;;;;;;; starting DMA operation
;		BL      DMARead
		;wait for frame to be received
;		LDR		R8, =dmaCompleted
;looprframewdma
;	    LDR     R5, [R8]
;		CMP		R5, #0x1						;see if dmaCompleted
;		BNE		looprframewdma							;loop till dma not completed
		
;		LDR		R7, =ETH_RX_COMMAND
;		ADD		R8, R4, R7
;		STR     R5, [R8]						;set rx command 
;		BL		handleIncommingFrame
;		MOV		R0, #0x1						;return true	
;outreceiveframewithdma
;		LDMFD	 SP!, {R4-R9, pc}	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ETHERNET INTERFACE RECEIVE FRAME with out a DMA operation to speed
; up the process for a small sized frame by avoiding DMA operation 
; overheads
; R0 the starting address of the buffer
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;receiveFrameASM
		;SW read of the frame
		;When SW detects that an unread frame is in RX_FRAME_BUFFER, the frame can be read by
		;use of the following:
		;• Start location indicated by RX_FRAME_BUF_RD_PTR
		;• Length of the frame indicated by currently-visible RX_DESC_WORD
		;After SW has dealt with the received frame, it indicates that it can be over-written by writing a
		;rising-edge to the RX_COMMAND register, executing READ_HANDSHAKE. This causes
		;RX_HOST_FSM to:
		;(1) increment the RX_FRAME_BUF_RD_PTR by RX_FRAME_LEN
		;(2) increment the RX_DESC_RAM_RD_PTR
		;****important****
		;CRC is stored with the data in the rx_frame_buffer, however, its 4 bytes are not included in the frame_lenght
		;stored in the rx_desc_ram (bits 10-0) and left to the application to hendle with this
		
;		STMFD	SP!, {R4-R9, LR}
;		LDR	 	R4, =ETH_BASE
		;;;;;;;;;;;;;;;;;;; initializing DMA to i.e. R1
;		MOV		R1, R0						   ;the location at DTCM to store the frame --R1 initialized here
	    ;;;;;;;;;;;;;;;;;;; initializing DMA length i.e. R2
;	    LDR		R7, =ETH_RX_DSC_RD_PTR         ;pointer to the rx_desc_ram
;		ADD		R8, R4, R7
;		LDR		R5, [R8]
;		LDR		R9, =0x3F						;bit 5-0 contain the offset of rx_desc_ram which contains the start of rx frame to be read by processor
;		AND 	R2, R5, R9						;to get bits 5-0, the value refers to the whole word in rx_desc_ram i.e. 32bit word, we need to multiply with 4 to get the start address of that word
;		LSL     R2, R2, #0x2					;multiply with 4 to get address on AHB
;		LDR		R7, =ETH_RX_DESC_RAM
;		ADD		R2, R2, R7						;address in relation to rx_desc_ram
;		ADD		R8, R2, R4						;physical address of rx desc ram where we will find the address of frame in rx frame buffer
;		LDR		R5, [R8]						;
;		LDR 	R9, =0x7FF						;bit 10-0 i.e. 11 bits contain the length of frame
;		AND 	R2, R5, R9						;length of frame    --R2 is initialized here
;		LDR		R8, =rxBufSize
;		STR		R2, [R8]						;saving the size of frame in the rxBufSize
;		ADD		R2, R2, #0x4					;adding 4 to get CRC as well alongwith the data to do crc check once again
		
		;;;;;;;;;;;;;;;;;;; initializing DMA from i.e. R0
;		LDR 	R7, =ETH_RX_BUF_RD_PTR
;		ADD		R8, R4, R7
;		LDR		R5, [R8]
;		LDR		R9, =0x3FF						;bit 9-0 contain the offset of rx frame buffer which contains the start of rx frame to be read by processor
;		AND 	R0, R5, R9						;to get bits 9-0, the value contains the position of 32 bit word in rx_frame_buffer
;		LSL		R0, R0, #0x2					;multiply with 4 to get the AHB address
;		LDR		R7, =ETH_RX_FRAME_BUFFER
;		ADD		R0, R0, R7						;address in relation to rx_desc_ram
;		ADD		R0, R0, R4						;physical address of rx desc ram where we will find the address of frame in rx frame buffer
		;;;;;;;;;;;;;;;;;; starting DMA operation
		;BL      startDMARead
		;wait for frame to be received
		;LDR		R8, =dmaCompleted

;		MOV		R3, #0
;looprframe
;	    LDR     R5, [R0, R3]	;copy from Ethernet interface i.e. source
;		STR		R5, [R1, R3]	;store at DTCM i.e. destination
;		ADD		R3, #4			;increment offset
;		CMP		R3, R2			;see if completed
;		BLT		looprframe		
		
;		LDR		R7, =ETH_RX_COMMAND
;		ADD		R8, R4, R7
;		STR     R5, [R8]						;set rx command 
;		BL		handleIncommingFrame
;		MOV		R0, #0x1						;return true	
;outreceiveframe
;		LDMFD	 SP!, {R4-R9, pc}	
				
		END 	
