#ifndef PROC_H
	#define PROC_H

	void change_to_low_vectors(void);
	void disable_clock(void);
	void disable_interrupts(void);
	void enable_interrupts(void);
	void read_CPSR(void);
	void wait_for_interrupt(void);

#endif
