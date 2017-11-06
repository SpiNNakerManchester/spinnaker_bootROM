;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; globals.s - contains definitions of hardware and software parameters mirroring those in
; globals.h. Any changes made here should be carefully checked against changes in the
; latter file. 
;
;
; Created by Mukaram Khan and Xin Jin
; Modified by Thomas Sharp and Cameron Patterson
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; CHIP DETAILS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IRQ_OFF						EQU     0x80
FIQ_OFF						EQU     0x40

IRQ_MODE					EQU     0x12
SVC_MODE					EQU     0x13

IRQ_STACK_SIZE				EQU		0x500		; CP changed 13th April to ensure stacks are not overrun
SVC_STACK_SIZE				EQU		0x500		; Both were 0x200

DTCM_BASE					EQU		0x400000
DTCM_SIZE					EQU		0x10000

ITCM_BASE					EQU		0x0
ITCM_SIZE					EQU		0x8000

SD_RAM_BASE					EQU		0x70000000

SYS_RAM_BASE				EQU		0xF5000000
SYS_RAM_SIZE				EQU		0x8000		; TODO:  !!!S2!!!	These are the only values in this file that change 
												;					between chip revs (to 0x8000 on Spinnaker2).
NUMBEROFCORES				EQU		0x12		;	!!! S2 !!!		2 CORES on Test Chip,  18 (0x12) CORES on the S2 chip												

VIC_OFFSET					EQU		0xFF0



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; SOFTWARE DEFINES ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ITCM_FAILURE				EQU		0x0
DTCM_FAILURE				EQU		0x1

SECURITY_CODE				EQU		0x5EC00000

UNEXPECTED_EXCEPTION_HIGH	EQU		0xC
UNEXPECTED_EXCEPTION_LOW	EQU		0xD

; NOTE: If the clock speeds are changed, the timer interval must be updated accordingly
PLL_1_SETUP					EQU		0x00060110 ; See comments at end of file. = 160MHz
PLL_2_SETUP					EQU		0x0007011A ;							  = 260MHz   // 0x00060114 = 200MHz
CLK_MUX_CTRL_SETUP			EQU		0x80508821 ; !!!S2!!! See comments at eof, Proc Even/Odd/Rtr/Sys=PLL1, Mem=PLL2 
												;TestChip = 0x50000165;  Div Sys AHB clk by 2. Invert odd CPU clk.
 													;  Sys/Rtr=PLL1, Mem=PLL2, Proc Odd/Even=PLL1 
												;S2= 0x80508821 Invert odd CPU clk. 
 													;Sys AHB clk=2/PLL1. Router=1/PLL1.  Mem=1/PLL2. Odd=1/PLL1. Even=1/PLL1. 	


;;;;;;;;; MEMORY ALLOCATIONS (NB: any changes must also be applied to globals.h) ;;;;;;;;;
ETH_PARAMS_SIZE			EQU		0x20 ; 32 bytes for IP/MAC addresses
ETH_PARAMS_BASE			EQU		(SYS_RAM_BASE + SYS_RAM_SIZE - ETH_PARAMS_SIZE)

FF_SPACE_SIZE			EQU		0x404 ; 1K of data + 4 byte CRC
FF_SPACE_BASE			EQU		(ETH_PARAMS_BASE - FF_SPACE_SIZE)

MAILBOX_SIZE			EQU		(0x4 * 0x4) ; 4 word mailbox common to all fascicles on chip
MAILBOX_BASE			EQU		(FF_SPACE_BASE - MAILBOX_SIZE)

MONITOR_HISTORY			EQU		(MAILBOX_BASE - 0x4)

DETECTED_SDRAM_INFO_SIZE EQU	(0x2 * 0x4)	; 2 word SDRAM info. 1st word = size in bytes. 2nd word = test failures - see code for more detail.
DETECTED_SDRAM_INFO_BASE EQU	(MONITOR_HISTORY - DETECTED_SDRAM_INFO_SIZE)

CHIP_ID					EQU		(DETECTED_SDRAM_INFO_BASE - 0x4) 

FAILURE_LOG_BASE		EQU		(CHIP_ID - (0x4*NUMBEROFCORES))


IVB_BLOCK_SIZE			EQU		148 ; ((32+4+1) * 4)	
ITCM_VALIDATION_BLOCK	EQU		(ITCM_BASE + ITCM_SIZE - IVB_BLOCK_SIZE)			
IVB_IMAGE_STARTLENGTH	EQU		0x00
IVB_IMAGE_LENGTH		EQU		0x04
IVB_IMAGE_EXECUTE_ADDR	EQU		0x08									
IVB_GRAND_CRC			EQU		0x8C
IVB_MAGIC_NUMBER_PLACE	EQU		0x90

IVB_MAGIC_NUMBER		EQU		0xC0FFEE18		; magic number used as a crude check of correct ITCM population on boot	



WATCHDOG_RESET_CODE		EQU		0x4				; 0b100 - signifies the watchdog reset condition 
DMA_CMD_MASTER_CRC_CHK	EQU		0x0018008C		; DMA control word:  dir=1 (tcm to sys), CRC=1 (write), len=(35*4)= 140 bytes
DMA_CMD_BLOCK_CRC_CHK	EQU		0x00180000		; DMA control word:  dir=1 (tcm to sys), CRC=1 (write) (length is added in main code)
DMA_TRANSFER_ACTIVE		EQU		0x1				; bit position remains high while DMA transfer is in progress
DMA_TRANSFER_PAUSED		EQU		0x2				; bit position set high following a DMA transfer error


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; PERIPHERAL REGISTERS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DMA_CTRL_BASE				EQU		0x30000000
DMA_CTRL_STAT				EQU		0x0014
DMA_CTRL_ADRS				EQU		0x0004
DMA_CTRL_ADRT				EQU		0x0008
DMA_CTRL_DESC				EQU		0x000C
DMA_CTRL_CRCC				EQU		0x001C 	
	
PL340_CONFIG_BASE			EQU		0xF0000000
PL340_CONFIG_REFRESH_PERIOD EQU		0x10

SYS_CTRL_BASE				EQU		0xF2000000
SYS_CTRL_BASE_BUFFERED		EQU		0xE2000000
SYS_CTRL_BASE_UNBUFFERED	EQU		0xF2000000
SYS_CTRL_TUBE				EQU		0x0000
SYS_CTRL_CPU_DISABLE		EQU		0x0004
SYS_CTRL_MISC_CONTROL		EQU		0x0038
SYS_CTRL_IO_PORT			EQU		0x0040
SYS_CTRL_IO_DIRECTION		EQU		0x0044
SYS_CTRL_SET_IO				EQU		0x0048
SYS_CTRL_CLEAR_IO			EQU		0x004C
SYS_CTRL_PLL1				EQU		0x0050
SYS_CTRL_PLL2				EQU		0x0054
SYS_CTRL_CLK_MUX_CTRL		EQU		0x0060
SYS_CTRL_RESET_CODE			EQU		0x0030


; PLL SETUP e.g	NS[5:0] 5:0 R/W input clock multiplier	: 010000 = 160MHz
;				MS[5:0] 13:8 R/W output clock divider	: 000001
;				FR[1:0] 17:16 R/W frequency range		: 10 = 100 - 200MHz
;				P 18 R/W NOT power down					: 1
;				T 24 R/W test							: 0

; CLK MUX SETUP e.g	Pe[1:0] 1:0 R/W clock selector for even numbered CPUs	: 01
;					Po[1:0] 3:2 R/W clock selector for odd numbered CPUs	: 01
;					Mem[1:0] 5:4 R/W clock selector for SDRAM (/2 in PL340) : 10
;					Rtr[1:0] 7:6 R/W clock selector for Router				: 01
;					Sys[1:0] 9:8 R/W clock selector for System AHB components : 01
;					Toric[1:0] 17:16 R/W clock selector for Toric clock synthesizer : 00
;					E 24 R/W divide even CPU clock by E+1 (= 1-2)			: 0
;					D 25 R/W divide odd CPU clock by D+1 (= 1-2)			: 0
;					M 26 R/W divide SDRAM clock by M+1 (= 1-2)				: 0
;					R 27 R/W divide Router clock by R+1 (= 1-2)				: 0
;					V 28 R/W invert odd CPU clock							: 1
;					Sdiv[1:0] 31:30 R/W divide System AHB clock by Sdiv+1 (= 1-4) : 01



		END
