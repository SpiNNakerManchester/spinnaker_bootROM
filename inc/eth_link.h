#ifndef ETH_FILL_H
#define ETH_FILL_H

void handleArpRequest(void);
void handlePingRequest(void);
void receive_frame(unsigned int tcmAddress);
void send_hello_frame(void);
void send_itubotron_message(void);
#endif
