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
#ifndef ETH_FILL_H
#define ETH_FILL_H

void handleArpRequest(void);
void handlePingRequest(void);
void receive_frame(unsigned int tcmAddress);
int send_frame(int length);
void send_hello_frame(void);
int formatAndSendFrame(unsigned int instruction, unsigned int option1, unsigned int option2, unsigned int option3);

#endif
