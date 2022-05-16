#ifndef SPINNAKERLIB_H_
#define SPINNAKERLIB_H_

#include "../inc/globals.h"

#include <stdio.h>
#include <stdarg.h>
// CP added 18th Jan 2009 to support printf overloading.

//#include <stdlib.h>
//#include "routingEntries.h"

/******************************************************************************************************
 * 						global functions from Assembly
 * 
 * ****************************************************************************************************/
extern void servSoftware(void); 				//from intService.s
extern void initProcID(void);					//from proc.s
extern void initChipID(void);					//from proc.s
extern void initPL340(void);					//from pl340.s
extern void initCommCtl(void);					//from commctl.s
extern void initInterrupt(void);				//from intctl.s
extern void initTimer(void);					//from timer.s
extern void initRouter(void);					//from mcrouter.s
//extern void initSysCtl(void);					//from sysctl.s
extern void setChipClock(void);					//from sysctl.s
extern void initEthernet(void);					//from ethernet.s
extern void initWatchdog(void);					//from watchdog.s
extern void initDMAC(void);						//from dmac.s
extern void	initPhy(void);						//from phy.s
extern int  testPhy(void);						//from phy.s
extern int  detectPhy(void);					//from phy.s
extern int testPL340(void);						//from pl340.s
extern int testSDRAM(void);						//from pl340.s
extern int testRAM(void);						//from ram.s
extern int testCommCtl(void);					//from commctl.s
extern int testDMAC(void);						//from dmac.s
extern int testIntCtl(void);					//from intctl.s
extern int testTimer(void);						//from timer.s
extern int testRouter(void);					//from mcrouter.s	
extern int testSysCtl(void);					//from sysctl.s
extern int testEthernet(void);					//from ethernet.s
extern int testWatchdog(void);					//from watchdog.s
extern void  gotoSleep(void);					//from proc.s
extern int  arbitForMonProc(int p);			//from proc.s
extern void  changeToLowVectors(void);			//from proc.s
extern void  copyVectorToITCM(void);			//from proc.s
extern void  gotoSleep(void);					//from proc.s
//extern void  enableIRQ(void);					//from proc.s
//extern void  enableFIQ(void);					//from proc.s
extern void  enableInterrupts(void);			//from proc.s
//extern void  disableIRQ(void);					//from proc.s
//extern void  disableFIQ(void);					//from proc.s
extern void  disableInterrupts(void);			//from proc.s
extern void  switchToITCM(void);				//from proc.s
//extern void  copyCodeToITCM(void);				//from proc.s
extern unsigned int  readMemory(unsigned int address); //from proc.s, returns value at the address if successfully read
extern void  writeMemory(unsigned int address, unsigned int value);  //from proc.s, low level function to write to memory,
extern writePacketWOPL(unsigned int con, unsigned int rk);   //parameters r0= Tx control, r1=routingKey, from systemLib.s 
extern writePacketWPL(unsigned int con, unsigned int rk, unsigned int pl); //parameters r0= Tx control, r1=routingKey, r2 = payload, from systemLib.s
//extern writePacketASM(unsigned int con, unsigned int rk, unsigned int pl); //parameters r0= Tx control, r1=routingKey, r2 = payload, from systemLib.s
extern sendSpike(unsigned int rk);     			//r0=rk, , from systemLib.s 	
extern void startDMAWrite(unsigned int dmafrom, unsigned int dmato, unsigned int noofBytes);	//from systemLib.s
extern void startDMARead(unsigned int dmafrom, unsigned int dmato, unsigned int noofBytes);	//from systemLib.s
extern void gotoSleep(void);					//from proc.s
extern void sendProcInt(int pid);				//from sysctl.s, used to send an interrupt to another processor e.g. shared memory message passing
extern int setCPUStatus(int pid, unsigned int status);				//from sysctl.s, used to set OK status of a processor, each processor writes its corresponding bit as high
extern void clearCPUStatus(int pid);			//from sysctl.s, used to clear the status of a particular CPU from live to dead
extern int  setChipStatus(unsigned int status);				//from sysctl.s, used to setup chip status in sys ctl. this can be used to report the status of the chip to the host system		
//extern void	sendFrameASM(unsigned int buffAddress, int len); //from ethernet.s
//extern void receiveFrameASM(unsigned int buffAddress);					//from ethernet.s
extern void setupMCTable(unsigned int offset, unsigned int route, unsigned int key, unsigned int mask); 
extern void setupP2PTable(unsigned int offset, unsigned int value); 
extern void SetupSourceID(int sourceID);		//from commctl.s
extern void loadApplicationAndStart(unsigned int source, unsigned int destination, unsigned int size, unsigned int mainAddress);
extern void loadApplicationData(unsigned int source, unsigned int destination, unsigned int size);
extern void dputc(char ch); // CP testing printf from start.s

/*******************************************************************
 * 							c functions
 * 
 ******************************************************************/

//-------------------------------------------------------------
// CP added in order to support printf for debugging 18th Jan 2010

#pragma import(__use_no_semihosting_swi)     // disable semihosting

/* These must be defined to avoid linking in stdio.o from the C Library*/

struct __FILE { int handle;   /* Add whatever you need here */};
FILE __stdout;
FILE __stdin;

void _sys_exit(void)
{
	while(1);
}

int fputc(int ch, FILE *f)
{ 
	/* e.g. write a character to an UART */
	char tempch = ch;
	dputc(tempch);
	return ch;
}

void _ttywrch(int ch)
{
   char tempch = ch;

   dputc(tempch);
}

// CP added in order to support printf for debugging 18th Jan 2010
//-------------------------------------------------------------





 
 /*******************************************************************
 * to read or write to a memory location
 * 
 ******************************************************************/
#define MEM(base, offset)	(*(volatile unsigned int *)(base + offset))
#define NNCon(rt, npt)  	((2 << 6)| ((npt & 0x1) << 5)| ((rt & 0x7) << 2)| (1 << 1))   //NN packet  0b10 i.e. 2   bit 7-6
#define CommCtlR0(route, con, id)   ((id & 0xFFFF)| (con << 16) | (route <<24))						  															 //NN Packet Type 0 normal, 1 direct bit 5
																						//NN route type bit 4-2
																						//data bit bit 1
			
#ifdef TLM_MODEL        //preprocessor to control the behaviour of printf() on TLM
	/******************************************************************
	 * 					funciton to support printf on TLM
	 * ***************************************************************/
	#ifdef VERBOSE
		int fputc(int ch, FILE *f)
		{ 
			// e.g. write a character to an UART 
			char tempch = ch;
			MEM(SYSCTLR_BASE_U, 0)= tempch;
			//sendCharASM(tempch);
			return ch;
		}	
	#endif
#endif	






/*******************************************************************
 * 
 * 
 ******************************************************************/
int readRouterTimePhase()
{
	unsigned int tphase;
	tphase=readMemory(RTR_BASE+RTR_R0_CONTROL);
	return ((tphase >> 6) & 3);
}
void setRouterTimePhase(int t)
{
	unsigned int tphase=0;
	tphase=readMemory(RTR_BASE+RTR_R0_CONTROL);
	tphase =tphase & 0xFFFFFF3F;    //clearing bit 7-6 containing time phase
	t = t & 0x3;  //to make sure it is only two bit wide
	tphase = tphase | (t  << 6);
	writeMemory(RTR_BASE+RTR_R0_CONTROL, tphase);
}


void handleNNPacket()
{
	
	//TODO - do we want to add an escape - so that eg. 1sec after FloodFill starts if we've not completed then it resets ?
    
	//local variables
	unsigned char calculatedchecksum=0, opcode, WordOffset;
	unsigned int i=0;
  	pointer BlockDataArray=(unsigned int*)SYS_RAM_FF_BLOCK;
  		
  		
    //calculate the 4-bit 1s complement of the received 64bit header+payload 
	for (i=0;i<7;i++) calculatedchecksum+=(RxRoutingKey>>(i*4)) & 0xF;
	for (i=0;i<8;i++) calculatedchecksum+=(RxPayload>>(i*4)) & 0xF;
	while(calculatedchecksum&0xF0) calculatedchecksum = (calculatedchecksum & 0xF)+(calculatedchecksum >> 4);   
    calculatedchecksum = 0xFFFF-calculatedchecksum; 
    
    if ( (RxRoutingKey>>28 == calculatedchecksum) && (RxRoutingKey&0xF == 0x3) )  // nn checksum OK, and packet ID==1, and ST's format (1) 
    {
		opcode = (RxRoutingKey>>24)&0xF;   // what OPCODE does our packet have?
		switch (opcode) // switch on OPCODE
		{
			case 1: //case 1=FFStart
			{
				if(Ph1FloodStarted == 0)  // only if we haven't received a start packet yet (on Eth or Ext)
				{
					FFTarget = (RxPayload>>24) &0xFF;  		// learn target for the file (should be monitor only for phase1 fill = 0)
      				FFBlockCount = (RxPayload>>16) & 0xFF;	// learn number of blocks in this flood fill
      				if (FFTarget == 0) 						// only if this floodfill is targetted at the monitor then listen
      				{
      					for (i=0;i<256;i++) BlocksBitArray[i]=0;	// this initialises the blocks populated array to zeros
						for (i=0;i<256;i++) DataBitArray[i]=0;		// this initialises the data words populated array to zeros
      					Ph1FloodStarted = 2;				// start listening!  Type2 = External Links, Type1 = Ethernet
      				}
				}
      			break;
      		}		
			case 2: //case 2=FF Block Start
			{	
				if (Ph1FloodStarted == 2)   // if we haven't heard the start of a flood fill then why bother listening?
				{
					if(Ph1BlockIDStarted == 0xFFFF)  // if we are not currently listening for a block
					{
						BlockID = (RxRoutingKey>>16)&0xFF;  // discover the block ID that is starting to be sent
						if (BlocksBitArray[BlockID] == 0) {    // if we have not already completed loading this block 
							Ph1BlockIDStarted=BlockID;  // if not already done, start listening for this block
							Ph1BlockWordCount=(RxRoutingKey>>8)&0xFF; // find number of words expected in the block
							Ph1BlockBaseAddress=RxPayload;	// find block base address (where to locate block in memory map)
						}
					}
				}
      			break;
      		}
      		case 3:   // case 3=FF Block Data
      		{
      			BlockID = (RxRoutingKey>>16)&0xFF;  // discover the block ID that this block data word comes from
      			if(BlockID == Ph1BlockIDStarted) // if this data is for our block (if =0xFFFF, not listening then ignore)
      			{
      				WordOffset = (RxRoutingKey>>8)&0xFF;   // find position in block to place data
      				
      				if (DataBitArray[WordOffset] == 0)  //if we haven't already heard this word
      				{
      					DataBitArray[WordOffset]=1;		// mark word as received in receive array
      					BlockDataArray[WordOffset]=RxPayload;  // load word data into SRAM memory location
      				}
      			}
      			break;
      		}
      		
			case 4:  //		case 4=FF Block end       		
      		{
      			BlockID = (RxRoutingKey>>16)&0xFF;  // discover the block ID that this block data word comes from
      			if(BlockID == Ph1BlockIDStarted) // if this data is for our block (if =0xFFFF, not listening then ignore)
      			{
      				blockCRC=RxPayload;
      				BlockDataArray[Ph1BlockWordCount]=blockCRC;  // load CRC into final position of block
      				
      				BlockFullyPopulated=1;  // check to see if all data positions in block have been populated
      				for(i=0;i<Ph1BlockWordCount;i++)  if (DataBitArray[i]==0) BlockFullyPopulated=0;
					
					if (BlockFullyPopulated)
					{
						while(dmaInProcess)	{;} //wait if an existing DMA is in progress
						crcOK=0;  // this will be set to 1 when a sucessful DMA has been recorded
						DMARead(unsigned int BlockDataArray, unsigned int itcmimage[(BlockID*Ph1BlockWordCount)], int Ph1BlockWordCount*4, 1); 
						// this performs a DMA read into the image in dtcm of block size, with CRC enabled 						
						while(dmaInProcess)	{;} //wait for DMA operation to complete
						
						if (crcOK)  CheckIfFFDataComplete() // if DMA was OK, check if all blocks are in place in itcm image, if so, get going! 
					
						for(i=0;i<Ph1BlockWordCount;i++)  DataBitArray[i]=0;  // block dealt with - clear out block data received bits
						Ph1BlockIDStarted =0xFFFF;   // ready for the next block to be sent - whatever that may be
					}
      			}
      		}	
      						
      		case 5:   // case 5=FF Control
      		{
				if(Ph1FloodStarted == 2)  // only if we have received a start packet on Ext by now
				{
					if ((RxRoutingKey>>8)&0xFF == 0x1)   // assigning 0x1 = execute for now, just like ID=1 for our purposes. 
					{
						FFExecuteAddress = RxPayload;  // what address we need to get the target devices to start executing at in their mem map
						Ph1FFCtrlReceived = 1;  //mark that we have received the control field
					}
				}				
      			break;
      		}
      		
      		default:
      		{
      			// do nothing! ie. drop nn packet and forget it ever existed.
      		}
			
		}

    }
    
    
 

      //  init vars:  unsigned short Ph1FloodStarted = 0, Ph1BlockIDStarted = 0xFFFF
      //  init arrays:  BlockBitArray(256 bits) - DTCM    BlockDataArray (256 words) - SRAM,   DataBitArray (256 bits) - DTCM   
      //  checksum 
      //     if OK.  
      //     we are looking at (ID==1, 1st phase flood fill only)
      //     we verify that bit 0 is set high also (ST packet format)
      //       Case for different packet types:
      //
      //	case 1=FFStart
      //       learn target for the file (should be monitor only for ph1 fill),  learn block count
      //       set Ph1FloodStarted = 1;
      //
      //	case 2=FF Block Start
      //       if Ph1FloodStarted == 1, 
      //          if Ph1BlockIDStarted==0xFFFF &&  BlocksBitArray[BlockID]!=Ph1BlockIDStarted (not already done this block)
      //             learn BaseAddress, assign Ph1BlockIDStarted & Ph1BlockWordCount (if both <=256)
      //                 
      //	case 3=FF Block Data
      //             if BlockID == Ph1BlockIDStarted
      //                read data word, and WordOffset
      //                if databitarray[WordOffset] == 0,     (worth checking, or should we just add it?)
      //                   place in SRAM BlockDataArray[WordOffset];  update DataBitArray[WordOffset];
      // 
      //	case 4=FF Block end
      //             if BlockID == Ph1BlockIDStarted
      //                read block CRC32
      //                if  databitarray[]== full, then do CRC check
      //                   if CRC check good, 
      //                       DMA length Ph1BlockWordCount from SRAM BlockDataArray to BaseAddress+(Ph1BlockWordCount*BlockID)
      //                          NOTE: BlockSize must be uniform throughout the load for this to work!   
      //                          NOTE: Ph1 is only for monitor processors, so this doesn't have to cover loading of fasc - Ph2 s/w!
      //                       Update BlocksBitArray[BlockID];       
      //                       Ph1BlockIDStarted =0xFFFF
      //                       if  BlockBitArray [] == full && Ph1FFCtrlReceived then Flood Fill Complete. !!!Ready for function!!!                   
      //                   if CRC fails,
      //                       clearout DataBitArray[], so will populate & try again
      //                   
      //	case 5=FF Control
      //          if function==execute
      //               read and populate ExecuteAddress  
      //               Ph1FFCtrlReceived==1;
      //
      //    case default=drop and do nothing
      //
      //
    
}


void CheckIfFFDataComplete()
{
	int i;
	BlocksBitArray[BlockID]=1;   // mark my block as done as data is now in TCM
					  
	// check if all data is in place
	if (Ph1FFCtrlReceived)   // if we have the control then it's worth checking to see if all blocks are done 
	{
		AllBlocksFullyPopulated=1;   // check to see if all expected blocks have been populated
		for(i=0;i<FFBlockCount;i++)  if (BlocksBitArray[i]==0) AllBlocksFullyPopulated=0;
      				
		if (AllBlocksFullyPopulated)
		{
			printf("\nWe have a full image in location: %x.\n We want to populate this at address: %x.\n  And execute from address: %x",itcmimage,Ph1BlockBaseAddress,FFExecuteAddress);
			// Izzy Whizzy Let's get busy!!! 

			if (0>lengthofimage>ITCMIMAGESIZE) 
			{
				Ph1FloodStarted = 0 //check for error in size (if too big for bootrom) , or zeroed. If wrong start again.
			}
			else
			{
				PopulateAndBoot(itcmimage, Ph1BlockBaseAddress, Ph1BlockWordCount*FFBlockCount, FFExecuteAddress); 
				// call routine (in safe memory space) to copy itcm image from dtcm into itcm & reboot me!
			}
		}
	}

}

// TODO - compile this to an invulnerable area in memory in order to execute from!
void PopulateAndBoot(unsigned int sourceofimage, unsigned int destinationforimage, unsigned int lengthofimage, unsigned int executeaddrforimage)
{

/*
	for(int i=0;i<lengthofimage;i++) destinationofimage[i]=sourceofimage[i];  //copy word by word from source to destination in tcms
	MSR		CPSR_c, #I_Bit|#F_Bit|Mode_SYS		;disabling the IRQ, FIQ and switching to systemmode  //TODO check this is OK
	B		executeaddrforimage					;reboot the machine (will take care of enabling irq, resetting stacks etc

OR - all in Assembly....

;R2 = size of image in words,  R0 = source location,  R1= target location

copier	
	MOVS	r3,r2, LSR #2 	; how many big 4 word chunks can we do, store result in R3
	BNE		chunkycopy		; if we can do > 1 then call chunkycopy
wordcopier
	ANDS    r2,#3			; check the last 2 bits for a remainder (0-3 words)
	BEQ		bootme			; if nothing left to copy then proceed to boot
	LDR		r3, [r0], #4	; Load just a single word from the source into R3 and postincrement R0
	STR		r3, [r1], #4	; Store that work into destination and postincrement R1 with word size
	SUBS	r2, r2, #1		; Reduce the wordcount by 1 as we've just copied a word
	BNE		wordcopy		; if needed copy more word(s)
bootme
	MSR		CPSR_c, #I_Bit|#F_Bit|Mode_SYS		;disabling the IRQ, FIQ and switching to systemmode  //TODO check this is OK
	B		executeaddrforimage					;reboot the machine (will take care of enabling irq, resetting stacks etc
	
chunkycopy
	LDM		r0!,{r4-r7}		; load 4 words from location of R0 & auto-index
	STM		r1!,{r4-r7}		; store 4 works into location R1 & auto-index
	SUBS	r3, r3, #1 		; reduce the number of chunks we still have to do by one
	BNE		chunkycopy		; as we still have chunks to copy do another 
	B		wordcopier		; no chunks still to do, head on back to copy any remaining words 

*/
} 


/*******************************************************************
 * 
 * 
 ******************************************************************/
startDMAOperation(unsigned int sysAddr, unsigned int tcmAddr, unsigned int desc)
{
	MEM(0x30000000, 0x4)=sysAddr;
	MEM(0x30000000, 0x8)=tcmAddr;
	MEM(0x30000000, 0xC)=desc;
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
unsigned int formDesc(int dataLength, int direction, int burstSize, int wordSize, int crcoption)
{
	return ( ((transferid & 0x3F)<< 26) | ((privilage & 0x1)<<25) | ((wordSize & 0x1)<<24) | ((burstSize & 0x7)<<21) | ((crcoption & 0x1)<<20) | ((direction & 0x1)<<19) | (dataLength & 0xFFFF) );
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void DMAWrite(unsigned int sysnocAddr, unsigned int dtcmAddr, int size)
{
	unsigned int desc;
	//setting the dmaInProcess flag high only after disabling the interrupts
	disableInterrupts();
	dmaInProcess=1;
	enableInterrupts();
	//dmaInProcess=1;       //clearing the flag to inform application that the dma is in use
	#ifdef VERBOSE
		printf("proc %d: starting DMA write sysnoc:0x%x, dtcm:0x%x, size:0x%x.\n", procID, sysnocAddr, dtcmAddr, size);	
	#endif
	if((sysnocAddr >= SDRAM_BASE) && (sysnocAddr<(SDRAM_BASE+SDRAM_SIZE)))  //allow double word only in case of SDRAM
		width=1;
	else
		width=0;
	#ifdef TLM_MODEL
		desc=formDesc(size, WRITE, burst, width, 0);  // 0 on end marks no CRC required
		startDMAOperation(sysnocAddr, dtcmAddr, desc);
	#else
		//in PL080 the size is the number of transfer, as it is set to transfer words, so convert these to number of words
		//and if not in multiples of 4 than add 1
		if(size % 4)
			size = (size/4)+1;
		else
			size=(size/4);
		startDMAWrite(dtcmAddr, sysnocAddr, size);
	#endif
	width=0;  //resetting the width to default again
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void DMARead(unsigned int sysnocAddr, unsigned int dtcmAddr, int size, int addcrc)
{
	unsigned int desc;
	disableInterrupts();
	dmaInProcess=1;     //setting the flag to inform application that the dma is in use
	enableInterrupts();      
	#ifdef VERBOSE
		printf("proc %d: starting DMA write sysnoc:0x%x, dtcm:0x%x, size:0x%x.\n", procID, sysnocAddr, dtcmAddr, size);	
	#endif
	if((sysnocAddr >= SDRAM_BASE) && (sysnocAddr<(SDRAM_BASE+SDRAM_SIZE)))  //allow double word only in case of SDRAM
		width=1;
	else
		width=0;
	#ifdef TLM_MODEL
		desc=formDesc(size, READ, burst, width, addcrc); 
		startDMAOperation(sysnocAddr, dtcmAddr, desc);
	#else
		//in PL080 the size is the number of transfer, as it is set to transfer words, so convert these to number of words
		//and if not in multiples of 4 than add 1
		if(size % 4)
			size = (size/4)+1;
		else
			size=(size/4);
		startDMARead(sysnocAddr, dtcmAddr, size);
	#endif
	width=0;  //resetting the width to default again
}

/*
void initMacFromSerialRom()
{
  unsigned int* sysram = (unsigned int *) SYS_RAM_VAR_SPACE;
  sourceMACAddressLS = sysram [32];
  sourceMACAddressHS = sysram [33];
}
*/

/*
void initIPAddressFromSerialRom()
{
  unsigned int* sysram = (unsigned int *) SYS_RAM_VAR_SPACE;
  sourceIPAddress = sysram [34];
  desinationIPAddress = sysram [35];
}
*/

//TODO FIX!!!!
void initMacFromSerialRom()
{
	sourceMACAddressLS=0xA400C193;  // CP readded as srom is now used to boot the machine
	sourceMACAddressHS=0x0000;   // Acorn OUI in use as per ST/CP agreement
}

void initIPAddressFromSerialRom()
{
	sourceIPAddress=0xC0A80001;	//192.168.0.1  // CP readded as srom is now used to boot the machine
	desinationIPAddress=0xC0A80002;	//192.168.0.4  // .4 = fletchermoss machine
}


int sendFrame(int length)
{
	//always use dma operation to send the frame to ethernet interface as the length of frame is always going to be more than 60
	unsigned int tcmAddress=(unsigned int)txFrameBase;
	int status = readMemory(ETH_BASE+ETH_GEN_STATUS);
	
	if(status & 0x1) //if the tx mii is still active i.e. the previous tx is still in operation, abort
		return false;

	while(dmaInProcess)	{;} //wait if an existing DMA is in progress

	txFramePending=1; //setting the flag for the application to indicate that the frame sending is in process
	writeMemory(ETH_BASE+ETH_TX_LENGTH, length);  //writing the length of frame into the tx length register
	DMAWrite(ETH_BASE+ETH_TX_FRAME_BUFFER /*sys noc address*/, tcmAddress, length);
	while(dmaInProcess) {;}  //wait for the dma to completed before asking the ethernet interface to start the frame send operation 
	writeMemory(ETH_BASE+ETH_TX_COMMAND, 1); //writing any value to tx command register will start the send frame operation
	return true;
}
/*******************************************************************
 * 
 * 
 *  SW read of the frame
 *		;When SW detects that an unread frame is in RX_FRAME_BUFFER, the frame can be read by
 *		;use of the following:
 *		;� Start location indicated by RX_FRAME_BUF_RD_PTR
 *		;� Length of the frame indicated by currently-visible RX_DESC_WORD
 *		;After SW has dealt with the received frame, it indicates that it can be over-written by writing a
 *		;rising-edge to the RX_COMMAND register, executing READ_HANDSHAKE. This causes
 *		;RX_HOST_FSM to:
 *		;(1) increment the RX_FRAME_BUF_RD_PTR by RX_FRAME_LEN
 *		;(2) increment the RX_DESC_RAM_RD_PTR
 *		;****important****
 *		;CRC is stored with the data in the rx_frame_buffer, however, its 4 bytes are not included in the frame_lenght
 *		;stored in the rx_desc_ram (bits 10-0) and left to the application to hendle with this
 ******************************************************************/	

int receiveFrame(unsigned int tcmAddress) 
{
    unsigned int framelength, framelength1, framelength2;
    unsigned int startAddr, startAddr1, startAddr2; //, wrappy;
    unsigned int desc, DoubleDMAreadRequired;

    desc = readMemory(ETH_BASE+ETH_RX_DSC_RD_PTR);   //Start location indicated by RX_FRAME_BUF_RD_PTR
    desc = (desc & 0x3F)*4;             //bit [5:0] contain offset of rx_desc_ram, which contains the start of rx frame to be read by processor
    				                    //to get bits 5-0, the value refers to the whole word in rx_desc_ram i.e. 32bit word, we need to multiply with 4 to get the start address of that word
                                        //multiply with 4 to get address on AHB
    desc = ETH_RX_DESC_RAM+desc;        //address in relation to rx_desc_ram
    
    rxFrameSize=readMemory(ETH_BASE+desc); //physical address of rx desc ram where we will find the address of frame in rx frame buffer
    rxFrameSize = (rxFrameSize & 0x7FF);   //bit 10-0 i.e. 11 bits contain the length of frame                                                    
    framelength = rxFrameSize + 4;        //adding 4 to get CRC as well, together with data to do crc check once again if required

    
    startAddr = readMemory(ETH_BASE+ETH_RX_BUF_RD_PTR);

    startAddr = (startAddr & 0xFFF);        //bit 11-0 contain the offset of rx frame buffer which contains the start of rx frame to be read by processor
                                        //to get bits 11-0, the value contains the position of byte in 3KByte rx_frame_buffer (word aligned).

	
    if ((startAddr+framelength)>0xC00)  // as the frame buffer is a circular 3k buffer, it's possible the frame might wrap around back to the start 
    {
        DoubleDMAreadRequired = 1;
        framelength1=0xC00-startAddr;
        framelength2=framelength-(0xC00-startAddr);
        startAddr1=startAddr+ETH_BASE+ETH_RX_FRAME_BUFFER;
        startAddr2=ETH_BASE+ETH_RX_FRAME_BUFFER;
    }
    else {
        DoubleDMAreadRequired = 0;
        startAddr1 = (startAddr + ETH_BASE + ETH_RX_FRAME_BUFFER); //the start address of the new frame, as there can be more than one frame received
        framelength1 = framelength;
    }
    

    while(dmaInProcess) {;}		//starting dma op to bring in frame - wait for any current DMA operation to complete before starting our read
    DMARead(startAddr1, tcmAddress, framelength1, crc);  //begin our read from global system position to copy to local processor tcm
    while(dmaInProcess) {;}		//wait until our DMA read (copy) completes

    if (DoubleDMAreadRequired)	// if frame has wrapped around then do a 2nd DMA read to complete the frame copy 
    {
        DMARead(startAddr2, tcmAddress+framelength1, framelength2, crc);  // copy 2nd portion to local tcm
        while(dmaInProcess) {;}	//wait while our DMA completes
    }
    writeMemory(ETH_BASE+ETH_RX_COMMAND, 1);   //writing some value to rx command register indicates frame receive operation complete
    
    return true;
}



/*******************************************************************
 * 
 * spinnakerdatalength=the data in the payload field i.e. after 
 * spinnakerDataAt
 ******************************************************************/	

void initFrameHeaders(unsigned int spinnakerdatalength)
{
    unsigned int sum=0, tempy1=0, tempy2=0, count=0;
   
    txFrameBase[0]=(destinationMACAddressHS & 0xFFFF)<<16 | (destinationMACAddressLS & 0xFFFF0000)>>16; //4 MSBytes of  Dest MAC address
    txFrameBase[1]=(destinationMACAddressLS & 0xFFFF)<<16 | (sourceMACAddressHS & 0xFFFF);              //2 LSBytes of D MAC, first 2 MSB of S MAC
    txFrameBase[2]=sourceMACAddressLS;																	//4 LSBytes of S MAC address
    txFrameBase[3]=(2048<<16) | ( 0x4500);       				//ethernet type =2048(0x800), IP version=4+ip header len=5+ip type of service=0
    txFrameBase[4]=( ((20+8+18+spinnakerdatalength) << 16) | EthPacketID++ );  //Dtgm Size = 20(IP)+8(UDP)+18(SpiNN)+SpiNPayload(Extras),  Pkt ID                                           
    txFrameBase[5]=0x4000FF11;                //Frag flags=4 + fragment offset = 0 + IP TTL = 255 (0xFF) + IP Protocol = 17 (0x11) UDP
    txFrameBase[6]=(sourceIPAddress & 0xFFFF0000)>>16;		//IP Header Checksum (init 0 - see later) + first 2 octets of Source IP address   
    txFrameBase[7]=(sourceIPAddress  & 0xFFFF)<<16 | (desinationIPAddress & 0xFFFF0000)>>16;	//last 2 octets S IP, 1st 2 of Dest IP
    txFrameBase[8]=(desinationIPAddress  & 0xFFFF)<<16 | 54321;					//last 2 octets of Dest IP, Source UDP port = 54321 (0xD431)                                       
    txFrameBase[9]=( (unsigned int)54321 << 16)  | (8+18+spinnakerdatalength);	//Dest UDP port = 54321, UDP Length (UDP Headers+SpiNN+Extras)                                           
    txFrameBase[10]=SPINNPROTVERSION;	//Checksum of UDP Pseudo Header = 0 (as optional) + SpiNNaker Protocol Version
 
    for(count=3; count<8; count++) {	// begin calculation of the 20 byte IP header checksum
            tempy1= (txFrameBase[count] & 0x0000FFFF);			// take first 16bits from the header word
            tempy2= ((txFrameBase[count+1] & 0xFFFF0000)>>16);  // take second 16bits from the header word
            sum+=tempy1+tempy2;   // add them together and increment the tally.  TODO - can do this all in 1 line!
    }    
    
    while(sum&0xFFFF0000) sum = (sum & 0xFFFF)+(sum >> 16);  //sums the two 16bit shorts from each word until no 16bit carry remains    
    count = 0xFFFF-sum;  // invert the result - which is the 1s complement of the IP header - the IP header checksum.

    txFrameBase[6]=(count << 16) | txFrameBase[6];  //insert the ip checksum in the correct position in the header
 }



/*******************************************************************
 * 
 * 
 ******************************************************************/
int formatAndSendFrame(unsigned int instruction, unsigned int option1, unsigned int option2, unsigned int option3)
{
	int length; 
	length = option2 + 60;		//option2 carries spinnaker optional data payload length, add to this the ethernet/ip/udp/spin header overheads

	initFrameHeaders(option2);	//spinnaker data payload length

	txFrameBase[spinnakerInstructionAt/4]=instruction;		//instruction to go at instruction field
	txFrameBase[instructionOption1At/4]=option1;			//option1 to go at option1
	txFrameBase[instructionOption2At/4]=option2;			//to go at option2
	txFrameBase[instructionOption3At/4]=option3;			//to go at option3

	sendFrame(length);										//whack the frame out on the Ethernet
	return length;   										//length of frame transmitted
}

void sendHelloFrame()  
{
	int length=formatAndSendFrame(HELLO, (VERSION << 24) | (SUBVERSION << 16) | opMode, 0, 0x43505453);
	//instruction, option1=version codes of ROM/library, option2=optional payload size, 3 = procID and core (n/a)
	//using this as a notification to the local LAN that we are alive in main() - this may get binned
}

/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendAckFrame()
{
	int length=formatAndSendFrame(ACK, 0, 0, 0);
	// probably not going to use for the new BOOTROM strategy
}


int lockProc(int procNo)
{
	//A unique single-bit value �X� appears in each register r64 to r95. Reading each register returns
	//0x00000000 or 0x80000000 depending on whether its respective bit was clear or set prior to the
	//read, and as a side-effect the bit is set by the read.
	//Together with r96 to r127, these registers provide support for mutual exclusion primitives for interprocessor
	//communication (IPC) and shared data structures, compensating for the lack of support for
	//locked ARM �swap� instructions into the System RAM. Magic.
	unsigned int val=0;
	unsigned int addr=SYSCTLR_Test_Set+(procNo << 2);
	val= readMemory(SYSCTLR_BASE_U+addr);
	if(val == 0x80000000)  //if the lock is already set
		return 0;
	else                     //lock is set to this proc
		return 1;
}
int unlockProc(int procNo)
{
	//The same unique single-bit value �X� appears in each register r96 to r127 as appears in r64 to r95
	//respectively. Reading each register returns 0x00000000 or 0x80000000 depending on whether its
	//respective bit was clear or set prior to the read, and as a side-effect the bit is cleared by the read.
	unsigned int val=0;
	unsigned int addr=SYSCTLR_Test_Clr+(procNo << 2);
	val= readMemory(SYSCTLR_BASE_U+addr);
	if(val == 0x80000000)  //if the lock was set, and cleared now
		return 1;
	else                     //lock was already cleared
		return 0;
}
	
/* ***************************************************************************************
 * shared memory message passing routine
 * 
 *///////////////////////////////////////////////////////////////////////////////////////
int passMessage(int msg, int proc, unsigned int dataStartAddress, int dataSize)
{
	int count=0;
	unsigned int addr=SYSTEM_RAM_BASE+(SYS_RAM_MSG_BUF_SIZE * proc);   //address of the proc's message box
	//acquire lock
	if(!lockProc(proc))  //if could not get a lock, quit
		return 0;
	//if data size is more than 16 bytes, do dma operation or just copy it into the system ram
	//bit 15-0 = message, bits 31-16 = size of transfer in bytes i.e. data size
	msg=(msg & 0xFFFF) | ((dataSize & 0xFFFF) << 16);
	//write the message in the first word of the proc N buffer
	writeMemory(addr, msg);
	addr=addr+4;
	if(dataSize > 16)   //if(usedma) //do DMA write
	{
		while(dmaInProcess) {;} //wait for any existing dma to complete
		DMAWrite(addr, dataStartAddress, dataSize); // send the DMA request
		while(dmaInProcess) {;} //wait till dma is done
	}
	else
	{
		for(count=0; count<(dataSize/4); count++)
			MEM(addr, (count*4))=readMemory(dataStartAddress+(count*4));
	}
	//setting proc N interrupt
	sendProcInt(proc);
	return 1;
}

void receiveMessage(unsigned int dtcmAddress)
{
	unsigned int addr=SYSTEM_RAM_BASE+(SYS_RAM_MSG_BUF_SIZE * procID);
	unsigned int msg, count;
	int dataSize;
	
	msg=readMemory(addr);
	dataSize=(msg >> 16) & 0xFFFF;
	msg=msg & 0xFFFF;
	//#define		SM_SEND_DATA					0x81
	//#define		SM_RECEIVE_DATA					0x82
	//#define		SM_LOAD_APPLICATION				0x83 //the source address in first word after the instruction, the destination address in the second word after the instruction, the address to move PC in the third wrod after the instruction

	
	//interpret the message and perform accordingly
	switch(msg)
	{
		case SM_RECEIVE_DATA:
		{
			//read data of dataSize bytes from the buffer
			addr+=4;
			if(dataSize > 16)
			{
				while (dmaInProcess)
					;  //wait till the dma is in process
				DMARead(addr, dtcmAddress, dataSize, crc);
				while (dmaInProcess)
					;  //wait till the dma is in process
			}
			else
			{
				for(count=0; count<dataSize/4; count++)
				{
					MEM(dtcmAddress, (count*4))=readMemory(addr+(count*4));
				}
			}
			//clear the lock
			unlockProc(procID);
			break;
		}
		case SM_SEND_DATA:
		{
			; //do nothing
			//clear the lock
			unlockProc(procID);
			break;
		}
		case SM_LOAD_APPLICATION:
		{
			//clear the lock
			unlockProc(procID);  //unlock the proc because the control is not going to come here and the lock will remain active
			//the words after instrcution in the mailbox are: source address, destination address, size, and the address to load into PC
			loadApplicationAndStart(readMemory(addr+4), readMemory(addr+8), readMemory(addr+12), readMemory(addr+16));
		}
		case SM_LOAD_APPLICATION_DATA:
		{
			; //do nothing
			//clear the lock
			loadApplicationData(readMemory(addr+4), readMemory(addr+8), readMemory(addr+12)); //address data from, address data to, size of data
			unlockProc(procID);
			break;
		}
		default:
			break;
	}
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void storeBlockToMemory(unsigned int* buf, unsigned int stAddr, int Size)
{
	//if the stAddr is of ITCM, do not request DMA operation and transfer the data block by
	//multiple registers load and store operation
	//otherwise request a DMA operation
	int counter;
	#ifdef VERBOSE
		printf("proc %d: in storeBlockToMemory\n", procID);
	#endif
	//keep a check not to overwrite the critical code region such as the vector table and ISRs etc
	if((stAddr > ITCM_BASE) && (stAddr < ITCM_LIMIT) || (stAddr > DTCM_BASE) && (stAddr < DTCM_LIMIT)) //it is in the local memory so DMA can not be used
	{
		for(counter=0; counter > (Size/4); counter=counter+4)
		{
			writeMemory(stAddr, buf[counter]);
			stAddr +=4;
		}
	}
	else
	{
		while(dmaInProcess)
		{
			#ifdef VERBOSE
				printf("proc %d: dma busy \n", procID);
			#endif
			;  //wait till the flag is not set
		}
		DMAWrite(stAddr, (unsigned int)buf, Size);
		while(dmaInProcess)
		{
			#ifdef VERBOSE
				printf("proc %d: dma busy \n", procID);
			#endif
			;  //wait till the flag is not set
		}
	}
}

void loadApplication(unsigned int applicationSize, unsigned int applicationAddress, unsigned int mainAddress)
{
	unsigned int val;
	int count;
	for(count=0; count<applicationSize/4; count=count+4)
	{
		val=readMemory(applicationAddress);
		writeMemory((unsigned int)count, val);
	}
	//move the PC to mainAddress
}

	
void wait(int time)
{
	int endTime;
	endTime=currentTime+time;
	while(currentTime < endTime);  //waiting here for time ms
	
}

/*******************************************************************
 * 
 * 
 ******************************************************************/	
void handleArpRequest()
{
	txFrameBase[0]=(rxFrameBase[1] & 0xFFFF)<<16 | (rxFrameBase[2] & 0xFFFF0000) >> 16;
	txFrameBase[1]=(rxFrameBase[2] & 0xFFFF)<<16 | sourceMACAddressHS & 0xFFFF;
	txFrameBase[2]=sourceMACAddressLS;	// swap Src to Dest MAC, and add local MAC for Src
	txFrameBase[3]=rxFrameBase[3];		// ethertype, & ARP h/w type stay the same
	txFrameBase[4]=rxFrameBase[4];		// ARP prot type, and hw/prot sizes also stay the same
	txFrameBase[5]=(0x2<<16)|sourceMACAddressHS & 0xFFFF; // opcode reply=2, followed by source MAC
	txFrameBase[6]=sourceMACAddressLS;	// addresses once again
	txFrameBase[7]=sourceIPAddress;
	txFrameBase[8]=txFrameBase[0];		// MSWord of DMAC address
	txFrameBase[9]=(txFrameBase[1] & 0xFFFF0000) | (rxFrameBase[7]>>16) & 0xFFFF;  // LS2Bytes of Dest MAC + 1st 2 bytes dest IP
	txFrameBase[10]=(rxFrameBase[7]<<16) & 0xFFFF0000; // final 2 bytes of Dest IP address

	sendFrame(60);  // send ARP reply on it's way, 42 actual bytes, but 60 min frame size, rest is padding trailer (content irrelevant)!   		   
}

/*******************************************************************
 * 
 * 
 ******************************************************************/	
void handlePingRequest()
{	
	unsigned int count, size, sizeinwords, lengthy, i, sum=0;
	
	size = (rxFrameBase[4] >> 16)+14;		//ethernet frame size in bytes
	sizeinwords = (size-1)/4;				//number of full words
		   
	for(i=3;i<=sizeinwords;i++) txFrameBase[i]=rxFrameBase[i];  // copy received frame across to use as a template tx reply

	// zero any appending bytes so not to skew the icmp checksum   		   
	if ((size&0x3)==1) txFrameBase[sizeinwords]&=0xFF000000;	//just 1 byte of real data in last word, zero 3 of them!
	if ((size&0x3)==2) txFrameBase[sizeinwords]&=0xFFFF0000;	//2 bytes of real data in last word, zero 2 of them
	if ((size&0x3)==3) txFrameBase[sizeinwords]&=0xFFFFFF00;	//3 bytes of real data in last word, zero last byte

	txFrameBase[0]=(rxFrameBase[1] & 0xFFFF)<<16 | (rxFrameBase[2] & 0xFFFF0000) >> 16;
	txFrameBase[1]=(rxFrameBase[2] & 0xFFFF)<<16 | (rxFrameBase[0] & 0xFFFF0000) >> 16;
	txFrameBase[2]=(rxFrameBase[0] & 0xFFFF)<<16 | (rxFrameBase[1] & 0xFFFF0000) >> 16; // swap Dest and Src MACs
	txFrameBase[6]=(rxFrameBase[7] & 0xFFFF);                                  // clear IP checksum and move...
	txFrameBase[7]=(rxFrameBase[8] & 0xFFFF0000) | (rxFrameBase[6] & 0xFFFF);  // ...dest and source IPs...
	txFrameBase[8]=(rxFrameBase[7] & 0xFFFF0000);                              //...and set command to echo reply
	txFrameBase[9]=(rxFrameBase[9] & 0xFFFF);       // type = 0, code = 0, and clearout ICMP checksum field
		   
	//perform the IP header checksum calculation
	for(i=3; i<8; i++) sum+= ((txFrameBase[i] & 0x0000FFFF) + ((txFrameBase[i+1] & 0xFFFF0000)>>16));
		// this for loop calculates a sum of the 20bytes as 16bit chunks for the IP header checksum
	while(sum&0xFFFF0000) sum = (sum & 0xFFFF)+(sum >> 16); //rolls over carry, and adds as required
	count = 0xFFFF-sum; //1s complement is the the result
	txFrameBase[6]=(count << 16) | txFrameBase[6];  //adds the ip checksum to the correct field in the IP header

	//perform the ICMP checksum calculation
	sum= ((txFrameBase[8]&0xFF00) + txFrameBase[8]&0xFF); // 1st 2 bytes of checksum for ICMP chk
	for(i=9; i<=sizeinwords; i++) sum+= ( ((txFrameBase[i]&0xFFFF000000)>>16) + (txFrameBase[i]&0xFFFF));
		// this for loop chunks the ICMP data into 16bit segments and adds for the ICMP checksum 
	while(sum&0xFFFF0000) sum = (sum & 0xFFFF)+(sum >> 16); //rolls over carry and adds as required
	count = 0xFFFF-sum; //1s complements the result
	txFrameBase[9]=(count << 16) | txFrameBase[9]; // adds ICMP checksum to correct field in ICMP header

	sendFrame(size);  // send it on it's way
}
	
#endif /*SPINNAKERLIB_H_*/
