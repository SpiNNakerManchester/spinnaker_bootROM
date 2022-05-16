#include "spinnakerlib.h"
#include "../inc/globals.h"


/******************************************************************
 * exception handling functions
 *////////////////////////////////////////////////////////////////
void undefHandler()
{
	#ifdef VERBOSE
		printf("proc %d: r I: undefined exception, going to sleep\n", procID);
	#endif
	disableInterrupts();
	gotoSleep();
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void swiHandler()
{
	#ifdef VERBOSE
		printf("proc %d:  r I: software interrupt\n", procID);
	#endif
	servSoftware();
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void prefetchHandler()
{
	#ifdef VERBOSE
		printf("proc %d: r I: prefetch interrupt, going to sleep\n", procID);
	#endif
	disableInterrupts();
	gotoSleep();
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void abortHandler()
{
	#ifdef VERBOSE
		printf("proc %d: r I: abort interrupt\n", procID);
	#endif
	disableInterrupts();
	gotoSleep();
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void resHandler()
{
	#ifdef VERBOSE
		printf("proc %d: r I: reserved interrupt\n", procID);
	#endif
	disableInterrupts();
	gotoSleep();
}
/*******************************************************************
 * to pool the int status register in the int ctl to find the souce of
 * int. it is useful to know in case of non vectored int 
 * in case of a non-vectored int, the address of this routine will be 
 * passed by the vector address register and the pc will move to this 
 * function. this function will then find the source of the int and 
 * after finding the source will move to the ISR.
 * 
 ******************************************************************/
void poolVicStatus()
{
	unsigned int status;
	#ifdef VERBOSE
		printf("proc %d: r I: non vectored int. pooling the vic status\n", procID);
	#endif
	status=readMemory(VIC_BASE); //reading status register at offset 0
	if(status & 1) //int 0
	;
	else if(status & (1 << 1))
	;
	else if(status & (1 << 2))
	;
	else if(status & (1 << 3))
	;
	else if(status & (1 << 4))
	;
	else if(status & (1 << 5))
	;
	else if(status & (1 << 6))
	;
	else if(status & (1 << 7))
	;
	else if(status & (1 << 8))
	;
	else if(status & (1 << 9))
	;
		
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleRouterErrorInterrupt(unsigned int errorStatus)
{
	
	#ifdef VERBOSE
	  //unsigned int RTR_BASE=0xF1000000;
	  unsigned int errPktHeader=0;
	  unsigned int errPktKey=0;
	  unsigned int errPktPLoad=0;
	  int tmp=0;

	  printf("proc %d: in handleRouterError\n", procID);
	  //errorStatus=readMemory(RTR_BASE + 0x14);
  	  errPktHeader=readMemory(RTR_BASE + 0x8);
	  errPktKey=readMemory(RTR_BASE + 0xC);
	  errPktPLoad=readMemory(RTR_BASE + 0x10);
	
	  tmp=readMemory(RTR_BASE + 0x4);
	  printf("proc %d: status:0x%x, header:0x%x, errstatus:0x%x\n", procID, tmp, errPktHeader, errorStatus);

	  tmp=(errPktHeader >> 24)&& 0x7;
	  printf("proc %d: rtr error packet no:0x%x, from port:0x%x\n", procID, rtrErrorCount, tmp);

  	  tmp=(errPktHeader >> 16)&& 0xFF;
	  printf("proc %d: con:0x%x, key:0x%x, payload:0x%x\n", procID, tmp, errPktKey, errPktPLoad);
	#endif
	
	rtrErrorCount=(errorStatus & 0xFFFF);  //15-0 bits for error count
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleRouterDumpInterrupt(unsigned int dmpStatus)
{

	#ifdef VERBOSE
	  //unsigned int RTR_BASE=0xF1000000;
  	  unsigned int dmpPktHeader=0;
	  unsigned int dmpPktKey=0;
	  unsigned int dmpPktPLoad=0;
	  unsigned int dmpOutputs=0;
  	  int tmp=0;
	  printf("proc %d: in handleRouterDumpInterrupt\n", procID);

  	  dmpPktHeader=readMemory(RTR_BASE + 0x18);
	  dmpPktKey=readMemory(RTR_BASE + 0x1C);
	  dmpPktPLoad=readMemory(RTR_BASE + 0x20);
	  dmpOutputs=readMemory(RTR_BASE + 0x24);

	  tmp=(dmpPktHeader >> 24)&& 0x7;
	  printf("proc %d: router dumped packet from port:0x%x\n, due to tx ports:0x%x, status:0x%x\n", procID, tmp, dmpOutputs, dmpStatus);

	  tmp=(dmpPktHeader >> 16)&& 0xFF;
	  printf("proc %d: con:0x%x, key:0x%x, payload:0x%x\n", procID, tmp, dmpPktKey, dmpPktPLoad);
	#endif
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleRouterDiagInterrupt()
{
	
	#ifdef VERBOSE
          unsigned int value;
  	  printf("proc %d: in handleRtrDiagCount\n", procID);
	//operate on rtrDiagCount[8]
	//routerDiagDone=1;
	//value=readMemory(0xFFFFF008);
	//#ifdef VERBOSE
	//	printf("proc %d: vic rawint=0x%x\n", procID, value);
	//#endif
	//value=readMemory(0xFFFFF000);
	//#ifdef VERBOSE
	//	printf("proc %d: vic irqstatus=0x%x\n", procID, value);
	//#endif
 	  value=readMemory(0xF1000200);
	  printf("proc %d: diag con=0x%x\n", procID, value);
	  value=readMemory(0xF1000004);
	  printf("proc %d: rtr status=0x%x\n", procID, value);
	#endif
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleDMAErrorInt()
{	
	dmaError=1;
	dmaInProcess=0;		

}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleDMATimeoutInt()
{
	dmaTimeout=1;
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void  handleDMACompleted()
{
	crcOK=1; 			// this sets the CRC check as OK if the DMA was completed successfully (only used if required when CRC is on).
	//dmaInProcess=0;  //done in the isr
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleTimerInterrupt()
{
	// very important routine here - if we use the timer - usually 1ms interrupt arrives here
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleTimer2Interrupt(void)
{
			
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void clearDMADoneInt()
{
	#ifdef TLM_MODEL
		writeMemory(0x30000010, (1 << DMAC_CTRL_CLINT_DONE)); //writing at bit 3 of dma ctrl register to clear dma done interrupt
	#else
		writeMemory(0x30000008, 1);       //writing any value (1) to DMACIntTCClear register to clear dma done interrupt
	#endif
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void clearDMAErrorInt()
{
	#ifdef TLM_MODEL
		unsigned int intclear=0;
		//first read the status register to make out the error, then clear it
		dmaErrorstatus=readMemory(0x30000014); //status register
		intclear=(0x1 << DMAC_CTRL_RESTART) | (0x1 << DMAC_CTRL_CLINT_WB);
		writeMemory(0x30000010, intclear); //writing at bit 3 of dma ctrl register to clear dma done interrupt
	#else
		writeMemory(0x30000008, 1);       //writing any value (1) to DMACIntTCClear register to clear dma done interrupt
	#endif
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void clearDMATimeoutInt()
{
	#ifdef TLM_MODEL
		writeMemory(0x30000010, (1 << DMAC_CTRL_CLINT_TIMEOUT)); //writing at bit 3 of dma ctrl register to clear dma done interrupt
	#else
		writeMemory(0x30000008, 1);       //writing any value (1) to DMACIntTCClear register to clear dma done interrupt
	#endif
}

/*******************************************************************
 * 
 * 
 ******************************************************************/

/* !! ST

Only two interrupts are enabled - Auto-Neg Complete and Link Down,
These are bits 6 and 4 in the status word.

*/

void handlePhyInterrupt (int status)
{
// bit7 - energyon(connected)
// bit6 - autonegotiationComplete
// bit5 - remotefault
// bit4 - linkdown
// bit3 - ack
// bit2 - paralleldetectionfault
// bit1 - pagereceived

	if(status & 0x10) //if bit 4 is high i.e.link is down
	{
		EthLinkUp = 0; //do something
		MEM(globalVarSpace, SR_HOST_CONNECTED_CHIP) = 0;
	}
	if (status & 0x40) //if bit 6 is high i.e. autonegotiation is done
	{
		dputc('N');
		if (EthLinkUp == 0)
		{
			if (enableEthernet == 0) //TODO Decide on how eth is brought up
			{
				enableEthernet = 1;
				initEthernet ();
			}
			EthLinkUp = 1; 
			MEM(globalVarSpace, SR_HOST_CONNECTED_CHIP) = 1;

			if(macDestAddressConfigured==0) 
			{
				opMode=HELLO;  // CP 15th Jan, if we haven't see the host (dest MAC not configured), then Eth is up and we need to send hellos
			}
		}
	}
}

/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleIncomingFrame()
{
	unsigned int instruction;
	unsigned int count; 
	unsigned char *temp;
	//first get the frame to the rxFrameBase
	
	receiveFrame((unsigned int) rxFrameBase);
	//CRC is stored with the data in the rx_frame_buffer, however, its 4 bytes are not included in the frame_length
	
	spinnakerDataSize=60+(rxFrameBase[dataLengthAt/4]);  	//dataLengthAt contains the length of the SpiNNaker frame inc. data payload after option3
	temp=(unsigned char*)rxFrameBase;  //the bytes in the frame may not be word aligned

	if (  (rxFrameBase[3] >> 16 == 0x0800)  &&   // 0x800 = IP  (define?)
		((rxFrameBase[5] & 0xFF) == 17)  &&       // 17 = UDP (define?)
		(rxFrameBase[9] >> 16 == 54321)  &&   // identified as a spinnaker packet 54321 (define?)
		 (rxFrameBase[10] & 0xFFFF == SPINNPROTVERSION) )   // make sure we understand the protocol version  
	{
 
		if(!macDestAddressConfigured)
		{
			destinationMACAddressHS=(rxFrameBase[1] & 0xFFFF);   // if we don't know our SpiNNaker server's MAC
			destinationMACAddressLS =rxFrameBase[2];			// then learn it's MAC address so we can construct 
			macDestAddressConfigured=1;							// unicast frames directly to it rather than b/cast
		}

		instruction = ((rxFrameBase[spinnakerInstructionAt/4]) & 0xFFFF);  // get the SpiNNaker Opcode to use for switch case...
	
		switch(instruction)
		{
			case HELLO:
				opMode=HELLO;  // effectively a SpiNNaker ping - reponds with a hello_ack to validate aliveness 
				break;
			
			case FLOOD_FILL_START:
				if(Ph1FloodStarted == 0)  // only if we haven't received a start packet yet (from Eth or Ext)
				{
					FFTarget = (rxFrameBase[instructionOption1At/4]) &0xFF;  // learn target for the file (should be monitor only for phase1 fill = 0)
      				FFBlockCount = (rxFrameBase[instructionOption3At/4]) & 0xFF;	// learn number of blocks in this flood fill
      				if( FFTarget == 0 ) 						// only if this floodfill is targetted at the monitor then listen
      				{
      					for (i=0;i<256;i++) BlocksBitArray[i]=0;		// this initialises the blocks populated array to zeros
      					for (i=0;i<ITCMIMAGESIZE;i++) itcmimage[i]=0;	// this initialises the full itcm bootrom image to blank
						Ph1FloodStarted = 1;				// start listening for Ethernet Ph1 load!
      				}
				}
				
				break;
			
				
			case FLOOD_FILL_PACKET:  //floodfill data
						
				if (Ph1FloodStarted == 1)   // if we haven't heard the start of an Eth flood fill then why bother listening?
				{
					BlockID = (rxFrameBase[instructionOption1At/4])&0xFF;  // discover the block ID that is being sent
					if (BlocksBitArray[BlockID] == 0)     // if we have not already loaded this block 
					{	
						Ph1BlockWordCount=(rxFrameBase[instructionOption1At/4]>>8)&0xFF; // find number of words expected in the block
						Ph1BlockBaseAddress=rxFrameBase[instructionOption3At/4];	// find block base address (where to locate block in memory map)
						
						// copy data to itcm image array in DTCM (implicitly checked via CRC in Ethernet frame)
						for (i=0;i<Ph1BlockWordCount;i++) itcmimage[Ph1BlockBaseAddress+(BlockID*Ph1BlockWordCount)+i]=rxFrameBase[(spinnakerDataAt/4)+i];
						
						CheckIfFFDataComplete(); // check if all blocks are in place in itcm image, if so, get going! 
     				} 
				}
				
				
				
				break;
		
			case FLOOD_FILL_END:  //end of floodfill, control information here
				if(Ph1FloodStarted == 1)  // only if we have received a start packet on Eth already
				{
					if (rxFrameBase[instructionOption1At/4]&0xFF == 0x1)   // assigning 0x1 = execute for now, just like ID=1 for our purposes. 
					{
						FFExecuteAddress = rxFrameBase[instructionOption3At/4];  // what address we need to get the target devices to start executing at in their mem map
						Ph1FFCtrlReceived = 1;  //mark that we have received the control field
					}
				}				
				break;
			
			default:
				// do nowt as means nothing to me ---- oh vienna...
				break;
		}
	}
		  
	else if (  (rxFrameBase[3] >> 16 == 0x806)  &&   //   0x806 == ARP  
	         ( ( (rxFrameBase[9] << 16) | (rxFrameBase[10] >> 16) ) == sourceIPAddress) ) // ARP request for me
	{
		handleArpRequest();
	}
	
	else if (  (rxFrameBase[3] >> 16 == 0x800)  &&    // it's an IP packet 
		   ((rxFrameBase[5] & 0xFF) == 1)  &&    // protocol is ICMP
		   (((rxFrameBase[8] >> 8) & 0xFF) == 8)  &&   // icmp echo request
		   (((rxFrameBase[7] << 16) | (rxFrameBase[(destIPAt+4)/4] >> 16)) == sourceIPAddress)  )  //  ping request aimed at me
	{
		handlePingRequest();
	}	
	
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleFrameTxInt()
{
	#ifdef VERBOSE
		printf("proc %d: in handleFrameTxInt\n", procID);
	#endif	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleWatchdogInterrupt()
{
	#ifdef VERBOSE
		printf("proc %d: in handleWatchdogTimer\n", procID);
	#endif
	
}	
void handleParityErrorInterrupt()
{
	#ifdef VERBOSE
		printf("proc %d: in handleParityErrorInterrupt\n", procID);
	#endif
	
	
}
void handleFrammingErrorInterrupt()
{
	#ifdef VERBOSE
		printf("proc %d: in handleFrammingErrorInterrupt\n", procID);
	#endif
	
}
void handleTxFullInterrupt()
{
	//unsigned int con=0;
	#ifdef VERBOSE
		printf("proc %d: handleTXFullInterrupt\n", procID);
	#endif
	txFull=1;     //setting the flag for the application to do polling before sending anoter packet
//	sendMCByKey(0x131);
//	sendMCByKey(0x132);
//	sendMCByKey(0x133);
	//con=readMemory(0x10000000);
	#ifdef VERBOSE
		printf("proc %d: comm ctlr r0:0x%x.\n", procID, readMemory(0x10000000));
	#endif
/*	con=readMemory(0x1000000C);
	#ifdef VERBOSE
		printf("proc %d: comm ctlr r3:0x%x.\n", procID, con);
	#endif
	con=readMemory(0xFFFFF000);
	#ifdef VERBOSE
		printf("proc %d: VIC int status:0x%x.\n", procID, con);
	#endif
	con=readMemory(0xFFFFF008);
	#ifdef VERBOSE
		printf("proc %d: vic raw int:0x%x.\n", procID, con);
	#endif
	con=readMemory(0xFFFFF010);
	#ifdef VERBOSE
		printf("proc %d: vic int enable:0x%x.\n", procID, con);
	#endif
	*/
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleTxOverrunInterrupt()
{
	#ifdef VERBOSE
		printf("proc %d: handleTXOverrunInterrupt\n", procID);
	#endif
	txOverrun=1;
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleReceivedPacket()
{	
	if(((RxStatus >> 22) & 0x3) == NN)
	{
		handleNNPacket();
	}
	else
	{
		//TODO discard packet and clear buffers - probably not needed as read and variables populated;
	}
}