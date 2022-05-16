/*****************************************************************************************
*	pl340.c - functions in this file are called from boot_processor() in main.c
*	to initialise the SDRAM, different initialisation are provided for both 
*   types of SDRAM, provided by Micron and Quimonda
*
*	Created by Cameron Patterson, from code provided by Steve Temple 24th March 2010.
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/


#include "../inc/globals.h"

void pl340_init (const uint table[])
{
	volatile uint* pl340 = (uint*) PL340_BASE;
	int i;

	pl340[MC_CMD] = 0x00000003;		// Pause PL340 Controller
	pl340[MC_CMD] = 0x00000004;		// Enter PL340 Config mode
  
	for (i = 0; i < 14; i++)
		pl340[MC_CASL + i] = table[i];	// populate CAS_latency through to t_esr

	pl340[MC_MCFG] = table[14];		// Init memory_cfg reg

	pl340[MC_REFP] = table[15];		// Init refresh_prd reg

	pl340[MC_CCFG0] = 0x000060e0;	// Set config reg

	pl340[MC_DIRC] = 0x000C0000;	// NOP
	pl340[MC_DIRC] = 0x00000000;	// PRECHARGEALL
	pl340[MC_DIRC] = 0x00040000;	// AUTOREFRESH
	pl340[MC_DIRC] = 0x00040000;	// AUTOREFRESH

	pl340[MC_DIRC] = 0x00080033;	// MODEREG (burst=8)

	pl340[MC_DIRC] = 0x000A0000;	// EXTMODEREG

	pl340[MC_CMD] = 0x00000000;		// Enable memory

	pl340[DLL_CONFIG1] = 0x00000000;	// Clear fine-tune (user_config1)
	pl340[DLL_CONFIG0] = 0x01000000;	// Enable DLL (user_config0)
}
