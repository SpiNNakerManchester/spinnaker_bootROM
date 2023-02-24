/*
 * Copyright (c) 2008 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
