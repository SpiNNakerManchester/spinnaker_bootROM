#ifndef DEBUGPRINTF_H
	#define DEBUGPRINTF_H

	void iputc_eth(uint itube_chip_id, uint itube_proc_id, char itube_toprint);
	void iputc_cc(uint itube_chip_id, uint itube_proc_id, char itube_toprint); 
	void itubecchandler(uint itube_chip_id, uint itube_proc_id, char itube_toprint);
	void _sys_exit(void);
	void init_tube_port(void);
	void dr_tx_byte(unsigned int tempch);

#endif
