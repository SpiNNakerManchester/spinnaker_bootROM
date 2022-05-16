/*****************************************************************************************
* 	Created by Cameron Patterson
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/


#include "../inc/globals.h"

#ifdef DEBUG
#include "../inc/debug.h"
#include <stdio.h>
#endif


unsigned int dma(unsigned int sys_NOC_address, unsigned int TCM_address,
				unsigned int crc, unsigned int direction, unsigned int length)
{
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
		//#ifdef DEBUG
		//if(PHY_PRESENT) printf("ROM DMA Error Detected - ARRGGHHH!!! - Resetting.  StatReg:0x%x\n",returner);
		//#endif
		DMA_CTRL[DMA_CTRL_CTRL] = DMA_CLEAR_ERRORS;  // if DMA error 13-19 detected, and controller paused - clear this away R:Restart
	}
	else {
		returner = 0;	// send back a good status value
		DMA_CTRL[DMA_CTRL_CTRL] = DMA_ACK_SUCCESS;  // after a successful DMA transfer, clear the done line D:clear Done Int   
	}
	
	return returner;  // return 0 for good, and >0 if error, with status register inclusive error ID
}
