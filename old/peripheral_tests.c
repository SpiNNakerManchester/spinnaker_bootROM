//TODO MOST OF THESE TESTS CAN BE COMPOSED DOWN INTO A GENERAL FUNCTION!


// Enables comm controller writing by setting commctrltest = 1 then writes the 
// test pattern 0xABCDEF37 to eachof the comm ctrl registers. Returns 0 if
// any of the read-back values are no the same as the written ones.
// TODO robodoc commentrs
// TODO does this need to be more comprehensive?
int test_comms_ctrl()
{
	int* i = 0;
	int tp = 0xABCDEF37;
	
	COMM_CTRL_TEST = 0x1;
	for(i = COMM_CTRL_BASE; i < COMM_CTRL_TEST; i += 4)
	{
		i = tp;
		if(i != tp)
		{
			COMM_CTRL_TEST = 0x0;
			return 0;
		}
	}
	
	COMM_CTRL_TEST = 0x0;
	return 1;
}

// Writes a test pattern to two of the address registers in the DMA controller
//then reads them back. Failure to match returns 0.
//TODO What does this do, and why? Does it need to be more comprehensive?
int test_DMA_ctrl()
{
	int tp = 0xFF0;
	
	DMA_CTRL_ADRS = tp;
	DMA_CTRL_ADRT = tp;
	if(DMA_CTRL_ADRS != tp || DMA_CTRL_ADRT != tp)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


// Test for ethernet MII. Simply writes test pat to TX frame length register and
// reads value back. Test passes if they are the same.
// TODO Does this need to be more comprehensive?
int test_ethernet_MII()
{
	int tp = 0xCD;
	
	ETH_MII_TX_LENGTH = tp;
	if(ETH_MII_TX_LENGTH != tp)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

// Store current value of pl340 refresh period register. Then try writing to/
// reading from it. Restore original value then return 0/1 accordingly.
int test_PL340()
{
	int tp = 0xAB;
	int bak = PL340_REFRESH_PRD;
	
	PL340_REFRESH_PRD = tp;
	if(PL340_REFRESH_PRD != tp)
	{
		PL340_REFRESH_PRD = bak;
		return 0;
	}
	else
	{
		PL340_REFRESH_PRD = bak;
		return 1;
	}
}

//...
int test_router()
{
	int tp = 0xABCDEF37;
	
	ROUTER_DIAG_COUNT = tp;
	if(ROUTER_DIAG_COUNT != tp)
	{
		ROUTER_DIAG_COUNT = 0x0;
		return 0;
	}
	else
	{
		ROUTER_DIAG_COUNT = 0x0;
		return 1;
	}
}

// Writes and reads pattern to every 256th location of System ram. Returns 0
// if read pattern differs from written one. returns 1 otherwise
int test_sys_RAM()
{
	int* i = 0;
	int tp = 0xABCDEF37;
	
	for(i = SYS_RAM_BASE; i < SYS_RAM_TOP; i += 0x100)
	{
		i = tp;
		if(i != tp)
		{
			return 0;
		}
	}
	
	return 1;
}

// same again... using integrated test/control reg
int test_timer()
{
	int tp = 0x1;
	
	TIMER_ITCR = tp;
	if(TIMER_ITCR != tp)
	{
		TIMER_ITCR = 0x0;
		return 0;
	}
	else
	{
		TIMER_ITCR = 0x0;
		return 1;
	}
}

// Unlocks wdog for testing. Writes to ITCR and reads back.
// Returns 0 if read/written vals  do not match, 1 otherwise.
// TODO In mukarams code, there is a commented out chunk that looks like it does a more comprehensive test. Do we need that?
int test_watchdog()
{
	WATCHDOG_LOCK = 0x1ACCE551;
	WATCHDOG_ITCR = 0x1;
	if(WATCHDOG_ITCR != 0x1)
	{
		WATCHDOG_ITCR = 0x0;
		WATCHDOG_LOCK = 0x0;
		return 0;
	}
	else
	{
		WATCHDOG_ITCR = 0x0;
		WATCHDOG_LOCK = 0x0; 
		return 1;
	}
}