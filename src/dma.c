/*****************************************************************************************
* 	Created by Thomas Sharp and Cameron Patterson
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

// Routines to enable printing during debug
#include <stdio.h>
#include <stdarg.h>

#include "../inc/globals.h"

uint dma(unsigned int sys_NOC_address, unsigned int TCM_address,
				unsigned int crc, unsigned int direction, unsigned int length)
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	unsigned int returner = 0;	 
	
//	if(DEBUGPRINTF) printf("*** Sys: 0x%x.  TCM: 0x%x.  Len:0x%x.\n",sys_NOC_address,TCM_address,length);
	
	while(DMA_CTRL[DMA_CTRL_STAT] & DMA_Q_FULL) {  
		// check to see whether the DMA queue is full and if we spinn to wait on new one
		
		if (DMA_CTRL[DMA_CTRL_STAT] & DMA_PAUSED) DMA_CTRL[DMA_CTRL_CTRL] = DMA_CLEAR_ERRORS;  
			// defensively clear paused transfer rather than spinn forever 		
	} 		
														
	DMA_CTRL[DMA_CTRL_ADRS] = sys_NOC_address;
	DMA_CTRL[DMA_CTRL_ADRT] = TCM_address;
	DMA_CTRL[DMA_CTRL_DESC] = (0x1 << DMA_BURST_OFFSET) | 			// CP burst set to 1 rathen thank 4 due to Silistix DMA burst bug
								(crc << DMA_CRC_OFFSET) |
								(direction << DMA_DIRECTION_OFFSET) |
								(length & 0xFFFF);
								
	while(DMA_CTRL[DMA_CTRL_STAT] & DMA_IN_PROGRESS);  				// Wait until the transfer has completed.  
		
	if (DMA_CTRL[DMA_CTRL_STAT] & DMA_PAUSED) {		
		returner = DMA_CTRL[DMA_CTRL_STAT];
		if(DEBUGPRINTF&&phy_present) printf("ROM DMA Error Detected - ARRGGHHH!!! - Resetting.  StatReg:0x%x\n",returner);
		DMA_CTRL[DMA_CTRL_CTRL] = DMA_CLEAR_ERRORS;  // if DMA error 13-19 detected, and controller paused - clear this away R:Restart
	}
	else {
		returner = 0;	// send back a good status value
		DMA_CTRL[DMA_CTRL_CTRL] = DMA_ACK_SUCCESS;  // after a successful DMA transfer, clear the done line D:clear Done Int   
	}
	
	return returner;  // return 0 for good, and >0 if error, with status register inclusive error ID
}


#ifdef SPINNAKER2					// CRC setup is only required on the Spinnaker2 chip. The testchip is hard pre-defined.
void setup_crc32_spinnaker2()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	unsigned int i;

	uint table_setup[32]= {
	0xFB808B20, 0x7DC04590, 0xBEE022C8, 0x5F701164, 0x2FB808B2, 0x97DC0459, 0xB06E890C, 0x58374486, 
	0xAC1BA243, 0xAD8D5A01, 0xAD462620, 0x56A31310, 0x2B518988, 0x95A8C4C4, 0xCAD46262, 0x656A3131, 
	0x493593B8, 0x249AC9DC, 0x924D64EE, 0xC926B277, 0x9F13D21B, 0xB409622D, 0x21843A36, 0x90C21D1B, 
	0x33E185AD, 0x627049F6, 0x313824FB, 0xE31C995D, 0x8A0EC78E, 0xC50763C7, 0x19033AC3, 0xF7011641 };
	// this is the CRC32 setup for the DMA's CRC controller - source. Martin Grymel: SpiNNaker Programmable CRC March 12, 2010 

	for (i=0;i<32;i++) DMA_CTRL[DMA_CTRL_CRCSETUPTABLE+i]=table_setup[i];  // loads CRC32 Ethernet polynomial 0x04C11DB7, (data width 32bits).

}
#endif
