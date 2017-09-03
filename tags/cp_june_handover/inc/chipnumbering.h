#ifndef CHIPNUMB_H
	#define CHIPNUMB_H
	
	#ifdef TEST_BOARD
		void chip_num_sender(uint my_chip_id, uint size_x, uint size_y);
		void chip_num_handler(uint data, uint rx_status);
		void p2p_algorithmically_fill_table(uint size_x, uint size_y);
	#endif
		
#endif
