/*****************************************************************************************
*	Created by Cameron Patterson and Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/


/* ---------------------------- STATUS VARIABLES -----------------------------*/
unsigned int time = 0;				// Time since boot in milliseconds

unsigned int rx_frame[380];	//to hold 1500 bytes for the frame to receive
unsigned int tx_frame[380];

// All the below network data now is overwritten from Serial ROM, but variables do need to be defined here
unsigned int destinationMACAddressLS=0x0;  	
unsigned int destinationMACAddressHS=0x0;  	       
unsigned int sourceMACAddressLS=0x0;
unsigned int sourceMACAddressHS=0x0;
unsigned int sourceIPAddress=0x0;	
unsigned int destinationIPAddress=0x0; 

unsigned int spinnakerDataSize;
unsigned int macDestAddressConfigured=0;
unsigned int FFTarget=0;
unsigned int rxFrameSize=0;
unsigned short EthPacketID=0;
