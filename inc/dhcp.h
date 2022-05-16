/*****************************************************************************************
*	dhcp.h - all about the DHCP supporting code 
*
*
*	Created Chopped Hacked and Sliced by Cameron Patterson
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008-11. All rights reserved.
*****************************************************************************************/

#ifndef DHCP_H
#define DHCP_H


/* --------------------------------- For DHCP additions ----------------------------------*/
#ifdef DHCP

#define FRAME_LEN(frame)			((frame[4] >> 0x10) + ETH_HEADER_SIZE)
#define BOOTP_PORT				68
#define	BOOTP(frame)				((frame[9] >> 0x10) == BOOTP_PORT)
#define ARP_SIZE				28
#define ETH_HEADER_SIZE				14
#define IP_HEADER_SIZE				20
#define ICMP_HEADER_SIZE			8
#define UDP_HEADER_SIZE				8
#define SPINN_HEADER_SIZE			18

// Stored in network byte order
#define ARP_HTYPE_ETH				0x0100
#define ARP_HLEN					0x06
#define ARP_PLEN					0x04
#define ARP_REQUEST					0x0100
#define ARP_REPLY					0x0200
#define ARP_REQUEST					0x0100

#define ETH_TYPE_ARP				0x0608
#define ETH_TYPE_IP					0x0008

#define ICMP_ECHO_PING_REQUEST		0x08
#define ICMP_ECHO_PING_REPLY		0x00

#define IP_PROTOCOL_ICMP			0x01
#define IP_PROTOCOL_UDP				0x11

#define IP_VER_LEN 					0x45
#define IP_DS 						0x00
#define IP_IDENT_ZERO				0x00
#define IP_DONT_FRAGMENT			0x0040
#define IP_TTL						0x40

#define HTONS(t)				((((t) & 0x00ff) << 8) | (((t) & 0xff00) >> 8))
#define HTONL(t)				((((t) & 0xff) << 24) | (((t) & 0xff00) << 8) | (((t) & 0xff0000) >> 8) | (((t) & 0xff000000) >> 24))

#define DHCP_DISCOVER_TYPE			1			// CP DHCP defines
#define DHCP_REQUEST_TYPE			3			// CP DHCP defines

#define DHCPINIT				0	// CP DHCP
#define DHCPSELECTING				1	// CP DHCP
#define DHCPREQUESTING				2	// CP DHCP
#define DHCPBOUND				3	// CP DHCP
#define DHCPNOREPLY				4	// CP DHCP
#define DHCPNOTSTARTED				5	// CP DHCP

#define DHCPMAXRETRIES				6	// will try to get an address 6 times
#define DHCPTIMEOUT				5000	// will attempt to get an address every 5000ms	

typedef volatile unsigned char vuchar;
typedef volatile unsigned int vuint;
typedef volatile unsigned short vushort;

typedef __packed struct
{
	ushort htype;
	ushort ptype;
	uchar hlen;
	uchar plen;
	ushort op;
	uchar sha[6];
	uchar spa[4];
	uchar tha[6];
	uchar tpa[4];
} arp_pkt_t;

typedef __packed struct
{
	uchar dst_mac[6];
	uchar src_mac[6];
	ushort type;
	uchar payload[1514 - ETH_HEADER_SIZE];
} eth_frame_t;

typedef __packed struct
{
	uchar type;
	uchar code;
	ushort checksum;
	ushort ident;
	ushort seq;
	uchar payload[];
} icmp_pkt_t;

typedef __packed struct
{
	uchar ver_len;
	uchar DS;
	ushort length;
	ushort ident;
	ushort flg_off;
	uchar TTL;
	uchar protocol;
	ushort checksum;
	uchar src_ip[4];
	uchar dst_ip[4];
	uchar payload[];
} ip_pkt_t;

typedef __packed struct
{
	ushort ver;
	uint op;
	uint arg0;
	uint arg1;
	uint arg2;
	uchar payload[];
} spinn_pkt_t;

typedef __packed struct
{
	ushort src_port;
	ushort dst_port;
	ushort length;
	ushort checksum;
	uchar payload[];
} udp_pkt_t;

typedef __packed struct
{
	uchar op;
	uchar htype;
	uchar hlen;
	uchar hops;
	uint xid;
	ushort secs;
	ushort flags;
	uchar ciaddr[4];
	uchar yiaddr[4];
	uchar siaddr[4];
	uchar giaddr[4];
	uchar chaddr[6];
	uchar chaddrpad[10];
	uchar sname[64];
	uchar file[128];
	uint cookie;
	uchar options[];
} dhcp_pkt_t;   // CP DHCP

typedef __packed struct
{
	ushort tailofIP;
	ushort length;
	ushort dst_port;
	uchar options[];
} receive_dhcp_pkt_t;   // CP DHCP

typedef __packed struct
{
	uint first;
	uint data[];
} ip_pkt_ints_t;

typedef struct
{
	vuint tx_frame_buf[4096];		// 0x0000
	vuint rx_frame_buf[4096];		// 0x4000
	vuint rx_desc_ram[4096];		// 0x8000
	vuint general_command;			// 0xc000
	vuint general_status;			// 0xc004
	vuint tx_length;				// 0xc008
	vuint tx_command;				// 0xc00c
	vuint rx_command;				// 0xc010
	vuint mac_addr_lo;				// 0xc014
	vuint mac_addr_hi;				// 0xc018
	vuint phy_control;				// 0xc01c
	vuint irq_clear;				// 0xc020
	vuint rx_buf_read_ptr;			// 0xc024
	vuint rx_buf_write_ptr;			// 0xc028
	vuint rx_desc_read_ptr;			// 0xc02c
	vuint rx_desc_write_ptr;		// 0xc030
} eth_t;

typedef __packed struct
{
	uint app_load_space[2048];
	uint reserved_0[1758+4096];
	uint ff_space[257];
	uint monitor_history;
	uint failure_logs[18];
	uint sdram_info_lo;
	uint sdram_info_hi;
	uint app_load_mailbox_opcode;
	uint app_load_mailbox_arg0;
	uint app_load_mailbox_arg1;
	uint app_load_mailbox_arg2;
	ushort p2p_addr;
	uchar mac_addr[6];
	uchar ip_addr[4];
	uchar gw_addr[4];
	uchar snm[4];
	ushort reserved5;
	ushort portnum;
	uint dhcprenewcountdown;
	uchar dhcpserveraddr[4];
} sys_ram_t;

extern uchar dhcp_learned;
extern uint dhcpipleasetime;	
extern unsigned int dhcpstarted;
extern unsigned int dhcpattemptnum;		// number of times to reattempt getting a DHCP address before giving up.
extern unsigned char dhcpstate;			// CP DHCP - starting state
extern unsigned char persistip[4],persistsnm[4],persistrtr[4],persistdhcpserver[4];	// CP DHCP - for verification of DHCP address from server

// prototypes
uint ip_sum(uchar *d, uint len, uint sum);
void copy_ip(uchar *a, uchar *b);
void copy_mac(uchar *a, uchar *b);
uchar dhcp_pkt(eth_frame_t *rx_frame, uint rx_len); // CP DHCP
uint dhcp_tx(uchar* current_local_ip, uchar mssg_type, uchar extra_options_length, uchar* extra_options); // CP DHCP
void timereddhcpoperations(void); // CP DHCP

extern eth_t* eth;
extern sys_ram_t* sys_ram;

#endif // DHCP


#endif // DHCP_H

