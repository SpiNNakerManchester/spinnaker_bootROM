#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>


#pragma pack(1)						// don't pad the struct



#define SERVERPORT 		"54321" 	// the port server will use

#define SPINNPROTVERSION	0x1 		// this is the Spinnaker Protocol Version (1= 1st release (ROM)) !!! 
#define SPINNPROTSUBVERSION 	0x0		// for testing

#define SPINNHEADERLENGTH	18		// this is the SpiNNaker protocol byte overhead, ver(2)+opcode(4)+3options(4)

#define MAXBLOCKSIZE		256		// max number of words in a block - !! should be 256

#define TARGET_GROUP		0x0		// for monitor processor only !!! 0
#define ACTION_OPERATION	1		// means execute code once all has been received
#define DEFAULT_EXECUTE_ADDRESS		0x0	// can be overriden by command line argument 5
#define DEFAULT_ITCM_LOAD_ADDR		0x0	// if user doesn't specify in argument 4, code is loaded linearly from ITCM 0x0.	

#define FF_START			0x1
#define FF_BLOCK_DATA		0x3
#define FF_CONTROL			0x5

#define INT_PACKET_DELAY	0.1		// this is the delay between packets being sent (in seconds)
#define IMAGE_REPEAT_DELAY	5.0		// this is how long we wait to begin transmitting the image in full once again




struct SpiNNakerPacket {
	unsigned char subversion;
	unsigned char version;
	unsigned int opcode;
	unsigned int option1;
	unsigned int option2;
	unsigned int option3;
	unsigned int array[MAXBLOCKSIZE];
};



int main(int argc, char *argv[]) {
	int rv, sockfd, numbytes, block_data_len, block_id, i,j,length, remaining_bytes;
	struct addrinfo hints, *servinfo, *p;
	FILE *romimage;
	struct SpiNNakerPacket workingpacket;
	struct SpiNNakerPacket *pointworkingpacket; 

	time_t start;
	time_t current;

	pointworkingpacket = &workingpacket;
        
	if ((argc < 3)||(argc >5)) {
		fprintf(stderr,"usage: server targetIP|Hostname ImageFileName [ITCMStartAddr ExecuteAddress]\n                  	                       Note:  If supplying an Execute address you must also supply a StartAddr\n 		                       Note:  Start and Execute address arguments, if not supplied, are implicit 0x0\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "OH-CRAP: failed to bind socket\n");
		return 2; // 2 means we couldn't bind to our socket (probably already running)
	}

	printf("Opening the image to Send\n");
	romimage=fopen(argv[2],"rb");
	if (romimage==NULL) {
       	printf("Oh bugger - couldn't open our rom source file\n");
       	return 3; // or something that means something!!
	}
	else {
		printf("File Opened OK\n");
       	
       	length=0;							//find the length of the file we need to encode
		fseek (romimage , 0 , SEEK_END);	//find the end of the file
		length = ftell (romimage);			//read the position back into length variable
		rewind (romimage); 					//reseek to position 0
	
		printf("Length of file opened: %d.\n",length);        	
       	     
		unsigned int data_block_count = length / (4*MAXBLOCKSIZE);
		unsigned int data_leftovers_size = length % (4*MAXBLOCKSIZE);
		if (data_leftovers_size) data_block_count++;        
       	//mod and div with the max block word size taken into account
       
		printf("  Blocks: %d,   Leftovers in final block: %d.\n",data_block_count,data_leftovers_size);  
        
       	while (1) {		// will rewrite this to be conditional on ACK being received!
       		     		
       		// send flood fill start first
       		printf("Sending Flood Fill Start\n");        		
       		
       		block_data_len =0;	// in this section we don't send data
       		workingpacket.version = SPINNPROTVERSION;
       		workingpacket.subversion = SPINNPROTSUBVERSION;
       		workingpacket.opcode = htonl(FF_START);
			workingpacket.option1 = htonl((TARGET_GROUP & 0xFF));
			workingpacket.option2 = htonl(block_data_len*4);
			workingpacket.option3 = htonl(data_block_count-1);  // // word count 0:FF = 1-256 
		
        	if ((numbytes = sendto(sockfd, pointworkingpacket, SPINNHEADERLENGTH+(block_data_len*4), 0, p->ai_addr, p->ai_addrlen)) == -1)
			{
				perror("oh dear - we didn't send our data!\n");
				exit(1);
			}

    
       
		time(&start);
		do {
			time(&current);
		}while(difftime(current,start) < INT_PACKET_DELAY);

	       //for(j=0;j<1000000000;j++) {;}   // have a bit of a lie down after sending the start packet
		

       		// send flood fill packets

			for(block_id=1;block_id<=data_block_count;block_id++) {
			
        		block_data_len = MAXBLOCKSIZE;	// set as full block unless..
        		if((block_id==data_block_count) && (data_leftovers_size)) {	// if last block and we're sending only leftovers
        			block_data_len = (data_leftovers_size/4);
/*        			if(data_leftovers_size%4==0) {			
        				block_data_len = (data_leftovers_size/4);	// if leftovers is divisible by 4 then send that many whole words
        			} else {
        				block_data_len = (data_leftovers_size/4)+1;	// if leftovers leaves a remainder, send 1 additional partial word
        			}
*/
        		}
				        		
        		workingpacket.version = SPINNPROTVERSION;
	       		workingpacket.subversion = SPINNPROTSUBVERSION;
        		workingpacket.opcode = htonl(FF_BLOCK_DATA);
				workingpacket.option1 = ((MAXBLOCKSIZE-1) & 0xFF)<<8; // block size ranges from 0-255, shift 8 bits left
				workingpacket.option1 |= ((block_id-1) & 0xFF); // block ID ranges from 0-255
				workingpacket.option1 |= ((block_id-1) & 0xFF); // block ID ranges from 0-255
				workingpacket.option1 = htonl(workingpacket.option1);
				workingpacket.option2 = htonl(block_data_len*4);
				if (argc < 4) {
					workingpacket.option3 = htonl((MAXBLOCKSIZE*(block_id-1))+DEFAULT_ITCM_LOAD_ADDR); 
							// default as per #define at top of code.
				} else {
					workingpacket.option3 = htonl((MAXBLOCKSIZE*(block_id-1))+(unsigned int)argv[4]);  
							// supplied via command line argument 4.
				}   	// this calculates the address in ITCM the block should be stored at.
				
		
				// note if net byte ordering required then change below line to end =htonl(getw(romimage))
				for(i=0;i<block_data_len;i++) workingpacket.array[i]=htonl(getw(romimage));

				// note if net byte ordering required then change below 15 lines or so to suit (not so trivial!)
				remaining_bytes=0;
				if(block_id==data_block_count) {
					remaining_bytes=data_leftovers_size%4;
					workingpacket.option2=htonl((block_data_len*4)+remaining_bytes);
				 	if (remaining_bytes!=0) {  // if not a word aligned number of bytes in last packet
						workingpacket.array[block_data_len]=0;  //initialise word to zero
						for (i=0;i<remaining_bytes;i++) {
							workingpacket.array[block_data_len] |= getc(romimage)<<24; // put received byte in lowest bits
							workingpacket.array[block_data_len] = workingpacket.array[block_data_len] >>8; // shift up 8 for next byte
							printf("Additional bytes: 0x%x ...",workingpacket.array[block_data_len]);
						}
						if (remaining_bytes==1) workingpacket.array[block_data_len] = workingpacket.array[block_data_len] >>16;
						if (remaining_bytes==2) workingpacket.array[block_data_len] = workingpacket.array[block_data_len] >>8;
						printf("\nRemainingBytes: 0x%x. Additional bytes: 0x%x ...",remaining_bytes,workingpacket.array[block_data_len]);
						// shift bytes up to 'front' of the packet as required				
					}			
				}

	        	printf("Sending Flood Fill Packet %d of %d\n",block_id,data_block_count);        		
	        		
	        	if ((numbytes = sendto(sockfd, pointworkingpacket, SPINNHEADERLENGTH+(block_data_len*4)+remaining_bytes, 0, p->ai_addr, p->ai_addrlen)) == -1) {
					perror("oh dear - we didn't send our data!\n");
					exit(1);
				}

		time(&start);
		do { time(&current);
		}while(difftime(current,start) < INT_PACKET_DELAY);
		
//		for(j=0;j<1000000000;j++) {;}  // delay packet sending between data packets
       		}
			
			
        		
        		
       		// send flood fill control
        		
       		printf("Sending Flood Fill Control\n");        	
       		block_data_len =0;	// in this section we don't send data
       		workingpacket.version = SPINNPROTVERSION;
       		workingpacket.subversion = SPINNPROTSUBVERSION;
       		workingpacket.opcode = htonl(FF_CONTROL);
			workingpacket.option1 = htonl((ACTION_OPERATION & 0xFF));
			workingpacket.option2 = htonl(block_data_len*4);
			if (argc != 5) {
				workingpacket.option3 = htonl(DEFAULT_EXECUTE_ADDRESS); // default as per #define at top of code.
			} else {
				workingpacket.option3 = htonl((unsigned int)argv[5]);  // supplied via command line argument 5.
			}		// supplies the execute address of the image to be branched to after load verified complete.
				
			
        	if ((numbytes = sendto(sockfd, pointworkingpacket, SPINNHEADERLENGTH+(block_data_len*4), 0, p->ai_addr, p->ai_addrlen)) == -1) 
        	{
			perror("oh dear - we didn't send our data!\n");
			exit(1);
		}
        		

		time(&start);
		do { time(&current);
		}while(difftime(current,start) < IMAGE_REPEAT_DELAY);
        		
//        	for(j=0;j<100000000;j++) {;}  // wait interval - probably longer as between whole images rather than packets
		printf("Here we go round the mulberry bush\n\n\n");        		
			
			rewind (romimage); 		
		}        		
        
       
		
		
	}		 


	freeaddrinfo(servinfo);
	printf("spinnaker talker: sent %d bytes to %s\n", numbytes, argv[1]);

	close(sockfd);

return 0;
}
//123456789AB
