#ifndef SPINNAKERLIB_H_
#define SPINNAKERLIB_H_

#include "globals.h"

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
extern void dputc(char ch); // CP testing printf
/*******************************************************************
 * 							c functions
 * 
 ******************************************************************/




//-------------------------------------------------------------
// CP added in order to support printf for debugging 18th Jan 2010

#pragma import(__use_no_semihosting_swi)     // disable semihosting


/*
** These must be defined to avoid linking in stdio.o from the
** C Library
*
*/

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


//-------------------------------------------------------------




 
 /*******************************************************************
 * to read or write to a memory location
 * 
 ******************************************************************/
#define MEM(Base, offset)	(*(volatile unsigned int *)(Base + offset))
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
 * initializes the chip level variables
 * 
 ******************************************************************/
//testBootRomAddress()
//{
//	unsigned int con;
//	con=MEM(SYSCTLR_BASE_U, SYSCTLR_MISC_CONTROL);
//	if(con & 0x1)
//		addressMapped=true;
//	else
//		addressMapped=false;
//	
//}
/*******************************************************************
 * initializes the chip level variables
 * 
 ******************************************************************/
static void initChipVariables()
{
	//if(addressMapped)  //boot rom not remapped to ram
	//	globalVarSpace=SDRAM_BASE+VAR_SPACE;
	//else
	//	globalVarSpace=SYSTEM_RAM_BASE+VAR_SPACE;
	MEM(globalVarSpace, SR_CHIP_ADDRESS)=0;
	MEM(globalVarSpace, SR_NCHIPS_LINK_STATE)=0;    //bits 5-0 nn chips state, bits 21-16 nn links state
	MEM(globalVarSpace, SR_CONF_DONE)=0;
	chipID=0;
	nChipsLinksState=0;                //local copy of neighbouring links and chips state bits 5-0 nn chips state, bits 21-16 nn links state
	configDone=0;
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void setupChipID()
{
	//save the chip address to the systemram var space and 
	//cofigure the communication controller
	#ifdef VERBOSE
		printf("proc %d: setupChipID\n", procID);
	#endif
	MEM(globalVarSpace, SR_CHIP_ADDRESS)=chipAddress;
	chipID=chipAddress;
	#ifdef VERBOSE
		printf("proc %d: ChipID: %d\n", procID, chipID);
	#endif
	SetupSourceID((int)chipID);     //init source id in the comm controller
}
void setupRouterP2PTable()
{
	unsigned int offset, value, count;
	         //256
	#ifdef  TLM_MODEL
		//p2pRamSize=256;        //1KB = 256 words
	#endif
			
	#ifdef VERBOSE
		printf("proc %d: in setupRouterP2PTable\n", procID);
	#endif
	//settting up the entries for the valid routes
	/*while(i<noOfP2PEntries)
	{
		offset=i*4;
		value=(p2ptable[i] & 0xFFFFFF) ;		
		setupP2PTable(offset, value);
		i++;
	}
	//setting up the unused enteries to 7 i.e. drop to the
	//monitor proc
	i=noOfP2PEntries;*/
	count=0;
	  
	  	while(count<P2PRAMSIZE)
	{
		offset=count*4;
		value=0xFFFFFF;     //setting value 7 in all the entries to drop it to the mon proc
		setupP2PTable(offset, value);
	  	count++;
	}
}
/*******************************************************************
 * 
 * 
 ******************************************************************/	
void setupRouterMCTable()
{
	int count;
	#ifdef VERBOSE
		printf("proc %d: in setupRouterMCTable\n", procID);
	#endif
	count=0;
	while( count < NO_OF_MC_TABLE_ENTRIES)
	{
		setupMCTable(count*4, 0, 0xFFFFFFFF, 0);
		count++;
	}
	/*i=0;
	while( i< noOfMCEntries)
	{
		setupMCTable(i*4, mctable[i][0], mctable[i][1], mctable[i][2]);  //location, key, mask, mcroute
		i++;
	}*/
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
int testCommNoC()
{
	#ifdef VERBOSE
		printf("proc %d: testing comm noc\n", procID);
	#endif
	//send nn packet to the monitor processor
	//sendNNPacket(7 /*to itself*/, NORMAL, unsigned int rkey, unsigned int pload)
	return 1;
}
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
	#ifdef VERBOSE
		printf("proc %d: in setRouterTimePhase\n", procID);
	#endif
	tphase=readMemory(RTR_BASE+RTR_R0_CONTROL);
	tphase =tphase & 0xFFFFFF3F;    //clearing bit 7-6 containing time phase
	t = t & 0x3;  //to make sure it is only two bit wide
	tphase = tphase | (t  << 6);
	writeMemory(RTR_BASE+RTR_R0_CONTROL, tphase);
}

/*******************************************************************
 * sends NN packet with payload
 * rt=route to send (3bits), npt=NN packet type (2bits 0, 1, 2)
 * rkey=routing key to be sent, pload=payload to be sent
 ******************************************************************/
void sendNNPacket(unsigned int rt, unsigned int npt, unsigned int rkey, unsigned int pload)
{
	unsigned int control = 	  (2 << 6)   //NN packet  0b10 i.e. 2   bit 7-6
							| ((npt & 0x1) << 5)   //NN Packet Type 0 normal, 1 direct bit 5
							| ((rt & 0x7) << 2)		//NN route type bit 4-2
							| (1 << 1)		//data bit bit 1
							;
	control = (control << 16)					//control bit 23-16
		  | (7 << 24)						//default route 7 to represent packet is from internal processor
		  | (1 << 29)						//to ensure it does not overwrite tx overflow flag
		  | (1 << 30)						//to ensure it does not overwrite txfull flag
		  | (chipAddress);
	#ifdef VERBOSE
		printf("proc %d: s P: NN con:0x%x, rkey:0x%x, pload:0x%x\n", procID, control, rkey, pload);
	#endif
	writePacketWPL(control, rkey, pload);
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendNNPacketWOPL(int rt, int npt, unsigned int rkey)
{
	unsigned int control =    (2 << 6)   //NN packet  0b10 i.e. 2   bit 7-6
							| ((npt & 0x1) << 5)   //NN Packet Type 0 normal, 1 direct bit 5
							| ((rt & 0x7) << 2)		//NN route type bit 4-2
							| (0 << 1)		//data bit bit 1
							;				//bit 0 empty to be set by comm ctl, parity bit 
	control = (control << 16)					//control bit 23-16
		  | (7 << 24)						//default route 7 to represent packet is from internal processor
		  | (1 << 29)						//to ensure it does not overwrite tx overflow flag
		  | (1 << 30)						//to ensure it does not overwrite txfull flag
		  | (chipAddress);
	#ifdef VERBOSE
		printf("proc %d: s P: NN con:0x%x, rkey:0x%x\n", procID, control, rkey);;
	#endif
	writePacketWOPL(control, rkey);
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendP2PPacket(int seqcode, unsigned int destID, unsigned int pload)
{
	unsigned int rkey;
	unsigned int control =    (1 << 6)   			//packet type = P2P  0b01 i.e. 1   bit 7-6
							| (seqcode << 4)   		//seqcode for p2p packet bit 5-4
							| (0 << 2)		//timestamp bit 3-2
							| (1 << 1)				//data bit bit 1
							;						//bit 0 empty to be set by comm ctl, parity bit 
	control = (control << 16)					//control bit 23-16
		  | (7 << 24)						//default route 7 to represent packet is from internal processor
		  | (1 << 29)						//to ensure it does not overwrite tx overflow flag
		  | (1 << 30)						//to ensure it does not overwrite txfull flag
		  | (chipAddress);
	#ifdef VERBOSE
		printf("proc %d: s P: P2P con:0x%x, rkey:0x%x, pload:0x%x\n", procID, control, rkey, pload);
	#endif
	rkey = ( ( (seqcode & 0x3) << 16)  | (destID & 0xFFFF) );//seq and destination address to go to r3 register of comm ctlr.
	writePacketWPL(control, rkey, pload);
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendP2PPacketWOPL(int seqcode, unsigned int rkey)
{
	unsigned int control = 	  (1 << 6)   			//packet type = P2P  0b01 i.e. 1   bit 7-6
							| (seqcode << 4)   		//seqcode for p2p packet bit 5-4
							| (0 << 2)		//timestamp bit 3-2
							| (0 << 1)				//data bit bit 1
							;						//bit 0 empty to be set by comm ctl, parity bit 
	control = (control << 16)					//control bit 23-16
		  | (7 << 24)						//default route 7 to represent packet is from internal processor
		  | (1 << 29)						//to ensure it does not overwrite tx overflow flag
		  | (1 << 30)						//to ensure it does not overwrite txfull flag
		  | (chipAddress);
		 
	#ifdef VERBOSE
		printf("proc %d: s P: P2P con:0x%x, rkey:0x%x\n", procID, control, rkey);
	#endif
	writePacketWOPL(control, rkey);
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendMCPacket(unsigned int rkey, unsigned int pload)
{
	unsigned int control=		  (0 << 6)   			//packet type = P2P  0b01 i.e. 1   bit 7-6
								| (0 << 4)   		    //emergency route bit 5-4
								| (0 << 2)		//timestamp bit 3-2
								| (1 << 1)				//data bit bit 1
								;						//bit 0 empty to be set by comm ctl, parity bit 
	control = (control << 16)					//control bit 23-16
		  | (7 << 24)						//default route 7 to represent packet is from internal processor
		  | (1 << 29)						//to ensure it does not overwrite tx overflow flag
		  | (1 << 30)
		  | (chipAddress);
	#ifdef VERBOSE
		printf("proc %d: s P: MC con:0x%x, rkey:0x%x, pload:0x%x\n", procID, control, rkey, pload);
	#endif
	writePacketWPL(control, rkey, pload);
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendMCPacketWOPL(unsigned int rkey)
{
	unsigned int control=   0;					//bit 0 empty to be set by comm ctl, parity bit 
	control = (control << 16)					//control bit 23-16
		  | (7 << 24)						//default route 7 to represent packet is from internal processor
		  | (1 << 29)						//to ensure it does not overwrite tx overflow flag
		  | (1 << 30)						//to ensure it does not overwrite txfull flag
		  | (chipAddress);
		 
	#ifdef VERBOSE
		printf("proc %d: s P: MC con:0x%x, rkey:0x%x\n", procID, control, rkey);
	#endif
	writePacketWOPL(control, rkey);
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
unsigned int formDesc(int dataLength, int direction, int burstSize, int wordSize)
{
	return ( ((transferid & 0x3F)<< 26) | ((privilage & 0x1)<<25) | ((wordSize & 0x1)<<24) | ((burstSize & 0x7)<<21) | ((crc & 0x1)<<20) | ((direction & 0x1)<<19) | (dataLength & 0xFFFF) );
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
		desc=formDesc(size, WRITE, burst, width);
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
void DMARead(unsigned int sysnocAddr, unsigned int dtcmAddr, int size)
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
		desc=formDesc(size, READ, burst, width);
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
//dummy functions to support serial rom interaction
/*******************************************************************
 * 
 * 
 ******************************************************************/

// !! ST - SYS_RAM_VAR_SPACE = 0xf5003c00

/*
void initMacFromSerialRom()
{
  unsigned int* sysram = (unsigned int *) SYS_RAM_VAR_SPACE;
  sourceMACAddressLS = sysram [32];
  sourceMACAddressHS = sysram [33];
}
*/

void initMacFromSerialRom()
{
	sourceMACAddressLS=0x48000001;  // CP readded as srom is now used to boot the machine
	sourceMACAddressHS=0x2CDE;   // 2CDE and not ACDE because of bug with MC receive on MAC
}


/*******************************************************************
 * 
 * 
 ******************************************************************/

// !! ST

/*
void initIPAddressFromSerialRom()
{
  unsigned int* sysram = (unsigned int *) SYS_RAM_VAR_SPACE;
  sourceIPAddress = sysram [34];
  desinationIPAddress = sysram [35];
}
*/

void initIPAddressFromSerialRom()
{
	sourceIPAddress=0xC0A80001;	//192.168.0.1  // CP readded as srom is now used to boot the machine
	desinationIPAddress=0xC0A80002;	//192.168.0.4
}

/*******************************************************************
 * 
 * 
 ******************************************************************/
/*
typedef unsigned short u16;
typedef unsigned long u32;

u16 ip_sum_calc(u16 len_ip_header, u16 buff[])
{
	u16 word16;
	u32 sum=0;
	u16 i;
	    
		// make 16 bit words out of every two adjacent 8 bit words in the packet
		// and add them up
		for (i=0;i<len_ip_header;i=i+2){
			word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
			sum = sum + (u32) word16;	
		}
		
		// take only 16 bits out of the 32 bit sum and add up the carries
		while (sum>>16)
		  sum = (sum & 0xFFFF)+(sum >> 16);
	
		// one's complement the result
		sum = ~sum;
		
	return ((u16) sum);
}
*/

int sendFrame(int length)
{
	//always use dma operation to send the frame to ethernet interface as the length of frame is going to be more than 60
	unsigned int tcmAddress=(unsigned int)txFrameBase;
	int status = readMemory(ETH_BASE+ETH_GEN_STATUS);
	
	#ifdef VERBOSE
		printf("proc %d: in sendFrame\n", procID);
	#endif
	if(status & 0x1) //if the tx mii is still active i.e. the previous tx is still in operation, abort
		return false;
	while(dmaInProcess)
	{
		;
		#ifdef VERBOSE
			printf("proc %d: in sendFrame, waiting for previous dma to complete\n", procID);
		#endif
	}
	txFramePending=1; //setting the flag for the application to indicate that the frame sending is in process
	writeMemory(ETH_BASE+ETH_TX_LENGTH, length);  //writing the length of frame into the tx length register
	DMAWrite(ETH_BASE+ETH_TX_FRAME_BUFFER /*sys noc address*/, tcmAddress, length);
	while(dmaInProcess)  //wait for the dma to completed before asking the ethernet interface to start the frame send operation 
	{
		;
		#ifdef VERBOSE
			printf("proc %d: in sendFrame, waiting for dma to complete\n", procID);
		#endif
	}
	writeMemory(ETH_BASE+ETH_TX_COMMAND, 1); //writing any value to tx command register will start the send frame operation
	return true;
}
/*******************************************************************
 * 
 * 
 *  SW read of the frame
 *		;When SW detects that an unread frame is in RX_FRAME_BUFFER, the frame can be read by
 *		;use of the following:
 *		;• Start location indicated by RX_FRAME_BUF_RD_PTR
 *		;• Length of the frame indicated by currently-visible RX_DESC_WORD
 *		;After SW has dealt with the received frame, it indicates that it can be over-written by writing a
 *		;rising-edge to the RX_COMMAND register, executing READ_HANDSHAKE. This causes
 *		;RX_HOST_FSM to:
 *		;(1) increment the RX_FRAME_BUF_RD_PTR by RX_FRAME_LEN
 *		;(2) increment the RX_DESC_RAM_RD_PTR
 *		;****important****
 *		;CRC is stored with the data in the rx_frame_buffer, however, its 4 bytes are not included in the frame_lenght
 *		;stored in the rx_desc_ram (bits 10-0) and left to the application to hendle with this
 ******************************************************************/	
//code from cameraon tested on rtl model
int receiveFrame(unsigned int tcmAddress) 
{
    unsigned int framelength, framelength1, framelength2;
    unsigned int startAddr, startAddr1, startAddr2; //, wrappy;
    unsigned int desc, DoubleDMAreadRequired;
    #ifdef VERBOSE
            printf("proc %d: Inside receiveFrame.\n", procID);
    #endif
    desc = readMemory(ETH_BASE+ETH_RX_DSC_RD_PTR);   //Start location indicated by RX_FRAME_BUF_RD_PTR
    #ifdef VERBOSE
            printf("proc %d: raw Pointer to RX Frame Description: 0x%x\n", procID, desc);
    #endif
    desc = (desc & 0x3F);                  //bit 5-0 contain the offset of rx_desc_ram which contains the start of rx frame to be read by processor
    desc = desc * 4;                      //to get bits 5-0, the value refers to the whole word in rx_desc_ram i.e. 32bit word, we need to multiply with 4 to get the start address of that word
                                        //multiply with 4 to get address on AHB
    desc = ETH_RX_DESC_RAM+desc;        //address in relation to rx_desc_ram
    #ifdef VERBOSE
            printf("proc %d: Memory Loc of RX Frame Description after manipulation: 0x%x\n", procID, ETH_BASE+desc);
    #endif
    
    rxFrameSize=readMemory(ETH_BASE+desc);        //physical address of rx desc ram where we will find the address of frame in rx frame buffer

    #ifdef VERBOSE
            printf("proc %d: RX_DESC_RAM (bits13-11, matching BC(2000),MC(1000),UC(800) hits): 0x%x\n", procID, framelength&0x3800);
    #endif
    
    rxFrameSize = (rxFrameSize & 0x7FF);  //bit 10-0 i.e. 11 bits contain the length of frame                                           
             
    //rxBufSize = framelength;            //storing the length of frame in a global variable for the use of application
    framelength = rxFrameSize + 4;        //adding 4 to get CRC as well, together with data to do crc check once again if required

    #ifdef VERBOSE
            printf("proc %d: Length recovered from RX Frame Desc Memory location+4 byte CRC: 0x%x\n", procID, framelength);
    #endif
    
    startAddr = readMemory(ETH_BASE+ETH_RX_BUF_RD_PTR);
    #ifdef VERBOSE
            printf("proc %d: ***** raw Pointer to Buffer Memory Loc: 0x%x\n", procID, startAddr);
    #endif
//    startAddr = (startAddr & 0x3FF);        //bit 9-0 contain the offset of rx frame buffer which contains the start of rx frame to be read by processor
//                                        //to get bits 9-0, the value contains the position of 32 bit word in rx_frame_buffer
    //wrappy = (startAddr & 0x1000);        // if bit12 is set then this frame wraps around from the end to the start of the buffer
    startAddr = (startAddr & 0xFFF);        //bit 11-0 contain the offset of rx frame buffer which contains the start of rx frame to be read by processor
                                        //to get bits 11-0, the value contains the position of byte in 3KByte rx_frame_buffer (word aligned).
    #ifdef VERBOSE
            printf("proc %d: * Wrap around - bit12 in RX_FRAME_BUF_RD_PTR is set.\n", procID);
    #endif


    if ((startAddr+framelength)>0xC00) 
    {
        DoubleDMAreadRequired = 1;
        //printf("proc %d: *** SPLIT packet, Start1:0x%x Length1:0x%x ... Start2:0x%x Length2:0x%x.\n", procID, startAddr+ETH_BASE+ETH_RX_FRAME_BUFFER, 0xC00-startAddr, ETH_BASE+ETH_RX_FRAME_BUFFER, framelength-(0xC00-startAddr));
        framelength1=0xC00-startAddr;
        framelength2=framelength-(0xC00-startAddr);
        startAddr1=startAddr+ETH_BASE+ETH_RX_FRAME_BUFFER;
        startAddr2=ETH_BASE+ETH_RX_FRAME_BUFFER;
        #ifdef VERBOSE
        	printf("proc %d: *** SPLIT packet, Start1:0x%x Length1:0x%x ... Start2:0x%x Length2:0x%x.\n", procID, startAddr1, framelength1, startAddr2, framelength2);
    	#endif
    }
    else {
        DoubleDMAreadRequired = 0;
        startAddr1 = (startAddr + ETH_BASE + ETH_RX_FRAME_BUFFER); //the start address of the new frame, as there can be more than one frame received
        framelength1 = framelength;
    }
    
    
    //startAddr = (startAddr * 4);        //multiply with 4 to get the AHB address - wrong commented - byte based not word based.
    //startAddr = (startAddr + ETH_BASE + ETH_RX_FRAME_BUFFER); //the start address of the new frame, as there can be more than one frames received
    
    #ifdef VERBOSE
            printf("proc %d: ***** Memory Loc of Start of RX Frame Buffer after manipulation: 0x%x\n", procID, startAddr1);
    #endif
    
    //starting dma operation to bring in the frame
    while(dmaInProcess)
    {
        ;
//        #ifdef VERBOSE
//            printf("proc %d: in receiveFrame, waiting for previous dma to complete\n", procID);
//        #endif
    }
    //dmaInProcess=1; //setting the flag to tell the application dma is busy
    DMARead(startAddr1 /*sys noc address*/, tcmAddress, framelength1);
    while(dmaInProcess)
    {
        ;
//        #ifdef VERBOSE 
//            printf("proc %d: in receiveFrame, waiting for our dma read to complete\n", procID);
//        #endif
    }


    if (DoubleDMAreadRequired) 
    {
		//dmaInProcess=1; //setting the flag to tell the application dma is busy
        DMARead(startAddr2 /*sys noc address*/, tcmAddress+framelength1, framelength2);
        while(dmaInProcess)
        {
            ;
//            #ifdef VERBOSE
//                printf("proc %d: in receiveFrame, waiting for our dma read to complete\n", procID);
//            #endif
        }
    }
    writeMemory(ETH_BASE+ETH_RX_COMMAND, 1);   //writing some value to rx command register to indicate that the frame receive operation is over
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
   
    txFrameBase[0]=(destinationMACAddressHS & 0xFFFF)<<16 | (destinationMACAddressLS & 0xFFFF0000)>>16;                                                       //1st 4 bytes of  destination MAC address
    txFrameBase[1]=(destinationMACAddressLS & 0xFFFF)<<16 | (sourceMACAddressHS & 0xFFFF);                ////last 2 bytes of  destination MAC address, and 1st 2 of source MAC
    txFrameBase[2]=sourceMACAddressLS;                        //0xACDE4800;    sourceMACAddressHS=0xACDE;    //last 4 bytes of  destination MAC address
    txFrameBase[3]=(2048<<16) | ( 0x4500);                //ethernet type =2048 (15-0) = IP.  +IP version=4+ip header len=5(23-16)+ip type of service=0(31-24)
    txFrameBase[4]=( ((20+8+18+spinnakerdatalength) << 16) | EthPacketID++ );                //Total IP Datagram Length in Bytes = 20(IP)+8(UDP)+18(SpiNN)+SpiNNakerLength(Extras)    + Identification (for fragments) = 0                                           
    txFrameBase[5]=0x4000FF11;                //Frag flags=4 + fragment offset = 0    + IP TTL = 255    IP Protocol = 17 = udp
    txFrameBase[6]=(sourceIPAddress & 0xFFFF0000)>>16;                //IP Header Checksum + 1st 2 octets of Source IP address   
    txFrameBase[7]=(sourceIPAddress  & 0xFFFF)<<16 | (desinationIPAddress & 0xFFFF0000)>>16;                //last 2 octets of source ip + 1st 2 octets of Destination IP address
    txFrameBase[8]=(desinationIPAddress  & 0xFFFF)<<16 | 54321;                //last 2 octets of DestIP and Source UDP port = 54321                                       
    txFrameBase[9]=( (unsigned int)54321 << 16)  | (8+18+spinnakerdatalength);                //Destination UDP port = 54321 + UDP length = 8(UDP)+18(SpiNN)+SpiNNakerLength(Extras)                                           
    txFrameBase[10]=0x0;            //Checksum of UDP Pseudo Header = 0 (as optional)  + Length of SpiNNaker data payload (added later in formatAndSendFrame())
 
    for(count=3; count<8; count++) {
            tempy1= (txFrameBase[count] & 0x0000FFFF);
            tempy2= ((txFrameBase[count+1] & 0xFFFF0000)>>16);
            sum+=tempy1+tempy2;
            //printf("16-bit words from count=%d: 1st:0x%x, 2nd:0x%x. RunningSum:0x%x \n",count,tempy1,tempy2,sum);
    }
    // this for loop calculates a sum of the 20bytes as 16bit chunks for the IP header checksum

    sum = (sum & 0xFFFF)+(sum >> 16);
    count = 0xFFFF-sum;
    // this rolls over the carry and adds it, then 1's complements the result to get the checksum
    //printf("Carried over sum: %d, or 0x%x,  1's complemented: %d, or 0x%x.\n",sum, sum, count, count);

    txFrameBase[6]=(count << 16) | txFrameBase[6];  //adds the ip checksum to the correct field
 }

/* !! ST old
void initFrameHeaders(int spinnakerdatalength)
{
	unsigned short count;
	unsigned short temp[20];
	unsigned short *temp1=(unsigned short*)txFrameBase;
	txFrameBase[0]=destinationMACAddressLS;    													//0x48000000	destinationMACAddressLS=0x48000000;	//destination address
	txFrameBase[1]=(sourceMACAddressLS << 16) | (destinationMACAddressHS & 0xFFFF);				//0x0001ACDE;	sourceMACAddressLS=0x48000001;	destinationMACAddressHS=0xACDE//source+destination address
	txFrameBase[2]=(sourceMACAddressHS << 16) | (sourceMACAddressLS >> 16);						//0xACDE4800;	sourceMACAddressHS=0xACDE;	//source address
	txFrameBase[3]=(2048<<16) | ( 0x4500);				//ethernet/ip type =2048 (15-0)+ip version=4(19-16)+ip header=5(23-20)+ip type of service=0(31-24)
	txFrameBase[4]=((20+18+spinnakerdatalength) << 16);				//Total IP Datagram Length in Bytes = 20+(2(spinnakerdatalength field)+4(intr)+4(option1)+4(option2)+4(option3)=18)+SpiNNakerLength	+ Identification (for fragments) = 0											
	txFrameBase[5]=0x8000FF11;				//Frag flags=4 + fragment offset = 0	+ IP TTL = 255	IP Protocol = 17				
	txFrameBase[6]=(sourceIPAddress & 0xFFFF);				//IP Header Checksum (see notes) + Source IP address = 10.10.x.y	
	txFrameBase[7]=(sourceIPAddress  & 0xFFFF0000) | (desinationIPAddress & 0xFFFF);				//source ip + Destination IP address = 10.11.0.1								
	txFrameBase[8]=(desinationIPAddress  & 0xFFFF0000) | 54321;				//Source UDP port = 54321										
	txFrameBase[9]=(54321 << 16)  | (8+18+spinnakerdatalength);				//Destination UDP port = 54321															Length inc. UDP headers in bytes = 8+SpiNNakerLength												
	txFrameBase[10]=0x0;			//Checksum of UDP Pseudo Header = 0	+ 	Length of SpiNNaker data payload										
	for(count=0; count<20; count++)         //ip header starts from byte 14 and unsigned short (16bits) 7
		temp[count]=temp1[count+7];
	count=ip_sum_calc(20, temp);                   //calculating the ip checksum
	txFrameBase[6]=(count << 16) | txFrameBase[6];  //including the ip checksum
}
*/

/*******************************************************************
 * 
 * 
 ******************************************************************/
int formatAndSendFrame(unsigned int instruction, unsigned int opton1, unsigned int option2, unsigned int option3)
{
	int length; //, count; //dest addr, src addr, headers, intr, option1, option2, option3
	//unsigned int crcVal;
	//unsigned char *temp; //, *temp1;
	length = 60 + option2;      //option2 carries the spinnaker data payload length
	initFrameHeaders(option2);   //spinnaker data payload length
	//unsigned int* txFrame=new unsigned char[length];   			
	//txFrameBase[dataLengthAt/4]=(txFrameBase[dataLengthAt/4] & 0xFFFF) | ((length & 0xFFFF) << 16); //length to go at byte 42, 43


	txFrameBase[dataLengthAt/4]=(txFrameBase[dataLengthAt/4] & 0xFFFF0000) | (length & 0xFFFF); //length to go at byte 42, 43


	txFrameBase[spinnakerInstructionAt/4]=instruction;		   //instruction to go at instruction field
	txFrameBase[instructionOption1At/4]=opton1;	 //option1 to go at option1
	txFrameBase[instructionOption2At/4]=option2;  //to go at optoin2
	txFrameBase[instructionOption3At/4]=option3;	    //to go at option3
	//temp=(unsigned char*)txFrameBase;
	sendFrame(length);
	return length;   //length of frame
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
// int formatDataFrame(unsigned int* vals, int instruction, unsigned int option1, unsigned int option2)
//{
//	//start flood fill, the data block 1K at the moment, is in the rxFrameBase. option1 contains the size of the block, 
//	//option2 contains the physical address to copy the block to, and optoin3 contains the crc of the block, the data starts from rxFrameBase[24/4]
//	//first do the crc check on the block and if ok carry on with the floodfill, or send a frame asking for the block again
//	//if a frame comes during the floodfill of already received frame, it will be discarded and reported to the Host to wait
//	//for a while unless a message to abort comes from the Host
//	int length, count;
//	unsigned int crcVal;
//	unsigned char* temp;
//	if(option1 > 1024)      
//	{
//		//cout<<"block size should be upto 1K, try again"<<endl;
//		return 0;
//	}
//	length=option1+60; //dest addr, src addr, headers, intr, option1, option2, option3
//	txFrameBase[dataLengthAt/4]=((length & 0xFFFF) >> 16); //length to go at byte 40
//	txFrameBase[spinnakerInstructionAt/4]=instruction;		   //instruction to go at instruction field
//	txFrameBase[instructionOption1At/4]=option1;	 //option1 to go at option1
//	txFrameBase[instructionOption2At/4]=option2;  //to go at optoin2
//	
//	for(count=0; count<option1; count++)          //to make it 60 bytes equivallent
//		txFrameBase[count+(spinnakerDataAt/4)]=vals[count];
//	//leaving preamble and sdf
//	temp=(unsigned char*)txFrameBase;
//	crcVal = Get_CRC((unsigned char*)txFrameBase, length);
//	if(!(length%4))
//		txFrameBase[length/4]=crcVal;
//	else
//	{
//		temp[length]=crcVal & 0xFF;
//		temp[length+1]=(crcVal >> 8) & 0xFF;
//		temp[length+2]=(crcVal >> 16) & 0xFF;
//		temp[length+3]=(crcVal >> 24) & 0xFF;
//	}
//	//txFrameBase[length/4]=crcVal;
//	length+=4;
//	sendFrame(length);
//	return length;   //length of frame
//}

//void formatAndSendFrame(int length, int instruction)
//{
//	int count=0, paycount=0;
//	txFrameBase[0]=sourceMACAddressLS; //destination address ls
//	txFrameBase[1]=((destinationMACAddressLS & 0xFFFF) << 16) | (sourceMACAddressHS & 0xFFFF); //source address hs last 2 bytes and destination address ls 2 firt bytes
//	txFrameBase[2]=((destinationMACAddressHS << 16) | (destinationMACAddressLS >> 16)); //0xACDE4800; //source address hs
//	if(length > 60)
//		instruction = (instruction << 16) | length;
//	else
//		instruction = (instruction << 16) | 60;
//	
//	txFrameBase[3]=instruction; //payload length with instruction, first two bytes to contain the length of payload, next two bytes to contain an instruction
//	if(length < 60)   //padding the frame to make it more than min sized frame
//	{
//		for(count=4; count<60; count++)          //to make it 60 bytes equivallent
//		{
//			if(length > count)
//			{
//				length--;
//				paycount++;
//			}
//			else
//				txFrameBase[count]=count;
//		}
//	}
//	
//	sendFrame(60);
//}
void sendHelloFrame()  
{
	int length=formatAndSendFrame(HELLO, (VERSION<<8) | SUBVERSION, 0, 0);   //instruction, option1=version codes of ROM/library (if >0 then system-wide configuration done), option2=size, 3 = procID and core (n/a)
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendErrorReportFrame(int err)
{
	int length=formatAndSendFrame(ERR, error_code, 0, 0);
	#ifdef VERBOSE
		printf("proc %d: s F: error Report\n", procID);
	#endif
	sendFrame(length);
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendAckFrame()
{
	int length=formatAndSendFrame(ACK, 0, 0, 0);
	#ifdef VERBOSE
		printf("proc %d: s F: ack\n", procID);
	#endif
	sendFrame(length);
}

/*******************************************************************
 * 
 * 
 ******************************************************************/	
//void floodFillData()
//{
//	#ifdef VERBOSE
//		printf("proc %d: in floodFillData\n", procID); 
//	#endif
//	;
//}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void sendHelloToNeighbor()
{
	//instruction 20:   a neighbor to other, hello i am live  (no payload)
	unsigned int rkey=0;
	unsigned int linkstate=0;
	rkey=(0x0F8 << 20) | (NN_DIAG_HELLO << 12);    //31-20 NN Intruction address space, 19-12 instruction type 20
	rkey= (rkey | (0 & 0xFFF));  	    //bits 11-0 
	#ifdef VERBOSE
		printf("proc %d: s P: hello\n", procID);
	#endif
	//check if the mon proc of all the neighbouring chips have been selected
	//if so send a broadcast msg or to only those where the mon proc is alive
	linkstate=(nChipsLinksState >>24) & 0x3F;
	if(linkstate == 0x3F)
		sendNNPacketWOPL(BROADCAST, NORMAL, rkey);
	else    //to only those where the mon proc is live
	{
		if(linkstate & 0x1)
			sendNNPacketWOPL(0, NORMAL, rkey);
		if(linkstate & 0x2)
			sendNNPacketWOPL(1, NORMAL, rkey);
		if(linkstate & 0x4)
			sendNNPacketWOPL(2, NORMAL, rkey);
		if(linkstate & 0x8)
			sendNNPacketWOPL(3, NORMAL, rkey);
		if(linkstate & 0x10)
			sendNNPacketWOPL(4, NORMAL, rkey);
		if(linkstate & 0x20)
			sendNNPacketWOPL(5, NORMAL, rkey);
	}	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
//int pow(int i, int j)
//{
//	int result;
//	result=i;
//	if(j==0)
//		return 1;
//	else if(j < 0) // if power is negative, result error i.e. -1
//		return -1;
//	while(j>0)
//	{
//		result=result*i;
//		j--;
//	}
//	return result;
//}
int lockProc(int procNo)
{
	//A unique single-bit value ‘X’ appears in each register r64 to r95. Reading each register returns
	//0x00000000 or 0x80000000 depending on whether its respective bit was clear or set prior to the
	//read, and as a side-effect the bit is set by the read.
	//Together with r96 to r127, these registers provide support for mutual exclusion primitives for interprocessor
	//communication and shared data structures, compensating for the lack of support for
	//locked ARM ‘swap’ instructions into the System RAM.
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
	//The same unique single-bit value ‘X’ appears in each register r96 to r127 as appears in r64 to r95
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
	if(!lockProc(proc))  //if could not get a lock, quite
		return 0;
	//if data size is more than 16 bytes, do dma operation or just copy it into the system ram
	//bit 15-0 = message, bits 31-16 = size of transfer in bytes i.e. data size
	msg=(msg & 0xFFFF) | ((dataSize & 0xFFFF) << 16);
	//write the message in the first word of the proc N buffer
	writeMemory(addr, msg);
	addr=addr+4;
	if(dataSize > 16)   //if(usedma) //do DMA write
	{
		while(dmaInProcess)
			; //wait till dma is done
		DMAWrite(addr, dataStartAddress, dataSize);
		while(dmaInProcess)
			; //wait till dma is done
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
//passmessageusingpackets(int proc, unsigned int dataStartAddress, int dataSize)
//{
//	//use key 0x0f8000nn where nn is the proc number
//	//first setup the routing tables to support these packets sending across the processors
//	unsigned int no_of_procs=2;
//	unsigned int value=0, count=0;
//	unsigned int rkey=0x0f800000;
//	for(count=0; count<no_of_procs; count++)
//	{
//		value=(0x1 << (6+count));
//		rkey=rkey+count;
//		setupMCTable(count*4, rkey, 0xFFFFFFFF, value); //to proc 
//	}
//	rkey=0x0f800000+proc; //to send to proc
//	for(count=0; count<(dataSize/4); count++)
//	{
//		value=readMemory(dataStartAddress+(count*4));
//		sendMCPacket(rkey, value);
//	}
//}
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
				DMARead(addr, dtcmAddress, dataSize);
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
void sendNN3MsgFwd(int incomingPort, int key, int pload)
{
	int port = (incomingPort + 3) % 6; //opposite port
	if((nChipsLinksState >> (8 + port)) & 0x1) //if the link is live then send an NN packet
		sendNNPacket(port, NORMAL, RxRoutingKey, RxPayload);
	port = (incomingPort + 2) % 6; //one less the opposite port
	if((nChipsLinksState >> (8 + port)) & 0x1) //if the link is live then send an NN packet
		sendNNPacket(port, NORMAL, RxRoutingKey, RxPayload);
	port = (incomingPort + 4) % 6; //one plus the opposite port
	if((nChipsLinksState >> (8 + port)) & 0x1) //if the link is live then send an NN packet
		sendNNPacket(port, NORMAL, RxRoutingKey, RxPayload);
	
}
void handleNNPacket()
{
	  unsigned int incommingAddress, status;
	  int content, count;  //, port;
	  int instruction;
	  int route;
	  int control;
	  route=(RxStatus >> 24)  & 0x7; 			 
//	  printf("NN Incoming packet\n"); // CP debugging 26th Jan 2010

	  if(RxRoutingKey >  NNOP_STARTING_ADDRESS && RxRoutingKey <= NNOP_END_ADDRESS) //the NN normal type of packet with some instruction
	  {
    		  printf("AA Got a NN packet over the ext link\n"); // CP debugging 26th Jan 2010
		  content =	RxRoutingKey & 0x000FFFFF;				    //remove 0x0F8 i.e. 12 MSBs
		  instruction=(content >> 12) & 0xFF;					//instruction in 19-12 
		  switch (instruction)
		  {
		  case NN_FF_START:			//start of broadcast message with block size given in routingkey.range(11, 0) and starting address in the payload
		      //ready to receive the broadcast
		      #ifdef VERBOSE
					printf("proc %d: r P: broadcast msg\n", procID);
			  #endif
			  if(RxPayload == rxBufStAddr) //the block has already been received, or it is the refernce chip
			  {
			  	 #ifdef VERBOSE
					printf("proc %d: block already received, don't do any thing\n", procID);
			  	 #endif
			  	 break;
			  }
			  else
			  {
			  	  rxBufSize =(content & 0xFFF);
			  	  rxBufStAddr = RxPayload;     //if it has not already receive it, copy it
			  	  //make a table in DTCM with entries equal to the blockSize
				  //rxBuffer[rxCurrentBlock] will be used
				  //make a bitmap equal to the rxbufsize
				  //rxBitmap[rxCurrentBlock] will be used
				  //store the starting address and blockSize in some location in DTCM to compare each coming block content for a valid memeber of block
				  //and if so, to store it at its location in the table
				  //passon the packet to other neighbors
				  for(count=0; count<(rxBufSize/32); count++)
					rxBitmap[count]=0;
					
				  if((rxBufSize % 32) > 0)    //it will inform the chip that it is the source or reference chip and it does not have to record the data on receipt of NN packets from the neighbours
				  {
					rxBitmap[count]=0;
				  }
				  //send it to three neighbours in the forward direction
				  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			  }
			  break;
		  case NN_FF_END:				   //end of current broadcast message, crc in the payload
		  {
		        //check CRC and load the data to its location
			    #ifdef VERBOSE
					printf("proc %d: r P: check CRC, load the data\n", procID);
			    #endif
			    if((RxRoutingKey & 0xFFF) == floodfillEndInstrID)  //if this instruction has been received before and acted upon, then do nothing
					break;
				//changed to the above code as rxBufCRC may be the same for two blocks of same data
				//			    if(RxPayload == rxBufCRC)  //block received already
				//			    	break;   //don't pass it further
			    rxBufCRC=RxPayload;
		  	  	//check the bitmap for any missing word
				status=0;   //if block completed
			  	for(count=0; count<(rxBufSize/32); count++)
				{
					if(rxBitmap[count]!=0xFFFFFFFF)
						status=0;
				}
				if((rxBufSize % 32) > 0)  //if some bitmap bits are left then
				{
					int offset = rxBufSize % 32;
					unsigned int value=rxBitmap[rxBufSize/32];
					for(count=0; count<offset; count++)
					{
						if(((value >> count) & 0x1) ==0)
							status=0;
					}
				}
				if(status==1) //if block is complete
				{
					  if(!checkCRC(rxBuf, rxBufSize, rxBufCRC))
					  {
					  		//send nn packet to incoming port chip crc failed, send me the block again
					  		int currentIntructionID=0;
					  		unsigned int rkey=0, pload=0;
					   		rkey=(0x0F8 << 20) | (NN_FF_RESEND_BLOCK << 12);     //31-20 NN Intruction address space, 19-12 instruction
							currentIntructionID = currentTime + NN_FF_RESEND_BLOCK;  //used to avoid looping around of the same instruction
							rkey= (rkey | ( currentIntructionID & 0xFFF));  //
							pload= rxBufStAddr;  //option3=destination address
							sendNNPacket(route, NORMAL, rkey, pload);   	
					  }
					  else
					  {
					  		opMode=STORE_BLOCK_TO_MEMORY;
					  		floodfillEndInstrID = (RxRoutingKey & 0xFFF);
					  }
					  		//storeBlockToMemory(rxBuf, rxBufStAddr, rxBufSize);
				}
				else    //if block is not received properly
				{
					opMode = REQUEST_NEIGHBOUR_FOR_MISSING_WORDS;
					incomingPacketPort=route;
				}
			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			  break;
		  }
//		  case NN_FF_INTERRUPT:				   // interrupt current broadcast
//		  {
//		  	  #ifdef VERBOSE
//					printf("proc %d: r P: int broadcast msg\n", procID);
//		      #endif
//			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
//			  break;
//		  }
//		  case NN_FF_RESUME:				   //resume broadcast
//		  {
//		  	  #ifdef VERBOSE
//					printf("proc %d: r P: resume broadcast msg\n", procID);
//		  	  #endif
//			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
//			  break;
//		  }
//		  case NN_FF_CRC:				    //block level checksum, size in the routingkey.range(11, 0), checksum in payload
//		  {
//			  #ifdef VERBOSE
//					printf("proc %d: r P: checksum msg\n", procID);
//			  #endif
//			  rxBufCRC=RxPayload;
//			  //rxBufCRCSize=content & 0xFFF; //bit 11-0  //for the time being fixed to 32 bit CRC
//			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
//			  break;
//		  }
//		  case NN_FF_REMAINING_CRC:						//remaining of checksum (if more than 32 bits), sequence number in routingkey.range(11, 0)
//		  {   //not implemented
//			  #ifdef VERBOSE
//					printf("proc %d: r P: remaining checksum msg\n", procID);
//			  #endif
//			  //rxBufCRC2=RxPayload;
//			  //rxBufCRCSize=content & 0xFFF; //bit 11-0  //for the time being fixed to 32 bit CRC
//			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
//			  break;
//		  }
		  case NN_CONF_SYS_SIZE:						//size of system in payload
		  {
		  	  if(((RxRoutingKey & 0xFFF) == systemSizeInstrID) || isRefChip) //if this instruction has been received before and acted upon, then do nothing
					break;
			  systemSizeInstrID = (RxRoutingKey & 0xFFF);
		  	  spinnakerSystemSize=RxPayload;
		  	  #ifdef VERBOSE
					printf("proc %d: r P: size of system: %d\n", procID, spinnakerSystemSize);
		  	  #endif
		  	  systemSizeInstrID = (RxRoutingKey & 0xFFF);
			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			  break;
		  }
		  case NN_CONF_BREAK_SYMMETRY:					// 8:   break the symmetery, assign address to yourself, the address of sender chip is in payload
		  {
			//the mon processor needs to calculate its address and write it to system RAM
			//so that it is available to all processors and to a register in the router 
			//so that the router can get the packets destined for itself
			//the mon processor will need this address as sourceID in the P2P packets
			//the mon processor will calculate this address from two things in the incomming packet
			//i.e. the address of the neighboring chip that has sent this packet and the port from where it
			//has received this packet. this information is in the 'route' part of the NN type packet
			//embedded by the local router
			#ifdef VERBOSE
					printf("proc %d: r P: break Symmetry\n", procID);
			#endif
			incommingAddress = RxPayload;  //gettting first 16 bits as the address of the sending chip 
			//route=(RxStatus >> 24)  & 0x7;  //bit 26-24 of R1 register contain NNroute
			if(((RxRoutingKey & 0xFFF) == symmetryBrokenInstrID) || isRefChip) //if this instruction has been received before and acted upon, then do nothing
				break;
			else    //if it is not the reference chip, assign itself the relative address
			{
				switch(route)
				{
					case 0:      //East
						//reduce one from x coordinate
						chipAddress=((incommingAddress & 0xFF) -1 % spinnakerSystemSize);
						chipAddress=chipAddress | (incommingAddress & 0xFF00);
						break;
					case 1:		//north east
						//reduce one from x and y coordinates
						chipAddress = ((incommingAddress -1) % spinnakerSystemSize);
						chipAddress = (chipAddress | ((incommingAddress & 0xFF00) -1 % spinnakerSystemSize));
						break;
					case 2:		//north 
						//reduce one from y coordinate
						chipAddress = (incommingAddress & 0xFF) ;
						chipAddress = (chipAddress | ((incommingAddress & 0xFF00) -1 % spinnakerSystemSize));
						break;
					case 3:		//west
						//increase one to x coordinate
						chipAddress = ((incommingAddress + 1) % spinnakerSystemSize);
						chipAddress = (chipAddress | (incommingAddress & 0xFF00));
						break;
					case 4:		//south west
						//increase one to x and y coordinates
						chipAddress = ((incommingAddress & 0xFF)+1 % spinnakerSystemSize);;
						chipAddress = (chipAddress | ((incommingAddress & 0xFF00) + 1 % spinnakerSystemSize));
						break;
					case 5:		//south 
						//increase one to y coordinate
						chipAddress = (incommingAddress & 0xFF) ;
						chipAddress = (chipAddress | ((incommingAddress & 0xFF00) + 1 % spinnakerSystemSize));
						break;
					default:
						break;
				}
				setupChipID();
				symmetryBrokenInstrID=(RxRoutingKey & 0xFFF);  //set the flag to repeat the process
				setupRouterP2PTable();
				#ifdef VERBOSE
						printf("proc %d: r P: initialize P2P table\n", procID);
				#endif
				sendNN3MsgFwd(route, RxRoutingKey, RxPayload);	//passing on own chip address forward
			}
			break;
		  }
		  case NN_CONF_RESET_TIME_PHASE:				//reset time phase, timephase granularity (number of cycles) to be used is in the payload
		  {
		  	  if((RxRoutingKey & 0xFFF) == resetTimePhaseInstrID) //if this instruction has been received before and acted upon, then do nothing
				break;
		  	  systemTimePhase=RxPayload;				//timephase controls the timephase clock frequency in the router
			  #ifdef VERBOSE
					printf("proc %d: r P: timephase: %d\n", procID, systemTimePhase);
			  #endif
			  //see if the router timephase is the same, don't pass the packet forward
			  status=readRouterTimePhase();
			  if(status != 0)
			  {
			  	setRouterTimePhase(0);
			  	sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			  }
			  symmetryBrokenInstrID = (RxRoutingKey & 0xFFF);
			  //else do nothing and dont send it to forwards
			  break;
		  }
		  case NN_CONF_SEND_OK_REPORT:					//report ok (no payload)
			  //send the OK messsage to the host PC
		      //OK means everything is fine, or send the state of processors, systemram, sdram, router, neighbors
		      //send a series of p2p packets with a payload containing the instruction in the payload, all ok is sent as one p2p packet, if any problem, the status is sent in the following packet
			  #ifdef VERBOSE
					printf("proc %d: r P: report OK\n", procID);
			  #endif
			  		 
			  if((RxRoutingKey & 0xFFF) == sendStatusReportInstrID )  //if this instruction has been received before and acted upon, then do nothing
				  break;
			  status=readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
			  if(((enableEthernet == 1) && ((status >> 20) == 0xFF)) || ((enableEthernet == 0) && ((status >> 20) == 0x3F)))
		  	  {
		  			status=(0x0F8 << 20) | (P2P_COMM_STATUS_OK << 12) | (p2pInDataSize);
		  			sendP2PPacket(0, 0, status); //all correct report to Host System
		  	  }
		  	  sendStatusReportInstrID = (RxRoutingKey & 0xFFF);
			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			  break;   //kernel is loaded, start using it
			//		  case NN_CONF_BOOT_INTRS_DONE:				// exra boot instructions and micro kernal loaded (no payload)
			//		      #ifdef VERBOSE
			//					printf("proc %d: r P: kernel loaded\n", procID);
			//		      #endif
			//			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);	//passing on own chip address forward
			//			  
			//			  break;
			//		  case NN_CONF_APP_LOAD_FROM:    //the size of the application to be loaded into the ITCM
			//		  		if((RxRoutingKey & 0xFFF) == appLoadFromInstrID) //if this instruction has been received before and acted upon, then do nothing
			//				  	break;
			//		  		applicationSourceAddress=RxPayload;
			//		  		sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			//		  	  	break;
			//		  case NN_CONF_APP_LOAD_TO:    //the address of the main procedure in the application, to hand over the control to the application
			//		  		if((RxRoutingKey & 0xFFF) == appLoadToInstrID)  //if this instruction has been received before and acted upon, then do nothing
			//				  	break;
			//		  		applicationDestAddress=RxPayload;
			//		  		sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			//		  	 	break;	
			//		  case NN_CONF_APP_LOAD_SIZE:
			//		  		if((RxRoutingKey & 0xFFF) == appLoadSizeInstrID)  //if this instruction has been received before and acted upon, then do nothing
			//				  	break;
			//		  		applicationSize=RxPayload;
			//		  		sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			//		  	  	break;	  		
		  case NN_CONF_APP_LOAD_START:    //application is loaded
		  		if((RxRoutingKey & 0xFFF) == appLoadStartInstrID) //if this instruction has been received before and acted upon, then do nothing
				  	break;
		  		applicationSourceAddress=RxPayload;  
				//application address in SDRAM in payload, 
				//copy the application from this address to the ITCM address and hand over the control to the application
				//the 
				#ifdef VERBOSE
						printf("proc %d: r P: load and start application\n", procID);				  			  
				#endif
				//				applicationMainAddress=RxPayload;
				//				sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
				//				  //now disable all the interrupts and go to the code that will load the application here
				//				control = readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
				//				control = control & 0xFFFFF; //see the live processors
				//				//pass the message individually to all the processors
			    //see which processors are alive and pass the message to only those who are alive								
   				control = readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
   				control = control & 0xFFFFF; //see the live processors
   				//pass the message individually to all the processors
   				for(count=0; count<20; count++)
   				{
   					if(((control >> count) & 0x1) && (procID != count)) //the proc is live and it is not the mon proc then
   					{
   						passMessage(SM_LOAD_APPLICATION_DATA, count, applicationSourceAddress, 16);  						//move 12 bytes i.e. 4 words from the data load point in SDRAM 
   																															//containing the address to load data from, address of TCM to load data to
   																															//the size of the data and the address to load into the pc
   					}
   				}
   				appLoadStartInstrID = (RxRoutingKey & 0xFFF); 
   				sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			    break;
				//		  case NN_CONF_MON_APP_LOAD_FROM: 
				//		  	   if((RxRoutingKey & 0xFFF) == monAppLoadFromInstrID)        //if this instruction has been received before and acted upon, then do nothing
				//				  	break;
				//		       applicationSourceAddress=RxPayload;
				//		       sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
				//			   break;
				//		  case NN_CONF_MON_APP_LOAD_TO: 
				//		  	  if((RxRoutingKey & 0xFFF) == monAppLoadToInstrID)   //if this instruction has been received before and acted upon, then do nothing
				//				  	break;
				//		      applicationDestAddress=RxPayload;
				//		      sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
				//			  break;
				//		  case NN_CONF_MON_APP_SIZE: 
				//		  	  if((RxRoutingKey & 0xFFF) == monAppSizeInstrID)   //if this instruction has been received before and acted upon, then do nothing
				//				 	break;
				//			  applicationSize=RxPayload;
				//			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
				//			  break;
		  case NN_CONF_MON_APP_START: 
		  	  if((RxRoutingKey & 0xFFF) == monAppLoadStartInstrID) //if this instruction has been received before and acted upon, then do nothing
				  	break;
		  	  applicationSourceAddress=RxPayload;  
				  //application address in SDRAM in payload, 
				  //copy the application from this address to the ITCM address 0 and hand over the control to the application
			  #ifdef VERBOSE
						printf("proc %d: r P: load and start application\n", procID);				  			  
			  #endif
			  //applicationMainAddress=RxPayload;
			  //applicationSourceAddress=RxPayload;
			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
			  monAppLoadStartInstrID = (RxRoutingKey & 0xFFF); 
		  	  loadApplicationAndStart(readMemory(applicationSourceAddress), readMemory(applicationSourceAddress+4), readMemory(applicationSourceAddress+8), readMemory(applicationSourceAddress+12));
			  break;
		   case NN_CONF_LOAD_APP_DATA: 
		  	  if((RxRoutingKey & 0xFFF) == loadAppDataInstrID) //if this instruction has been received before and acted upon, then do nothing
				  	break;
		  	  applicationSourceAddress=RxPayload;  
				  //application address in SDRAM in payload, 
				  //copy the application from this address to the ITCM address 0 and hand over the control to the application
			  #ifdef VERBOSE
						printf("proc %d: r P: load and start application\n", procID);				  			  
			  #endif
			  //applicationMainAddress=RxPayload;
			  //applicationDestAddress=RxPayload;
			  loadAppDataInstrID = (RxRoutingKey & 0xFFF);
			  sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
		  	  control = readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
			  control = control & 0xFFFFF; //see the live processors
				//pass the message individually to all the processors
			  for(count=0; count<20; count++)
			  {
				  if(((control >> count) & 0x1) && (procID != count)) //the proc is live and it is not the mon proc then
				  {
					  passMessage(SM_LOAD_APPLICATION_DATA, count, applicationSourceAddress, 16);  //move 16 bytes i.e. 4 words from the data load point in SDRAM 
																															//containing the address to load data from, address of TCM to load data to
																															//the size of the data and the address to load into pc
				  }
			  }
			  break;
		  case NN_DIAG_HELLO:		//a neighbor to other, i am fine how are you   (no payload)
						//when it receives the hello message, the chip keep updating the status of its neighbors in neighbor status table in DTCM
			  			//if it received ok from all the chips, it is fine or it will probe into the resources of any chip fail to 
			  			//send ok message so far.
			  #ifdef VERBOSE
					printf("proc %d: r P: hello\n", procID);
			  #endif
			  //route=(RxStatus >> 24) & 0x7;  //to be replaced by 24 to get the route from the route field of r1 register of communication controller after confirming
			  //unsigned int nChipsLinksState=0;        //local copy of sysRam[SR_NCHIPS_LINK_STATE], bits(5-0) nn chips state, bits(21-16) Rx links state, bits(29-24) Tx links state
			  nChipsLinksState=nChipsLinksState | (0x1 << route);  //set the chip status
			  //not sure about tx links but rx is fine
			  //nChipsLinksState=nChipsLinksState | (0x1 << (route + 8));  //set the tx link status
			  nChipsLinksState=nChipsLinksState | (0x1 << (route + 16));  //set the rx link status
			  nChipsLinksState=nChipsLinksState | (0x1 << (route + 24));  //set the mon proc status 
			  
			  break;
		  case NN_DIAG_HOW_R_U:					//how are you, from a neighbor who did not receive the hello message, send a hello message again to that neighbor
			  //unsigned int nChipsLinksState=0;        //local copy of sysRam[SR_NCHIPS_LINK_STATE], bits(5-0) nn chips state, bits(21-16) Rx links state, bits(29-24) Tx links state

			  #ifdef VERBOSE
					printf("proc %d: r P: how are you NN\n", procID);
			  #endif
			  //route=(RxStatus >> 24) & 0x7;  //to be replaced by 24 to get the route from the route field of r1 register of communication controller after confirming
			  //first see if the status of this chip is not marked alive, mark it alive
			  nChipsLinksState=nChipsLinksState | (0x1 << route);  //set the chip status
			  //not sure about tx links but rx is fine
			  //nChipsLinksState=nChipsLinksState | (0x1 << (route + 8));  //set the tx link status
			  nChipsLinksState=nChipsLinksState | (0x1 << (route + 16));  //set the tx link status
			  nChipsLinksState=nChipsLinksState | (0x1 << (route + 24));  //set the mon proc status 
			  
			 
			  if(coreChipStatus!=0)   //if this chip is ok, send ok message
			  {
				  content=  NNOP_STARTING_ADDRESS;
				  content= content | (NN_DIAG_HELLO << 12);    //hello, i am fine 
				  sendNNPacketWOPL(route, NORMAL, content);
			  }	
			  break;
		  case NN_DIAG_HAVE_U_HEARD_OUR_N:				   // have you heard from your neighbor on  E, W, NE, NW, SE, SW	 in	 routingkey.range(11, 0), no payload
			  //route=(RxStatus >> 24) & 0x7;  //to be replaced by 24 to get the route from the route field of r1 register of communication controller after confirming
			  incommingAddress=(RxRoutingKey & 0xF);  //the port number of dead chip about which the neighbour is asking
			  #ifdef VERBOSE
					printf("proc %d: r P: msg from: %d, have you heard on port:%d neighbour\n", procID, route, incommingAddress);
			  #endif
			  if((nChipsLinksState & (0x1 << incommingAddress)) != 0)
			  {
			  	  content=  NNOP_STARTING_ADDRESS;
				  content= content | (NN_DIAG_YES_I_HEARD << 12);    //response to the common neighbor, yes the common neighbor on my port (in bits 11-0) is alive
			      content= content | incommingAddress;
			      sendNNPacketWOPL(route, NORMAL, content);
			  }
			  else
			  {
			  	  content=  NNOP_STARTING_ADDRESS;
				  content= content | (NN_DIAG_NO_I_DIDNOT << 12);    //response to the common neighbor, no the common neighbor on my port (in bits 11-0) is not alive
			      content= content | incommingAddress;
			      sendNNPacketWOPL(route, NORMAL, content);
			  }
			  break;
		  case NN_DIAG_YES_I_HEARD:	 //response from a common neighbor, yes the common neighbor on my port (in bits 11-0) is alive
			  //route=(RxStatus >> 24) & 0x7;  //to be replaced by 24 to get the route from the route field of r1 register of communication controller after confirming
			  incommingAddress=(RxRoutingKey & 0xF);
			  #ifdef VERBOSE
					printf("proc %d: r P: res. from:%d, yes i have heard on port:%d\n", procID, route, incommingAddress);
			  #endif
			  //update the nChipsLinksState
			  if(route == 0)
			  {
			  	//it is the response about the chip on my link 1
			  	 if(incommingAddress == 2)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (1+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (1+8));
			  	 	if(((nChipsLinksState  >> (1+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (1+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 1);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (1+24));
			  	 }
			  	 else if(incommingAddress == 4)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (5+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (5+8));
			  	 	if(((nChipsLinksState  >> (5+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (5+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 5);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (5+24));
			  	 }
			  }
			  else if(route == 1)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 3)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (2+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (2+8));
			  	 	if(((nChipsLinksState  >> (2+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (2+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 2);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (2+24));
			  	 	
			  	 } 
			  	 else if(incommingAddress == 5)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (0+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (0+8));
			  	 	if(((nChipsLinksState  >> (0+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (0+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 0);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (0+24));
			  	 	
			  	 }
			  	 	
			  }
			  else if(route == 2)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 0)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (1+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (1+8));
			  	 	if(((nChipsLinksState  >> (1+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (1+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 1);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (1+24));
			  	 }
			  	 else if(incommingAddress == 4)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (3+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (3+8));
			  	 	if(((nChipsLinksState  >> (3+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (3+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 3);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (3+24));
			  	 }
			  }
			  else if(route == 3)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 1)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (2+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (2+8));
			  	 	if(((nChipsLinksState  >> (2+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (2+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 2);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (2+24));
			  	 }
			  	 else if(incommingAddress == 5)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (4+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (4+8));
			  	 	if(((nChipsLinksState  >> (4+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (4+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 4);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (4+24));
			  	 }
			  }
			  else if(route == 4)
			  {
			  	//it is the response about the chip on my link 1
			  	 if(incommingAddress == 0)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (5+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (5+8));
			  	 	if(((nChipsLinksState  >> (5+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (5+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 5);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (5+24));
			  	 }
			  	 else if(incommingAddress == 2)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (3+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (3+8));
			  	 	if(((nChipsLinksState  >> (3+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (3+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 3);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (3+24));
			  	 }
			  }
			  else if(route == 5)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 1)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (0+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (0+8));
			  	 	if(((nChipsLinksState  >> (0+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (0+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 0);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (0+24));
			  	 }
			  	 else if(incommingAddress == 3)
			  	 {
			  	 	//mark the chip as alive and mark the link as dead if the were still not marked alive
			  	 	if(((nChipsLinksState  >> (2+8)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (2+8));
			  	 	if(((nChipsLinksState  >> (2+16)) & 0x1) == 0)  //txlink 1 was still not marked alive
			  	 		nChipsLinksMarkState = nChipsLinksMarkState  | (0x1 << (2+16));
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << 2);
			  	 	nChipsLinksState=nChipsLinksState | (0x1 << (2+24));
			  	 	
			  	 }
			  }
			  
			  break;
		  case NN_DIAG_NO_I_DIDNOT:	//response from a common neighbor, no i have not heard from neighbor neighbor on my port (bits 11-0)
		  	  //route=(RxStatus >> 24)  & 0x7;
			  incommingAddress=(RxRoutingKey & 0xF);
			  #ifdef VERBOSE
				  printf("proc %d: r P: res from:%d, no i have not heard on port:%d\n", procID, route, incommingAddress);
			  #endif
			  //update the nChipsLinksState to ensure the chip has been marked as dead
			  //make it a case statement   
			  if(route == 0)
			  {
			  	//it is the response about the chip on my link 1
			  	 if(incommingAddress == 2)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 1);//0xFD; //| (0x1 << 1); 1111 1101
			  	 else if(incommingAddress == 4)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 5);//0xDF; //| (0x1 << 5); 1101 1111
			  }
			  else if(route == 1)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 3)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 2);//0xFB; //| (0x1 << 2); 1111 1011
			  	 else if(incommingAddress == 5)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 0);//& 0xFE; //| (0x1 << 0); 1111 1110
			  }
			  else if(route == 2)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 0)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 1);//0xFD; //| (0x1 << 1); 1111 1101
			  	 else if(incommingAddress == 4)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 3);//0xF7; //| (0x1 << 3); 1111 0111
			  }
			  else if(route == 3)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 1)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 2);// 0xFB; //| (0x1 << 2); 1111 1011
			  	 else if(incommingAddress == 5)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 4);//0xEF; //| (0x1 >> 4); 1110 1111
			  }
			  else if(route == 4)
			  {
			  	//it is the response about the chip on my link 1
			  	 if(incommingAddress == 0)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 5);//0xDF; //| (0x1 << 5); 1101 1111
			  	 else if(incommingAddress == 2)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 3);//0xF7; //| (0x1 << 3); 1111 0111
			  }
			  else if(route == 5)
			  {
			  	 //it is the response about the chip on my link 1
			  	 if(incommingAddress == 1)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 0);//0xFE; //| (0x1 << 0); 1111 1110
			  	 else if(incommingAddress == 3)
			  	 	nChipsLinksState=nChipsLinksState & ~(0x1 << 2);//0xFB; //| (0x1 << 2); 1111 1011
			  }
			  break;
		  default:
			  break;
		  }
	  }
	  else if(RxRoutingKey & 0x1 > 0)//if it is a response of a direct type of packet, bit 0 of routing key in a response is set to 1
	  {
	  	 //route=(RxStatus >> 24)  & 0x7;
	  	 if(route == nnAckDueRoute)
	  	        printf("AAA\n");
	  	 	nnAckDue=0;  //clearing the ack due flag for the application
	  	 RxRoutingKey = (RxRoutingKey >> 2) << 2;  //clearing first 2 bits
	  	 if(RxRoutingKey == SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID) //mon proc id register read value
	  	 {
	  	 	printf("BBB Routing Key: 0x%x + Status: 0x%x.\n",RxRoutingKey,RxStatus >> 16); // CP debugging 26th Jan 2010
	  	 	readMonProcIDDone = 1;
	  	 	//reset the nChipsLinksState 
	  	 	nChipsLinksState |= (0x1 << (route+8)); //setting the txlink as up
	  	 	nChipsLinksState |= (0x1 << (route+16)); //setting the rxlink as up
	  	 	deadChipMonProcValue=RxPayload;
	  	 	if(!((RxPayload & 0x1F) == 0x1F))  //if the mon proc has been selected
	  	 	{
	  	 		nChipsLinksState |= (0x1 << (route+24)); //setting the link as up
	  	 	}
	  	 }
	  	 if(RxRoutingKey == SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK) //mon proc id register read value
	  	 {
	  	 	printf("CCC\n");
	  	 	readCPUOKDone = 1;
	  	 	//reset the nChipsLinksState 
	  	 	deadChipCPUOKValue=RxPayload;
	  	 }
	  	 printf("DDD\n");
	  }
	  else if(RxRoutingKey & 0x2 > 0)//if is a response of a direct type of packet, bus error
	  {
	  		printf("CC Got a NN packet over the ext link\n"); // CP debugging 26th Jan 2010
	  	 //do something or resend the request
	  	;
	  }
	  else //it is a block content with a physical address in its routing key and data in its payload, needs be stored in the table after validating its address 
	  {
	  		printf("DD Got a NN packet over the ext link\n"); // CP debugging 26th Jan 2010
	  		#ifdef VERBOSE
					printf("proc %d: r P: got data %d\n", procID, RxPayload);
			#endif
			//check if it is a response to NN direct packet
			control = (RxStatus >> 16) & 0xFF;
			if(control & 0x20)  //if direct packet and came to the monitor processor, it is a response
			{
				if(control & 0x2)  //check if it was read request coming back with data
				{
					if(RxRoutingKey == (SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK))
					{
						readCPUOKDone=1;
						deadChipCPUOKValue=RxPayload;
					}
					else if(RxRoutingKey == (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID))
					{
						readMonProcIDDone=1;
						deadChipMonProcValue=RxPayload;
					}
				}
			}
							
			if(RxRoutingKey >= rxBufStAddr && RxRoutingKey < rxBufStAddr+rxBufSize)
			{
				//store it at the designated location in the block table i.e. inside DTCM
				int storingAddress=(int)(RxRoutingKey - rxBufStAddr);
				int bitmapbase=storingAddress/32;
				int bitmapoffset=storingAddress%32;
				if((rxBitmap[bitmapbase] >> bitmapoffset) & 0x1)
					; //just leave it, duplicate
				else  //send and save only if not seen before
				{
					rxBitmap[bitmapbase]=rxBitmap[bitmapbase] | 0x1 << bitmapoffset; //mark relevant bit in the bitmap
					rxBuf[storingAddress]=RxPayload;
					sendNN3MsgFwd(route, RxRoutingKey, RxPayload);
				}
				if(RxRoutingKey == missingWordAddress)  //a response to the resend word request
					missingWordWaiting=0;
				
			}
			else
			{
				;
			}
	  }
}
void sendAckP2PPacket(unsigned int destID, int lastInstruction)  //send ack p2p packet in response to some instuction or data, requesting instruction is send along in bits 11-0
{
	unsigned int rkey, pload;
	rkey=((chipAddress << 16) | (destID & 0xFFFF));   //source address, destination address
	pload=(0x0F8 << 20) | (P2P_COMM_ACK << 12) | (lastInstruction & 0xFF);
	sendP2PPacket(0 /*sequence code*/, rkey, pload);
}
void sendP2PErrorPacket()
{

	
}
int p2pHostToOtherChip(int chipAddress, int instruction, unsigned int option, int seqNo)
{
	unsigned int rkey, pload;
	int count=0;
	if(instruction == P2P_COMM_DATA_PACKET)
	{
		rkey=((chipAddress << 16) | (chipAddress & 0xFFFF));   //source address, destination address
		pload=option;
		sendP2PPacket(seqNo, rkey, pload);
	}
	else
	{
		rkey=((chipAddress << 16) | (p2pInAddress & 0xFFFF));   //source address, destination address
		pload=(0x0F8 << 20) | (instruction << 12) | (option);
		sendP2PPacket(seqNo, rkey, pload);
	}
	while(p2pAckPending && (count < P2P_WAIT_CYCLES)) //wait till response does not comeback or time out
	{
		count++;   //wait for the ack to come 
	}
	if(!p2pAckPending) //send the status
	{
		//sendAckFrame();
		return 1;
	}
	else //timeout
	{
		sendErrorReportFrame(ERR_REQUEST_TIMEOUT);
		return 0;
	}
}

/*******************************************************************
 * 
 * 
 ******************************************************************/	
void handleP2PPacket()
{
	//	rkey=((chipAddress << 16) | (p2pInAddress & 0xFF));   //source address, destination address
	//	pload=(0x0F8 << 20) | (p2pInInstruction << 12) | (p2pInDataSize);
	int instr, count, count2, p2pSeqNo;
	unsigned int val; //, rkey;
	int sourceAddress=(RxRoutingKey >> 16) & 0xFFFF;
	p2pSeqNo=((RxStatus >> 4) & 3);
	if(((RxPayload >> 20) & 0xFFF) == 0x0F8) //it is an instruction p2p packet
	{
		instr = (RxPayload  >> 12) & 0xFF;   //p2p instruction
		switch(instr)
		{
			case P2P_COMM_REPORT_STATUS:		  //	send the cpu_ok register in the system controller, packet is sent as an acknowledgement
				if(isRefChip==1) //if it is a ref chip i.e. host connected and asking for status
				{
					//it may be from some other host connected chip
					val=readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
					sendP2PPacket(0, sourceAddress, val);  //status is sent as an ack
				}
				else
				{
					val=readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
					sendP2PPacket(0, sourceAddress, val);
					
				}
				break;
			case P2P_COMM_GET_DATA:		  		//  receive the data to be copied to memory, size of data in the bits 11-0, next packet to carry the starting address of the block to be copied
				if(isRefChip==1) //if it is a ref chip i.e. host connected then do nothing
				{
					;
				}
				else
				{
					p2pCurrentSrcChipAddress=sourceAddress;
					currentP2PSeqNo=0;
				}
				break;
			case P2P_COMM_SEND_DATA:			 //  send the data from the memory, the physical address in the payload, only one words is sent as a result of this request
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					val=readMemory(RxPayload);
					sendP2PPacket(0, sourceAddress, val);   //data packet is sent as an ackowlegement
					
				}
				break;
			case P2P_LOAD_APPLICATION_PROCIDS:
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					//the procIDs are send with this instruction in two p2p packets
					//the first packet with the instruction P2P_LOAD_APPLICATION_PROCIDS but seqno=0 contains proID (11-0) in bits (11-0) 
					//of the payload, the second packet with the same instruction but seqno=1 contains the remaining
					//procIDs in its bits (11-0)
					//first clear the bits and then copy the new bits
					if(((RxStatus >> 4) & 0x3) == 0) //seq no 0
					{
						p2pProcsForLoadingApplication = p2pProcsForLoadingApplication & 0xFFFFF000; //clearing bits 11-0
						p2pProcsForLoadingApplication = p2pProcsForLoadingApplication | (RxPayload & 0xFFF);
						sendAckP2PPacket(sourceAddress, P2P_LOAD_APPLICATION_PROCIDS);
					}
					else if(((RxStatus >> 4) & 0x3) == 1) //seq no 1
					{
						p2pProcsForLoadingApplication = p2pProcsForLoadingApplication & 0xFF000FFF; //clearing bits 11-0
						p2pProcsForLoadingApplication = p2pProcsForLoadingApplication | ((RxPayload & 0xFFF) << 11);
						sendAckP2PPacket(sourceAddress, P2P_LOAD_APPLICATION_PROCIDS);
					}
				}
				break;			
						
			case P2P_LOAD_APPLICATION_ADDRESS:	
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					//the application address is passed with this instruction in three p2p packets
					//the first packet with this instruction but seqno=0 contains addres bits (11-0) in payload bits (11-0) 
					//the second packet with the same instruction but seqno=1 contains the address bits (23-12)
					//the third packet with the same instruction but seqno=2 contains the remaining address bits
					//first clear the bits and then copy the new bits
					if(((RxStatus >> 4) & 0x3) == 0) //seq no 0
					{
						p2pAddressForLoadingApplication = p2pAddressForLoadingApplication & 0xFFFFF000; //clearing bits 11-0
						p2pAddressForLoadingApplication = p2pAddressForLoadingApplication | (RxPayload & 0xFFF);
						sendAckP2PPacket(sourceAddress, P2P_LOAD_APPLICATION_ADDRESS);
					}
					else if(((RxStatus >> 4) & 0x3) == 1) //seq no 1
					{
						p2pAddressForLoadingApplication = p2pAddressForLoadingApplication & 0xFF000FFF; //clearing bits 11-0
						p2pAddressForLoadingApplication = p2pAddressForLoadingApplication | ((RxPayload & 0xFFF) << 12);
						sendAckP2PPacket(sourceAddress, P2P_LOAD_APPLICATION_ADDRESS);
					}
					else if(((RxStatus >> 4) & 0x3) == 2) //seq no 2
					{
						p2pAddressForLoadingApplication = p2pAddressForLoadingApplication & 0x00FFFFFF; //clearing bits 11-0
						p2pAddressForLoadingApplication = p2pAddressForLoadingApplication | ((RxPayload & 0xFFF) << 24);
						sendAckP2PPacket(sourceAddress, P2P_LOAD_APPLICATION_ADDRESS);
					}
				}
				break;		
			case P2P_LOAD_APPLICATION:
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					val = readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
	   				val = val & 0xFFFFF; //see the live processors
	   				val = val & (p2pProcsForLoadingApplication & 0xFFFFF); //the processor ids to load application and they are alive
	   				//pass the message individually to all the processors
	   				for(count=0; count<20; count++)
	   				{
	   					if(((val >> count) & 0x1) && (procID != count)) //the proc is live and it is not the mon proc then
	   					{
	   						passMessage(SM_LOAD_APPLICATION, count, p2pAddressForLoadingApplication, 16);  //move 16 bytes i.e. 4 words from the application load point in SDRAM 
	   																														//containing the address to load application from, address of ITCM to load application to
	   																														//the size of the application and the location of starting point in the application
	   					}
	   				}
	   				sendAckP2PPacket(sourceAddress, P2P_LOAD_APPLICATION);
				}
				break;
								
			case P2P_COMM_DIAG_CHIP:		    //  be nurse chip to repair chip on your port in bits 11-0
				//p2pState=P2P_DIAG_CHIP;
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					opMode = NN_DIAG;
	 				nndiagstate=NNDIAG_DEAD_CHIP_RECOVERY;				//case 5: read the system ctl
	        		deadchipDiagState=DEADCHIPDIAG_READ_SYSCTRL;
	        		sendAckP2PPacket(sourceAddress, P2P_COMM_DIAG_CHIP);
				}
				break;
			case P2P_COMM_RESET_CHIP:			//  reset chip on you port in bits 11-0
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					val=(0x5EC <<20)|(0xFFFFF);  //resetting all components
					sendNNPacket(count2, DIRECT, SYSCTLR_BASE_U+SYSCTLR_SBSYS_RST_PU, val);    //writing to bit 0 of SYSCTLR_SBSYS_RST_LV register to reset the whole chip
					sendAckP2PPacket(sourceAddress, P2P_COMM_RESET_CHIP);
				}
				break;
			case P2P_COMM_DISABLE_CHIP:		   //  disable the processors on chip on you port in bits 11-0
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					val=(0x5EC <<20)|(0xFFFFF);  //resetting all components
					sendNNPacket(count2, DIRECT, SYSCTLR_BASE_U+SYSCTLR_CPU_DISABLE, val);    //writing to bit 0 of SYSCTLR_SBSYS_RST_LV register to reset the whole chip
					sendAckP2PPacket(sourceAddress, P2P_COMM_DISABLE_CHIP);
				}
				break;
			case P2P_COMM_DISABLE_NN_PROC:		//  disable the processors nubmer in bits 11-8 on chip on you port in bits 7-0
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					val=(0x5EC <<20)|(0x1 << count);  //resetting a particular processor
					sendNNPacket(count2, DIRECT, SYSCTLR_BASE_U+SYSCTLR_CPU_DISABLE, val);    //writing to bit 0 of SYSCTLR_SBSYS_RST_LV register to reset the whole chip
					sendAckP2PPacket(sourceAddress, P2P_COMM_DISABLE_NN_PROC);
				}
				break;
			case P2P_COMM_RESET_LINK:			//  reset your link in bits 11-0
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					val = RxPayload & 0xFFF; //link number to be reset
					writeMemory(SYSCTLR_BASE_U+SYSCTLR_SBSYS_RST_LV, ( ( 0x1 << (4 + val) ) | ( 0x1 << (10 + val) ) ) );  //tx and rx link bit numbers in the register
					sendAckP2PPacket(sourceAddress, P2P_COMM_DISABLE_NN_PROC);
				}
				break;
			case P2P_COMM_DISABLE_LINK:		//  disable your link in bits 11-0
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					;
				}
				else
				{
					val = RxPayload & 0xFFF; //link number to be reset
					writeMemory(SYSCTLR_BASE_U+SYSCTLR_LINK_DISABLE, ( (0x1 << (0 + val) ) | (0x1 << (16 + val) ) ) );  //tx and rx link bits in the register
					sendAckP2PPacket(sourceAddress, P2P_COMM_DISABLE_LINK);
				}
				break;
			case P2P_COMM_DATA_FOR_HOST:		//  data for the Host as a response to its request
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					sendAckP2PPacket(sourceAddress, P2P_COMM_DATA_FOR_HOST);
				}
				else
				{
					;
				}
				break;
			case P2P_COMM_STATUS_OK:    //p2p carrying status of a chip
				if(isRefChip) //if it is a ref chip i.e. host connected and asking for status
				{
					//it is a response to nn packet asking for chip's status
					unsigned int index=0, offset = 0;
					RxRoutingKey=((RxRoutingKey >> 16) & 0xFFFF);  //getting the source address
					index = RxRoutingKey /32;
					offset = RxRoutingKey % 32;
					offset = 0x1 << offset;
					statusBuffer[index] |= offset;  //setting the corresponding bit high
		
				}
				else
				{
					;    //ignore
				}
				break;
			case P2P_COMM_ACK:					//  disable your link in bits 11-0
				p2pAckDue=0;
				break;
			case P2P_COMM_DATA_PACKET:	 //part of P2P_COMM_GET_DATA
				if(!p2pMemAddressReceived)
				{
					if((sourceAddress == p2pCurrentSrcChipAddress) && (p2pSeqNo == 1) )
					{
						p2pCurrentMemAddress=RxPayload;
						p2pMemAddressReceived=1;
					}
				}
				else if(p2pSeqNo == (currentP2PSeqNo+1)) //it is the data to be stored into the memory
				{
					currentP2PSeqNo++;
					writeMemory(p2pCurrentMemAddress, RxPayload);
					p2pDataTfrSize -=4;
					if(p2pDataTfrSize > 0)  //end of transfer
						p2pCurrentMemAddress +=4;
				}
				else
				{
					sendP2PErrorPacket();
		
				}
				sendAckP2PPacket(sourceAddress, P2P_COMM_GET_DATA);
			default:
				break;
		}
	}
	else  //it is a data p2p packet followed by an instruction, 
	{
  		//see if the source address received is same as p2pInProcessSource and sequenceNo is the next expected
		//then first see if it is the packet containing the memory address to get the data from
		//if so, store the address for future use in p2pInProcessMemAddress
		//next data containig packets to be stored in the next memory location and ack is sent
		if(p2pAckPending==1 && (p2pInAddress == ((RxRoutingKey >> 16) & 0xFFFF))) //it is a response to my request to send status
		{
			chipStatusRecieved=RxPayload;
			p2pAckPending=0;
		}
		else  //it is some other data
		{
			;
		}
	}
}
/*******************************************************************
 * 
 * 
 ******************************************************************/	
void handleMCPacket()
{
	if(procType==1)  //it is a mon proc
	{
		if(hostConnectedChip==1) //if it is monitor processor and connected to the Host PC
		{
			//keep saving into the responseOut buffer along with the timestamp and set the flag to send it out as response after every response interval
			//responseOutDue=1;
			responseOutReceived++;
			responseOutBase[currentResponseIndex++]=currentTime;  //timestamp of receiving response
			responseOutBase[currentResponseIndex++]=RxRoutingKey;
		}
		else //it is message passing thing
		{
			;
		}
	}
	else
	; //do nothing, leave it to the application
	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/	
void handleUNDEFPacket()
{
	
	
	
}

/*******************************************************************
 * 
 * 
 ******************************************************************/	
void wait(int time)
{
	int endTime;
	#ifdef VERBOSE
		printf("proc %d: watiting for %d ms\n", procID, time);
	#endif
	endTime=currentTime+time;
	while(currentTime < endTime)
	;                                   //waiting here for time ms
	
}
void waitCycles(int cycles)
{
	while(cycles)
	{
		cycles--;
	}
}

	
#endif /*SPINNAKERLIB_H_*/
