/*****************************************************************************************
* 	Created by Cameron Patterson   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/



#include "../inc/globals.h"
#include "../inc/dma.h"
#include "../inc/flood_sender.h"
#include "../inc/testers.h"
#include "../inc/peripheral_inits.h"
#include "../inc/chipnumbering.h"

#include <stdio.h> // To enable printing during debugging

#ifdef TEST_BOARD
// CP section for handling chip numbering on sending/receipt of NN type13 packets to/from neighbour

void chip_num_sender(uint my_chip_id, uint size_x, uint size_y)
{
	uint key, data;
	key = CHIP_NUMBER_CC << 24 | FF_PHASE_1;
	data = ((size_x << 24) | ((size_y&0xFF) << 16) | my_chip_id & 0xFFFF);
	// transmit(6, WITH_PAYLOAD, key, data);  	// this has been commented out, as broadcast can get blocked by any port being blocked
	transmit(EAST, WITH_PAYLOAD, key, data);	// now each direction is explicitly targetted, not a massive overhead
	transmit(NORTHEAST, WITH_PAYLOAD, key, data);
	transmit(NORTH, WITH_PAYLOAD, key, data);
	transmit(WEST, WITH_PAYLOAD, key, data);
	transmit(SOUTHWEST, WITH_PAYLOAD, key, data);
	transmit(SOUTH, WITH_PAYLOAD, key, data);
	//if (DEBUGPRINTF) printf("Chip Numberer Packet Sent: Me=0x%x, x=0x%x, y=0x%x\n",my_chip_id,size_x,size_y);
}  				

void chip_num_handler(uint data, uint rx_status)
{
	pointer CHIP_IDENTIFIER = (uint*) CHIP_ID;
	pointer COMMS_CTRL = (uint*) COMMS_CTRL_BASE;
//	pointer ROUTER = (unsigned int*) ROUTER_BASE;
//	uint i,j,p2ptablevector,p2ptableid,p2ptabledestx,p2ptabledesty,myxcoord,myycoord,vecX,vecY,vector;
	uint size_x,size_y,sending_x,sending_y;
	
	if(DEBUGPRINTF) printf("\nchip_num_handler from 0x%x\n",(rx_status>>24)&0x7);					// CP debug
	if ((*CHIP_IDENTIFIER & 0xFFFF0000) != SECURITY_CODE) {  
		// read my chipid, if security code in place then already done (also protects against loops)
		size_x = ((data>>24)&0xFF);
		size_y = ((data>>16)&0xFF);
		sending_x = ((data>>8)&0xFF);
		sending_y = (data&0xFF);
		
		testers_TEMP_DISABLE_WATCHDOG();
		
		switch((rx_status>>24)&0x7) {
			// if arrived on EXT0: x-1 y=y. EXT 1: x-1 y=-1. EXT 2: x=x y-1. EXT 3: x+1 y=y. EXT 4: x=+1 y+1. EXT 5: x=x y+1
			case 0:	{*CHIP_IDENTIFIER= SECURITY_CODE | sending_x ? (sending_x-1)<<8 : (size_x-1)<<8 | sending_y;	break;}
			case 1:	{*CHIP_IDENTIFIER= SECURITY_CODE | sending_x ? (sending_x-1)<<8 : (size_x-1)<<8 | sending_y ? sending_y-1 : size_y-1;	break;}
			case 2:	{*CHIP_IDENTIFIER= SECURITY_CODE | (sending_x)<<8	  |	sending_y ? sending_y-1 : size_y-1;		break;}
			case 3:	{*CHIP_IDENTIFIER= SECURITY_CODE | ((sending_x+1)%size_x)<<8	  |	sending_y;					break;}
			case 4:	{*CHIP_IDENTIFIER= SECURITY_CODE | ((sending_x+1)%size_x)<<8	  |	(sending_y+1)%size_y;		break;}
			case 5:	{*CHIP_IDENTIFIER= SECURITY_CODE | (sending_x)<<8				  |	(sending_y+1)%size_y;		break;}
			default:{break;}		
		}
		
		
		#ifdef SPINNAKERTESTCHIP											// testchip
			COMMS_CTRL[COMMS_CTRL_TX_CTRL] &= *CHIP_IDENTIFIER&0xFFFF; 		// Set P2P sourceID, keep existing register as is
		#endif
		#ifdef SPINNAKER2													// spinnaker2
			COMMS_CTRL[COMMS_CTRL_SOURCE_ADDR] &= *CHIP_IDENTIFIER&0xFFFF; 	// SPINN2 SourceID is in new SAR
		#endif
	

		p2p_algorithmically_fill_table(size_x, size_y); 					// populate P2P routing table algorithmically 
																				// (note: does not take account of any faults in the system)
	
		LED_FLASH_INTERVAL = 0x100;  // This flashes the LEDs periodically to indicate that numbering is achieved on this chip
			
		if (DEBUGPRINTF) printf("Chip Numberer: Me=0x%x, xsize 0x%x, ysize 0x%x. SendX: 0x%x, SendY: 0x%x.\n",
																*CHIP_IDENTIFIER,size_x,size_y,sending_x, sending_y);
		chip_num_sender((uint)(*CHIP_IDENTIFIER & 0xFFFF), size_x, size_y); 
			// send it on! (always 'broadcast'). Those already in possession of valid chipid will ignore it.
			//		this means that the initial numbering 'broadcast' storm will die off
			
		init_watchdog();
	}
			
}				

// populate P2P routing table algorithmically (note: does not take account of any faults in the system)

void p2p_algorithmically_fill_table(uint size_x, uint size_y) 
{

	pointer CHIP_IDENTIFIER = (uint*) CHIP_ID;
	pointer ROUTER = (unsigned int*) ROUTER_BASE;
	uint i,j,p2ptablevector,p2ptableid,p2ptabledestx,p2ptabledesty,myxcoord,myycoord,vecX,vecY,vector;
		
	myxcoord=(*CHIP_IDENTIFIER&0xFF00)>>8;	// tables are based on position of this chip/router
	myycoord=(*CHIP_IDENTIFIER&0xFF);		// and algorthmically calculated
		
	for (i=0;i<ROUTER_P2P_TABLE_SIZE;i++) {			// 24bit table 8*3bit outputs.
		p2ptablevector=0;
		for (j=0;j<8;j++) {							// 24bit table 8*3bit outputs.
			p2ptableid=(i*8)+j;						// Destination we are populating. Testchip has 2048,  spinn2 = 65536.
			p2ptabledestx=p2ptableid>>8;			// destination x coordinate this table ID represents (testchip x-max=8 (3bits))
			p2ptabledesty=p2ptableid&0xFF;			// destination y coordinate this table ID represents (8 bits)
	
			if((p2ptabledestx<size_x)&&(p2ptabledesty<size_y)) {	// only calculate entries that are within machine size
	
				if (p2ptabledestx==myxcoord) {		// calculate which direction we must travel in the x direction
					vecX=STAY;						// if we are already at the correct x axis
				} else {
					if (((p2ptabledestx-myxcoord)%size_x) <= (size_x/2)) {	
						vecX=EAST;					//if we have to move, check if its best to travel +x
					} else {													
						vecX=WEST;					//otherwise the move we make is to travel -x
					}
				}

				if (p2ptabledesty==myycoord) {		// calculate which direction we must travel in the y direction
					vecY=STAY;						// if we are already at the correct y axis
				} else {
					if (((p2ptabledesty-myycoord)%size_y) <= (size_y/2)) { 	
						vecY=NORTH;					//if we have to move, check if its best to travel +y
					} else {											
						vecY=SOUTH;					//otherwise the move we make is to travel -y
					}
				}
				
							
				if (vecX==STAY && vecY==STAY)	vector=P2PMON;			// my own entry, set vector monitor proc
				if (vecX==STAY && vecY==NORTH) 	vector=NORTH;			// go North		
				if (vecX==STAY && vecY==SOUTH)	vector=SOUTH;			// go South
				if (vecX==WEST && vecY==STAY)	vector=WEST;			// go West
				if (vecX==WEST && vecY==NORTH)	vector= (p2ptableid%2) ? NORTH : WEST;		
							// need to go NorthWest. However no direct NW path, so randomise N or W based on my odd/even y-coord
				if (vecX==WEST && vecY==SOUTH)	vector=SOUTHWEST;		// go SouthWest
				if (vecX==EAST && vecY==STAY)	vector=EAST;			// go East
				if (vecX==EAST && vecY==NORTH)	vector=NORTHEAST;		// go NorthEast
				if (vecX==EAST && vecY==SOUTH)	vector= (p2ptableid%2) ? SOUTH : EAST;
							// need to go SouthEast. However no direct SE path, so randomise E or S based on my odd/even y-coord
	
				if (DEBUGPRINTF) printf("(%d,%d), VecX:%x, VecY:%x. vector:%x.\n",p2ptabledestx,p2ptabledesty,vecX,vecY,vector);				
			} else {
				vector=SPINN2DROP;
			}
			p2ptablevector|=(vector<<(3*j));	// shift the vector to the appropriate position in the P2P entry
			
		}			
		//if (DEBUGPRINTF) printf("P2P Route Table Population: WordEntry (0x%x): 0x%x\n",i, p2ptablevector);
		ROUTER[ROUTER_P2P_TABLE + i] = p2ptablevector;
	}


}

		
#endif
