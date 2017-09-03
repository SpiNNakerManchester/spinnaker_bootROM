/*****************************************************************************************
* 	Created by Steve Temple
*   Modified by Thomas Sharp   
*
*	SpiNNaker Project, The University of Manchester
*	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
* 
*****************************************************************************************/

#include "../inc/start.h"
#include "../inc/globals.h"
#include "../inc/ram_test.h"
#include <stdio.h>  // To enable printing during debug


#define OFFSET_MASK     0x00000fff
#define BYTE_MASK     	0x000ff000
#define BYTE_SHIFT     	12


#define TEST_WRITE 	0x00800000
#define TEST_ONES 	0x01000000
#define TEST_ZEROS 	0x02000000
#define TEST_RW		0x04000000
#define TEST_RST	0x08000000
#define TEST_MASK	0x10000000
#define TEST_BYTE_BIT	0x20000000
#define TEST_REP_BIT	0x40000000
#define TEST_STOP	0x80000000


#define TEST_BYTE(v)    (TEST_BYTE_BIT + (v << BYTE_SHIFT))
#define TEST_REP(v)     (TEST_REP_BIT  + (v << BYTE_SHIFT))


const uint rw_pat[] = {0x00000000, 0xffffffff, 0xaaaaaaaa, 0x55555555};


uint reg_test (char *s, const uint *vec)
{
	volatile uint *reg_base = (volatile uint *) vec[0];
	uint ptr = 1;
	uint err = 0;

	while (1)
	{
		uint ctrl = vec[ptr++];
		uint byte = (ctrl >> BYTE_SHIFT) & 255;

		uint rep = (ctrl & TEST_REP_BIT) ? byte : 1;
		uint offset = ctrl & OFFSET_MASK;

		uint mask = 0xffffffff;
		uint value;

		volatile uint *reg = reg_base + offset;

		if (ctrl & TEST_ONES)
			value = 0xffffffff;
		else if (ctrl & TEST_ZEROS)
			value = 0x00000000;
		else if (ctrl & TEST_BYTE_BIT)
			value = byte;
		else
			value = vec[ptr++];

		if (ctrl & TEST_MASK)
			mask = vec[ptr++];

		if (ctrl & TEST_WRITE)
		{
			*reg = value;
			continue;
		}

		while (rep--)
		{
	  		//if (DEBUGPRINTF) printf("%s: %08x reg %08x ptr %d\n", s, ctrl, reg, ptr);

			if ((ctrl & TEST_RST) && ((value ^ *reg) & mask))
			{
	     		if (DEBUGPRINTF) printf("!Err! Reg:%08x Expct:%08x -> Got:%08x Msk:%08x ErrBits:%08x\n", reg, value, *reg, mask, (*reg&mask)^value);
				err = 1;
			}

			if (ctrl & TEST_RW)
			{
				uint i = 0;
				const uint *pat = rw_pat;

				while ((i++ < 4) && !err)
				{
					*reg = *pat;

					if ((*reg ^ *pat) & mask)
					{
						if (DEBUGPRINTF) printf("!! RW  %08x -> %08x mask %08x\n", *pat, *reg, mask);
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
// register.

// TEST_MASK - specifies that testing should use a mask word (which follows
// in the list).
//
// TEST_RW - specifies that the register should also be tested for
// writability using 4 values - 0, 0xffffffff, 0xaaaaaaaa, 0x55555555
//
// TEST_REP(count) - specifies that the test should be repeated for
// successive registers at incrementing addresses.
//
// TEST_STOP - signals that this is the last word of the test
//
// Use the tester like this....
//
// rc = reg_test ("timer", timer_test);
//
// "rc" will be zero if test passed, non-zero for failure


const uint timer_test[] =
  {
    TIMER_BASE,
    1    + TEST_RST + TEST_ONES,
    4    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  5    + TEST_RST + TEST_ZEROS,
    0    + TEST_RST + TEST_ZEROS + TEST_RW,
    2    + TEST_RST + TEST_MASK + TEST_RW + TEST_BYTE(0x20), 0x000000ef,
    6    + TEST_RST + TEST_ZEROS + TEST_RW,

    9    + TEST_RST + TEST_ONES,
    12   + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  13   + TEST_RST + TEST_ZEROS,
    8    + TEST_RST + TEST_ZEROS + TEST_RW,
    10   + TEST_RST + TEST_RW + TEST_MASK + TEST_BYTE(0x20), 0x000000ef,
    14   + TEST_RST + TEST_ZEROS + TEST_RW,
    1016 + TEST_RST + TEST_BYTE(0x04) + TEST_STOP
  };


const uint comms_ctrl_test[] =
  {
    COMMS_CTRL_BASE,
    0    + TEST_RST + TEST_RW + TEST_MASK, 0x80000000, 0x07ffffff,
    1    + TEST_RST + TEST_ZEROS,
    2    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(2),
//  3    + TEST_RST + TEST_ZEROS + TEST_RW,
    4    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  5    + TEST_RST + TEST_ZEROS,
    7    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000001,
    7    + TEST_WRITE + TEST_BYTE(0x01),
    0    + TEST_ZEROS + TEST_RW + TEST_REP(6),
    7    + TEST_WRITE + TEST_BYTE(0x00) + TEST_STOP
  };


// !! Spin2 - CPU_ID bits & polynomial

const uint DMA_ctrl_test[] =
  {
    DMA_CTRL_BASE,
    0    + TEST_RST, 0x82608edb,
    1    + TEST_RST + TEST_ZEROS + TEST_MASK + TEST_REP(2), 0xfffffffc,
//  2    + TEST_RST + TEST_ZEROS + TEST_MASK, 0xfffffffc,
    3    + TEST_RST + TEST_ZEROS + TEST_MASK, 0xfff9fffc,
    4    + TEST_RST + TEST_ZEROS + TEST_MASK, 0x0000003f,
    5    + TEST_RST + TEST_ZEROS + TEST_MASK, 0xfefdffff,		// CP changed fefb to fefd to mask SoftReset issue
    6    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x001ffc01,
    7    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  8    + TEST_RST + TEST_ZEROS,
    9    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x000003e0,
    10   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000003,
    16   + TEST_RST + TEST_ZEROS + TEST_REP(8) + TEST_STOP
// ... more stats regs
    // ... active transfer regs
  };


const uint VIC_test[] =
  {
    VIC_BASE,
    0    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  1    + TEST_RST + TEST_ZEROS,
//  2 raw status - do not test?
    3    + TEST_RST + TEST_ZEROS + TEST_RW,
    4    + TEST_RST + TEST_ZEROS + TEST_REP(4), // Set-bit register
//  5    + TEST_RST + TEST_ZEROS, // Clr-bit register
//  6    + TEST_RST + TEST_ZEROS, // Set-bit register
//  7    + TEST_RST + TEST_ZEROS, // Clr-bit register
    8    + TEST_RST + TEST_ZEROS + TEST_MASK, 1,
    12    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(2),
//  13   + TEST_RST + TEST_ZEROS + TEST_RW,
    64   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(16),
//  ... more vector address regs
    128  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK + TEST_REP(16), 0x0000003f,
//  ... more vector control regs
    1016 + TEST_RST + TEST_BYTE(0x90) + TEST_STOP
  };


const uint router_test[] =
  {
    ROUTER_BASE,
#ifdef SPINNAKER2 
    0    + TEST_RST + TEST_BYTE(1) + TEST_RW + TEST_MASK, 0xffff9fc7,		// CP if on testchip router initialised already
    1    + TEST_RST + TEST_ZEROS,											// and may have sent packets
#endif
    2    + TEST_RST, 0x07000000,
    3    + TEST_RST + TEST_ZEROS + TEST_REP(3),
//  4    + TEST_RST + TEST_ZEROS,
//  5    + TEST_RST + TEST_ZEROS,
//    6    + TEST_RST, 0x07000000,											// CP due to serial ROM these are initialised already
//    7    + TEST_RST + TEST_ZEROS + TEST_REP(4),							// CP due to serial ROM these are initialised already
//  8    + TEST_RST + TEST_ZEROS,
//  9    + TEST_RST + TEST_ZEROS,
//  10   + TEST_RST + TEST_ZEROS,
    11   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x000000ff,
    128  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK + TEST_REP(8), 0x41fffdff,
//  ... more diagnostic control regs
    192  + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(8),
//  ... more diagnostic counters
//    960  + TEST_RST + TEST_ZEROS,											// CP populated with 0x1FF - check with ST
    961  + TEST_RST + TEST_ZEROS + TEST_STOP
  };


const uint eth_MII_test[] =
  {
    ETH_MII_REG_BASE,														// CP altered to point at register locations
    0    + TEST_RST + TEST_BYTE(0x78) + TEST_RW + TEST_MASK, 0x000007ff,
    1    + TEST_RST + TEST_ZEROS,
    2    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x000007ff,
//  3 do not test
//  4 do not test
    5    + TEST_RST + TEST_ZEROS + TEST_RW,
    6    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x0000ffff,
    7    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x0000001d,
//  8 do not test
    9    + TEST_RST + TEST_ZEROS,
    10   + TEST_RST, 0x00001000,
    11   + TEST_RST + TEST_ZEROS,
    12   + TEST_RST + TEST_BYTE(0x40),
    13   + TEST_RST + TEST_BYTE(0x11),
    14   + TEST_RST + TEST_BYTE(0x01),
    15   + TEST_RST + TEST_STOP, 0x00011123,
  };


const uint watchdog_test[] =
  {
    WATCHDOG_BASE,
    1    + TEST_RST + TEST_ONES,
    2    + TEST_RST + TEST_ZEROS + TEST_RW + TEST_MASK, 0x00000003,
    4    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  5    + TEST_RST + TEST_ZEROS,
    0    + TEST_RST + TEST_ONES + TEST_RW,
//  3 do not test
    768  + TEST_RST + TEST_ZEROS,
    960  + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  961  + TEST_RST + TEST_ZEROS,
    1016 + TEST_RST + TEST_BYTE(0x05) + TEST_STOP
  };


 // !! Spin2 - ID reg & many more...

const uint sys_ctrl_test[] =
  {
    SYS_CTRL_BASE,
//    0    + TEST_RST, 0x59100902,   // CP Spinn2 will be 0x59111018 (or so! - last 2 digits may vary depending #CPUs on chip).
//    1    + TEST_RST + TEST_ZEROS, //TS - CPUs may be disabled by testing time
    2    + TEST_RST + TEST_ZEROS + TEST_REP(2),			// CP misses out R3 and R4 as fascicle CPUs my already marked OK by now.
    6    + TEST_RST + TEST_ZEROS + TEST_REP(7),
//    13   + TEST_RST, 0x0000011f,		// CP can't test for static value as monitor is already assigned (and running this test!)
    14   + TEST_RST + TEST_ZEROS + TEST_MASK, 0xfffffffe, // !!
//    15   + TEST_RST + TEST_ZEROS,				// Reassigned to GP/IO in SpiNN2 - TODO check
#ifdef SPINNAKER2 
    16   + TEST_RST + TEST_ZEROS + TEST_RW,
    17   + TEST_RST + TEST_ONES + TEST_RW,		// CP:  GPIO pins used for signalling on testboard
#endif
//  18 do not test
//  19 do not test
//    20   + TEST_RST + TEST_RW + TEST_MASK + TEST_REP(2), 0x0002010a, 0x01073f3f,		// CP PLL already setup and running
//  21   + TEST_RST + TEST_RW + TEST_MASK, 0x0002010a, 0x01073f3f,
//    22   + TEST_RST + TEST_ZEROS + TEST_RW + TEST_REP(2),  // Reassigned to GP/IO in SpiNN2 - TODO check
//  23   + TEST_RST + TEST_ZEROS + TEST_RW,
//  24 do not test !!
    15   + TEST_RST + TEST_ZEROS,
//    32   + TEST_RST, 0x80000000,	//TS - Testing arbiter makes monitor arbitration hard
//    33   + TEST_RST + TEST_ZEROS + TEST_REP(31),
    65   + TEST_RST + TEST_ZEROS + TEST_REP(30),				// CP test and set registers0 & 31 in use already (was 64 with rep 31)
    97   + TEST_RST + TEST_REP(31) + TEST_STOP, 0x80000000
  };


// !! Spin2 - number of chips?

const uint PL340_test[] =
  {
    PL340_BASE,
    0    + TEST_RST, 0x000000b4,
    1    + TEST_RST + TEST_ZEROS + TEST_REP(2),
//  2    + TEST_RST + TEST_ZEROS,
    3    + TEST_RST + TEST_RW, 0x00010020,

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

    20   + TEST_RST + TEST_BYTE(0x07) + TEST_RW + TEST_MASK, 0x00001fff,

    64   + TEST_RST + TEST_ZEROS + TEST_MASK + TEST_REP(16), 0x000003ff,

    128  + TEST_RST + TEST_MASK + TEST_REP(2) + TEST_STOP, 0x0000ff00, 0x0001ffff,

//  192 ... DLL regs??
  };



// Tests chip peripherals, recording error and disabling clock in case of failure
void test_chip_peripherals()
{
	if(reg_test("eth_mii", eth_MII_test))		boot_fail(ETH_MII_FAILURE);
	if(reg_test("PL340", PL340_test))			boot_fail(PL340_FAILURE);
	if(reg_test("router", router_test))			boot_fail(ROUTER_FAILURE); 
	if(reg_test("sys ctrl", sys_ctrl_test))		boot_fail(SYS_CTRL_FAILURE); 
	if(reg_test("watchdog", watchdog_test))		boot_fail(WATCHDOG_FAILURE);
}


// Tests processor peripherals, recording error and disabling clock in case of failure
void test_processor_peripherals()
{
	if(reg_test("comms_ctrl", comms_ctrl_test))	boot_fail(COMMS_CTRL_FAILURE);
	if(reg_test("DMA_ctrl", DMA_ctrl_test))		boot_fail(DMA_CTRL_FAILURE);
	if(reg_test("timer", timer_test))			boot_fail(TIMER_FAILURE);
	if(reg_test("VIC", VIC_test))				boot_fail(VIC_FAILURE);
}


// this test strides through the SDRAM at word postions in positive powers of 2. 
//       ie. SD_RAM[0x2] ... 0x4,0x8,0x10,0x20 ... 0x4000,0x8000,0x10000,0x20000 ... 0x400000,0x800000,0x1000000,0x2000000
//          (as max SDRAM size is 0x10000000 bytes, then 0x0->0x3FFFFFF is the range of possible valid positions of words)
//   it then does a more intensive test of the first and last 4KB of SDRAM
void test_sdram()
{
	pointer SD_RAM = (unsigned int*) SD_RAM_BASE;
	pointer DETECTED_SDRAM_INFO = (unsigned int*) DETECTED_SDRAM_INFO_BASE;
	unsigned int thorough_test_size = 0x10;			// !!! TLM !!! was 0x1000 how many bytes of memory we want to perform top&btm exhaustive testing on (4KB)
	unsigned int errorcode=0;								// outcome error code of routine (0 if no errors)
	unsigned int word_offset=2;								// used to indicate SDRAM offset position & testing value (we start at 2)
	unsigned int ram_size_detected = 2;						// initialise the size of the RAM detected to 2^1 (as we start here)

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
		
	// Note: Failure of SDRAM does not constitute a failure of the chip so we do not:  //boot_fail(SD_RAM_FAILURE);
}



/*
#-------------------------------------------------------------------------------

# !! Some changes in Spin2?

PL340_BASE @ f0000000 size 00001000

MC_STAT		RO	00	000000b4	ffffffff
MC_CMD 		WO	04	00000000	ffffffff
MC_DIRC 	WO	08	00000000	ffffffff
MC_MCFG 	RW	0c	00010020	ffffffff

MC_REFP 	RW	10	00000a60	00007fff
MC_CASL 	RW	14	00000006	0000000f
MC_DQSS 	RW	18	00000001	00000003
MC_MRD		RW	1c	00000002	0000007f

MC_RAS		RW	20	00000007	0000000f
MC_RC		RW	24	0000000b	0000000f
MC_RCD		RW	28	0000001d	0000003f
MC_RFC		RW	2c	00000212	000003ff

MC_RP		RW	30	0000001d	0000003f
MC_RRD		RW	34	00000002	0000000f
MC_WR		RW	38	00000003	00000007
MC_WTR		RW	3c	00000002	00000007

#16
MC_XP		RW	40	00000001	000000ff
MC_XSR		RW	44	0000000a	000000ff
MC_ESR		RW	48	00000014	000000ff
MC_MCFG2	RW	4c	000002d1	000007ff
#20
MC_MCFG3	RW	50	00000007	00001fff
#64
MC_QOS0		RW	100	00000000	000003ff
MC_QOS1		RW	104	00000000	000003ff
MC_QOS2		RW	108	00000000	000003ff
MC_QOS3		RW	10c	00000000	000003ff
MC_QOS4		RW	110	00000000	000003ff
MC_QOS5		RW	114	00000000	000003ff
MC_QOS6		RW	118	00000000	000003ff
MC_QOS7		RW	11c	00000000	000003ff
MC_QOS8		RW	120	00000000	000003ff
MC_QOS9		RW	124	00000000	000003ff
MC_QOS10	RW	128	00000000	000003ff
MC_QOS11	RW	12c	00000000	000003ff
MC_QOS12	RW	130	00000000	000003ff
MC_QOS13	RW	134	00000000	000003ff
MC_QOS14	RW	138	00000000	000003ff
MC_QOS15	RW	13c	00000000	000003ff

MC_CCFG0	RW	200	0000ff00	0001ffff
MC_CCFG1	RW	204	0000ff00	0001ffff
MC_CCFG2	RW	208	0000ff00	0001ffff
MC_CCFG3	RW	20c	0000ff00	0001ffff

DLL_STATUS	RO	300	00240001	!!
DLL_CONFIG0	WO	304	00000000	!!
DLL_CONFIG1	WO	308	00000000`	!!

#-------------------------------------------------------------------------------
# !! Chip ID change for Spin2

SYSCTL_BASE @ f2000000 size ??

SC_CHIP_ID	RO	00	59100902	ffffffff

SC_CPU_DIS	RW	04	00000000	000fffff !! 5ec
SC_SET_IRQ	RW	08	00000000	000fffff !! 5ec
SC_CLR_IRQ	RW	0c	00000000	000fffff !! 5ec

SC_SET_OK	RWS	10	00000000	ffffffff
SC_CLR_OK	RWC	14	00000000	ffffffff

SC_SOFT_RST_L	RW	18	00000000	000fffff !! 5ec
SC_HARD_RST_L   RW  	1c	00000000	000fffff !! 5ec
SC_SUBS_RST_L   RW  	20	00000000	000fffff !! 5ec

SC_SOFT_RST_P	RW	24	00000000	000fffff !! 5ec & pulse
SC_HARD_RST_P   RW  	28	00000000	000fffff !! 5ec & pulse
SC_SUBS_RST_P   RW  	2c	00000000	000fffff !! 5ec & pulse

SC_RST_CODE	RO	30	00000000	00000007
SC_MON_ID	RW	34	0000011f	0001011f !! 5ec etc

SC_MISC_CTRL    RW	38	00000000	ffffffff !! NB bit 0
SC_MISC_STAT    RO	3c	00000000	ffffffff

GPIO_PORT	RW	40	00000000	ffffffff
GPIO_DIR	RW	44	ffffffff	ffffffff
GPIO_SET	NT	48
GPIO_CLR	NT	4c

SC_PLL1		RW	50	0002010a	01073f3f !! check init value
SC_PLL2		RW	54	0002010a	01073f3f
SC_TORIC1	RW	58	00000000	ffffffff
SC_TORIC2	RW	5c	00000000	ffffffff
SC_CLKMUX	RW	60	00000000	df0303ff !! dangerous!

SC_ARB0		NT	80	!! special
SC_TAS0		NT	100	!! special
SC_TAC0		NT	180	!! special

SC_MISC_TEST	RW	200	00000000	ffffffff
SC_LINK_DIS	RW	204	00000000	003f003f

#-------------------------------------------------------------------------------


WDOG @ f3000000 size 00001000

# After testing reg restore to reset value

# Name		Type    Offset	Reset		Mask		

WD_LOAD   	RW    00	ffffffff	ffffffff
WD_COUNT  	RO    04	ffffffff
WD_CTRL   	RW    08	00000000	00000003
WD_INTCLR 	WO    0c	
WD_RAWINT 	RO    10	00000000
WD_MSKINT 	RO    14	00000000
WD_LOCK   	RO**  c00	00000000

WD_PID0		RO    fe0	00000005
WD_PID1		RO    fe4	00000018
WD_PID2		RO    fe8	00000004
WD_PID3		RO    fec	00000000

WD_CID0		RO    ff0	0000000d
WD_CID1		RO    ff4	000000f0
WD_CID2		RO    ff8	00000005
WD_CID3		RO    ffc	000000b1
#-------------------------------------------------------------------------------

ETH_BASE @ f400c000 size 00000080

ETH_COMMAND	RW	00	00000078	000007ff
ETH_STATUS  	RO	04	00000000	ffffffff
ETH_TX_LEN 	RW	08	00000000	000007ff
ETH_TX_CMD 	WO	0c
ETH_RX_CMD 	WO	10
ETH_MAC_LO 	RW	14	00000000	ffffffff
ETH_MAC_HI 	RW	18	00000000	0000ffff
ETH_PHY_CTRL    RW	1c	00000000	0000001d
ETH_IRQ_CLR	WO	20

ETH_RX_BUF_RP	RO	24	00000000	
ETH_RX_BUF_WP	RO	28	00001000
ETH_RX_DESC_RP	RO	2c	00000000
ETH_RX_DESC_WP	RO	30	00000040
ETH_RX_STATE	RO	34	!!
ETH_TX_STATE	RO	38	!!
ETH_PER_ID	RO	3c	00011123	ffffffff	

#-------------------------------------------------------------------------------

RTR_BASE @ f1000000 size 00004000

RTR_CONTROL	RW	00	00000001	ffff9fc7
RTR_STATUS	RO	04	00000000	ffffffff
RTR_EHDR	RO	08	07000000	ffffffff
RTR_EKEY	RO	0c	00000000	ffffffff
RTR_EDAT	RO	10	00000000	ffffffff
RTR_ESTAT	RO	14	00000000	ffffffff
RTR_DHDR	RO	18	00000000	ffffffff
RTR_DKEY	RO	1c	00000000	ffffffff
RTR_DDAT	RO	20	00000000	ffffffff
RTR_DLINK	RO	24	00000000	ffffffff
RTR_DSTAT	RO	28	00000000	ffffffff
RTR_DGEN	RW	2c	00000000	000000ff

RTR_DGF0	RW	200	00000000	41fffdff
RTR_DGF1	RW	204	00000000	41fffdff
RTR_DGF2	RW	208	00000000	41fffdff
RTR_DGF3	RW	20c	00000000	41fffdff
RTR_DGF4	RW	210	00000000	41fffdff
RTR_DGF5	RW	214	00000000	41fffdff
RTR_DGF6	RW	218	00000000	41fffdff
RTR_DGF7	RW	21c	00000000	41fffdff

RTR_DGC0	RW	300	00000000	ffffffff
RTR_DGC1	RW	304	00000000	ffffffff
RTR_DGC2	RW	308	00000000	ffffffff
RTR_DGC3	RW	30c	00000000	ffffffff
RTR_DGC4	RW	310	00000000	ffffffff
RTR_DGC5	RW	314	00000000	ffffffff
RTR_DGC6	RW	318	00000000	ffffffff
RTR_DGC7	RW	31c	00000000	ffffffff

RTR_TST1	RO	f00	00000000	ffffffff
RTR_TST2	RW	f04	00000000	ffffffff

#-------------------------------------------------------------------------------

VIC_BASE @ 2f000000 size 1000

# Name		Type    Offset	Reset		Mask		

VIC_IRQST	RO	00	00000000
VIC_FIQST	RO	04	00000000
VIC_RAW		WO **	08
VIC_SELECT	RW	0c	00000000	ffffffff
VIC_ENABLE	RW **	10	00000000	ffffffff
VIC_ENCLR	WO **	14
VIC_SOFTINT	RW	18	00000000	ffffffff
VIC_SOFTCLR	WO	1c
VIC_PROTECT 	RW	20	00000000	00000001
VIC_VADDR	RW	30	00000000	ffffffff
VIC_DEFADDR	RW **	34	00000000	ffffffff

VIC_ADDR0	RW	100	00000000	ffffffff
VIC_ADDR15	RW	13c	00000000	ffffffff

VIC_CNTL0	RW	200	00000000	0000003f
VIC_CNTL15	RW	23c	00000000	0000003f

VIC_PID0	RO    	fe0	00000090
VIC_PID1	RO    	fe4	00000011
VIC_PID2	RO    	fe8	00000004
VIC_PID3	RO    	fec	00000000

VIC_CID0	RO    	ff0	0000000d
VIC_CID1	RO    	ff4	000000f0
VIC_CID2	RO    	ff8	00000005
VIC_CID3	RO    	ffc	000000b1

#-------------------------------------------------------------------------------
# !! Spin2 - DMA_STAT (CPU ID)

DMA_BASE @ 30000000 size 00000200

DMA_CRC  	RO	00	82608edb	ffffffff
DMA_ADRS 	RW	04	00000000	fffffffc
DMA_ADRT 	RW	08	00000000	fffffffc
DMA_DESC 	RW	0c	00000000	fff9fffc
DMA_CTRL 	RW	10	00000000	0000003f
DMA_STAT 	RO	14	00000000	fefbffff
DMA_GCTL 	RW	18	00000000	001ffc01
DMA_CRCC 	RO	1c	00000000	ffffffff
DMA_CRCR 	RO	20	00000000	ffffffff
DMA_TMTV 	RW	24	00000000	000003e0
DMA_SCTL 	RW	28	00000000	00000003

DMA_STAT0 	RO	40	00000000	ffffffff
DMA_STAT1 	RO	44	00000000	ffffffff
DMA_STAT2 	RO	48	00000000	ffffffff
DMA_STAT3 	RO	4c	00000000	ffffffff
DMA_STAT4 	RO	50	00000000	ffffffff
DMA_STAT5 	RO	54	00000000	ffffffff
DMA_STAT6 	RO	58	00000000	ffffffff
DMA_STAT7	RO	5c	00000000	ffffffff

DMA_CRC2	RO	100
DMA_AD2S	RO	104
DMA_AD2T	RO	108
DMA_DES2	RO	10c

#-------------------------------------------------------------------------------

# NB - setting bit 0 of CC_TEST allows memtest of first 6 regs

CC_BASE @ 10000000 size 00001000

CC_TCR		RW	00	80000000	07ffffff
CC_RSR		RO	04	00000000	ffffffff
CC_TXDATA	RW	08	00000000	ffffffff
CC_TXKEY	RW	0c	00000000	ffffffff
CC_RXDATA	RO	10	00000000	ffffffff
CC_RXKEY	RO	14	00000000	ffffffff
CC_TEST 	RW	1c	00000000	00000001

#-------------------------------------------------------------------------------

TIMER_BASE @ 21000000 size 00001000

# Name		Type    Offset	Reset		Mask		

TIM1_LOAD	RW	00	00000000	ffffffff
TIM1_VAL	RO	04	ffffffff
TIM1_CTRL	RW	08	00000020	000000ff
TIM1_INTC	WO	0c
TIM1_RAW	RO	10	00000000
TIM1_MASK	RO	14	00000000
TIM1_BGL	RW	18	00000000

TIM2_LOAD	RW	20	00000000	ffffffff
TIM2_VAL	RO	24	ffffffff
TIM2_CTRL	RW	28	00000020	000000ff
TIM2_INTC	WO	2c
TIM2_RAW	RO	30	00000000
TIM2_MASK	RO	34	00000000
TIM2_BGL	RW	38	00000000

TC_PID0		RO    	fe0	00000004
TC_PID1		RO    	fe4	00000018
TC_PID2		RO    	fe8	00000004
TC_PID3		RO    	fec	00000000

TC_CID0		RO    	ff0	0000000d
TC_CID1		RO    	ff4	000000f0
TC_CID2		RO    	ff8	00000005
TC_CID3		RO    	ffc	000000b1

*/
