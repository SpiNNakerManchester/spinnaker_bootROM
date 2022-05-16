;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Created by Mukaram Khan and Xin Jin
; Modified by Thomas Sharp and Cameron Patterson - data from Steve Temple
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		GET		../inc/globals.s
 		
 		EXPORT	init_PL340
 		
 		AREA 	pl340, CODE, READONLY



; Initialise the PL340
init_PL340
		LDR		R0,	=PL340_CONFIG_BASE
		
		LDR		R1, =0x00000410						
		STR		R1, [R0, #0x010]		; Refresh period = 1040 mem clk cycles
		
		MOV		R1, #0x00000006						
		STR		R1, [R0, #0x014]		; CAS latency = 6 mem cycles
				
		MOV		R1, #0x00000001						
		STR		R1, [R0, #0x018]		; Write to DQS = 1 mem cycle
		
		MOV		R1, #0x00000006						
		STR		R1, [R0, #0x01c]		; Mode reg command time = 6 mem cycles
		
		MOV		R1, #0x00000006						
		STR		R1, [R0, #0x020]		; RAS to precharge delay = 6 mem cycles
		
		MOV		R1, #0x00000009						
		STR		R1, [R0, #0x024]		; Bank X to bank X delay = 9 mem cycles
		
		MOV		R1, #0x00000003						
		STR		R1, [R0, #0x028]		; RAS to CAS min delay = 3 mem cycles
		
		LDR		R1, =0x0000010b					
		STR		R1, [R0, #0x02c]		; Auto-refresh cmd time = 11 mem cycles
		
		MOV		R1, #0x00000003						
		STR		R1, [R0, #0x030]		; Precharge to RAS delay = 3 mem cycles
		
		MOV		R1, #0x00000002						
		STR		R1, [R0, #0x034]		; Bank X to bank Y delay = 2 mem cycles
		
		MOV		R1, #0x00000003						
		STR		R1, [R0, #0x038]		; Write to precharge delay = 3 mem cycles
		
		MOV		R1, #0x00000001						
		STR		R1, [R0, #0x03c]		; Write to read delay = 1 memory cycle
		
		MOV		R1, #0x00000001
		STR		R1, [R0, #0x040]		; Exit pwr-down time = 1 mem cycle
		
		MOV		R1, #0x00000010
		STR		R1, [R0, #0x044]		; Exit self-refresh time = 16 mem cycles
		
		MOV		R1, #0x00000014
		STR		R1, [R0, #0x048]		; Self-refresh time = 20 mem cycles
		
		LDR		R1, =0x00210012
		STR		R1, [R0, #0x00c]		; Mem cfg setup
		
		LDR		R1, =0x00000a60						
		STR		R1, [R0, #0x010]		; Refresh period = 2656 mem cycles
		
		LDR		R1, =0x000060e0	
		STR		R1, [R0, #0x200]		; ?
		
		; Commands to mem device attached to PL340
		LDR		R1, =0x000c0000						
		STR		R1, [R0, #0x008]		; Direct cmd
		MOV		R1, #0x00000000						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x00040000						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x00040000						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x00080032						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x001c0000						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x00100000						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x00140000						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x00140000						
		STR		R1, [R0, #0x008]		; Direct cmd
		LDR		R1, =0x00180032						
		STR		R1, [R0, #0x008]		; Direct cmd
		
		MOV		R1, #0x0
		STR		R1, [R0, #0x004]		; Go command to PL340
	    
	    BX		LR



		END
