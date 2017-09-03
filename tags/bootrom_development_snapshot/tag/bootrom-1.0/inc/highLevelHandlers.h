#ifndef HIGHLEVELHANDLERS_H_
#define HIGHLEVELHANDLERS_H_

#include "spinnakerlib.h"


/******************************************************************
 * exception handling functtions
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
	//unsigned int RTR_BASE=0xF1000000;
	unsigned int errPktHeader=0;
	unsigned int errPktKey=0;
	unsigned int errPktPLoad=0;
	int tmp=0;
	#ifdef VERBOSE
		printf("proc %d: in handleRouterError\n", procID);
	#endif
	//errorStatus=readMemory(RTR_BASE + 0x14);
	errPktHeader=readMemory(RTR_BASE + 0x8);
	errPktKey=readMemory(RTR_BASE + 0xC);
	errPktPLoad=readMemory(RTR_BASE + 0x10);
	rtrErrorCount=(errorStatus & 0xFFFF);  //15-0 bits for error count
	tmp=readMemory(RTR_BASE + 0x4);
	#ifdef VERBOSE
		printf("proc %d: status:0x%x, header:0x%x, errstatus:0x%x\n", procID, tmp, errPktHeader, errorStatus);
	#endif
	tmp=(errPktHeader >> 24)&& 0x7;
	#ifdef VERBOSE
		printf("proc %d: rtr error packet no:0x%x, from port:0x%x\n", procID, rtrErrorCount, tmp);
	#endif
	tmp=(errPktHeader >> 16)&& 0xFF;
	#ifdef VERBOSE
		printf("proc %d: con:0x%x, key:0x%x, payload:0x%x\n", procID, tmp, errPktKey, errPktPLoad);
	#endif
	/*if(errorStatus & (0x1 << 26))
	{
		rtrUndefPacketError=1;
		#ifdef VERBOSE
			printf("proc %d: got undef. packet error in router\n", procID);
	}
	if(errPktHeader & (0x1 << 27))
	{
		rtrTimephaseError=1;
		#ifdef VERBOSE
			printf("proc %d: got timephase error in router\n", procID);
	}
	if(errPktHeader & (0x1 << 28))
	{
		rtrFrammingError=1;
		#ifdef VERBOSE
			printf("proc %d: got framming error in router\n", procID);
	}
	if(errPktHeader & (0x1 << 29))
	{
		rtrParityError=1;
		#ifdef VERBOSE
			printf("proc %d: got parity error in router\n", procID);
	}
	if(errPktHeader & (0x1 << 30))
	{
		rtrErrorOverflow=1;
		#ifdef VERBOSE
			printf("proc %d: got error overflow in router\n", procID);
	}
	  	*/	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleRouterDumpInterrupt(unsigned int dmpStatus)
{
	//unsigned int RTR_BASE=0xF1000000;
	unsigned int dmpPktHeader=0;
	unsigned int dmpPktKey=0;
	unsigned int dmpPktPLoad=0;
	unsigned int dmpOutputs=0;
	int tmp=0;
	#ifdef VERBOSE
		printf("proc %d: in handleRouterDumpInterrupt\n", procID);
	#endif
	dmpPktHeader=readMemory(RTR_BASE + 0x18);
	dmpPktKey=readMemory(RTR_BASE + 0x1C);
	dmpPktPLoad=readMemory(RTR_BASE + 0x20);
	dmpOutputs=readMemory(RTR_BASE + 0x24);
	tmp=(dmpPktHeader >> 24)&& 0x7;
	#ifdef VERBOSE
		printf("proc %d: router dumped packet from port:0x%x\n, due to tx ports:0x%x, status:0x%x\n", procID, tmp, dmpOutputs, dmpStatus);
	#endif
	tmp=(dmpPktHeader >> 16)&& 0xFF;
	#ifdef VERBOSE
		printf("proc %d: con:0x%x, key:0x%x, payload:0x%x\n", procID, tmp, dmpPktKey, dmpPktPLoad);
	#endif
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleRouterDiagInterrupt()
{
	unsigned int value;
	#ifdef VERBOSE
		printf("proc %d: in handleRtrDiagCount\n", procID);
	#endif
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
	#ifdef VERBOSE
		printf("proc %d: diag con=0x%x\n", procID, value);
	#endif
	value=readMemory(0xF1000004);
	#ifdef VERBOSE
		printf("proc %d: rtr status=0x%x\n", procID, value);
	#endif
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleDMAErrorInt()
{
	#ifdef VERBOSE
		printf("proc %d: in handleDMAErrorInt\n", procID);
	#endif
			
	dmaError=1;
	#ifdef VERBOSE
		printf("proc %d: dma status:0x%x\n", procID, dmaErrorstatus);
	#endif
	dmaInProcess=0;		
	/*
	if(dmaErrorstatus & (0x1 << DMAC_STATUS_ENINT_CRCERROR))		//13
	{
		#ifdef VERBOSE
			printf("proc %d: crc error occured in dma\n", procID);
		#endif
			
	}
	if(dmaErrorstatus & (0x1 << DMAC_STATUS_ENINT_TCMERROR))		//14
	{
		#ifdef VERBOSE
			printf("proc %d: tcm error occured in dma\n", procID);
		#endif
			
	}
	if(dmaErrorstatus & (0x1 << DMAC_STATUS_ENINT_AXIERROR))		//15
	{
		#ifdef VERBOSE
			printf("proc %d: axi error occured in dma\n", procID);
		#endif
			
	}
	if(dmaErrorstatus & (0x1 << DMAC_STATUS_ENINT_USERABORT))		//16
	{
		#ifdef VERBOSE
			printf("proc %d: user abort error occured in dma\n", procID);
		#endif
			
	}
	if(dmaErrorstatus & (0x1 << DMAC_STATUS_ENINT_SOFTRST))			//17
	{
		#ifdef VERBOSE
			printf("proc %d: soft reset error occured in dma\n", procID);
		#endif
			
	}
	if(dmaErrorstatus & (0x1 << DMAC_STATUS_ENINT_WB))				//20
	{
		#ifdef VERBOSE
			printf("proc %d: buffered write error occured in dma\n", procID);
		#endif
			
	}
	*/
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleDMATimeoutInt()
{
	#ifdef VERBOSE
		printf("proc %d: in handleDMATimeoutInt\n", procID);
	#endif
	dmaTimeout=1;
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void  handleDMACompleted()
{
	#ifdef VERBOSE
		printf("proc %d: dma completed\n", procID);
	#endif
	//dmaInProcess=0;  //done in the isr
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleTimerInterrupt()
{
	#ifdef VERBOSE
		printf("proc %d: in handleTimerInterrupt\n", procID);
		printf("proc %d: time:%d\n", procID, currentTime);
	#endif
//	#ifdef RESET_ON_WATCHDOG_INT
//	//configure the sys ctlr to reset the chip on the watchdog interrupt
//	//and configure the timer interrupt to reload the timer before the watchdog sends an interrupt
//	if(currentTime % (WDOG_COUNTER_VALUE/2)) //reset the watchdog counter by writing into the clear interrupt register after and interval half the watchdog time
//	{
//		writeMemory(WDOG_BASE+WDOG_INTCLR, 1); //writing any value in the int clear register will clear any pending interrupt and reload the counter
//		#ifdef VERBOSE
//			printf("proc %d: reset the watchdog timer after %d ms\n", procID, (WDOG_COUNTER_VALUE/2));
//		#endif
//	}
//	#endif
	//see if the time for status out
	if((currentTime % RESPONSE_OUT_INTERVAL)==0) //set the flag to send the responses out
		responseOutDue=1;
	if(statusReportDue==1)
	{
		if((currentTime >= (reportStatusTime+WAIT_FOR_REPORTSTATUSOUT)) && !ackFrameDue) 
		{
			if(opMode==IDLE)
				opMode=SEND_STATUS_REPORT;
		}
	}
	
	
		
	//responseOutDue=1;  //setting the flag high
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleTimer2Interrupt(void)
{
	#ifdef VERBOSE
		printf("proc %d: in Tmr2Int.\n", procID);
		printf("proc %d: time:%d\n", procID, currentTime);
	#endif
			
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
#ifdef VERBOSE
    printf("proc %d: in HandlePhyInterrupt\n", procID);
#endif

// bit7 - energyon(connected)
// bit6 - autonegotiationComplete
// bit5 - remotefault
// bit4 - linkdown
// bit3 - ack
// bit2 - paralleldetectionfault
// bit1 - pagereceived

/* !! ST removed

    if (status & 0x80) //if bit 7 is high i.e poweron (connection detected)
      {
#ifdef VERBOSE
	printf("proc %d: powerOnPhyInt\n", procID);
#endif
	; //do something
      }
*/

    if (status & 0x40) //if bit 6 is high i.e. autonegotiation is done
      {
#ifdef VERBOSE
	printf("proc %d: autonegoPhyInt\n", procID);
#endif
	if (EthLinkUp == 0)
	  {
	    if (enableEthernet == 0)
	      {
		enableEthernet = 1;
		initEthernet ();
	      }
	    EthLinkUp = 1; 

//			if(opMode==HELLO)
//			{
//				sendHelloFrame();
//				frameSent=1;
//				ackDue=1;
//			}
			
	  }
      }

/* !! ST removed
    if(status & 0x20) //if bit 5 is high i.e. remote fault is detected
      {
#ifdef VERBOSE
	printf ("proc %d: remoteConnFaultInt\n", procID);
#endif
	; //do something
	}
*/

    if(status & 0x10) //if bit 4 is high i.e.link is down
      {
#ifdef VERBOSE
	printf ("proc %d: linkDownInt\n", procID);
#endif
	EthLinkUp = 0; //do something
      }
}

/*******************************************************************
 * 
 * 
 ******************************************************************/
void handleIncommingFrame()
{
	unsigned int instruction;
	unsigned int count; //, crc, ;
	unsigned char *temp, *temp1;
	//first get the frame to the rxFrameBase
	receiveFrame((unsigned int) rxFrameBase);
	//CRC is stored with the data in the rx_frame_buffer, however, its 4 bytes are not included in the frame_lenght
	//stored in the rx_desc_ram (bits 10-0) and left to the application to hendle with this
	#ifdef VERBOSE
		printf("proc %d: in handleIncommingFrame\n", procID);
	#endif
	
	//checkCRC(uint32_t* text, int len, unsigned int CRC)
	#ifdef VERBOSE
		printf("proc %d: in handleIncommingFrame\n", procID);
	#endif

	spinnakerDataSize=(rxFrameBase[dataLengthAt/4]) & 0xFFFF;  	//dataLengthAt contains the length of the data i.e. the data payload after option3

	//rxBufSize=rxBufSize >> 16;   //first 11 bits i.e. 2B to contain the length of frame

	temp=(unsigned char*)rxFrameBase;  //the bytes in the frame may not be word aligned
	#ifdef VERBOSE
		printf("proc %d: r F: frameSize:%X, crc:%X\n", procID, rxFrameSize, rxBufCRC);
		for(count=0; count < (rxFrameSize/4); count++)
		{
			printf("0x%x ", temp[count]);
		}
		printf("\n");
	#endif
	//no need to check the frame level crc as it has been checked in the ethernet interface
	//	rxBufCRC = temp[rxBufSize] & 0xFF;
	//	rxBufCRC = rxBufCRC | ((temp[rxBufSize+1] & 0xFF) << 8);
	//	rxBufCRC = rxBufCRC | ((temp[rxBufSize+2] & 0xFF) << 16);
	//	rxBufCRC = rxBufCRC | ((temp[rxBufSize+3] & 0xFF) << 24);
	//	
	//  rxBufCRC = rxFrameBase[(rxBufSize/4)];  // & 0xFF;   // crc 4 bytes are not included in the rxBufSize, but the CRC is there in the Ethernet interface rx_frame_buffer
	
	//	crc=Get_CRC((unsigned char*)rxFrameBase, rxBufSize);
	//	if(crc != rxBufCRC ) //checkCRC((uint32_t*)rxFrameBase, rxBufSize, rxBufCRC))
	//	{
	//		//exit for checksum failure
	//		#ifdef VERBOSE
	//			printf("proc %d: s F: error msg\n", procID);
	//		#endif
	//		sendErrorReportFrame(ERR_FRAME_CHECKSUM_MSG);  //send a frame to indicate there was a checksum error in the frame, to requet resending the frame
	//		return;
	//	}
	//rxDataBit=16;  //to keep the start of data in the frame soft, it will help in case we adopt some protocol
				   // like TCP/IP in which case the start of data bit can change as
				   // that may contain some headers (tcp, ip, etc).
	//important***** the word containing 2 bytes for length of frame contains instruction in the next 16 bits

   if(rxFrameBase[9]>>16 == 54321) {
    // CP 21stJuly2009 - check that this is a SpiNNaker frame (UDP Dest port 54321)
 
      if(!macDestAddressConfigured)
      {
          //    txFrameBase[1]=(sourceMACAddressLS << 16) | (destinationMACAddressHS & 0xFFFF);                //0x0001ACDE;    sourceMACAddressLS=0x48000001;    destinationMACAddressHS=0xACDE//source+destination address
          //    txFrameBase[2]=(sourceMACAddressHS << 16) | (sourceMACAddressLS >> 16);   
          destinationMACAddressHS=(rxFrameBase[1] & 0xFFFF);
          destinationMACAddressLS =rxFrameBase[2];
          macDestAddressConfigured=1;
      }
      // moved inside the check for spinnaker packets rather than outside


	instruction = rxFrameBase[spinnakerInstructionAt/4];
	instruction = instruction & 0xFFFF;
	
	switch(instruction)
	{
		case HELLO:  //hello
			#ifdef VERBOSE
				printf("proc %d: r F: hello\n", procID);
			#endif
			opMode=HELLO;
			break;
			
		case FLOOD_FILL:  //floodfill data
			//start flood fill, the data block 1K at the moment, is in the rxFrameBase. rxFrameBase[16/4] contains the size of the block, 
			//rxFrameBase[20/4] contains the physical address to copy the block to, and rxFrameBase[24/4] contains the crc of the block, the data starts from rxFrameBase[24/4]
			//first do the crc check on the block and if ok carry on with the floodfill, or send a frame asking for the block again
			//if a frame comes during the floodfill of already received frame, it will be discarded and reported to the Host to wait
			//for a while unless a message to abort comes from the Host
			#ifdef VERBOSE
				printf("proc %d: r F: FF instr\n", procID);
			#endif
			if(floofillInProcess==0)  //if a previous floodfill request is not ongoing
			{
				opMode=FLOOD_FILL;
				flood_fill_state=FF_BLOCK_START;
				floofillInProcess=1;
				#ifdef VERBOSE
					printf("proc %d: no pending FF, starting FF, bufSize:0x%x, stAddr:0x%x, crc:0x%x\n", procID, rxBufSize, rxBufStAddr, rxBufCRC);
				#endif
				
			}
			else
			{
				#ifdef VERBOSE
					printf("proc %d: FF already in process\n", procID);
				#endif
				formatAndSendFrame(FF_IN_PROCESS, 0, 0, 0);
				//don't change the opMode
			}
			break;
		
		case END_FLOOD_FILL:  //end of floodfill
			#ifdef VERBOSE
				printf("proc %d: r F: sendAck instr\n", procID);
			#endif
			opMode=SEND_ACK;
			break;
		
		case BREAK_SYMMETRY:  //break symmetry
			#ifdef VERBOSE
				printf("proc %d: r F: breakSym instr\n", procID);
			#endif
			//size of the system in the first word after the instruction
			//spinnakerSystemSize=rxFrameBase[instructionOption1At/4]; it will be done in main
			opMode=BREAK_SYMMETRY;
			break;
			
		case REPORT_STATUS: //report OK
			#ifdef VERBOSE
				printf("proc %d: r F: reportOk instr\n", procID);
			#endif
			opMode=REPORT_STATUS;
			break;
		
		case P2P_IN_COMM: //point to point communication with a particular chip, eg sending routing table entries
			#ifdef VERBOSE
				printf("proc %d: r F: P2P msg\n", procID);
			#endif
			//spinnnaker instruction in instruction field bit 15-0
			//p2pInInstruction with the spinnaker instruction bit 31-16
			//chipaddress in option1 bit 15-0, port no or proc address etc in option1 bit 31-16
			//datasize in option2
			//memory address in option3
			p2pInInstruction = ((rxFrameBase[spinnakerInstructionAt/4] >> 16) & 0xFFFF);
			p2pInAddress =(rxFrameBase[instructionOption1At/4] & 0xFFFF);  
			p2pInDataSize=(rxFrameBase[instructionOption2At/4] & 0xFFFF);
			//bit 31-16 proc id, bit 15-0 chip address
			//copy the data into p2pInDataBuffer to avoid being overwritten
			temp=(unsigned char*)rxFrameBase;
			temp1=(unsigned char*)p2pInDataBase;
			for(count=0; count<p2pInDataSize; count++)
				temp1[count]=temp[spinnakerDataAt + count];
			opMode=P2P_IN_COMM;
			break;
		case STIMULUS_IN:
			#ifdef VERBOSE
				printf("proc %d: r F: P2P msg\n", procID);
			#endif
			//			p2pInDataSize=(rxFrameBase[instructionOption2At/4] & 0xFFFF);
			//			temp=(unsigned char*)rxFrameBase;
			//			temp1=(unsigned char*)p2pInDataBase;
			//			for(count=0; count<p2pInDataSize; count++)
			//				temp1[count]=temp[spinnakerDataAt + count];
			opMode=STIMULUS_IN;
			break;
		case START_APPLICATION: //execute application, i.e. load the application from the SDRAM to the fascicles and start the application
			#ifdef VERBOSE
				printf("proc %d: r F: start application instr\n", procID);
			#endif
			opMode=START_APPLICATION;
			break;
			
		case LOAD_APPLICATION_DATA: //execute application, i.e. load the application from the SDRAM to the fascicles and start the application
			#ifdef VERBOSE
				printf("proc %d: r F: start application instr\n", procID);
			#endif
			opMode=LOAD_APPLICATION_DATA;
			break;
			
		case LOAD_MON_APPLICATION: //execute application, i.e. load the application from the SDRAM to the fascicles and start the application
			#ifdef VERBOSE
				printf("proc %d: r F: start application instr\n", procID);
			#endif
			opMode=LOAD_MON_APPLICATION;
			break;
			
		case ACK: //ack frame
			#ifdef VERBOSE
				printf("proc %d: r F: waitForAck instr\n", procID);
			#endif
			ackFrameDue=0;
			if(moreStatusReportsDue==1)  //more status reports are due
			{
				if(opMode==IDLE)
					opMode=SEND_STATUS_REPORT;
				else  //busy doing something else, wait for the time being and send the remaining status reports after the time
				{
					ackFrameDue=1;
					reportStatusTime = currentTime;
				}
			}
			break;	
			
		default:
		
			break;
	}
   }
    else {
      #ifdef VERBOSE
          printf("Info: NonSpiNNaker frame received on port %d - dropping.\n",rxFrameBase[9]>>16);
      #endif
      // opMode = IDLE; // mode change not required
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
	unsigned int con;
	con=(RxStatus >> 16)& 0xFF;
	#ifdef VERBOSE
	{
		printf(" proc %d: r P", procID);
		printf(" con=0x%x", con);
		printf(" rkey=0x%x", RxRoutingKey);
		printf(" pload=0x%x", RxPayload);
		if(con & 0x2)  //if bit 1 is high
			printf(" payload=0x%x\n", RxPayload );
		printf("\n");
	}
	#endif
	RxPacketType=(con >> 6) & 0x3;
	if(RxPacketType==NN)    //if monitor processor and packet is an NN type
	{
		if(procType==1) 			//if monitor processor then handle the nn packet
	 	  handleNNPacket();
	 	else
	 	{
	 		//report to the monitor processor about this lapse
	 		;
	 	}
	}
	else if(RxPacketType==P2P)
	{
		if(procType==1) 			//if monitor processor then handle the nn packet
	 	  handleP2PPacket();
	 	else
	 	{
	 		//report to the monitor processor about this lapse
	 		;
	 	}
	}
	else if(RxPacketType==MC)
	{
		if(procType==1) 			//if monitor processor then handle the nn packet
	 	  handleMCPacket();
	 	else
	 	{
	 		//report to the monitor processor about this lapse
	 		;
	 	}
	}
	else
	{
		//this should not happen 
		if(procType==1) 			//if monitor processor then do something about it 
	 	  handleUNDEFPacket();
	 	else
	 	{
	 		//report to the monitor processor about this lapse
	 		;
	 	}	
	}
}	
#endif /*HIGHLEVELHANDLERS_H_*/
