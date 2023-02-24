;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; globals.s - contains definitions of hardware and software parameters mirroring those in
; globals.h. Any changes made here should be carefully checked against changes in the
; latter file.
;
;
; Created by Mukaram Khan and Xin Jin
; Modified by Thomas Sharp and Cameron Patterson
;
; Copyright (c) 2008 The University of Manchester
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; CHIP DETAILS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IRQ_OFF						EQU     0x80
FIQ_OFF						EQU     0x40

IRQ_MODE					EQU     0x12
SVC_MODE					EQU     0x13

IRQ_STACK_SIZE				EQU		0x500
SVC_STACK_SIZE				EQU		0x500

DTCM_BASE					EQU		0x400000
DTCM_SIZE					EQU		0x10000

ITCM_BASE					EQU		0x0
ITCM_SIZE					EQU		0x8000

SD_RAM_BASE					EQU		0x70000000

SYS_RAM_BASE				EQU		0xF5000000

VIC_OFFSET					EQU		0xFF0



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; SOFTWARE DEFINES ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ITCM_FAILURE				EQU		0x4
DTCM_FAILURE				EQU		0x2
UNEXPECTED_EXCEPTION_HIGH	EQU		0xD
UNEXPECTED_EXCEPTION_LOW	EQU		0xE

SECURITY_CODE				EQU		0x5EC00000



; NOTE: If the clock speeds are changed, the timer interval must be updated accordingly
PLL_1_SETUP					EQU		0x00060110 ; = 160MHz See data sheet
PLL_2_SETUP					EQU		0x0007011A ; = 260MHz

SPINNAKER2_CLK_MUX_CTRL_SETUP	EQU		0x80528821	; CPUs clocked at PLL1 / 1 = 160MHz
													; SDRAM clocked at PLL2 / 1 = 260MHz
													; Router clocked at PLL1 / 2 = 80MHz
													; SysAHB clocked at PLL1 / 2 = 80HMz

TEST_CHIP_CLK_MUX_CTRL_SETUP	EQU		0x58000165	; CPUs clocked at PLL1 / 1 = 160MHz
													; SDRAM clocked at PLL2 / 1 = 260MHz
													; Router clocked at PLL1 / 2 = 80MHz
													; SysAHB clocked at PLL1 / 2 = 80HMz

WATCHDOG_RESET_CODE			EQU		0x4			; 0b100 - signifies the watchdog reset condition
DMA_CMD_MASTER_CRC_CHK		EQU		0x0018008C	; DMA control word:  dir=1 (tcm to sys), CRC=1 (write), len=(35*4)= 140 bytes
DMA_CMD_BLOCK_CRC_CHK		EQU		0x00180000	; DMA control word:  dir=1 (tcm to sys), CRC=1 (write) (length is added in main code)
DMA_TRANSFER_ACTIVE			EQU		0x1			; bit position remains high while DMA transfer is in progress
DMA_TRANSFER_PAUSED			EQU		0x2			; bit position set high following a DMA transfer error

IVB_MAGIC_NUMBER			EQU		0xC0FFEE18	; magic number used as a crude check of correct ITCM population on boot

IVB_IMAGE_STARTLENGTH		EQU		0x00
IVB_IMAGE_LENGTH			EQU		0x04
IVB_IMAGE_EXECUTE_ADDR		EQU		0x08
IVB_GRAND_CRC				EQU		0x8C
IVB_MAGIC_NUMBER_PLACE		EQU		0x90



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; MEMORY ALLOCATIONS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; These are derived from globals.h TODO double check all of them

SPINNAKER2_ETH_PARAMS_BASE	EQU		0xF5007FE0

SPINNAKER2_FF_SPACE_BASE	EQU		0xF5007B78
TEST_CHIP_FF_SPACE_BASE		EQU		0xF5003BB8 ; BUGZILLA 51. CP 20/08/2010

SPINNAKER2_FAILURE_LOG_BASE	EQU		0xF5007F80
TEST_CHIP_FAILURE_LOG_BASE	EQU		0xF5003FC0

IVB_BLOCK_SIZE				EQU		148 ; ((32 + 4 + 1) * 4)
ITCM_VALIDATION_BLOCK		EQU		(ITCM_BASE + ITCM_SIZE - IVB_BLOCK_SIZE)

MANUFACTURING_TESTS			EQU		0xFFFF4000



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; PERIPHERAL REGISTERS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DMA_CTRL_BASE				EQU		0x30000000
DMA_CTRL_STAT				EQU		0x14
DMA_CTRL_ADRS				EQU		0x04
DMA_CTRL_ADRT				EQU		0x08
DMA_CTRL_DESC				EQU		0x0C
DMA_CTRL_CRCC				EQU		0x1C

SYS_CTRL_BASE				EQU		0xF2000000
SYS_CTRL_BASE_BUFFERED		EQU		0xE2000000
SYS_CTRL_BASE_UNBUFFERED	EQU		0xF2000000
SYS_CTRL_TUBE				EQU		0x00
SYS_CTRL_CPU_DISABLE		EQU		0x04
SYS_CTRL_CPU_RESET_PULSE	EQU		0x24
SYS_CTRL_MONITOR_ID			EQU		0x34  		; BUGZILLA 59. CP 20/08/2010
SYS_CTRL_MISC_CONTROL		EQU		0x38
SYS_CTRL_IO_PORT			EQU		0x40
SYS_CTRL_IO_DIRECTION		EQU		0x44
SYS_CTRL_SET_IO				EQU		0x48
SYS_CTRL_CLEAR_IO			EQU		0x4C
SYS_CTRL_PLL1				EQU		0x50
SYS_CTRL_PLL2				EQU		0x54
SYS_CTRL_CLK_MUX_CTRL		EQU		0x60
SYS_CTRL_RESET_CODE			EQU		0x30

VIC_BASE					EQU 	0xFFFFF000	; BUGZILLA 59. CP 20/08/2010 (for turning off VIC when we send a processor to sleep)
VIC_ENABLE_CLEAR			EQU		0x14		; BUGZILLA 59. CP 20/08/2010 (for turning off VIC when we send a processor to sleep)


		END
