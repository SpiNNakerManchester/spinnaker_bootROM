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

