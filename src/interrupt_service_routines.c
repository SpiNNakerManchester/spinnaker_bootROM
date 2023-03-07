/*****************************************************************************************
*	interrupt_service_routines.c - functions in this file are called in the event of a
*	corresponding interrupt. Aside from the processor initialisation that takes place in
*	main.c, all computation phase 0 of SpiNNaker bootup takes place in these functions,
*	or functions called by these functions.
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

#include "../inc/copier.h"
#include "../inc/dma.h"
#include "../inc/eth_link.h"
#include "../inc/flood_fill.h"
#include "../inc/globals.h"
#include "../inc/phy.h"
#include "../inc/processor.h"

#if DEBUG
#include "../inc/debug.h"
#include <stdio.h> // To enable printing during debug
#endif


uchar generate_check(uint a, uint b) // this routine generate a 4bit 1s complement checksum over a NN packet
{
	/*
	uchar i, check = 0;
	for(i = 0; i < 7; i++) check += (a >> (i * 4)) & 0xF;		// sum over the 7 non-chk nibbles in the key data
	for(i = 0; i < 8; i++) check += (b >> (i * 4)) & 0xF;	// add the sum over the 8 data nibbles
	while(check & 0xF0) check = (check & 0xF) + (check >> 4);	// loop until left with a single hex digit checksum
	return 0xF - check;											// complement this value - returning a 4bit 1's complement checksum
	*/

	uint o;

	__asm
	{
		bic		a, a, 0xf0000000	// Clear checksum nibble
		adds	o, a, b				// s = a + b
		addcs	o, o, #1			// Add back carry
		adds	o, o, o, lsl #16	// s = s + s << 16
		addcs	o, o, #0x00010000	// Add back carry
		bic		o, o, #0x0000ff00	// Ensure no carry in
		adds	o, o, o, lsl #8		// s = s + s << 8
		addcs	o, o, #0x01000000	// Add back carry
		bic		o, o, #0x00ff0000	// Ensure no carry in
		adds	o, o, o, lsl #4		// s = s + s << 4
		addcs	o, o, #0x10000000	// Add back carry
		and		o, o, #0xf0000000	// Isolate checksum
		eor		o, o, #0xf0000000	// Complement top 4
	}

	return (o >> 28);
}


/* Called on receipt of a packet on the inter-chip links. If the packet is an NN with no
 * parity errors then its 4 bit checksum is calculated. If this is correct, and the
 * packet is of a flood fill type, it is passed to the appropriate handler function. */
__irq void cc_rx_isr()
{
	pointer COMMS_CTRL = (unsigned int*) COMMS_CTRL_BASE;
	pointer VIC = (unsigned int*) VIC_BASE;
	unsigned char new_check = 0;
	unsigned int image_length = 0;
	unsigned int rx_status = COMMS_CTRL[COMMS_CTRL_RX_STATUS];
	unsigned int data = COMMS_CTRL[COMMS_CTRL_RECEIVE_DATA];
	unsigned int key = COMMS_CTRL[COMMS_CTRL_RECEIVE_KEY]; 		// key is read sensitive, read last!


	if(VALID_NN_PACKET(rx_status))								// checks a valid NN packet (other types mean nothing to bootrom)
	{
		#ifdef DEBUG
	   		printf("Packet received:  Status:0x%x,  Key:0x%x,  Data:0x%x\n",rx_status,key,data);
		#endif

		if(NN_PAYLOAD(rx_status))	new_check = generate_check(key, data);					// get calculated checksum including payload
		else						new_check = generate_check(key, 0);

		if((FF_CHECKSUM(key) == new_check) && (FF_PACKET_ID(key) == FF_PHASE_1))	// if received & calculated checksums match
	    {
			switch(FF_OPCODE(key))													// look at the packet opcode to see what to do
			{
				case FF_START:			{cc_ff_start(data);					break;}
				case FF_BLOCK_START:	{cc_ff_block_start(key);			break;}
	      		case FF_BLOCK_DATA:		{cc_ff_block_data(key, data);		break;}
				case FF_BLOCK_END:		{cc_ff_block_end(key, data);		break;}
				case FF_CONTROL:		{image_length = cc_ff_control(key);	break;}
				#ifdef DEBUG
				case ITUBOTRON_CC:		{itube_cc_handler(data);			break;}
				#endif
				default:				{									break;}
			}
		}
	}

	if(COMMS_CTRL[COMMS_CTRL_RX_STATUS] & CC_ERR_PACKET) // if the packet was in error (parity/framing)
	{
		COMMS_CTRL[COMMS_CTRL_RX_STATUS] = CC_ERR_PACKET;							// Clear the error status now as dealt with
	}																				// in order to allow receiving packets again

	if(image_length)
	{
		#ifdef DEBUG
	   		printf("Executing at: 0x%x.\n",data);
		#endif
		VIC[VIC_VECT_ADDR] = 0x1;
		execute_image(DTCM_IMAGE_SPACE, image_length, 0x0, data);
	}

	VIC[VIC_VECT_ADDR] = 0x1;  					// clear Comms Controller RX interrupt from VIC
}


// call receive frame to stick it all the rx_frame buffer
// If it's a spin message
//		if the MAC address isn't configured, do so to allow using unicast
//		switch to function based on spinn instruction
// elif its an arp
//		Deal with it
// elif its a ping
//		deal with it
/* Called on receipt of a packet on the ethernet link... */
__irq void eth_rx_isr()
{
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;
	pointer VIC = (unsigned int*) VIC_BASE;
	unsigned int image_length = 0;

	while(ETH_MII[ETH_MII_GENERAL_STATUS] & 0x7E)	// Loop around all packets in the Ethernet buffer
	{
		receive_frame((unsigned int) rx_frame);			// populate received frame into rx_frame array (DTCM)

		if(IP(rx_frame))							// if the frame received is validated as an IP packet
		{
			if(UDP(rx_frame) && SPINN(rx_frame) && SPINN_PROTO_V1(rx_frame))	// check that it's UDP & SpiNNaker packet format v1
			{
				if(!macDestAddressConfigured)		// if the MAC addr of the SpiNNaker server not yet been populated then learn it
				{
					destinationMACAddressLS = rx_frame[2];						// populate 4LSBytes of server MAC
					destinationMACAddressHS = rx_frame[1] & 0xFFFF;				// populate 2MSBytes of server MAC
					destinationIPAddress = (((rx_frame[6] & 0xFFFF) << 16) | ((rx_frame[7] & 0xFFFF0000)>>16));
																			// populate the server IP address as SpiNNakers dstIP
					macDestAddressConfigured = 1;								// mark addresses of server found
				}

				switch(SPINN_INSTRCTN(rx_frame))								// look at the packet instruction to see what to do
				{
					case FF_START:		{eth_ff_start();					break;}	// Eth Flood Fill Start Packet
					case FF_BLOCK_DATA:	{eth_ff_block_data();				break;} //  Eth Flood Fill Block
					case FF_CONTROL:	{image_length = eth_ff_control();	break;}	// populate address to execute from
					default:			{									break;} // do nothing with other eth frames
				}
			}
			else if(ICMP(rx_frame) && ICMP_ECHO(rx_frame))
			{
				handlePingRequest();											// turn around a ping reply to the request
			}
		}
		else if(ARP(rx_frame) && (((rx_frame[9] << 16) | (rx_frame[10] >> 16)) == sourceIPAddress))
		{
			handleArpRequest();
		}

		if(image_length)
		{
			VIC[VIC_VECT_ADDR] = 0x1;
			execute_image(DTCM_IMAGE_SPACE, image_length, 0x0, SPINN_INSTRCTN_OP_3(rx_frame));
		}

		ETH_MII[ETH_MII_INT_CLR] = ETHERNET_CLR_RX_INTERRUPT; // Clear any new IRQ
	}

	VIC[VIC_VECT_ADDR] = 0x1;
}


//Function to notify fascicles of availability of image. Via mailbox message passing.
//If security code in mailbox is correct: DMA block into TCM at appropriate position, or
//  copy complete verified image to ITCM & begin executing image (depending on opcode)
__irq void sys_ctrl_isr()
{
	pointer MAILBOX = (unsigned int*) MAILBOX_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer VIC = (unsigned int*) VIC_BASE;

	unsigned int opcode = MAILBOX[MAILBOX_OPCODE];
	unsigned int source = MAILBOX[MAILBOX_COPY_SOURCE];
	unsigned int destination = MAILBOX[MAILBOX_COPY_DESTINATION];
	unsigned int execution = MAILBOX[MAILBOX_EXECUTE_ADDRESS];
	unsigned int length;

	if((opcode & 0xFFF00000) == SECURITY_CODE) 									// checks security code in mailbox is correct
	{
		length = (opcode & 0xFFFF);												// length in bytes
		if (length%4) length+=3;												// if length not exactly word divisible, add 3,
																				// this means when dividing by 4 partial words included
		if(((opcode & 0x000F0000) >> 16) == MAILBOX_FASCICLE_COPY)
		{ 			// if opcode data block to copy
			dma(source, destination, DMA_CRC_OFF, DMA_READ, length); 			// opcode & 0xFFFF == length in bytes, source/dest above
		}
		if(((opcode & 0x000F0000) >> 16) == MAILBOX_FASCICLE_EXECUTE)
		{  		// if opcode is execute command
			SYS_CTRL[SYS_CTRL_CLR_CPU_IRQ] = SECURITY_CODE | (0x1 << PROCESSOR_ID);	// clear system controller interrupt for my core
			VIC[VIC_VECT_ADDR] = 0x1;  										// clear System Controller interrupt from the VIC
			execute_image(source, (length>>2), destination, execution);
		}
	}

	SYS_CTRL[SYS_CTRL_CLR_CPU_IRQ] = SECURITY_CODE | (0x1 << PROCESSOR_ID);		// clear system controller interrupt for my core
	VIC[VIC_VECT_ADDR] = 0x1;  												// clear System Controller interrupt on VIC
}


/* Called when timer interrupt goes high (notionally every 1ms). First clears the
 * interrupt, then increments the time. The watchdog is periodically refreshed and an LED
 * is wiggled to show liveness. Ethernet connected chips also send a hello frame every
 * HELLO_FREQUENCY. Finally, an arbitrary value is written to the VIC to denote 'done'. */
__irq void timer_isr()
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer TIMER = (unsigned int*) TIMER_BASE;
	pointer VIC = (unsigned int*) VIC_BASE;
	unsigned int led_stat;

	TIMER[TIMER_1_INT_CLR] = 0x1;

  	time++;

	if(!(time % LED_FLASH_INTERVAL))
	{
		led_stat = SYS_CTRL[SYS_CTRL_GPIO_SET];
		SYS_CTRL[SYS_CTRL_GPIO_SET] = LED_0;
		SYS_CTRL[SYS_CTRL_GPIO_CLR] = led_stat & LED_0;
	}

	if(PHY_PRESENT && !(time % HELLO_FREQUENCY))
	{
		send_hello_frame();
	}

	VIC[VIC_VECT_ADDR] = 0x1;
}
