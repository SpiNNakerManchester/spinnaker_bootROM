#ifndef IVB_H
	#define IVB_H

	unsigned int IVB_CRC(unsigned int TCM_Source, unsigned int dma_tx_length);
	void ivb_populator(unsigned int execute_address, unsigned int start_address);

#endif
