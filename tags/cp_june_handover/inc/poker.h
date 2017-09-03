#ifndef POKER_H
	#define POKER_H

	unsigned int link_write_word (uint addr, uint link, const uint *buf);
	unsigned int link_write (uint addr, uint len, uint link, const uint *buf);
	void poke_neighbours_up(uint len, const uint *buf, uint link);
	
#endif
