/*****************************************************************************************
*	
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#ifdef DEBUG


#include "../inc/debug.h"
#include "../inc/dma.h"
#include "../inc/globals.h"

#include <stdarg.h>
#include <stdio.h>


#pragma import(__use_no_semihosting_swi) 

struct __FILE { int handle; };
	FILE __stdout;
	FILE __stdin;


uint source_MAC_hi = 0;
uint source_MAC_lo = 0;
uint dest_MAC_hi = 0;
uint dest_MAC_lo = 0;
uint source_IP = 0;
uint dest_IP = 0;

uint eth_variables_set = 0;

uint frame[380];
uint data[256];
uint count = 0;
uint eth_packet_ID = 0;


int fputc(int ch, FILE *f)
{ 
	pointer DMA_CTRL = (uint*) DMA_CTRL_BASE;
	pointer CHIP_IDENTIFIER = (uint*) CHIP_ID;
	char tempch = ch;

	iputc_eth((*CHIP_IDENTIFIER & 0xFFFF), PROCESSOR_ID, tempch);
	
	return ch;
}


void init_eth_variables()
{
	pointer ETH_PARAMS = (uint*) ETH_PARAMS_BASE;
	
	// Set source IP and MAC addresses acording to the data copied out of serial ROM
	source_MAC_hi = (ETH_PARAMS[0] >> 24) | ((ETH_PARAMS[0] & 0xFF0000) >> 8);
	source_MAC_lo = (ETH_PARAMS[1] >> 24) | ((ETH_PARAMS[1] & 0xFF0000) >> 8) |
					((ETH_PARAMS[1] & 0xFF00) << 8) | (ETH_PARAMS[1] << 24);
	source_IP = (ETH_PARAMS[2] >> 24) | ((ETH_PARAMS[2] & 0xFF0000) >> 8) |
				((ETH_PARAMS[2] & 0xFF00) << 8) | (ETH_PARAMS[2] << 24);	
	
	// Set destinations to broadcast addresses
	dest_IP = 0xFFFFFFFF;
	dest_MAC_hi = 0xFFFF;
	dest_MAC_lo = 0xFFFFFFFF;
}


void iputc_eth(uint chip_id, uint proc_id, char c)
{
	data[count++] = (chip_id << 16) | (proc_id << 8) | c;
	
	if(count >= 250 || c == '\n')
	{
		send_print_frame(ITUBOTRON, ITUBE_CMD << 16, count * 4, chip_id);
		count = 0;
	}
}


int send_print_frame(uint instruction, uint command, uint payload_length, uint chip_id)
{
	pointer ETH_MII = (uint*) ETH_MII_BASE;
	int length = payload_length + 60;
	uint sum = 0, count = 0, i;
	
	if(!eth_variables_set)
	{
		init_eth_variables();
		eth_variables_set = 1;
	}
   
	frame[0] = (dest_MAC_hi & 0xFFFF)<<16 | (dest_MAC_lo & 0xFFFF0000)>>16;	//4 MSBytes of  Dest MAC address
	frame[1] = (dest_MAC_lo & 0xFFFF)<<16 | (source_MAC_hi & 0xFFFF);		//2 LSBytes of DMAC, first 2 MSB of SMAC
	frame[2] = source_MAC_lo;												//4 LSBytes of S MAC address
	frame[3] = (IP_ETHERTYPE<<16) | (IP_VER_LEN_TOS);     					//ethernet type, IP version, header len, type of service
	frame[4] = ((( 20 + 8 + 18 + payload_length) << 16) | eth_packet_ID++);	//IP Sz = 20(IP)+8(UDP)+18(SpiNN)+SpiNPayload(Extras),PktID                                          
	frame[5] = IP_FRAG_FLAGS_TTL_PROT;										//Frag, flags, offset, TTL, IP Protocol UDP
	frame[6] = 0x0 | (source_IP & 0xFFFF0000) >> 16;						//IP Hdr Chk (init 0 - see later) | 2 octets SrcIP address   
	frame[7] = (source_IP  & 0xFFFF) << 16 | (dest_IP & 0xFFFF0000) >> 16;	//last 2 octets SrcIP, 1st two of DstIP
	frame[8] = (dest_IP  & 0xFFFF) << 16 | SPINN_PORT;						//last 2 octets of DstIP, Source UDP port                                       
	frame[9] = ((uint) SPINN_PORT << 16)  | (8 + 18 + payload_length);		//Dest UDP port, UDP Length (UDP Headers+SpiNN+Extras)                                           
	frame[10] = 0x0 | SPINNPROTVERSION;										//Chk UDP Pseudo Hdr = 0 (as optional) | SpiNNaker Protocol Version
	
	// Calculate the IP header checksum over the 20byte header
	for(count = 3; count < 8; count++)
	{
		sum += ((frame[count] & 0x0000FFFF) + ((frame[count + 1] & 0xFFFF0000) >> 16));
	}
	while(sum & 0xFFFF0000) 
	{
		sum = (sum & 0xFFFF) + (sum >> 16); // Sum 2 shorts until no carry remains
	}    
	count = 0xFFFF - sum; // Invert the result
	
	frame[6] |= (count << 16); // Insert IP checksum

	frame[11] = instruction;
	frame[12] = command;
	frame[13] = payload_length;
	frame[14] = chip_id;
	
	for(i = 0; i < (payload_length / sizeof(int)); i++) frame[15 + i] = data[i];
	
    if((ETH_MII[ETH_MII_GENERAL_STATUS] & ETH_MII_TX_ACTIVE) || (length > 1500+14))  
    {
    	return 0; 		// h/w busy or length too long to be a valid Ethernet frame
    }
    
    ETH_MII[ETH_MII_TX_LENGTH] = ((length < 60) ? 60 : length); // Pad frame to 60 bytes
    if(length % 4) length += (4 - (length % 4)); // Pad length to be multiple of word size 
       			
    dma(ETH_MII_BASE + ETH_MII_TX_FRAME_BUFFER, (uint) frame, DMA_CRC_OFF, DMA_WRITE, length);     		

    ETH_MII[ETH_MII_TX_COMMAND] = 0x1; // Send frame
    
    return 1;
}


void _sys_exit(void)
{
	while(1);
}


#endif
