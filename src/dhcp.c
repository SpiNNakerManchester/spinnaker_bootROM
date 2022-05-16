/*****************************************************************************************
*	dhcp.c - all about the DHCP supporting code 
*
*
*	Created Chopped Hacked and Sliced by Cameron Patterson
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008-11. All rights reserved.
*****************************************************************************************/

#include "../inc/globals.h"
#include "../inc/dhcp.h"
#include "../inc/processor.h"
#include "../inc/eth_link.h"

/* --------------------------------- For DHCP additions ----------------------------------*/
#ifdef DHCP

uchar dhcp_learned = 0;
uint dhcpipleasetime=0;	
unsigned int dhcpstarted;
unsigned int dhcpattemptnum=0;				// number of times to reattempt getting a DHCP address before giving up.
unsigned char dhcpstate=DHCPNOTSTARTED;			// CP DHCP - state at the outset
unsigned char persistip[4],persistsnm[4],persistrtr[4],persistdhcpserver[4];	// CP DHCP - for verification of DHCP address from server
unsigned char local_ip[4];
unsigned char host_mac[6];
unsigned char local_mac[6];
eth_t* eth = (eth_t*) ETH_MII_BASE;
sys_ram_t* sys_ram = (sys_ram_t*) SYS_RAM_BASE;


uint ip_sum(uchar *d, uint len, uint sum) 
{
	if(len & 1) sum += d[--len] << 8;

	for(uint i = 0; i < len; i += 2) sum += (d[i] << 8) + d[i + 1];

	while(sum >> 16) sum = (sum >> 16) + (sum & 0xffff);

	return sum;
}

void copy_ip(uchar *a, uchar *b)
{
	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
	a[3] = b[3];
}

void copy_mac(uchar *a, uchar *b)
{
	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
	a[3] = b[3];
	a[4] = b[4];
	a[5] = b[5];
}

uint eth_tx(uint *frame, uint length)
{
/*
	disable_interrupts();

	while(eth->general_status & 0x1); //TODO a timeout?
	
	for(uint i = 0; i < length / 4 + 1; i++) eth->tx_frame_buf[i] = HTONL(frame[i]);
	
	if(length < 60) length = 60; // Ensure minimum frame length requirement is met
	
	eth->tx_length = length;
	eth->tx_command = 1; // Write arbitrary value to tx_command to send frame
	
	enable_interrupts();

*/


	//while(eth->general_status & 0x1); 	//TODO a timeout? Done - see below

	uint j=10000;				// loop around only 10K times waiting for the Ethernet
	do { 
	    if (!eth->general_status & 0x1) {	// if Ethernet not in use transmitting
	        disable_interrupts();		// stop interrupts stealing our ability to control the Ethernet
		for(uint i = 0; i < length / 4 + 1; i++) eth->tx_frame_buf[i] = HTONL(frame[i]); // copy the data across to the TX buffer. DMA?  per lines 29-31 eth_link.c - not performance critical
		if(length < 60) length = 60; 	// Ensure minimum frame length requirement is met
		eth->tx_length = length;	// set tx length field in Eth controller 
		eth->tx_command = 1; 		// Write arbitrary value to tx_command to send frame
		enable_interrupts();		// can receive interrupts again now
		j=0;				// it's been sent so we can exit the do-while loop
	    }
	} while (j-->0);				// CP needs to be --j  in order to timeout, otherwise just a loop
	
	return 1;
}

uint dhcp_tx(uchar* current_local_ip, uchar mssg_type, uchar extra_options_length, uchar* extra_options)
{
	int dhcp_header_size = 240; // CP DHCP - length of DHCP header
	int length = 8+extra_options_length;  // CP DHCP - additional length of DHCP payload for discover
	int i,j;

	eth_frame_t txf;
	eth_frame_t *tx_frame = &txf;
	ip_pkt_t *tx_ip_pkt = (ip_pkt_t *) tx_frame->payload;
	udp_pkt_t *tx_udp_pkt = (udp_pkt_t *) tx_ip_pkt->payload;
	dhcp_pkt_t *dhcp_pkt = (dhcp_pkt_t *) tx_udp_pkt->payload;

	//copy_mac(tx_frame->dst_mac, host_mac);
	for(i=0;i<6;i++) tx_frame->dst_mac[i]=0xFF; // set to broadcast request
	copy_mac(tx_frame->src_mac, sys_ram->mac_addr);
	
	tx_frame->type = ETH_TYPE_IP;

	tx_ip_pkt->ver_len = IP_VER_LEN;
	tx_ip_pkt->DS = IP_DS;

	tx_ip_pkt->length = HTONS(IP_HEADER_SIZE + UDP_HEADER_SIZE + dhcp_header_size + length);
	tx_ip_pkt->ident = IP_IDENT_ZERO;
	tx_ip_pkt->flg_off = IP_DONT_FRAGMENT;
	tx_ip_pkt->TTL = IP_TTL;
	tx_ip_pkt->protocol = IP_PROTOCOL_UDP;
	tx_ip_pkt->checksum = 0;
	copy_ip(tx_ip_pkt->src_ip, current_local_ip);
	for(i=0;i<4;i++) tx_ip_pkt->dst_ip[i] = 0xFF;	// set to broadcast
	
	tx_ip_pkt->checksum = HTONS(~ip_sum((uchar *) tx_ip_pkt, IP_HEADER_SIZE, 0));

	tx_udp_pkt->src_port = HTONS(68); // CP DHCP
	tx_udp_pkt->dst_port = HTONS(67); // CP DHCP

	tx_udp_pkt->length = HTONS(UDP_HEADER_SIZE + dhcp_header_size + length); // CP DHCP
	tx_udp_pkt->checksum = 0;

	dhcp_pkt->op=1; // Request
	dhcp_pkt->htype=1; // Ethernet
	dhcp_pkt->hlen=6; // h/w addr len (MAC)
	dhcp_pkt->hops=0; // not a relay agent
	dhcp_pkt->xid=sourceMACAddressLS;
	       // unique id - usually randomised - last 4 octets of MAC should be unique
	dhcp_pkt->secs=HTONS(dhcpstarted/1000); // # secs since addr aquisition began
	dhcp_pkt->secs=0;
	dhcp_pkt->flags=0x80;			// ask for broadcast DHCP replies - for debugging  visibility
	for(i=0;i<4;i++) dhcp_pkt->ciaddr[i]=0;
	for(i=0;i<4;i++) dhcp_pkt->yiaddr[i]=0;
	for(i=0;i<4;i++) dhcp_pkt->siaddr[i]=0;
	for(i=0;i<4;i++) dhcp_pkt->giaddr[i]=0; // all unknown at present so zeroed
	copy_mac(dhcp_pkt->chaddr,sys_ram->mac_addr); // Local MAC
	for(i=0;i<10;i++) dhcp_pkt->chaddrpad[i]=0; // padded with zeroes (probably uneccesary)
	for(i=0;i<64;i++) dhcp_pkt->sname[i]=0; // not known, that's why we're looking!
	for(i=0;i<128;i++) dhcp_pkt->file[i]=0; // not relevant, not bootp
	dhcp_pkt->cookie=0x63538263;	// bootp magic cookie
	i=0;
	dhcp_pkt->options[i++]=0x35;
	dhcp_pkt->options[i++]=0x1;
	dhcp_pkt->options[i++]=mssg_type; // DHCP message type
	for(j=0;j<extra_options_length;j++) dhcp_pkt->options[i++]=extra_options[j]; // pass in any other options required
	dhcp_pkt->options[i++]=0x37;
	dhcp_pkt->options[i++]=0x2;
	dhcp_pkt->options[i++]=0x1;
	dhcp_pkt->options[i++]=0x3; // Ask for Mask, and Router(Gateway) fields
	dhcp_pkt->options[i++]=0xFF; // end of options
	for(j=i;j<length;j++)dhcp_pkt->options[j]=0x0; // add any 0 padding required 

	return eth_tx((uint *) tx_frame, ETH_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + dhcp_header_size + length);
 }
 
 
 uchar dhcp_pkt(eth_frame_t *rx_frame, uint rx_len)
{
	ip_pkt_t *rx_ip_pkt = (ip_pkt_t *) rx_frame->payload;
	udp_pkt_t *rx_udp_pkt = (udp_pkt_t *) rx_ip_pkt->payload;
	dhcp_pkt_t *dhcp_pkt = (dhcp_pkt_t *) rx_udp_pkt->payload;

 	uchar optioncode = 0, optionlen = 0, dhcpmessagetype = 0, mask = 0, router = 0;
	uchar snm[4],rtr[4];

	int *frame = (int *) rx_frame;

	//sys_ram->ff_space[0]=rx_len;// **** CP DEBUGGING  when entering this routine set variable to say length of packet
	
	for(int i = 0; i < (rx_len / 4); i++) frame[i] = HTONL(frame[i]);  // CP: is this right? Last word may not get swopped... len=4:1word, len=5:1word, len=6:1word, len7:1word, len8:2words
	
	// formatAndSendFrame(HELLO,  dhcp_pkt->op, dhcp_pkt->htype, dhcp_pkt->hlen, 0);  // Send D to say I got here! 

	// will get here if waiting for reply to discover (SELECTING mode), or for an ACK/NAK to a request (REQUESTING) 
	if(dhcp_pkt->xid==sourceMACAddressLS) {				
		// ensure the transaction ID is correct, and its a bootp reply message for us to pay attention to
		// spinn_packet_tx(0xc3010000, persistip[0]<<24+persistip[1]<<16+persistip[2]<<8+persistip[3], persistsnm[0]<<24+persistsnm[1]<<16+persistsnm[2]<<8+persistsnm[3], persistrtr[0]<<24+persistrtr[1]<<16+persistrtr[2]<<8+persistrtr[3],(uchar *) 0,0);
		uint position = 0;						// Data is at 0x11A, where 1st option will be (just after magic cookie)
		while ((position+0x11A)<(HTONS(rx_ip_pkt->length)+ETH_HEADER_SIZE)){	// got to make sure we don't fall off the end of a packet
			optioncode=dhcp_pkt->options[position++];		// read option code of triplet
			if (optioncode == 0xFF) break;				// got to the end of the options already (end option is a 0xFF)
			
			optionlen=dhcp_pkt->options[position++];		// read length of option data (in bytes)

			if((optioncode == 53) && (optionlen == 1)) {		// DHCP message type, expecting 1 byte of data
				dhcpmessagetype = dhcp_pkt->options[position++];// read what the DHCP message type is
			} else if((optioncode == 1) && (optionlen == 4)) {	// Subnet Mask Option Data is 4 bytes long
				snm[0]= dhcp_pkt->options[position++]; 
				snm[1]= dhcp_pkt->options[position++];
				snm[2]= dhcp_pkt->options[position++];
				snm[3]= dhcp_pkt->options[position++];
				mask = 1;	// mark mask received
			} else if((optioncode == 3) && (optionlen == 4)) {	// Router Address Option Data is 4 bytes long
				rtr[0]= dhcp_pkt->options[position++]; 
				rtr[1]= dhcp_pkt->options[position++];
				rtr[2]= dhcp_pkt->options[position++];
				rtr[3]= dhcp_pkt->options[position++];
				router = 1;	// mark router received
			} else if((optioncode == 51) && (optionlen == 4)) {	// Router Address Option Data is 4 bytes long
				dhcpipleasetime=dhcp_pkt->options[position++]<<24; 
				dhcpipleasetime+=dhcp_pkt->options[position++]<<16; 
				dhcpipleasetime+=dhcp_pkt->options[position++]<<8;
				dhcpipleasetime+=dhcp_pkt->options[position++];
			} else {
				position+=optionlen;				// Not interested in this option. 
			}
			//spinn_packet_tx(0xc2010000, position+0x11A, (HTONS(rx_ip_pkt->length)+ETH_HEADER_SIZE), optioncode,(uchar *) 0,0);
		} // end of DHCP packet, all bytes consumed
	

		//spinn_packet_tx(0xc2010000, mask, router, dhcpmessagetype,(uchar *) 0,0);
		
		if (dhcpstate==DHCPSELECTING && dhcpmessagetype == 2 && mask == 1 && router == 1) { 		// DHCP offer with all required data received
			// if DHCP offer then keep IP address, Mask and G/W in memory.  Send a Request with this info,  wait for an ACK back.
			for(char i=0;i<4;i++) {
				persistip[i]=dhcp_pkt->yiaddr[i];
				persistsnm[i]=snm[i];
				persistrtr[i]=rtr[i];
				persistdhcpserver[i]=rx_ip_pkt->src_ip[i];
				
			}
			dhcpstate=DHCPREQUESTING; 
			uchar j=0;
			uchar extra_dhcp_options[12];	// extra options declared
			extra_dhcp_options[j++]=0x36;
			extra_dhcp_options[j++]=0x4;
			for(char i=0;i<4;i++) extra_dhcp_options[j++]=persistdhcpserver[i];  // Option DHCP server selected
			extra_dhcp_options[j++]=0x32;
			extra_dhcp_options[j++]=0x4;
			for(char i=0;i<4;i++) extra_dhcp_options[j++]=persistip[i];  // Option DHCP address requested
			//sys_ram->ff_space[1]=0xBEEF1;// **** CP DEBUGGING
			dhcp_tx(local_ip, DHCP_REQUEST_TYPE, 12, extra_dhcp_options); 	// ask for this server to grant this offer (->REQUESTING mode)
			//sys_ram->ff_space[2]=0xBEEF2;// **** CP DEBUGGING
			//dhcp_request_tx();						// ask for this server to grant this offer (->REQUESTING mode)
			//spinn_packet_tx(0xc4010000, (persistip[0]<<24)+(persistip[1]<<16)+(persistip[2]<<8)+persistip[3], (persistsnm[0]<<24)+(persistsnm[1]<<16)+(persistsnm[2]<<8)+persistsnm[3], (persistrtr[0]<<24)+(persistrtr[1]<<16)+(persistrtr[2]<<8)+persistrtr[3],(uchar *) 0,0);
										// for testing, send hello message with this info as the options!
		}

		if (dhcpstate==DHCPREQUESTING && dhcpmessagetype == 5 && mask == 1 && router == 1) { 		// DHCP ACK with correct data
			// if DHCP ack validate same IP address, Mask and G/W received as in memory. 
			
			//sys_ram->ff_space[3]=0xBEEF3;// **** CP DEBUGGING

			int mismatch = 0; 						// start with the assumption they will match

			for(char i=0;i<4;i++) {
				if(persistip[i]!=dhcp_pkt->yiaddr[i]) mismatch = 1;
				if(persistsnm[i]!=snm[i]) mismatch = 1;
				if(persistrtr[i]!=rtr[i]) mismatch = 1;
			}
			//sys_ram->ff_space[4]=0xBEEF4;// **** CP DEBUGGING
			if (mismatch == 0) {
				//sys_ram->ff_space[5]=0xBEEF5;// **** CP DEBUGGING

				copy_ip(local_ip, persistip);  		// to local var
				//sourceIPAddress=(persistip[3]<<24+persistip[2]<<16+persistip[1]<<8+persistip[0]);  		// to local var				
				sourceIPAddress=((int)persistip[0]*256*256*256)+((int)persistip[1]*256*256)+((int)persistip[2]*256)+persistip[3];  // shifting <<24 caused a crash, so have to use multipliers CP 06Oct2011.
				copy_ip(sys_ram->ip_addr,persistip);  	// to Ethernet area, IP, SNM, GW, Timeout.
				copy_ip(sys_ram->snm,persistsnm);  	// to Ethernet area, IP, SNM, GW, Timeout.
				copy_ip(sys_ram->gw_addr,persistrtr);  	// to Ethernet area, IP, SNM, GW, Timeout.
				copy_ip(sys_ram->dhcpserveraddr,persistdhcpserver);	// to Ethernet area, the DHCP server address (used for renewals)
				sys_ram->dhcprenewcountdown=dhcpipleasetime>>1;	// DHCP renewal time (traditionally half of the overall lease time)
				dhcpstate=DHCPBOUND; 			// state set
				// load data into SYSRAM Ethernet Area.  Mode -> BOUND			
				//spinn_packet_tx(0xc5010000, (persistip[0]<<24)+(persistip[1]<<16)+(persistip[2]<<8)+persistip[3], sys_ram->dhcprenewcountdown, sys_ram->dhcprenewcountdown,(uchar *) 0,0);

			} else {
				// if mismatch, then State -> INIT
			}
			
			//spinn_packet_tx(0xc5010000, persistip[0]<<24+persistip[1]<<16+persistip[2]<<8+persistip[3], persistsnm[0]<<24+persistsnm[1]<<16+persistsnm[2]<<8+persistsnm[3], persistrtr[0]<<24+persistrtr[1]<<16+persistrtr[2]<<8+persistrtr[3],(uchar *) 0,0);
			// for testing, send hello message with this info as the options!
		}

		if (dhcpstate==DHCPREQUESTING && dhcpmessagetype == 6) { 		// DHCP NAK...
			// if DHCP NAK with same xid means can't allocate requested address, drop back to init 
			dhcpstate=DHCPINIT; 			
		}
	}

	//sys_ram->ff_space[0]=0x0;// **** CP DEBUGGING  when leaving this routine clears variable to say out

	return 0;
}


void timereddhcpoperations() {

	if (sourceIPAddress==0 && dhcpstate==DHCPNOTSTARTED) {
	  // if we have a zero IP address, and we haven't begun the DHCP process, lets get going
	  for (uchar i=0;i<4;i++) sys_ram->dhcpserveraddr[i]=0;		// reset fields for DHCP server
	  sys_ram->dhcprenewcountdown=0;				//   and renew timers
	  dhcpstate=DHCPINIT;			// begin the process by entering the DHCP INIT state
	}
	  

	if(dhcpstate==DHCPINIT) { 			// Send out DHCP request periodically until in other state or exceeded max #
		if(dhcpattemptnum<DHCPMAXRETRIES) {	// if we've not already exceeded the max # of DHCP attempts
			dhcpattemptnum++;		// increment the counter logging the number of attempts taken to get a DHCP address
			for (uchar i=0;i<4;i++) local_ip[i]=0;// we have no claim on any current IP so reset it
							// TODO - how about the other IP variables?
			uchar extra_dhcp_options[]="";	// no extra options
			dhcpstate=DHCPSELECTING;	// change state as now awaiting offers from DHCP servers
			dhcp_tx(local_ip, DHCP_DISCOVER_TYPE, 0, extra_dhcp_options); 	// if in INIT mode, send out a vanilla discover, and move to SELECTING mode
			dhcpstarted = time;		// record time when this attempt began
		}
	}

	if(dhcpstate!=DHCPBOUND) {			// if we have not bound to an IP address
		if(dhcpattemptnum<DHCPMAXRETRIES) {	// if we've not already exceeded the max # of DHCP attempts
			if((time-dhcpstarted)>DHCPTIMEOUT) dhcpstate=DHCPINIT;	// Start again if timeout breeched
		} else {
			dhcpstate=DHCPNOREPLY;  	// per standard we should give up.  However SpiNNaker will continue to use it's old IP address anyway in these circumstances -- see TODO above
		}
	} else {
		if(!(time % 0x400)) {			// every ~1s we should decrement the _bound_ DHCP renew timer by a second
			uint current_dhcp_renewcount = sys_ram->dhcprenewcountdown;
			if (current_dhcp_renewcount==0xFFFFFFFF) {
				// do nothing as it's lease is infinite
			} else if (current_dhcp_renewcount==0) { 	// oh dear - lease should be renewed now
				dhcpstate=DHCPREQUESTING;		// turn mode back to start the requesting phase (will request same thing)
				uchar j=0;
				uchar extra_dhcp_options[12];	// extra options declared
				extra_dhcp_options[j++]=0x36;
				extra_dhcp_options[j++]=0x4;
				for(char i=0;i<4;i++) extra_dhcp_options[j++]=persistdhcpserver[i];  // Option: DHCP server selected (one we got previous one from)
				extra_dhcp_options[j++]=0x32;
				extra_dhcp_options[j++]=0x4;
				for(char i=0;i<4;i++) extra_dhcp_options[j++]=persistip[i];  // Option: DHCP address requested (our current one)
				dhcp_tx(local_ip, DHCP_REQUEST_TYPE, 12, extra_dhcp_options); 	// ask for this server to re-grant this offer (->REQUESTING mode)
				dhcpstarted = time;			// record time when this attempt began
				dhcpattemptnum=0;			// reset the counters so it can try multiple times (if 1st request fails, enters DHCPINIT mode and waits a bit)
			} else {
				sys_ram->dhcprenewcountdown=current_dhcp_renewcount-1;	// else drop the renew timer by 1 second
			}
		}			
	
	}
}




#endif // DHCP

