/*****************************************************************************************
*	testers.c - these are functions that as per the label, test out functions from within
*	 the bootrom, but are not intrinsicly part of the bootrom itself.
*
*	Created by Cameron Patterson   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

		
#include "../inc/globals.h"
#include "../inc/peripheral_inits.h"
#include "../inc/processor.h"
#include "../inc/dma.h"
#include "../inc/leds.h"
#include "../inc/flood_sender.h"
#include "../inc/poker.h"
#include "../inc/ivb.h"
#include "../inc/chipnumbering.h"
#include "../inc/ram_test.h"
#include "../inc/testers.h"


// Routines to enable printing during debug
#include <stdio.h>
#include <stdarg.h>


#ifndef SPINNAKER2			// none of this testing will happen on the real S2 chip!



// This test has monitor processors on all chips printf a message every 100ms 
//      We attempt to slight randomise the time based on chipID to try and avoid synchronising	
#ifdef TEST_MULTICHIP_PRINTF
void testers_MULTICHIP_PRINTF() 
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer CHIP_IDENTIFIER = (uint*) CHIP_ID;
	pointer VIC = (unsigned int*) VIC_BASE;
	
	#ifdef APPTEST
		pointer ETH_MII = (uint*) ETH_MII_BASE;
		if (monitor && ( ! ((time + ((*CHIP_IDENTIFIER<<2) &0xFF) + ((*CHIP_IDENTIFIER>>4) &0xFF)) % 1000) )  ) 
			if(DEBUGPRINTF) printf("%ds) 2nd Phase of boot!  VIC[MASKED]:0x%x. VIC[RAW]:0x%x. VIC[ADDR]:0x%x.   ETH[GEN_STAT]:0x%x.\n",
						time/1000, VIC[0], VIC[2], VIC[9], ETH_MII[ETH_MII_GENERAL_STATUS]); 
	#endif
	#ifndef APPTEST
		pointer ETH_MII = (uint*) ETH_MII_BASE;
		pointer DETECTED_SDRAM_INFO = (unsigned int*) DETECTED_SDRAM_INFO_BASE;
		if (monitor && ( ! ((time + ((*CHIP_IDENTIFIER<<2) &0xFF) + ((*CHIP_IDENTIFIER>>4) &0xFF)) % 4000) )  ) 
			if(DEBUGPRINTF) printf("%ds) quick brown fox jumps after the lazy dog. SDRAMsz:0x%x, Errs:0x%x. ETH[GEN_STAT]:0x%x.\n",
						time/1000, DETECTED_SDRAM_INFO[0], DETECTED_SDRAM_INFO[1],ETH_MII[ETH_MII_GENERAL_STATUS]); 
	#endif
		                                     
}
#endif


// This routine tests message passing from monitor processor to fascicles (which are asleep).
//   The monitor populates the message passing MAILBOX on each chip, and sends a system controller interrupt to 
//   wake the fascicles. 
// Each second a block of data is sent for copying.  (simulating a block of an image arriving)
// After each chip has been active 75 seconds, a mailbox message is sent to branch to 0x0   (emulating an new image loaded)
//
//   A debug "printf" is also supplied for the fascicles to print that they have woken & actioned the block/instruction from the mailbox
#ifdef TEST_FASCICLE_WAKE_AND_BOOT
void testers_FASCICLE_WAKE_AND_BOOT()
{		
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer TESTIMAGETCM = (unsigned int*) DTCM_IMAGE_SPACE;
	pointer MAILBOX = (unsigned int*) MAILBOX_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer SYSRAM_FF_BLOCK = (unsigned int*) FF_SPACE_BASE;
	unsigned int i;

	if(monitor && !(time%1000)) {								// monitor performs this test every second
		for (i=0 ; i<256 ; i++) SYSRAM_FF_BLOCK[i] = ((((i*4)+3)%256)<<24)|((((i*4)+2)%256)<<16)|((((i*4)+1)%256)<<8)|(i*4)%256;
			// populate a 1KB block of data repeating pattern 00->FF.
		MAILBOX[MAILBOX_OPCODE]=SECURITY_CODE|0x10000|0x0400; 	// Security Validation Code | Opcode=1 (copy block) | Blocksize = 1024
		MAILBOX[MAILBOX_COPY_SOURCE]=FF_SPACE_BASE;				// System source of data block (here the FF block, could use SDRAM)
		MAILBOX[MAILBOX_COPY_DESTINATION]=DTCM_IMAGE_SPACE;		// Target in TCM for the data (here the Image space in DTCM)
		if(DEBUGPRINTF) printf("%d) Me:%d, WakeupTo:%d. CurrentInt:0x%x,  CPUOK:0x%x\n",time, processor_ID,(processor_ID ? 1 : 2),SYS_CTRL[SYS_CTRL_SET_CPU_IRQ],SYS_CTRL[SYS_CTRL_SET_CPU_OK]);
		SYS_CTRL[SYS_CTRL_SET_CPU_IRQ] = SECURITY_CODE|(processor_ID ? 1 : 2);  // send a CPU system ctrl interrupt to fascicle core.
	}
		
	if(monitor && (time>75000)) {								// monitor performs this test at monitor uptime=75secs
		MAILBOX[MAILBOX_OPCODE]=SECURITY_CODE|0x20000|0x8000; 	// Security Validation Code | Opcode=2 (execute), Total Image Size = 32K
		MAILBOX[MAILBOX_COPY_SOURCE]=DTCM_IMAGE_SPACE;			// Base of image stored in TCM.  Will be copied into 0x0 ITCM
		MAILBOX[MAILBOX_COPY_DESTINATION]=0x0;					// what address to execute from after copying complete 
		if(DEBUGPRINTF) printf("%d) Me:%d, WakeupTo:%d. CurrentInt:0x%x,  CPUOK:0x%x\n",time, processor_ID,(processor_ID ? 1 : 2),SYS_CTRL[SYS_CTRL_SET_CPU_IRQ],SYS_CTRL[SYS_CTRL_SET_CPU_OK]);
		SYS_CTRL[SYS_CTRL_SET_CPU_IRQ] = SECURITY_CODE|(processor_ID ? 1 : 2);  // send system ctrl interrupt to 'restart' fascicle core.
	}

	if (!monitor) {
		if(DEBUGPRINTF) printf("I got woken. Mailbox Opcode: 0x%x, Src: 0x%x, Dst: 0x%x.   Placed in dest...\n",MAILBOX[MAILBOX_OPCODE],MAILBOX[MAILBOX_COPY_SOURCE],MAILBOX[MAILBOX_COPY_DESTINATION]);
		for (i=0 ; i<3 ; i++) if(DEBUGPRINTF) printf("Mem loc: 0x%x = 0x%x   or orig: 0x%x \n",(i*4)+DTCM_IMAGE_SPACE, TESTIMAGETCM[i], SYSRAM_FF_BLOCK[i]);
		// prints details of the message received on the mailbox, and compares source with original to ensure copy was successful.
	}
				
}
#endif


#ifdef TEST_FASCICLE_BOOTER
void testers_FASCICLE_BOOTER()
{		
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer TESTIMAGETCM = (unsigned int*) DTCM_IMAGE_SPACE;
	pointer MAILBOX = (unsigned int*) MAILBOX_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer SYSRAM_FF_BLOCK = (unsigned int*) FF_SPACE_BASE;
	unsigned int i,j;

	for (j=0;j<32;j++) {													// 32*1KB blocks in DTCM_IMAGE_SPACE
		for (i=0 ; i<256 ; i++) SYSRAM_FF_BLOCK[i] = TESTIMAGETCM[(256*j)+i];
			// populate next 1KB block of data from DTCM Image Block.
			
		MAILBOX[MAILBOX_OPCODE]=SECURITY_CODE|0x10000|0x0400; 	// Security Validation Code | Opcode=1 (copy block) | Blocksize = 1024
		MAILBOX[MAILBOX_COPY_SOURCE]=FF_SPACE_BASE;				// System source of data block (here the FF block, could use SDRAM)
		MAILBOX[MAILBOX_COPY_DESTINATION]=DTCM_IMAGE_SPACE+(1024*j);	// Target in TCM for the data (here the Image space in DTCM)
		if(DEBUGPRINTF) printf("%d) Me:%d, WakeupTo:%d. CurrentInt:0x%x,  CPUOK:0x%x\n",time, processor_ID,(processor_ID ? 1 : 2),SYS_CTRL[SYS_CTRL_SET_CPU_IRQ],SYS_CTRL[SYS_CTRL_SET_CPU_OK]);
		SYS_CTRL[SYS_CTRL_SET_CPU_IRQ] = SECURITY_CODE|(processor_ID ? 1 : 2);  // send a CPU system ctrl interrupt to fascicle core.
	
		while ((SYS_CTRL[SYS_CTRL_SET_CPU_IRQ]&0x3) == (processor_ID ? 1 : 2));	// wait until relevant CPU interrupt cleared
	}
		
	MAILBOX[MAILBOX_OPCODE]=SECURITY_CODE|0x20000|0x8000; 	// Security Validation Code | Opcode=2 (execute), Total Image Size = 32K
	MAILBOX[MAILBOX_COPY_SOURCE]=DTCM_IMAGE_SPACE;			// Base of image stored in TCM.
	MAILBOX[MAILBOX_COPY_DESTINATION]=0x0;					// what TCM address to copy source data to.
	MAILBOX[MAILBOX_COPY_EXECUTION]=0x0;					// what address to execute from after copying complete	 
	SYS_CTRL[SYS_CTRL_SET_CPU_IRQ] = SECURITY_CODE|(processor_ID ? 1 : 2);  // send system ctrl interrupt to 'restart' fascicle core.

}
#endif



// This test, turns the Ethernet MII off and on alternatively for 5 seconds each, testing for a alignment bug (per ST/SD)
#ifdef TEST_ETHERNET_RESETS
void testers_ETHERNET_RESETS()
{
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;
	if(phy_present && !(time%10000)) init_ethernet_MII(); // turn Ethernet MII on every 10 seconds to test MII reset - per Sergio problem
	if(phy_present && !((5000+time)%10000)) ETH_MII[ETH_MII_GENERAL_CMD] = 0x00; // set Ethernet MII off every 10sec (offset 5s from on)
}
#endif


// This test will flood a 32KB test image out from DTCM to downstream chips as specified in flood_sender.c.  
//    It repeats 32 times and mechanisms are in place in flood_fill.c to recognise the test and not reboot each time! 
#ifdef TEST_FLOOD_FILL_IMAGE_OUT
void testers_FLOOD_FILL_IMAGE_OUT()
{	
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	
	testers_TEMP_DISABLE_WATCHDOG();
	if (monitor) {
		if (DEBUGPRINTF&&phy_present) printf("\nAbout to Flood Once to Spokes\n"); 
		flood();  // flood fill the image buffer (per flood_sender.c)
		if (DEBUGPRINTF&&phy_present) printf("\nAnd Suddenly, the Floods ceased. Others should be up now.\n"); 
	}
	init_watchdog();
}
#endif


		
// This test will flood a 32KB test image out from DTCM to downstream chips as specified in flood_sender.c.  
//    It repeats 32 times and mechanisms are in place in flood_fill.c to recognise the test and not reboot each time! 
#ifdef TEST_REPEAT_FLOOD_FILL_IMAGE_OUT
void testers_REPEAT_FLOOD_FILL_IMAGE_OUT()
{	
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer TESTIMAGETCM = (unsigned int*) DTCM_IMAGE_SPACE; 
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	unsigned int i;
	
	testers_TEMP_DISABLE_WATCHDOG();
	if (phy_present && monitor) {
		if (DEBUGPRINTF&&phy_present) printf("\nAbout to Flood\n");
		for (i=0 ; i<8192 ; i++) TESTIMAGETCM[i] = ((((i*4)+3)%256)<<24)|((((i*4)+2)%256)<<16)|((((i*4)+1)%256)<<8)|(i*4)%256;
	 		// populate a 32KB block of data repeating incrementing byte pattern 00->FF (for easy validation).	
		for (i=0 ; i<32 ; i++) {
			if (DEBUGPRINTF&&phy_present) printf("%u)",i);			// printfs the flood fill cycle number from the hub monitor
			flood(); 											// flood fill the image as per the flood_sender.c routine 
		}
		if (DEBUGPRINTF&&phy_present) printf("\nAnd Suddenly, the Floods ceased. Sys Control Mux was: 0x%x.\n",SYS_CTRL[SYS_CTRL_CLK_MUX_CTRL]); 
	}
	init_watchdog();
}
#endif


// This test routine goes through the DMA bug found with block sizes copying from problematic SystemRAM to DTCM.
// it algorithmically goes through all block sizes on a known bad transfer size, printing word position errors detected in the transfer.
// This is repeated to allow for freeze can use or other diagnostic technique to determine behavioural changes
#ifdef TEST_DMA_BURST_BUG
void testers_DMA_BURST_BUG()
{	
	// this section used for testing DMA transfers & bug 
	unsigned int word_count, crcforloop, burstsize, errcnt;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer TESTIMAGETCM = (unsigned int*) DTCM_IMAGE_SPACE;  	
	pointer TESTIMAGETCM2 = (unsigned int*) DTCM_IMAGE_SPACE + 0x13C4;  
	pointer BLOCK = (uint*) 0xf5002000;
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	unsigned int i,k;
		
	if (phy_present && monitor) {										// if I am an Ethernet attached chip
 	 for (i=0 ; i<4096 ; i++) TESTIMAGETCM[i] = ((((i*4)+3)%256)<<24)|((((i*4)+2)%256)<<16)|((((i*4)+1)%256)<<8)|(i*4)%256;	
	 		// populate a 16KB block of data repeating incrementing byte pattern 00->FF (for easy validation).	
	 
	 for (i=0;i<1000;i++) {											// repeats the test 1000 times
      for (word_count=110;word_count<=110;word_count+=20) {    		// changes the size of the transfer (fixed to 110 as good test)
       for (burstsize=0; burstsize<=4; burstsize++) {				// goes through all burst sizes 2^0 -> 2^4
        for (crcforloop=1; crcforloop<=1; crcforloop++) {			// turns CRC off and on in the transfer to see if influental
      
		 dma((unsigned int) BLOCK, (unsigned int) (TESTIMAGETCM),DMA_CRC_ON, DMA_WRITE, (word_count*4));   
				// Do the DMA copy from relevant TCM location to SYSRAM buffer, and calculate the CRC
		 if (DEBUGPRINTF&&phy_present) { 
			for (k=0;k<word_count;k++)  if (BLOCK[k] != TESTIMAGETCM[k]) printf("%u) SysCopiedData: 0x%x == TCMOrigData: 0x%x.\n",k,BLOCK[k],TESTIMAGETCM[k]);
				// this systematically compares original with copied data, printing errors as detected (usually 0 as this direction OK) 
		 }
			
		// this section is specific for SysRAM to TCM (we don't use the !dma! function, as we use all parameters)
		 while(DMA_CTRL[DMA_CTRL_STAT] & DMA_Q_FULL);				// wait until DMA queue is not full
		 DMA_CTRL[DMA_CTRL_ADRS] = (unsigned int) BLOCK;				// systemRAM source for DMA transfer
		 DMA_CTRL[DMA_CTRL_ADRT] = (unsigned int) TESTIMAGETCM2;		// TCM destination for DMA transfer
		 DMA_CTRL[DMA_CTRL_DESC] = (burstsize << 0x15) |				
								(crcforloop << 0x14) |
								(DMA_READ << 0x13) |
								((word_count*4) & 0xFFFF);			// load burstsize, crc options and set tx size and direction bits
			
		 while(DMA_CTRL[DMA_CTRL_STAT] & DMA_IN_PROGRESS);  			// wait until the transfer has completed

		 if (DEBUGPRINTF&&phy_present) { 									// if I've got access to the tube	
			errcnt=0;
			for (k=0;k<word_count;k++)  {							// loops through word by word
				if ((BLOCK[k]!= TESTIMAGETCM2[k]) || (!BLOCK[k])) {	// if copy doesn't match the original
			    	if (errcnt==0) printf("Sz: %u Bst: %u) ErrWdPos:%u ",word_count, burstsize, k);		
			    	if (errcnt) printf("%u ",k);					// print details about the failing transfer and word positions
			    	errcnt++;			    						
				}
			}
			if(errcnt>=1) printf("\n");   							// print a newline for formatting if an error detected in this nest
		 }

		 if(DMA_CTRL[DMA_CTRL_STAT] & DMA_PAUSED) {					// is DMA controller in a paused state (after an error)
			if (DEBUGPRINTF&&phy_present&&!errcnt) printf("wc:%u, burst:%u, crc:%u. Mem:0x%x. DMA-calc[7]:0x%x DMA-rd[8]:0x%x\n", word_count,burstsize,crcforloop, BLOCK[word_count], DMA_CTRL[0x7], DMA_CTRL[0x8]);
			//if (word_count>7) for (j=0;j<word_count;j++)  if(DEBUGPRINTF) printf("%u) SrcData: 0x%x ?? DestData: 0x%x.\n",j,BLOCK[j],TESTIMAGETCM2[j]);
			DMA_CTRL[DMA_CTRL_CTRL] = DMA_CLEAR_ERRORS;  			// if CRC is detected and DMA controller paused - clear this away
		 }

       } // for crcloop     
      } // for burstsize
     } // for wordcount
	 k = 10000000; while (k--);  									// wait a bit before starting the loop once again
	} // outer i for loop
	
   } // if ethernet attached chip
   
}
#endif


// This test flashes the LEDs if External link packets are received, for use when watchdog/interrupts are turned off
#ifdef TEST_CC_PACKET_RECEIVED
void testers_CC_PACKET_RECEIVED()
{ 
	pointer COMMS_CTRL = (uint*) COMMS_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	uint i,j;
	if (!phy_present && monitor) {										// if I am Ethernet / tubotron attached
		j=0;
		while (1) {													// keep running forever
			while((COMMS_CTRL[COMMS_CTRL_RX_STATUS] & CC_RX_PACKET) == 0) ;  // check for parity or framing problemos with 0x60000000
			i = COMMS_CTRL[COMMS_CTRL_RECEIVE_KEY]; 				// read in key (by reading clears from buffer)
			if(!i && DEBUGPRINTF) printf("BlankKey: 0x%x\n",i); 	// print message if key is blank (0)
			if (!(j++%16)) sparkle_leds(); 							// only sparkle LEDs every 16 packets - otherwise invisible-too fast
		}
	}	 
}

// this test populates the ITCM validation block 
void testers_IBV_POPULATION()
{
	pointer IVB = (unsigned int*) ITCM_VALIDATION_BLOCK;
	unsigned int i;
	if(DEBUGPRINTF && !(time%80000)) {								// every 80seconds
		ivb_populator(0x0,0x0); 									// populate the ITCM Recovery Block with data/CRCs about the ITCM
		for (i=0;i<=(IVB_SPACE/sizeof(int));i++) {					// loop around every entry in the IVB block
			printf("IVB[%d] = 0x%x.  ",i,IVB[i]);					// print the entry of the IVB to check if correct
		}
	}
}
#endif


// this test pokes out the image stored starting 0xF5000C00, length 12032Bytes to spoke test board chips
// this allows a single image to be loaded to the (0,0) chip, booted and then to poke all other chips into life with own image
// it has a pre-requisite that the ybug image in bootrom on (0,0) is <3KB in size (ie, Serial not Ethernet version).
// as the spoke non-(0,0) chips are not running ybug, then these chips have NOPs populated in 1st 3KB 
#ifdef TEST_POKING_IMAGE_OUT
void testers_POKING_IMAGE_OUT()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer poker_image = (unsigned int*) POKER_IMAGE_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	testers_TEMP_DISABLE_WATCHDOG();
	if (phy_present && monitor)														// Run from chip (0,0) - the Ethernet attached chip S0 
	{	
		poke_neighbours_up(POKER_IMAGE_SIZE,(const uint *)poker_image,0);		// I am S0. Send boot image down to S1 (ext link 0)
		poke_neighbours_up(POKER_IMAGE_SIZE,(const uint *)poker_image,2);		// I am S0. Send boot image down to S2 (ext link 2)
		poke_neighbours_up(POKER_IMAGE_SIZE,(const uint *)poker_image,1);		// I am S0. Send boot image down to S3 (ext link 1)
	}
	init_watchdog();
}
#endif


// this test when run from the origin chip (ethernet attached (0,0) or S0) will send out a NN numbering packet to its neighbours
//   the neighbours take account of where it came from and increment/decrement the x & y accordingly to find it's own ID
//
// For the final version of this routine, we expect a server to send a "numbering" packet to (each) Ethernet attached chip with 
//  it's (x,y) and the x and y dimensions of the machine - it will then flood outwards.
#ifdef TEST_NUMBER_CHIPS_WITH_NN
void testers_NUMBER_CHIPS_WITH_NN()
{			
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer CHIP_IDENTIFIER = (uint*) CHIP_ID;
	uint size_x=2;
	uint size_y=2;
	if (phy_present && monitor && time==3000)									// the start comes from the S0, ethernet chip
	{	
		*CHIP_IDENTIFIER= SECURITY_CODE|0x0; 									// write my chipID to 0,0 with valid security code.			
		if(DEBUGPRINTF) printf("CHIP ID Assigned: 0x%x.\n",*CHIP_IDENTIFIER);	// print my ChipID (x<<8+y)
		p2p_algorithmically_fill_table(size_x, size_y);							// populate my routing table 
		chip_num_sender((uint)(*CHIP_IDENTIFIER & 0xFFFF), size_x, size_y);  	// broadcast NN numbering message out to
	}																// number the other chips (on a regular cartesian grid)
}
#endif


// this test turns the 4 LEDs (2 red, 2 green) on for the non (0,0) chips.  
// it wouldn't work for the (0,0) chip as the polarity is reversed!  (and also the GPIO pins are functioning as tubotron interface)		
#ifdef TEST_LIGHT_UP_SPOKE_LEDS
void testers_LIGHT_UP_SPOKE_LEDS()
{		
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	if(!phy_present && monitor) {	// only on the non ethernet attached chips where the LEDs are not muxed
		init_leds(); 			// initialise LEDs to be sparkled - test boards only S1-S3  (S0 uses GPIO for tubotron)
	}
	#ifndef DEBUGPRINTFSERIAL
	  if (phy_present && monitor) init_leds();  // if we are not using the GPIO for serial printf, then we can waggle LEDs!
	#endif
}
#endif


// this is used to temporarily disable the watchdog for tests which take longer than 1s to run.  Renable watchdog with init_watchdog()
void testers_TEMP_DISABLE_WATCHDOG()
{
	pointer WATCHDOG = (unsigned int*) WATCHDOG_BASE;				
	WATCHDOG[WATCHDOG_LOCK] = 0x1ACCE551;							// disable lock, so have write access to registers
	WATCHDOG[WATCHDOG_CTRL] = 0x0;									// Disable watchdog counter/interrupt and reset
	WATCHDOG[WATCHDOG_LOCK] = 0x0;									// re-enable the lock to protect the registers 	
}




#ifdef TEST_SDRAM

// this test does a full stress test of the SDRAM.  Note, Takes 145secs to check 256Mbits / 32MBytes. ie. @ 0.22MBytes/s or 1.77Mbit/s
void testers_TEST_SDRAM_FULL()
{	
	pointer DETECTED_SDRAM_INFO = (unsigned int*) DETECTED_SDRAM_INFO_BASE;
	unsigned int startclock=time;
	unsigned int errorcode;
	unsigned int SD_RAM_SIZE=*DETECTED_SDRAM_INFO;		// run the test over the whole of the SDRAM detected

	testers_TEMP_DISABLE_WATCHDOG();
	if (DEBUGPRINTF) printf("Running comprehensive SDRAM test.  Expected to take approx. %dsecs to complete.\n",SD_RAM_SIZE/225000);
	errorcode=ram_test_c(SD_RAM_BASE, SD_RAM_BASE+SD_RAM_SIZE, 0xFFFFFFFF);
	if (DEBUGPRINTF) { 
		printf("0x%x Error code detected in SDRAM test. Took %dms to run.\n",errorcode,time-startclock);
	} else {
		printf("No errors detected in SDRAM test. Took %dms to run.\n",time-startclock);
	}
	init_watchdog(); 
	
}
#endif


#ifdef TEST_PRINT_P2PROUTINGTABLE

// this test does a full printout of the P2P routing table
void testers_PRINT_P2PROUTINGTABLE()
{	
	unsigned int i,j,temp;
	pointer ROUTER = (unsigned int*) ROUTER_BASE;

	testers_TEMP_DISABLE_WATCHDOG();
	if (time>13500&&time<13575) {							// print only as time between 13.5 and 75ms later secs
		for (i=0;i<ROUTER_P2P_TABLE_SIZE;i++) {
			temp = ROUTER[ROUTER_P2P_TABLE + i]; 			// only read the once
			if (temp!=0xDB6DB6) {							// if there are non-drop entries (non-6s)
				printf("Dest:Target. (0x%x). ",temp);
				for (j=0;j<8;j++) printf("(%x,%x):%x. ",((i*8)+j)>>8,((i*8)+j)&0xFF,(temp>>(j*3))&0x7);
				printf("\n");
			}
		}
	}
	init_watchdog(); 
}
#endif






#endif
	
