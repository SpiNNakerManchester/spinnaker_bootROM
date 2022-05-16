/*****************************************************************************************
*	Created by Thomas Sharp and Cameron Patterson
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#include "../inc/globals.h"
#include "../inc/dma.h"
#include <stdio.h> // To enable printing during debug


uchar generate_check(uint key, uint data)						// this routine generate a 4bit 1s complement checksum over a NN packet 
{
	uchar i, check=0;
	for(i = 0; i < 7; i++) check += (key >> (i * 4)) & 0xF;		// sum over the 7 non-chk nibbles in the key data
	for(i = 0; i < 8; i++) check += (data >> (i * 4)) & 0xF;	// add the sum over the 8 data nibbles 
	while(check & 0xF0) check = (check & 0xF) + (check >> 4);	// loop until left with a single hex digit checksum
	return 0xF - check;											// complement this value - returning a 4bit 1's complement checksum
}

unsigned int transmit(uint route, uchar payload, uint key, uint data)	// send a NN message out with parameters provided
{
	unsigned int interpacketdelay = 0xFF;  						// safe at 0xFF on test board
	unsigned int waitforcctxbuffertime = 0xFFFFF;				// how long to wait for comms buffer to come free
	unsigned int i,t;
	pointer COMMS_CTRL = (uint*) COMMS_CTRL_BASE;
	unsigned int keysend;
	unsigned int ctrlsend;  
	
	if(payload) {
		ctrlsend = NN_PACKET_PAYLOAD(route); 					// set NN Normal, to correct link(s) and with payload set
		keysend = (generate_check(key, data) << 0x1C) | key;	// generate checksum using key and data 
	} else {
		ctrlsend = NN_PACKET_NOPAYLOAD(route); 					// set NN Normal, to correct link(s) and with no payload set
		keysend = (generate_check(key, 0) << 0x1C) | key;		// generate checksum using key only
	}  
	
	i = interpacketdelay; while(i--);  				// spin on an interpacket delay 
		
	t=waitforcctxbuffertime; 										// timeout used to wait for the transmit channel to become free
	while ((COMMS_CTRL[COMMS_CTRL_TX_CTRL] & TX_BUFFER_EMPTY) == 0) // wait for the tx buffer to report empty
    {
    	if (--t == 0)	{
			if (DEBUGPRINTF) printf("Oops:ff_block_data tx_buff didn't empty. What should I do now?\n");
    		return 0; 												// we timed out waiting for the tx buffer emptying
    	}															
    }	
	
	//if (DEBUGPRINTF) printf("Ctrl: 0x%x.  Key: 0x%x.  Data: 0x%x.\n",ctrlsend,keysend,data); // CP Debug
	COMMS_CTRL[COMMS_CTRL_TX_CTRL] = ctrlsend;  					// this will also clear any F or O bits that were set sticky
	COMMS_CTRL[COMMS_CTRL_SEND_DATA] = data;						// will be ignored if no payload
	COMMS_CTRL[COMMS_CTRL_SEND_KEY] = keysend;  					// away it goes

	return 1;
}

#ifndef SPINNAKER2


//#ifdef TEST_FLOOD_FILL_IMAGE_OUT
#if (defined (TEST_FLOOD_FILL_IMAGE_OUT) || defined (TEST_REPEAT_FLOOD_FILL_IMAGE_OUT))

uint link_status[LINKS] = {FF_PENDING,FF_PENDING,FF_PENDING,FF_PENDING,FF_OFF,FF_OFF};	// send out links 0-3 only (others not in use!)
uint flood_complete = 0x100000;															// 0-2 PCB links to chips, 3 loopback cable
uint block_count = 32; 		// From 1-256,  lower numbers of blocks is more efficient
uint word_count = 256;  	// From 1-256,  8192 max words in total (ITCM size)  eg. 32x256, 64x128 in either combination




void ff_control(uchar route, uint parameter) 						// function that sends a control packet out via an EXT link
{
	uint key = FF_CONTROL << 0x18 | FF_EXECUTE | FF_PHASE_1; 		// sets up the key field
	uint data = parameter;											// data is supplied as the 2nd argument
	if (DEBUGPRINTF) printf("C");
	transmit(route, WITH_PAYLOAD, key, data);						// send the NN packet via the transmit function
}

void ff_block_data(uchar route, uchar block_ID, uchar word_ID, uint data)  // function to send a single data word out via an EXT link
{
	uint key = FF_BLOCK_DATA << 0x18 | block_ID << 0x10 | word_ID << 0x8 | FF_PHASE_1;  // construct the key
//	if (DEBUGPRINTF) printf("D");
	transmit(route, WITH_PAYLOAD, key, data);  						// send the NN packet via the transmit function
}

void ff_block_end(uchar route, uchar block_ID, uint checksum)		// function to send the block end message out via the EXT link
{
	uint key = FF_BLOCK_END << 0x18 | block_ID << 0x10 | FF_PHASE_1;// construct the key
	uint data = checksum;											// data supplied here is the block level checksum
//	if (DEBUGPRINTF) printf("E");
	transmit(route, WITH_PAYLOAD, key, data);						// send the NN packet via the transmit function
}

void ff_block_start(uchar route, uchar block_ID, uint word_count)	// function to send the block start message out via the EXT link
{
	uint key = FF_BLOCK_START << 0x18 | block_ID << 0x10 | ((word_count-1) << 0x8) | FF_PHASE_1; 	// construct the key
	uint data = block_ID*word_count*4;  							// if using FF_PHASE_1, implies contiguous ITCM memory locations
//	if (DEBUGPRINTF) printf("B");
	transmit(route, WITH_PAYLOAD, key, data);						// send the NN packet via the transmit function
}

void ff_start(uchar route, uint block_count)  						// function that sends flood fill start packet out via an EXT link
{
	uint key = FF_START << 0x18 | FF_PHASE_1;						// construct the key
	uint data = (FF_TARGET_MONITOR << 0x18) | ((block_count-1) << 0x10);	// the data field consists of targetID, and # of blocks
	if (DEBUGPRINTF) printf("S");
	transmit(route, WITH_PAYLOAD, key, data);						// send the NN packet via the transmit function
}


void flood()														// Function to call when flood filling an image on EXT links
{
	pointer IMAGE = (uint*) DTCM_IMAGE_SPACE;
	pointer BLOCK = (uint*) FF_SPACE_BASE;
	uchar i;
	unsigned int waitforremotetocompute=0xFFFF;  					// delay between packets, where remotes reqd to 'compute' something
	unsigned int j, k, t;
	unsigned int calculatedCRC;
	
	//if (DEBUGPRINTF&&phy_present) printf("Flooding Out Begins Here...\n"); // CP Debug

	for(i = 0; i < LINKS; i++) if (link_status[i] == FF_PENDING) ff_start(i, block_count);				// send FF start to peers
	t = waitforremotetocompute; while(t--);  															// wait while remotes compute
		
	for(j = 0; j < block_count; j++) {																	// loop through each data block
		for(i = 0; i < LINKS; i++) if (link_status[i] == FF_PENDING) ff_block_start(i, j, word_count);	// send each FF block start 
		t = waitforremotetocompute; while(t--);  														// wait while remotes compute
		
		dma((unsigned int) BLOCK, (unsigned int) (IMAGE+(j * word_count)),DMA_CRC_ON, DMA_WRITE, word_count*4);	 
			// DMA copy from TCM location to SYSRAM buffer and calculate CRC.
		calculatedCRC=BLOCK[word_count];  																// store CRC calculated
			
		for(k = 0; k < word_count; k++)	{																// loop each data word in block
   			for(i = 0; i < LINKS; i++) if (link_status[i] == FF_PENDING) ff_block_data(i, j, k, IMAGE[(j*word_count)+k]);  
		}			// Saves CPU/bus cycles using TCM copy rather than SysRAM BLOCK[k], transmits all data words in block
   		
		for(i = 0; i < LINKS; i++) if (link_status[i] == FF_PENDING) ff_block_end(i, j, calculatedCRC); 
					// Loads calculated CRC as n+1 word of block transferred (BLOCK[word_count]) and sends out as block end packets
		t = waitforremotetocompute; while(t--);  														// wait while remotes compute CRC
	}
		
	for(i = 0; i < LINKS; i++) if (link_status[i] == FF_PENDING) ff_control(i, 0x0);
				// Send Flood Fill End/Control messages out, defaulting to 0x0 execution (Phase 1 filling) TODOPH1 read from Mailbox?
	t = waitforremotetocompute; while(t--);  															// wait- remotes checking all OK
		
	//if (DEBUGPRINTF&&phy_present) printf("Flooding Out Ends Here...\n"); // CP Debug
	flood_complete = time;

	#ifndef TEST_REPEAT_FLOOD_FILL_IMAGE_OUT			// testing repeats the send multiple times for validation, so aren't marked active.  
		for(i = 0; i < LINKS; i++) if (link_status[i] == FF_PENDING) link_status[i] = FF_ACTIVE;
	#endif

}

#endif

#endif

