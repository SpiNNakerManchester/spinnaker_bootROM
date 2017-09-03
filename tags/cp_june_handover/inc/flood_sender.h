#ifndef FLOOD_SENDER_H
	#define FLOOD_SENDER_H

	uchar generate_check(uint key, uint data);
	unsigned int transmit(uint route, uchar payload, uint key, uint data);
	void ff_control(uchar route, uint parameter);
	void ff_block_data(uchar route, uchar block_ID, uchar word_ID, uint data);
	void ff_block_start(uchar route, uchar block_ID, uint word_count);
	void flood(void);

#endif
