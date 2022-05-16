
#ifndef SPINNAKER2			// none of this testing will happen on the real S2 chip!

	#ifndef TESTERS_H
		#define TESTERS_H
		
		void testers_MULTICHIP_PRINTF(void); 
		void testers_FASCICLE_WAKE_AND_BOOT(void);
		void testers_FASCICLE_BOOTER(void);
		void testers_ETHERNET_RESETS(void);
		void testers_FLOOD_FILL_IMAGE_OUT(void);
		void testers_REPEAT_FLOOD_FILL_IMAGE_OUT(void);
		void testers_DMA_BURST_BUG(void);
		void testers_CC_PACKET_RECEIVED(void);
		void testers_IBV_POPULATION(void);
		void testers_POKING_IMAGE_OUT(void);
		void testers_NUMBER_CHIPS_WITH_NN(void);
		void testers_LIGHT_UP_SPOKE_LEDS(void);
		void testers_TEST_SDRAM(void);
		void testers_TEST_SDRAM_FULL(void);
		void testers_TEMP_DISABLE_WATCHDOG(void);
		void testers_PRINT_P2PROUTINGTABLE(void);
	#endif

#endif
	
