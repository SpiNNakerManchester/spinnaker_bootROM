/*****************************************************************************************
*	Created by Thomas Sharp
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#include "../inc/dma.h"
#include "../inc/globals.h"



uchar generate_check(uint key, uint data)
{
	uchar i, check = 0;
	for(i = 0; i < 7; i++) check += (key >> (i * 4)) & 0xF;
	for(i = 0; i < 8; i++) check += (data >> (i * 4)) & 0xF;
	while(check & 0xF0) check = (check & 0xF) + (check >> 4);
	return 0xF - check;
}

void transmit(uint route, uint check, uint key, uint data)
{
	pointer COMMS_CTRL = (uint*) COMMS_CTRL_BASE;
	
	while(!(COMMS_CTRL[COMMS_CTRL_TX_CTRL] & TX_BUFFER_EMPTY));
	
	COMMS_CTRL[COMMS_CTRL_TX_CTRL] = NN_PACKET_PAYLOAD(route);
	COMMS_CTRL[COMMS_CTRL_TRANSMIT_DATA] = data;
	COMMS_CTRL[COMMS_CTRL_TRANSMIT_KEY] = FF_CHECK(check) | key;
}

void ff_block_data(uchar route, uchar BID, uchar WID, uint data)
{
	uint key = FF_OP(FF_BLOCK_DATA) | FF_BID(BID) |
									FF_WID(WID) | FF_PID(FF_PHASE_1);
	uchar check = generate_check(key, data);
	
	transmit(route, check, key, data);
}

void ff_block_end(uchar route, uchar BID, uint checksum)
{
	uint key = FF_OP(FF_BLOCK_END) | FF_BID(BID) | FF_PID(FF_PHASE_1);
	uint data = checksum;
	uchar check = generate_check(key, data);
	
	transmit(route, check, key, data);
}

void ff_block_start(uchar route, uchar BID, uchar word_count)
{
	uint key = FF_OP(FF_BLOCK_START) | FF_BID(BID) |
									FF_WID(word_count) | FF_PID(FF_PHASE_1);
	uchar check = generate_check(key, 0x0);
	
	transmit(route, check, key, 0x0);
}

void ff_control(uchar route, uint parameters) 
{
	uint key = FF_OP(FF_CONTROL) | FF_CTRL_FUNC(FF_EXECUTE) | FF_PID(FF_PHASE_1);
	uint data = parameters;
	uchar check = generate_check(key, data);
	
	transmit(route, check, key, data);
}

void ff_start(uchar route, uchar block_count)
{
	uint key = FF_OP(FF_START) | FF_PID(FF_PHASE_1);
	uint data = FF_TRGT(0x0) | FF_BID(block_count);
	uchar check = generate_check(key, data);
	
	transmit(route, check, key, data);
}

void flood()
{
	pointer IMAGE = (uint*) DTCM_IMAGE_SPACE;
	int i, link, delay;
	
	link = 3;
	delay = 3000000;
		
	i = delay; while(i--); // Spin wait to prevent overloading receiver
	ff_start(link, 1);
	i = delay; while(i--);
	ff_block_start(link, 0, 22);
	
	i = delay; while(i--);
	ff_block_data(link, 0, 0, 0xE59FF04C); // Program to switch LEDs on
	i = delay; while(i--);
	ff_block_data(link, 0, 1, 0xE1A00000);
	i = delay; while(i--);
	ff_block_data(link, 0, 2, 0xE1A00000);
	i = delay; while(i--);
	ff_block_data(link, 0, 3, 0xE1A00000);
	i = delay; while(i--);
	ff_block_data(link, 0, 4, 0xE1A00000);
	i = delay; while(i--);
	ff_block_data(link, 0, 5, 0xE1A00000);
	i = delay; while(i--);
	ff_block_data(link, 0, 6, 0xE1A00000);
	i = delay; while(i--);
	ff_block_data(link, 0, 7, 0xE1A00000);
	i = delay; while(i--);
	ff_block_data(link, 0, 8, 0xE3A014F2);
	i = delay; while(i--);
	ff_block_data(link, 0, 9, 0xE3A02044);
	i = delay; while(i--);
	ff_block_data(link, 0, 10, 0xE7910002);
	i = delay; while(i--);
	ff_block_data(link, 0, 11, 0xE3C000C3);
	i = delay; while(i--);
	ff_block_data(link, 0, 12, 0xE7810002);
	i = delay; while(i--);
	ff_block_data(link, 0, 13, 0xE3A0204C);
	i = delay; while(i--);
	ff_block_data(link, 0, 14, 0xE7910002);
	i = delay; while(i--);
	ff_block_data(link, 0, 15, 0xE3A000C3);
	i = delay; while(i--);
	ff_block_data(link, 0, 16, 0xE7810002);
	i = delay; while(i--);
	ff_block_data(link, 0, 17, 0xE3A02048);
	i = delay; while(i--);
	ff_block_data(link, 0, 18, 0xE3A00002);
	i = delay; while(i--);
	ff_block_data(link, 0, 19, 0xE7810002);
	i = delay; while(i--);
	ff_block_data(link, 0, 20, 0xEAFFFFFE);
	i = delay; while(i--);
	ff_block_data(link, 0, 21, 0x00000020);
	
	i = delay; while(i--);
	ff_block_end(link, 0, 0xE5B8F87B);
	i = delay; while(i--);
	ff_control(link, 0x0);
}




/*void flood()
{
	pointer BLOCK = (uint*) FF_SPACE_BASE;
	pointer DMA_CTRL = (uint*) DMA_CTRL_BASE;
	pointer IMAGE = (uint*) DTCM_IMAGE_SPACE;
	uchar i, j, k;

	for(i = 0; i < LINKS; i++)
	{
		if(link_status[i] != FF_PENDING) continue;
		
		link_status[i] = FF_ACTIVE;
		ff_start(i, block_count);
		
		for(j = 0; j < block_count; j++)
		{
			ff_block_start(i, j, word_count);
			
			dma((uint) BLOCK, (uint) (IMAGE + j * word_count),
										DMA_CRC_ON, DMA_WRITE, FF_SPACE_SIZE - 4);
			while(DMA_CTRL[DMA_CTRL_STAT] & DMA_IN_PROGRESS);
			
			for(k = 0; k < word_count; k++)
			{
				ff_block_data(i, j, k, IMAGE[j * word_count + k]);
			}
			ff_block_end(i, j, BLOCK[word_count]);
		}
		ff_control(i, 0x0); //TODO pass parameters
	}
	flood_complete = time;
}*/


/*

if(DEBUGPRINTF) printf("Sending FF_START\n");
	ff_start(3, 1);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_START\n");
	ff_block_start(3, 0, 22);
	
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 0\n");
	ff_block_data(3, 0, 0, 0xE59FF04C); //Program to switch red 0 on in slave chips
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 1\n");
	ff_block_data(3, 0, 1, 0xE1A00000);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 2\n");
	ff_block_data(3, 0, 2, 0xE1A00000);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 3\n");
	ff_block_data(3, 0, 3, 0xE1A00000);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 4\n");
	ff_block_data(3, 0, 4, 0xE1A00000);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 5\n");
	ff_block_data(3, 0, 5, 0xE1A00000);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 6\n");
	ff_block_data(3, 0, 6, 0xE1A00000);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 7\n");
	ff_block_data(3, 0, 7, 0xE1A00000);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 8\n");
	ff_block_data(3, 0, 8, 0xE3A014F2);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 9\n");
	ff_block_data(3, 0, 9, 0xE3A02044);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 10\n");
	ff_block_data(3, 0, 10, 0xE7910002);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 11\n");
	ff_block_data(3, 0, 11, 0xE3C000C3);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 12\n");
	ff_block_data(3, 0, 12, 0xE7810002);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 13\n");
	ff_block_data(3, 0, 13, 0xE3A0204C);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 14\n");
	ff_block_data(3, 0, 14, 0xE7910002);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 15\n");
	ff_block_data(3, 0, 15, 0xE3A000C3);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 16\n");
	ff_block_data(3, 0, 16, 0xE7810002);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 17\n");
	ff_block_data(3, 0, 17, 0xE3A02048);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 18\n");
	ff_block_data(3, 0, 18, 0xE3A00002);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 19\n");
	ff_block_data(3, 0, 19, 0xE7810002);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 20\n");
	ff_block_data(3, 0, 20, 0xEAFFFFFE);
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_DATA 21\n");
	ff_block_data(3, 0, 21, 0x00000020);
	
	if(DEBUGPRINTF) printf("Sending FF_BLOCK_END\n");
	ff_block_end(3, 0, 0xE5B8F87B);
	if(DEBUGPRINTF) printf("Sending FF_CONTROL\n");
	ff_control(3, 0x0);
	*/