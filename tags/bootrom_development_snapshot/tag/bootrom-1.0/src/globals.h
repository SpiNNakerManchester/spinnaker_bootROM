#ifndef GLOBALS_
#define GLOBALS_

//behavioral definitions
//*******************  VERY IMPORTANT************************************************//
//uncomment the directives VERBOSE AND TLM_MODEL to enable the printf messages and 
//compiling for RTL TLM model respectively
//when uncommenting the TLM_MODEL, please also uncomment IMPORT TLM_MODEL from 	dmac.s, proc.s, and intService.s

//#define   VERBOSE
#define   TLM_MODEL 

#ifdef VERBOSE
	#include <stdio.h>
#endif
//#define   RESET_ON_WATCHDOG_INT
//#endif

/************************************************************************************************************
 *						global variables in 1K variable space of DTCM
 ************************************************************************************************************/

typedef enum {false=0, true} bool;	
typedef enum {fascicle=0, monitor} monFasc;
typedef enum {E=0, NE, N, W, SW, S, BROADCAST, LOCAL} routeType;
typedef enum {NORMAL=0, DIRECT} NNPacketType;
typedef enum {MC=0, P2P, NN, UNDEF} packetType;

#define ITCM_BASE		0x00000000
#define	ITCM_LIMIT		0x00007FFF
#define	DTCM_BASE		0x00400000
#define	DTCM_LIMIT		0x0040FFFF
#define	SYSTEM_RAM_BASE		0xF5000000
#define	SDRAM_BASE		0x70000000
#define	BOOT_ROM_BASE		0xFFFF0000
#define	NO_OF_MC_TABLE_ENTRIES	256			//1 K space
#define MC			0x0
#define P2P			0x1
#define	NN			0x2
#define	UNDEF_TYPE		0x3
#define FASCICLE_PROC		0x0
#define	MON_PROC		0x1
#define	SDRAM_SIZE		0x8000000

#ifdef 	TLM_MODEL
	#define  SYSTEM_RAM_SIZE		0x4000
#else
	#define  SYSTEM_RAM_SIZE		0x8000
#endif

#define 	SYS_RAM_VAR_SPACE		(SYSTEM_RAM_BASE+SYSTEM_RAM_SIZE-1024)    //saving 1K at the end for variables

#define 	SDRAM_VAR_SPACE			(SDRAM_BASE+SDRAM_SIZE-1024)				//saving 1K at the end for variables

#define		SYS_RAM_MSG_LOCK		0x20
#define     SYS_RAM_MSG_BUF			0x100       //the start address of the procs mail boxes
#define 	SYS_RAM_MSG_BUF_SIZE	0x100
#define     P2PRAMSIZE				256          //2K enteries of 3 bits each i.e. 2048/8 = 256

// !! ST removed defn. below - not used?
// #define     CHIP_ADDRESS_SPACE		SYSTEM_RAM_BASE + SYS_RAM_VAR_SPACE

//#define		VAR_SPACE				0x0
#define		DTCM_BUF_SIZE			0x100
#define 	DTCM_RXFRAME1_BASE		0x1000
#define		DTCM_RXFRAME2_BASE		0x2000
#define		DTCM_RXFRAME3_BASE		0x3000
#define		DTCM_TXFRAME_BASE		0x4000
#define		DTCM_STATUSBUFFER_BASE	0x5000
#define     NNOP_STARTING_ADDRESS	0x0F800000
#define     NNOP_END_ADDRESS		0x0FFFFFFF
//ehternet communication states and instructions
#define     WAIT_FOR_HELLO			51
#define 	FLOOD_FILL				52
#define		SEND_ACK				53
#define		SEND_SYSTEM_SIZE		54
#define		BREAK_SYMMETRY			55
//#define		REPORT_OK			56
#define		P2P_IN_COMM				57
#define		P2P_OUT_COMM			58
#define		START_APPLICATION		59
#define		STORE_BLOCK_TO_MEMORY	60
#define 	ACK						61
#define		SEND_ACK_FRAME			62
#define		SEND_STATUS_REPORT		63
#define    	IDLE					64
#define 	HELLO					65       //hello configuration done
#define 	REPORT_STATUS			66
#define		WAIT_FOR_ACK			67
#define 	ERR						68	   //error message
#define 	END_FLOOD_FILL			69
#define     NN_DIAG					70
#define		CRC_FAILED_RESEND_DATA  71
#define		FF_IN_PROCESS			72
#define     STIMULUS_IN				73	//stimulus spike for the neurons in a processor in the destination chip, 
										//(once sent by the Ethernet, the instr in instruction field with the size of data in option2,
#define     RESPONSE_OUT			74	// "               "
#define     LOAD_MON_APPLICATION    75
#define     LOAD_APPLICATION_DATA	76
#define     REQUEST_NEIGHBOUR_FOR_MISSING_WORDS 77
#define     REQUEST_NEIGHBOUR_TO_RESEND_BLOCK 	78
//shared memory instructions to be used for a meaningful communication between the on-chip processors
//instruction to contain 0x0F8 in the bits 31-20 to make sure it is not a fake instruction
#define		SM_SEND_DATA			 		81
#define		SM_RECEIVE_DATA			 		82
#define		SM_LOAD_APPLICATION		 		83 //the source address in first word after the instruction, the destination address in the second word after the instruction, the address to move PC in the third wrod after the instruction
#define     SM_LOAD_APPLICATION_DATA 		84

//flood fill states	
#define		FF_BLOCK_START					91
#define 	FF_BLOCK_DATA					92
#define		FF_BLOCK_CRC					93
#define		FF_BLOCK_END					94
//nn diag states
#define		NNDIAG_SEND_HELLO_FOR_RESPONSE	101
#define		NNDIAG_ENQUIRE_FROM_NEIGHBOUR 	102
#define		NNDIAG_DECIDE_LINK_OR_CHIP	    103
#define		NNDIAG_DEAD_CHIP_WHO_TO_CURE	104
#define		NNDIAG_DEAD_CHIP_DIAG			105
#define		NNDIAG_BOOTROM_TOGGLE			106
#define		NNDIAG_MON_PROC_CHANGE			107
#define		NNDIAG_WAIT_FOR_DIAG			108
#define		NNDIAG_MARK_CHIPS_LINKS_STATE	109
#define		NNDIAG_RESET_DEAD_CHIP			110
#define		NNDIAG_RESET_DEAD_CHIP_PROCS	111
#define     NNDIAG_DEAD_CHIP_RECOVERY     	112
#define     NNDIAG_CHECK_LINKS				113
#define		DEADCHIPDIAG_READ_SYSCTRL		121
#define		DEADCHIPDIAG_READ_BOOTROM		122
//	nn packet instruction format:
//the address will start from NNOP_STARTING_ADDRESS (0x0F800000) and limited to NNOP_END_ADDRESS (0x0FFFFFFF) as defined in spinnaker datasheet address map
//first 12(LSB) bits will be used for block size, remaining 11 bits will be used for instruction
//2K instructions can be created with this but we are starting from 1 and may go up to 256 instructions 
//i.e. leaving a cushion of 3 bits
//application loading (flood-fill) instructions		 	
#define    NN_FF_START				 	211	//instruction 211: i.e. routingkey.range(19, 12)=501  means sending broadcast message with block size given in routingkey.range(11, 0) and starting address in the payload
#define    NN_FF_END				 	212	//instruction 212: i.e. routingkey.range(19, 12)=502  means end of current broadcast message, crc in the payload (in case of standard 32 bit crc)
#define    NN_FF_INTERRUPT  		 	213	//instruction 213:										means interrupt current broadcast
#define    NN_FF_RESUME				 	214	//instruction 214:								               resume broadcast
#define	   NN_FF_CRC				 	215	//			  215:									   		   block level checksum, size in the routingkey.range(11, 0), checksum in payload (in case of crc more than 32 bits)
#define	   NN_FF_REMAINING_CRC		 	216	//			  216:									   		   remaining of checksum (if more than 32 bits), sequence number in routingkey.range(11, 0)
#define	   NN_FF_RESEND_BLOCK		 	217    //block start address in the payload
#define	   NN_FF_RESEND_WORD		 	218	//word start address in the payload
//nn configuration instruction
#define    NN_CONF_SYS_SIZE			 	221	//			  221:									   		   size of system in payload
#define    NN_CONF_USE_TIME_PHASE	 	222	//			  222:									   		   use the granularity (number of milliseconds) to be used for counting the timephase for router
#define    NN_CONF_RESET_TIME_PHASE	 	223	//			  222:									   		   reset the time phase to 0
#define    NN_CONF_BREAK_SYMMETRY    	224	//			  223:									   		   break the symmetery, assign address to yourself, the size of system in the payload
#define    NN_CONF_SEND_OK_REPORT	 	225	//			  224:									   		   request to send ok report to the Host Connected chip
#define    NN_CONF_REPORT_ALL_FINE	 	226	//			  224:									   		   ok report to the Host Connected chip
#define    NN_CONF_REPORT_WITH_STATUS 	227	//		  224:									   		   request to send ok report to the Host Connected chip
#define    NN_CONF_BOOT_INTRS_DONE	 	228	//			  225:									   		   extra boot instructions loading completed, start running kernel instructions
#define    NN_CONF_APP_LOAD_FROM	 	229	//			  226:									   		   application download completed, load it into the into the ITCM and start executing, location of application in the sdram is in the payload of this packet
#define    NN_CONF_APP_LOAD_TO		 	230	//			  226:									   		   application download completed, load it into the into the ITCM and start executing, location of application in the sdram is in the payload of this packet
#define    NN_CONF_APP_LOAD_SIZE	 	231	//		//the size of the application to be loaded into the ITCM
#define    NN_CONF_APP_LOAD_START	 	232	//		location of main procedure in the application to hand over the control to the application 
#define    NN_CONF_MON_APP_LOAD_FROM 	233         
#define    NN_CONF_MON_APP_LOAD_TO   	234    // 
#define    NN_CONF_MON_APP_SIZE		 	235         
#define    NN_CONF_MON_APP_START	 	236    // 
#define    NN_CONF_LOAD_APP_DATA	 	237

//nn diagnostic instructions
#define    NN_DIAG_HELLO			 	241	//            231:									   		   a neighbor to other, hello i am live  (no payload)
#define    NN_DIAG_HOW_R_U			 	242	//			  232:									   	       a neighbor to other, how are you (expect a response back i.e. a response with 'hello i am live' from that particular neighbor
#define    NN_DIAG_HAVE_U_HEARD_OUR_N	243 //			  233:									   		   have you heard from my neighbor making triangle with us (link on one less the number of faulty link as in emergency routing) in routingkey.range(11, 0), no payload
#define    NN_DIAG_YES_I_HEARD		 	244	//			  234:									   		   a neighbor to other, yes i have heard from our common neighbor 
#define    NN_DIAG_NO_I_DIDNOT		 	245	//			  235:									   		   a neighbor to otehr, no i have not heard from this neighbor
#define    NN_DIAG_ASK_N_TO_RESET_LINKS 246 //			  236:									   		   a neighbor to other, ask the common neighbor to reset the link towards me and resend hello message while i am doing this 

//point to point instructions
#define    P2P_COMM_REPORT_STATUS		21  //	send the cpu_ok register in the system controller
#define    P2P_COMM_GET_DATA			22  //  receive the data to be copied to memory, size of data in the bits 11-0, next packet to carry the starting address of the block to be copied
#define    P2P_COMM_SEND_DATA			23  //  send the data from the memory, size of data in the bits 11-0, next packet to carry the starting address of the block to be copied
#define    P2P_COMM_DIAG_CHIP			24  //  be nurse chip to repair chip on your port in bits 11-0
#define    P2P_COMM_RESET_CHIP			25	//  reset chip on you port in bits 11-0
#define    P2P_COMM_DISABLE_CHIP		26	//  disable the processors on chip on you port in bits 11-0
#define    P2P_COMM_DISABLE_NN_PROC		27	//  disable the processors nubmer in bits 11-8 on chip on you port in bits 7-0
#define    P2P_COMM_RESET_LINK			28	//  reset your link in bits 11-0
#define    P2P_COMM_DISABLE_LINK		29	//  disable your link in bits 11-0
#define    P2P_COMM_DATA_FOR_HOST		30	//  data for the Host as a response to its request
#define    P2P_COMM_ACK					31	//  disable your link in bits 11-0
#define    P2P_LOAD_APPLICATION			32  //  load application, to support hetrogenous applications on various chips
#define    P2P_LOAD_APPLICATION_PROCIDS 33
#define    P2P_LOAD_APPLICATION_ADDRESS 34
#define    P2P_COMM_STATUS_OK			35	//  p2p packet containing status report
#define    P2P_COMM_DATA_PACKET			36	//  p2p packet containing data

//P2P STATES
//#define    P2P_IDLE					41
//#define    P2P_GET_DATA				42			
//#define    P2P_SEND_DATA				43
//#define    P2P_DIAG_CHIP				44
//#define    P2P_RESET_CHIP				45
//#define    P2P_DISABLE_CHIP			46
//#define    P2P_DATA_FOR_HOST			49


#define		WAIT_BET_FF_PACKETS		 20
#define		WAIT_FOR_REPORTSTATUSOUT 10   //10 ms
//error messages
#define		ERR_FRAME_CHECKSUM_MSG	1001
#define		ERR_REQUEST_TIMEOUT		1002
#define		ERR_INVALID_OPTION1		1003
#define		ERR_INVALID_OPTION2		1004
#define		ERR_INVALID_OPTION3		1005

//#define 	IRQCT_BASE	 	 		0x0A801000
//#define 	IRQCT_ENABLESET	 		0x08
//#define 	IRQCT_ENABLECLR	 		0x0c
#define 	SR_CHIP_ADDRESS			0x00
#define 	SR_NCHIPS_LINK_STATE	0x04                  //bits 5-0 nn chips state, bits 21-16 nn links state
#define     SR_CONF_DONE			0x08
#define     SR_HOST_CONNECTED_CHIP	0x0C                  //hostConnectedChip=0;  //to identify if the chip is connected to the Host, to be kept in system ram
#define     SR_IS_REF_CHIP			0x10				 // isRefChip=0;
//dma related stuff
#define 	WRITE 1
#define 	READ  0
#define 	SWORD 0
#define 	DBWORD 1
#define 	CRCON 1
#define 	CRC0FF 0
#define 	P2P_WAIT_CYCLES  			 500  //in number of cycles
#define 	MISSING_WORD_TIMEOUT_CYCLES  200  //in number of cycles
#define 	RESPONSE_OUT_INTERVAL		 10   //in ms
#define 	STATUS_OUT_INTERVAL			 15   //in ms
int crc=0;  //by default crc in dma ctlr is disabled
int privilage=0; //by defautl the access to dma is in non privillaged mode
int transferid=0; //by default the transferid=0
int width=SWORD;  //by default the word size is single word to support on all memories, only SDRAM supports double word
int burst=4;      //by default the burst is set to the max to make it the most efficient


/********************************************************************************
 * global variables
 * ******************************************************************************/
//bool		 testbool;
//bool		 addressMapped=false;
#ifdef TLM_MODEL
	unsigned int TLMMODEL=1;      //to support model based compilation in assembly
#endif

// !! ST
// unsigned int globalVarSpace=SYSTEM_RAM_BASE+SYS_RAM_VAR_SPACE;
unsigned int globalVarSpace = SYS_RAM_VAR_SPACE;

unsigned int procID=0;
unsigned int procType=0;
unsigned int chipID=0;			//local copy of the chipID in DTCM, actual at sysRam[SR_CHIPID]
unsigned int opMode=0;
unsigned int hostConnectedChip=0;  //to identify if the chip is connected to the Host, to be kept in system ram
unsigned int isRefChip=0;
unsigned int sendStatusReportInstrID=0;
unsigned int resetTimePhaseInstrID=0;
unsigned int symmetryBrokenInstrID=0;
unsigned int systemSizeInstrID=0;
unsigned int appLoadStartInstrID=0;
unsigned int monAppLoadStartInstrID=0;
unsigned int loadAppDataInstrID=0;
unsigned int floodfillEndInstrID=0;
unsigned int appLoadFromInstrID=0;
unsigned int appLoadToInstrID=0;
unsigned int appLoadSizeInstrID=0;
unsigned int monAppLoadFromInstrID=0;
unsigned int monAppLoadToInstrID=0;
unsigned int monAppSizeInstrID=0;


unsigned int flood_fill_state=0;
unsigned int floofillInProcess=0;
unsigned int frameSent=0;
unsigned int helloTime=0;
unsigned int previousState=0;
unsigned int dmaInProcess=0;
unsigned int NNDiagDone=0;
unsigned int readCPUOKDone=0;
unsigned int linksResetDone=0;
unsigned int readMonProcIDDone=0;
unsigned int usedma=0;    //flag for the shared memory to use the dma or not for the transfer of data
unsigned int responseOutDue=0;
unsigned int statusOutDue=0;
unsigned int moreStatusReportsDue=0;
unsigned int incomingPacketPort=0;
//unsigned int routerDiagDone=0;
unsigned int deadChipCPUOKValue=0;
unsigned int deadChipMonProcValue=0;
unsigned int BootRomCodeLoaded=0;
unsigned int deadChipBootRomError=0;
unsigned int deadChipMonProcError=0;
unsigned int nndiagstate=0;
unsigned int deadchipDiagState=0;
unsigned int NNDiagTime=0;
unsigned int dmaError=0;
unsigned int nnAckDueRoute=0;   		//the port number to help the application if it is expecting an ack on a specific port
unsigned int nnAckDue=0;				//the nn ack flag to help the application if it is expecting an ack 
unsigned int p2pAckDue=0;	
unsigned int p2pMemAddressReceived=0;	
unsigned int p2pProcsForLoadingApplication=0;
unsigned int p2pAddressForLoadingApplication=0;			
unsigned int dmaErrorstatus=0;
unsigned int dmaTimeout=0;
unsigned int parityError=0;
unsigned int frammingError=0;
unsigned int txEmpty=0;
unsigned int txFull=0;
unsigned int txOverrun=0;
unsigned int rtrErrorCount=0;
unsigned int frameInProcess=0;
unsigned int rtrParityError=0;
unsigned int rtrTimephaseError=0;
unsigned int rtrFrammingError=0;
unsigned int rtrUndefPacketError=0;
unsigned int rtrTimePhaseErrorCount=0;
unsigned int rtrErrorOverflow=0;
unsigned int rtrTxLinkStatus=0;
unsigned int rtrDumpOverflow=0;
unsigned int enableEthernet=0;
unsigned int EthLinkUp=0;	
unsigned int ethInstr=0;	        //instruction received with ethernet frame
unsigned int rxCurrentBlock=0;
unsigned int spinnakerSystemSize=0;
unsigned int statusReportDue=0;
unsigned int reportStatusTime=0;
unsigned int chipAddress=0;
unsigned int systemTimePhase=0;
unsigned int ackFrameDue=0;
unsigned int error_code=0;
unsigned int statusReportIndex=0;
unsigned int configDone=0;		//local copy of sysRam[SR_CONF_DONE]
unsigned int testResult=0;
unsigned int nChipsLinksState=0;        //local copy of sysRam[SR_NCHIPS_LINK_STATE], bits(5-0) nn chips state, bits(13-8) tx links state, bits(21-16) rx proc state, bits(29-24) mon proc state
unsigned int nChipsLinksMarkState=0;
unsigned int RxStatus=0;
unsigned int RxPayload=0;
unsigned int RxRoutingKey=0; 
unsigned int RxPacketType=0;
unsigned int currentTime=0;
unsigned int terminateTime=0;
unsigned int coreChipStatus=0;
unsigned int txFramePending=0;
unsigned int DtcmDMABuffer=0; 
unsigned int rxFrame2Base=0;
unsigned int rxFrame3Base=0;
unsigned int applicationSize=0; 
unsigned int applicationSourceAddress=0;
unsigned int applicationDestAddress=0;
unsigned int applicationMainAddress=0;
unsigned int currentP2PSeqNo=0;
unsigned int currentResponseIndex=0;
unsigned int responseOutReceived=0;
unsigned int noOfResponsesReceived=0;
unsigned int p2pCurrentSrcChipAddress=0;
unsigned int chipStatusRecieved=0;
unsigned int p2pDataTfrSize=0;
unsigned int p2pCurrentMemAddress=0;
unsigned int sourceMACAddressLS=0x48000001;
unsigned int sourceMACAddressHS=0xACDE;
unsigned int destinationMACAddressLS=0xFFFFFFFF;   //broadcast     old=0x48000000;
unsigned int destinationMACAddressHS=0xFFFF;       //broadcast 	   old=0xACDE;
unsigned int sourceIPAddress=0xC0A80002;	//192.168.0.2
unsigned int desinationIPAddress=0xC0A80001;	//192.168.0.1
unsigned int macDestAddressConfigured=0;
unsigned int rxBufStAddr=0;					//start address of the block received with frame i.e. where to store this block
unsigned int rxBufSize=0;						//block size received with frame i.e how large the buf would be
unsigned int rxFrameSize=0;					//block size received with frame i.e how large the buf would be
unsigned int spinnakerDataSize=0;
unsigned int rxDataBit=0;						//the starting bit of the data in the frame, in order to keep it soft for using any protocol on top of it
unsigned int rxBufCRC=0;						//CRC received with the block
												//spinnnaker instruction in instruction field bit 15-0
												//p2pInInstruction with the spinnaker instruction bit 31-16
												//chipaddress in option1 bit 15-0, port no or proc address etc in option1 bit 31-16
												//datasize in option2
												//memory address in option3
unsigned int missingRxBufCRC=0;					//CRC received with the block
unsigned int missingRxBufSize=0;				//block size received with frame i.e how large the buf would be
unsigned int missingRxBufStAddr=0;				//start address of the block received with frame i.e. where to store this block
unsigned int missingWordAddress=0;
unsigned int missingWordWaiting=0;												
unsigned int p2pInDataSize=0;
unsigned int p2pOutDataSize=0;
unsigned int p2pAckPending=0;
unsigned int p2pOutInstruction=0;
unsigned int p2pInInstruction=0;
unsigned int p2pOutAddress=0;
unsigned int p2pInAddress=0;
//start flood fill, the data block 1K at the moment, is in the rxFrameBase. rxFrameBase[16/4] contains the size of the block, 
//rxFrameBase[20/4] contains the physical address to copy the block to, and rxFrameBase[24/4] contains the crc of the block, the data starts from rxFrameBase[24/4]
//first do the crc check on the block and if ok carry on with the floodfill, or send a frame asking for the block again
//if a frame comes during the floodfill of already received frame, it will be discarded and reported to the Host to wait
//for a while unless a message to abort comes from the Host
#define dataLengthAt				42  //byte 12-13 in the frame data
#define spinnakerInstructionAt		44  //byte 14-15
#define instructionOption1At		48  //byte 16-19
#define instructionOption2At		52  //byte 20-23
#define instructionOption3At	   	56  //byte 24-27			
#define spinnakerDataAt				60
//unsigned int rxBufCRCSize=0;					//CRC size 32 by default
unsigned int rtrDiagCount[8];				//diag counts for the router diag count registers
unsigned int rxBitmap[47];  				//to take 1500 bits in each one
unsigned int missingRxBitmap[47];  				//to take 1500 bits in each one
unsigned int rxBuf[380];   					//to take 1500 bytes for the data to be sent of receive as nn floodfill by all the chips
unsigned int missingRxBuf[380];   					//to take 1500 bytes for the data to be sent of receive as nn floodfill by all the chips
unsigned int rxFrameBase[380];  		    //to hold 1500 bytes for the frame to receive
unsigned int txFrameBase[380];				//to hold the frame before sending it
//unsigned int tempBuffer[380];
unsigned int p2pInDataBase[256];
unsigned int p2pOutDataBase[256];
unsigned int responseOutBase[256];
volatile unsigned int *statusBuffer = (volatile unsigned *)(DTCM_BASE+0x8000);               //it will hold the bitmap of the status of the chips, may to to 65536/32 = 2048*4=8192bytes				
										  //the size varies dynamically as per the size of the system
										  //the status received from the chips is stored here before sending
										  
volatile unsigned *sysRam			= (volatile unsigned *)(SYSTEM_RAM_BASE);
volatile unsigned *SDRam			= (volatile unsigned *)(SDRAM_BASE);
volatile unsigned *dtcmBase     	= (volatile unsigned *)(DTCM_BASE);
/*************************************************************************************
 *                   constants from peripherals.s
 * ***********************************************************************************/ 

//////////////////////////////////////////////////////////////////////////////////////////////
// ProcID Register
/////////////////////////////////////////////////////////////////////////////////////////////
#define	PROCID_REGISTER						0x40000000
#define	CHIPID_REGISTER						0xF200003C
// Timer Registers
//////////////////////////////////////////////////////////////////////////////////////////////
#define	TIMER_BASE						 	0x11000000
#define	TIMER1Load						 	0x000			//value from which the counter is to decrement. This is the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero
#define	TIMER1Value							0x004			//gives the current value of the decrementing counter
#define	TIMER1Control					    0x008	//Control Register bits are defined as 
													//[31:8] - - Reserved bits, do not modify, and ignore on read
											 		//[7] TimerEn Read/write Enable bit: 0 = Timer module disabled (default), 1 = Timer module enabled.
											 		//[6] TimerMode Read/write Mode bit: 0 = Timer module is in free-running mode (default), 1 = Timer module is in periodic mode, 
											 		//[5] IntEnable Read/write Interrupt Enable bit: 0 = Timer module Interrupt disabled, 1 = Timer module Interrupt enabled (default).
											 		//[3:2] TimerPre Read/write Prescale bits:	00 = clock is divided by 1 (default), 01=clock is divided by 16, 
											 		//               10 =clock is divided by 256, 11 = Undefined, do not use.
											 		//[1] TimerSize Selects 16/32 bit counter operation: 0 = 16-bit counter (default), 1 = 32-bit counter.
											 		//[0] OneShot Selects one-shot or wrapping counter mode: 0 = wrapping mode (default), 1 = one-shot mode.
#define	TIMER1IntClr				        0x00C			 //Any write to this register, clears the interrupt output from the counter.
#define	TIMER1RIS							0x010			 //raw interrupt status in bit 0
#define	TIMER1MIS							0x014			 //Enabled interrupt status from the counter
#define	TIMER1BGLoad						0x018			 //contains the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero

//////////////////////////////////////TIMER 2//////////////////////////////////////////////////////////////////
#define	TIMER2Load						 	0x020			//value from which the counter is to decrement. This is the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero
#define	TIMER2Value							0x024			//gives the current value of the decrementing counter
#define	TIMER2Control						0x028			 //Control Register bits are defined as 
													 //[31:8] - - Reserved bits, do not modify, and ignore on read
											 		//[7] TimerEn Read/write Enable bit: 0 = Timer module disabled (default), 1 = Timer module enabled.
											 		//[6] TimerMode Read/write Mode bit: 0 = Timer module is in free-running mode (default), 1 = Timer module is in periodic mode, 
											 		//[5] IntEnable Read/write Interrupt Enable bit: 0 = Timer module Interrupt disabled, 1 = Timer module Interrupt enabled (default).
											 		//[3:2] TimerPre Read/write Prescale bits:	00 = clock is divided by 1 (default), 01=clock is divided by 16, 
											 		//               10 =clock is divided by 256, 11 = Undefined, do not use.
											 		//[1] TimerSize Selects 16/32 bit counter operation: 0 = 16-bit counter (default), 1 = 32-bit counter.
											 		//[0] OneShot Selects one-shot or wrapping counter mode: 0 = wrapping mode (default), 1 = one-shot mode.
#define	TIMER2IntClr				        0x02C			 //Any write to this register, clears the interrupt output from the counter.
#define	TIMER2RIS							0x030			 //raw interrupt status in bit 0
#define	TIMER2MIS							0x034			 //Enabled interrupt status from the counter
#define	TIMER2BGLoad						0x038			 //contains the value used to reload the counter when Periodic mode is enabled, and the current count reaches zero
//////////////////////////////////////TIMER TEST REGISTERS////////////////////////////////////////
#define	TIMERITCR 							0xF00 			//Read/write Integration Test Control Register,
#define	TIMERITOP					 	    0xF04 			 //Write Integration Test Control Register


#define	TIMER_EN_BIT						(0x1<<7)	     //1-enable// 0-disable
#define	TIMER_MODE_BIT						(0x1<<6)		 //1-periodic mode// 0-free-running
#define	TIMER_INT_EN_BIT			        (0x1<<5)	     //Interrupt Enable 
													 //0 = Timer module Interrupt disabled
													 //1 = Timer module Interrupt enabled (default).
#define	TIMER_PRESCALE						(0x1<<2)		 //clock divided by 16

//////divider 16, prescaler 4, period mode, timer start 
////////0b11000100
#define	TIMER1_OPT_CONTROL					(TIMER_EN_BIT|TIMER_MODE_BIT|TIMER_INT_EN_BIT|TIMER_PRESCALE) //contol bits to timer1

//////Timer1 default interrupt control source bit is 4 and clk is 100MHz 
//////value load to timer1 for 1ms timer int
#define	TIMER1_OPT_VALUE					0x186A   	
	 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PL190 VIC Registers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define	VIC_BASE	 						0xFFFFF000
#define	VICIRQSTATUS 						0x000 //RO Shows the status of the interrupts after masking by the VICINTENABLE and VICINTSELECT
#define	VICFIQSTATUS 						0x004 //RO Shows the status of the interrupts after masking by the VICINTENABLE and VICINTSELECT
#define	VICRAWINTR 							0x008 //RO Shows the status of the interrupts before masking by the enable registers
#define	VICINTSELECT 						0x00C //R/W Selects the type of interrupt for interrupt rests: 1 = FIQ interrupt 0 = IRQ interrupt.
#define	VICINTENABLE 						0x010 //R/W Enables the interrupt rest lines: 1 = Interrupt enabled 0 = Interrupt disabled
#define	VICINTENCLEAR 						0x014 //WO Write Clears bits in the VICINTENABLE Register. A HIGH bit clears the corresponding bit in the VICINTENABLE Register. A LOW bit has no effect
#define	VICSOFTINT 							0x018 //R/W Setting a bit generates a software interrupt for the specific source interrupt before interrupt masking
#define	VICSOFTINTCLEAR 					0x01C //WO Clears bits in the VICSOFTINT Register. A HIGH bit clears the corresponding bit in the VICSOFTINT Register. A LOW bit has no effect
#define	VICPROTECTION 						0x020 //R/W Enables or disables protected register access. When enabled, only privileged mode accesses, reads and writes, can access the interrupt controller registers
#define	VICVECTADDR 						0x030 //R/W Contains the address of the currently active ISR. Any writes to this register clear the interrupt.
#define	VICDEFVECTADDR 						0x034 //R/W Contains the address of the default ISR handler
#define	VICVECTADDR0 						0x100 //R/W Contains ISR vector addresses for device at int line 0

#define	VICVECTCNTL0 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL1 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL2 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL3 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL4 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL5 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL6 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL7 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL8 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL9 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL10 						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL11						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL12						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL13						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL14						0x200 //R/W selects the interrupt source 0 for the vectored interrupt
#define	VICVECTCNTL15						0x200 //R/W selects the interrupt source 0 for the vectored interrupt

#define	VICPERIPHID0 						0xFE0 //RO 0x90
#define	VICPERIPHID1 						0xFE4 //RO 0x11 See Peripheral Identification Registers on page 3-11
#define	VICPERIPHID2 						0xFE8 //RO 0x04
#define	VICPERIPHID3 						0xFEC //RO 0x00
#define	VICPCELLID0 						0xFF0 //RO 0x0D See PrimeCell Identification Registers on page 3-14
#define	VICPCELLID1 						0xFF4 //RO 0xF0
#define	VICPCELLID2 						0xFF8 //RO 0x05
#define	VICPCELLID3 						0xFFC //RO 0xB1
#define	VIC_VECTADDR_OFFSET					0xFF0 //used in vectors.s
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TLM DMAC registers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define	TLM_DMAC_BASE						  0x30000000
#define	TLM_DMAC_CRCP						  0x00 //R/W DMA address on the system interface
#define	TLM_DMAC_ADRS						  0x04 //R/W DMA address on the system interface
#define	TLM_DMAC_ADRT						  0x08 //R/W DMA address on the TCM interface
#define	TLM_DMAC_DESC						  0x0C //R/W Length of the transfer in bytes
#define	TLM_DMAC_CTRL						  0x10 //R/W Control DMA transfer
#define	TLM_DMAC_STAT						  0x14 //R Status of DMA and other transfers
#define	TLM_DMAC_GCTL						  0x18 //R/W Control of the DMA device
#define	TLM_DMAC_CRCC						  0x1C //R CRC value calculated by CRC block
#define	TLM_DMAC_CRCR						  0x20 //R CRC value in received block
#define	TLM_DMAC_TMTV						  0x24 //R CRC value in received block
#define	TLM_DMAC_CRC2						  0x100 //R* Queued system address
#define	TLM_DMAC_AD2S						  0x104 //R* Queued TCM address
#define	TLM_DMAC_LN2T						  0x108 //R* Queued length
#define	TLM_DMAC_DES2						  0x10C //R* Queued control
#define     DMAC_DESC_DIR_BIT		19
#define     DMAC_DESC_CRC_BIT		20
#define     DMAC_DESC_BURST			21
#define     DMAC_DESC_WIDTH			24
#define     DMAC_DESC_PRIVILLEGE	25
#define     DMAC_DESC_TRFID			26
#define     DMAC_STATUS_TRF_INPROG	0
#define     DMAC_STATUS_TRF_PAUSED	1
#define     DMAC_STATUS_TRF_QUEUED	2
#define     DMAC_STATUS_WB_FULL		3
#define     DMAC_STATUS_WB_ACTIVE	4
#define     DMAC_STATUS_COND_CODE	10
#define     DMAC_STATUS_ENINT_DONE		10
#define     DMAC_STATUS_ENINT_DONE2		11
#define     DMAC_STATUS_ENINT_TOUT		12
#define     DMAC_STATUS_ENINT_CRCERROR	13
#define     DMAC_STATUS_ENINT_TCMERROR	14
#define     DMAC_STATUS_ENINT_AXIERROR	15
#define     DMAC_STATUS_ENINT_USERABORT	16
#define     DMAC_STATUS_ENINT_SOFTRST	17
#define     DMAC_STATUS_ENINT_WB		20
#define     DMAC_STATUS_PROCID		24
#define     DMAC_CTRL_UNCOMMIT		0
#define     DMAC_CTRL_ABORT			1
#define     DMAC_CTRL_RESTART		2
#define     DMAC_CTRL_CLINT_DONE	3
#define     DMAC_CTRL_CLINT_TIMEOUT	4
#define     DMAC_CTRL_CLINT_WB		5
#define     DMAC_GCTL_EN_BR_BUF		0
#define     DMAC_GCTL_ENINT_DONE	10
#define     DMAC_GCTL_ENINT_DONE2	11
#define     DMAC_GCTL_ENINT_TOUT	12
#define     DMAC_GCTL_ENINT_CRCERROR	13
#define     DMAC_GCTL_ENINT_TCMERROR	14
#define     DMAC_GCTL_ENINT_AXIERROR	15
#define     DMAC_GCTL_ENINT_USERABORT	16
#define     DMAC_GCTL_ENINT_SOFTRST		17
#define     DMAC_GCTL_ENINT_WB			20

///////////////////////////////////////////////////////////////////////////////////
// DMAC registers
////////////////////////////////////////////////////////////////////////////////////
#define	PROCID_REGISTER_TLM					0x30000010    //dmac status register

#define	DMAC_BASE							0x30000000
#define	DMACIntStatus						0x000
#define	DMACIntTCStatus						0x004
#define	DMACIntTCClear						0x008
#define	DMACIntErrorStatus					0x00C
#define	DMACIntErrClr						0x010
#define	DMACRawIntTCStatus					0x014
#define	DMACRawIntErrorStatus 				0x018
#define	DMACEnbldChns						0x01C
#define	DMACSoftBReq						0x020
#define	DMACSoftSReq						0x024
#define	DMACSoftLBReq						0x028
#define	DMACSoftLSReq						0x02C
#define	DMACConfiguration					0x030
#define	DMACSync							0x034
#define	DMACC0SrcAddr						0x100
#define	DMACC0DestAddr						0x104
#define	DMACC0LLI							0x108
#define	DMACC0Control						0x10C
#define	DMACC0Configuration 				0x110
///////////////////////////////////////////////////////////////////////////////////////
// Comm Ctlr registers
///////////////////////////////////////////////////////////////////////////////////////
#define	COMMCTLR_BASE						0x10000000
#define	COMMCTLR_R0							0x00	//Tx Control register
#define	COMMCTLR_R1							0x04	//Rx Control register
#define	COMMCTLR_R2							0x08	//Tx payload
#define	COMMCTLR_R3							0x0C	//Tx routing key
#define	COMMCTLR_R4							0x10	//Rx payload
#define	COMMCTLR_R5							0x14	//Rx routing key
#define	COMMCTLR_R6							0x18	//reserved
#define	COMMCTLR_R7							0x1C	//test control register

//Comm Ctlr Bits

#define	COMMCTLR_R1_DATA					(0x1<<17)   //payload
#define	COMMCTLR_R1_PKTTYPE_MC				(0x0<<22)	//packety type MC
#define	COMMCTLR_R1_PKTTYPE_P2P				(0x1<<22)
#define	COMMCTLR_R1_PKTTYPE_NN		     	(0x2<<22)
#define	COMMCTLR_R1_ROUTE			     	(0x7<<24)  //route value to be 7 and sourceid=0
#define	COMMCTLR_R0_CON						 0x16
#define	COMMCTLR_R0_ROUTE					 0x24
#define	COMMCTLR_R0_DATA					 0x17
#define	COMMCTLR_R0_TXOVERFLOW				 0x29
#define	COMMCTLR_R0_TXFULL					 0x30
#define	COMMCTLR_R0_TXEMPTY					 0x31


/////////////////////////////////////////////////////////////////////////////////////////////
// Router Registers
/////////////////////////////////////////////////////////////////////////////////////////////
#define	RTR_BASE							0xF1000000
#define	RTR_R0_CONTROL 						0x0 		//R/W Router control register
#define	RTR_R1_STATUS  						0x4 		//R Router status
#define	RTR_R2_ERROR_HEADER					0x8       	//error header  R error packet control byte and flags
#define	RTR_R3_ERROR_ROUTE			     	0xC       	//error routing R error packet routing word
#define	RTR_R4_ERROR_PL				    	0x10		//error payload  R error packet data payload
#define	RTR_R5_ERROR_STATUS			     	0x14      	//error status  R error packet status
#define	RTR_R6_DUMP_HEADER			     	0x18      	//dump header  R dumped packet control byte and flags
#define	RTR_R7_DUMP_ROUTE			     	0x1C      	//dump routing  R dumped packet routing word
#define	RTR_R8_DUMP_PL				     	0x20      	//dump payload  R dumped packet data payload
#define	RTR_R9_DUMP_OUTPUTS			     	0x24      	//dump outputs  R dumped packet intended destinations
#define	RTR_R10_DUMP_STATUS			     	0x28      	//dump status 0x28 R dumped packet status
#define	RTR_R11_DIAG_ENABLES		     	0x2C      	//diag enables 0x2C R/W diagnostic counter enables
#define	RTR_R2N_DIAG_CONTROLS		     	0x200     	//diag filter 0x200-21F R/W diagnostic count filters (N = 0-7)
#define	RTR_R3N_DIAG_COUNT			     	0x300     	//diag count 0x300-31F R/W diagnostic counters (N = 0-7)
#define	RTR_RT1						    	0xF00     	//test register  R/W hardware test register 1
#define	RTR_RT2 						 	0xF04     	//test key  R/W hardware test register 2 - CAM input test key
#define	RTR_ROUTE_TABLE				  	 	0x4000    	//[1023:0]  R/W MC Router routing word values
#define	RTR_KEY_TABLE				     	0x8000    	//[1023:0]  W MC Router key values
#define	RTR_MASK_TABLE				  	 	0xC000    	//[1023:0]  W MC Router mask values
#define	RTR_P2P_TABLE				     	0x10000   	//[65535:0]  R/W P2P Router routing entries



/////////////////////////////////////////////////////////////////////////////////////////////////////
// System Controller Registers
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define	SYSCTLR_BASE_U						0xF2000000
#define	SYSCTLR_BASE_B						0xE2000000
#define	SYSCTLR_TUBE 						0x00    //W
#define	SYSCTLR_CHIP_ID						0x00 	//R Chip ID register (hardwired)
#define	SYSCTLR_CPU_DISABLE					0x04 	//R/W Each bit disables the clock of a processor
#define	SYSCTLR_SET_CPU_IRQ					0x08 	//R/W Writing a 1 sets a processor’s interrupt line
#define	SYSCTLR_CLR_CPU_IRQ					0x0C 	//R/W Writing a 1 clears a processor’s interrupt line
#define	SYSCTLR_SET_CPU_OK					0x10 	//R/W Writing a 1 sets a CPU OK bit
#define	SYSCTLR_CLR_CPU_OK					0x14 	//R/W Writing a 1 clears a CPU OK bit
#define	SYSCTLR_CPU_RST_LV					0x18 	//R/W Level control of CPU resets
#define	SYSCTLR_NODE_RST_LV					0x1C 	//R/W Level control of CPU node resets
#define	SYSCTLR_SBSYS_RST_LV				0x20 	//R/W Level control of subsystem resets
#define	SYSCTLR_CPU_RST_PU					0x24 	//R/W Pulse control of CPU resets
#define	SYSCTLR_NODE_RST_PU					0x28 	//R/W Pulse control of CPU node resets
#define	SYSCTLR_SBSYS_RST_PU				0x2C 	//R/W Pulse control of subsystem resets
#define	SYSCTLR_RESET_CODE					0x30 	//R Indicates cause of last chip reset
#define	SYSCTLR_MONITOR_ID					0x34 	//R/W ID of Monitor Processor
#define	SYSCTLR_MISC_CONTROL				0x38 	//R/W Miscellaneous control bits
#define	SYSCTLR_MISC_STATUS					0x3C 	//R Miscellaneous status bits
#define	SYSCTLR_IO_PORT						0x40 	//R/W I/O pin output register
#define	SYSCTLR_IO_DIRECTION				0x44 	//R/W External I/O pin is input (1) or output (0)
#define	SYSCTLR_SET_IO						0x48 	//R/W Writing a 1 sets IO register bit
#define	SYSCTLR_CLEAR_IO					0x4C 	//R/W Writing a 1 clears IO register bit
#define	SYSCTLR_PLL1						0x50	//R/W PLL1 frency control
#define	SYSCTLR_PLL2						0x54 	//R/W PLL2 frency control
#define	SYSCTLR_TORIC0						0x58 	//R Toric frency synthesis control 0
#define	SYSCTLR_TORIC1						0x5C 	//R/W Toric frency synthesis control 1
#define	SYSCTLR_CLK_MUX_CTL					0x60 	//R/W Clock multiplexer controls
#define	SYSCTLR_CPU_SLEEP					0x64 	//R CPU sleep (awaiting interrupt) status
#define	SYSCTLR_ARBITER						0x80 	//to FC R Read sensitive semaphores to determine MP
#define	SYSCTLR_Test_Set 					0x100   //-17C R Test & Set registers for general software use
#define	SYSCTLR_Test_Clr 					0x180   //-1FC R Test & Clear registers for general software use
#define	SYSCTLR_MISC_TEST					0x200 	//R/W Miscellaneous chip test control bits
#define	SYSCTLR_LINK_DISABLE 				0x204 	//R/W Disables for Tx and Rx link interfaces//r33_Arbiter 0x80-FC R Read sensitive semaphores to determine MP

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Watchdog Timer Registers
////////////////////////////////////////////////////////////////////////////////////////////////////////
#define	WDOG_BASE							0xF3000000
#define	WDOG_LOAD 							0x0 	//R/W Count load register
#define	WDOG_VALUE 							0x4 	//R Current count value
#define	WDOG_CONTROL 						0x8 	//R/W Control register
#define	WDOG_INTCLR							0xC 	//W Interrupt clear register
#define	WDOG_RIS 							0x10 	//R Raw interrupt status register
#define	WDOG_MIS  							0x14 	//R Masked interrupt status register
#define	WDOG_LOCK  							0xC00 	//R/W Lock register
#define	WDOG_ITCR 							0xF00 	//R/W Watchdog integration test control register
#define	WDOG_ITOP  							0xF04 	//W Watchdog integration test output set register

#define WDOG_COUNTER_VALUE					20		//in ms


////////////////////////////////////////////////////////////////////////////////////////////////////////
// DMC PL340 Configuration Registers for configuration
////////////////////////////////////////////////////////////////////////////////////////////////////////
#define	SDRAM_BASE_B						0x60000000
#define	SDRAM_BASE							0x70000000
#define	PL340CFG_BASE						0xF0000000
#define	PL340CFG_STATUS  					0x0 //R memory controller status
#define	PL340CFG_COMMAND		 			0x4 //W PL340 command
#define	PL340CFG_DIRECT 		 			0x8 //W direct command
#define	PL340CFG_MEM_CFG 					0xC //R/W memory configuration
#define	PL340CFG_REFRESH_PRD 				0x10 //R/W refresh period
#define	PL340CFG_CAS_LATENCY 				0x14 //R/W CAS latency
#define	PL340CFG_T_DQSS 					0x18 //R/W write to DQS time
#define	PL340CFG_T_MRD 						0x1C //R/W mode register command time
#define	PL340CFG_T_RAS 						0x20 //R/W RAS to precharge delay
#define	PL340CFG_T_RC 						0x24 //R/W active bank x to active bank x delay
#define	PL340CFG_T_RCD 						0x28 //R/W RAS to CAS minimum delay
#define	PL340CFG_T_RFC 						0x2C //R/W auto-refresh command time
#define	PL340CFG_T_RP 						0x30 //R/W precharge to RAS delay
#define	PL340CFG_T_RRD 						0x34 //R/W active bank x to active bank y delay
#define	PL340CFG_T_WR 						0x38 //R/W write to precharge delay
#define	PL340CFG_T_WTR 						0x3C //R/W write to read delay
#define	PL340CFG_T_XP 						0x40 //R/W exit power-down command time
#define	PL340CFG_T_XSR 						0x44 //R/W exit self-refresh command time
#define	PL340CFG_T_ESR 						0x48 //R/W self-refresh command time
#define	PL340CFG_T_ID_N_CFG 				0x100 //R/W QoS settings
#define	PL340CFG_CHIP_N_CFG 				0x200 //R/W external memory device configuration
#define	PL340CFG_USER_STATUS 				0x300 //R state of user_status[7:0] primary inputs
#define	PL340CFG_USER_CONFIG 				0x304 //W sets the user_config[7:0] primary outputs
#define	PL340CFG_INT_CFG 					0xE00 //R/W integration configuration register
#define	PL340CFG_INT_INPUTS 				0xE04 //R integration inputs register
#define	PL340CFG_INT_OUTPUTS 				0xE08 //W integration outputs register

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Ethernet Interface
//////////////////////////////////////////////////////////////////////////////////////////////////////
#define	ETH_BASE							0xF4000000
#define	ETH_TX_FRAME_BUFFER 				0x0000 	//W Transmit frame RAM area
#define	ETH_RX_FRAME_BUFFER 				0x4000 	//R Receive frame RAM area
#define	ETH_RX_DESC_RAM 					0x8000 	//R Receive descriptor RAM area
#define	ETH_GEN_COMMAND 					0xC000 	//R/W General command
#define	ETH_GEN_STATUS 						0xC004 	//R General status
#define	ETH_TX_LENGTH 						0xC008 	//R/W Transmit frame length
#define	ETH_TX_COMMAND 						0xC00C 	//W Transmit command
#define	ETH_RX_COMMAND 						0xC010 	//W Receive command
#define	ETH_MAC_ADDR_LS  					0xC014 	//R/W MAC address low bytes
#define	ETH_MAC_ADDR_HS 					0xC018 	//R/W MAC address high bytes
#define	ETH_PHY_CONTROL 					0xC01C 	//R/W PHY control
#define	ETH_IRQ_CLEAR 						0xC020 	//W Interrupt clear
#define	ETH_RX_BUF_RD_PTR 					0xC024 	//R Receive frame buffer read pointer
#define	ETH_RX_BUF_WR_PTR 					0xC028 	//R Receive frame buffer write pointer
#define	ETH_RX_DSC_RD_PTR 					0xC02C 	//R Receive descriptor read pointer
#define	ETH_RX_DSC_WR_PTR 					0xC030 	//R Receive descriptor write pointer
#define	ETH_RX_SYS_STATE 					0xC034 	//R Receive system FSM state (debug & test use)
#define	ETH_TX_MII_STATU 					0xC038 	//R Transmit MII FSM state (debug & test use)
#define	ETH_PERIPH_ID 						0xC03C 	//R Peripheral ID (debug & test use)
#define TX_FRAME_BUFFER_SIZE   				1536
#define RX_FRAM_BUFFER_SIZE    				3072

//////////////////////////////////////////////////////////////////////////////////////////////////////
// PHY
//////////////////////////////////////////////////////////////////////////////////////////////////////

/* !! ST removed
#define	PHY_BASE						    0xF400C100   
#define	PHY_CONTROL							0x00
#define	PHY_STATUS							0x04
#define	PHY_ID						    	0x08
#define	PHY_INT_STATUS						0x0C
#define	PHY_INT_MASK						0x10
*/ 
 
#endif /*GLOBALS_*/
