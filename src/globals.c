/*****************************************************************************************
*	Created by Cameron Patterson and Thomas Sharp
*
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
*****************************************************************************************/


/* ---------------------------- STATUS VARIABLES -----------------------------*/
unsigned int time = 0;				// Time since boot in milliseconds

unsigned int rx_frame[380];	//to hold 1500 bytes for the frame to receive
unsigned int tx_frame[380];

// All the below network data now is overwritten from Serial ROM, but variables do need to be defined here
unsigned int destinationMACAddressLS=0x0;
unsigned int destinationMACAddressHS=0x0;
unsigned int sourceMACAddressLS=0x0;
unsigned int sourceMACAddressHS=0x0;
unsigned int sourceIPAddress=0x0;
unsigned int destinationIPAddress=0x0;

unsigned int spinnakerDataSize;
unsigned int macDestAddressConfigured=0;
unsigned int FFTarget=0;
unsigned int rxFrameSize=0;
unsigned short EthPacketID=0;
