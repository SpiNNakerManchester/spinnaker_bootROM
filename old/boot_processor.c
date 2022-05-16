/*******************************************************************************
*	bootProcess.h
* 
* 	TODO Description of file
* 
* 	Created by Mukaram Khan and Xin Jin
*	Modified by Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
* 
*******************************************************************************/

#include "../inc/peripheral_inits.h"
#include "../inc/peripheral_tests.h"

// testProcResources() tests the resources each processor has. These tests
// are run by every processor core. If any of the tests fail, the function 
// ensures the CPU OK bit
// in the system controller corresponding to the processor ID is unset and 0
// is returned. Otherwise, the OK bit is set and 1 is returned as a success code
// TODO Comment properly for ROBO doc
int testProcResources() //TODO Ordering of tests?
{
	int status = 0;
	if(testCommCtl())	status |= (0x1 << 0);
	if(testIntCtl())	status |= (0x1 << 1);
	if(testDMAC())		status |= (0x1 << 2);
	if(testTimer())		status |= (0x1 << 3);
	
	if(status == 0xF)
	{
		SYS_CTRL_SET_CPU_OK = (0x1 << procID);
		return 1;
	}
	else
	{
		SYS_CTRL_CLR_CPU_OK = (0x1 << procID);
		return 0;
	}
}

//this function is for the monitor to check the chip resources
//if they all pass, bit 31 of the sys ctrl cpu OK is set high
int testChipResources() //TODO ordering of tests?
{
	int status = 0;
	if(testRouter())	status |= (0x1 << 0);
	if(testSysCtl())	status |= (0x1 << 1);
	if(testPL340())		status |= (0x1 << 2);
	if(testSDRAM())		status |= (0x1 << 3);	
	if(testWatchdog())	status |= (0x1 << 4);
	if(testRAM())		status |= (0x1 << 5);
	
	if(status == 0x3F)
	{
		SYS_CTRL_SET_CPU_OK = (0x1 << 31);
		return 1;
	}
	else
	{
		SYS_CTRL_CLR_CPU_OK = (0x1 << 31);
		return 0;
	}
}

void initChipResources()
{
	initPL340();
	initRouter();
	setChipClock();
	initCRC32Table();
	init_router();
	initTimer();
}

void initProcResources()
{
	initCommCtl();
	initDMAC();
}


//TODO only reset CPU OK register at power on reset?
int bootupProc()
{
	procID = (DMA_CTRL_STAT >> 24);
	
	changeToLowVectors();
	
	if(SYS_CTRL_RESET_CODE != POWER_ON_RESET)
	{
		//TODO kill current monitor processor and free up for new arb round
	}
	
	if(!testProcResources) return 0;
	initProcResources();
	
	if(SYS_CTRL_ARBIT_BASE + procID * 4 == 0x80000000)
	{
		procType=MON_PROC;
		if(!testChipResources()) return 0;
		initChipResources();
		
		//TODO build in check for phy/ethernet
	}			
	else
	{
		procType=FASCICLE_PROC;
	}
	
	initInterrupt();	//TODO check behaviour/usage
	enableInterrupts(); 
		
	return 1;
}



int testEthernet() //TODO fix order of these tests
{
	if(testPhy())
	{
		if(testEthernet())
		{
			return 1;
		}
	}
	return 0;
}



int testInitPhy()
{
	if(testPhy() == 0)
	{
		enableEthernet=0;
	}
	else
	{
		enableEthernet=1;
		if(testEthernet() == 1)
		{
			status |= (0x1 << 14);
		}
		else
		{
			enableEthernet=0;
		}
		
		if(testPhy() == 1)
		{
			status |= (0x1 << 15);
		}
		else
		{
			enableEthernet=0;
		}
	}
	
	
	if(enableEthernet == 1)    //if phy is attached
	{
		initMacFromSerialRom();
		initIPAddressFromSerialRom();
		initEthernet();     //depends on initMacFromSerialRom() for initializing the mac address

		initPhy();
	}
}
