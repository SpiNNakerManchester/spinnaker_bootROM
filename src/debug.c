/*
 * Copyright (c) 2008 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef DEBUG


#include "../inc/debug.h"
#include "../inc/dma.h"
#include "../inc/eth_link.h"
#include "../inc/globals.h"

#include <stdarg.h>
#include <stdio.h>


#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; };
	FILE __stdout;
	FILE __stdin;


uint data[256];
uint count = 0;


int fputc(int ch, FILE *f)
{
	#ifdef TEST_CHIP
	pointer DMA_CTRL = (uint*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (uint*) SYS_CTRL_BASE;

	if(PHY_PRESENT)
	{
		iputc_eth(0xFFFF, PROCESSOR_ID, ch);
	}
	else
	{
		iputc_cc(0xFFFF, PROCESSOR_ID, ch);
	}
	#endif

	#ifdef SPINNAKER2
	pointer SYS_CTRL = (uint*) SYS_CTRL_BASE;
	SYS_CTRL[0] = ch;
	#endif

	return ch;
}


void iputc_cc(uint chip_id, uint proc_id, char ch)
{
	pointer COMMS_CTRL = (uint*) COMMS_CTRL_BASE;
	uint i, link = 0, key, data, check = 0;

	link = 4;

	key = ITUBOTRON_CC << 24 | FF_PHASE_1;
	data = (chip_id << 16) | (proc_id << 8) | ch;

	for(i = 0; i < 7; i++) check += (key >> (i * 4)) & 0xF;
	for(i = 0; i < 8; i++) check += (data >> (i * 4)) & 0xF;
	while(check & 0xF0) check = (check & 0xF) + (check >> 4);
	check = 0xF - check;

	i = 0x1000;
	while(i--);

	while((COMMS_CTRL[COMMS_CTRL_TX_CTRL] & TX_BUFFER_EMPTY) == 0);

	COMMS_CTRL[COMMS_CTRL_TX_CTRL] = NN_PACKET_PAYLOAD(link);;
	COMMS_CTRL[COMMS_CTRL_TRANSMIT_DATA] = data;
	COMMS_CTRL[COMMS_CTRL_TRANSMIT_KEY] = (check << 28) | key;
}


void iputc_eth(uint chip_id, uint proc_id, char c)
{
	int i = 0;

	data[count++] = (chip_id << 16) | (proc_id << 8) | c;

	if(count >= 250 || c == '\n')
	{
		for(i = 0; i < count; i++) tx_frame[15 + i] = data[i];
		formatAndSendFrame(ITUBOTRON, ITUBE_CMD << 16, count * 4, chip_id);
		count = 0;
	}
}


void itube_cc_handler(uint data)
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	uint chip_id = data >> 16;
	uint proc_id = (data & 0xFF00) >> 8;
	char ch = data & 0xFF;

	if(PHY_PRESENT)
	{
		iputc_eth(chip_id, proc_id, ch);
	}
	else
	{
		iputc_cc(chip_id, proc_id, ch);
	}
}


void _sys_exit(void)
{
	while(1);
}


#endif
