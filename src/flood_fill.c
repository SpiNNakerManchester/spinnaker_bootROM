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


#include "../inc/dma.h"
#include "../inc/globals.h"

#ifdef DEBUG
#include "../inc/debug.h"
#include <stdio.h>
#endif


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
	unsigned int i;

	for(i = 0; i < ff_block_count; i++)
	{
		if(!ff_blocks_received[i])
		{
			#ifdef DEBUG
	   			printf("Block %d Missing.\n",i);
			#endif
			return 0;
		}
	}
	return 1;
}



/************ EXT LINK SECTION **********************************************************/

/* Called on receipt of a FF start message on the chip links. If the FF is targeted at the
 * monitor and a flood fill is not currently in progress, FF vars are initialised and
 * status is changed to reflect the commencing of flood fill. */
void cc_ff_start(unsigned int data)
{
	//pointer IMAGE = (unsigned int*) DTCM_IMAGE_SPACE;
	unsigned int i;

	if(ff_status == FF_OFF && FF_TARGET(data) == FF_TARGET_MONITOR)
	{
    	ff_block_count = FF_BLOCK_COUNT(data) + 1;
      	for(i = 0; i < MAX_FF_BLOCKS; i++) ff_blocks_received[i] = 0;
      	//for(i = 0; i < MAX_FF_IMAGE_SIZE; i++) IMAGE[i] = 0;
      	ff_status = FF_INTER_CHIP;
      	#ifdef DEBUG
	   		printf("StartFF. Block Count: %d.\n",ff_block_count);
		#endif
	}
}


/* Called on receipt of a FF block start message on the chip links. If a block is not
 * currently being listened for, and this block has not yet been received, FF vars are
 * initialised in preparation for receiving the words of the block. */
void cc_ff_block_start(unsigned int key)
{
	unsigned int i;

	if(ff_status == FF_INTER_CHIP && ff_current_block == 0xFFFF && !ff_blocks_received[FF_BLOCK_ID(key)])
	{
		ff_current_block = FF_BLOCK_ID(key);
		ff_word_count = FF_WORD_ID(key) + 1;
		for(i = 0; i < MAX_FF_WORDS; i++) ff_words_received[i] = 0;
      	#ifdef DEBUG
	   		printf("StartBlock %d. Word Count: %d.\n",ff_current_block,ff_word_count);
		#endif

	}
}


/* Called on receipt of a FF block data message on the chip links. If the data belongs to
 * the block currently being listened for and has not yet been received, it is stored. */
void cc_ff_block_data(unsigned int key, unsigned int data)
{
	pointer BLOCK_DATA = (unsigned int*) FF_SPACE_BASE;

	if(ff_status == FF_INTER_CHIP && (FF_BLOCK_ID(key) == ff_current_block) && !ff_words_received[FF_WORD_ID(key)])
	{
		ff_words_received[FF_WORD_ID(key)] = 1;
		BLOCK_DATA[FF_WORD_ID(key)] = data;
      	#ifdef DEBUG
	   		printf("Data: %d of %d\n",FF_WORD_ID(key)+1,ff_word_count);
		#endif
	}
}


/* Called on receipt of a FF block end message on the chip links. If the message pertains
 * to the current block and all words have been received, it is DMAed into the DTCM image
 * space and the resulting CRC is checked. */
void cc_ff_block_end(unsigned int key, unsigned int data)
{
	pointer BLOCK_DATA = (unsigned int*) FF_SPACE_BASE;
	unsigned int i, block_complete, src, dest, dma_status;

	#ifdef DEBUG
		printf("Block End Received\n");
	#endif

	if(ff_status == FF_INTER_CHIP && FF_BLOCK_ID(key) == ff_current_block)
	{
		#ifdef DEBUG
			printf("Block End Received for the block we are processing\n");
		#endif
		BLOCK_DATA[ff_word_count] = data;
		block_complete = 1;

		for(i = 0; i < ff_word_count; i++)
		{
			if(!ff_words_received[i])
			{
				block_complete = 0;
				#ifdef DEBUG
		   			printf("Block Incomplete. First Detected Missing Word at position: %d\n",i);
				#endif
				break;
			}
		}

		if(block_complete)
		{
			src = (unsigned int) BLOCK_DATA;
			dest = DTCM_IMAGE_SPACE + (FF_BLOCK_ID(key) * ff_word_count * 4);
			dma_status = dma(src, dest, DMA_CRC_ON, DMA_READ, ff_word_count * 4);

			if(dma_status == DMA_OK)
			{
				ff_blocks_received[FF_BLOCK_ID(key)] = 1;
				#ifdef DEBUG
		   			printf("CRC OK\n");
				#endif
			} else {
				#ifdef DEBUG
		   			printf("CRC error in DMA check\n");
				#endif
			}

			#ifdef DEBUG
	   			printf("Block %d processing complete.\n",ff_current_block);
			#endif

			ff_current_block = 0xFFFF;
		}
	}
}


/* Called on receipt of an FF control message on the chip link. If the control message
 * contains an execute command and the image is complete, the size of the image is
 * returned, otherwise 0 is returned. */
unsigned int cc_ff_control(unsigned int key)
{
	if(ff_status == FF_INTER_CHIP && key & FF_EXECUTE_COMMAND)
	{
		#ifdef DEBUG
	   		printf("Control Received\n");
		#endif
		if(image_complete())
		{
			#ifdef DEBUG
	   			printf("Image Complete. Length: 0x%x\n",ff_block_count * ff_word_count);
			#endif
			return (ff_block_count * ff_word_count);
		}
	}

	return 0;
}



/************ ETHERNET SECTION **********************************************************/

/* Called on receipt of a FF start message over ethernet. If the FF is targeted at the
 * monitor and a flood fill is not currently in progress, FF vars are initialised and
 * status is changed to reflect the commencement of flood fill. */
void eth_ff_start()
{
	int i;
	//pointer IMAGE = (unsigned int*) DTCM_IMAGE_SPACE;

	if((ff_status == FF_OFF) && ((SPINN_INSTRCTN_OP_1(rx_frame) & 0xFF) == FF_TARGET_MONITOR))
	{
		ff_block_count = 1 + (SPINN_INSTRCTN_OP_3(rx_frame) & 0xFF);
		for(i = 0; i < 256; i++) ff_blocks_received[i] = 0;
		//for(i = 0; i < MAX_FF_IMAGE_SIZE; i++) IMAGE[i] = 0;
		ff_status = FF_ETHERNET;
	}
}


/* Called on receipt of a FF block data message over ethernet. If the core is currently in
 * ethernet flood-fill mode and the block has not yet been received, it is stored in the
 * image space and marked as having been received. */
void eth_ff_block_data()
{
	int block_ID, i;
	pointer IMAGE = (unsigned int*) DTCM_IMAGE_SPACE;

	if(ff_status == FF_ETHERNET)
	{
		block_ID = SPINN_INSTRCTN_OP_1(rx_frame) & 0xFF;

		if(!ff_blocks_received[block_ID])
		{
			ff_word_count = 1 + ((SPINN_INSTRCTN_OP_1(rx_frame) & 0xFF00) >> 8);

			for (i = 0; i < ff_word_count; i++)
			{
				IMAGE[block_ID * ff_word_count + i] = SPINN_DATA(rx_frame, i);
			}

			ff_blocks_received[block_ID] = 1;
		}
	}
}


/* Called on receipt of an FF control message on the ethernet link. If the core is
 * currently in ethernet flood-fill mode, the control message contains an execute command
 * and the image is complete, the size of the image is returned, else 0 is returned. */
unsigned int eth_ff_control()
{
	if((ff_status == FF_ETHERNET) && (SPINN_INSTRCTN_OP_1(rx_frame) & FF_EXECUTE_ETH))
	{
		if(image_complete()) return (ff_block_count * ff_word_count);
	}
	return 0;
}
