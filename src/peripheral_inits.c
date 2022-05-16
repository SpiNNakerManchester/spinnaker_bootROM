/*****************************************************************************************
*	peripheral_inits.c - functions in this file are called from boot_processor() in main.c
*	to initialise the various chip peripherals.
*
*	Created by Thomas Sharp, modified by Cameron Patterson  
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/



#include "../inc/globals.h"
#include "../inc/interrupt_service_routines.h"
#include "../inc/phy.h"
#include "../inc/dma.h"

#include <stdio.h> // To enable printing during debug


// init_comms_ctrl() is called by boot_processor in main.c, clears any CC errors, and sets all 1s "route" for self-genned packets 
void init_comms_ctrl()
{
	pointer COMMS_CTRL = (unsigned int *) COMMS_CTRL_BASE;

	#ifdef SPINNAKERTESTCHIP										// testchip
		COMMS_CTRL[COMMS_CTRL_TX_CTRL] = CC_FAKE_ROUTE_INIT; 
	#endif

	#ifdef SPINNAKER2												// spinnaker2
		COMMS_CTRL[COMMS_CTRL_SOURCE_ADDR] = CC_FAKE_ROUTE_INIT; 	// SPINN2 route is in new SAR
	#endif
}


/* init_DMA_ctrl() populates the CRC table for SpiNNaker2, and required to check data during transfers. */
void init_DMA_ctrl()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	DMA_CTRL[DMA_CTRL_GTCL] = DMA_GLOBAL_CTRL_INIT;		// init DMA controller with slow clock, no interrupts or writebuffer
	DMA_CTRL[DMA_CTRL_CTRL] = DMA_CLEAR_ERRORS;  		// clear any residual errors by restarting the controller
	
	#ifdef SPINNAKER2	// CRC setup is only required on the Spinnaker2 chip. The testchip is pre-defined CRC32 in h/w.
		setup_crc32_spinnaker2();						// Bits X^31->X^0. X^32==1 (implicit). MGrymel SpiNNaker Progr CRC 12/Mar/2010.
		// DMA_CTRL[DMA_CTRL_CRCP] = 0x82608EDB;  		// CRC32 per SpiNNaker test chip datasheet 1.0. Depreciated per MG 05/May/2010.	   
	#endif
}

// init_ethernet() is called my the mon when phy detected. Gets own MAC/IP addresses from serial ROM, and initialised required features.
void init_ethernet_MII()
{
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;
	pointer ETH_PARAMS = (unsigned int*) ETH_PARAMS_BASE;
	
	printf("MIInit1\n");	//!!! TLM !!!
	sourceMACAddressHS = (ETH_PARAMS[0] >> 24) | ((ETH_PARAMS[0] & 0xff0000) >> 8);		// Read MSBytes of MAC Addr from BootROM block
	printf("MIInit2\n");	//!!! TLM !!!
	sourceMACAddressLS = (ETH_PARAMS[1] >> 24) | ((ETH_PARAMS[1] & 0xff0000) >> 8) | 	// Read LSBytes of MAC Addr from BootROM block
							((ETH_PARAMS[1] & 0xff00) << 8) | (ETH_PARAMS[1] << 24);	
	printf("MIInit3\n");	//!!! TLM !!!
	sourceIPAddress = (ETH_PARAMS[2] >> 24) | ((ETH_PARAMS[2] & 0xff0000) >> 8) | 		// Read my IP from BootROM block
							((ETH_PARAMS[2] & 0xff00) << 8) | (ETH_PARAMS[2] << 24);	
	printf("MIInit4\n");	//!!! TLM !!!
	destinationIPAddress = 0xFFFFFFFF;		// start with broadcast destination until server address learned
	destinationMACAddressHS = 0xFFFF;		// start with broadcast destination until server address learned
	destinationMACAddressLS = 0xFFFFFFFF;   // start with broadcast destination until server address learned                        

	ETH_MII[ETH_MII_GENERAL_CMD] = 0x0;													// setup Ethernet MII as off for setup.
	printf("MIInit5\n");	//!!! TLM !!!
	ETH_MII[ETH_MII_MAC_ADDR_LS] = sourceMACAddressLS;									// Setup MII with my MAC
	printf("MIInit6\n");	//!!! TLM !!!
	ETH_MII[ETH_MII_MAC_ADDR_HS] = sourceMACAddressHS;									// Setup MII with my MAC
	printf("MIInit7\n");	//!!! TLM !!!
	
	while(ETH_MII[ETH_MII_GENERAL_STATUS]&0x7E) ETH_MII[ETH_MII_RX_COMMAND] = 0xF00;	// empty the Eth RX buffer
	ETH_MII[ETH_MII_GENERAL_CMD] = ETHERNET_MODE_INIT&0xFC;								// set mode without enabling RX/TX
	printf("MIInit8\n");	//!!! TLM !!!
	ETH_MII[ETH_MII_GENERAL_CMD] = ETHERNET_MODE_INIT;									// TX/RX/ErrorFilter on, UC/MC/BC listen
	printf("MIInit9\n");	//!!! TLM !!!
}


// init_phy() is called by the monitor processor when a phy is detected locally, and initialises the PHY mode
void init_phy()
{
	// phy_write(PHY_AUTO_NEG_ADVERT, 0x61); 					// A fix to accommodate auto-neg failure, set full duplex 10meg
	// phy_write(PHY_INTERRUPT_MASK, 0x50); 					// Enable auto-neg complete & link down interrupts (not reqd in Bootrom)
	phy_write(PHY_AUTO_NEG_ADVERT, PHY_AUTO_NEG_100FD_INIT);	// Force 100Mbit/s Full-Duplex IEEE802.3 autonegotiate
	phy_write(PHY_CTRL_REG, PHY_RESTART_AUTONEG); 				// Enable and Restart autonegotiation (with these parameters)	
}


/* init_router() is called by the monitor processor from boot_processor in main.c. It sets
 * the monitor processor ID along with wait values in the router controller and enables
 * routing and interrupts. Finally, the routing tables are initialised empty. */
void init_router()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer ROUTER = (unsigned int*) ROUTER_BASE;
	int i;
	
	ROUTER[ROUTER_CTRL] = (processor_ID << 0x8) | ROUTER_CTRL_INIT;	// Wait1 (before ER) & Wait2 (Er before drop) to 2f=108 cycles each

	printf("MCI \n");	//!!! TLM !!!
	for(i = 0; i < ROUTER_MC_TABLE_SIZE; i++)	{					// initialise the MC routing table
	    if((i%64)==0) printf("%x,",i);  //!!! TLM !!!
		ROUTER[ROUTER_MC_KEY_TABLE + i] = 0xFFFFFFFF;				// key will only match this packet (as mustn't be 0!)
		ROUTER[ROUTER_MC_MASK_TABLE + i] = 0x0;						// mask of all 0s means no match as:  (key&mask) != key
		ROUTER[ROUTER_MC_ROUTE_TABLE + i] = 0x0;					// this vector is not important, but set to zero (no links/cores)
	}
	printf("\nP2PI \n");	//!!! TLM !!!
	for(i = 0; i < ROUTER_P2P_TABLE_SIZE; i++)	{					// initialise the P2P routing table
	    if((i%256)==0) printf("%x,",i);  //!!! TLM !!!	
		ROUTER[ROUTER_P2P_TABLE + i] = ((6<<21)|(6<<18)|(6<<15)|(6<<12)|(6<<9)|(6<<6)|(6<<3)|6);   // 24bit table 8*3bit outputs.  
	}																// 0b110 = 6 = drop on SpiNN2.  On test chip routes to MonProc
	printf("\nROK\n");	//!!! TLM !!!
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
	TIMER[TIMER_1_LOAD] = TIMER1_LOAD_INIT;							// =160000 count down each clock cycle = gives 1000/second @ 160Mhz 
	TIMER[TIMER_1_BG_LOAD] = TIMER1_LOAD_INIT;						// as above, clear any current clock counter
	if (monitor) TIMER[TIMER_1_CTRL] = TIMER1_CTRL_INIT;			// enable, periodic, interrupt. Divide by 1, 32bit wrapping mode
}

/* init_watchdog() is called by the monitor processor from boot_processor in main.c, sets 
 * the value from which the watchdog should count down and then enables it using the
 * control register. */
void init_watchdog()
{
  #ifdef WATCHDOGON
	pointer WATCHDOG = (unsigned int*) WATCHDOG_BASE;				
	WATCHDOG[WATCHDOG_LOCK] = 0x1ACCE551;							// dis-able lock, so have write access to registers
	WATCHDOG[WATCHDOG_CTRL] = 0x3;									// Enable watchdog counter/interrupt and reset
	WATCHDOG[WATCHDOG_LOAD] = WATCHDOG_COUNT;						// set the counter to the predetermined count
	WATCHDOG[WATCHDOG_LOCK] = 0x0;									// re-enable the lock to protect the registers
  #endif 
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
	uint i;
	
	for (i=0;i<16;i++) {												// clear vector table
		VIC[VIC_CTRL + i] = 0x1F;										// disable all entries in VIC, associating with null intrpt ID31 
		VIC[VIC_ADDR + i] = 0;											// associate all interrupt vector addresses to #0 (reset vector)
	}  
	VIC[VIC_INTERRUPT_SELECT] = 0; 										// set all IRQs to be _NOT_ FIQed (not using FIQ in ROM)
	VIC[VIC_ENABLE_CLEAR] = 0xFFFFFFFF; 								// clear all interrupt bits (turn off all interrupt lines)
	
	
	VIC[VIC_ADDR+SYS_CTRL_IRQ_PRIORITY] = (unsigned int) sys_ctrl_isr;			// set up the sys ctrl int vector to call sys_ctrl_isr
	VIC[VIC_CTRL+SYS_CTRL_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | SYS_CTRL_IRQ;	// Enable & set up to act on SystemCtrl itrpt line ID 
	irq_enable |= 0x1 << SYS_CTRL_IRQ;											// add this IRQ to the enable list
	
	if(monitor)																	// if monitor I need to listen to additional interrupts
	{
		VIC[VIC_ADDR + CC_RX_IRQ_PRIORITY] = (unsigned int) cc_rx_isr;			// set up the comms ctrl rx vector to call cc_rx_isr
		VIC[VIC_CTRL + CC_RX_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | CC_RX_IRQ;	// Enable & set up to act on CommsRX itrpt line ID
		irq_enable |= (0x1 << CC_RX_IRQ);										// add this IRQ to the enable list
		VIC[VIC_ADDR + TIMER_IRQ_PRIORITY] = (unsigned int) timer_isr;			// set up the timer int vector to call timer_isr
		VIC[VIC_CTRL + TIMER_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | TIMER_IRQ;	// Enable & set up to act on Timer interrupt line ID
		irq_enable |= (0x1 << TIMER_IRQ);										// add this IRQ to the enable list
				
		if(phy_present)
		{
			VIC[VIC_ADDR + ETH_RX_IRQ_PRIORITY] = (unsigned int) eth_rx_isr;	// set up the Eth RX int vector to call sys_ctrl_isr
			VIC[VIC_CTRL + ETH_RX_IRQ_PRIORITY] = VIC_CTRL_INTERRUPT_ON | ETH_RX_IRQ;	// Enable & set up EthRX interrupt line ID
			irq_enable |= 0x1 << ETH_RX_IRQ;									// add this IRQ to the enable list
		}
	}
	if(DEBUGPRINTF && phy_present) printf("v1 Enabling IRQ with: 0x%x\n",irq_enable);
	
	VIC[VIC_ENABLE] = irq_enable;										// Enable the VIC controller with required Interrupt sources
}
