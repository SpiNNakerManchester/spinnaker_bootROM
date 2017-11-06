;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; paripheral.s file 
; contains the base addresses for various chip resources and offsets to their various registers
; used in the device drivers' coding
; by mm khan and xin jin
; modified by 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
						
ITCM_BASE					EQU		0x00000000
ITCM_LIMIT					EQU		0x00007FFF
DTCM_BASE					EQU		0x00400000
DTCM_LIMIT					EQU		0x0040FFFF
SYSTEM_RAM_BASE				EQU		0xF5000000
BOOT_ROM_BASE				EQU		0xFF000000
SVC_Stack       			EQU     DTCM_LIMIT           ; 256 byte SVC stack at top of memory
IRQ_Stack       			EQU     DTCM_LIMIT-0x100     ; followed by IRQ stack
; add FIQ_Stack, ABT_Stack, UNDEF_Stack here if you need them
USR_Stack       			EQU     DTCM_LIMIT-0x100       ; followed by USR stack
Len_FIQ_Stack   			EQU     0x100     ; check embedded_scat.scat for the STACK size
Len_IRQ_Stack   			EQU     0x200
Len_USR_Stack   			EQU     0x200
Len_SVC_Stack				EQU		0x200
Len_ABT_Stack				EQU		0x50
Len_UND_Stack				EQU		0x50
; --- Standard definitions of mode bits and interrupt (I & F) flags in PSRs
Mode_USR        			EQU     0x10
Mode_FIQ        			EQU     0x11
Mode_IRQ        			EQU     0x12
Mode_SVC        			EQU     0x13
Mode_ABT        			EQU     0x17
Mode_UND        			EQU     0x1B
Mode_SYS        			EQU     0x1F    ; available on ARM Arch 4 and later
INTON						EQU		0x0		; enable interrupts
I_Bit           			EQU     0x80    ; when I bit is set, IRQ is disabled
F_Bit           			EQU     0x40    ; when F bit is set, FIQ is disabled    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ProcID Register
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PROCID_REGISTER				EQU		0x40000000
CHIPID_REGISTER				EQU		0xF200003C
; Timer Registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TIMER_BASE					EQU	 	0x11000000
TIMER1Load					EQU	 	0x000			;value from which the counter is to decrement. This is the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero
TIMER1Value					EQU		0x004			;gives the current value of the decrementing counter
TIMER1Control				EQU		0x008			 ;Control Register bits are defined as 
													 ;[31:8] - - Reserved bits, do not modify, and ignore on read
											 		;[7] TimerEn Read/write Enable bit: 0 = Timer module disabled (default), 1 = Timer module enabled.
											 		;[6] TimerMode Read/write Mode bit: 0 = Timer module is in free-running mode (default), 1 = Timer module is in periodic mode, 
											 		;[5] IntEnable Read/write Interrupt Enable bit: 0 = Timer module Interrupt disabled, 1 = Timer module Interrupt enabled (default).
											 		;[3:2] TimerPre Read/write Prescale bits:	00 = clock is divided by 1 (default), 01=clock is divided by 16, 
											 		;               10 =clock is divided by 256, 11 = Undefined, do not use.
											 		;[1] TimerSize Selects 16/32 bit counter operation: 0 = 16-bit counter (default), 1 = 32-bit counter.
											 		;[0] OneShot Selects one-shot or wrapping counter mode: 0 = wrapping mode (default), 1 = one-shot mode.
TIMER1IntClr				EQU     0x00C			 ;Any write to this register, clears the interrupt output from the counter.
TIMER1RIS					EQU		0x010			 ;raw interrupt status in bit 0
TIMER1MIS					EQU		0x014			 ;Enabled interrupt status from the counter
TIMER1BGLoad				EQU		0x018			 ;contains the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero

;;;;;;;;;;;;;;;;;;;TIMER 2;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TIMER2Load					EQU	 	0x020			;value from which the counter is to decrement. This is the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero
TIMER2Value					EQU		0x024			;gives the current value of the decrementing counter
TIMER2Control				EQU		0x028			 ;Control Register bits are defined as 
													 ;[31:8] - - Reserved bits, do not modify, and ignore on read
											 		;[7] TimerEn Read/write Enable bit: 0 = Timer module disabled (default), 1 = Timer module enabled.
											 		;[6] TimerMode Read/write Mode bit: 0 = Timer module is in free-running mode (default), 1 = Timer module is in periodic mode, 
											 		;[5] IntEnable Read/write Interrupt Enable bit: 0 = Timer module Interrupt disabled, 1 = Timer module Interrupt enabled (default).
											 		;[3:2] TimerPre Read/write Prescale bits:	00 = clock is divided by 1 (default), 01=clock is divided by 16, 
											 		;               10 =clock is divided by 256, 11 = Undefined, do not use.
											 		;[1] TimerSize Selects 16/32 bit counter operation: 0 = 16-bit counter (default), 1 = 32-bit counter.
											 		;[0] OneShot Selects one-shot or wrapping counter mode: 0 = wrapping mode (default), 1 = one-shot mode.
TIMER2IntClr				EQU     0x02C			 ;Any write to this register, clears the interrupt output from the counter.
TIMER2RIS					EQU		0x030			 ;raw interrupt status in bit 0
TIMER2MIS					EQU		0x034			 ;Enabled interrupt status from the counter
TIMER2BGLoad				EQU		0x038			 ;contains the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero
;;;;;;;;;;;;;;;;;;;TIMER TEST REGISTERS;;;;;;;;;;;;;;;;;;;;
TIMERITCR 					EQU		0xF00 			;Read/write Integration Test Control Register,
TIMERITOP					EQU 	0xF04 			 ;Write Integration Test Control Register


TIMER_EN_BIT				EQU		(0x1<<7)	     ;1-enable; 0-disable
TIMER_MODE_BIT				EQU		(0x1<<6)		 ;1-periodic mode; 0-free-running
TIMER_INT_EN_BIT			EQU     (0x1<<5)	     ;Interrupt Enable 
													 ;0 = Timer module Interrupt disabled
													 ;1 = Timer module Interrupt enabled (default).
TIMER_PRESCALE				EQU		(0x1<<2)		 ;clock divided by 16

;;;divider 16, prescaler 4, period mode, timer start 
;;;;0b11000100
TIMER1_OPT_CONTROL			EQU		TIMER_EN_BIT|TIMER_MODE_BIT|TIMER_INT_EN_BIT|TIMER_PRESCALE ;contol bits to timer1
TIMER2_OPT_CONTROL			EQU		TIMER_EN_BIT|TIMER_MODE_BIT|TIMER_INT_EN_BIT|TIMER_PRESCALE ;contol bits to timer1

;;;Timer1 default interrupt control source bit is 4 and clk is 100MHz 
;;;value load to timer1 for 1ms timer int

; !! ST - CPU clock is 120MHZ - period 8.3333 - div 16 -> 133.33ns
; !! so timer constant is 1ms or 10^6 ns / 133.33ns -> 7500

TIMER1_OPT_VALUE			EQU		7500

;!! TIMER1_OPT_VALUE			EQU		0x186A   	

TIMER2_OPT_VALUE			EQU		0x124F  	

	 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PL190 VIC Registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VIC_BASE	 				EQU		0xFFFFF000
VICIRQSTATUS 				EQU		0x000 ;RO Shows the status of the interrupts after masking by the VICINTENABLE and VICINTSELECT
VICFIQSTATUS 				EQU		0x004 ;RO Shows the status of the interrupts after masking by the VICINTENABLE and VICINTSELECT
VICRAWINTR 					EQU		0x008 ;RO Shows the status of the interrupts before masking by the enable registers
VICINTSELECT 				EQU		0x00C ;R/W Selects the type of interrupt for interrupt requests: 1 = FIQ interrupt 0 = IRQ interrupt.
VICINTENABLE 				EQU		0x010 ;R/W Enables the interrupt request lines: 1 = Interrupt enabled 0 = Interrupt disabled
VICINTENCLEAR 				EQU		0x014 ;WO Write Clears bits in the VICINTENABLE Register. A HIGH bit clears the corresponding bit in the VICINTENABLE Register. A LOW bit has no effect
VICSOFTINT 					EQU		0x018 ;R/W Setting a bit generates a software interrupt for the specific source interrupt before interrupt masking
VICSOFTINTCLEAR 			EQU		0x01C ;WO Clears bits in the VICSOFTINT Register. A HIGH bit clears the corresponding bit in the VICSOFTINT Register. A LOW bit has no effect
VICPROTECTION 				EQU		0x020 ;R/W Enables or disables protected register access. When enabled, only privileged mode accesses, reads and writes, can access the interrupt controller registers
VICVECTADDR 				EQU		0x030 ;R/W Contains the address of the currently active ISR. Any writes to this register clear the interrupt.
VICDEFVECTADDR 				EQU		0x034 ;R/W Contains the address of the default ISR handler
VICVECTADDR0 				EQU		0x100 ;R/W Contains ISR vector addresses for device at int line 0

VICVECTCNTL0 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL1 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL2 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL3 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL4 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL5 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL6 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL7 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL8 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL9 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL10 				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL11				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL12				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL13				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL14				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt
VICVECTCNTL15				EQU		0x200 ;R/W selects the interrupt source 0 for the vectored interrupt

VICPERIPHID0 				EQU		0xFE0 ;RO 0x90
VICPERIPHID1 				EQU		0xFE4 ;RO 0x11 See Peripheral Identification Registers on page 3-11
VICPERIPHID2 				EQU		0xFE8 ;RO 0x04
VICPERIPHID3 				EQU		0xFEC ;RO 0x00
VICPCELLID0 				EQU		0xFF0 ;RO 0x0D See PrimeCell Identification Registers on page 3-14
VICPCELLID1 				EQU		0xFF4 ;RO 0xF0
VICPCELLID2 				EQU		0xFF8 ;RO 0x05
VICPCELLID3 				EQU		0xFFC ;RO 0xB1

;PL190 VIC bits
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

VIC_WDOG_INT				EQU		 0   ;int line from DMA Ctlr
VIC_SOFTWARE_INT			EQU		 1  ;int line from DMA Ctlr
VIC_COMM_RX_INT				EQU		 2  ;int line from DMA Ctlr
VIC_COMM_TX_INT				EQU		 3  ;int line from DMA Ctlr
VIC_TIMER1_INT				EQU		 4  ;int line from DMA Ctlr
VIC_TIMER2_INT				EQU		 5  ;int line from DMA Ctlr
VIC_CC_PKT_RECEIVED_INT		EQU		 6  ;int line from DMA Ctlr
VIC_CC_PARITY_ERROR_INT		EQU		 7  ;int line from DMA Ctlr
VIC_CC_FRAMMING_ERROR_INT 	EQU		 8  ;int line from DMA Ctlr
VIC_CC_TX_FULL_INT			EQU		 9  ;int line from DMA Ctlr
VIC_CC_TX_OVERRUN_INT 		EQU		10  ;int line from DMA Ctlr
VIC_CC_TX_EMPTY_INT 		EQU		11  ;int line from DMA Ctlr
VIC_DMA_COMPLETE_INT 		EQU		12  ;int line from DMA Ctlr
VIC_DMA_ERROR_INT			EQU		13  ;int line from DMA Ctlr
VIC_DMA_TIMEOUT_INT			EQU		14
VIC_ROUTER_DIAG_INT			EQU		15  ;int line from DMA Ctlr
VIC_ROUTER_DUMP_INT			EQU		16  ;int line from DMA Ctlr
VIC_ROUTER_ERROR_INT		EQU		17  ;int line from DMA Ctlr
VIC_SYS_CTLR_INT			EQU		18  ;int line from DMA Ctlr
VIC_ETH_TX_INT				EQU     19  ;Ethernet transmit frame interrupt
VIC_ETH_RX_INT				EQU		20  ;Ethernet receive frame interrupt
VIC_ETH_PHY_INT				EQU		21  ;Ethernet PHY/external interrupt

VIC_CNTL_EN_BIT				EQU		(0x1<<5)    	;;;enable bit of VIC control register


;Mon proc Priority list
MON_WDOG_PRITY					EQU		 0   ;int line from DMA Ctlr
MON_SOFTWARE_PRITY				EQU		16  ;int line from DMA Ctlr
MON_COMM_RX_PRITY				EQU		17  ;int line from DMA Ctlr
MON_COMM_TX_PRITY				EQU		18  ;int line from DMA Ctlr
MON_TIMER1_PRITY				EQU		10  ;int line from DMA Ctlr
;MON_TIMER2_PRITY				EQU		20  ;int line from DMA Ctlr
MON_PKT_RECEIVED_PRITY			EQU		 1  ;int line from DMA Ctlr
MON_PARITY_ERROR_PRITY			EQU		11  ;int line from DMA Ctlr
MON_FRAMMING_ERROR_PRITY 		EQU		19  ;int line from DMA Ctlr
MON_TX_FULL_PRITY				EQU		 6  ;int line from DMA Ctlr
MON_TX_OVERRUN_PRITY 			EQU		 7  ;int line from DMA Ctlr
;MON_TX_EMPTY_PRITY				EQU		 21  ;int line from DMA Ctlr     ;not enabled
MON_DMA_COMPLETE_PRITY 			EQU		 2  ;int line from DMA Ctlr
MON_DMA_ERROR_PRITY				EQU		 8  ;int line from DMA Ctlr
MON_DMA_TIMEOUT_PRITY			EQU		 9
MON_ROUTER_ERROR_PRITY			EQU		 3  ;int line from DMA Ctlr
MON_ROUTER_DUMP_PRITY			EQU		 4  ;int line from DMA Ctlr
MON_ROUTER_DIAG_PRITY			EQU		 5
MON_SYS_CTL_PRITY				EQU		12  ;int line from DMA Ctlr
MON_ETH_TX_PRITY				EQU     13  ;Ethernet transmit frame interrupt
MON_ETH_RX_PRITY				EQU		14  ;Ethernet receive frame interrupt
MON_ETH_PHY_PRITY				EQU		15  ;Ethernet PHY/external interrupt

;Fascicle proc priority list
FAS_SOFTWARE_PRITY				EQU		10  ;int line from DMA Ctlr
;FAS_COMM_RX_PRITY				EQU		12  ;int line from DMA Ctlr
;FAS_COMM_TX_PRITY				EQU		13  ;int line from DMA Ctlr
FAS_TIMER1_PRITY				EQU		 2  ;int line from DMA Ctlr
;FAS_TIMER2_PRITY				EQU		11  ;int line from DMA Ctlr
FAS_PKT_RECEIVED_PRITY			EQU		 0  ;int line from DMA Ctlr
FAS_PARITY_ERROR_PRITY			EQU		 3  ;int line from DMA Ctlr
FAS_FRAMMING_ERROR_PRITY 		EQU		 4  ;int line from DMA Ctlr
FAS_TX_FULL_PRITY				EQU		 5  ;int line from DMA Ctlr
FAS_TX_OVERRUN_PRITY 			EQU		 6  ;int line from DMA Ctlr
;FAS_TX_EMPTY_PRITY				EQU		 7  ;int line from DMA Ctlr  ;not enabled
FAS_DMA_COMPLETE_PRITY 			EQU		 1  ;int line from DMA Ctlr
FAS_DMA_ERROR_PRITY				EQU		 7  ;int line from DMA Ctlr
FAS_DMA_TIMEOUT_PRITY			EQU		 8
FAS_SYS_CTL_PRITY				EQU		 9



VIC_VECTADDR_OFFSET			EQU		0xFF0 ;used in vectors.s
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TLM DMAC registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DMAC_BASE					EQU		  0x30000000
TLM_DMAC_CRCP				EQU		  0x00 ;R/W DMA address on the system interface
TLM_DMAC_ADRS				EQU		  0x04 ;R/W DMA address on the system interface
TLM_DMAC_ADRT				EQU		  0x08 ;R/W DMA address on the TCM interface
TLM_DMAC_DESC				EQU		  0x0C ;R/W Length of the transfer in bytes
TLM_DMAC_CTRL				EQU		  0x10 ;R/W Control DMA transfer
TLM_DMAC_STAT				EQU		  0x14 ;R Status of DMA and other transfers
TLM_DMAC_GCTL				EQU		  0x18 ;R/W Control of the DMA device
TLM_DMAC_CRCC				EQU		  0x1C ;R CRC value calculated by CRC block
TLM_DMAC_CRCR				EQU		  0x20 ;R CRC value in received block
TLM_DMAC_TMTV				EQU		  0x24 ;R CRC value in received block
TLM_DMAC_CRC2				EQU		  0x100 ;R* Queued system address
TLM_DMAC_AD2S				EQU		  0x104 ;R* Queued TCM address
TLM_DMAC_AD2T				EQU		  0x108 ;R* Queued TCM address
TLM_DMAC_DES2				EQU		  0x10C ;R* Queued control
DMAC_DESC_DIR_BIT			EQU		  19
DMAC_DESC_CRC_BIT			EQU		  20
DMAC_DESC_BURST				EQU		  21
DMAC_DESC_WIDTH				EQU		  24
DMAC_DESC_PRIVILLEGE		EQU		  25
DMAC_DESC_TRFID				EQU		  26
DMAC_STATUS_TRF_INPROG		EQU		  0
DMAC_STATUS_TRF_PAUSED		EQU		  1
DMAC_STATUS_TRF_QUEUED		EQU		  2
DMAC_STATUS_WB_FULL			EQU		  3
DMAC_STATUS_WB_ACTIVE		EQU		  4
DMAC_STATUS_COND_CODE		EQU		  10
DMAC_STATUS_ENINT_DONE		EQU		  10
DMAC_STATUS_ENINT_DONE2		EQU		  11
DMAC_STATUS_ENINT_TOUT		EQU		  12
DMAC_STATUS_ENINT_CRCERROR	EQU		  13
DMAC_STATUS_ENINT_TCMERROR	EQU		  14
DMAC_STATUS_ENINT_AXIERROR	EQU		  15
DMAC_STATUS_ENINT_USERABORT	EQU		  16
DMAC_STATUS_ENINT_SOFTRST	EQU		  17
DMAC_STATUS_ENINT_WB		EQU		  20
DMAC_STATUS_PROCID			EQU		  24
DMAC_CTRL_UNCOMMIT			EQU		  0
DMAC_CTRL_ABORT				EQU		  1
DMAC_CTRL_RESTART			EQU		  2
DMAC_CTRL_CLINT_DONE		EQU		  3
DMAC_CTRL_CLINT_TIMEOUT		EQU		  4
DMAC_CTRL_CLINT_WB			EQU		  5
DMAC_GCTL_EN_BR_BUF			EQU		  0
DMAC_GCTL_ENINT_DONE		EQU		  10
DMAC_GCTL_ENINT_DONE2		EQU		  11
DMAC_GCTL_ENINT_TOUT		EQU		  12
DMAC_GCTL_ENINT_CRCERROR	EQU		  13
DMAC_GCTL_ENINT_TCMERROR	EQU		  14
DMAC_GCTL_ENINT_AXIERROR	EQU		  15
DMAC_GCTL_ENINT_USERABORT	EQU		  16
DMAC_GCTL_ENINT_SOFTRST		EQU		  17
DMAC_GCTL_ENINT_WB			EQU		  20

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DMAC registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DMACIntStatus				EQU		0x000
DMACIntTCStatus				EQU		0x004
DMACIntTCClear				EQU		0x008
DMACIntErrorStatus			EQU		0x00C
DMACIntErrClr				EQU		0x010
DMACRawIntTCStatus			EQU		0x014
DMACRawIntErrorStatus 		EQU		0x018
DMACEnbldChns				EQU		0x01C
DMACSoftBReq				EQU		0x020
DMACSoftSReq				EQU		0x024
DMACSoftLBReq				EQU		0x028
DMACSoftLSReq				EQU		0x02C
DMACConfiguration			EQU		0x030
DMACSync					EQU		0x034
DMACC0SrcAddr				EQU		0x100
DMACC0DestAddr				EQU		0x104
DMACC0LLI					EQU		0x108
DMACC0Control				EQU		0x10C
DMACC0Configuration 		EQU		0x110

; DMAC chanel address offset
DMAC_C0_BASE				EQU     0x100
DMAC_C1_BASE				EQU     0x120
DMAC_C2_BASE				EQU     0x140
DMAC_CH_SRC	  	  			EQU     0x00
DMAC_CH_DEST	  			EQU     0x04
DMAC_CH_LLI		  			EQU     0x08
DMAC_CH_CONTROL	  			EQU	    0x0c
DMAC_CH_CONFIG	  			EQU     0x10

;;;;;;;;;;;;;;;;;;;;;DMA Interrupt bits 
DMAC_INT_BIT      			EQU     0x40

;;;;;;;;;;;;;;;;;;;;;DMA control bits
						
DWidth						EQU		0x2					;to support word width transfer
SWidth						EQU		0x2					;to support word width transfer
DMA_INT_TERM				EQU		(0x1<<31)           ;bit31	Interrupt on termination
DMA_SRC_INC					EQU		(0x1<<26)			;bit26	Src incr
DMA_DEST_INC				EQU		(0x1<<27)			;bit27	Dst incr
DMA_WRITE_SRCBUS			EQU		(0x1<<24)			;bit24	Src AHB Bus
DMA_WRITE_DESTBUS			EQU		(0x0<<25)			;bit25	Dst AHB Bus
DMA_READ_SRCBUS				EQU		(0x0<<24)			;bit24  Src AHB Bus
DMA_READ_DESTBUS			EQU		(0x1<<25)			;bit25 	Dst AHB Bus
DMA_DWIDTH					EQU     (DWidth<<21)		;bit21	DWidth (0-byte; 1-half word; 2-word)
DMA_SWIDTH					EQU		(SWidth<<18)		;bit18	SWidth (0-byte; 1-half word; 2-word)
DMA_NOOFBYTES_MASK			EQU		0xfff				
DMA_READ_CONTROL			EQU	 	(DMA_INT_TERM|DMA_DEST_INC|DMA_SRC_INC|DMA_READ_SRCBUS|DMA_READ_DESTBUS|DMA_DWIDTH|DMA_SWIDTH)
DMA_WRITE_CONTROL 			EQU	 	(DMA_INT_TERM|DMA_DEST_INC|DMA_SRC_INC|DMA_WRITE_SRCBUS|DMA_WRITE_DESTBUS|DMA_DWIDTH|DMA_SWIDTH)

;control = 
;        	  (dmaWidth << 21) // DWidth
;		| 	  (dmaWidth << 18) // SWidth
;		| 	  (1 << 26) // Src incr
;		| 	  (1 << 27) // Dst incr
;		| 	  ((unsigned)1 << 31) // Interrupt on termination
;		| 	  (pTransfer->bytes & 0xfff) // size
;		| 	  ((pTransfer->srcBus & 1) << 24) // Src AHB Bus
;		| 	  ((pTransfer->dstBus & 1) << 25) // Dst AHB Bus
;		;
;;;;;;;;;;;;;;;;;;;;;DMA chanel CONFIG bits
DMA_TCI_MASK				EQU		0x1<<15			;bit15	Terminal count interupt mask
DMA_FlOW_CTl				EQU     0x0<<11			;peripheral to memory
DMA_CH_EN					EQU		0x1<<0			;bit0	Enable			
DMA_OPT_CONFIG				EQU		(DMA_TCI_MASK|DMA_CH_EN)	
;config = (1 << 15) // Terminal count interupt mask
;		| 	 (1 <<  0) // Enable
;		;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Comm Ctlr registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
COMMCTLR_BASE				EQU		0x10000000
COMMCTLR_R0					EQU		0x00	;Tx Control register
COMMCTLR_R1					EQU		0x04	;Rx Control register
COMMCTLR_R2					EQU		0x08	;Tx payload
COMMCTLR_R3					EQU		0x0C	;Tx routing key
COMMCTLR_R4					EQU		0x10	;Rx payload
COMMCTLR_R5					EQU		0x14	;Rx routing key
COMMCTLR_R6					EQU		0x18	;reserved
COMMCTLR_R7					EQU		0x1C	;test control register

;Comm Ctlr Bits

COMMCTLR_R1_DATA			EQU		(0x1<<17)   ;payload
COMMCTLR_R1_PKTTYPE_MC		EQU		(0x0<<22)	;packety type MC
COMMCTLR_R1_PKTTYPE_P2P		EQU		(0x1<<22)
COMMCTLR_R1_PKTTYPE_NN		EQU     (0x2<<22)
COMMCTLR_R1_ROUTE			EQU     (0x7<<24)  ;route value to be 7 and sourceid=0

COMMCTLR_R0_CON				EQU		 0x16
COMMCTLR_R0_ROUTE			EQU		 0x24
COMMCTLR_R0_DATA			EQU		 0x17
COMMCTLR_R0_TXOVERFLOW		EQU		 0x29
COMMCTLR_R0_TXFULL			EQU		 0x30
COMMCTLR_R0_TXEMPTY			EQU		 0x31


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Router Registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RTR_BASE					EQU		0xF1000000
RTR_R0_CONTROL 				EQU		0x0 		;R/W Router control register
RTR_R1_STATUS  				EQU		0x4 		;R Router status
RTR_R2_ERROR_HEADER			EQU		0x8       	;error header  R error packet control byte and flags
RTR_R3_ERROR_ROUTE			EQU     0xC       	;error routing R error packet routing word
RTR_R4_ERROR_PL				EQU     0x10		;error payload  R error packet data payload
RTR_R5_ERROR_STATUS			EQU     0x14      	;error status  R error packet status
RTR_R6_DUMP_HEADER			EQU     0x18      	;dump header  R dumped packet control byte and flags
RTR_R7_DUMP_ROUTE			EQU     0x1C      	;dump routing  R dumped packet routing word
RTR_R8_DUMP_PL				EQU     0x20      	;dump payload  R dumped packet data payload
RTR_R9_DUMP_OUTPUTS			EQU     0x24      	;dump outputs  R dumped packet intended destinations
RTR_R10_DUMP_STATUS			EQU     0x28      	;dump status 0x28 R dumped packet status
RTR_R11_DIAG_ENABLES		EQU     0x2C      	;diag enables 0x2C R/W diagnostic counter enables
RTR_R2N_DIAG_CONTROLS		EQU     0x200     	;diag filter 0x200-21F R/W diagnostic count filters (N = 0-7)
RTR_R3N_DIAG_COUNT			EQU     0x300     	;diag count 0x300-31F R/W diagnostic counters (N = 0-7)
RTR_RT1						EQU     0xF00     	;test register  R/W hardware test register 1
RTR_RT2 					EQU		0xF04     	;test key  R/W hardware test register 2 - CAM input test key
RTR_ROUTE_TABLE				EQU  	0x4000    	;[1023:0]  R/W MC Router routing word values
RTR_KEY_TABLE				EQU     0x8000    	;[1023:0]  W MC Router key values
RTR_MASK_TABLE				EQU  	0xC000    	;[1023:0]  W MC Router mask values
RTR_P2P_TABLE				EQU     0x10000   	;[65535:0]  R/W P2P Router routing entries



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; System Controller Registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SYSCTLR_BASE_U				EQU		0xF2000000
SYSCTLR_BASE_B				EQU		0xE2000000
SYSCTLR_TUBE 				EQU		0x00    ;W
SYSCTLR_CHIP_ID				EQU		0x00 	;R Chip ID register (hardwired)
SYSCTLR_CPU_DISABLE			EQU		0x04 	;R/W Each bit disables the clock of a processor
SYSCTLR_SET_CPU_IRQ			EQU		0x08 	;R/W Writing a 1 sets a processor’s interrupt line
SYSCTLR_CLR_CPU_IRQ			EQU		0x0C 	;R/W Writing a 1 clears a processor’s interrupt line
SYSCTLR_SET_CPU_OK			EQU		0x10 	;R/W Writing a 1 sets a CPU OK bit
SYSCTLR_CLR_CPU_OK			EQU		0x14 	;R/W Writing a 1 clears a CPU OK bit
SYSCTLR_CPU_RST_LV			EQU		0x18 	;R/W Level control of CPU resets
SYSCTLR_NODE_RST_LV			EQU		0x1C 	;R/W Level control of CPU node resets
SYSCTLR_SBSYS_RST_LV		EQU		0x20 	;R/W Level control of subsystem resets
SYSCTLR_CPU_RST_PU			EQU		0x24 	;R/W Pulse control of CPU resets
SYSCTLR_NODE_RST_PU			EQU		0x28 	;R/W Pulse control of CPU node resets
SYSCTLR_SBSYS_RST_PU		EQU		0x2C 	;R/W Pulse control of subsystem resets
SYSCTLR_RESET_CODE			EQU		0x30 	;R Indicates cause of last chip reset
SYSCTLR_MONITOR_ID			EQU		0x34 	;R/W ID of Monitor Processor
SYSCTLR_MISC_CONTROL		EQU		0x38 	;R/W Miscellaneous control bits
SYSCTLR_MISC_STATUS			EQU		0x3C 	;R Miscellaneous status bits
SYSCTLR_IO_PORT				EQU		0x40 	;R/W I/O pin output register
SYSCTLR_IO_DIRECTION		EQU		0x44 	;R/W External I/O pin is input (1) or output (0)
SYSCTLR_SET_IO				EQU		0x48 	;R/W Writing a 1 sets IO register bit
SYSCTLR_CLEAR_IO			EQU		0x4C 	;R/W Writing a 1 clears IO register bit
SYSCTLR_PLL1				EQU		0x50	;R/W PLL1 frequency control
SYSCTLR_PLL2				EQU		0x54 	;R/W PLL2 frequency control
SYSCTLR_TORIC0				EQU		0x58 	;R Toric frequency synthesis control 0
SYSCTLR_TORIC1				EQU		0x5C 	;R/W Toric frequency synthesis control 1
SYSCTLR_CLK_MUX_CTL			EQU		0x60 	;R/W Clock multiplexer controls
SYSCTLR_CPU_SLEEP			EQU		0x64 	;R CPU sleep (awaiting interrupt) status
SYSCTLR_ARBITER				EQU		0x80 	;to FC R Read sensitive semaphores to determine MP
SYSCTLR_Test_Set 			EQU		0x100   ;R Test & Set registers for general software use
SYSCTLR_Test_Clr 			EQU		0x180   ;R Test & Clear registers for general software use
SYSCTLR_MISC_TEST			EQU		0x200 	;R/W Miscellaneous chip test control bits
SYSCTLR_LINK_DISABLE 		EQU		0x204 	;R/W Disables for Tx and Rx link interfaces;r33_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r34_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r35_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r36_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r37_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r38_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r39_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r40_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r41_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r42_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r43_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r44_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r45_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r46_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r47_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r48_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r49_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r50_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r51_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r52_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r53_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r54_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r55_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r56_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r57_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r58_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r59_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r60_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r61_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
;r62_Arbiter 0x80-FC R Read sensitive semaphores to determine MP
SYSCTLR_r63_Arbiter			EQU		0xFC	; R Read sensitive semaphores to determine MP
SYSCTLR_rT1_Misc_Test		EQU		0x100 	; R/W Miscellaneous chip test control bits
SYSCTLR_rT2_Link_Disable	EQU		0x104 	; R/W Disables for Tx and Rx link interfaces

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Watchdog Timer Registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
WDOG_BASE					EQU		0xF3000000
WDOG_LOAD 					EQU		0x0 	;R/W Count load register
WDOG_VALUE 					EQU		0x4 	;R Current count value
WDOG_CONTROL 				EQU		0x8 	;R/W Control register
WDOG_INTCLR					EQU		0xC 	;W Interrupt clear register
WDOG_RIS 					EQU		0x10 	;R Raw interrupt status register
WDOG_MIS  					EQU		0x14 	;R Masked interrupt status register
WDOG_LOCK  					EQU		0xC00 	;R/W Lock register
WDOG_ITCR 					EQU		0xF00 	;R/W Watchdog integration test control register
WDOG_ITOP  					EQU		0xF04 	;W Watchdog integration test output set register
WDOG_COUNTER_VALUE			EQU		20		;in ms



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DMC PL340 Configuration Registers for configuration
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SDRAM_BASE_B				EQU		0x60000000
SDRAM_BASE					EQU		0x70000000
PL340CFG_BASE				EQU		0xF0000000
PL340CFG_STATUS  			EQU		0x0 ;R memory controller status
PL340CFG_COMMAND		 	EQU		0x4 ;W PL340 command
PL340CFG_DIRECT 		 	EQU		0x8 ;W direct command
PL340CFG_MEM_CFG 			EQU		0xC ;R/W memory configuration
PL340CFG_REFRESH_PRD 		EQU		0x10 ;R/W refresh period
PL340CFG_CAS_LATENCY 		EQU		0x14 ;R/W CAS latency
PL340CFG_T_DQSS 			EQU		0x18 ;R/W write to DQS time
PL340CFG_T_MRD 				EQU		0x1C ;R/W mode register command time
PL340CFG_T_RAS 				EQU		0x20 ;R/W RAS to precharge delay
PL340CFG_T_RC 				EQU		0x24 ;R/W active bank x to active bank x delay
PL340CFG_T_RCD 				EQU		0x28 ;R/W RAS to CAS minimum delay
PL340CFG_T_RFC 				EQU		0x2C ;R/W auto-refresh command time
PL340CFG_T_RP 				EQU		0x30 ;R/W precharge to RAS delay
PL340CFG_T_RRD 				EQU		0x34 ;R/W active bank x to active bank y delay
PL340CFG_T_WR 				EQU		0x38 ;R/W write to precharge delay
PL340CFG_T_WTR 				EQU		0x3C ;R/W write to read delay
PL340CFG_T_XP 				EQU		0x40 ;R/W exit power-down command time
PL340CFG_T_XSR 				EQU		0x44 ;R/W exit self-refresh command time
PL340CFG_T_ESR 				EQU		0x48 ;R/W self-refresh command time
PL340CFG_T_ID_N_CFG 		EQU		0x100 ;R/W QoS settings
PL340CFG_CHIP_N_CFG 		EQU		0x200 ;R/W external memory device configuration
PL340CFG_USER_STATUS 		EQU		0x300 ;R state of user_status[7:0] primary inputs
PL340CFG_USER_CONFIG 		EQU		0x304 ;W sets the user_config[7:0] primary outputs
PL340CFG_INT_CFG 			EQU		0xE00 ;R/W integration configuration register
PL340CFG_INT_INPUTS 		EQU		0xE04 ;R integration inputs register
PL340CFG_INT_OUTPUTS 		EQU		0xE08 ;W integration outputs register

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Ethernet Interface
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ETH_BASE					EQU		0xF4000000
ETH_TX_FRAME_BUFFER 		EQU		0x0000 	;W Transmit frame RAM area
ETH_RX_FRAME_BUFFER 		EQU		0x4000 	;R Receive frame RAM area
ETH_RX_DESC_RAM 			EQU		0x8000 	;R Receive descriptor RAM area
ETH_GEN_COMMAND 			EQU		0xC000 	;R/W General command
ETH_GEN_STATUS 				EQU		0xC004 	;R General status
ETH_TX_LENGTH 				EQU		0xC008 	;R/W Transmit frame length
ETH_TX_COMMAND 				EQU		0xC00C 	;W Transmit command
ETH_RX_COMMAND 				EQU		0xC010 	;W Receive command
ETH_MAC_ADDR_LS  			EQU		0xC014 	;R/W MAC address low bytes
ETH_MAC_ADDR_HS 			EQU		0xC018 	;R/W MAC address high bytes
ETH_PHY_CONTROL 			EQU		0xC01C 	;R/W PHY control
ETH_IRQ_CLEAR 				EQU		0xC020 	;W Interrupt clear
ETH_RX_BUF_RD_PTR 			EQU		0xC024 	;R Receive frame buffer read pointer
ETH_RX_BUF_WR_PTR 			EQU		0xC028 	;R Receive frame buffer write pointer
ETH_RX_DSC_RD_PTR 			EQU		0xC02C 	;R Receive descriptor read pointer
ETH_RX_DSC_WR_PTR 			EQU		0xC030 	;R Receive descriptor write pointer
ETH_RX_SYS_STATE 			EQU		0xC034 	;R Receive system FSM state (debug & test use)
ETH_TX_MII_STATU 			EQU		0xC038 	;R Transmit MII FSM state (debug & test use)
ETH_PERIPH_ID 				EQU		0xC03C 	;R Peripheral ID (debug & test use)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PHY
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PHY_BASE					EQU	    0xF400C100   ;
PHY_CONTROL					EQU		0x00
PHY_STATUS					EQU		0x04
PHY_ID						EQU     0x08
PHY_INT_STATUS				EQU		0x0C
PHY_INT_MASK				EQU		0x10
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Memory Components
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



					END
        
        