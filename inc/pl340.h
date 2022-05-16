#ifndef PL340_H

#define PL340_H


#ifdef QUIMONDA

// Quimonda 133MHz SDRAM - 256Mb - 32MB

// Config = 0x00018009
// Refresh = 2080 (15.6us / 7.5ns)

const uint pl340_data[] = {
  0x00000006,	// [0]  MC_CASL CAS latency =3	     
  0x00000001,	// [1]  MC_DQSS T_dqss		     
  0x00000002,	// [2]  MC_MRD  T_mrd		     
  0x00000006,	// [3]  MC_RAS  T_ras		     
  0x00000009,	// [4]  MC_RC   T_rc		     
  0x00000003,	// [5]  MC_RCD  T_rcd		     
  0x0000007a,	// [6]  MC_RFC  T_rfc		     
  0x00000003,	// [7]  MC_RP   T_rp		     
  0x00000002,	// [8]  MC_RRD  T_rrd		     
  0x00000002,	// [9]  MC_WR   T_wr		     
  0x00000001,	// [10] MC_WTR  T_wtr		     
  0x00000002,	// [11] MC_XP   T_xp		     
  0x00000010,	// [12] MC_XSR  T_xsr		     
  0x00000014,	// [13] MC_ESR  T_esr		     
		                                     
  0x00018009,   // [14] Memory_Cfg register (burst=8)
  2080		// [15] Refresh period               
};

#endif
#ifndef QUIMONDA

// Micron 166MHz SDRAM - 1Gb - 128MB

// Config = 0x00018012
// Refresh = 1300 (7.8us / 6ns)

const uint pl340_data[] = {
  0x00000006,  	// [0]  MC_CASL CAS latency =3
  0x00000001,  	// [1]  MC_DQSS T_dqss
  0x00000002,  	// [2]  MC_MRD  T_mrd
  0x00000007,  	// [3]  MC_RAS  T_ras
  0x0000000A,  	// [4]  MC_RC   T_rc
  0x00000003,  	// [5]  MC_RCD  T_rcd
  0x00000275,  	// [6]  MC_RFC  T_rfc
  0x00000003,  	// [7]  MC_RP   T_rp
  0x00000002,  	// [8]  MC_RRD  T_rrd
  0x00000003,  	// [9]  MC_WR   T_wr
  0x00000001,  	// [10] MC_WTR  T_wtr
  0x00000005,  	// [11] MC_XP   T_xp
  0x00000017,  	// [12] MC_XSR  T_xsr
  0x00000014,  	// [13] MC_ESR  T_esr

  0x00018012,   // [14] Memory_Cfg register (burst=8)
  1300		// [15] Refresh period               
};

#endif



void pl340_init (const uint table[]);

#endif
