#ifndef DMA_H
#define DMA_H

unsigned int dma(unsigned int sys_NOC_address, unsigned int TCM_address, 
				unsigned int length, unsigned int crc, unsigned int direction);
void setup_crc32_spinnaker2(void);

#endif
