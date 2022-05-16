#ifndef ETH_FILL_H
#define ETH_FILL_H

void handleArpRequest(void);
void handlePingRequest(void);
void receive_frame(unsigned int tcmAddress);
int send_frame(int length);
void send_hello_frame(void);
int formatAndSendFrame(unsigned int instruction, unsigned int option1, unsigned int option2, unsigned int option3);

#endif
