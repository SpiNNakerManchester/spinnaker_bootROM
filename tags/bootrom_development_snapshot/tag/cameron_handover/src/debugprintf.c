/*****************************************************************************************
*	debugprintf.c - these functions support the use of printf in the ROM 
*       via Ethernet if attached or using NN packets towards the origin (0,0) chip.
*
*	Created by Cameron Patterson   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#include "../inc/globals.h"
#include "../inc/peripheral_inits.h"
#include "../inc/processor.h"
#include "../inc/eth_link.h"
#include "../inc/flood_sender.h"
#include "../inc/start.h"
#include "../inc/debugprintf.h"

// Routines to enable printing during debug
#include <stdio.h>
#include <stdarg.h>

#pragma import(__use_no_semihosting_swi)     // disable semihosting

struct __FILE { int handle; };
	FILE __stdout;
	FILE __stdin;


void iputc_eth(uint itube_chip_id, uint itube_proc_id, char itube_toprint) {
	if(itubotroncount>=256) {
		itubotrondataoverflow++; // set the overflow option, and drop the character (unfortunately)
	} else {
		itubotrondata[itubotroncount++]= (itube_chip_id << 16) | (itube_proc_id << 8) | itube_toprint;
		if(itubotroncount>=250) send_itubotron_message();	// it's time to send a packet spontaneously if buffer is just about full  
	}   
}

void iputc_cc(uint itube_chip_id, uint itube_proc_id, char itube_toprint) 
{
	pointer CHIP_IDENTIFIER = (uint*) CHIP_ID;
	uint my_x, my_y, use_link, key, data;

	// decision on which direction in which to send the packet - works for test board only!!
	// if sending on EXT0: x+1 y=y. EXT 1: x+1 y=+1. EXT 2: x=x y+1. EXT 3: x-1 y=y. EXT 4: x-1 y-1. EXT 5: x=x y-1
	my_x = (*CHIP_IDENTIFIER & 0xFF00) >> 8;
	my_y = (*CHIP_IDENTIFIER & 0xFF);
	if (my_x > 0 && my_y > 0) use_link=4;
	if (my_x > 0 && my_y == 0) use_link=3;
	if (my_x == 0 && my_y > 0) use_link=5;	
	if (my_x == 0 && my_y == 0) use_link=7; // self for non-monitor core

	// TODOPH1 1) generalise to wraparound size to see if should go in other direction
	// TODOPH1 2)   convert to P2P after initialising routing tables when numbering packet received
	    
	key = ITUBOTRON_CC << 24 | FF_PHASE_1;
	data = (((uint) itube_chip_id << 16) | (itube_proc_id << 8) | itube_toprint);
//	if (DEBUGPRINTF) printf("Link: 0x%x, Key: 0x%x, Data: 0x%x\n",use_link, key, data);
	transmit(use_link, WITH_PAYLOAD, key, data);
}  		

void itubecchandler(uint itube_chip_id, uint itube_proc_id, char itube_toprint)
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	if (phy_present) {
		iputc_eth(itube_chip_id, itube_proc_id, itube_toprint);
		// if I am Eth attached then add to itubotron print buffer	
	}
	else {
		iputc_cc(itube_chip_id, itube_proc_id, itube_toprint);
		// if I am not Eth attached then send out towards 0,0... (see TODOPH1 1&2 above for more ideas on how to develop)
	}
	
}

int fputc(int ch, FILE *f)  { 
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	
	#ifndef TLMTUBEPRINTF					// if in the real world and not on the TLM
	 pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	 pointer CHIP_IDENTIFIER = (uint*) CHIP_ID;
	char tempch = ch; // e.g. write a character to an UART 

		if (phy_present) {
			iputc_eth((*CHIP_IDENTIFIER & 0xFFFF), processor_ID, tempch);
			#ifdef DEBUGPRINTFSERIAL
				//dputc(tempch); // old command that uses the GPIO/serial port
				dr_tx_byte(processor_ID);
				dr_tx_byte(tempch);
			#endif
		}
		else {
			#ifdef DEBUGPRINTFSERIAL
				////dputc(tempch); 			 //CP - debugging register tests only (before Eth up!) uses the GPIO/serial port
				//dr_tx_byte(tempch);
			#endif
			#ifdef DEBUGPRINTFINTERCHIP
				iputc_cc((*CHIP_IDENTIFIER & 0xFFFF), processor_ID, tempch);
			#endif     
		}
	#endif
	
	#ifdef TLMTUBEPRINTF					// if on the TLM
		SYS_CTRL[0]=ch;						// write the character out to the TLM simulated tube port 
	#endif
	
	return ch;
}


void _sys_exit(void)  {
	while(1);
}



void init_tube_port ()
{
  pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
  uint dir = SYS_CTRL[GPIO_DIR];

  SYS_CTRL[GPIO_CLR] = SER_OUT;
  dir &= ~SER_OUT;
  dir |= SER_IN;
  SYS_CTRL[GPIO_DIR] = dir;
}


void dr_tx_byte (uint r)
{
  pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
  uint count = 8;

  while (count != 0)
    {
      if (r & 0x80)
	{
  	  SYS_CTRL[GPIO_SET] = SER_OUT_1;
	  while (! (SYS_CTRL[GPIO_READ] & SER_IN_1))
	    continue;
  	  SYS_CTRL[GPIO_CLR] = SER_OUT_1;
	  while (SYS_CTRL[GPIO_READ] & SER_IN_1)
	    continue;
	}
      else
	{
  	  SYS_CTRL[GPIO_SET] = SER_OUT_0;
	  while (! (SYS_CTRL[GPIO_READ] & SER_IN_0))
	    continue;
  	  SYS_CTRL[GPIO_CLR] = SER_OUT_0;
	  while (SYS_CTRL[GPIO_READ] & SER_IN_0)
	    continue;
	}
      r = r << 1;
      count--;
    }
}





