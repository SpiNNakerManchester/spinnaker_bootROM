#ifndef DEBUGPRINTF_H
#define DEBUGPRINTF_H

#include <stdio.h>

unsigned char gen_check(unsigned int key, unsigned int data);
void chip_num_handler(unsigned int data, unsigned int rx_status);
void chip_num_sender(unsigned int my_chip_id, unsigned int size_x, unsigned int size_y);
void p2p_algorithmically_fill_table(unsigned int size_x, unsigned int size_y);
unsigned int transmit(unsigned int route, unsigned char payload, unsigned int key, unsigned int data);
int fputc(int ch, FILE *f);
void iputc_cc(unsigned int chip_id, unsigned int proc_id, char ch);
void iputc_eth(unsigned int chip_id, unsigned int proc_id, char c);
void itube_cc_handler(unsigned int data);
void _sys_exit(void);

#endif
