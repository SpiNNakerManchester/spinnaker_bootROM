/*****************************************************************************************
*	main.c - the main function in this file is called from __main (which is in turn called
*	in start.s from where this program begins executing). main() calls boot_processor,
*	which is responsible for monitor arbitration and processor/chip initialisation.
*	Finally, wait_for_interrupts() puts the processor into the low-power wait-state, which
*	is exited by interrupts that cause execution of the ISRs in
*	interrupt_service_routines.c.
*
*
*	Originally by Mukaram Khan and Xin Jin
*	Recreated by Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#include "../inc/globals.h"
#include "../inc/peripheral_inits.h"
#include "../inc/processor.h"
#include "../inc/pl340.h"
#include "../inc/phy.h"
#include "../inc/testers.h"
#include "../inc/reg_test.h"
#include "../inc/dma.h"

#include "../src/debugprintf.c"


/* Each core enters arbitration for monitor status. Cores that were monitors preceeding a
 * soft reset are excluded from arbitration and the new monitor ensures that they are 
 * disabled. A return value of 1 indicates that this core is the monitor, 0 otherwise. */
unsigned int arbitrate_monitor()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer PAST_MONITORS = (unsigned int*) MONITOR_HISTORY;
	unsigned int pid = 0x1 << processor_ID;										// shift a 1 processorID bits left for use below 
	
	#ifdef TEST_BOARD
		*PAST_MONITORS = SECURITY_CODE | 0;  // ignore anything existing bootrom has placed in this memory location, & as we're emulating power on
		// need to reset test and set for this code...
	#endif 
	
	printf("A1\n");	//!!! TLM !!!
	
	if(SYS_CTRL[SYS_CTRL_RESET_CODE] == POWER_ON_RESET)
	{
		if(!SYS_CTRL[PAST_MONITOR_RESET_MUTEX]) {								// 1st read will return bit 31, x==0, else 1.
			*PAST_MONITORS = SECURITY_CODE | 0;			//  arbritrate to write past monitors to 0, at power on reset
			SYS_CTRL[SYS_CTRL_MISC_CTRL]|=0x4;			// set bit2 of the Misc Control Register (used as a gate below) !per CP/ST chat 15Jun10
			printf("A2\n");	//!!! TLM !!!
		}
		while (!(SYS_CTRL[SYS_CTRL_MISC_CTRL] & 0x00000004)) ;		//	wait while the nominated clearer does the reset
		printf("A3\n");	//!!! TLM !!!
	}
		
	printf("A4\n");	//!!! TLM !!!

	if(!(pid & *PAST_MONITORS))													// if we have not been a previous monitor
	{
		printf("A5\n");	//!!! TLM !!!
		if (SYS_CTRL[SYS_CTRL_ARBIT_BASE + processor_ID])						// arbitrate for monitor 
		{
			printf("A6\n");	//!!! TLM !!!
			SYS_CTRL[SYS_CTRL_CPU_DISABLE] = *PAST_MONITORS;					// if we are elected monitor, disable previous monitors (note security code already in past_monitors)
			printf("A7\n");	//!!! TLM !!!
			*PAST_MONITORS |= pid;												// add ourselves to list of previous monitors
			return 0x1;															// return 1 to indicate this CPU is monitor
		}
	}
	return 0;																	// return a 0 if we are not monitor
}

/* Each processor core tests and initialises its own peripherals. Following arbitration
 * for monitor status, the monitor tests and initialises the chip peripherals. Finally,
 * interrupts are enabled and the successful booting of the core is registered. */
void boot_processor()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;							
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
//	pointer MAILBOX = (unsigned int*) MAILBOX_BASE;
	pointer CHIP_IDENTIFIER = (unsigned int*) CHIP_ID;

	#ifdef TEST_BOARD									// This determines whether the old MMK bootrom has been initialised.
   	  pointer VIC = (unsigned int*) VIC_BASE;
	  if (!VIC[VIC_ENABLE])								//   If it has, then the reg tests of test_processor_peripherals would not work
	#endif												//   so ignore.  Otherwise - perform tests as they are valid to undertake
	
	#ifdef DEBUGPRINTFSERIAL
		init_tube_port();
	#endif

	//test_processor_peripherals(); 						//TODO Versioning testChip vs Spinnaker2 in routine

	init_comms_ctrl();									// initialise the system controller (peripheral_inits.c)	
	init_DMA_ctrl();									// initialise the DMA controller 
//	processor_ID = (DMA_CTRL[DMA_CTRL_STAT] >> 24);		// read my own physical processor ID (from my DMA controller status register)
	
	if(arbitrate_monitor())								// if I win the race to be monitor
	{
		printf("B1\n");	//!!! TLM !!!
		
		*CHIP_IDENTIFIER = 0xFFFF;						// initialise my ID to unlocked 255,255 until proper numbering packet received.
			
		#ifdef TEST_BOARD								// This determines whether the old MMK bootrom has been initialised.
		  if (!VIC[VIC_ENABLE])							//   If it has, then the reg tests of test_chip_peripherals would not work
		#endif											//   so ignore.  Otherwise - perform tests as they are valid to undertake
		//test_chip_peripherals(); 							//TODO Versioning testChip vs Spinnaker2 in routine

		init_router();									// setup the router, initialise the routing tables
		printf("B2\n");	//!!! TLM !!!
		pl340_init(pl340_data);							// initialise the RAM controller
		printf("B3\n");	//!!! TLM !!!
		test_sdram();									// now the RAM is initialised we can write and test it
		printf("B4\n");	//!!! TLM !!!
		
		if(test_phy())									// if we have a phy then we are Ethernet connected
		{
			printf("E1\n");	//!!! TLM !!!
			init_phy();									// initialise the PHY chip to our specifications
			printf("E2\n");	//!!! TLM !!!
			init_ethernet_MII();						// initialise the MII Ethernet controller
			printf("E3\n");	//!!! TLM !!!
			//phy_present = 1;									// NB. Implies Ethernet Phy detected & enabled, not that link is up yet
			if (DEBUGPRINTF&&phy_present) printf("Eth h/w detected & enabled\n");
		}
		init_watchdog(); 								// setup the watchdog timer
		printf("B5\n");	//!!! TLM !!!
		SYS_CTRL[SYS_CTRL_MISC_CTRL]|=0x8;			// set bit3 of the Misc Control Register (used as a gate below) !per CP/ST chat 15Jun10
	}										//    we add this as fascicles could corrupt initial reg values we check
	else {
		if (DEBUGPRINTF) printf("I am fascicle - now wait for monitor to catch up, and then off to sleep with me\n");
		while (!(SYS_CTRL[SYS_CTRL_MISC_CTRL] & 0x00000008)) ;		// wait until the monitor catches up and sets bit 3 of reg
		printf("FW\n");	//!!! TLM !!!
	}
	printf("B6\n");	//!!! TLM !!!
	init_timer();										// CP moved here as processor block subsystem. Setup timer for 1ms itrpt clock
	printf("B7\n");	//!!! TLM !!!
	init_VIC();											// sets up the vector interrupt controller with our required interrupts 
	printf("B8\n");	//!!! TLM !!!
	change_to_low_vectors();							// take us out of high_vectors (per booting from ROM), into RAM low vectors.s
	printf("B9\n");	//!!! TLM !!!
	enable_interrupts(); 								// enable the interrupts set up in init_VIC
	printf("BA\n");	//!!! TLM !!!
	SYS_CTRL[SYS_CTRL_SET_CPU_OK] = (0x1<<processor_ID);// mark this core processor as good in the designated system controller register

}


/* Simply follows boot procedure, then goes into wait for interrupt mode. */
int main()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer WATCHDOG = (unsigned int*) WATCHDOG_BASE;
//	pointer ETH_MII = (uint*) ETH_MII_BASE;
	pointer ETH_PARAMS = (unsigned int*) ETH_PARAMS_BASE; //!!! TLM !!!
	unsigned int i; //!!! TLM !!!
		
	#ifndef SPINNAKER2
		uint inloop=0;
	#endif

	if (monitor) for (i=0;i<3;i++) printf("SerialROM recovered: 0x%x\n",ETH_PARAMS[i]);  //!!! TLM !!!


	#ifndef APPTEST
		boot_processor();								//// !!APP!! testing CP. Test and initialise the hardware on the chip  
	#endif
	#ifdef APPTEST
		#ifdef DEBUGPRINTFSERIAL						// !!APP!! testing CP
			init_tube_port();							// !!APP!! testing CP
		#endif											// !!APP!! testing CP
		if (phy_present) init_ethernet_MII();			// !!APP!! testing CP
		init_VIC();										// !!APP!! testing CP
		enable_interrupts();							// !!APP!! testing CP
		printf("Rebooted into App. ");					// !!APP!! testing CP
		if (monitor) printf("I am the monitor\n");		// !!APP!! testing CP
		if (!monitor) printf("I am the fascicle\n");	// !!APP!! testing CP
		if (phy_present) send_itubotron_message();		// !!APP!! testing CP
		if (!phy_present) LED_FLASH_INTERVAL = 0x400;	// !!APP!! testing CP - slower flash LEDs on spoke boards	
	#endif

	printf("Main1\n");	//!!! TLM !!!

	if (!phy_present && monitor) {
		SYS_CTRL[GPIO_DIR]&=0xFFFFFF3C;    // set up GPIO direction outbound for LEDs
		SYS_CTRL[GPIO_CLR]=0xC3; // Turn off all LEDs
		SYS_CTRL[GPIO_SET]=0x82; // Turn on both green LEDs on spoke chips (would be red on hub)
	} 	
  
	while(1) {
	
		printf("%d) In the Loop\n",time);  //!!! TLM !!!
		wait_for_interrupt();    						// on the monitor the timer interrupt will wake from this state every 1ms
															// on a fascicle only a system controller interrupt to the CPU wakes it

		if(monitor && !(time % WATCHDOG_REFRESH))  		// the monitor is responsible for resetting the watchdog, every few ms
		{
			WATCHDOG[WATCHDOG_LOCK] = 0x1ACCE551;		// unlock the watchdog registers with key to make them writable
			WATCHDOG[WATCHDOG_LOAD] = WATCHDOG_COUNT;	// reset the watchdog timer
			WATCHDOG[WATCHDOG_LOCK] = 0;				// relock the watchdog registers to prevent errant updating
		}
		// this is a valid place for the watchdog update, if we are within the main while loop, we are still under proper program 
		// control. Having the watchdog update in the timer interrupt doesn't protect against a runaway program with working interrupts

		
		// The following are testing routines. (and not to used on the ROM!) - they are turned off with defines in globals.h

		#ifndef SPINNAKER2
			// these are test routines that are only run a single time following bootup
			if (0==inloop++) {								// inloop is only 0 the first time through the while loop
				#ifdef TEST_LIGHT_UP_SPOKE_LEDS				 
					testers_LIGHT_UP_SPOKE_LEDS();			// this routine lights up test board LEDs on non-Ethernet attached chips
				#endif
				#ifdef TEST_POKING_IMAGE_OUT				
					testers_POKING_IMAGE_OUT();				// this pokes out a <=12032 Byte block image from SystemRAM on Eth chip to others 
				#endif
				#ifdef TEST_FLOOD_FILL_IMAGE_OUT
					if (phy_present) testers_FLOOD_FILL_IMAGE_OUT();
				#endif
				#ifdef TEST_FASCICLE_BOOTER
					testers_FASCICLE_BOOTER();				// this wakes up the fascicles and boots them
				#endif

			}			

			// these are test routines that may run more than once
			#ifdef TEST_NUMBER_CHIPS_WITH_NN			
				testers_NUMBER_CHIPS_WITH_NN();				// this writes out a chip numbering NN packet from Eth chip to others, it runs every 5 secs
			#endif
		
			#ifdef TEST_PRINT_P2PROUTINGTABLE
				testers_PRINT_P2PROUTINGTABLE();			// at around 25secs after boot dump the routing table out of printf
			#endif
					
			#ifdef TEST_MULTICHIP_PRINTF
				testers_MULTICHIP_PRINTF();					// if chips are numbered this periodically sends test "printf" to itubotron
			#endif
			#ifdef TEST_FASCICLE_WAKE_AND_BOOT
				testers_FASCICLE_WAKE_AND_BOOT();			// tests responsiveness of fascicles to MAILBOX messages & interrupts
			#endif
			#ifdef TEST_ETHERNET_RESETS
				testers_ETHERNET_RESETS();					// cycles MII on and off for 5sec intervals to check for bugs (per ST)
			#endif
			#ifdef TEST_DMA_BURST_BUG				  
				testers_DMA_BURST_BUG();					// routine that systematically checks/validates DMA bursting parameters
			#endif												// between System RAM / TCM, per bug in Silistix tools
			#ifdef TEST_REPEAT_FLOOD_FILL_IMAGE_OUT			  
				testers_REPEAT_FLOOD_FILL_IMAGE_OUT();				// test flood-fill routine that sends a test image out to peers using NN
			#endif
			#ifdef TEST_CC_PACKET_RECEIVED
				testers_CC_PACKET_RECEIVED();				// flashes a LED whenever a packet is received on an external link
			#endif
			#ifdef TEST_IBV_POPULATION
				testers_IBV_POPULATION();					// this writes a IVB for the current image, and reads it back to check
			#endif	
			#ifdef TEST_SDRAM
				if ((time>=60000) && (time<=61000)) testers_TEST_SDRAM_FULL();	// perform a full SDRAM test when uptime is 60secs				
			#endif
			
			
		#endif



// S2 TLM DMA & CRC testing only (below)
#ifdef SPINNAKER2 
{	
	// this section used for testing DMA transfers & bug 
	unsigned int word_count, crcforloop, burstsize, errcnt;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer TESTIMAGETCM = (unsigned int*) DTCM_IMAGE_SPACE;  	
	pointer TESTIMAGETCM2 = (unsigned int*) DTCM_IMAGE_SPACE + 0x13C4;
	pointer TESTIMAGETCM3 = (unsigned int*) DTCM_IMAGE_SPACE - 0x20;
	pointer TESTIMAGETCM4 = (unsigned int*) DTCM_IMAGE_SPACE - 0x40;    
	pointer BLOCK = (uint*) 0xf5002000;
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	unsigned int i,j,k,l,m;



	//SYS_CTRL[0]=4;
	printf("\4");  // end simulation and print how long it took.




		
	if (phy_present && monitor) {										// if I am an Ethernet attached chip
 	 for (i=0 ; i<4096 ; i++) TESTIMAGETCM[i] = ((((i*4)+3)%256)<<24)|((((i*4)+2)%256)<<16)|((((i*4)+1)%256)<<8)|(i*4)%256;	
	 		// populate a 16KB block of data repeating incrementing byte pattern 00->FF (for easy validation).	


	for (j=0;j<32;j++) printf("%d) 0x%x\n",j,DMA_CTRL[DMA_CTRL_CRCSETUPTABLE+j]);  // !!!TLM print CRC table
	TESTIMAGETCM3[0] =(unsigned int) 0x54455354; //"TEST";
	TESTIMAGETCM3[1] =(unsigned int) 0xB1111111;
	TESTIMAGETCM3[2] =(unsigned int) 0xC0C0C0C0;
	TESTIMAGETCM3[3] =(unsigned int) 0xD1BB1e50;
	TESTIMAGETCM3[4] =(unsigned int) 0xE0000000; 

	for(l=0;l<2;l++) {
		if (l>0) {
			printf("\n\nNow, same test, Corrupting the CRC table...\n");
			for (j=0;j<32;j+=4) DMA_CTRL[DMA_CTRL_CRCSETUPTABLE+j]=0xC0FFEE;
			for (j=0;j<32;j++) printf("Chg CRC tbl:  %d) 0x%x\n",j,DMA_CTRL[DMA_CTRL_CRCSETUPTABLE+j]);  // !!!TLM print CRC table
		}
		for(j=0;j<5;j++) { 
			dma((unsigned int) BLOCK, (unsigned int) (TESTIMAGETCM3),DMA_CRC_ON, DMA_WRITE, ((j+1)*4));
			for(m=0;m<=j;m++) printf("%d of %d) Rd:0x%x,  Wr: 0x%x.\n",m,j,TESTIMAGETCM3[m],BLOCK[m]);
			printf("DMA Write. Calc CRC: 0x%x\n\n",DMA_CTRL[0x7]);
			dma((unsigned int) BLOCK, (unsigned int) (TESTIMAGETCM4),DMA_CRC_ON, DMA_READ, ((j+1)*4));
			for(m=0;m<=j;m++) printf("%d of %d) Rd: 0x%x.\n",m,j,TESTIMAGETCM4[m]);
			printf("DMA Read. ReadCRC: 0x%x. Calc CRC: 0x%x\n\n",DMA_CTRL[0x8],DMA_CTRL[0x7]);
			BLOCK[j]=~BLOCK[j];  // deliberately currupt data in final word
			printf("Expecting Fault, data corrupted\n");
			dma((unsigned int) BLOCK, (unsigned int) (TESTIMAGETCM4),DMA_CRC_ON, DMA_READ, ((j+1)*4));
			for(m=0;m<=j;m++) printf("%d of %d) 0x%x.\n",m,j,TESTIMAGETCM4[m]);
			printf("DMA Read Corrupted. ReadCRC: 0x%x. Calc CRC: 0x%x\n\n",DMA_CTRL[0x8],DMA_CTRL[0x7]);
		}
	}
	
	setup_crc32_spinnaker2();  // correct CRC table

	printf("Doing a 64KB transfer from DTCM to SDRAM, with CRC on\n");
	dma(0x80000, 0x70000000,DMA_CRC_ON, DMA_WRITE, 0x10000);
	printf("64KB transfer complete. Did we get an error?\n");
	 
	 for (i=0;i<1000;i++) {											// repeats the test 1000 times
	  printf("Time: 0x%xms. DMA test Loop %d of 1000.\n",time,i);		//!!! TLM !!!
	  for (word_count=128;word_count<=1024;word_count+=128) {   	// changes the size of the transfer
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
//	 k = 10000000; while (k--);  									// wait a bit before starting the loop once again
	} // outer i for loop
	
   } // if ethernet attached chip
   
}
// S2 TLM DMA & CRC testing ONLY (above)
#endif











	}	
}
