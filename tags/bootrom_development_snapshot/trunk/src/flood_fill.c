/*****************************************************************************************
* 	flood_fill.c - contains functions responsible for handling flood-fill packets sent
* 	over the ethernet and inter-chip links. Images comprise of a number of blocks
*	(themselves consisting of a number of words) which are composed in memory as they are
*	received. On completion of flood-fill, a non-zero value is returned to the ISR that
*	called the function in this file and that ISR then causes the image to be executed. 
*
*
*	Created by Cameron Patterson and Thomas Sharp
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#include "../inc/dma.h"
#include "../inc/globals.h"

#include <stdio.h> // To enable printing during debug



// Status variables for flood fill process
unsigned int ff_block_count = 0;
unsigned int ff_current_block = 0xFFFF;
unsigned int ff_status = 0;
unsigned int ff_word_count = 0;

// Arrays for recording the receipt of components of an image
unsigned char ff_blocks_received[MAX_FF_BLOCKS];
unsigned char ff_words_received[MAX_FF_WORDS];



/* Iterates over the blocks_received array, returning 0 if any blocks have not been. */
unsigned int image_complete()
{
	unsigned int i, done = 1;
	if (DEBUGPRINTF) printf("   Checking image...");
	
	for(i = 0; i < ff_block_count; i++)
	{ 
		if(!ff_blocks_received[i])
		{ 
			done = 0;
			//TODO insert break?
			if (DEBUGPRINTF) printf("Block 0x%x is missing\n",i);
		}
	}
	return done;
}



/************ EXT LINK SECTION **********************************************************/




/* Called on receipt of a FF start message on the chip links. If the FF is targeted at the
 * monitor and a flood fill is not currently in progress, FF vars are initialised and
 * status is changed to reflect the commencing of flood fill. */
void cc_ff_start(unsigned int data)
{
	pointer IMAGE = (unsigned int*) DTCM_IMAGE_SPACE;
	unsigned int i;

	if(ff_status == FF_OFF && FF_TARGET(data) == FF_TARGET_MONITOR)		// only listen if we haven't already started and Ph1 (Mon)
	{  																	
	   	//if(DEBUGPRINTF&&phy_present) printf("s");
    	ff_block_count = FF_BLOCK_COUNT(data)+1;  						// block count 0:FF = 1-256
      	for(i = 0; i < MAX_FF_BLOCKS; i++) ff_blocks_received[i] = 0; 	// mark all blocks as not received
      	for(i = 0; i < MAX_FF_IMAGE_SIZE; i++) IMAGE[i] = 0;  			// clear imagespace itself with all zeros
      	ff_status = FF_INTER_CHIP;										// set status as receiving from interchip link
	}
}

/* Called on receipt of a FF block start message on the chip links. If a block is not
 * currently being listened for, and this block has not yet been received, FF vars are
 * initialised in preparation for receiving the words of the block. */
void cc_ff_block_start(unsigned int key)
{
	unsigned int i;
	
	
	if (ff_current_block == 0xFFFF && !ff_blocks_received[FF_BLOCK_ID(key)])				
			// if we're not currently listening for another block's data and we've not already received this block successfully
	{
		//	if(DEBUGPRINTF&&phy_present) printf("b");
		ff_current_block = FF_BLOCK_ID(key);							// listen for this blockID
		ff_word_count = FF_WORD_ID(key)+1;  							// word count in this block is 0:FF = 1-256
		for(i = 0; i < MAX_FF_WORDS; i++) ff_words_received[i] = 0;		// clear the data word table ready to start
	}
}

/* Called on receipt of a FF block data message on the chip links. If the data belongs to
 * the block currently being listened for and has not yet been received, it is stored. */
void cc_ff_block_data(unsigned int key, unsigned int data)
{
	pointer block_data = (unsigned int*) FF_SPACE_BASE;
	   	
	if((FF_BLOCK_ID(key) == ff_current_block) && !ff_words_received[FF_WORD_ID(key)])		// if data is in the block we are populating
	{																						// and we haven't heard the word already
		//if(DEBUGPRINTF&&phy_present) printf("d");
		ff_words_received[FF_WORD_ID(key)] = 1;												// mark word in relevant position received
		block_data[FF_WORD_ID(key)] = data;													// store the data in the block
	}
	
}

/* Called on receipt of a FF block end message on the chip links. If the message pertains
 * to the current block and all words have been received, it is DMAed into the DTCM image
 * space and the resulting CRC is checked. */
void cc_ff_block_end(unsigned int key, unsigned int data)
{
	pointer block_data = (unsigned int*) FF_SPACE_BASE;
	unsigned int i, block_complete, source, destination, dma_status;
	
	
	if(FF_BLOCK_ID(key) == ff_current_block)								// if end block corresponds to our block
	{
		//	if(DEBUGPRINTF&&phy_present) printf("e");
		block_data[ff_word_count] = data;									// data stores the received block CRC, store at end of data
		block_complete = 1;													// mark block as complete tentatively
		for(i = 0; i < ff_word_count; i++) if(!ff_words_received[i]) block_complete = 0;	// go through received words, if any missing
																							// mark block incomplete
		if(block_complete)
		{			
			source = (unsigned int) block_data;												// if block complete...
			destination = DTCM_IMAGE_SPACE + (FF_BLOCK_ID(key) * ff_word_count * 4); 
			
			dma_status = dma((unsigned int)source, (unsigned int)destination, DMA_CRC_ON, DMA_READ, (ff_word_count * 4));
					// DMA copy from SYSRAM buffer to TCM location with CRC, calculate own CRC and validate both CRCs are the same.
			
			if(dma_status == DMA_OK) ff_blocks_received[FF_BLOCK_ID(key)] = 1; 			// TX worked, no CRC error, mark block done
			
			ff_current_block = 0xFFFF;   													// start listening for another block 
		}	
	}
}

/* Called on receipt of an FF control message on the chip link. If the control message
 * contains an execute command and the image is complete, the size of the image is
 * returned, otherwise 0 is returned. */
unsigned int cc_ff_control(unsigned int key)
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	
	if(key & FF_EXECUTE_COMMAND) 						// If the control indicates an execute command
	{
		if(DEBUGPRINTF&&phy_present) printf("c");

		if(image_complete()) {							// Calls a function to check all blocks present, correct (CRC'ed)
			#ifdef TEST_REPEAT_FLOOD_FILL_IMAGE_OUT
				ff_status = FF_OFF;						// Testing only - to allow receipt of another flood fill next time around
			#endif
			return (ff_block_count * ff_word_count);	// returns this size of the image (in words) back to the callee
		}
	}
	return 0;											// If the control cmd didn't want an execute, or not all blocks in place
}




/************ ETHERNET SECTION ****************************************************************************/


/* Called on receipt of a FF start message over ethernet. If the FF is targeted at the
 * monitor and a flood fill is not currently in progress, FF vars are initialised and
 * status is changed to reflect the commencing of flood fill. */
void eth_ff_start()
{
	int i;
	pointer IMAGE = (unsigned int*) DTCM_IMAGE_SPACE;
	if((ff_status == FF_OFF) && ((SPINN_INSTRCTN_OP_1(rx_frame) & 0xFF) == FF_TARGET_MONITOR))	// if we haven't started receiving &
	{																							// the instruction is Ph1 boot (Mon)
		if (DEBUGPRINTF) printf("es");
		ff_block_count = 1 + (SPINN_INSTRCTN_OP_3(rx_frame) & 0xFF);		// blocks expected 0-255 (1:256)
		for(i = 0; i < 256; i++) ff_blocks_received[i] = 0;					// clear out blocks received table with 0s
		for(i = 0; i < MAX_FF_IMAGE_SIZE; i++) IMAGE[i] = 0;				// ckear out image data with 0s
		ff_status = FF_ETHERNET;											// set status to say we're listening to a FF on Ethernet
	}
}

/* Called on receipt of a FF block data message over ethernet. If the block has not yet
 * been received, it is stored in the image space and marked as having been received. */
void eth_ff_block_data()
{
	int block_ID, i;
	pointer IMAGE = (unsigned int*) DTCM_IMAGE_SPACE;
	
	if (ff_status == FF_ETHERNET)									// check to see if we're listening to FF on Ethernet
	{
		block_ID = SPINN_INSTRCTN_OP_1(rx_frame) & 0xFF;			// what blockID are we being sent
		if(!ff_blocks_received[block_ID])							// if we haven't already received this block
		{
			if (DEBUGPRINTF) printf("eb");		
			ff_word_count = 1+ ((SPINN_INSTRCTN_OP_1(rx_frame) & 0xFF00) >> 8);	// max block word size 0:FF 1-256.
			//if (DEBUGPRINTF) printf("BlkID: 0x%x of 0x%x. WdCnt: 0x%x.\n", block_ID, ff_block_count, ff_word_count);

			for (i = 0; i < ff_word_count; i++)	IMAGE[block_ID * ff_word_count + i] = SPINN_DATA(rx_frame, i);
			ff_blocks_received[block_ID] = 1;						// copy data from frame to block & mark block received
		} 
	}
}

/* Called on receipt of an FF control message on the ethernet link. If the control message
 * contains an execute command and the image is complete, the size of the image is
 * returned, otherwise 0 is returned. */
unsigned int eth_ff_control()
{
	if((ff_status == FF_ETHERNET) && (SPINN_INSTRCTN_OP_1(rx_frame) & FF_EXECUTE_ETH))	// If Ethernet FF & control = execute command
	{
		if(image_complete()) return (ff_block_count * ff_word_count);					
					// Calls a function to check all blocks present, correct (CRC'ed), and if so returns the data length 
	}
	return 0;		// if image is not complete or control is not execute, then return to main program control
}
