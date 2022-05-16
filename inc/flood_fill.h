#ifndef FLOOD_FILL_H
#define FLOOD_FILL_H

void cc_ff_start(unsigned int data);
void cc_ff_block_start(unsigned int key);
void cc_ff_block_data(unsigned int key, unsigned int data);
void cc_ff_block_end(unsigned int key, unsigned int data);
unsigned int cc_ff_control(unsigned int key);

void eth_ff_start(void);
void eth_ff_block_data(void);
unsigned int eth_ff_control(void);

#endif

