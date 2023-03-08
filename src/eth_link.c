/*****************************************************************************************
* 	Created by Cameron Patterson and Thomas Sharp
*
* Copyright (c) 2008 The University of Manchester
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     https://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************************/

#include "../inc/dma.h"
#include "../inc/globals.h"


// If TX currently in progress:
//		return 0
// otherwise, write length field and place data in tx buffer
// wait for dma to complete
// write arbitrary value to tx command reg to start transfer
// return 1
int send_frame(int length)
{
	pointer ETH_MII = (unsigned int*) ETH_MII_BASE;

	if( (ETH_MII[ETH_MII_GENERAL_STATUS] & ETH_MII_TX_ACTIVE) || (length > 1500+14) )
	{
		return 0; 		// h/w busy or length too long to be a valid Ethernet frame
	}

	// Eth frames must be no shorter than 60 bytes
	ETH_MII[ETH_MII_TX_LENGTH] = ((length < 60) ? 60 : length);
	if (length%4) length+=(4-(length%4));	// DMA transfers, multiples of 32-bit words. Adding to ensure any part-full word also sent.

	dma(ETH_MII_BASE + ETH_MII_TX_FRAME_BUFFER, (unsigned int) tx_frame, DMA_CRC_OFF, DMA_WRITE, length);

	ETH_MII[ETH_MII_TX_COMMAND] = 0x1;		// send frame by writing arbritary value to TX command register
	return 1;
}

void handleArpRequest()
{
	tx_frame[0]=(rx_frame[1] & 0xFFFF)<<16 | (rx_frame[2] & 0xFFFF0000) >> 16;
	tx_frame[1]=(rx_frame[2] & 0xFFFF)<<16 | sourceMACAddressHS & 0xFFFF;
	tx_frame[2]=sourceMACAddressLS;	// swap Src to Dest MAC, and add local MAC for Src
	tx_frame[3]=rx_frame[3];		// ethertype, & ARP h/w type stay the same
	tx_frame[4]=rx_frame[4];		// ARP prot type, and hw/prot sizes also stay the same
	tx_frame[5]=(0x2<<16)|sourceMACAddressHS & 0xFFFF; // opcode reply=2, followed by source MAC
	tx_frame[6]=sourceMACAddressLS;	// addresses once again
	tx_frame[7]=sourceIPAddress;
	tx_frame[8]=tx_frame[0];		// MSWord of DMAC address again
	tx_frame[9]=(tx_frame[1] & 0xFFFF0000) | (rx_frame[7]>>16) & 0xFFFF;  // LS2Bytes of Dest MAC + 1st 2 bytes dest IP
	tx_frame[10]=(rx_frame[7]<<16) & 0xFFFF0000; // final 2 bytes of Dest IP address

	send_frame(42);  // send ARP reply on it's way, 42byte length frame (padded later). If sending fails then ignore, it's not critical.
}

void handlePingRequest()
{
	unsigned int chk, size, sizeinwords, remainder, i, sum=0;

	size = (rx_frame[4] >> 16)+14;		//ethernet frame size in bytes
	sizeinwords = size/4;				//number of full words
	remainder = size%4;					//remainder (if non-zero then we have an extra non-full word)

	for(i=3;i<=sizeinwords;i++) tx_frame[i]=rx_frame[i];
		// copy received frame across as template tx reply (inc. any remainder word)

	// zero any appending bytes so not to skew the icmp checksum
	if (remainder==0) tx_frame[sizeinwords]=0x0;			//zero the the remainder word (not needed)
	if (remainder==1) tx_frame[sizeinwords]&=0xFF000000;	//just 1 byte of real data in last word, zero 3 of them!
	if (remainder==2) tx_frame[sizeinwords]&=0xFFFF0000;	//2 bytes of real data in last word, zero 2 of them
	if (remainder==3) tx_frame[sizeinwords]&=0xFFFFFF00;	//3 bytes of real data in last word, zero last byte

	tx_frame[0]=(rx_frame[1] & 0xFFFF)<<16 | (rx_frame[2] & 0xFFFF0000) >> 16;
	tx_frame[1]=(rx_frame[2] & 0xFFFF)<<16 | (rx_frame[0] & 0xFFFF0000) >> 16;
	tx_frame[2]=(rx_frame[0] & 0xFFFF)<<16 | (rx_frame[1] & 0xFFFF0000) >> 16; // swap Dest and Src MACs
	tx_frame[6]=(rx_frame[7] & 0xFFFF);                                  // clear IP checksum and move...
	tx_frame[7]=(rx_frame[8] & 0xFFFF0000) | (rx_frame[6] & 0xFFFF);  // ...dest and source IPs...
	tx_frame[8]=(rx_frame[7] & 0xFFFF0000);                              //...and set command to echo reply
	tx_frame[9]=(rx_frame[9] & 0xFFFF);       // type = 0, code = 0, and clearout ICMP checksum field

	//perform the IP header checksum calculation
	for(i=3; i<8; i++) sum+= ((tx_frame[i] & 0x0000FFFF) + ((tx_frame[i+1] & 0xFFFF0000)>>16));
		// this for loop calculates a sum of the 20bytes as 16bit chunks for the IP header checksum
	while(sum&0xFFFF0000) sum = (sum & 0xFFFF)+(sum >> 16); //rolls over carry, and adds as required
	chk = 0xFFFF-sum; //1s complement the rolled sum for the checksum
	tx_frame[6]=(chk << 16) | tx_frame[6];  //adds the ip checksum to the correct field in the IP header

	//perform the ICMP checksum calculation
	sum= ((tx_frame[8]&0xFF00) + tx_frame[8]&0xFF); // 1st 2 bytes of checksum for ICMP chk
	for(i=9; i<=sizeinwords; i++) sum+= ( ((tx_frame[i]&0xFFFF0000)>>16) + (tx_frame[i]&0xFFFF));
		// this for loop chunks the ICMP data into 16bit segments and adds for the ICMP checksum
	while(sum&0xFFFF0000) sum = (sum & 0xFFFF)+(sum >> 16); //rolls over carry and adds as required
	chk = 0xFFFF-sum; //1s complement the rolled sum for the checksum
	tx_frame[9]=(chk << 16) | tx_frame[9]; // adds ICMP checksum to correct field in ICMP header

	send_frame(size);  // send it on it's way.  If sending fails then ignore, it's not critical.
}


void receive_frame(uint tcm_address)
{
    pointer ETH_MII = (uint*) ETH_MII_BASE;
    unsigned int desc, double_dma, frame_length, length, rx_buffer, start_address;

    desc = ETH_MII[ETH_MII_RX_DSC_RD_PTR];
    desc = (ETH_MII_RX_DESC_RAM + (desc & 0x3F));

    frame_length = ETH_MII[desc];
    frame_length = (frame_length & 0x7FF) + 4;
    start_address = ETH_MII[ETH_MII_RX_BUF_RD_PTR] & 0xFFF;
	double_dma = (start_address + frame_length) > 0xC00;

	rx_buffer = ETH_MII_RX_FRAME_BUFFER + start_address;
	length = double_dma ? 0xC00 - start_address : frame_length;

	dma(rx_buffer, tcm_address, DMA_CRC_OFF, DMA_READ, length);   		// we ignore any DMA failure

	if(double_dma)
	{
		rx_buffer = ETH_MII_RX_FRAME_BUFFER;
		tcm_address += length;
		length = frame_length - length;

		dma(rx_buffer, tcm_address, DMA_CRC_OFF, DMA_READ, length);		// we ignore any DMA failure
	}
    ETH_MII[ETH_MII_RX_COMMAND] = 0xF00;								// mark our frame as read
}


int formatAndSendFrame(unsigned int instruction, unsigned int option1, unsigned int option2, unsigned int option3)
{
	unsigned int i, sum = 0, length = option2 + 60;		//option2 carries spinnaker optional data payload length, add to this the ethernet/ip/udp/spin header overheads

	tx_frame[0] = (destinationMACAddressHS << 16) | (destinationMACAddressLS >> 16);	//4 MSBytes of  Dest MAC address
    tx_frame[1] = (destinationMACAddressLS << 16) | (sourceMACAddressHS & 0xFFFF);		//2 LSBytes of DMAC, first 2 MSB of SMAC
    tx_frame[2] = sourceMACAddressLS;													//4 LSBytes of S MAC address
    tx_frame[3] = (IP_ETHERTYPE << 16) | IP_VER_LEN_TOS;     							//ethernet type, IP version, header len, type of service
    tx_frame[4] = ((20 + 8 + 18 + option2) << 16) | EthPacketID++;  					//IP Sz = 20(IP)+8(UDP)+18(SpiNN)+SpiNPayload(Extras),PktID
    tx_frame[5] = IP_FRAG_FLAGS_TTL_PROT;												//Frag, flags, offset, TTL, IP Protocol UDP
    tx_frame[6] = 0x0 | (sourceIPAddress >>16);											//IP Hdr Chk (init 0 - see later) | 2 octets SrcIP address
    tx_frame[7] = (sourceIPAddress << 16) | (destinationIPAddress >>16);				//last 2 octets SrcIP, 1st two of DstIP
    tx_frame[8] = (destinationIPAddress << 16) | SPINN_PORT;							//last 2 octets of DstIP, Source UDP port
    tx_frame[9] = ((uint) SPINN_PORT << 16)  | (8 + 18 + option2);								//Dest UDP port, UDP Length (UDP Headers+SpiNN+Extras)
    tx_frame[10] = 0x0 | SPINNPROTVERSION;												//Chk UDP Pseudo Hdr = 0 (as optional) | SpiNNaker Protocol Version

    // calculate the IP header checksum over the 20byte header - increment tally by adding 1st & 2nd 16bits from each header word
    for(i = 3; i < 8; i++)
    {
    	sum += (tx_frame[i] & 0xFFFF) + (tx_frame[i + 1] >> 16);
    }
	while(sum & 0xFFFF0000)
	{
		sum = (sum & 0xFFFF) + (sum >> 16);  //sums the two 16bit shorts from each word until no 16bit carry remains
	}
    sum = 0xFFFF - sum;  // invert the result - which is the 1s complement of the IP header - the IP header checksum.

    tx_frame[6] = (sum << 16) | tx_frame[6];  //insert the ip checksum in the correct position in the header

	tx_frame[11] = instruction;	// Place SpiNNaker protocol data in UDP packet
	tx_frame[12] = option1;
	tx_frame[13] = option2;
	tx_frame[14] = option3;

	send_frame(length);
	return length;
}

void send_hello_frame()
{
	formatAndSendFrame(HELLO, (VERSION << 24) | (SUBVERSION << 16) | 0, 0, 0x43505453);
		//instruction, option1=version codes of ROM/library, option2=optional payload (0) size, 3 = procID and core for ID (n/a)
		//using this as a notification to the local LAN that we are alive in main()
}
