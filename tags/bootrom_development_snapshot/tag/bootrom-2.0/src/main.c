/******************************************************************************************************
* 	created by mmk, x.jin
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
* 
****************************************************************************************************/	
#include "RC32Calc.h"
#include "spinnakerlib.h"
#include "highLevelHandlers.h"
#include "bootProcess.h"




//#include "messagePassing.h"
/*todo
 * explaination of the options with spinnaker instruction send with the frame - 1 page doc
 * constants in global to be streamlined all in decimal or hex
 * turn off the log files and output files
 * macDestAddressConfigured = 1; 
 * 
 * 
 * 1. missing words after block-level floodfill.
 * 2. response out after an interval.
 * 3. status out.
 * 4. ethernet to change its destination mac address after receiving a frame.
 * 5. source mac and the ip address from the serial rom.
 * 6. memory access from outside to avoid undefined region.
 * 13.  testCommNoC()
 * 14.  floodfill testing with nn packets enabled
 * 15. improve formatAndSendFrame()
 * 16. p2p communication between the Host and a particular chip
 * 17. set packet received as an FIQ,       not feasible in boot up process
 * 18. status report accumulation and sending process, the chips to send a series of p2p packets with the status
 * 19. change chiplinkstate to have the state of chip+tx+rx+mon as an rx can be alive but not tx etc
 * questions
 * to reset processors should i write to reset register or reset pulse register
 * *************to do on timer after 10ms, set the timephase of router, check it the p2pCommOut is having 
 * some value then send these out of the system
 */
//**********************************************************************************************
//* main function
//* to be moved later to the timer interrupt after going to sleep
//**********************************************************************************************
#define 	WAIT_FOR_TIMEOUT		3			//to set the waiting time in each stage with a parameter
#define		NO_OF_LINK_RESETS		2			//to repeat the link reset for a parameterized number of times










void putvar (unsigned int n, int d)
{
  char hex[] = "0123456789abcdef";
  int i;

  for (i = d - 1; i >= 0; i--)
    dputc (hex [(n >> (4 * i)) & 15]);
}


int main(void)
{   
	//int value, value1;  //MEM(SYS_RAM_BASE, 100)=0xFF00FF;  //value=MEM(SYS_RAM_BASE, 100); 
	 //value1=value + 1;
	int count=0, count2=0, size=0;    						//to be used in for loops
	unsigned int rkey, pload;
	//int noOFLinkReset=0;					//counter to keep a record of the link resets for a parameterized number of times
	unsigned int val;
	//int sourceAddress;

	dputc ('M'); // !!!
//        printf("Main Program");
        
	configDone=0;
	opMode=HELLO;         //NN_DIAG;   //
	flood_fill_state=FF_BLOCK_START;
	frameSent=0;
	ackFrameDue=0;
	previousState=NN_DIAG;
	nndiagstate=NNDIAG_CHECK_LINKS;
	deadchipDiagState=DEADCHIPDIAG_READ_SYSCTRL;
	#ifdef VERBOSE
		printf("proc %d: Into main() now!!\n", procID);
	#endif
	if(bootupProc() != 0)  //bootup done and neighborhood probing activated
	{
		//initializing local variables
	  dputc ('K'); // !!!
		configDone =	MEM(globalVarSpace, SR_CONF_DONE);
		coreChipStatus = MEM(SYSCTLR_BASE_U, SYSCTLR_SET_CPU_OK);
	  	  
		#ifdef VERBOSE
			printf("proc %d: first phase bootup completed!!\n", procID);
		#endif
	}
	while(1)
	{


		if(configDone  && procType == 1) //monitor processor
		{
//	                dputc ('M');
//	                dputc (':');
//	                putvar(opMode,2);
//	                dputc (' ');
//     	                dputc (' ');
	                
			switch(opMode)
	         {
	         	case NN_DIAG:    //first do neighbours's diagnostics
	         	{
	         		#ifdef VERBOSE
						printf("proc %d: in NN_DIAG state!!\n", procID);
					#endif
					    //if nn diag done ok change the state
	         		    //else
	    				//case 1: sendHelloForResponse()
	    				//case 2: enquireTheHealthFromNeighbor
	    				//case 3: decideIfLinkBroken
	    				//case 4: if chip dead decide who to cure
	    				//case 5: read the system ctl
	    				//case 6: if bootrom is dead, load the code in system ram and change the address and restart
	    				//case 7: if monitor proc was selected but gone dead, disable its clock and reset the others
    		      	NNDiagDone=0;
    		      	if( ((nChipsLinksState  | nChipsLinksMarkState) & 0xFFFFFF) == 0x003F3F3F) //if either all the chips/links are ok or they have been marked 
	         			NNDiagDone=1;
	         		if(NNDiagDone==1)
	         		{
	         			#ifdef VERBOSE
							printf("proc %d: NN Diag done!!\n", procID);
						#endif
						opMode=HELLO;
	         		}
	         		else
	         		{
	         			switch(nndiagstate)    //state of links and chips is maintained in the nChipsLinksState and  sysRam[SR_NLINKCHIIPS_STATE]
	         			{
	         				case NNDIAG_CHECK_LINKS:
	         				{
	         					//check if the links are marked fine
	         					if( ((nChipsLinksState >> 8) & 0xFFFF) == 0x3F3F) //rx and tx links are ok
	         					{
	         						sendHelloToNeighbor();  //sending all the neighbours
	         						nndiagstate=NNDIAG_SEND_HELLO_FOR_RESPONSE;
	         						NNDiagTime=currentTime;
	         					}
	         					else if(currentTime >= (NNDiagTime+WAIT_FOR_TIMEOUT))   //wait for 2 ms before doing NN diagnostics
			         			{
			         				//if no response within this time then reset the links and check again
			         				//do not send to this chip, rather mark it so
			         				//send hello to the chips with 
			         				if(linksResetDone < NO_OF_LINK_RESETS) //it is the first time, we have not reset the links yet
			         				{
				         				nndiagstate=NNDIAG_CHECK_LINKS;
		         						NNDiagTime=currentTime;
		         						linksResetDone = linksResetDone + 1;
		         						resetLinks();
				         				checkInterchipLinks();
				         				if(linksResetDone >= NO_OF_LINK_RESETS)
				         				{
				         					nndiagstate=NNDIAG_SEND_HELLO_FOR_RESPONSE;
	         								NNDiagTime=currentTime;
				         				}
				         			}
			         				else   //establish if the links are broken or the chips
			         				{
			         					//Tx faulty ||	Rx faulty || Dead chip || Clock delay
			         					//nChipsLinksMarkState |= ((~(nChipsLinksState >>16)) << 16);
			         					nndiagstate=NNDIAG_SEND_HELLO_FOR_RESPONSE;  
	         							NNDiagTime=currentTime;
			         				}
			         			}
			         			break;
			         		}
	         				case NNDIAG_SEND_HELLO_FOR_RESPONSE:       //case 201: sendHelloForResponse()
    						{
    							#ifdef VERBOSE
									printf("proc %d: in NNDIAG_SEND_HELLO_FOR_RESPONSE state!!\n", procID);
								#endif
								//prompt the neighbor(s) from whome did not hear to send hello again
								//send only to those whose links are ok and monitor proc has been selected
			    				if(currentTime >= (NNDiagTime+WAIT_FOR_TIMEOUT))   //wait for 5 ms before doing NN diagnostic
			         			{
			         				//****to do**** first see if all have not sent hello then broadcast, instead of sending packets individually
			         				//keep the state to the nn_diag
			         				NNDiagTime=currentTime;
			         				nndiagstate=NNDIAG_ENQUIRE_FROM_NEIGHBOUR;
			         				sendHelloForResponse();   //only to the live chips, response will update the neighboringChipsStatus[i] and will get the state to the hello
			         				
			         			}
    							break;
    						}
	    					case NNDIAG_ENQUIRE_FROM_NEIGHBOUR:     //case 2: enquireTheHealthFromNeighbor
	    					{										//the state of the links or chips will be recorded as a result of 
	    															//a response to this message from the common neighbours and marked in 
	    															//nChipsLinksState
	    															//if the chips are ok but the links are faulty, these will be reset from packetreceived
	    															//ISR or marked as such and the control will go to the Hello in the start of this state
	    															//if, however, there a dead chip, the next state will handle it
	    						#ifdef VERBOSE
									printf("proc %d: in NNDIAG_ENQUIRE_FROM_NEIGHBOUR state!!\n", procID);
								#endif
								if(currentTime >= (NNDiagTime+WAIT_FOR_TIMEOUT))   //wait for 5 ms before doing NN diagnostic
			         			{
			         				NNDiagTime=currentTime;
			         				nndiagstate=NNDIAG_DEAD_CHIP_WHO_TO_CURE;
			         				enquireCommonNeighbour();
			         				
			         			}
	    						break;
	    					}
	    					case NNDIAG_DEAD_CHIP_WHO_TO_CURE:			//case 4: if chip dead decide who to cure, only in case of dynamic selection of the nursing chip
	    					{
	    						#ifdef VERBOSE
									printf("proc %d: in NNDIAG_DEADCHIPWHOTOCURE state!!\n", procID);
								#endif
								if(currentTime >= (NNDiagTime+WAIT_FOR_TIMEOUT))   //wait for 5 ms before doing NN diagnostic, define the time and use that one
			         			{
			         				//mark the links as dead at this point
			         				//if((nChipsLinksState >>8) != 0x3F3F) //some link(s) are still not clear
			         				if((nChipsLinksState & 0x3F) != 0x3F) //if all the chips are not healthy
			         				{
			         					if(amITheNurse()==1) //if this chip is the nursing chip, according to the protocol implemented i.e. static, central, dynamic 
			    						{
			    							nndiagstate=NNDIAG_DEAD_CHIP_RECOVERY;
			       						}
			    						else
			    						{
			    							//go to wait state to wait for the relevant chip to bring the dead chip from the dead state
			    							nndiagstate=NNDIAG_WAIT_FOR_DIAG;
			    						}
			         				}
			         				else  //mark the non responsive links dead
			         				{
			         					nChipsLinksMarkState |=( ( (~(nChipsLinksState >> 8)) & 0x3F3F3F) << 8);
			         					opMode = HELLO;
			         				}
			         			}
	    						break;
	    					}
	    					case NNDIAG_DEAD_CHIP_RECOVERY:				//case 5: read the system ctl
	    			        {
	    			        	#ifdef VERBOSE
									printf("proc %d: in NNDIAG_DEADCHIPDIAG state!!\n", procID);
								#endif
								//will be done interactively with the help of packets, i.e. packet receive interrupt is also involved
    							switch(deadchipDiagState)
    							{
    								case DEADCHIPDIAG_READ_SYSCTRL:
    								{
    									readCPUOKDone = 0;
    									readMonProcIDDone = 0;
    									NNDiagTime = currentTime;
    									deadchipDiagState = DEADCHIPDIAG_READ_BOOTROM;
    									readDeadChipCPUOK();
    									readDeadChipMonProcID();
    									break;
    								}
    								case DEADCHIPDIAG_READ_BOOTROM:
    								{
    									if(readCPUOKDone==1 && readMonProcIDDone==1)
    									{
    										if( (deadChipCPUOKValue > 0) && (deadChipMonProcValue > 0) )
    										{
    											//boot rom may be there but there is some problem either in the bootrom code or the monitor proc selection processes
    											//do something
    											nndiagstate=NNDIAG_MON_PROC_CHANGE;;
    										}
    										else
    										{
    											//for sure the boot rom is dead, try to read it to ensure it is the case
    											//readBootRom();
    											nndiagstate=NNDIAG_BOOTROM_TOGGLE;
    										}
		    							}
    									else
    									{
    										//wait for 3 ms before marking the chip to dead for not being responsive i.e. there is something wrong with the router 
    										//mark the chips to dead and do nothing
    										if(currentTime >= (NNDiagTime+WAIT_FOR_TIMEOUT))
    										{ 
    											nChipsLinksMarkState |= ((~nChipsLinksState) & 0x3F); //marking the chips still non responsive as dead
    										}
    									}
    									break;
    								}
    								default:
    									break;
    							}
								break;
	    					}
	    					case NNDIAG_WAIT_FOR_DIAG:               //wait for the nursing chips to cure the dead chip
	    					{
	    						#ifdef VERBOSE
									printf("proc %d: in NNDIAG_WAITFORDIAG state!!\n", procID);
								#endif
								//just go to sleep do not do anything
	    						if(currentTime >= (NNDiagTime+WAIT_FOR_TIMEOUT*WAIT_FOR_TIMEOUT))   //wait for a bit longer before doing NN diagnostic
			         			{
			         				nndiagstate=NNDIAG_MARK_CHIPS_LINKS_STATE;
			         			}
			         			break;
	    					}
	    					case NNDIAG_BOOTROM_TOGGLE: 				//case 6: if bootrom is dead, load the code in system ram and change the address and restart
	    					{
	    						#ifdef VERBOSE
									printf("proc %d: in NNDIAG_BOOTROMTOGGLE state!!\n", procID);
								#endif
	    						loadBootRomCode();
	    						if(BootRomCodeLoaded == 1)
	    						{
	    							toggleBootRomAddress();
	    						}
	    						nndiagstate=NNDIAG_RESET_DEAD_CHIP_PROCS;
			         			break;
	    					}
	    					case NNDIAG_MON_PROC_CHANGE:				//case 7: if monitor proc was selected but gone dead, disable its clock and reset the others
	    		   			{
	    		   				#ifdef VERBOSE
									printf("proc %d: in NNDIAG_MONPROCCHANGE state!!\n", procID);
								#endif
								//send a soft reset to all the processors 
	    						break;
	    					}
	    					case NNDIAG_RESET_DEAD_CHIP_PROCS:
	    		   			{
	    		   				#ifdef VERBOSE
									printf("proc %d: in NNDIAG_RESETDEADCHIP state!!\n", procID);
								#endif
	    		   				resetDeadChipProcs();            //only reset processors
	    		   				NNDiagTime=currentTime;
	    		   				nndiagstate=NNDIAG_WAIT_FOR_DIAG;
	    		   				break;
	    		   			}
	    		   			case NNDIAG_RESET_DEAD_CHIP:
	    		   			{
	    		   				#ifdef VERBOSE
									printf("proc %d: in NNDIAG_RESETDEADCHIP state!!\n", procID);
								#endif
	    		   				resetDeadChip();
	    		   				NNDiagTime=currentTime;
	    		   				nndiagstate=NNDIAG_WAIT_FOR_DIAG;
	    		   				break;
	    		   			}
	    		   			default: 								//do something else, NNDIAG_MARKCHIPSLINKSSTATE
	    		   			{
	    		   				markChipsLinksState();
	    		   				opMode=HELLO;
	    		   			}
	    		   		}
	         		}
         			break;
	         	}
	         	case HELLO:
	         	{
	         		
	         		#ifdef VERBOSE
						printf("proc %d: in HELLO state!!\n", procID);
					#endif
					//give about 20ms for the NN diagnostic 
	         		if(EthLinkUp)
	         		{
	         			MEM(globalVarSpace, SR_HOST_CONNECTED_CHIP)=1;		    //0x0C                  //hostConnectedChip=0;  //to identify if the chip is connected to the Host, to be kept in system ram
						hostConnectedChip=1;
	         			
	         			if(!frameSent)
	         			{
	         				#ifdef VERBOSE
								printf("proc %d: s F: hello from main\n", procID);
		         			#endif
							sendHelloFrame(); 
							dputc('H');
		         			frameSent=1;
		         			ackFrameDue=1;
		         			previousState=HELLO;
		         			helloTime=currentTime;  //time in relation to current millisecond time counter when the hello message is sent
	         			}
	         			else
	         			{
	         				if(ackFrameDue)
	         				{
 	         					if(currentTime >= helloTime+1000) //if ack does not come in 1000 milliseconds, send hello again
	         					{
	         						frameSent=0;
	         					}
	         					else
	         						; //do nothing just wait for ack
	         				}
	         				else //ack received
	         				{
	         				     opMode=IDLE;
	         				}
	         			}
//	         			helloTime=currentTime;  removed by CP as just reset timer
	         					
	         		}
	         		else {
       				        dputc('Y');
	         			opMode=IDLE;  //if not connected to the Host PC via phy then go to idle mode
	         		}
	         		break;
	         	}
	   			case FLOOD_FILL:								 	//floodfill data
	   			{
	   				//start flood fill, the data block 1K at the moment, is in the rxFrameBase. rxFrameBase[16/4] contains the size of the block, 
					//rxFrameBase[20/4] contains the physical address to copy the block to, and rxFrameBase[24/4] contains the crc of the block, the data starts from rxFrameBase[24/4]
					//first do the crc check on the block and if ok carry on with the floodfill, or send a frame asking for the block again
	   				//if a frame comes during the floodfill of already received frame, it will be discarded and reported to the Host to wait
	   				//for a while unless a message to abort comes from the Host
	   				switch(flood_fill_state)
					{
						#ifdef VERBOSE
								printf("proc %d: in FLOOD_FILL state!!\n", procID);
						#endif
						case FF_BLOCK_START:						//send instruction 0: i.e. routingkey.range( 19, 12)=0  means sending broadcast message with block size given in routingkey.range(11, 0) and starting address in the payload
							#ifdef VERBOSE
								printf("proc %d: in FF_BLOCK_START state!!\n", procID);
							#endif
							
							for(count=0; count<(rxBufSize/4); count++)  //copy the data block to a safe place before floodfilling, so that the next frame should not damage the data
			   				{
			   					rxBuf[count]=rxFrameBase[count + spinnakerDataAt/4];  //the data starts at 28/4
			   				}
							//			   				if(rxBufSize % 4) //if there are more bytes than multiple of 4 then copy the last word
							//			   				{
							//			   					//count++;
							//			   					rxBuf[count]=rxFrameBase[count + spinnakerDataAt/4];
							//			   				}
			   				//get the block size from bits 19-16
			   				rxBufStAddr = rxFrameBase[instructionOption1At/4];
			   				//get start address from bits 23-20
			   				rxBufSize = rxFrameBase[instructionOption2At/4];
			   				//get the block-level crc from bits 27-24
			   				rxBufCRC = rxFrameBase[instructionOption3At/4];
			   				
			   				crc=Get_CRC((unsigned char*)rxBuf, rxBufSize);
							if(crc != rxBufCRC)
			   				{
			   					#ifdef VERBOSE
									printf("proc %d: FF block failed CRC, requesting resend!!\n", procID);
								#endif
			   					//send a frame to resend the data block again
			   					sendErrorReportFrame(CRC_FAILED_RESEND_DATA);
			   					//formatAndSendFrame(4, CRC_FAILED_RESEND_DATA);
			   					floofillInProcess=0;  //clear flag to restart the floodfill
			   					opMode=IDLE;	
			   					break;  //out of flood_fill case statement
			   				}								//don't start the floodfill
			   				//else
		   					//set the rxBitmap bits to high to show that this chip is the reference chip and does not need to store the data as part of nn packets
   							
   							for(count=0; count<(rxBufSize/32); count++)
   								rxBitmap[count]=0xFFFFFFFF;
   								
   							if((rxBufSize % 32) > 0)    //it will inform the chip that it is the source or reference chip and it does not have to record the data on receipt of NN packets from the neighbours
   							{
   								rxBitmap[count]=0xFFFFFFFF;
   							}
	   						rkey =(0x0F8 << 20) | (NN_FF_START << 12);    	//31-20 NN Intruction address space, 19-12 instruction
							rkey = (rkey | (rxBufSize & 0xFFF) );   //bits 11-0 for buf size
							pload = rxBufStAddr;					
							sendNNPacket(BROADCAST, NORMAL, rkey, pload);
							flood_fill_state=FF_BLOCK_DATA;
							floofillInProcess=1;    //flag to lock the process while the ongoing block is being handled
							size = rxBufSize;
							count2=0;			//(6+6+4+4)/4;
							break;
						case FF_BLOCK_DATA:
							#ifdef VERBOSE
								printf("proc %d: in FF_BLOCK_DATA state!!\n", procID);
							#endif
							while(count2 < size )  //if the block size is 4 bytes or more
							{
								pload=rxBuf[count2/4];
								rkey= rxBufStAddr+count2;
								sendNNPacket(BROADCAST, NORMAL, rkey, pload);
								waitCycles(WAIT_BET_FF_PACKETS);
								count2 += 4;
							}
							flood_fill_state=FF_BLOCK_END;
							//							else
							//							{
							//								pload=rxFrameBase[count];
							//								//offset=size%4;       //bytes left over
							//								pload=(pload & ((size%4) * 8));  //
							//								rkey= rxBufStAddr+(count * 4);
							//								count++;
							//								sendNNPacket(BROADCAST, NORMAL, rkey, pload);
							//								size=0;
							//								//flood_fill_state=FF_BLOCK_CRC;
							//								flood_fill_state=FF_BLOCK_END;
							//							}
							break;
						//no need for the FF_BLOCK_CRC as the FF_BLOCK_END will send the CRC with it
						//						case FF_BLOCK_CRC:
						//							#ifdef VERBOSE
						//								printf("proc %d: in FF_BLOCK_CRC state!\n", procID);
						//							#endif
						//							//send CRC
						//							//instruction 4:   block level checksum, size in the routingkey.range(11, 0), checksum in payload
						//							rkey=(0x0F8 << 20) | (NN_FF_CRC << 12);     //31-20 NN Intruction address space, 19-12 instruction type 4
						//							size= 4;							//32 bits
						//							rkey= (rkey | (size & 0xFFF));  //bits 11-0 for checksum size
						//							pload= rxBufCRC;
						//							sendNNPacket(BROADCAST, NORMAL, rkey, pload);
						//				 			flood_fill_state=FF_BLOCK_END;
						//				 			break;
					 		
				 		case FF_BLOCK_END:
				 			#ifdef VERBOSE
								printf("proc %d: in FF_BLOCK_END!\n", procID);
						 	#endif
							//instruction 1: i.e. routingkey.range(19, 12)=1 means end of current broadcast message
						 	rkey=(0x0F8 << 20) | (NN_FF_END << 12);     //31-20 NN Intruction address space, 19-12 instruction
							floodfillEndInstrID +=1;
							rkey= (rkey | (floodfillEndInstrID & 0xFFF));  //bits 11-0 for checksum size
							pload= rxBufCRC;
							sendNNPacket(BROADCAST, NORMAL, rkey, pload);
							flood_fill_state=FF_BLOCK_START;
						    opMode=STORE_BLOCK_TO_MEMORY;
						    floofillInProcess=0;  //clear flag to restart the floodfill
						    break;
						    
						default:
							break;
						}
						break;
	   				}
	   				case STORE_BLOCK_TO_MEMORY:
	   					#ifdef VERBOSE
							printf("proc %d: in STORE_BLOCK_TO_MEMORY state!\n", procID);
						#endif
						//save the buffer to the memory
						storeBlockToMemory(rxBuf, rxBufStAddr, rxBufSize);
						for(count=0; count<((rxBufSize/32)+1); count++) //clearing the bitmap
   							rxBitmap[count]=0;
						if(hostConnectedChip)
							sendAckFrame();
						opMode=IDLE;
						break;
						//system size is received with the break symmetery instruction
						//					case SEND_SYSTEM_SIZE:  //break symmetry
						//						#ifdef VERBOSE
						//							printf("proc %d: in STORE_BLOCK_TO_MEMORY state!!\n", procID);
						//						#endif
						//						//8:	 break the symmetery, assign address to yourself, the address of monitoring host will be as given in payload
						//						rkey=(0x0F8 << 20) | (NN_CONF_SYS_SIZE << 12);     //31-20 NN Intruction address space, 19-12 instruction
						//						rkey= (rkey | (spinnakerSystemSize & 0xFFF));  //bits 11-0 for checksum size
						//						pload= 0;  //(0, 0) HS can be accessed at (0, 0)
						//						sendNNPacket(BROADCAST, NORMAL, rkey, pload);
						//					    sendAckFrame();
						//						opMode=IDLE;
						//						break;
						
					case BREAK_SYMMETRY:  //break symmetry combined with the system size
						#ifdef VERBOSE
							printf("proc %d: in STORE_BLOCK_TO_MEMORY state!!\n", procID);
						#endif
						//8:	 break the symmetery, assign address to yourself, the address of monitoring host will be as given in payload
						//chip host connected chip has been selected as a reference chip (0,0)
						//the size of system is received in option1 of the ethernet frame
						spinnakerSystemSize=rxFrameBase[instructionOption1At/4];
						//assign the chip address to (0,0) and pass it on
						chipAddress=0;
						isRefChip=1;
						MEM(globalVarSpace, SR_CHIP_ADDRESS)=0;
						MEM(globalVarSpace, SR_IS_REF_CHIP)=1;		
								
						rkey=(0x0F8 << 20) | (NN_CONF_SYS_SIZE << 12);     //31-20 NN Intruction address space, 19-12 instruction
						val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
						rkey= (rkey | (val & 0xFFF));  //
						systemSizeInstrID=val;
						pload= spinnakerSystemSize;  //(spinnakerSystemSize in the payload
						sendNNPacket(BROADCAST, NORMAL, rkey, pload);
						//wait before sending next packet
					    waitCycles(WAIT_BET_FF_PACKETS);
						
						rkey=(0x0F8 << 20) | (NN_CONF_BREAK_SYMMETRY << 12);     //31-20 NN Intruction address space, 19-12 instruction
						val =(rxFrameBase[instructionOption3At/4] & 0xFFF);  //used to avoid looping around of the same instruction
						rkey= (rkey | (val & 0xFFF));  //
						symmetryBrokenInstrID=val;
						pload= 0;  //my address is (0, 0), HS can be accessed at (0, 0)
						sendNNPacket(BROADCAST, NORMAL, rkey, pload);
					    sendAckFrame();
						opMode=IDLE;
						break;
						
					case REPORT_STATUS: //report OK
						//on receipt of this instruction, broadcast the message to all chips and
						//set a flag to indicate that a status is due
						//also set the time counter to current time 
						//once the current time + the wait time for status report becomes equal to or more than
						//the max wait time for status report, send the status of all the chips received so far
						//the status is received from the chips and if the chip is ok, the corresponding bit in 
						//the bitmap equal to the size of system /32 at the end of the dtcm is set to 1 or 0
						//we send the status buffer as one or more frames to the host pc
						//a chip ok in all respect will send the status report back, otherwise will not send it
						//and the host connected chip will set 0 for its corresponding bit
						#ifdef VERBOSE
							printf("proc %d: in REPORT_STATUS state!\n", procID);
						#endif
						//			  9:									   report ok (no payload)
						//step0: set its own status
						val=readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
						for(count=0; count < (spinnakerSystemSize/32 + 1); count++)
							statusBuffer[count]=0;
						if((val >> 20) == 0xFF)  //it is connected through phy
						{
							int offset = chipAddress % 32;
							int base = (chipAddress / 32) & 0xFFFF;
							statusBuffer[base] |= 0x1 << offset;
						}
						
						//step1: set the flag
						statusReportDue=1;
						//step2: set the time to the current time
						//if(currentTime >= (reportStatusTime+WAIT_FOR_REPORTSTATUSOUT)) will be done in timer interrupt
						reportStatusTime = currentTime;
						//step3: floodfill an nn packet to all the chips to report status 
						rkey=(0x0F8 << 20) | (NN_CONF_SEND_OK_REPORT << 12);     //31-20 NN Intruction address space, 19-12 instruction
						val =(rxFrameBase[instructionOption3At/4] & 0xFFF);  //used to avoid looping around of the same instruction
						rkey= (rkey | (val & 0xFFF));  //
						sendStatusReportInstrID=val;
						pload= 0;  //(0, 0) HS can be accessed at (0, 0)
						sendNNPacketWOPL(BROADCAST, NORMAL, rkey);
						//step4: send ack frame
						sendAckFrame();
						//step5: set the state of idle
						opMode=IDLE;
					    break;
					
					case SEND_STATUS_REPORT:
						#ifdef VERBOSE
							printf("proc %d: SEND_STATUS_REPORT state!\n", procID);
						#endif
						//this state will be activated by the packetReceive interrupt on seeing 
						//the reportStatus buffer state. if the chip has received report from all 
						//then it will send the state as a key:value pair to the HS
						//see the state in system ram and send OK report
						//payload=read from system ram
						//sendframe
					    //copy the buffer to the frame to be sent to the HS
						//txFrameBase[5]=no_of_status_received; //the number of key:value paired status reports
						
												
						if((spinnakerSystemSize/8) > 1420) //more than one frame size
						{
							printf("In Status Report > 1420\n"); // debug
							if(statusReportIndex < spinnakerSystemSize/32 )   //if status sent so far (statusReportIndex) are less than the due (spinnakerSystemSize)
							{
								if(!ackFrameDue)
								{
									//copy the chip status from the statusBuffer
									for(count=0, count2=statusReportIndex; ((count<(1420/4)) && (count2 < spinnakerSystemSize/8)); count++, count2++)          //to make it 60 bytes equivallent
									{
										txFrameBase[(spinnakerDataAt/4)+count]=statusBuffer[count2];
									}
									//no_of_status_received=0;
									statusReportIndex=count2;
									formatAndSendFrame(SEND_STATUS_REPORT, 0, count, 0);  //chip status is stored in the statusBuffer as a key:value pair, requiring 1 word for the chip address and 1 for its status totol=no_of_status_received*2*4. CP 19thJan10, message type corrected.
									if(statusReportIndex < spinnakerSystemSize/32) //there are still some to be sent
										moreStatusReportsDue=1; //still to send more status reports
									else
										moreStatusReportsDue=0;
									reportStatusTime = currentTime;         
									opMode=IDLE;
										
								}
								//else wait till next timer interrupt
							}
							else
							{
								statusReportDue=0; //CP 19thJan10, added to be able to exit the loop as status report now sent.
								moreStatusReportsDue=0;
								opMode=IDLE;
							}
							
						}
						else
						{
							printf("In Status Report < 1420\n"); // debug
							for(count=0; count <= spinnakerSystemSize/8; count++)          // <= CP 19thJan10 fixed so size1 = sent!
							{
								txFrameBase[(spinnakerDataAt/4)+count]=statusBuffer[count];
							}
							//no_of_status_received=0;
							formatAndSendFrame(SEND_STATUS_REPORT, 0, count, 0);  //chip status is stored in the statusBuffer as a key:value pair, requiring 1 word for the chip address and 1 for its status totol=no_of_status_received*2*4.   CP 19thJan10, message type corrected.
							statusReportDue=0; //CP 19thJan10, added to be able to exit the loop as status report now sent.
							ackFrameDue=1;
							opMode=IDLE;
							
						}
						break;
						
					case P2P_IN_COMM: //point to point communication from HS to a particular chip, eg sending routing table entries
					{
						#ifdef VERBOSE
							printf("proc %d: in P2P_IN_COMM state!\n", procID);
						#endif
						//send point to point packets 
						//first packet to carry an instruction in the payload followed by the data
						//the instruction, again starting from 0x08F, contains instruction in bits 19-12 and the size of data to follow in the 
						//bits 11-0, the data will be sent with sequence number
						//don't send ack, till the data is sent all
						//the word after the length and instruction in the Ethernet frame i.e. Option1 (rxFrmeBase[16/4]) contains instruction for the specific chip 
						//allong with its address
						//in case of data transfer, the size of data to be sent or received is in the next word i.e. Option2 (rxFrmeBase[20/4])
						//in case of stimulus, the procID is in Option3 (rxFrameBase[24/4])
						//spinnnaker instruction in instruction field bit 15-0
						//p2pInInstruction with the spinnaker instruction bit 31-16
						//chipaddress in option1 bit 15-0, port no or proc address etc in option1 bit 31-16
						//datasize in option2
						//memory address in option3
						//the values initialized in highLevelHanlers.h are
						//p2pInInstruction = ((rxFrameBase[spinnakerInstructionAt/4] >> 16) & 0xFFFF);
						//p2pInAddress =(rxFrameBase[instructionOption1At/4] & 0xFFFF);  
						//p2pInDataSize=(rxFrameBase[instructionOption2At/4] & 0xFFFF);
						//in case of stimulus, the data size and the chip address is passed with instruction as option 2 and 3
						//the neuron ids are passed in the data field as 16 bit id (5 bits for proc id and 11 bits for the neuron id) i.e. 0x040F means neuron id 15 in proc number 1
						
						switch(p2pInInstruction)
						{
							case P2P_COMM_REPORT_STATUS:		  //	send the cpu_ok register in the system controller
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									val=readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
									//send frame with the status
									txFrameBase[spinnakerDataAt/4] = chipAddress;
									txFrameBase[(spinnakerDataAt/4) + 1] = val;
									formatAndSendFrame(REPORT_STATUS, 0, 8, 0);  //chip status is stored in the statusBuffer as a key:value pair, requiring 1 word for the chip address and 1 for its status totol=no_of_status_received*2*4
								}
								else  //it is for some other chip
								{
									p2pHostToOtherChip(p2pInAddress, p2pInInstruction, 0, 0);
									count=0;
									while(p2pAckPending && count < P2P_WAIT_CYCLES) //wait till response does not comeback or time out
									{
										count++;   //wait for the ack to come 
									}
									if(!p2pAckPending) //send the status, status has been copied to status buffer at the location of p2pInAddress
									{
										txFrameBase[spinnakerDataAt/4] = p2pInAddress;
										txFrameBase[(spinnakerDataAt/4) + 1] = chipStatusRecieved;
										formatAndSendFrame(REPORT_STATUS, p2pInAddress, 8, 0);  //chip status is stored in the statusBuffer as a key:value pair, requiring 1 word for the chip address and 1 for its status totol=no_of_status_received*2*4
								
									}
									else //timeout
									{
										sendErrorReportFrame(ERR_REQUEST_TIMEOUT);
									}
								}
								break;
							case P2P_COMM_GET_DATA:		  		//  receive the data to be copied to memory, size of data in the bits 11-0, next packet to carry the starting address of the block to be copied
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									//by single p2p packets with ack packet from the destination
									currentP2PSeqNo=0;
									//p2pState=P2P_GET_DATA;
									//get data from the frame and copy it to the given location
									//memory address at instructionOption1At
									p2pInAddress=rxFrameBase[instructionOption3At/4];
									for(count=0; count < p2pInDataSize/4; count++)
									{
										//copy to the memory location (in the first word of data) + the offset
										writeMemory((p2pInAddress+(count*4)), rxFrameBase[(spinnakerDataAt/4+count)]);
									}
									
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, 0, 0)==0) //timeout error
									{
										opMode=IDLE;
										break;
									}
																	
									for(count=0; count<(p2pInDataSize/4); count++)
									{
										pload=p2pInDataBase[count];
										if(p2pHostToOtherChip(p2pInAddress, P2P_COMM_DATA_PACKET, pload, ((currentP2PSeqNo++)%3))==0) //timeout error
										{
											opMode=IDLE;
											break;
										}
										
									}
									
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_COMM_SEND_DATA:			 //  send the data from the memory, size of data in the bits 11-0, next packet to carry the starting address of the block to be copied
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									//by single p2p packets with ack packet from the destination
									currentP2PSeqNo=0;
									//p2pState=P2P_SEND_DATA;
									//send the Host the required data from the given location
									//memory address at instructionOption1At
									val=rxFrameBase[instructionOption3At/4];  //memory address to copy data from
									txFrameBase[instructionOption3At/4]=rxFrameBase[instructionOption3At/4];    //memory location the data is being copied from
									for(count=0; count < p2pInDataSize/4; count++)
									{
										txFrameBase[(spinnakerDataAt/4) + count]=readMemory(val + (count*4));
									}
									formatAndSendFrame(P2P_OUT_COMM, P2P_COMM_GET_DATA, p2pInDataSize, chipAddress);  //chip status is stored in the statusBuffer as a key:value pair, requiring 1 word for the chip address and 1 for its status totol=no_of_status_received*2*4
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, rxFrameBase[instructionOption3At/4], 0)==0)
									{
										opMode=IDLE;
										break;
									}
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_LOAD_APPLICATION:
								//to help loading hetrogenous application in various processors on various chips
								//the frame will carry the p2p instructions with the ethernet instructions is passed in the 
								//spinnakerInstruction field, the chip ID is passed in the option1, and the processors bits (each bit representing 
								//the processor id) is passed in the option3 of the ethernet frame. The address of the application in SDRAM
								//is passed in the datafield i.e. txFrameBase[spinnakerDataAt/4]
						
														
								//send two p2p packets, first to carry the processor IDs as high bits in the payload which have to load this application 
								//second packet to carry the address of the application to be loaded into these processors
								
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									val = readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
					   				val = val & 0xFFFFF; //see the live processors
					   				val = val & rxFrameBase[instructionOption3At/4]; //the processor ids to load application and they are alive
					   				//pass the message individually to all the processors
					   				sendAckFrame();
									opMode=IDLE;
					   				for(count=0; count<20; count++)
					   				{
					   					if(((val >> count) & 0x1) && (procID != count)) //the proc is live and it is not the mon proc then
					   					{
					   						passMessage(SM_LOAD_APPLICATION, count, rxFrameBase[(spinnakerDataAt/4)], 16);  //move 16 bytes i.e. 4 words from the application load point in SDRAM 
					   																										//containing the address to load application from, address of ITCM to load application to
					   																										//the size of the application and the location of starting point in the application
					   					}
					   				}
					   				
					   				
								}
								else
								{
									//total 6 packets will be send to the chip in the following steps
									//*************************** step1 - P2P_LOAD_APPLICATION_PROCIDS ****************************/
									//the procIDs are send with this instruction in two p2p packets
									//the first packet with the instruction P2P_LOAD_APPLICATION_PROCIDS but seqno=0 contains proID (11-0) in bits (11-0) 
									//of the payload, the second packet with the same instruction but seqno=1 contains the remaining
									//procIDs in its bits (11-0) val=rxFrameBase[instructionOption3At/4] & 0x7FF;   //bits 11-0
									val=(rxFrameBase[instructionOption3At/4] & 0xFFF);   //bits 11-0
									if(p2pHostToOtherChip(p2pInAddress, P2P_LOAD_APPLICATION_PROCIDS, val, 0)==0)
									{
										opMode=IDLE;
										break;
									}
									val=(rxFrameBase[instructionOption3At/4]>> 12) & 0xFFF;   //bits 11-0
									if(p2pHostToOtherChip(p2pInAddress, P2P_LOAD_APPLICATION_PROCIDS, val, 1)==0)  //seqno = 1
									{
										opMode=IDLE;
										break;
									}
									//************************* step2 - P2P_LOAD_APPLICATION_ADDRESS *******************************/
									//the application address is passed with this instruction in three p2p packets
									//the first packet with this instruction but seqno=0 contains addres bits (11-0) in payload bits (11-0) 
									//the second packet with the same instruction but seqno=1 contains the address bits (23-12)
									//the third packet with the same instruction but seqno=2 contains the remaining address bits
									val=(rxFrameBase[spinnakerDataAt/4] & 0xFFF);   //bits 11-0
									if(p2pHostToOtherChip(p2pInAddress, P2P_LOAD_APPLICATION_ADDRESS, val, 0)==0)
									{
										opMode=IDLE;
										break;
									}
									val=(rxFrameBase[spinnakerDataAt/4]>> 12) & 0xFFF;   //bits 11-0
									if(p2pHostToOtherChip(p2pInAddress, P2P_LOAD_APPLICATION_ADDRESS, val, 1)==0)  //seqno = 1
									{
										opMode=IDLE;
										break;
									}
									val=(rxFrameBase[spinnakerDataAt/4]>> 24) & 0xFFF;   //bits 11-0
									if(p2pHostToOtherChip(p2pInAddress, P2P_LOAD_APPLICATION_ADDRESS, val, 2)==0)  //seqno = 2
									{
										opMode=IDLE;
										break;
									}
									//sending P2P_LOAD_APPLICATION instruction with the address of the application in SDRAM
									if(p2pHostToOtherChip(p2pInAddress, P2P_LOAD_APPLICATION, 0, 0)==0)
									{
										opMode=IDLE;
										break;
									}
									sendAckFrame();
									opMode=IDLE;
								}
								break; 
							
							case P2P_COMM_DIAG_CHIP:		    //  be nurse chip to repair chip on your port in bits 11-0
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									//p2pState=P2P_DIAG_CHIP;
									//do nn diagnostics of the said chip. send ack frame
									//option1=p2pintruction, option2=datasize, option3=chipAddress(bits15-0)+portNo(bit31-16)
	/*port no to be used in nndiag*/	//put the opMode state in the nn diagnostics and nn_diag_state in recovery moe
									opMode = NN_DIAG;
			         				nndiagstate=NNDIAG_DEAD_CHIP_RECOVERY;				//case 5: read the system ctl
	    			        		deadchipDiagState=DEADCHIPDIAG_READ_SYSCTRL;
									
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, rxFrameBase[instructionOption3At/4], 0)==0)
									{
										opMode=IDLE;
										break;
									}									
									opMode=IDLE;
								}
								break;
							case P2P_COMM_RESET_CHIP:			//  reset chip on you port in bits 11-0
								//p2pState=P2P_RESET_CHIP;
								//reset the asked chip and send an ack frame
								count2=(p2pInAddress >> 16) & 0xF;   //port no
								if(count2 > 5)
								{
									//send error frame
									sendErrorReportFrame(ERR_INVALID_OPTION1);
									opMode=IDLE;
									break;
								}
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									val=(0x5EC <<20)|(0xFFFFF);  //resetting all components of the chip on port count2
									sendNNPacket(count2, DIRECT, SYSCTLR_BASE_U+SYSCTLR_SBSYS_RST_PU, val);    //writing to bit 0 of SYSCTLR_SBSYS_RST_LV register to reset the whole chip
									
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, count2, 0)==0)
									{
										opMode=IDLE;
										break;
									}
									
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_COMM_DISABLE_CHIP:		   //  disable the processors on chip on you port in bits 11-0
								count2=(p2pInAddress >> 16) & 0xF;   //port no
								if(count2 > 5)
								{
									//send error frame
									sendErrorReportFrame(ERR_INVALID_OPTION1);
									opMode=IDLE;
									break;
								}
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									//p2pState=P2P_DISABLE_CHIP;
									//disable the said chip and send ack frame
									val=(0x5EC <<20)|(0xFFFFF);  //disabling all processors on port count2
									sendNNPacket(count2, DIRECT, SYSCTLR_BASE_U+SYSCTLR_CPU_DISABLE, val);    //writing to bit 0 of SYSCTLR_SBSYS_RST_LV register to reset the whole chip
									
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, count2, 0)==0)
									{
										opMode=IDLE;
										break;
									}
									
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_COMM_DISABLE_NN_PROC:		//  disable the processor nubmer in bits 11-8 on chip on you port in bits 7-0
								count2=(p2pInAddress >> 16) & 0xF;   //port no
								count=(RxRoutingKey >> 24) & 0xFF; //proc no
								if(count2 > 5 || count > 19)
								{
									//send error frame
									sendErrorReportFrame(ERR_INVALID_OPTION1);
									opMode=IDLE;
									break;
								}
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									//p2pState=P2P_DISABLE_NN_PROC;
									//disable the said proc in the said chip and send ack frame
									//chipAddress in option3 (bit 15-0), port no (bit 23-16) and proc no (bit 31-24)
									count2=(p2pInAddress >> 16) & 0xF;  //port number
									if(count2 > 5)
										break;
									count=(p2pInAddress >> 24) & 0xFF; //proc no
									if(count > 19)
										break;
									val=(0x5EC <<20)|(0x1 << count);  //resetting a particular processor
									sendNNPacket(count2, DIRECT, SYSCTLR_BASE_U+SYSCTLR_CPU_DISABLE, val);    //writing to bit 0 of SYSCTLR_SBSYS_RST_LV register to reset the whole chip
									
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, count2, 0)==0)
									{
										opMode=IDLE;
										break;
									}
									
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_COMM_RESET_LINK:			//  reset your link in bits 11-0
								count2=(p2pInAddress >> 16) & 0xF;   //port no
								if(count2 > 5)
								{
									//send error frame
									sendErrorReportFrame(ERR_INVALID_OPTION1);
									opMode=IDLE;
									break;
								}
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									val=(p2pInAddress >> 16) & 0xF;
									if(val > 5)
										break;
									writeMemory(SYSCTLR_BASE_U+SYSCTLR_SBSYS_RST_LV, ( ( 0x1 << (4 + val) ) | ( 0x1 << (10 + val) ) ) );  //tx and rx link bit numbers in the register
									
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, count2, 0)==0)
									{
										opMode=IDLE;
										break;
									}
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_COMM_DISABLE_LINK:		//  disable your link in bits 11-0
								count2=(p2pInAddress >> 16) & 0xF;   //port no
								if(count2 > 5)
								{
									//send error frame
									sendErrorReportFrame(ERR_INVALID_OPTION1);
									opMode=IDLE;
									break;
								}
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									val = pload & 0xFFF; //link number to be reset
									if(val > 5)
										break;  //invalid link number
									writeMemory(SYSCTLR_BASE_U+SYSCTLR_LINK_DISABLE, ( (0x1 << (0 + val) ) | (0x1 << (16 + val) ) ) );  //tx and rx link bits in the register
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, count2, 0)==0)
									{
										opMode=IDLE;
										break;
									}
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_COMM_DATA_FOR_HOST:		//  data for the Host as a response to its request
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									//if(hostConnectedChip)  //if it is a host connected chip then
									//{
										//p2pState=P2P_DATA_FOR_HOST;
									//}
									//copy data into the txFrameBase and send frame
									//not implemented in host-connected chip
									;  //do nothing
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, count2, 0)==0)
									{
										opMode=IDLE;
										break;
									}
									
								}
								sendAckFrame();
								opMode=IDLE;
								break;
							case P2P_COMM_ACK:					//  ack from host
								if((p2pInAddress & 0xFFFF) == chipAddress) //chip address is the same as of this chip i.e. the frame is for this chip then
								{
									p2pAckDue=0;
									//it is an ack frame
								}
								else
								{
									if(p2pHostToOtherChip(p2pInAddress, p2pInInstruction, 0, 0)==0)
									{
										opMode=IDLE;
										break;
									}
								}
								sendAckFrame();
								opMode=IDLE;
								break;
								
							default:
								opMode=IDLE;
								break;
						
						}
						//						else  //it is for some other chip
						//						{
						//							rkey=((chipAddress << 16) | (p2pInAddress & 0xFFFF));   //source address, destination address
						//							pload=(0x0F8 << 20) | (p2pInInstruction << 12) | (p2pInDataSize);
						//							sendP2PPacket(0 /*sequence code*/, rkey, pload);
						//							currentP2PSeqNo=0;
						//							while(p2pAckPending)
						//								;   //wait for the ack to come 
						//								
						//							for(count=0; count<(p2pInDataSize/4); count++)
						//							{
						//								pload=p2pInDataBase[count];
						//								sendP2PPacket(((currentP2PSeqNo++)%3), rkey, pload);
						//								if(count%3 ==0) //wait for response after sending 4 packets i.e. sequence id 0-3
						//								{
						//									while(p2pAckPending)
						//										;   //wait for the ack to come 
						//								}
						//							}
						//							sendAckFrame();
						//						}
												//finished sending the data to a specific chip,
						//now send an ack frame to the Host
						
						opMode=IDLE;
						break;
	   				}
					case STIMULUS_IN:
					{
						#ifdef VERBOSE
							printf("proc %d: in STIMULUS_IN state!\n", procID);
						#endif
						//stimulus spike for the neurons in a processor in the destination chip, 
						//(once sent by the Ethernet, the instr in spinnaker instruction, size of data in option2,
						//chip address and procID in option3, the data contains the stimulus for each neuron i.e. the first byte byte of data is for neuron ID 0 in that processor, and
						//if the value is 1, stimulus is for that neuron etc)
						size = rxFrameBase[instructionOption2At/4];
						for(count=0; count< (size/4); count++)
						{
							sendMCPacketWOPL(rxFrameBase[(spinnakerDataAt/4) + count]);
							waitCycles(WAIT_BET_FF_PACKETS);
						}
						sendAckFrame();
						opMode=IDLE;
						break;
					}
					case RESPONSE_OUT:
					{
						#ifdef VERBOSE
							printf("proc %d: in RESPONSE_OUT state!\n", procID);
						#endif
						//will send a frame with response
						while(responseOutBase[count++] != 0)
						{
							txFrameBase[(spinnakerDataAt/4) + count]=responseOutBase[count];
							responseOutBase[count]=0;
						}
						
						//sendHelloFrame(); 
						formatAndSendFrame(RESPONSE_OUT, 0, count*4, 0);
						frameSent=1;
	         			ackFrameDue=1;
						opMode=IDLE;
						break;	
					}
					case P2P_OUT_COMM: 
					{
						//point to point packet from a particular chip to the HS, first word will be the instruction,
						//next word's first byte will be the control byte of the packet, next word will be the the routing key
						//to inform the HS about the source chip, while the next word will be the payload
					
							#ifdef VERBOSE
								printf("proc %d: in P2P_OUT_COMM state!\n", procID);
							#endif
							//the word after the length and instruction in the Ethernet frame i.e. rxFrmeBase[16/4] contains instruction for the specific chip 
							//allong with its address
							//in case of data transfer, the size of data to be sent or received is in the next word i.e. rxFrmeBase[20/4]
							txFrameBase[0]=(p2pOutInstruction >> 12) & 0xFF;  //bit 19-12 contains the instruction/msg for the Host
							txFrameBase[0]=(txFrameBase[0] << 16) | p2pOutAddress;   //bit 31-16 instruction/msg, and bit 15-0 the sender chip address 
							p2pOutDataSize=(p2pOutInstruction & 0x7FF);
							if(((p2pOutInstruction >> 12) & 0xFF) == P2P_COMM_DATA_FOR_HOST)
							{
								txFrameBase[1]=p2pOutDataSize; //11 bits containing the data size
								for(count=0; count <p2pOutDataSize/4; count++)
									txFrameBase[2+count]=p2pOutDataBase[count];
							}
								
							//in case of data to be sen, the data size is 
							formatAndSendFrame(P2P_OUT_COMM, p2pOutInstruction, 8+p2pOutDataSize, p2pOutAddress);
							ackFrameDue=1;
							opMode=IDLE;
							break;	
					}					
					case START_APPLICATION:    
					{
							//the start address to load application from in option1 field and a control sequence number (timestamp)
							//in the option3
							//the 4 words at this start address in the sdram contain information for the start application instruction
							//including the start address of the application, the start address of ITCM to load application to 
							//and the address to load into the pc (one word reserved for future use)
							#ifdef VERBOSE
								printf("proc %d: in START_APPLICATION state!\n", procID);
							#endif
							//send NN packet with instruction to start application
							rkey=(0x0F8 << 20) | (NN_CONF_APP_LOAD_START << 12);     //31-20 NN Intruction address space, 19-12 instruction
							val = (rxFrameBase[instructionOption3At/4] & 0xFFF);  //used to avoid looping around of the same instruction
							rkey= (rkey | val);  //
							appLoadStartInstrID=val;
							pload= rxFrameBase[instructionOption1At/4];  //option1=source address
							sendNNPacket(BROADCAST, NORMAL, rkey, pload);   
							//old implementation	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							//							rkey=(0x0F8 << 20) | (NN_CONF_APP_LOAD_TO << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
							//							rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[instructionOption3At/4];  //option3=destination address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							//							rkey=(0x0F8 << 20) | (NN_CONF_APP_LOAD_SIZE << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
							//							rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[spinnakerDataAt/4];  //option3=destination address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							//							rkey=(0x0F8 << 20) | (NN_CONF_APP_LOAD_START << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							pload= rxFrameBase[(spinnakerDataAt/4) +1];  //option3=destination address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//do it to yourself		
							//see which processors are alive and pass the message to only those who are alive								
			   				val = readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
			   				val = val & 0xFFFFF; //see the live processors
			   				//pass the message individually to all the processors
			   				for(count=0; count<20; count++)
			   				{
			   					if(((val >> count) & 0x1) && (procID != count)) //the proc is live and it is not the mon proc then
			   					{
			   						passMessage(SM_LOAD_APPLICATION, count, rxFrameBase[instructionOption1At/4], 16);  //move 16 bytes i.e. 4 words from the application load point in SDRAM 
			   																														//containing the address to load application from, address of ITCM to load application to
			   																														//the size of the application and the location of starting point in the application
			   					}
			   				}
			   				sendAckFrame();
							opMode=IDLE;
							//do something to start the application
							break;
					}
					case LOAD_APPLICATION_DATA: //execute application, i.e. load the application from the SDRAM to the fascicles and start the application
					{					//the instruction in spinnaker instruction field, option1=source address (SDRAM), option2=destination address(ITCM), 
							                //option3=size, datafield first word=main address (to move PC to)
							#ifdef VERBOSE
								printf("proc %d: in START_APPLICATION state!\n", procID);
							#endif
							//send NN packet with instruction to start application
							//							rkey=(0x0F8 << 20) | (NN_CONF_APP_LOAD_FROM << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
							//							rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[instructionOption1At/4];  //option1=source address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							//							rkey=(0x0F8 << 20) | (NN_CONF_APP_LOAD_TO << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
							//							rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[instructionOption3At/4];  //option3=destination address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							//							rkey=(0x0F8 << 20) | (NN_CONF_APP_LOAD_SIZE << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
							//							rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[spinnakerDataAt/4];  //option3=destination address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							val =rxFrameBase[instructionOption3At/4] & 0xFFF;  //used to avoid looping around of the same instruction
							rkey=(0x0F8 << 20) | (NN_CONF_LOAD_APP_DATA << 12);     //31-20 NN Intruction address space, 19-12 instruction
							rkey= (rkey | val);
							loadAppDataInstrID = val;
							pload= rxFrameBase[instructionOption1At/4];  //option1=source address
							sendNNPacket(BROADCAST, NORMAL, rkey, pload);   	
							//do it to yourself		
							//see which processors are alive and pass the message to only those who are alive								
			   				val = readMemory(SYSCTLR_BASE_U+SYSCTLR_SET_CPU_OK);
			   				val = val & 0xFFFFF; //see the live processors
			   				//pass the message individually to all the processors
			   				for(count=0; count<20; count++)
			   				{
			   					if(((val >> count) & 0x1) && (procID != count)) //the proc is live and it is not the mon proc then
			   					{
			   						passMessage(SM_LOAD_APPLICATION_DATA, count, rxFrameBase[instructionOption1At/4], 16);  //move 12 bytes i.e. 3 words from the data load point in SDRAM 
			   																															//containing the address to load data from, address of TCM to load data to
			   																															//the size of the data
			   					}
			   				}
			   				sendAckFrame();
							opMode=IDLE;
							//do something to start the application
							break;
					}
					case LOAD_MON_APPLICATION:   //instruction for the mon proc to load application and run it.
					{
							//send the nn packets before loading the application to its ITCM as the application may overwrite the existing code
							//rkey=(0x0F8 << 20) | (NN_CONF_MON_APP_LOAD_FROM << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//val =rxFrameBase[instructionOption3At/4] & 0xFFF;  //used to avoid looping around of the same instruction
							//
							//rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[instructionOption1At/4];  //option1=source address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							//							rkey=(0x0F8 << 20) | (NN_CONF_MON_APP_LOAD_TO << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
							//							rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[instructionOption3At/4];  //option3=destination address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							//							rkey=(0x0F8 << 20) | (NN_CONF_MON_APP_SIZE << 12);     //31-20 NN Intruction address space, 19-12 instruction
							//							val =rxFrameBase[instructionOption3At/4];  //used to avoid looping around of the same instruction
							//							rkey= (rkey | (val & 0xFFF));  //
							//							pload= rxFrameBase[spinnakerDataAt/4];  //option3=destination address
							//							sendNNPacketWOPL(BROADCAST, NORMAL, rkey);   	
							//							waitCycles(WAIT_BET_FF_PACKETS);
							rkey=(0x0F8 << 20) | (NN_CONF_MON_APP_START << 12);     //31-20 NN Intruction address space, 19-12 instruction
							val =rxFrameBase[instructionOption3At/4] & 0xFFF;  //used to avoid looping around of the same instruction
							monAppLoadStartInstrID=val;
							rkey= (rkey | val);  //
							pload= rxFrameBase[instructionOption1At/4];  //option1=destination address
							sendNNPacket(BROADCAST, NORMAL, rkey, pload); 
							//load itself  	
							sendAckFrame();
							opMode=IDLE;
							//val = rxFrameBase[instructionOption1At/4];
							loadApplicationAndStart(readMemory(pload), readMemory(pload+4), readMemory(pload+8), readMemory(pload+12));
							break;
					}	
					case REQUEST_NEIGHBOUR_FOR_MISSING_WORDS:
					{
						//load the current buffer and bitmap to a temporary location so that the next floodfill 
						//does not over write it and then ask the neighbour to send the missing words
						//						missingRxBufCRC=rxBufCRC;					//CRC received with the block
						//						missingRxBufSize=rxBufSize;				//block size received with frame i.e how large the buf would be
						//						missingRxBufStAddr=rxBufStAddr;
						//						for(count=0; count<47; count++)
						//							missingRxBitmap[count]=rxBitmap[count];
						//						for(count=0; count<380; count++)
						//							missingRxBuf[count]=rxBuf[count];
						count2=rxBufSize/32;
						for(count=0; count<count2 && count<47; count++)
						{
							if(rxBitmap[count] != 0xFFFFFF);
							{
								//check each word and get it from the neighbour
								int offset=0;
								unsigned int missingbitmap=rxBitmap[count];
								int count3=0;
								offset = rxBufSize % 32;
								for(count3=0; count3<32; count3++)
								{
									if(((missingbitmap >> count) & 0x1) == 0) //if the bit is not set, we need to get the word from the neighbour
									{
										int currentInstructionID=0, count4=0;
								  		unsigned int rkey=0, pload=0;
								   		rkey=(0x0F8 << 20) | (NN_FF_RESEND_BLOCK << 12);     //31-20 NN Intruction address space, 19-12 instruction
										currentInstructionID = currentTime + NN_FF_RESEND_WORD;  //used to avoid looping around of the same instruction
										rkey= (rkey | (currentInstructionID & 0xFFF));  //
										missingWordAddress = rxBufStAddr + (count3 + count*32);  //option3=destination address
										sendNNPacket(incomingPacketPort, NORMAL, rkey, pload); 
										while(missingWordWaiting && count4<MISSING_WORD_TIMEOUT_CYCLES)
											count4++;   //wait for the missing word
									}
								}
								if(offset > 0)
								{
									unsigned int value=rxBitmap[rxBufSize/32];
									for(count=0; count<offset; count++)
									{
										if(((value >> count) & 0x1) ==0)
										{
											int currentIntructionID=0, count4=0;
									  		unsigned int rkey=0, pload=0;
									   		rkey=(0x0F8 << 20) | (NN_FF_RESEND_BLOCK << 12);     //31-20 NN Intruction address space, 19-12 instruction
											currentIntructionID = currentTime + NN_FF_RESEND_BLOCK;  //used to avoid looping around of the same instruction
											rkey= (rkey | ( currentIntructionID & 0xFFF));  //
											missingWordAddress = rxBufStAddr + (count3 + count*32);  //option3=destination address
											sendNNPacket(incomingPacketPort, NORMAL, rkey, pload);   
											while(missingWordWaiting || count4<MISSING_WORD_TIMEOUT_CYCLES)
												count4++;   //wait for the missing word
										}
									}
								}
							}
						}
						break;
					}
					default:   //idle state, do nothing here as the next state is decided by the HandleReceivedFrame() method
					{
						//see if the time for response out

											
						//see if the time to send responses and status out
/*  deprecated CP 19th Jan 2010! 
					if(responseOutDue && !ackFrameDue)
						{
							for(count=0; (count < noOfResponsesReceived*2) && (count < 1420/4); count++)           //each response is a pair of two words i.e. timestamp and the routingkey received as a mc packet
								txFrameBase[(spinnakerDataAt/4)+count]=responseOutBase[count];
							formatAndSendFrame(RESPONSE_OUT, 0, noOfResponsesReceived*2*4, 0);
							responseOutReceived=0;
							currentResponseIndex=0;
							ackFrameDue=1;
						}
*/ //deprecated CP 19th Jan 2010! 



//						if(((currentTime % STATUS_OUT_INTERVAL)==0) && statusOutDue && !ackFrameDue)
//						{
//							for(count=0; count < no_of_status_received*2; count++)               //each response is a pair of two words i.e. timestamp and the routingkey received as a mc packet
//								txFrameBase[spinnakerDataAt+count]=statusBuffer[count];
//							formatAndSendFrame(RESPONSE_OUT, 0, no_of_status_received*2*4, 0);
//							ackFrameDue=1;
//						}
						break;
					}
				}
				
		}  
		else   //it is a fascicle processor or configDone is false
		{
			configDone=	MEM(globalVarSpace, SR_CONF_DONE);
		}
		#ifdef VERBOSE
			printf("chip: %d, proc %d: going to sleep\n", chipID, procID);
		#endif
		gotoSleep();
	}
	return 0;
}

//	instruction format:
//first 12(LSB) bits will be used for block size, remaining 11 bits will be used for instruction
//2K instructions can be created with this but we are starting from 1 and may go up to 256 instructions 
//i.e. leaving a cussion of 3 bits
//instruction 0: i.e. routingkey.range( 19, 12)=0  means sending broadcast message with block size given in routingkey.range(11, 0) and starting address in the payload
//instruction 1: i.e. routingkey.range(19, 12)=1 means end of current broadcast message
//instruction 2:								 means interrupt current broadcast
//instruction 3:								       resume broadcast
//			  4:									   block level checksum, size in the routingkey.range(11, 0), checksum in payload
//			  5:									   remaining of checksum (if more than 32 bits), sequence number in routingkey.range(11, 0)
//			  6:									   size of system in payload
//			  7:									   the time phase to be used is in the payload
//			  8:									   break the symmetery, assign address to yourself, the address of monitoring host will be as given in payload
//			  9:									   report ok (no payload)
//			  10:									   kernel completed, start running kernel instructions
//			  11:									   application download completed
//            20:									   a neighbor to other, hello i am live  (no payload)
//			  21:									   a neighbor to other, how are you (expect a response back i.e. a response with 'hello i am live' from that particular neighbor
//			  22:									   have you heard from my neighbor making triangle with us (link on one less the number of faulty link as in emergency routing) in routingkey.range(11, 0), no payload
//			  23:									   a neighbor to other, yes i have heard from our common neighbor 
//			  24:									   a neighbor to otehr, no i have not heard from this neighbor
//			  25:									   a neighbor to other, ask the common neighbor to reset the link towards me and resend hello message while i am doing this
		 	
 
//			  6:									   size of system in payload
//			  7:									   the time phase to be used is in the payload
//			  8:									   break the symmetery, assign address to yourself, the address of monitoring host will be as given in payload
//			  9:									   report ok (no payload)
//			  10:									   kernel completed, start running kernel instructions
//			  11:									   application download completed
//            20:									   a neighbor to other, hello i am live  (no payload)
//			  21:									   a neighbor to other, how are you (expect a response back i.e. a response with 'hello i am live' from that particular neighbor
//			  22:									   have you heard from my neighbor making triangle with us (link on one less the number of faulty link as in emergency routing) in routingkey.range(11, 0), no payload
//			  23:									   a neighbor to other, yes i have heard from our common neighbor 
//			  24:									   a neighbor to otehr, no i have not heard from this neighbor
//			  25:									   a neighbor to other, ask the common neighbor to reset the link towards me and resend hello message while i am doing this
			 	
