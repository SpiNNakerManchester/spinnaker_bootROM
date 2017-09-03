#ifndef NNDIAGPROTOCOL_H_
#define NNDIAGPROTOCOL_H_

#include "globals.h"


/*******************************************************************
 * 
 * 
 ******************************************************************/
void checkInterchipLinks()
{
	//check the links to all the neighbouring chips by sending an NN dirct broadcast 
	//to read the value of cpu ok register and monitor processor id
	readCPUOKDone = 0;
	readMonProcIDDone = 0;
	NNDiagTime = currentTime;
	if((nChipsLinksState >> 8) == 0) //links have not been tested before
	{
		sendNNPacketWOPL(6, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));  //NN direct broadcast to read mon proc id
	}
	else
	{
		if(!((nChipsLinksState >> 8) & 1)) //tx0 or rx0 needs testing
		{
			sendNNPacketWOPL(0, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));  //NN direct broadcast to read mon proc id
		}
		else if(!((nChipsLinksState >> 9) & 1)) //tx1 or rx1 needs testing
		{
			sendNNPacketWOPL(1, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));  //NN direct broadcast to read mon proc id
		}
		else if(!((nChipsLinksState >>10) & 1)) //tx2 or rx2 needs testing
		{
			sendNNPacketWOPL(2, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));  //NN direct broadcast to read mon proc id
		}
		else if(!((nChipsLinksState >>11) & 1)) //tx3 or rx3 needs testing
		{
			sendNNPacketWOPL(3, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));  //NN direct broadcast to read mon proc id
		}
		else if(!((nChipsLinksState >>12) & 1)) //tx4 or rx4 needs testing
		{
			sendNNPacketWOPL(4, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));  //NN direct broadcast to read mon proc id
		}
		else if(!((nChipsLinksState >>13) & 1)) //tx5 or rx5 needs testing
		{
			sendNNPacketWOPL(5, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));  //NN direct broadcast to read mon proc id
		}
	}
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void resetLinks()
{
	unsigned int txlinkstate, rxlinkstate;
	unsigned int resetcode=0;
	#ifdef VERBOSE
		printf("proc %d: in resetLinks\n", procID);
	#endif
	txlinkstate=(nChipsLinksState >>8) & 0x3F;
	rxlinkstate=(nChipsLinksState >>16) & 0x3F;
	if(txlinkstate != 0x3F) //some links need resetting
	{
		txlinkstate = (~txlinkstate) & 0x3F; //the links which need resetting are set to 1
		//write to system ctlr reset sub system register to reset the rx and tx links 
		resetcode=(0x5EC << 20);
		resetcode |= (txlinkstate << 4);  //tx links
		writeMemory(SYSCTLR_BASE_U+SYSCTLR_SBSYS_RST_LV, resetcode);
	}
	if(rxlinkstate != 0x3F) //some links need resetting
	{
		rxlinkstate = (~rxlinkstate) & 0x3F; //the links which need resetting are set to 1
		//write to system ctlr reset sub system register to reset the rx and tx links 
		resetcode=(0x5EC << 20);
		resetcode |= (rxlinkstate << 10);  //rx links
		writeMemory(SYSCTLR_BASE_U+SYSCTLR_SBSYS_RST_LV, resetcode);
	}
}
 void sendHelloForResponseTo(int linkno)
 {
 	  unsigned int content=0;
	  #ifdef VERBOSE
		   printf("proc %d: in sendHelloForResponseTo: %d\n", procID, linkno);
	  #endif
	  //ask how are you?
	  content=  NNOP_STARTING_ADDRESS;
	  content=content | (NN_DIAG_HOW_R_U << 12);    //NNDIAG_SEND_HELLO_FOR_RESPONSE, how are you, send response
	  sendNNPacketWOPL(linkno, NORMAL, content);
      //ask common neighbor about his health
      
 }
/*******************************************************************
 * 
 * 
 ******************************************************************////
 void sendHelloForResponse()
{
	  #ifdef VERBOSE
		   printf("proc %d: in neighbourProbing\n", procID);
	  #endif
	  //send to those who have not sent a hello
	  //send only to those links which are alive and where the mon proc is selected
	  if(!(nChipsLinksState & 1) && (nChipsLinksState & (1 << 24)) && (nChipsLinksState & (1 << 8)) ) //neighbor on links 0
	  {
	     	sendHelloForResponseTo(0);
	  }
	  if(!(nChipsLinksState & 2) && (nChipsLinksState & (1 << 25)) && (nChipsLinksState & (0x1 << 9)) ) //neighbor on links 1
	  {
	    	sendHelloForResponseTo(1);
	  }
	  if(!(nChipsLinksState & 4) && (nChipsLinksState & (1 << 26)) && (nChipsLinksState & (1 << 10)) )//neighbor on links 2
	  {
			sendHelloForResponseTo(2);
	  }
	  if(!(nChipsLinksState & 8) && (nChipsLinksState & (1 << 27)) && (nChipsLinksState & (1 << 11)) ) //neighbor on links 3
	  {
		    sendHelloForResponseTo(3);
	  }
	  if(!(nChipsLinksState & 0x10) && (nChipsLinksState & (1 << 28)) && (nChipsLinksState & (1 << 12)) ) //neighbor on links 4
	  {
		    sendHelloForResponseTo(4);
	  }
	  if(!(nChipsLinksState & 0x20) && (nChipsLinksState & (1 << 29)) && (nChipsLinksState & (1 << 13)) ) //neighbor on links 5
	  {
			sendHelloForResponseTo(5);
	  }	
}	
void enquireCommonNeighbourTo(int linkno)
{
	  unsigned int content=0;
	  int linkplus, linkminus;     //the links on both sides of the linkno i.e. in case of linkno=0, linkplus=1, linkminus=5
	  linkplus=(linkno + 1) % 6;
	  linkminus=(linkno + 5) % 6;
	  #ifdef VERBOSE
		   printf("proc %d: in enquireCommonNeighbour\n", procID);
	  #endif
	  content = NNOP_STARTING_ADDRESS;
	  content = content | (NN_DIAG_HAVE_U_HEARD_OUR_N << 12);    // have you heard from my neighbor on my E, W, NE, NW, SE, SW	 in	 routingkey.range(11, 0), no payload
	  if(nChipsLinksState & 0x2 ) //if the common neighbour on port 1 is alive, ask it about the chip on its port no 5
	  {
	  	content = content | linkminus; 		   //port number of the chip to be enquired in bits 11-0
	    sendNNPacketWOPL(linkplus, NORMAL, content);
	  }
	  else if(nChipsLinksState & 0x20) //or check the chip on port number 5 to enquire about the health of the chip on port 1
	  {
	  	content=content | linkplus; 		   //port number of the chip to be enquired in bits 11-0
	    sendNNPacketWOPL(linkminus, NORMAL, content);
	  }
	  else
	  	; //do something, not sure at the moment
	
}
/*******************************************************************
 * to enquire a common neighbour about the health of a chip whose
 * hello message has not yet arrived 
 ******************************************************************/
void enquireCommonNeighbour()
{
	  #ifdef VERBOSE
		   printf("proc %d: in enquireCommonNeighbour\n", procID);
	  #endif
	  if((nChipsLinksState & 0x1)==0) //neighbor on links 0 is not responsive
	  {
	      	enquireCommonNeighbourTo(0);
	  }
	  if((nChipsLinksState & 0x2)==0) //neighbor on links 1
	  {
	   		enquireCommonNeighbourTo(1);
	  }
	  if((nChipsLinksState & 0x4)==0) //neighbor on links 2
	  {
		  	enquireCommonNeighbourTo(2);
	  }
	  if((nChipsLinksState & 0x8)==0) //neighbor on links 3
	  {
	      enquireCommonNeighbourTo(3);
	  }
	  if((nChipsLinksState & 0x10)==0) //neighbor on links 4
	  {
	      enquireCommonNeighbourTo(4);
	  }
	  if((nChipsLinksState & 0x20)==0) //neighbor on links 5
	  {
		  enquireCommonNeighbourTo(5);
	  }	
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void decideLinkOrChip()
{
	#ifdef VERBOSE
		   printf("proc %d: in decideLinkOrChip\n", procID);
	#endif
	//if(((nChipsLinksState & (1 << i)) == 0) && ((nChipsLinksMarkState & (1 << i)) == 0)) 
}
/*******************************************************************
 * helps to select the nursing chip to cure the dead chip
 * can implement one of three methods
 * 1. central i.e. report to the host chip and let the host to decide
 * 2. static i.e. always the chips on link 0 of the dead chip to be the nursing chip
 * 3. dynamic i.e. the six neighbouring chips to decide autonomously who to be the nursing chip 
 * presently only static nursing chip selection mechanism 
 ******************************************************************/
int amITheNurse()  //
{
	//three protocols can be implemented i.e. 
	//static i.e. the chip on link 0 of the dead chip will try to cure
	//central i.e. the chips will report the matter to the Host which will decide the nursing chip
	//dynamic i.e. the chips around the dead chip will communicate with each other to decide who to become the nurse 
	//either by reading the value of monitor processor selection register in the system controller of the dead chip
	//which  will give value 0x80000000 to the nursing chip and 0x00000000 to other chips, or by a token ring protocol
	//presently a combination of the static and central is implemented i.e. first the chip on the port 0 of the dead chip
	//will try to cure the chip, if the chip on port 0 is dead,
	//if fail then the control is passed to the Host PC.
	#ifdef VERBOSE
		printf("proc %d: in amITheNurse\n", procID);
	#endif
	if((nChipsLinksState & 0x8)==0) //if chip on port 3 is dead i.e. this chip is on the link 0 of dead chip, it will be the nursing chip
		return 1;
	else
		return 0;
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void readDeadChipCPUOK()
{
	#ifdef VERBOSE
		   printf("proc %d: in readCPUOK\n", procID);
	#endif
	//read the system controller registers in the dead chip to establish the 
	sendNNPacketWOPL(3, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK));    //cpu ok register to know the state of cpus and chip periherals
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void readDeadChipMonProcID()
{
	#ifdef VERBOSE
		printf("proc %d: in readMonProcID\n", procID);
	#endif
	//read the system controller registers in the dead chip to establish the 
	sendNNPacketWOPL(3, DIRECT, (SYSCTLR_BASE_U+SYSCTLR_MONITOR_ID));    //mon proc. id register to know the mon proc 
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void readBootRom()
{
	#ifdef VERBOSE
		printf("proc %d: in diagDeadChip\n", procID);
	#endif
	//read the system controller registers in the dead chip to establish the 
	sendNNPacketWOPL(3, DIRECT, 0xF2000034);    //cpu ok register to know the state of cpus and chip periherals
	BootRomCodeLoaded=1;
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void loadBootRomCode()
{
	int j;
	unsigned int con=0;
	#ifdef VERBOSE
	   printf("proc %d: in loadBootRomCode\n", procID);
	#endif
	con=NNCon(3, DIRECT);  //route=3, nn packet type =DIRECT
	con = CommCtlR0(7, con, chipAddress);
	for(j=0; j<0x6000; j=j+4)
	{
		nnAckDue=1;
		nnAckDueRoute=3;
		writePacketWPL(con, SYSTEM_RAM_BASE+j, MEM(BOOT_ROM_BASE, j));
		while(nnAckDue)  //wait till we get an ack for successfull wirte on the dead chip's system RAM
			;
	}
	BootRomCodeLoaded=1;
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void toggleBootRomAddress()
{
	#ifdef VERBOSE
		printf("proc %d: in toggleBootRomAddress\n", procID);
	#endif
	nnAckDueRoute=3;
	sendNNPacket(3, DIRECT, SYSCTLR_BASE_U+SYSCTLR_MISC_CONTROL, 0x1);    //writing to bit 0 of SYSCTLR_MISC_CONTROL register to toggle the RAM-ROM address to access RAM instead of ROM
	while(nnAckDue)  //wait till we get an ack for successfull wirte on the dead chip's system RAM
			;

}
/*******************************************************************
 * 
 * 
 ******************************************************************/
void resetDeadChip()
{
	unsigned int resetbit;
	#ifdef VERBOSE
		   printf("proc %d: in resetDeadChip\n", procID);
	#endif
	resetbit=(0x5EC <<20)|(0xFFFFF);  //resetting all components
	sendNNPacket(3, DIRECT, SYSCTLR_BASE_U+SYSCTLR_SBSYS_RST_PU, resetbit);    //writing to bit 0 of SYSCTLR_SBSYS_RST_LV register to reset the whole chip
}
/*******************************************************************
 * 
 * 
 ******************************************************************/
resetDeadChipProcs()            //only reset processors
{
	unsigned int resetbit;
	#ifdef VERBOSE
		   printf("proc %d: in resetDeadChip\n", procID);
	#endif
	resetbit=(0x5EC <<20)|(0xFFFFF);  //resetting all 20 processors
	sendNNPacket(3, DIRECT, SYSCTLR_BASE_U+SYSCTLR_CPU_RST_PU, resetbit);    //writing to bit 0 of SYSCTLR_CPU_RST_PU register to to reset the processors
}
/******************************************************************
 * 
 * 
 ******************************************************************/
void markChipsLinksState()
{
	#ifdef VERBOSE
		   printf("proc %d: in markChipsLinksState\n", procID);
	nChipsLinksMarkState=0x3F;   //set for all the state has been marked
	#endif
}


/*******************************************************************
 * 
 * 
 ******************************************************************/

#endif /*NNDIAGPROTOCOL_H_*/
