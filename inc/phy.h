/*
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
 */
#ifndef PHY_H
#define PHY_H

unsigned short int phy_shift_in(void);
void phy_shift_out(unsigned int command, unsigned int length);
unsigned short int phy_read(unsigned char phy_addr);
void phy_write(unsigned char phy_addr, unsigned short int value);
int test_phy(void);
void print_phy_register_table(void);

#endif /*PHY_H*/
