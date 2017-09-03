/*****************************************************************************************
*	globals.h - contains definitions of hardware and software parameters mirroring those 
*	in globals.s. Any changes made here should be carefully checked against changes in the
*	latter file. 
*
*
*	Created by Mukaram Khan and Xin Jin
*	Chopped Hacked and Sliced by Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#ifndef GLOBALS_H
#define GLOBALS_H

//#define SPINNAKER2					// uncommented for the SpiNNaker2 chip, commented TestChip. Search: !!!S2!!! for changes reqd

//#define QUIMONDA						// !!!S2!!! comment this out if using MICRON SDRAM part and not the QUIMONDA RAM.
#define WATCHDOGON						// comment me out if you don't want the watchdog to come on
//#define APPTEST							// turns of initialisations to pretend to be an app

#define DEBUGPRINTF					0x1 // set to 0 if not using printf!
//#define TLMTUBEPRINTF					// comment this out if not simulating on the TLM (overrides the 3 below when defined)
//#define ETHTUBEPRINTF					// comment this out if you don't want debugging frames being sent on Ethernet
//#define DEBUGPRINTFINTERCHIP			// comment out if you don't want non-Ethernet chips sending debugging by NN packets
//#define DEBUGPRINTFSERIAL				// comment this out if you don't want to use the serial tube

#ifndef SPINNAKER2
  #define TEST_MULTICHIP_PRINTF
//  #define TEST_LIGHT_UP_SPOKE_LEDS
//  #define TEST_SDRAM

//  #define TEST_NUMBER_CHIPS_WITH_NN	// For Ph1 image. NB comment "#define APPTEST" above
//  #define TEST_POKING_IMAGE_OUT		// For Ph1 image. NB in start.s comment: "B stackinit	; !!!APP!!!"

//  #define TEST_FLOOD_FILL_IMAGE_OUT		// For Ph2 (app) image. NB uncomment "#define APPTEST" above
//  #define TEST_FASCICLE_BOOTER			// For Ph2 (app) image. NB in start.s uncomment: "B stackinit	; !!!APP!!!"
    
//  #define TEST_PRINT_P2PROUTINGTABLE
//  #define TEST_REPEAT_FLOOD_FILL_IMAGE_OUT
//  #define TEST_FASCICLE_WAKE_AND_BOOT
//  #define TEST_ETHERNET_RESETS
//  #define TEST_DMA_BURST_BUG
//  #define TEST_CC_PACKET_RECEIVED
//  #define TEST_IBV_POPULATION

#endif



/*---------------------------------- AUTO-DEFINE ---------------------------------------*/
#ifndef SPINNAKER2
 #define SPINNAKERTESTCHIP
 #define TEST_BOARD
#endif



/*---------------------------------- VERSIONING ----------------------------------------*/
#ifdef SPINNAKER2
#define VERSION						0x1		// Stream of library/ROM code...
#endif
#ifdef SPINNAKERTESTCHIP
#define VERSION						0x2		// Stream of library/ROM code...
#endif
#define SUBVERSION					0x1		// ... release ID (i.e. v1.1 for TestChip,  v2.1 for SpiNNaker2)



/*---------------------------------- TYPE DEFINITIONS ----------------------------------*/
typedef volatile unsigned int *const pointer;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;



/* ------------------------------------ CHIP DETAILS -----------------------------------*/
#ifdef SPINNAKER2
	#define NUMBEROFCORES			18		// number of cores in SpiNNaker2 chip  TODO !!!S2!!! verify
#endif
#ifdef SPINNAKERTESTCHIP
	#define NUMBEROFCORES			2		// number of cores in test chip
#endif

#define DMA_CRC_OFF					0x0
#define DMA_CRC_ON					0x1
#define	DMA_READ					0x0
#define DMA_WRITE					0x1
#define DMA_IN_PROGRESS				0x0001
#define DMA_PAUSED					0x0002
#define DMA_Q_FULL					0x0004
#define DMA_TRANSFER_DONE			0x0400
#define DMA_CRC_ERROR				0x2000
#define DMA_CLEAR_ERRORS			0x4
#define DMA_ACK_SUCCESS				0x8

#define HELLO_FREQUENCY				0x60		// send an Ethernet hello approx. every 4096ms !!!TLM 100ms, was 0x1000!!!
#define ITUBOTRON_FREQUENCY			0x80		// service any printf output approx every 128ms
#define ITUBE_CMD					0x04		// Command 04 is used for message passing on itubotron (per ST)

#ifdef SPINNAKER2
	#define ROUTER_MC_TABLE_SIZE	0x400	// number of entries
	#define ROUTER_P2P_TABLE_SIZE	0x2000	// number of entries
#endif
#ifdef SPINNAKERTESTCHIP
	#define ROUTER_MC_TABLE_SIZE	0x100
	#define ROUTER_P2P_TABLE_SIZE	0x100
#endif

#define SECURITY_CODE				0x5EC00000
#define IVB_MAGIC_NUMBER			0xC0FFEE18			// magic number used as a crude check of correct ITCM population on boot	
#define MAGIC_NUMBER				IVB_MAGIC_NUMBER	// for more general use without the IVB label

#define SD_RAM_BASE					0x70000000

#define SYS_RAM_BASE				0xF5000000					

#ifdef SPINNAKERTESTCHIP			// testchip has 16KB of SysRAM
 #define	SYS_RAM_SIZE			0x4000
#endif
#ifdef SPINNAKER2					// spinnaker2 doubles the SysRAM size to 32k
 #define	SYS_RAM_SIZE			0x8000
#endif

#define TX_BUFFER_EMPTY				0x80000000

#define ITCM_SIZE					0x8000
#define DTCM_SIZE					0x10000

#define ITCM_BASE					0x0
#define DTCM_BASE					0x400000

#define WATCHDOG_COUNT				0x5F5E100 // If Sysbus running at 80Mhz/100,000,000 = 800ms
#define WATCHDOG_REFRESH			0x5 // Refresh watchdog every 5ms (160 opportunities to reset!)

#define PAST_MONITOR_RESET_MUTEX	95	// we use the Test and Set mutex lock to stop multiple CPUs resetting the monitor history



/* ------ MEMORY ALLOCATIONS (NB: any changes must also be applied to globals.s) ------ */ 

// This section only required for the PCB poker stuff
#ifdef SPINNAKERTESTCHIP			// testchip has 16KB of SysRAM
 #define POKER_IMAGE_SIZE			0x3B00	// around 15KB is allocated for the PCB poker board image
#endif
#ifdef SPINNAKER2					// spinnaker2 doubles the SysRAM size to 32k
 #define POKER_IMAGE_SIZE			0x7B00	// just less than 31KB is available for poking directly
#endif
#define POKER_IMAGE_BASE			(SYS_RAM_BASE)

#define COPIER_BASE					(POKER_IMAGE_BASE + POKER_IMAGE_SIZE)		// this is the 'invulnerable' copier routine outside of ITCM	
// This section only required for the PCB poker stuff really
																				
// Space for IP/MAC address parameters
#define ETH_PARAMS_SIZE				0x20 // 32 bytes for IP/MAC addresses
#define ETH_PARAMS_BASE				(SYS_RAM_BASE + SYS_RAM_SIZE - ETH_PARAMS_SIZE)

// Space for flood-filled images
#define FF_SPACE_SIZE				0x404 // 1K of data + 4 byte CRC
#define FF_SPACE_BASE				(ETH_PARAMS_BASE - FF_SPACE_SIZE)
//#define FF_SPACE_BASE				0x60000000

// 4 word mailbox common to all fascicles
#define	MAILBOX_SIZE				(0x4 * sizeof(int))
#define MAILBOX_BASE				(FF_SPACE_BASE - MAILBOX_SIZE)

// Record of processors that have been the monitor
#define MONITOR_HISTORY				(MAILBOX_BASE - sizeof(int))

// At boot the SDRAM is probed and size detected and errors spotted by sampling at 2^x word addresses. 
// 1st word == SDRAM memory size (bytes). 2nd word - errors detected bits [28:1] error at word (2^bitid).  bit 30/31=intense low/high fail.
#define DETECTED_SDRAM_INFO_SIZE	(0x2 * sizeof(int))
#define DETECTED_SDRAM_INFO_BASE	(MONITOR_HISTORY - DETECTED_SDRAM_INFO_SIZE)

// Populated ChipID - for testboard ethernet attached chip is 0,0 - SpiNNaker2 needs a "numbering packet" to arrive
#define CHIP_ID						(DETECTED_SDRAM_INFO_BASE - sizeof(int))

// Record of failed peripherals in the case of a boot failure
#define FAILURE_LOG_BASE			(CHIP_ID - (sizeof(int)*NUMBEROFCORES))

// Location at top of DTCM where ITCM Validation Block information is stored (for reboot recovery)
#define IVB_SPACE					((32+4+1) * sizeof(int))	// Space to store 32 block CRCs + 4 control words+ 1 magic number
#define ITCM_VALIDATION_BLOCK	    (ITCM_BASE + ITCM_SIZE - IVB_SPACE)	// Place plumb last in ITCM					

// Space in DTCM for composing flood-filled images
#define DTCM_IMAGE_SPACE			(DTCM_BASE + (DTCM_SIZE / 2)) 	// 32KB is reserved for 1st phase flood fill data at end of DTCM



/* ------------------------------- PERIPHERAL REGISTERS ------------------------------- */
#define COMMS_CTRL_BASE				0x10000000

#define COMMS_CTRL_TX_CTRL			(0x0000 / sizeof(int))
#define COMMS_CTRL_RECEIVE_DATA		(0x0010 / sizeof(int))
#define COMMS_CTRL_RECEIVE_KEY		(0x0014 / sizeof(int))
#define COMMS_CTRL_TEST				(0x001C / sizeof(int))
#define COMMS_CTRL_SEND_DATA		(0x0008 / sizeof(int))
#define COMMS_CTRL_SEND_KEY			(0x000C / sizeof(int))

#ifdef SPINNAKERTESTCHIP			// testchip
#define COMMS_CTRL_RX_STATUS		(0x0004 / sizeof(int))
#define COMMS_CTRL_TRANSMIT_DATA	(0x0008 / sizeof(int))
#define COMMS_CTRL_TRANSMIT_KEY		(0x000C / sizeof(int))
#endif

#ifdef SPINNAKER2					// spinnaker2
#define COMMS_CTRL_TRANSMIT_DATA	(0x0004 / sizeof(int))
#define COMMS_CTRL_TRANSMIT_KEY		(0x0008 / sizeof(int))
#define COMMS_CTRL_RX_STATUS		(0x000C / sizeof(int))
#define COMMS_CTRL_SOURCE_ADDR		(0x0018 / sizeof(int))  // new for SpiNNaker2
#endif

#define DMA_CTRL_BASE				0x30000000
#define DMA_CTRL_CRCP				(0x0000 / sizeof(int))  // SpiNNaker2 this is writable
#define DMA_CTRL_ADRS				(0x0004 / sizeof(int))
#define DMA_CTRL_ADRT				(0x0008 / sizeof(int))
#define DMA_CTRL_DESC				(0x000C / sizeof(int))
#define DMA_CTRL_CTRL				(0x0010 / sizeof(int))
#define DMA_CTRL_STAT				(0x0014 / sizeof(int))
#define DMA_CTRL_GTCL				(0x0018 / sizeof(int))
#define DMA_CTRL_CRCSETUPTABLE		(0x0180 / sizeof(int))

#define ETH_MII_BASE				0xF4000000
#define ETH_MII_REG_BASE			ETH_MII_BASE+0xC000
#define ETH_MII_RX_FRAME_BUFFER		ETH_MII_BASE+0x4000
#define ETH_MII_TX_FRAME_BUFFER		(0x0000 / sizeof(int))
#define ETH_MII_GENERAL_CMD			(0xC000 / sizeof(int))
#define ETH_MII_GENERAL_STATUS		(0xC004 / sizeof(int))
#define ETH_MII_TX_LENGTH			(0xC008 / sizeof(int))
#define ETH_MII_TX_COMMAND			(0xC00C / sizeof(int))
#define ETH_MII_RX_COMMAND			(0xC010 / sizeof(int))
#define ETH_MII_MAC_ADDR_LS			(0xC014 / sizeof(int))
#define ETH_MII_MAC_ADDR_HS			(0xC018 / sizeof(int))
#define ETH_MII_PHY_CTRL			(0xC01C / sizeof(int))
#define ETH_MII_INT_CLR				(0xC020 / sizeof(int))
#define ETH_MII_RX_BUF_RD_PTR		(0xC024 / sizeof(int))
#define ETH_MII_RX_DSC_RD_PTR		(0xC02C / sizeof(int))
#define ETH_MII_RX_DESC_RAM			(0x8000 / sizeof(int))

// Phy registers are not memory mapped and must be read through phy_read(reg)
#define PHY_CTRL_REG				0x00
#define PHY_AUTO_NEG_ADVERT			0x04
#define PHY_INTERRUPT_SOURCE		0x1D
#define PHY_INTERRUPT_MASK			0x1E

#define PL340_BASE					0xF0000000
#define MC_CMD  					(0x04 / sizeof(int))
#define MC_DIRC 					(0x08 / sizeof(int))
#define MC_MCFG						(0x0C / sizeof(int))
#define	MC_REFP						(0x10 / sizeof(int))
#define MC_CASL						(0x14 / sizeof(int))
#define MC_CCFG0					(0x200 / sizeof(int))
#define DLL_CONFIG0					(0x304 / sizeof(int))
#define DLL_CONFIG1					(0x308 / sizeof(int))

#define ROUTER_BASE					0xF1000000
#define ROUTER_CTRL					(0x0000 / sizeof(int))
#define ROUTER_DIAG_COUNT			(0x0300 / sizeof(int))
#define ROUTER_MC_ROUTE_TABLE		(0x4000 / sizeof(int))
#define ROUTER_MC_KEY_TABLE			(0x8000 / sizeof(int))
#define ROUTER_MC_MASK_TABLE		(0xC000 / sizeof(int))
#define ROUTER_P2P_TABLE			(0x10000 / sizeof(int))

#define SYS_CTRL_BASE				0xF2000000
#define SYS_CTRL_CPU_DISABLE		(0x0004 / sizeof(int))
#define SYS_CTRL_SET_CPU_IRQ		(0x0008 / sizeof(int))
#define SYS_CTRL_CLR_CPU_IRQ		(0x000C / sizeof(int))
#define SYS_CTRL_SET_CPU_OK			(0x0010 / sizeof(int))
#define SYS_CTRL_CLR_CPU_OK			(0x0014 / sizeof(int))
#define SYS_CTRL_SOFT_RESET 		(0x0018	/ sizeof(int))
#define SYS_CTRL_RESET_CODE			(0x0030 / sizeof(int))
#define SYS_CTRL_MONITOR_ID			(0x0034 / sizeof(int))
#define SYS_CTRL_MISC_CTRL  		(0x0038 / sizeof(int))
#define SYS_CTRL_CLK_MUX_CTRL		(0x0060 / sizeof(int))
#define SYS_CTRL_ARBIT_BASE			(0x0080 / sizeof(int))
#define SYS_CTRL_TEST_SET_BASE		(0x0100 / sizeof(int))
#define SYS_CTRL_TEST_CLR_BASE		(0x0180 / sizeof(int))
#define SYS_CTRL_SET_CPU_IRQ		(0x0008 / sizeof(int))

#define SYSCTRL_SOFT_RESET 			0xf2000018	// CPU soft reset reg
#define SYSCTRL_MONITOR_ID 			0xf2000034	// Monitor Processor ID register
#define SYSCTRL_MISC_CTRL  			0xf2000038	// Misc control reg

#define TIMER_BASE					0x11000000
#define TIMER_1_LOAD				(0x0000 / sizeof(int))
#define TIMER_1_CTRL				(0x0008 / sizeof(int))
#define TIMER_1_INT_CLR				(0x000C / sizeof(int))
#define TIMER_1_BG_LOAD				(0x0018 / sizeof(int))
#define TIMER_ITCR					(0x0F00 / sizeof(int))

#define VIC_BASE					0xFFFFF000
#define VIC_IRQ_STATUS				(0x0000 / sizeof(int))
#define VIC_INTERRUPT_SELECT		(0x000C / sizeof(int))
#define VIC_ENABLE					(0x0010 / sizeof(int))
#define VIC_ENABLE_CLEAR			(0x0014 / sizeof(int))
#define VIC_VECT_ADDR				(0x0030 / sizeof(int))
#define VIC_DEFAULT_ADDR			(0x0034 / sizeof(int))
#define VIC_ADDR					(0x0100 / sizeof(int))
#define VIC_CTRL					(0x0200 / sizeof(int))

#define WATCHDOG_BASE				0xF3000000
#define WATCHDOG_LOAD				(0x0000 / sizeof(int))
#define WATCHDOG_CTRL				(0x0008 / sizeof(int))
#define WATCHDOG_LOCK				(0x0C00 / sizeof(int))



/* ----------------------------------- RESET TYPES ------------------------------------ */
#define POWER_ON_RESET				0x0
#define	WATCHDOG_RESET				0x1
#define USER_RESET					0x2
#define	ENTIRE_CHIP_RESET 			0x3
#define WATCHDOG_INTERRUPT			0x4



/* ----------------------------------- P2P Routing Directions-------------------------- */
#define EAST						0x0
#define NORTHEAST					0x1
#define NORTH						0x2
#define WEST						0x3
#define SOUTHWEST					0x4
#define SOUTH						0x5
#define SPINN2DROP					0x6
#define P2PMON						0x7
#define STAY						0xFFFF



/* ---------------------------------- FAILURE CODES ----------------------------------- */
#define COMMS_CTRL_FAILURE			0x0
#define DMA_CTRL_FAILURE			0x1
#define DTCM_FAILURE				0x2
#define ETH_MII_FAILURE				0x3
#define ITCM_FAILURE				0x4
#define PL340_FAILURE				0x5
#define ROUTER_FAILURE				0x6
#define SD_RAM_FAILURE				0x8
#define SYS_CTRL_FAILURE			0x8
#define TIMER_FAILURE				0x9
#define VIC_FAILURE					0xA
#define WATCHDOG_FAILURE			0xB
#define UNEXPECTED_EXCEPTION_HIGH	0xC
#define UNEXPECTED_EXCEPTION_LOW	0xD



/* -------------------------------- INTERRUPT SOURCES --------------------------------- */
#define TIMER_IRQ					0x04
#define CC_RX_IRQ					0x06
#define SYS_CTRL_IRQ				0x12
#define ETH_RX_IRQ					0x14

#define TIMER_IRQ_PRIORITY			0x1   	
#define CC_RX_IRQ_PRIORITY			0x2		
#define SYS_CTRL_IRQ_PRIORITY		0x0		
#define ETH_RX_IRQ_PRIORITY			0x3		



/* ------------------------------- DMA RESPONSE CODED --------------------------------- */
#define DMA_OK						0x0			// a non-zero response is an error code
#define DMA_BURST_OFFSET			0x15		// bit 21 in the DMA transfer description
#define DMA_CRC_OFFSET				0x14		// bit 20 in the DMA transfer description
#define DMA_DIRECTION_OFFSET		0x13		// bit 19 in the DMA transfer description



/* ------------------------------ RESOURCES FOR GPIO CONTROL ---------------------------*/
#define GPIO_PORT					(0x40 / 4)
#define GPIO_DIR					(0x44 / 4)
#define GPIO_SET					(0x48 / 4)
#define GPIO_CLR					(0x4c / 4)
#define GPIO_READ					(0x48 / 4)
		
#define LED_0						(1 << 0)
#define LED_1						(1 << 1)
#define LED_2						(1 << 6)
#define LED_3						(1 << 7)

#define LEDS						(LED_0 + LED_1 + LED_2 + LED_3)

#define SER_OUT_0					0x01
#define SER_OUT_1					0x02
#define SER_OUT						(SER_OUT_0 + SER_OUT_1)

#define SER_IN_0					0x40
#define SER_IN_1					0x80
#define SER_IN						(SER_IN_0 + SER_IN_1)


		
/* ------------------------------- FLOOD-FILL CONTROLS -------------------------------- */
#define FF_START					0x1
#define FF_BLOCK_START				0x2
#define	FF_BLOCK_DATA				0x3
#define FF_BLOCK_END				0x4
#define FF_CONTROL					0x5
#define CHIP_NUMBER_CC				0xD		// used for chip numbering in NN
#define ITUBOTRON_CC				0xE		// used to get a debug character back to Ethernet sink point

#define WITH_PAYLOAD				0x1
#define WITHOUT_PAYLOAD				0x0

#define	FF_OFF						0x0
#define FF_ETHERNET					0x1
#define FF_INTER_CHIP				0x2

#define FF_PHASE_1					0x3

#define FF_PACKET_ID(key)			((key & 0xF))
#define FF_WORD_ID(key)				((key >> 0x08) & 0xFF)
#define FF_BLOCK_ID(key)			((key >> 0x10) & 0xFF)
#define FF_OPCODE(key)				((key >> 0x18) & 0xF)
#define FF_CHECKSUM(key)			((key >> 0x1C) & 0xF)
#define	FF_BLOCK_COUNT(payload)		((payload >> 0x10) & 0xFF)
#define FF_TARGET(payload)			((payload >> 0x18) & 0xFF)

#define MAILBOX_OPCODE				0
#define MAILBOX_COPY_SOURCE			1
#define MAILBOX_COPY_DESTINATION	2
#define MAILBOX_COPY_EXECUTION		3

#define	MAX_FF_BLOCKS				0x100
#define MAX_FF_WORDS				0x100
#define MAX_FF_IMAGE_SIZE			0x2000 // Max image size for Ph1 in words CP = 32KB (size of ITCM and image buffer)

#define FF_EXECUTE_COMMAND			0x100

#define FF_EXECUTE					0x100
#define FF_EXECUTE_ETH				0x1



/* ------------------------------- FLOOD-FILL CONTROLS (SENDING) ---------------------- */
#define FF_TARGET_MONITOR			0x0
#define FF_PENDING					0x1
#define FF_ACTIVE					0x2
#define TX_BUFFER_EMPTY				0x80000000
#define TX_BUFFER_FULL				0x40000000
#define LINKS						0x7



/* -------------------------------- ETHERNET CONTROLS --------------------------------- */
#define SPINN_PORT					54321	// we use UDP port 54321 for the SpiNNaker protocol notionally

#define SPINNPROTVERSION			0x1 // Spinnaker Eth Protocol Version (1= 1st release (ROM))
#define IP_ETHERTYPE				0x800
#define IP_VER_LEN_TOS				0x4500		//IP version=4+ip header len=5+ip type of service=0
#define IP_FRAG_FLAGS_TTL_PROT		0x4000FF11  //Frag flags=4 + fragment offset = 0 + IP TTL = 0xFF + UDP Prot = 17 (0x11) UDP

#define PHY_LINK_DOWN(status)		(status & 0x10)
#define PHY_AUTO_NEG_DONE(status)	(status & 0x40)

#define IP(frame)					((frame[3] >> 0x10) == IP_ETHERTYPE)
#define ARP(frame)					((frame[3] >> 0x10) == 0x806)
#define UDP(frame)					((frame[5] & 0xFF) == 0x11)
#define ICMP(frame)					((frame[5] & 0xFF) == 0x1)
#define ICMP_ECHO(frame)			((frame[8] & 0xFF00) == 0x800)
#define	SPINN(frame)				((frame[9] >> 0x10) == SPINN_PORT)
#define SPINN_PROTO_V1(frame)		((frame[10] & 0xFFFF) == SPINNPROTVERSION)

#define SPINN_INSTRCTN(frame)		(frame[11] & 0xFFFF)
#define SPINN_INSTRCTN_OP_1(frame)	(frame[12] & 0xFFFF) 
#define SPINN_INSTRCTN_OP_2(frame)	(frame[13] & 0xFFFF)
#define SPINN_DATA_LENGTH(frame)	(frame[13] & 0xFFFF) // Same as OP_2
#define SPINN_INSTRCTN_OP_3(frame)	(frame[14])
#define SPINN_DATA(frame, i)		(frame[15 + i])

#define ETH_MII_TX_ACTIVE			0x1

#define HELLO						65
#define	ITUBOTRON					99

#define destIPAt					30
#define spinnakerProtocolVersionAt	42  //byte 12-13
#define spinnakerInstructionAt		44  //byte 14-15	
#define instructionOption1At		48  //byte 16-19
#define instructionOption2At		52  //byte 20-23
#define dataLengthAt				52  //now same as instructionOption2At
#define instructionOption3At	   	56  //byte 24-27
#define spinnakerDataAt				60



/* ------------------------------- Peripheral Inits setup ------------------------------*/
#define CC_FAKE_ROUTE_INIT			0x07000000
#define DMA_GLOBAL_CTRL_INIT		0x80000000
#define ETHERNET_MODE_INIT			0x7B			// TX/RX/ErrorFilter on, UC/MC/BC listen
#define PHY_AUTO_NEG_100FD_INIT		0x101			// Force PHY to advertise 100Mb/s Full duplex only & Selector Field IEEE803.3
#define PHY_RESTART_AUTONEG			0x1200			// Enable Auto-negotiate & restart autonegotiation
#define ROUTER_CTRL_INIT			0x2F2F0007		// Wait1 (before ER) and Wait2 (Er before drop) to 2F=108 cycles each
													// enable routing, and dump/error interrupt generation + set myprocID as mon 
#define TIMER1_LOAD_INIT			0x27100;		// =160000 count down each clock cycle = gives 1000/second @ 160Mhz 
#define TIMER1_CTRL_INIT			0xE2			// enable, periodic, interrupt. Divide by 1, 32bit wrapping mode.

#define VIC_CTRL_INTERRUPT_ON		0x20			// bit position 5 set high in the vector control register turns it on
#define PHY_EXISTENCE_TEST_BITS		0x4080			// bits 7 and 14 are set (loopback & coll test bits)
#define PHY_READ_COMMAND			0x60000000		// set command as 0b0110 (start of frame 0b01, and OPcode 0b10
#define PHY_WRITE_COMMAND			0x50000000		// set command as 0b0101 (start of frame 0b01, and OPcode 0b01

#ifdef SPINNAKERTESTCHIP
	#define CC_RX_PACKET			0x80000000		// Packet received bit 31.
	#define CC_TX_NN_DIRECT			0x07a00000		// set fake route 0b111 (self) and NN packet Direct Bit Set
#endif
#ifdef SPINNAKER2
	#define CC_RX_PACKET			0x90000000		// Packet received bit 31. SpiNN has bit28 = error free.
	#define CC_TX_NN_DIRECT			0x00a00000		// Set NN packet with Direct Bit Set
#endif
#define CC_ERR_PACKET				0x60000000		// error received bit 30=parity and bit 29=framing.

#define ETHERNET_CLR_RX_INTERRUPT	0x10			// clear a Ethernet Receive interrupt
#define MAILBOX_FASCICLE_COPY		0x1				// indicates that we should copy data from SystemRAM block to image (not execute) 
#define MAILBOX_FASCICLE_EXECUTE	0x2				// indicates that we should copy image into ITCM and execute



/* ----------------------------- MISCELLANEOUS MACROS --------------------------------- */
#ifdef SPINNAKERTESTCHIP			// testchip
#define NN_PACKET_PAYLOAD(route)	(0x07820000 | (route << 18)) 
#define NN_PACKET_NOPAYLOAD(route)	(0x07800000 | (route << 18)) 
#endif

#ifdef SPINNAKER2					// spinnaker2
#define NN_PACKET_PAYLOAD(route)	(0x00820000 | (route << 18))   	// SPINN2 route&src moved to SAR (init as fake rte = 0b111)
#define NN_PACKET_NOPAYLOAD(route)	(0x00800000 | (route << 18))   	// SPINN2 route&src moved to SAR (init as fake rte = 0b111)
#endif

#define VALID_NN_PACKET(rx_status)	((rx_status & 0xC00000)==0x800000) && !(rx_status & 0x60000000) // check noerrs rcvd & begins 0b10...
#define NN_PAYLOAD(rx_status)		((rx_status & 0x020000)>0) 		// check whether the payload bit is set

#define monitor						(processor_ID == (SYS_CTRL[SYS_CTRL_MONITOR_ID & 0x1F]))
#define phy_present					(SYS_CTRL[SYS_CTRL_SET_CPU_OK]>>31)		// 31st bit of CPU OK register indicates presence of Eth
#define processor_ID 				(DMA_CTRL[DMA_CTRL_STAT] >> 24)			// read my own physical processor ID (from my DMA ctrl)



/* --------------------------------- STATUS VARIABLES ----------------------------------*/
extern unsigned int time; 				// Time since boot in milliseconds
extern unsigned int LED_FLASH_INTERVAL;			// frequency of test board LED flashing
extern unsigned int last_flash;

extern const unsigned int SYS_CTRL_MUX;		// used for clock source assignment and settings

extern unsigned int rx_frame[380];	//to hold 1500 bytes for the frame to receive
extern unsigned int tx_frame[380];
extern unsigned int sourceMACAddressLS;
extern unsigned int sourceMACAddressHS;
extern unsigned int spinnakerDataSize;
extern unsigned int destinationMACAddressLS;   
extern unsigned int destinationMACAddressHS;       
extern unsigned int macDestAddressConfigured;
extern unsigned int sourceIPAddress;
extern unsigned int FFTarget;
extern unsigned int rxFrameSize;
extern unsigned short EthPacketID;
extern unsigned int destinationIPAddress;

// CP - these used for debugging via itubotron
extern unsigned int itubotrondata[256];
extern unsigned int itubotroncount;
extern unsigned short itubotrondataoverflow;



#endif
