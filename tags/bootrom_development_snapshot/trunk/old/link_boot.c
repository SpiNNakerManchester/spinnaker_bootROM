#include "spinnaker.h"

#define SOFT_RESET 0xf2000018 // CPU soft reset reg
#define MISC_CTRL  0xf2000038 // Misc control reg


// Write (poke) a single word to a neighbouring chip. Has a simple
// timeout mechanism which returns 0 on failure.
uint link_write_word (uint addr, uint link, const uint *buf)
{
	volatile uint *cc = (uint *) CC_BASE;
	uint t = 1024; // !! timeout - tweak this?

	while ((cc[CC_TCR] & 0x80000000) == 0)
	{
		if (--t == 0)
		return 0;
	}

	cc[CC_TCR] = 0x07a00000 + (link << 18); // NN direct packet
	cc[CC_TXDATA] = *buf;
	cc[CC_TXKEY] = addr;

	while ((cc[CC_RSR] & 0x80000000) == 0)
	{
		if (--t == 0)
			return 0;
	}

	addr = cc[CC_RXKEY];

	return 1;
}


// Write "len" words to a neighbouring chip from buffer "buf".
// If any word times out returns 0, otherwise 1 when whole
// block sent.
uint link_write (uint addr, uint len, uint link, const uint *buf)
{
	uint i;

	for (i = 0; i < len; i++)
	{
		uint rc = link_write_word (addr, link, buf);

		if (rc == 0)
			return 0;

		addr += 4;
		buf++;
	}

	return 1;
}



// Boot a neighbouring chip with data supplied in a buffer. The
// load address ("addr") should normally be 0xf5000000 (System RAM)
// When download is complete CPU #0 is allowed to run, all others
// remain reset. NB - return codes not checked!

// addr - the load address for the data
// len - number of words
// link - the link number to use
// buf - array of words to be loaded

const uint w[] = {0x5ecfffff, 0x00000001, 0x5ec00002};

void link_boot (uint addr, uint len, uint link, uint *buf)
{
	(void) link_write (SOFT_RESET, 1, link, w+0); // Reset all CPUs

	(void) link_write (MISC_CTRL, 1, link, w+1);  // Remap ROM/RAM

	(void) link_write (addr, len, link, buf);	// Copy buffer

	(void) link_write (SOFT_RESET, 1, link, w+2); // Allow CPU #0 to run
}
