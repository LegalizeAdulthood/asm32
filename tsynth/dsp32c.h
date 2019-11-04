//	dsp32c.h - definitions for DSP-32C board

#include <dos.h>

//	DspBoard - class for low-level control of a DSP-32C board.
//
//	External definitions only; see dsp32c.cpp for internal defs
//	(I/O ports, etc.).
//
//	Creating an instance of DspBoard DOES NOT reset the DSP -
//	it must be explicitly reset and halted by using reset().
//	No status (other than the presence/absence of the hardware board)
//	is kept in DspBoard, so instances can be copied and
//	created/destroyed at any time without affecting the program
//	executing in the DSP. Keeping track of what program is running
//	in the DSP, and communicating with it, are the responsibilities
//	of the calling (controlling) program.
//
//	load/read/write DO NOT halt the DSP, nor do they start it running.
//
//	PC-DSP DMA is normally disabled, except within load/read/write.
//
//		Written by: Tom Roberts, 11/15/91
//
//
/***************

THIS SOFTWARE IS RELEASED "AS IS", WITH NO WARRANTY EXPRESSED OR IMPLIED.
This software is copyright 1984, 1991, 1992, 1993, 1994 by Tom Roberts.
License is granted for unlimited distribution, as long as the following
conditions are met:
  A. This notice is preserved intact.
  B. No charge is made for the software (other than a reasonable
     distribution/duplication fee).

***************/

class DspBoard {
protected:
	int	org;		// I/O port origin; 0 if no board is present
public:
	DspBoard(int boardnum);	// boardnum = 1,2,... up to # in computer
	int is_present();	// non-zero if DSP board is present
	int is_running();	// non-zero if DSP is running
	int is_pir_full();	// non-zero if PIR has new data
	int is_pdr_full();	// non-zero if PDR has new data
	void reset();           // resets the DSP and holds it in reset
	int pcr();		// contents of PCR (-1 if no board)
	void pcr(int value);	// sets PCR
	int esr();		// contents of ESR (-1 if no board)
	int emr();		// contents of EMR (-1 if no board)
	void emr(int value);	// sets EMR
	int pir();		// contents of PIR (-1 if no board)
	void pir(int value);	// sets PIR
	long pdr32();		// contents of PDR,PDR2 (-1L if no board)
	void pdr32(long value);	// sets PDR,PDR2
	long par();		// contents of PAR (-1L if no DSP board)
	void par(long value);	// sets PAR
	int pdr();		// contents of PDR (-1 if no board)
	void pdr(int value);	// sets PDR
	int set_dma(int flags);	// enables/disables DMA to/from DSP;
				//			returns prev-PCR
	void run();		// starts DSP executing; does reset first
	// For the following, addr and nbuf will be forced to be even:
	void read(long addr, char *buf, int nbuf); // read DSP memory
	void write(long addr, char *buf, int nbuf);// write DSP memory
	int load(char *filename, long addr=0L); // load file into DSP memory
				// returns 0 if OK, -1 if failure
	void write32(long addr, long value); // writes DSP memory
};
// flags for set_dma():
const int DSP_DMA16=0x0008;	// DMA is 16 bits
const int DSP_DMA16INCR=0x0018;	// DMA is 16 bits, autoincrement
const int DSP_DMA32=0x0108;	// DMA is 32 bits
const int DSP_DMA32INCR=0x0118;	// DMA is 32 bits, autoincrement
const int DSP_DMADISABLE=0x0000;// disable DMA

inline int DspBoard::is_present() { return org != 0; }
inline int DspBoard::is_running() { return (org!=0) && (inportb(org+7)&1); }
inline int DspBoard::is_pir_full() { return (org!=0) && (inportb(org+7)&64); }
inline int DspBoard::is_pdr_full() { return (org!=0) && (inportb(org+7)&32); }
inline int DspBoard::set_dma(int flags) { if(!org) return -1;
	int i=pcr(); 	pcr((i&~0x0118)|(flags&0x0118)); return i; }

// the following are in SAMPLE.CPP
void playsample(DspBoard dsp, int huge *buf, long n_buf);
void sample(DspBoard dsp, int huge *buf, long n_buf);
