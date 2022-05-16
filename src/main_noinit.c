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
#include "../inc/dhcp.h"
#include "../inc/peripheral_inits.h"
#include "../inc/processor.h"
#include "../inc/phy.h"
#include "../inc/reg_test.h"
#include "../inc/start.h"


#ifdef DEBUG
#include <stdio.h>
#endif



/* boot_processor() is called by each core to initialise its peripherals and arbitrate for
 * monitor status. Following arbitration, the monitor tests and initialises the chip 
 * peripherals and sets status LEDs. Finally, each core enables its interrupts and records
 * successful boot-up in the system controller CPU OK register. */
void boot_processor()
{
	init_ethernet_MII();
	init_VIC();			// need to reset this as interrupt handling routines in different places in this build
	enable_interrupts();		// as were turned off
}


/* The main function calls boot_processor() to arbitrate for monitor status and to test
 * and initialise the core and chip peripherals. Then the core enters an infinite loop in
 * which it waits for interrupts (timer and packet interrupts for monitors, system
 * controller interrupts for fascicles) and updates the watchdog counter periodically. */
int main()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;	// Used to get proc ID
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;	// Used to get monitor ID
	pointer WATCHDOG = (unsigned int*) WATCHDOG_BASE;
		
	boot_processor();

	while(1)
	{
		wait_for_interrupt();
		
		if(!(time % WATCHDOG_REFRESH) && MONITOR) // MONITOR compares proc and monitor IDs
		{
			WATCHDOG[WATCHDOG_LOCK] = 0x1ACCE551;
			WATCHDOG[WATCHDOG_LOAD] = WATCHDOG_COUNT;
			WATCHDOG[WATCHDOG_LOCK] = 0;
		}
		
		#ifdef DEBUG
			printf("Main loop %d\n", time);
			//if (time>100) SYS_CTRL[0]=0x4;				 // CP: 20/08/2010 don't let a TLM simulation run forever
		#endif
	}
}
