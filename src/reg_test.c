/*****************************************************************************************
* 	Created by Steve Temple
*   Modified by Thomas Sharp
*
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
*****************************************************************************************/


#include "../inc/globals.h"
#include "../inc/peripheral_inits.h"
#include "../inc/ram_test.h"
#include "../inc/start.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#define OFFSET_MASK     0x00000fff
#define BYTE_MASK     	0x000ff000
#define MEM_MASK     	0x000fffff

#define BYTE_SHIFT     	12

#define TEST_STOP	0x80000000
#define TEST_REP_BIT	0x40000000
#define TEST_BYTE_BIT	0x20000000
#define TEST_MEM_BIT 	0x10000000
#define TEST_MASK	0x08000000
#define TEST_RST	0x04000000
#define TEST_RW		0x02000000
#define TEST_ZEROS 	0x01000000
#define TEST_ONES 	0x00800000
#define TEST_WRITE 	0x00400000


#define TEST_BYTE(v)    (TEST_BYTE_BIT + (v << BYTE_SHIFT))
#define TEST_REP(v)     (TEST_REP_BIT  + (v << BYTE_SHIFT))
#define TEST_MEM(v)     (TEST_MEM_BIT  + v)


const uint rw_pat[] = {0x00000000, 0xffffffff, 0xaaaaaaaa, 0x55555555};


uint reg_test (const uint *vec)
{
  volatile uint *reg_base = (volatile uint *) vec[0];
  uint ptr = 1;
  uint err = 0;

  while (1)
    {
      uint ctrl = vec[ptr++];
      uint byte = (ctrl >> BYTE_SHIFT) & 255;

      uint value;

      if (ctrl & TEST_ONES)
	value = 0xffffffff;
      else if (ctrl & TEST_ZEROS)
	value = 0x00000000;
      else if (ctrl & TEST_BYTE_BIT)
	value = byte;
      else
	value = vec[ptr++];

      uint mask = 0xffffffff;

      if (ctrl & TEST_MASK)
	mask = vec[ptr++];

      uint offset = ctrl & OFFSET_MASK;
      volatile uint *reg = reg_base + offset;

      if (ctrl & TEST_WRITE) // !! Bug ? - misses TEST_STOP??
	{
	  *reg = value;
	  continue;
	}

      uint rep = (ctrl & TEST_REP_BIT) ? byte : 1;

      if (ctrl & TEST_MEM_BIT) // Bodge to get larger repeat value
	{
	  rep = ctrl & MEM_MASK;
	  offset = 0;
	}

      while (rep-- && !err)
	{
		#ifdef DEBUG
		printf("%08x reg %08x ptr %d\n", ctrl, reg, ptr);
		#endif


	  if ((ctrl & TEST_RST) && ((value ^ *reg) & mask))
	    {
	    	#ifdef DEBUG
	    	printf("!! RST %08x -> %08x mask %08x\n", value, *reg, mask);
	    	#endif
	      err = 1;
	    }

	  if (ctrl & TEST_RW)
	    {
	      const uint *pat = rw_pat;
	      uint i = 0;

	      while ((i++ < 4) && !err)
	      {
		*reg = *pat;

		if ((*reg ^ *pat) & mask)
		  {
//		    io_printf (IO_STD, "!! RW  %08x -> %08x mask %08x\n", *pat, *reg, mask);
		    err = 1;
		  }

		pat++;
	      }

	      *reg = value;
	    }

	  reg++;
	}

      if (err || (ctrl & TEST_STOP))
	break;
    }

/* BUGZILLA 52. CP 20/08/2010
#ifdef TLM
  if (err)
    cpu_error (0, FAIL_TLM);
#endif
*/

  return err;
}


// Each test is a list of integers. The first item is the address of the
// peripheral being tested and the rest are control words which control
// testing of particular registers. Each control word contains the offset
// of the register from the base address and also a set of flags (defined
// above) which control the test. The last word MUST contain the TEST_STOP
// flag to terminate the test. A quick summary of the flags
//
// TEST_RST - the reset (initial value) of the register is tested
//
// TEST_ONES - specifies that the tested value is 0xffffffff
// TEST_ZEROS - specifies that the tested value is 0
// TEST_BYTE(val) - specifies the tested value (which fits into a byte)
//
// If none of the 3 above is provided, the next word in the list is used
//
// TEST_WRITE specifies that the value provided should be written to the
// register. This must not occur in the same word as TEST_STOP!

// TEST_MASK - specifies that testing should use a mask word (which follows
// in the list).
//
// TEST_RW - specifies that the register should also be tested for
// writability using 4 values - 0, 0xffffffff, 0xaaaaaaaa, 0x55555555
// After testing the word, the 'tested value' is written back.
//
// TEST_REP(count) - specifies that the test should be repeated for
// successive registers at incrementing addresses. Count is 0..255
//
// TEST_MEM(count) - specifies that the test should be repeated for
// successive registers at incrementing addresses. Offset is forced
// to be zero and count can range from 0 to 1048575.
//
// TEST_STOP - signals that this is the last word of the test
//
// Use the tester like this....
//
// rc = reg_test (timer_test);
//
// "rc" will be zero if test passed, non-zero for failure


const uint timer_test[] =
  {
    TIMER_BASE,
    1    + TEST_RST + TEST_ONES,					// T1 VALUE
    4    + TEST_RST + TEST_ZEROS + TEST_REP(2),				// T1 RIS, MIS
    0    + TEST_RST + TEST_ZEROS + TEST_RW,				// T1 LOAD
    2    + TEST_RST + TEST_MASK + TEST_RW + TEST_BYTE(0x20), 0x000000ef,// T1 CONTROL
    6    + TEST_RST + TEST_ZEROS + TEST_RW,				// T1 BGLOAD

    9    + TEST_RST + TEST_ONES,   					// Rpt for T2
    12   + TEST_RST + TEST_ZEROS + TEST_REP(2),
    8    + TEST_RST + TEST_ZEROS + TEST_RW,
    10   + TEST_RST + TEST_RW + TEST_MASK + TEST_BYTE(0x20), 0x000000ef,
    14   + TEST_RST + TEST_ZEROS + TEST_RW,

    1016 + TEST_RST + TEST_BYTE(0x04) + TEST_STOP			// Token ID reg
  };


#ifdef SPINNAKER2
const uint cc_test[] =
  {
    COMMS_CTRL_BASE,
    0    + TEST_RST + TEST_RW + TEST_MASK, 0x90000000, 0x00ff0000,	// TCR
    //1    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(2),		// TDR, TKR. TS: Bug 64
    1    + TEST_RST + TEST_ZEROS + TEST_RW,						// Bugzilla 66 CP - R1 is safe to test, but not R2 (no repeat)
    //4    + TEST_RST + TEST_ZEROS + TEST_REP(2),				// RDR, RKR             Bugzilla 66 CP, not safe
    6    + TEST_RST + TEST_RW + TEST_MASK + TEST_STOP, 0x07000000, 0x0700ffff	// SAR  Bugzilla 66 CP, not safe, added STOP
    //7    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000001,	// TEST         Bugzilla 66 CP, not safe
    //7    + TEST_WRITE + TEST_BYTE(0x01),				// TEST=1                       Bugzilla 66 CP, not safe
    //0    + TEST_ZEROS + TEST_RW + TEST_REP(6),				// TEST 6 regs          Bugzilla 66 CP, not safe
    //7    + TEST_WRITE + TEST_BYTE(0x00),				// TEST=0                       Bugzilla 66 CP, not safe
    //3    + TEST_RST + TEST_ZEROS + TEST_STOP   	       			// RSR              Bugzilla 66 CP, not safe
  };
#else
const uint cc_test[] =
  {
    COMMS_CTRL_BASE,
	0    + TEST_RST + TEST_RW + TEST_MASK, 0x80000000, 0x07ffffff, 	// TCR
	//2    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(2),		// TDR, TKR. TS: Bug 64
	2    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_STOP			// Bugzilla 66 CP - R2 is safe to test, but not R3 (no repeat)
	//4    + TEST_RST + TEST_ZEROS + TEST_REP(2),				// RDR, RKR       Bugzilla 66 CP, not safe
    //7    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000001,	// TEST   Bugzilla 66 CP, not safe
    //7    + TEST_WRITE + TEST_BYTE(0x01),     				// TEST=1             Bugzilla 66 CP, not safe
	//0    + TEST_ZEROS + TEST_RW + TEST_REP(6),		// TEST 6 regs            Bugzilla 66 CP, not safe
    //7    + TEST_WRITE + TEST_BYTE(0x00),				// TEST=0                 Bugzilla 66 CP, not safe
	//1    + TEST_RST + TEST_ZEROS + TEST_STOP   	       			// RSR        Bugzilla 66 CP, not safe
  };
#endif


#ifdef SPINNAKER2
const uint dma_test[] =
  {
    DMA_CTRL_BASE,
    1    + TEST_RST + TEST_ZEROS + TEST_MASK + TEST_REP(2), 0xfffffffc,	// ADRS, ADRT
    3    + TEST_RST + TEST_ZEROS + TEST_MASK, 0xfff9fffc,		// DESC
    4    + TEST_RST + TEST_ZEROS + TEST_MASK, 0x0000003f,		// CTRL
    5    + TEST_RST + TEST_ZEROS + TEST_MASK, 0xe0fbffff,		// STAT
    6    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x0013fc01,	// GCTL
    7    + TEST_RST + TEST_ZEROS + TEST_REP(2),				// CRCC, CRCR
    9    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x000003e0,	// TMTV
    10   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000001,	// STAT_CTL
    16   + TEST_RST + TEST_ZEROS + TEST_REP(8),				// STATS
    96   + TEST_RW + TEST_REP(32) + TEST_STOP				// CRC_POLY 32 regs
  };
#else
const uint dma_test[] =
  {
    DMA_CTRL_BASE,
    0    + TEST_RST, 0x82608edb,					// CRCP
    1    + TEST_RST + TEST_ZEROS + TEST_MASK + TEST_REP(2), 0xfffffffc,	// ADRS, ADRT
    3    + TEST_RST + TEST_ZEROS + TEST_MASK, 0xfff9fffc,		// DESC
    4    + TEST_RST + TEST_ZEROS + TEST_MASK, 0x0000003f,		// CTRL
    5    + TEST_RST + TEST_ZEROS + TEST_MASK, 0xfefdffff,		// STAT.
    6    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x0013fc01,	// GCTL
    7    + TEST_RST + TEST_ZEROS + TEST_REP(2),				// CRCC, CRCR
    9    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x000003e0,	// TMTV
    10   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000001,	// STAT_CTL
    16   + TEST_RST + TEST_ZEROS + TEST_REP(8) + TEST_STOP		// STATS
  };
#endif


const uint vic_test[] =
  {
    VIC_BASE,
    0    + TEST_RST + TEST_ZEROS + TEST_REP(2),				// IRQ,FIQ STAT
    3    + TEST_RST + TEST_ZEROS + TEST_RW,				// INT SEL
    4    + TEST_RST + TEST_ZEROS + TEST_REP(4), 			// Set-bit register
//	5    + TEST_RST + TEST_ZEROS,  					// Clr-bit register
//	6    + TEST_RST + TEST_ZEROS, 					// Set-bit register
//	7    + TEST_RST + TEST_ZEROS, 					// Clr-bit register
    8    + TEST_RST + TEST_ZEROS + TEST_MASK, 1,			// PROT
    13   + TEST_RST + TEST_ZEROS + TEST_RW,				// DEF_VEC
    64   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(16),		// ADDR 0-15
//  CNTL 0-15
    128  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK + TEST_REP(16), 0x0000003f,
    1016 + TEST_RST + TEST_BYTE(0x90) + TEST_STOP      	 	      	// Token ID reg
  };


const uint wdog_test[] =
  {
    WATCHDOG_BASE,
    1    + TEST_RST + TEST_ONES,
    2    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000003,
    4    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  5    + TEST_RST + TEST_ZEROS,
    0    + TEST_RST + TEST_ONES + TEST_RW,
//  3 do not test
    768  + TEST_RST + TEST_ZEROS,
    1016 + TEST_RST + TEST_BYTE(0x05) + TEST_STOP
  };


const uint sdram_test[] = // !! not tested
  {
    SD_RAM_BASE,
    TEST_MEM(1024) + TEST_ZEROS + TEST_RW + TEST_STOP
  };


#ifdef SPINNAKER2
const uint router_test[] =
  {
    ROUTER_BASE,
//    0    + TEST_RST + TEST_RW + TEST_MASK, 0x00800001, 0xffff1fff,	TS commented
//    1    + TEST_RST + TEST_ZEROS,										TS commented
//    2    + TEST_RST, 0x07000000,										//TS Bug 64
//    3    + TEST_RST + TEST_ZEROS + TEST_REP(3),						//TS Bug 64
//  4    + TEST_RST + TEST_ZEROS,
//  5    + TEST_RST + TEST_ZEROS,
//    6    + TEST_RST, 0x07000000,										TS commented
//    7    + TEST_RST + TEST_ZEROS + TEST_REP(4),						TS commented
//  8    + TEST_RST + TEST_ZEROS,
//  9    + TEST_RST + TEST_ZEROS,
//  10   + TEST_RST + TEST_ZEROS,
    11   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x0000ffff,
    12   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000007,
//  13 - not safe to test
    14   + TEST_RST + TEST_ZEROS + TEST_REP(18),
    32   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000fff,
    33   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00ffffff,
    128  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK + TEST_REP(16), 0x41fffdff,
//  ... more diagnostic control regs
    192  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(16),
//  ... more diagnostic counters
//  960 - not safe to test
    961  + TEST_RST + TEST_ZEROS + TEST_STOP
  };
#else
const uint router_test[] =
  {
    ROUTER_BASE,
	//0    + TEST_RST + TEST_BYTE(1) + TEST_RW + TEST_MASK, 0xffff9fc7,	//TS commented
	//1    + TEST_RST + TEST_ZEROS,										//TS commented
    //2    + TEST_RST, 0x07000000,										//TS Bug 64
    //3    + TEST_RST + TEST_ZEROS + TEST_REP(3),						//TS Bug 64
	//4    + TEST_RST + TEST_ZEROS,
	//5    + TEST_RST + TEST_ZEROS,
	//6    + TEST_RST, 0x07000000, 										//TS commented
	//7    + TEST_RST + TEST_ZEROS + TEST_REP(4),						//TS commented
	//8    + TEST_RST + TEST_ZEROS,
	//9    + TEST_RST + TEST_ZEROS,
	//10   + TEST_RST + TEST_ZEROS,
    11   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x000000ff,
    128  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK + TEST_REP(8), 0x41fffdff,
    192  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(8),
	//960  + TEST_RST + TEST_ZEROS,
    961  + TEST_RST + TEST_ZEROS + TEST_STOP
  };
#endif


const uint pl340_test[] =
  {
    PL340_BASE,
    0    + TEST_RST, 0x000000b4,
//  1    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  2    + TEST_RST + TEST_ZEROS,
#ifdef DEBUG   // BUGZILLA 52. CP 20/08/2010
    3    + TEST_RST, 0x00010020, // !! TEST_RW gets X from TLM?
#else
    3    + TEST_RST + TEST_RW, 0x00010020,
#endif
    4    + TEST_RST + TEST_RW + TEST_MASK, 0x00000a60, 0x00007fff,
    5    + TEST_RST + TEST_BYTE(0x06) + TEST_RW + TEST_MASK, 0x0000000f,
    6    + TEST_RST + TEST_BYTE(0x01) + TEST_RW + TEST_MASK, 0x00000003,
    7    + TEST_RST + TEST_BYTE(0x02) + TEST_RW + TEST_MASK, 0x0000007f,

    8    + TEST_RST + TEST_BYTE(0x07) + TEST_RW + TEST_MASK, 0x0000000f,
    9    + TEST_RST + TEST_BYTE(0x0b) + TEST_RW + TEST_MASK, 0x0000000f,
    10   + TEST_RST + TEST_BYTE(0x1d) + TEST_RW + TEST_MASK, 0x0000003f,
    11   + TEST_RST + TEST_RW + TEST_MASK, 0x00000212, 0x000003ff,

    12   + TEST_RST + TEST_BYTE(0x1d) + TEST_RW + TEST_MASK, 0x0000003f,
    13   + TEST_RST + TEST_BYTE(0x02) + TEST_RW + TEST_MASK, 0x0000000f,
    14   + TEST_RST + TEST_BYTE(0x03) + TEST_RW + TEST_MASK, 0x00000007,
    15   + TEST_RST + TEST_BYTE(0x02) + TEST_RW + TEST_MASK, 0x00000007,

    16   + TEST_RST + TEST_BYTE(0x01) + TEST_RW + TEST_MASK, 0x000000ff,
    17   + TEST_RST + TEST_BYTE(0x0a) + TEST_RW + TEST_MASK, 0x000000ff,
    18   + TEST_RST + TEST_BYTE(0x14) + TEST_RW + TEST_MASK, 0x000000ff,
    19   + TEST_RST + TEST_RW + TEST_MASK, 0x000002d1, 0x000007ff,

    20   + TEST_RST + TEST_BYTE(0x07) + TEST_RW + TEST_MASK, 0x00000007,

    64   + TEST_RST + TEST_ZEROS + TEST_MASK + TEST_REP(16), 0x000003ff,

    128  + TEST_RST + TEST_MASK + TEST_REP(2) + TEST_STOP, 0x0000ff00, 0x0001ffff,

//  192 ... DLL regs??
  };

// BUGZILLA 56. CP 20/08/2010
// This function thoroughly tests SYSRAM (excluding the region at top (highest addresses) used for status etc)
void test_sysram()
{
	#ifdef TEST_CHIP
	if(ram_test_c(SYS_RAM_BASE + 0x800, FF_SPACE_BASE, 0xFFFFFFFF)) boot_fail(SYS_RAM_FAILURE);
	#elif DEBUG
	if(ram_test_c(SYS_RAM_BASE, SYS_RAM_BASE+0x20, 0xFFFFFFFF)) boot_fail(SYS_RAM_FAILURE);
	#else
	if(ram_test_c(SYS_RAM_BASE, FF_SPACE_BASE, 0xFFFFFFFF)) boot_fail(SYS_RAM_FAILURE);
	#endif
}

void test_sdram()
{
	pointer SD_RAM = (unsigned int*) SD_RAM_BASE;
	pointer DETECTED_SDRAM_INFO = (unsigned int*) DETECTED_SDRAM_INFO_BASE;
	pointer PL340 = (unsigned int*) PL340_BASE;				// BUGZILLA 61. CP 25/08/2010, used for DLL lock delay
	unsigned int dllLocked=0, dllcountdown = 0x100;			// BUGZILLA 61. CP 25/08/2010, maximum 256 wait loops
	unsigned int thorough_test_size = 0x10;					// BUGZILLA 55. CP 20/08/2010  - leave at 0x10 - not really doing any good if bigger
	unsigned int errorcode=0;								// outcome error code of routine (0 if no errors)
	unsigned int word_offset=2;								// used to indicate SDRAM offset position & testing value (we start at 2)
	unsigned int ram_size_detected = 2;						// initialise the size of the RAM detected to 2^1 (as we start here)

	if(!reg_test(pl340_test))	// If PL340 test passes, test SDRAM otherwise record failure
	{
		init_PL340();

		while(dllcountdown--)					// BUGZILLA 61. CP 25/08/2010, wait for DLL to lock, or timeout
		{ 													// BUGZILLA 61.
			dllLocked=PL340[DLL_STATUS] & 0x00040000;		// BUGZILLA 61.
			if (dllLocked) break;							// BUGZILLA 61.
		}

		if (dllLocked)							// BUGZILLA 61. CP 25/08/2010, only run the RAM tests if DLL +ve lock
		{
			SD_RAM[0]=0;											// reset SDRAM position 0 as this is used for wraparound test

			while(word_offset<0x4000000) {							// we can have up to 0x3FFFFFF words in the max SDRAM size
				SD_RAM[word_offset] = word_offset;					// write our test word (1 in relevant bit position) into the relevant position
				if(SD_RAM[word_offset] == word_offset) {			// read back this value and check against what we'd written
					if (SD_RAM[0] != word_offset) ram_size_detected <<=1;	// if readback OK & not wrap-around double the ram size we've detected so far
				} else {											// if we don't read back the right test value
					ram_size_detected <<=1;							// multiply the ram size we've detected so far by 2, as it read (even if in error)
					errorcode|=word_offset;							// write a 1 in relevant 2^i position indicating fault
				}
				word_offset<<=1;									// increment the offset of the word we are checking by an additional power of 2
			}														// note we use the equivalent offset<<i to represent 2^i+1 for arithmetic simplicity

			if (ram_size_detected == 2) ram_size_detected=0;		// as size has not increased, then we haven't detected ANY SDRAM, so set to 0.

			if (ram_test_c(SD_RAM_BASE, SD_RAM_BASE+thorough_test_size, 0xFFFFFFFF)) errorcode|=(unsigned int)1<<30;
																// thoroughly test the first chunk of SDRAM, if problem: error bit 30
			if (ram_test_c(SD_RAM_BASE+(ram_size_detected*4)-thorough_test_size, SD_RAM_BASE+(ram_size_detected*4),0xFFFFFFFF)) errorcode|=(unsigned int)1<<31;
			 													// thoroughly test the final chunk of SDRAM, if problem: error bit 31

			DETECTED_SDRAM_INFO[0]=(4*ram_size_detected);			// populate SYSRAM info with SDRAM size detected (in bytes)
			DETECTED_SDRAM_INFO[1]=errorcode;						// populate info about any errors detected in our testing
		}
		else
		{
			DETECTED_SDRAM_INFO[1] = 0xFFFFFFFF;	// BUGZILLA 61. CP 25/08/2010, All Bits marked indicates DLL Lock failure
		}
	}
	else
	{
		DETECTED_SDRAM_INFO[1] = 0x1;	// Bit 0 marked indicates PL340 failure
	}

	// Note: Failure of SDRAM does not constitute a failure of the chip so we do not:  //boot_fail(SD_RAM_FAILURE);
}


/* Tests chip peripherals, recording error and disabling clock in case of failure. */
void test_chip_peripherals()
{
	if(reg_test(router_test))	boot_fail(ROUTER_FAILURE);
	//if(reg_test(sysctl_test))	boot_fail(SYS_CTRL_FAILURE); // Sys ctrl tests removed
	if(reg_test(wdog_test))		boot_fail(WATCHDOG_FAILURE);
}


/* Tests processor peripherals, recording error and disabling clock in case of failure. */
void test_processor_peripherals()
{
	if(reg_test(cc_test))		boot_fail(COMMS_CTRL_FAILURE);
	if(reg_test(dma_test))		boot_fail(DMA_CTRL_FAILURE);
	if(reg_test(timer_test))	boot_fail(TIMER_FAILURE);
	if(reg_test(vic_test))		boot_fail(VIC_FAILURE);
}
