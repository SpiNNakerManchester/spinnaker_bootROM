#ifndef INTERRUPT_SERVICE_ROUTINES_H
#define INTERRUPT_SERVICE_ROUTINES_H

__irq void cc_rx_isr(void);
__irq void dma_done_isr(void);
__irq void dma_error_isr(void);
__irq void dma_timeout_isr(void);
__irq void eth_phy_isr(void);
__irq void eth_rx_isr(void);
__irq void software_isr(void);
__irq void sys_ctrl_isr(void);
__irq void timer_isr(void);

#endif
