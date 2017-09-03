/*****************************************************************************************
* 	Created by Cameron Patterson
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
*****************************************************************************************/

#include "../inc/globals.h"
#include <stdio.h> // To enable printing during debug

void delay(unsigned int delay)
{
	while(delay--);
}

unsigned short int phy_shift_in()						//
{
	int i;
	unsigned short int offwire = 0;
	pointer ETH_MII = (unsigned int *) ETH_MII_BASE; 
  
	for (i = 0; i < 16; i++)										// receive 16bits of input
	{
		if(ETH_MII[ETH_MII_PHY_CTRL] & 0x2) {						// if the SMI data input is high - we received a 1 
			offwire = (offwire << 1) | 0x1;  						// if received a 1, set a 1 in output variable and shift left
		} else {
			offwire = (offwire << 1);  								// if 0 then shift lsb=0 without setting a 1
		}
		ETH_MII[ETH_MII_PHY_CTRL] = 0x11;  							// Phy Cmd to clock in next bit as has received 
		delay(100);													// cp 7/Jan - add small delay, needs 160ns between MDC edges
		ETH_MII[ETH_MII_PHY_CTRL] = 0x1;  							// Phy on, clock bit cleared 
	}    
	return offwire;
}

void phy_shift_out(unsigned int command, unsigned int length)
{
	int i;
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;
     
	for(i = 0; i < length; i++)										// cycle through each bit of the command to send
	{
		if(command & 0x80000000) { 									// if the MSB is a 1
			ETH_MII[ETH_MII_PHY_CTRL] = 0xD; 						// prepare a 1 (SMI data output, enable, PHY on)
			delay(100);												// cp 6/May - add small delay, needs 160ns between MDC edges
			ETH_MII[ETH_MII_PHY_CTRL] = 0x1D; 						// clock out
		} else	{ 
			ETH_MII[ETH_MII_PHY_CTRL] = 0x9; 						// prepare a 0 (SMI data output, PHY on)
 			delay(100);												// cp 7/Jan - add small delay, needs 160ns between MDC edges
			ETH_MII[ETH_MII_PHY_CTRL] = 0x19; 						// clock out
		}
		command = command << 1;										// shift command 1 bit right, to process the next bit
		ETH_MII[ETH_MII_PHY_CTRL] &= 0xF;							// clear the clock bit
	}
}

unsigned short int phy_read(unsigned char phy_addr)
{
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;
	unsigned int phy_command = PHY_READ_COMMAND | (phy_addr << 18);	// Read | from register

	ETH_MII[ETH_MII_PHY_CTRL] = 0x9;  								// Enable SMI data output, phy on
	
	phy_shift_out(0xFFFFFFFF, 32); 									// shift out 32 1s as preamble to begin
	phy_shift_out(phy_command, 14);   								// send our phy command down to the phy
	phy_shift_out(0x2, 2);  										// turn around 2 bits (0b10)
	
	ETH_MII[ETH_MII_PHY_CTRL] = 0x1;  								// Phy normal (PHY on)

	return phy_shift_in();											// read 16 bits from PHY and return to callee
}

void phy_write(unsigned char phy_addr, unsigned short int value)
{
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;
	unsigned int phy_command = PHY_WRITE_COMMAND | (phy_addr << 18) | value;	// Write | to register | value 

	ETH_MII[ETH_MII_PHY_CTRL] = 0x9;								// SMI data output enable, PHY on
	phy_shift_out(0xFFFFFFFF, 32); 									// shift out 32 1s as preamble to begin
	phy_shift_out(phy_command, 32);   								// send our 32 bit phy command to the phy
	ETH_MII[ETH_MII_PHY_CTRL] = 0x1;  								// Phy normal (PHY on)
}

int test_phy() 
{
	pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
	unsigned int result;
	unsigned int receivedvalue, writtenvalue;
	unsigned int phy_ctrl_reg = phy_read(PHY_CTRL_REG);				// read the register so we can restore it later
    
    printf("Phy1\n");	//!!! TLM !!!
    writtenvalue=(PHY_EXISTENCE_TEST_BITS|phy_ctrl_reg);			// calculate value to write to register for test							 
    phy_write(PHY_CTRL_REG, writtenvalue);  						// write to the PHY control register
    printf("Phy2\n");	//!!! TLM !!!
    receivedvalue = phy_read(PHY_CTRL_REG);							// read back the register value
    printf("Phy3\n");	//!!! TLM !!!
   
    if (receivedvalue==writtenvalue) {   							// read should match written to validate PHY is present!
        phy_write(PHY_CTRL_REG, phy_ctrl_reg);  					// restore original register value
        result = 1;
    	printf("PhyYES\n");	//!!! TLM !!!
        SYS_CTRL[SYS_CTRL_SET_CPU_OK]=0x80000000;					// set bit 31 of CPU OK to 1 indicating PhyDetected
    } else {
    	result = 0;
	    printf("NOPhy\n");	//!!! TLM !!!
    	SYS_CTRL[SYS_CTRL_CLR_CPU_OK]=0x80000000;					// clear bit 31 of CPU status register indicating no Phy
    }
    
    return result;													// return success or fail to the callee
}


void print_phy_register_table() 
{
    pointer SYS_CTRL = (unsigned int*) SYS_CTRL_BASE;
    unsigned int i;												
    for (i=0;i<32;i++) { 
    	if (DEBUGPRINTF&&phy_present) { 
    		printf("PhyRead Reg:0x%x = 0x%x\n",i,phy_read(i));		// for routine to read and print table of Phy reg values
    	}
    }
}   
 
   	
