/*****************************************************************************************
* 	Created by Cameron Patterson
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008-2010. All rights reserved.
*****************************************************************************************/

#include "../inc/globals.h"
#include "../inc/dma.h"
#include "../inc/flood_sender.h"


// This routine performs a DMA from TCM to SysRAM FF buffer, and returns the calculated CRC of the block
unsigned int IVB_CRC(unsigned int TCM_Source, unsigned int dma_tx_length)
{
	pointer SYSRAM_BLOCK = (uint*) FF_SPACE_BASE;
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;

//	if(DEBUGPRINTF) printf("*** IVB Sys: 0x%x.  TCM: 0x%x.  Len:0x%x.\n",SYSRAM_BLOCK,TCM_Source,dma_tx_length);
	dma((unsigned int) SYSRAM_BLOCK, TCM_Source, DMA_CRC_ON, DMA_WRITE, dma_tx_length); 	// initiate the CRC
	return (DMA_CTRL[0x7]);  // read back & return the CRC calculated across all the IVB data required (not no validation of dma)
}


void ivb_populator(unsigned int execute_address, unsigned int start_address)
{
	pointer IVB = (unsigned int*) ITCM_VALIDATION_BLOCK;
	uint i;
	uint ivb_start_address = ITCM_BASE;							// ITCM is what we will validate. The the ITCM
	uint ivb_size = ITCM_SIZE-IVB_SPACE;						//  32KB - the IVB (ivb_size must be multiple of 4, for DMA later)
	uint ivb_execute_address = execute_address;					// this is the address that will be called if a recovery reboot
	
	// Create the CRCs for each block to be validated using the IVB (for reset recovery)
	for (i=0;i<=((ivb_size-1)/1024);i++)  { 					// loop around for each required 1024 block (or part block)
		IVB[3+i] = IVB_CRC(ivb_start_address+(i*1024), i==((ivb_size-1)/1024) ? ((ivb_size-1)%1024)+1 : 1024);	
	}		// write block's CRC to the IVB (ITCM-Validation-Block) in position. If last block, size may be <1024

	// Create the ITCM Validation Block control information (for reset recovery) 
	IVB[0] = ivb_start_address;   				//write start address for protected block
	IVB[1] = ivb_size; 							//write size of the protected block (in bytes, must be multiple of 4 for DMA)
	IVB[2] = ivb_execute_address;				//write the address to be called on recovery attempt
	IVB[35] = IVB_CRC((unsigned int) IVB, 35*4);//Grand Block CRC - calculated over control words and block CRCs (not self/magic num)
	IVB[36] = IVB_MAGIC_NUMBER;					// Checked 1st on boot to determine if should try and recover then does CRCs checks etc		
}
