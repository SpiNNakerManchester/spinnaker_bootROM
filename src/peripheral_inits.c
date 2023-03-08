/*****************************************************************************************
*	peripheral_inits.c - functions in this file are called from boot_processor() in main.c
*	to initialise the various chip peripherals.
*
*	Created by Thomas Sharp, modified by Cameron Patterson
*
* Copyright (c) 2008 The University of Manchester
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     https://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************************/


#include "../inc/globals.h"
#include "../inc/interrupt_service_routines.h"
#include "../inc/phy.h"




#ifdef QUIMONDA
// Quimonda 133MHz SDRAM - 256Mb - 32MB
// Config = 0x00018009
// Refresh = 2080 (15.6us / 7.5ns)
const uint pl340_data[] =
{
	0x00000006,	// [0]  MC_CASL CAS latency =3
	0x00000001,	// [1]  MC_DQSS T_dqss
	0x00000002,	// [2]  MC_MRD  T_mrd
	0x00000006,	// [3]  MC_RAS  T_ras
	0x00000009,	// [4]  MC_RC   T_rc
	0x00000003,	// [5]  MC_RCD  T_rcd
	0x0000007a,	// [6]  MC_RFC  T_rfc
	0x00000003,	// [7]  MC_RP   T_rp
	0x00000002,	// [8]  MC_RRD  T_rrd
	0x00000002,	// [9]  MC_WR   T_wr
	0x00000001,	// [10] MC_WTR  T_wtr
	0x00000002,	// [11] MC_XP   T_xp
	0x00000010,	// [12] MC_XSR  T_xsr
	0x00000014,	// [13] MC_ESR  T_esr

	0x00018009, // [14] Memory_Cfg register (burst=8)
	2080 // [15] Refresh period
};

#endif

#ifndef QUIMONDA
// Micron 130MHz SDRAM - 1Gb - 128MB
// Config = 0x00018012
// Refresh = 1300 (7.8us / 6ns)
const uint pl340_data[] =
{
	0x00000006, // [0]  MC_CASL CAS latency =3
	0x00000001, // [1]  MC_DQSS T_dqss
	0x00000002, // [2]  MC_MRD  T_mrd
	0x00000007, // [3]  MC_RAS  T_ras
	0x0000000A, // [4]  MC_RC   T_rc
	0x00000003, // [5]  MC_RCD  T_rcd
	0x00000275, // [6]  MC_RFC  T_rfc
	0x00000003, // [7]  MC_RP   T_rp
	0x00000002, // [8]  MC_RRD  T_rrd
	0x00000003, // [9]  MC_WR   T_wr
	0x00000001, // [10] MC_WTR  T_wtr
	0x00000005, // [11] MC_XP   T_xp
	0x00000017, // [12] MC_XSR  T_xsr
	0x00000014, // [13] MC_ESR  T_esr

	0x00018012, // [14] Memory_Cfg register (burst=8)
	1000 // [15] Refresh period      // BUGZILLA 53. CP 20/08/2010
};

#endif



// init_comms_ctrl() is called by boot_processor in main.c, clears any CC errors, and sets all 1s "route" for self-genned packets
void init_comms_ctrl()
{
	pointer COMMS_CTRL = (unsigned int *) COMMS_CTRL_BASE;

	#ifdef TEST_CHIP
	COMMS_CTRL[COMMS_CTRL_TX_CTRL] = CC_FAKE_ROUTE_INIT;
	#elif SPINNAKER2
	COMMS_CTRL[COMMS_CTRL_SOURCE_ADDR] = CC_FAKE_ROUTE_INIT;
	#endif
}


/* init_DMA_ctrl() populates the CRC table for SpiNNaker2, and required to check data during transfers. */
void init_DMA_ctrl()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;

	#ifdef SPINNAKER2	// CRC setup is only required on the Spinnaker2 chip. The testchip is pre-defined CRC32 in h/w.
	unsigned int i;

	uint table_setup[32] =
	{
		0xFB808B20, 0x7DC04590, 0xBEE022C8, 0x5F701164, 0x2FB808B2, 0x97DC0459,
		0xB06E890C, 0x58374486, 0xAC1BA243, 0xAD8D5A01, 0xAD462620, 0x56A31310,
		0x2B518988, 0x95A8C4C4, 0xCAD46262, 0x656A3131, 0x493593B8, 0x249AC9DC,
		0x924D64EE, 0xC926B277, 0x9F13D21B, 0xB409622D, 0x21843A36, 0x90C21D1B,
		0x33E185AD, 0x627049F6, 0x313824FB, 0xE31C995D, 0x8A0EC78E, 0xC50763C7,
		0x19033AC3, 0xF7011641
	}; // this is the CRC32 setup for the DMA's CRC controller - source. Martin Grymel: SpiNNaker Programmable CRC March 12, 2010

	for (i = 0; i < 32; i++) DMA_CTRL[DMA_CTRL_CRCSETUPTABLE + i] = table_setup[i];  // loads CRC32 Ethernet polynomial 0x04C11DB7, (data width 32bits).
	#endif

	DMA_CTRL[DMA_CTRL_GTCL] = DMA_GLOBAL_CTRL_INIT;		// disable DMA interrupts
	DMA_CTRL[DMA_CTRL_CTRL] = DMA_CLEAR_ERRORS;  		// clear any residual errors by restarting the controller
}


// init_ethernet() is called my the mon when phy detected. Gets own MAC/IP addresses from serial ROM, and initialised required features.
void init_ethernet_MII()
{
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;
	pointer ETH_PARAMS = (unsigned int*) ETH_PARAMS_BASE;

	sourceMACAddressHS = (ETH_PARAMS[0] >> 24) | ((ETH_PARAMS[0] & 0xff0000) >> 8);		// Read MSBytes of MAC Addr from BootROM block
	sourceMACAddressLS = (ETH_PARAMS[1] >> 24) | ((ETH_PARAMS[1] & 0xff0000) >> 8) | 	// Read LSBytes of MAC Addr from BootROM block
							((ETH_PARAMS[1] & 0xff00) << 8) | (ETH_PARAMS[1] << 24);
	sourceIPAddress = (ETH_PARAMS[2] >> 24) | ((ETH_PARAMS[2] & 0xff0000) >> 8) | 		// Read my IP from BootROM block
							((ETH_PARAMS[2] & 0xff00) << 8) | (ETH_PARAMS[2] << 24);
	destinationIPAddress = 0xFFFFFFFF;		// start with broadcast destination until server address learned
	destinationMACAddressHS = 0xFFFF;		// start with broadcast destination until server address learned
	destinationMACAddressLS = 0xFFFFFFFF;   // start with broadcast destination until server address learned

	ETH_MII[ETH_MII_GENERAL_CMD] = 0x0;													// setup Ethernet MII as off for setup.
	ETH_MII[ETH_MII_MAC_ADDR_LS] = sourceMACAddressLS;									// Setup MII with my MAC
	ETH_MII[ETH_MII_MAC_ADDR_HS] = sourceMACAddressHS;									// Setup MII with my MAC

	while(ETH_MII[ETH_MII_GENERAL_STATUS]&0x7E) ETH_MII[ETH_MII_RX_COMMAND] = 0xF00;	// empty the Eth RX buffer
	ETH_MII[ETH_MII_INT_CLR] = ETHERNET_CLR_RX_INTERRUPT;								// Clear IRQ
	ETH_MII[ETH_MII_GENERAL_CMD] = ETHERNET_MODE_INIT & 0xFC;								// set mode without enabling RX/TX
	ETH_MII[ETH_MII_GENERAL_CMD] = ETHERNET_MODE_INIT;									// TX/RX/ErrorFilter on, UC/MC/BC listen
}


// init_phy() is called by the monitor processor when a phy is detected locally, and initialises the PHY mode
void init_phy()
{
	// phy_write(PHY_AUTO_NEG_ADVERT, 0x61); 					// A fix to accommodate auto-neg failure, set full duplex 10meg
	// phy_write(PHY_INTERRUPT_MASK, 0x50); 					// Enable auto-neg complete & link down interrupts (not reqd in Bootrom)
	phy_write(PHY_AUTO_NEG_ADVERT, PHY_AUTO_NEG_100FD_INIT);	// Force 100Mbit/s Full-Duplex IEEE802.3 autonegotiate
	phy_write(PHY_CTRL_REG, PHY_RESTART_AUTONEG); 				// Enable and Restart autonegotiation (with these parameters)
}


void init_PL340()
{
	volatile uint* pl340 = (uint*) PL340_BASE;
	int i;

	pl340[MC_CMD] = 0x00000003;		// Pause PL340 Controller
	pl340[MC_CMD] = 0x00000004;		// Enter PL340 Config mode

	for (i = 0; i < 14; i++)
		pl340[MC_CASL + i] = pl340_data[i];	// populate CAS_latency through to t_esr

	pl340[MC_MCFG] = pl340_data[14];	// Init memory_cfg reg

	pl340[MC_REFP] = pl340_data[15];	// Init refresh_prd reg

	pl340[MC_CCFG0] = 0x000060e0;	// Set config reg

	pl340[MC_DIRC] = 0x000C0000;	// NOP
	pl340[MC_DIRC] = 0x00000000;	// PRECHARGEALL
	pl340[MC_DIRC] = 0x00040000;	// AUTOREFRESH
	pl340[MC_DIRC] = 0x00040000;	// AUTOREFRESH
	pl340[MC_DIRC] = 0x00080033;	// MODEREG (burst=8)
	pl340[MC_DIRC] = 0x000A0000;	// EXTMODEREG
	pl340[MC_CMD] = 0x00000000;		// Enable memory
	pl340[DLL_CONFIG1] = 0x00000000;	// Clear fine-tune (user_config1)
	pl340[DLL_CONFIG0] = 0x01000000;	// Enable DLL (user_config0)
}


/* init_router() is called by the monitor processor from boot_processor in main.c. It sets
 * the monitor processor ID along with wait values in the router controller and enables
 * routing and interrupts. Finally, the routing tables are initialised empty. */
void init_router()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer ROUTER = (unsigned int*) ROUTER_BASE;
	int i;

	ROUTER[ROUTER_CTRL] = (PROCESSOR_ID << 0x8) | ROUTER_CTRL_INIT;	// Wait1 (before ER) & Wait2 (Er before drop) to 2f=108 cycles each

	for(i = 0; i < ROUTER_MC_TABLE_SIZE; i++)	{					// initialise the MC routing table
		ROUTER[ROUTER_MC_KEY_TABLE + i] = 0xFFFFFFFF;				// key will only match this packet (as mustn't be 0!)
		ROUTER[ROUTER_MC_MASK_TABLE + i] = 0x0;						// mask of all 0s means no match as:  (key&mask) != key
		ROUTER[ROUTER_MC_ROUTE_TABLE + i] = 0x0;					// this vector is not important, but set to zero (no links/cores)
	}
	for(i = 0; i < ROUTER_P2P_TABLE_SIZE; i++)	{					// initialise the P2P routing table
		ROUTER[ROUTER_P2P_TABLE + i] = ((6<<21)|(6<<18)|(6<<15)|(6<<12)|(6<<9)|(6<<6)|(6<<3)|6);   // 24bit table 8*3bit outputs.
	}																// 0b110 = 6 = drop on SpiNN2.  On test chip routes to MonProc
}


/* init_timer() is called by all processors (CP 25/May/2010) from boot_processor in main.c. It is
 * configured using the control register to run periodically using a 32 bit counter and to
 * interrupt every 1ms (given a clock frequency of 160MHz, set up in start.s).
 * Non-Monitor processors have their clock initialised, but turned off. (CP) */
void init_timer()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer TIMER = (unsigned int*) TIMER_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;

	TIMER[TIMER_1_CTRL] = 0x62;										// Stop current timer and set up control register.
	TIMER[TIMER_1_INT_CLR] = 0x1;  									// Clear any current timer interrupt
	TIMER[TIMER_1_LOAD] = TIMER1_LOAD_INIT;							// =160000 count down each clock cycle = gives 1000 per second @ 160Mhz
	TIMER[TIMER_1_BG_LOAD] = TIMER1_LOAD_INIT;						// as above, clear any current clock counter
	if (MONITOR) TIMER[TIMER_1_CTRL] = TIMER1_CTRL_INIT;			// enable, periodic, interrupt. Divide by 1, 32bit wrapping mode
}


/* init_watchdog() is called by the monitor processor from boot_processor in main.c, sets
 * the value from which the watchdog should count down and then enables it using the
 * control register. */
void init_watchdog()
{
	pointer WATCHDOG = (unsigned int*) WATCHDOG_BASE;

	WATCHDOG[WATCHDOG_LOCK] = 0x1ACCE551;							// dis-able lock, so have write access to registers
	WATCHDOG[WATCHDOG_CTRL] = 0x3;									// Enable watchdog counter/interrupt and reset
	WATCHDOG[WATCHDOG_LOAD] = WATCHDOG_COUNT;						// set the counter to the predetermined count
	WATCHDOG[WATCHDOG_LOCK] = 0x0;									// re-enable the lock to protect the registers
}


/* init_VIC() is called by boot_processor in main.c and is responsible for populating the
 * associative memory of the Vectored Interrupt Controller and enabling the corresponding
 * interrupt sources. */
void init_VIC()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer VIC = (unsigned int*) VIC_BASE;
	unsigned int irq_enable = 0x0;
	unsigned int i;

	for(i = 0; i < 16; i++)													// clear vector table
	{
		VIC[VIC_CTRL + i] = 0x1F;										// disable all entries in VIC, associating with null intrpt ID31
		VIC[VIC_ADDR + i] = 0;											// associate all interrupt vector addresses to #0 (reset vector)
	}

	VIC[VIC_INTERRUPT_SELECT] = 0; 										// set all IRQs to be _NOT_ FIQed (not using FIQ in ROM)
	VIC[VIC_ENABLE_CLEAR] = 0xFFFFFFFF; 								// clear all interrupt bits (turn off all interrupt lines)

	VIC[VIC_ADDR + SYS_CTRL_IRQ_PRIORITY] = (unsigned int) sys_ctrl_isr;			// set up the sys ctrl int vector to call sys_ctrl_isr
	VIC[VIC_CTRL + SYS_CTRL_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | SYS_CTRL_IRQ;	// Enable & set up to act on SystemCtrl itrpt line ID
	irq_enable |= 0x1 << SYS_CTRL_IRQ;											// add this IRQ to the enable list

	if(MONITOR)																	// if monitor I need to listen to additional interrupts
	{
		VIC[VIC_ADDR + CC_RX_IRQ_PRIORITY] = (unsigned int) cc_rx_isr;			// set up the comms ctrl rx vector to call cc_rx_isr
		VIC[VIC_CTRL + CC_RX_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | CC_RX_IRQ;	// Enable & set up to act on CommsRX itrpt line ID
		irq_enable |= (0x1 << CC_RX_IRQ);										// add this IRQ to the enable list
		VIC[VIC_ADDR + TIMER_IRQ_PRIORITY] = (unsigned int) timer_isr;			// set up the timer int vector to call timer_isr
		VIC[VIC_CTRL + TIMER_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | TIMER_IRQ;	// Enable & set up to act on Timer interrupt line ID
		irq_enable |= 0x1 << TIMER_IRQ;										// add this IRQ to the enable list

		if(PHY_PRESENT)
		{
			VIC[VIC_ADDR + ETH_RX_IRQ_PRIORITY] = (unsigned int) eth_rx_isr;	// set up the Eth RX int vector to call sys_ctrl_isr
			VIC[VIC_CTRL + ETH_RX_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | ETH_RX_IRQ;	// Enable & set up EthRX interrupt line ID
			irq_enable |= 0x1 << ETH_RX_IRQ;									// add this IRQ to the enable list
		}
	}

	VIC[VIC_ENABLE] = irq_enable;										// Enable the VIC controller with required Interrupt sources
}
