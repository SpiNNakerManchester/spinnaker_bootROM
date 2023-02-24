/*
 * Copyright (c) 2008 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
