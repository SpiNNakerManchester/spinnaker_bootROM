/*****************************************************************************************
* 	Created by Steve Temple, Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/


#include "../inc/processor.h"
#include "../inc/globals.h"
#include <stdio.h> // To enable printing during debug

#ifdef TEST_BOARD
// CP section for Poke Fill Routine, stolen from Steve Temple, but modified to allow election of monitor etc.


// Write (poke) a single word to a neighbouring chip. Simple timeout mechanism returns 0 on failure.
unsigned int link_write_word (uint addr, uint link, const uint *buf)
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer cc = (unsigned int*) COMMS_CTRL_BASE;
	unsigned int tt = 0xFFFFF;								// Timeout tweaked for higher clock freq on poking chip. cf. STs original
	unsigned int t = tt;
	unsigned int returner=1; 
	unsigned int rcvdaddr=0xFFFFFFFF;

	while ((cc[COMMS_CTRL_TX_CTRL] & TX_BUFFER_EMPTY) == 0)	{ // wait for the tx buffer to report empty
		if (--t == 0)	{
			if (DEBUGPRINTF&&phy_present) printf("Oops:LinkWriteWord tx_buff didn't empty\n");
			return 0;   									// if we timeout waiting for the tx buffer to empty
		}
	}

	cc[COMMS_CTRL_TX_CTRL] = CC_TX_NN_DIRECT | (link << 18);// NN direct packet 
 	cc[COMMS_CTRL_TRANSMIT_DATA] = *buf;					// NN payload word for memory location...
	cc[COMMS_CTRL_TRANSMIT_KEY] = addr;						// NN address - and away it goes to poke where no-one has poked before

	t = tt;													// reset timeout variable
	while ((cc[COMMS_CTRL_RX_STATUS] & CC_RX_PACKET) == 0) if (--t == 0) returner=0;	
			// while we are waiting on a response to come back, tick down the timer, if we zero then timeout

	if (cc[COMMS_CTRL_RX_STATUS] & CC_ERR_PACKET) {
		cc[COMMS_CTRL_RX_STATUS] = CC_ERR_PACKET;			// parity/framing error, clear them now.
		returner=0;
	}
  
	rcvdaddr = cc[COMMS_CTRL_RECEIVE_KEY];  				// read data so it clocks out ready for next acknowledge
	if ((rcvdaddr&0xFFFFFFFC)!=(addr&0xFFFFFFFC)) returner=1;	// if we don't get an ack matching our poke addr (top30bits) then return a fail
						// TODO FIX ME!
	return returner;
}




// Write "len" words to a neighbouring chip from buffer "buf". If any word 
//   times out returns 0, otherwise 1 when whole block sent.

unsigned int link_write (uint addr, uint len, uint link, const uint *buf)
{
	unsigned int i,rc;

	for (i = 0; i < len; i++) {								// Send as many words as we're asked to
		rc = link_write_word (addr, link, buf);				// Send the Direct NN packet across the requested link
		if (rc == 0) return 0;								// If a receive problem encounters then exit with a zero
		addr += 4;											// Increment the address to be poked to
		buf++;												// Move to the next word to be sent
	}
	return 1;												// If no errors are detected return a 1 to signify success
}


// Boot a neighbouring chip with data supplied in a buffer. The
// load address ("addr") should normally be 0xf5000000 (System RAM)
// When download is complete CPUs are allowed to run.
// NB - return codes not checked!
//
// addr - the load address for the data
// len - number of words
// link - the link number to use
// buf - array of words to be loaded

const uint w[] = {0x5ecfffff, 0x00000001, 0x5ec00000, 0xe1a00000, 0x5ec0011F,              0xFFFFFF3C,0xC3,0xC3,0x41};

void poke_neighbours_up(uint len, const uint *buf, uint link)
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;

	disable_interrupts();									// Don't want to get interrupted during this process

	link_write (0xf2000044,1,link,w+5);
	link_write (0xf200004C,1,link,w+6);
	link_write (0xf2000048,1,link,w+8);			// display red as image is downloading

	(void) link_write (SYSCTRL_SOFT_RESET, 1, link, w+0);	// Reset all CPUs on far chip
	(void) link_write (SYSCTRL_MISC_CTRL, 1, link, w+1);	// Remap ROM/SYSRAM on poked chip
	if (DEBUGPRINTF&&phy_present) printf("Reset and Remap done. Copying from 0x%x Len 0x%x, to link:%x.\n",buf,len,link);
	link_write (POKER_IMAGE_BASE, len, link, buf);	// Copy buffer out over links to far away chips
	
  	(void) link_write (SYSCTRL_MONITOR_ID, 1, link, w+4);	// Force the monitor ID to invalid to force an election
  															// TODO: clear the monitor arbitration bit
	(void) link_write (SYSCTRL_SOFT_RESET, 1, link, w+2);	// Allow all remote CPUs ok poked chip to now run from sysram
	if (DEBUGPRINTF&&phy_present) printf("\nResetting Remote\n");
		
	link_write (0xf5000000,1,link,w+5);
	link_write (0xf5000004,1,link,w+6);
	link_write (0xf5000008,1,link,w+7);			// display amber after image has downloaded
		
		
	enable_interrupts();									// turn interrupts back on as poking complete
}

#endif
