/*****************************************************************************************
*	Created by Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/
#include "../inc/globals.h"

/* ---------------------------- STATUS VARIABLES -----------------------------*/
//unsigned int phy_present = 0; 			// now a macro =1 implies ethernet phy exists established, 0 no PHY chip
//unsigned int monitor = 0;				// now a macro
//unsigned int processor_ID = 0;		// now a macro, Processor ID on chip
unsigned int time = 0;				// Time since boot in milliseconds




/*#ifdef SPINNAKERTESTCHIP			// testchip
 const unsigned int SYS_CTRL_MUX = 0x50000165;		// setup clock mux for testchip.  Div Sys AHB clk by 2. Invert odd CPU clk.
 													//  Sys/Rtr=PLL1, Mem=PLL2, Proc Odd/Even=PLL1 
#endif
#ifdef SPINNAKER2					// spinnaker2
 const unsigned int SYS_CTRL_MUX = 0x80508821;		// setup clock mux for SpiNNaker2 chip. Invert odd CPU clk. 
 													//  Sys AHB clk=2/PLL1. Router=1/PLL1.  Mem=1/PLL2. Odd=1/PLL1. Even=1/PLL1. 	
#endif
*/


//
unsigned int rx_frame[380];	//to hold 1500 bytes for the frame to receive
unsigned int tx_frame[380];

// All the below network data now is overwritten from Serial ROM, but variables do need to be defined here
unsigned int destinationMACAddressLS=0x0;  	
unsigned int destinationMACAddressHS=0x0;  	       
unsigned int sourceMACAddressLS=0x0;
unsigned int sourceMACAddressHS=0x0;
unsigned int sourceIPAddress=0x0;	
unsigned int destinationIPAddress=0x0; 

unsigned int spinnakerDataSize;
unsigned int macDestAddressConfigured=0;
unsigned int FFTarget=0;
unsigned int rxFrameSize=0;
unsigned short EthPacketID=0;

// CP - these used for debugging via itubotron
unsigned int itubotrondata[256];
unsigned int itubotroncount=0;
unsigned short itubotrondataoverflow=0;

// CP - for keeping LEDs flashing visibly
unsigned int last_flash = 0;
unsigned int LED_FLASH_INTERVAL = 0xF0000000;		// CP debugging flood fill and flashing LEDs!  Set 1 bit high for low frequency flash
