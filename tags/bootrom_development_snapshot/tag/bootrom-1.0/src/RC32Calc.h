#ifndef RC32CALC_H_
#define RC32CALC_H_
typedef unsigned int uint32_t;
uint32_t crc32_table[256];  // Lookup table array

uint32_t Reflect(uint32_t ref, char ch) 
{
	// Used only by Init_CRC32_Table(). 
	int i;
	uint32_t value=0; 

	// Swap bit 0 for bit 7 
	// bit 1 for bit 6, etc. 
	for(i = 1; i < (ch + 1); i++) 
	{ 
			if(ref & 1) 
				value |= 1 << (ch - i); 
			ref >>= 1; 
	} 
	return value; 
} 

void Init_CRC32_Table() 
{
	// Call this function only once to initialize the CRC table. 
	// This is the official polynomial used by CRC-32 
	// in PKZip, WinZip and Ethernet. 
	uint32_t ulPolynomial = 0x04c11db7; 
	int i, j;
	// 256 values representing ASCII character codes. 
	for(i = 0; i <= 0xFF; i++) 
	{ 
			crc32_table[i]=Reflect(i, 8) << 24; 
			for (j = 0; j < 8; j++) 
				crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0); 
			crc32_table[i] = Reflect(crc32_table[i], 32); 
	} 
} 

uint32_t Get_CRC(unsigned char* text, int len) 
{// Pass a text string to this function and it will return the CRC. 

	// Once the lookup table has been filled in by the two functions above, 
	// this function creates all CRCs using only the lookup table. 

	// Be sure to use unsigned variables, 
	// because negative values introduce high bits 
	// where zero bits are required. 

	// Start out with all bits set high. 
	uint32_t  ulCRC=0xffffffff; 
	//int len; 
	unsigned char* buffer; 

	// Get the length. 
	//len = strlen(text); 
	// Save the text in the buffer. 
	buffer = text; 
	// Perform the algorithm on each character 
	// in the string, using the lookup table values. 
	while(len--) 
			ulCRC = (ulCRC >> 8) ^ crc32_table[(int)((ulCRC & 0xFF) ^ *buffer++)]; 
	// Exclusive OR the result with the beginning value. 
	return ulCRC ^ 0xffffffff; 
} 
int checkCRC(uint32_t* text, int len, unsigned int CRC)
{
	//get the length of buffer
	//int j=0, i=0;
	//uint32_t* byteBuf;
	uint32_t word;
	//len=text[3];
	//len=len & 0xFFFF; //first two bytes contain the length of buffer
	//making the byte buffer of size len
	//j--;
	//getting crc
	//word=text[(len/4 -1)];
	word=Get_CRC((unsigned char*)text, len);
	if(word == CRC)
		return 1;
	else
		return 0;
}

#endif /*RC32CALC_H_*/
