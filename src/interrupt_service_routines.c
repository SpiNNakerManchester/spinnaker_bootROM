/*****************************************************************************************
*	interrupt_service_routines.c - functions in this file are called in the event of a
*	corresponding interrupt. Aside from the processor initialisation that takes place in
*	main.c, all computation phase 0 of SpiNNaker bootup takes place in these functions,
*	or functions called by these functions.
*
*
*	Created by Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#include "../inc/dma.h"
#include "../inc/eth_link.h"
#include "../inc/flood_fill.h"
#include "../inc/globals.h"
#include "../inc/phy.h"
#include "../inc/leds.h"
#include "../inc/chipnumbering.h"
#include "../inc/debugprintf.h"
#include "../inc/copier.h"
#include "../inc/debugprintf.h"
#include "../inc/flood_sender.h"
#include "../inc/processor.h"


#include <stdio.h> // To enable printing during debug


/* Called on receipt of a packet on the inter-chip links. If the packet is an NN with no
 * parity errors then its 4 bit checksum is calculated. If this is correct, and the 
 * packet is of a flood fill type, it is passed to the appropriate handler function. */
__irq void cc_rx_isr()
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer COMMS_CTRL = (unsigned int*) COMMS_CTRL_BASE;
	pointer VIC = (unsigned int*) VIC_BASE;
	unsigned char new_check = 0;
	unsigned int image_length = 0;
	unsigned int execute_address = 0;
	unsigned int rx_status = COMMS_CTRL[COMMS_CTRL_RX_STATUS];
	unsigned int data = COMMS_CTRL[COMMS_CTRL_RECEIVE_DATA];
	unsigned int key = COMMS_CTRL[COMMS_CTRL_RECEIVE_KEY]; 		// key is read sensitive, read last!
	
	#ifdef TEST_BOARD
		if (!phy_present) sparkle_leds();								// flash the LEDs if I receive a packet
	#endif
	
	if(VALID_NN_PACKET(rx_status))								// checks a valid NN packet (other types mean nothing to bootrom)
	{
		if(NN_PAYLOAD(rx_status)) {								// if I have a payload
			new_check = generate_check(key, data);					// get calculated checksum including payload
		} else {
			new_check = generate_check(key, 0);						// otherwise checksum over key only
		}
		
		// if(DEBUGPRINTF) printf("cc_rx_isr - opcode 0x%x\n",FF_OPCODE(key));		//debug	
		
	    if((FF_CHECKSUM(key) == new_check) && (FF_PACKET_ID(key) == FF_PHASE_1))	// if received & calculated checksums match
	    {																			// and packet is ID'd as FloodFill Ph1, then listen
			
			switch(FF_OPCODE(key))													// look at the packet opcode to see what to do
			{
				case FF_START:			{cc_ff_start(data);						break;}		// Flood Fill Start Packet
				case FF_BLOCK_START:	{cc_ff_block_start(key);				break;}		//  FF Block Start Packet
	      		case FF_BLOCK_DATA:		{cc_ff_block_data(key, data);			break;}		//	 FF Block Data Packet 
				case FF_BLOCK_END:		{cc_ff_block_end(key, data);			break;}		//  FF Block End Packet
				case FF_CONTROL:		{image_length=cc_ff_control(key);					// FF End/Control Packet. If complete then	
											if (image_length) execute_address=data; break;}	//  image has length. Populate Execute addr 

				#ifdef TEST_BOARD															// ROM will not require these 2 test types 
				  case CHIP_NUMBER_CC:	{chip_num_handler(data, rx_status);		break;}		// a NN packet to number the chip
				  case ITUBOTRON_CC:	{itubecchandler((data&0xFFFF0000)>>16,(data&0xFF00)>>8,(data&0xFF));	break; } 
				#endif															// a printf message for itubotron from remote chip

				default:				{										break;}		// anything else - do nothing
			}
	
		}
			   
	}
	if(COMMS_CTRL[COMMS_CTRL_RX_STATUS] & CC_ERR_PACKET) {							// if the packet was in error (parity/framing)									
		if (DEBUGPRINTF&&(phy_present)) printf(" **** Error on RX Packet ****\n");		// print a message (no further bootrom action)
		COMMS_CTRL[COMMS_CTRL_RX_STATUS] = CC_ERR_PACKET;							// Clear the error status now as dealt with
	}  																				// in order to allow receiving packets again
	
	// if(image_length&&!phy_present) LED_FLASH_INTERVAL = 0x100;								// if image OK flash LEDs quickly (spoke chips)
	if(image_length&&DEBUGPRINTF&&phy_present) printf("All received and understood. %u bytes.\n",image_length*4); 
													// Eth attach chip will print a message to say image received OK before reboot

	#ifndef TEST_REPEAT_FLOOD_FILL_IMAGE_OUT		// if testing flood fill we don't want to reboot as more iterations are to come 
		if(image_length) {
//			if (image_length%4) image_length+=3;	// if length not exactly word divisible, add 3, 
													// this means when processing words, partials are included
			execute_image_protected(DTCM_IMAGE_SPACE, image_length, 0x0, execute_address);
		}					// call routine (outside ITCM) to copy image from DTCM location into ITCM and execute
							// sends source, length (in words), destination & execute address to the 'protected' copier routine
							// Parameters r0 = image source, r1 = image size (words), r2 = target address, r3 = execute_address
	#endif
	
	VIC[VIC_VECT_ADDR] = 0xF00;  					// clear Comms Controller RX interrupt from VIC
	
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
  pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
  unsigned int image_length = 0;
  unsigned int execute_address;
	
  #ifdef APPTEST
//	printf("E");
  #endif 

  while(ETH_MII[ETH_MII_GENERAL_STATUS]&0x7E) {		// Loop around all packets in the Ethernet buffer	
	
	receive_frame((unsigned int) rx_frame);			// populate received frame into rx_frame array (DTCM)
	
	if(!((rx_frame[1] & 0xFFFF) == sourceMACAddressHS) && !(rx_frame[2] == sourceMACAddressLS)) 
	{ 												// ensure we don't listen to our own looped back frames
		
		if(IP(rx_frame))							// if the frame received is validated as an IP packet
		{
			if(UDP(rx_frame) && SPINN(rx_frame) && SPINN_PROTO_V1(rx_frame))	// check that it's UDP & SpiNNaker packet format v1
			{
				if(!macDestAddressConfigured)		// if the MAC addr of the SpiNNaker server not yet been populated then learn it
				{
					destinationMACAddressLS = rx_frame[2];						// populate 4LSBytes of server MAC
					destinationMACAddressHS = rx_frame[1] & 0xFFFF;				// populate 2MSBytes of server MAC
					destinationIPAddress = ( ((rx_frame[6] & 0xFFFF) << 16) | ((rx_frame[7] & 0xFFFF0000)>>16) );
																				// populate the server IP address as SpiNNakers dstIP  
					macDestAddressConfigured = 1;								// mark addresses of server found
				}
				if (DEBUGPRINTF&&phy_present) printf("OpCode: 0x%x\n",SPINN_INSTRCTN(rx_frame));			// print OpCode just received
				
				switch(SPINN_INSTRCTN(rx_frame))								// look at the packet instruction to see what to do
				{
					case FF_START:		{eth_ff_start();									break;}	// Eth Flood Fill Start Packet
					case FF_BLOCK_DATA:	{eth_ff_block_data();								break;} //  Eth Flood Fill Block
					case FF_CONTROL:	{image_length = eth_ff_control();							// Eth Flood Fill control/end Pck
										if (image_length)											// if complete, image has length
											execute_address=SPINN_INSTRCTN_OP_3(rx_frame);	break;}	// populate address to execute from
					default:			{													break;} // do nothing with other eth frames
				}
				if (DEBUGPRINTF&&phy_present) printf("S");							// print message indicating SpiNNaker packet processed
			}
			
			
			else if(ICMP(rx_frame) && ICMP_ECHO(rx_frame)) {					// If not SpiNNaker, then if ICMP Echo Request (ping)
				handlePingRequest();											// turn around a ping reply to the request
				if (DEBUGPRINTF&&phy_present) printf("P");							// print message indicating ping packet processed
			}
		}
		else if(ARP(rx_frame) && (((rx_frame[9] << 16) | (rx_frame[10] >> 16)) == sourceIPAddress))	{
							// if ARP request for my IP (these are broadcast to MAC 0xFFFFFFFFFFFF hence why not in previous brace)
			handleArpRequest();													// reply to the ARP request appropriately with my MAC												
			if (DEBUGPRINTF&&phy_present) printf("A"); 								// print message indicating ARP packet processed
		}
	}
	ETH_MII[ETH_MII_INT_CLR] = ETHERNET_CLR_RX_INTERRUPT; 						// clear Eth Receive Interrupt, now dealt with


	if(image_length) {
		if (DEBUGPRINTF) printf("Start: 0x%x, Len(words): 0x%x, Exec:0x%x\n",DTCM_IMAGE_SPACE, image_length, execute_address);
		if (DEBUGPRINTF) printf("EthImageOK - rebootingNOW!\n");				// print a message to say image OK before reboot
		if (phy_present) send_itubotron_message();  									// flush any last messages out of the Ethernet 
//		if (image_length%4) image_length+=3;									// if length not exactly word divisible, add 3, 
																				// this means when processing words, partials inclusive
		//	for (i=0;i<image_length;i++) printf("0x%x) 0x%x\n",i,IMAGE[i]);
		//	printf("Start: 0x%x, Len(words): 0x%x, Exec:0x%x\n",DTCM_IMAGE_SPACE, image_length, execute_address);
		VIC[VIC_VECT_ADDR] = 0xF00;  											// clear Eth RX interrupt from VIC
		execute_image_protected(DTCM_IMAGE_SPACE, image_length, 0x0000 ,execute_address);	
							// call routine (outside ITCM) to copy image from DTCM location into ITCM and execute
							// sends source, length (in words), destination & execute address to the 'protected' copier routine
							// Parameters r0 = image source, r1 = image size (words), r2 = target address, r3 = execute_address

	}
	
  }	
  VIC[VIC_VECT_ADDR] = 0xF00;  												// clear Eth RX interrupt from VIC
	
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
//	pointer SYSRAM_FF_BLOCK = (unsigned int*) FF_SPACE_BASE;
//	pointer TESTIMAGETCM = (unsigned int*) DTCM_IMAGE_SPACE;
	
	unsigned int opcode = MAILBOX[MAILBOX_OPCODE];
	unsigned int source = MAILBOX[MAILBOX_COPY_SOURCE];
	unsigned int destination = MAILBOX[MAILBOX_COPY_DESTINATION];
	unsigned int execution = MAILBOX[MAILBOX_COPY_EXECUTION];
	unsigned int length;
	
	if(DEBUGPRINTF) printf("SYS ISR with opcode: 0x%x\n",opcode);			// print message to say I'm in the SystemCtrl ISR
		
	if((opcode & 0xFFF00000) == SECURITY_CODE) 									// checks security code in mailbox is correct
	{
		length = (opcode & 0xFFFF);												// length in bytes
		if (length%4) length+=3;												// if length not exactly word divisible, add 3, 
																				// this means when dividing by 4 partial words included
		if(((opcode & 0x000F0000) >> 16) == MAILBOX_FASCICLE_COPY) { 			// if opcode data block to copy  
			dma(source, destination, DMA_CRC_OFF, DMA_READ, length); 			// opcode & 0xFFFF == length in bytes, source/dest above
			if(DEBUGPRINTF) printf("!!COPY!! ImgSrc:0x%x, ImgDst:0x%x, Size:0x%x words.\n",source, destination, ((opcode & 0xFFFF)>>2));
			if(DEBUGPRINTF) printf("0x%x): 0x%x.\n",destination,*(unsigned int *)destination);  // chk blk start
			if(DEBUGPRINTF) printf("0x%x): 0x%x.\n",destination+1020,*(unsigned int *)(destination+1020)); // chk blk end
		}
		if(((opcode & 0x000F0000) >> 16) == MAILBOX_FASCICLE_EXECUTE) {  		// if opcode is execute command
			if(DEBUGPRINTF) printf("!!EXECUTE!! ImgSrc:0x%x, ImgDst:0x%x, Size:0x%x words, Exec:0x%x\n",source, destination, ((opcode & 0xFFFF)>>2), execution);
			SYS_CTRL[SYS_CTRL_CLR_CPU_IRQ] = SECURITY_CODE | (0x1 << processor_ID);	// clear system controller interrupt for my core
			VIC[VIC_VECT_ADDR] = 0xF00;  										// clear System Controller interrupt from the VIC
			execute_image_protected(source, (length>>2), destination, execution);  	// mailbox registers used for execute cmds
							// call routine (outside ITCM) to copy image from DTCM location into ITCM and execute
							// sends source, length (in words), destination & execute address to the 'protected' copier routine
							// Parameters r0 = image source, r1 = image size (words), r2 = target address, r3 = execute_address							
		}
	
	}
	SYS_CTRL[SYS_CTRL_CLR_CPU_IRQ] = SECURITY_CODE | (0x1 << processor_ID);		// clear system controller interrupt for my core
	VIC[VIC_VECT_ADDR] = 0xF00;  												// clear System Controller interrupt on VIC
}


// function handling interrupts from the timer (notionally every 1ms)
__irq void timer_isr()
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer TIMER = (unsigned int*) TIMER_BASE;
	pointer VIC = (unsigned int*) VIC_BASE;
	
	TIMER[TIMER_1_INT_CLR] = 0x18BADC0D;   			// anything written here will clear the interrupt. Therefore I ate bad cod.
	
  	time++;											// increment the global ms timer for the monitor


	if(phy_present && itubotroncount) {  				// if I can send itubotron data (eth attached) and there's something to send
		if(itubotroncount>=250 || !(time % ITUBOTRON_FREQUENCY)) {  	// if it's time to send a packet - or buffer is full already
			send_itubotron_message();				// initialise a packet and transmit itubotron printf output on the Ethernet  
		}	
	}
	
	
	if(phy_present && !(time % HELLO_FREQUENCY))			// If Ethernet attached, and if time is periodic matching hello send frequency			
	{
		send_hello_frame();							// This transmits a hello message out on the Ethernet to SpiNNaker server			
	}
	
	
	#ifdef TEST_BOARD								// this section only applies if on the testboard with the 4 LEDs per chip								
		if(!(phy_present) && (time % LED_FLASH_INTERVAL)==0)	// If I don't have an Eth I'm not attached to the ctrl board via GPIO
		{												// therefore I can flash LEDs without breaking tubotron multiplexed output
			if (LED_FLASH_INTERVAL == 0x100) sparkle_leds();   // If LED_FLASH_INTERVAL==100 - then swops about every ~250ms
			if (LED_FLASH_INTERVAL == 0x400 && !(time % (LED_FLASH_INTERVAL*8))) sparkle_leds();  // if LED_FLASH_INTERVAL==400, swops about every 2s
		}												// other frequencies or combinations could be added here
	#endif

	VIC[VIC_VECT_ADDR] = 0xF00;  						// clear Timer1 interrupt on VIC
}

