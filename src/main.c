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


#include "../inc/globals.h"
#include "../inc/peripheral_inits.h"
#include "../inc/processor.h"
#include "../inc/phy.h"
#include "../inc/reg_test.h"
#include "../inc/start.h"

#ifdef DEBUG
#include <stdio.h>
#endif


/* This function controls the arbitration for monitor status. In the case of a power-on
 * reset, each core races for the responsibility to reset the record of PAST_MONITORS to
 * 0, with the losers spin-waiting for the winner to complete. After this, all those cores
 * that have not previously been a monitor (all of them in the case of a power-on reset
 * but perhaps only certain cores in the case of a soft reset) race for monitor status.
 * Any cores that have previously been a monitor disable themselves, as they appear to
 * have caused an erroneous soft reset. Finally, the winner records itself as a monitor. A
 * return value of 1 indicates that this core is the monitor, 0 otherwise. */
unsigned int arbitrate_monitor()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	pointer PAST_MONITORS = (unsigned int*) MONITOR_HISTORY;
	unsigned int pid = 0x1 << PROCESSOR_ID;

	// At power-on, all cores race to reset monitor status records
	if(SYS_CTRL[SYS_CTRL_RESET_CODE] == POWER_ON_RESET)
	{
		if(!SYS_CTRL[PAST_MONITOR_RESET_MUTEX]) // If the lock is aquired...
		{
			*PAST_MONITORS = SECURITY_CODE | 0;
			SYS_CTRL[SYS_CTRL_MISC_CTRL] |= 0x4; // Report 'clear complete'
		}
		else
		{
			while(!(SYS_CTRL[SYS_CTRL_MISC_CTRL] & 0x4)); // Spin-wait on 'clear complete'
		}
	}

	// If the core was previously a monitor, it shuts itself down otherwise it arbitrates
	if(pid & *PAST_MONITORS)
	{
		boot_fail(PAST_MONITOR_FAILURE);
	}
	else
	{
		if(SYS_CTRL[SYS_CTRL_ARBIT_BASE + PROCESSOR_ID])
		{
			*PAST_MONITORS |= pid;
			return 1;
		}
	}

	return 0;
}


/* boot_processor() is called by each core to initialise its peripherals and arbitrate for
 * monitor status. Following arbitration, the monitor tests and initialises the chip
 * peripherals and sets status LEDs. Finally, each core enables its interrupts and records
 * successful boot-up in the system controller CPU OK register. */
void boot_processor()
{
	pointer DMA_CTRL = (unsigned int*) DMA_CTRL_BASE;
	pointer ETH_PARAMS = (unsigned int*) ETH_PARAMS_BASE;
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;

	if((SYS_CTRL[SYS_CTRL_RESET_CODE] == POWER_ON_RESET) && (SYS_CTRL[SYS_CTRL_GPIO_SET]&POSTDISABLED)==0)
					// BUGZILLA 65 CP - if GPIO pin set to disable POST then do not perform processor level testing
	{
		test_processor_peripherals();
	}

	init_comms_ctrl();
	init_DMA_ctrl();

	// Winner of arbitration initialises chip peripherals. Losers just spin-wait
	if(arbitrate_monitor())
	{

		if((SYS_CTRL[SYS_CTRL_RESET_CODE] == POWER_ON_RESET) && (SYS_CTRL[SYS_CTRL_GPIO_SET]&POSTDISABLED)==0)
					// BUGZILLA 65 CP - if GPIO pin set to disable POST then do not perform system level testing
		{
			test_sysram();
			test_chip_peripherals();
			test_sdram();
		}

		init_router();

		// If there's a phy and Eth data is initialised from serial ROM
		#ifdef SPINNAKER2									// BUGZILLA 67 - S2 straps presence of SerialROM, testchip doesn't
		if(SYS_CTRL[SYS_CTRL_GPIO_SET]&SERIALROMPRESENT)	// BUGZILLA 67 CP Only if a serial ROM is present do Ethernet stuff
		{
		#endif
			if(test_phy() && (*ETH_PARAMS != 0))
			{
				init_phy();
				init_ethernet_MII();
			}
		#ifdef SPINNAKER2									// BUGZILLA 67 - S2 straps presence of SerialROM, testchip doesn't
		}
		#endif

		SYS_CTRL[SYS_CTRL_GPIO_DIR] &= ~LEDS;
		SYS_CTRL[SYS_CTRL_GPIO_SET] = LEDS;
		SYS_CTRL[SYS_CTRL_GPIO_CLR] = LED_0; // Begin wiggling one LED

		init_watchdog();
		SYS_CTRL[SYS_CTRL_MISC_CTRL] |= 0x8; // Monitor reports 'setup complete'
	}
	else
	{
		while(!(SYS_CTRL[SYS_CTRL_MISC_CTRL] & 0x8)); // Fascicles spin-wait for monitor
	}

	init_timer();
	init_VIC();
	change_to_low_vectors();
	enable_interrupts();
	SYS_CTRL[SYS_CTRL_SET_CPU_OK] = (0x1 << PROCESSOR_ID);
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
