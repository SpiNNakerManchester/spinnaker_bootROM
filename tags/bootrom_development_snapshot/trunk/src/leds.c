/*****************************************************************************************
* 	Created by Steve Temple, Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/



#include "../inc/globals.h"
#include <stdio.h> // To enable printing during debug if required

#ifdef TEST_BOARD
// CP section for LED Routines only relevant for test board


void init_leds() {
	pointer sysctl = (unsigned int*) SYS_CTRL_BASE;
	unsigned int dir = sysctl[GPIO_DIR];	// read in direction of IOs
	sysctl[GPIO_CLR] = LEDS;				// clear bits that are used by IOs for LEDs  (all but 1 will light as polarity is inverted)
	dir &= ~LEDS;							// set to 0 bits that are used for IOs (output)
	sysctl[GPIO_DIR] = dir;					// write back direction of relevants LED IOs
}


void extinguish_leds() {
	pointer sysctl = (unsigned int*) SYS_CTRL_BASE;
	unsigned int dir = sysctl[GPIO_DIR];	// read in direction of IOs
	sysctl[GPIO_SET] = LEDS;				// set outputs off on IOs for LEDs  (all will extinguish - polarity inverse)
}


void sparkle_leds() {
	int pat[] = {LED_0, LED_1, LED_2, LED_3};			// populate array with LED positions
	pointer sysctl = (unsigned int*) SYS_CTRL_BASE;
	sysctl[GPIO_CLR] = LEDS;							// put all LEDs on (inverse)
	sysctl[GPIO_SET] = pat[(last_flash++)%4];			// turn off selected LED (last_flash is global var)
}

	
#endif
