unsigned int crc32_table[256];  // Lookup table array

//TODO clean up. Also, where do we put the CRC32 table?


//Swap bit 0 for bit 7,  bit 1 for bit 6, etc. 
unsigned int reflect(unsigned int ref, char ch) 
{
	int i;
	unsigned int value = 0; 
	
	for(i = 1; i < ch + 1; i++) 
	{ 
		if(ref & 0x1)
		{
			value |= 1 << (ch - i);
		} 
		ref >>= 1; 
	} 
	return value; 
}

// Call this function only once to initialize the CRC table. 
// This is the official polynomial used by CRC-32 
// in PKZip, WinZip and Ethernet. 
void initCRC32Table() 
{
	int i, j;
	unsigned int poly = 0x04c11db7; 

	for(i = 0; i <= 0xFF; i++) 
	{ 
		crc32_table[i] = reflect(i, 8) << 24;
		for (j = 0; j < 8; j++)
		{ 
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? poly : 0);
		} 
		crc32_table[i] = reflect(crc32_table[i], 32); 
	} 
}

// Pass a text string to this function and it will return the CRC. 

// Once the lookup table has been filled in by the two functions above, 
// this function creates all CRCs using only the lookup table. 

// Be sure to use unsigned variables, 
// because negative values introduce high bits 
// where zero bits are required. 
unsigned int Get_CRC(unsigned char* text, int len) 
{

	// Start out with all bits set high. 
	unsigned int  ulCRC=0xffffffff; 
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

int checkCRC(unsigned int* text, int len, unsigned int CRC)
{
	//get the length of buffer
	//int j=0, i=0;
	//unsigned int* byteBuf;
	unsigned int word;
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
