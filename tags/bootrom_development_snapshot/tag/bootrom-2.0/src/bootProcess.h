#ifndef BOOTPROCESS_H_
#define BOOTPROCESS_H_

#include "spinnakerlib.h"
#include "highLevelHandlers.h"
#include "nnDiagProtocol.h"


/*******************************************************************
 *  first see if the monitor proc id=1F and the reset code=00 (power on reset), do the normal selection. 
* if the monitor proc !=1F and reset code=00, the neighbour has reset the processors, 
*			if the mon proc id=own id and the arbiteration bit=1 (neighbour selected you as new neighbour), do as the monitor processor but donot test and initialize the components, 
*			just do high level funcational tests to see if all the components are working fine or do the initialization
*			if the arbiteration bit=0 (even if the mon proc id=own id), do the arbiteration (the neighbour reset all the processors to restart the chip)
*
* 
******************************************************************/
//int setup monitor processor(int procid)
//{
//	unsigned int idReg=0;
//	unsigned int resetCode=0;
//	int returnCode=0, arbBit=0;
//	idReg=readMemory(SYSCTLR_BASE_U, SYSCTLR_MONITOR_ID);
//	resetCode=readMemory(SYSCTLR_BASE_U, SYSCTLR_RESET_COD);
//	arbBit=(idReq >> 8) & 0x1; //arbitration bit (8) in the mon proc id register
//	resetCode=resetCode & 0x3; //first two bits
//	switch(resetCode)
//	{
//		case 0:
//		{
//			if(!arbBit)    //if the arbitration bit is low i.e. it is power on reset, do the normal arbitration and exit
//			{
//				if(arbitForMonProc(procid))   //to see if I am selected as mon proc by the system controller
//					procType=MON_PROC;
//				else
//					procType=FASCICLE_PROC;
//			}
//			else		//if the arbiteration bit is high i.e. the nieghbouring chip did the soft reset
//			{
//				if(procid == (idReg & 0x1F)) //if the neighbour has assigned me as a new mon proc, i need to assign myself as the monitor proc
//											//and avoid testing the components except the interrupts setting for mon proc and running 
//											// the mon proc code
//				{
//					procType= MON_PROC; 	//setting itself as the mon proc
//					if(MEM(globalVarSpace, SR_CONF_DONE)==1)  	//read global variable space in the system controller to know 
//																//if the configuration was done already then return 1
//							returnCode= 1;	            		//to indicate that it does not have to do the detailed testing
//				}
//				else               	//if some other proc has been selected as the mon proc by the neighbouring chip, 
//								 	//though not a valid state, just select itself as a fascicle and relax
//				{
//						procType=FASCICLE_PROC; //setting itself as the fascicle proc
//				}
//			}
//			break;
//		}
//		case 1:  	//watchdog reset, everything is reset and needs initializing, 
//					//the system controller will select another proc as the mon proc
//		{
//			if(arbitForMonProc(procid))
//				procType=MON_PROC;
//			else
//				procType=FASCICLE_PROC;
//	
//			break;
//		}
//		case 2:   //not used but the same action as in case 1
//		{
//			if(arbitForMonProc(procid))
//				procType=MON_PROC;
//			else
//				procType=FASCICLE_PROC;
//	
//			break;
//	
//		}
//		case 3:	 //the watchdog interrupt has been used as a soft reset to me, 
//	 			 //I was going as the mon proc but had something wrong in the process
//		{
//			//the mon proc has to continue working as mon proc, dont initialize the peripherals,
//			//return code=2 to indicate that it needs to only clear all the pending interrupts and do some functional tests, 
//			//no need to configure the VIC now
//			procType= MON_PROC;; //setting itself as the mon proc
//			returnCode= 2;	//to indicate that it does not have to do the detailed testing and initialization 
//							//just clear all interrupts including the watchdog interrupt	
//			break;
//		}
//	}
//	return returnCode;
//
//
//}	
			
			
					
/*******************************************************************
 * 
 * 
 ******************************************************************/

int testLocalPeripherals()
{
	int procChipStatus=0, result=0;
	if(testCommCtl() == 1)
	{
		procChipStatus |= (0x1 << 0);  //setting bit 0 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed comm ctlr test\n", procID);
		#endif
	}
	if(testIntCtl() == 1)
	{
		procChipStatus |= (0x1 << 1);  //setting bit 1 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed int ctlr test\n", procID);
		#endif
	}
	if(testDMAC() == 1)
	{
		procChipStatus |= (0x1 << 2);  //setting bit 0 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed DMAC test\n", procID);
		#endif
	}
	if(testTimer() == 1)
	{
		procChipStatus |= (0x1 << 3);  //setting bit 0 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed timer test\n", procID);
		#endif
	}
	result=setCPUStatus(procID, procChipStatus);
	if(result != 1)
	{
		#ifdef VERBOSE
		{
			printf("proc %d: failed 1st level POST\n", procID);
		}
		#endif
		//the processing node is not functional, just return to gotosleep
		return 0;
	}
	#ifdef VERBOSE
		printf("proc %d: setup ok status\n", procID); 
	#endif
	
	#ifdef VERBOSE
		printf("proc %d: 1st POST completed\n", procID); 
	#endif
	return 1;
}	
//setResetOnWatchdogInt()
//{
//	unsigned int value;
//	#ifdef VERBOSE
//		printf("proc %d: setting the reset on watchdog interrupt in sys ctlr\n", procID); 
//	#endif
//	value=readMemory(SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID);
//	value = ( value | (1 << 16) | (0x5EC << 20) ); //setting the bit 16 i.e. enable reset mon proc on watchdog interrupt
//	writeMemory(SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID, value);
//}
int testInitChipResources()
{
	int procChipStatus=0, result=0;
	initChipVariables();
	#ifdef VERBOSE
		printf("proc %d: init global variables\n", procID);
	#endif
	if(testRouter() == 1)
	{
		procChipStatus |= (0x1 << 8);  //setting bit 0 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed rtr test\n", procID);
		#endif
	}
	if(testSysCtl() == 1)
	{
		procChipStatus |= (0x1 << 9);  //setting bit 0 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed sys. ctl. test\n", procID);
		#endif
	}
	if(testPL340() == 1)
	{
		procChipStatus |= (0x1 << 10);  //setting bit 0 for comm. ctlr
		initPL340();
		#ifdef VERBOSE
			printf("proc %d: init PL340\n", procID);
		#endif
		if(testSDRAM() == 1)
		{
			procChipStatus |= (0x1 << 11);  //setting bit 0 for comm. ctlr
		}
		else
		{
			#ifdef VERBOSE
				printf("proc %d: failed SDRAM test\n", procID);
			#endif
		}
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed pl340 test\n", procID);
		#endif
	}
	if(testWatchdog() == 1)
	{
		procChipStatus |= (0x1 << 12);  //setting bit 0 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed wdog test\n", procID);
		#endif
	}
	if(testRAM() == 1)
	{
		procChipStatus |= (0x1 << 13);  //setting bit 0 for comm. ctlr
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: failed ram test\n", procID);
		#endif
	}
	if(detectPhy() == 0)
	{
		#ifdef VERBOSE
			printf("proc %d: no phy attached to this chip\n", procID);
		#endif
		enableEthernet=0;
		dputc ('N');
	}
	else
	{
		enableEthernet=1;
		dputc ('P');
		if(testEthernet() == 1)
		{
			procChipStatus |= (0x1 << 14);  //setting bit 0 for comm. ctlr
		}
		else
		{
			enableEthernet=0;
			#ifdef VERBOSE
				printf("proc %d: failed ethernet test\n", procID);
			#endif
		}
		
		if(testPhy() == 1)
		{
			procChipStatus |= (0x1 << 15);  //setting bit 0 for comm. ctlr
		}
		else
		{
			enableEthernet=0;
			#ifdef VERBOSE
				printf("proc %d: failed phy test\n", procID);
			#endif
		}
	}
	result=setChipStatus(procChipStatus);
	if(result == 0)
	{
		#ifdef VERBOSE
			printf("proc %d: chip level test not fully successful\n", procID);
		#endif
		//return to put the processor into sleep mode
		return 0;
	}
	else
	{
		#ifdef VERBOSE
			printf("proc %d: setup chip status\n", procID);
		#endif
	}
	///////////////////////////////////// init process ///////////////////////////////////
	initRouter();
	setupRouterMCTable();
	setupRouterP2PTable();
	#ifdef VERBOSE
		printf("proc %d: init MCRouter\n", procID);
	#endif
	if(enableEthernet == 1)    //if phy is attached
	{
		initMacFromSerialRom();
		initIPAddressFromSerialRom();
		initEthernet();     //depends on initMacFromSerialRom() for initializing the mac address
		#ifdef VERBOSE
			printf("proc %d: init eth interface\n", procID);
		#endif
		initPhy();
		#ifdef VERBOSE
			printf("proc %d: init phy\n", procID);
		#endif
		
		//initFrameHeaders();
		//check it connection is established already
	}
	//initWatchdog();   //watchdog timer is not enabled and initialized during the bootup process - decided in meeting with SBF and ST etc
	
	//configure the sys ctlr to reset the chip on the watchdog interrupt
	//and configure the timer interrupt to reload the timer before the watchdog sends an interrupt
	//setResetOnWatchdogInt();
	
	#ifdef VERBOSE
   	 	printf("proc %d: watchdog timer init\n", procID);
    #endif
    return 1;
}
//int doFuntionalTests()
//{
//	//test comm noc, dma, and ethernet if enabled
//	if(testCommNoC() == 0)
//	{
//		#ifdef VERBOSE
//			printf("proc %d: failed comm. noc. test\n", procID);
//		#endif
//		//return for failure
//		return 0;
//	}
//	return 1;
//}
int bootupProcType0()
{	
  dputc ('0');

	if(testLocalPeripherals()==0)
	{
		return 0;
	}
	initCommCtl();
	#ifdef VERBOSE
		printf("proc %d: comm. ctlr. init\n", procID);
	#endif
	initDMAC();
	#ifdef VERBOSE
		printf("proc %d: DMA. ctlr. init\n", procID);
	#endif
	if(arbitForMonProc(procID)==1)   //to see if I am selected as mon proc by the system controller
	{
		dputc ('m');
		procType=MON_PROC;
		#ifdef VERBOSE
			printf("proc %d: selected as mon proc\n", procID); 
		#endif
/*
   !! ST  - proc below inits some vars in System RAM. These vars are
   read on return from this proc to "main" and the fascicle processor
   gets there before the monitor processor because the monitor does
   more work here. Moving "testInitChipResources" here is a bodge to
   avoid this race - a better mechanism is really needed!
*/
		
		if(testInitChipResources()==0) //if chip level tests and init is successful
			return 0;
		
		
	   	#ifdef VERBOSE
	   	 	printf("proc %d: chip conf. done\n", procID);
		#endif

		setChipClock();                           //initSysCtl();  
		
		#ifdef VERBOSE
			printf("proc %d: init sys ctlr with PLL\n", procID);
		#endif

/* !! ST moved up to try to avoid race
		if(testInitChipResources()==0) //if chip level tests and init is successful
			return 0;
	   	#ifdef VERBOSE
	   	 	printf("proc %d: chip conf. done\n", procID);
		#endif
*/

		Init_CRC32_Table();
		#ifdef VERBOSE
			printf("proc %d: init CRC32 lookup table\n", procID); 
		#endif
		initInterrupt();
	   	#ifdef VERBOSE
	   		printf("proc %d: init inturrupts...\n", procID);
	   	#endif
		enableInterrupts();
	   	#ifdef VERBOSE
			printf("proc %d: int en\n", procID);
	    #endif
		initTimer();
	    #ifdef VERBOSE
			printf("proc %d: Timer init\n", procID);
		#endif
		NNDiagTime=currentTime;
		MEM(globalVarSpace, SR_CONF_DONE)=1;  //conf done=true;
		#ifdef VERBOSE
			printf("proc %d: starting neighbours probing\n", procID);
		#endif
		checkInterchipLinks();
	}			
	else
	{
	  dputc ('f');
		procType=FASCICLE_PROC;
		#ifdef VERBOSE
			printf("proc %d: selected as fascicle proc\n", procID); 
		#endif
		initInterrupt();
	   	#ifdef VERBOSE
	   		printf("proc %d: init inturrupts...\n", procID);
	   	#endif
		enableInterrupts();
	   	#ifdef VERBOSE
			printf("proc %d: int en\n", procID);
	    #endif
	}
	//testBootRomAddress();			//test to see if the address has been remapped to ram
	//not required as the RAM can be used after the code has been copied into the ITCM, moreover we use the end of RAM for
	//the chip level variables, so it does not matter
	//initChipID();                //done temporarily, will be changed and later will be reinitialized by the system-level boot up process
	return 1;
}
/*
 * synchronise the local variables with the global ones like ethernetEnable, confDone, chipAddress, referenceChip, hostConnectedChip, etc
 *
 * 
*/
reinitLocalVariables()
{
	Init_CRC32_Table();
    #ifdef VERBOSE
		printf("proc %d: init CRC32 lookup table\n", procID); 
	#endif
}
int bootupProcType1()
{	
  dputc ('1');
	procType=MON_PROC;
	#ifdef VERBOSE
		printf("proc %d: selected as mon proc\n", procID); 
	#endif
	//read the cpu ok register to see if ethernet is enabled and set the variable
	//set the local variables as mon proc and the control register in the router to set new mon proc id
	reinitLocalVariables();
	NNDiagTime=currentTime;
	MEM(globalVarSpace, SR_CONF_DONE)=1;  //conf done=true;
   	initInterrupt();     //initialize the interrupts as mon proc
   	#ifdef VERBOSE
   		printf("proc %d: init inturrupts...\n", procID);
   	#endif
	enableInterrupts();
   	#ifdef VERBOSE
		printf("proc %d: int en\n", procID);
    #endif
	initTimer();
    #ifdef VERBOSE
		printf("proc %d: Timer init\n", procID);
	#endif
	#ifdef VERBOSE
		printf("proc %d: starting neighbours probing\n", procID);
	#endif
	checkInterchipLinks();
	#ifdef VERBOSE
   	 	printf("proc %d: chip conf. done\n", procID);
	#endif
	return 1;
}
int bootupProcType2()
{	
  dputc ('2');
	//just reinitialize the local variables and clear all interrupts, reassign itself as mon proc, rest everything is already done
	procType=MON_PROC;
	#ifdef VERBOSE
		printf("proc %d: selected as mon proc\n", procID); 
	#endif
	//read the cpu ok register to see if ethernet is enabled and set the variable
	//set the local variables as mon proc and the control register in the router to set new mon proc id
	reinitLocalVariables();
	NNDiagTime=currentTime;
	initInterrupt();     //initialize the interrupts as mon proc
   	#ifdef VERBOSE
   		printf("proc %d: init inturrupts...\n", procID);
   	#endif
   	MEM(globalVarSpace, SR_CONF_DONE)=1;  //conf done=true;
   	#ifdef VERBOSE
   	 	printf("proc %d: chip conf. done\n", procID);
	#endif
	enableInterrupts();
   	#ifdef VERBOSE
		printf("proc %d: int en\n", procID);
    #endif
	initTimer();
    #ifdef VERBOSE
		printf("proc %d: Timer init\n", procID);
	#endif
	#ifdef VERBOSE
		printf("chip: %d, proc %d: Timer init\n", MEM(globalVarSpace, SR_CHIP_ADDRESS), procID);
	#endif
	#ifdef VERBOSE
		printf("proc %d: starting neighbours probing\n", procID);
	#endif
	checkInterchipLinks();
	return 1;
}
/*******************************************************************
* first see if the monitor proc id=1F and the reset code=00 (power on reset), do the normal selection. 
* if the monitor proc !=1F and reset code=00, the neighbour has reset the processors, 
* if the mon proc id=own id and the arbiteration bit=1 (neighbour selected you as new neighbour), do as the monitor processor but donot test and initialize the components, 
* just do high level funcational tests to see if all the components are working fine or do the initialization
* if the arbiteration bit=0 (even if the mon proc id=own id), do the arbiteration (the neighbour reset all the processors to restart the chip)
*
* 
******************************************************************/
int bootupProc()
{
	//unsigned int procChipStatus=0;  //to keep record of the peripheral state, bit0=comm ctlr, bit1=int ctlr bit2=dma, bit3=timer,    ---- 
	//int result=0;					//bit8=rotuer, bit9=sysctrl, bit10=pl340, bit11=sdram, bit12=watchdog, bit13=ram, bit14=ethernet, bit15=phy (if there) 
 	unsigned int idReg=0;
	unsigned int resetCode=0;
	int bootupType=0, arbBit=0;
	#ifdef VERBOSE
		printf("proc %d: copied code to ITCM\n", procID);
	#endif

	dputc ('B'); // !!!

	initProcID();
	#ifdef VERBOSE
		printf("proc %d: procID init\n", procID);
	#endif
	idReg=readMemory(SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID);
	resetCode=readMemory(SYSCTLR_BASE_U+SYSCTLR_RESET_CODE);
	#ifdef VERBOSE
		printf("proc %d: monIdReg:0x%x, restcode:0x%x init\n", procID, idReg, resetCode);
	#endif
	arbBit=(idReg >> 8) & 0x1; //arbitration bit (8) in the mon proc id register
	resetCode=resetCode & 0x3; //first two bits
	changeToLowVectors();

	dputc ('V'); // !!!

	#ifdef VERBOSE
		printf("proc %d: changed to low vec\n", procID);
	#endif
	switch(resetCode)
	{
		case 0:
		{
			if(!arbBit)    //if the arbitration bit is low i.e. it is power on reset, do the normal arbitration and exit
			{
				bootupType=0;   //to see if I am selected as mon proc by the system controller
					
			}
			else		//if the arbiteration bit is high i.e. the nieghbouring chip did the soft reset
			{
				if(procID == (idReg & 0x1F)) //if the neighbour has assigned me as a new mon proc, i need to assign myself as the monitor proc
											//and avoid testing the components except the interrupts settings for mon proc and running 
											// the mon proc code
				{
					procType= MON_PROC; 	//setting itself as the mon proc
					if(MEM(globalVarSpace, SR_CHIP_ADDRESS)==1)  	//read global variable space in the system controller to know 
							bootupType = 1;								//if the configuration was done already then bootupType=1, to indicate that it does not have to do the detailed testing
					else
							bootupType = 0;						//configuration was not done successfully, so it needs to do every thing
				}
				else               	//if some other proc has been selected as the mon proc by the neighbouring chip, though not a valid state, just select itself as a fascicle and relax
				{
						bootupType = 0;	 //go to reset 0 to become fascicle and initialize every thing
				}
			}
			break;
		}
		case 1:  	//watchdog reset, everything is reset and needs initializing, 
					//the system controller will select another proc as the mon proc
		{
			bootupType = 0;;
			break;
		}
		case 2:     //not used but the same action as in case 1
		{
			bootupType = 0;
			break;
	
		}
		case 3:	 //the watchdog interrupt has been used as a soft reset to me, 
	 			 //I was going as the mon proc but had something wrong in the process
		{
			//the mon proc has to continue working as mon proc, dont initialize the peripherals,
			//return code=2 to indicate that it needs to only clear all the pending interrupts and do some functional tests, 
			//no need to configure the VIC now
			bootupType = 2;	//to indicate that it does not have to do the detailed testing and initialization 
							//just clear all interrupts including the watchdog interrupt	
			break;
		}
	}
	switch (bootupType)
	{
		case 0:
		{
			if(bootupProcType0()==0)
				return 0;
			break;
		}
		case 1:
		{
			if(bootupProcType1()==0)
				return 0;
			break;
		}
		case 2:
		{
			if(bootupProcType2()==0)
				return 0;
			break;
		}
		default:
			return 0;
			
	}
	return 1;
}
#endif /*BOOTPROCESS_H_*/
