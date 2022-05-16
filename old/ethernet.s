;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ethernet.s
; 
; Created by Mukaram Khan and Xin Jin
; Modified by Thomas Sharp
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;TODO Tidied by TS 29/01/10. Needs to be documented.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
		GET peripherals.s        

		IMPORT	sourceMACAddressLS
		IMPORT  sourceMACAddressHS
 		
;		EXPORT	initEthernet
;		EXPORT  testEthernet
		
		PRESERVE8
		AREA 	ethernet, CODE, READONLY
		ENTRY  


;initEthernet Commented out by TS. See perhipheral_inits.c
;		LDR		R2, =ETH_BASE
;		
;		LDR		R1, =sourceMACAddressLS
;		LDR		R1, [R1]
;		LDR		R3, =ETH_MAC_ADDR_LS
;		ADD		R3, R3, R2
;		STR		R1, [R3]
;		
 ;		LDR		R1, =sourceMACAddressHS
	;	LDR		R1, [R1]
	;	LDR		R3, =ETH_MAC_ADDR_HS
;		ADD		R3, R3, R2
;		STR		R1, [R3]
;		
;		LDR		R1, =0x000
;		LDR 	R3, =ETH_GEN_COMMAND
;		ADD		R3, R3, R2
;		STR		R1, [R3]  			; Set up GEN_COMMAND register with TX_SYS and RX_SYS disabled
		
;		MOV		R1, #0x79		; RX_BCAST_EN RX_MCAST_EN RX_UCAST_EN RX_DROP_EN TX_SYS_EN
;		STR		R1, [R3]  		; Set up GEN_COMMAND register with these options
;		
;		ORR		R1, R1, #2		; Set RX_SYS_EN
;		STR		R1, [R3]
;		BX		LR


;testEthernet Commented out by TS. See peripheral tests.c
;		LDR	 	R2, =(ETH_BASE + ETH_TX_LENGTH)
;		MOV		R1,	#0xCD
;		STR		R1,	[R2]
 ;   	LDR		R0, [R2]
  ;  	CMP		R0, R1
   ; 	MOVNE	R0, #0x0
;     	MOVEQ 	R0, #0x1
;		BX		LR

		END
