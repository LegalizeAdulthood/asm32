//	dsp32c.cpp - class functions for DSP-32C board
//
//	setup for 2 DSP boards only (as is the I/O-address-selection PAL)
//
//	The DSP32C is set up to do 16-bit DMAs; PDR2 is never used
//	(except by user via pdr32()/pdr32(long)).
//	All errors are masked off by reset() - they can be re-enabled
//	by using emr(int).
//
//	Creating a DspBoard instance DOES NOT affect any program on the DSP.
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

#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include "dsp32c.h"

const int ORG1=0x300, ORG2=0x310;	// I/O origins

// I/O ports
#define	PAR_L	(org+0)
#define PAR_H	(org+1)
#define PDR_L	(org+2)
#define PDR_H	(org+3)
#define EMR_L	(org+4)
#define EMR_H	(org+5)
#define ESR	(org+6)
#define PCR_L	(org+7)
#define PIR_L	(org+8)
#define PIR_H	(org+9)
#define PCR_H	(org+10)
#define PARE	(org+11)
#define PDR2_L	(org+12)
#define PDR2_H	(org+13)

const int PCR_HALT=0x0402;	// HALT, full regmap, PIF disabled,
				// no DMA, no AutoIncrement, 16-bit DMA,
				// PIO is 8 bits, PIF/PDF on trailing edge
const int PCR_RUN=PCR_HALT|1;	// as above, but RUN


// Creating a DspBoard instance DOES NOT affect the currently-running
// DSP program (if any).
DspBoard::DspBoard(int boardnum)
{
	switch(boardnum) {
	case 1:	org = ORG1;	break;
	case 2: org = ORG2;	break;
	default: org = 0;	return;
	}

	// check that board is present
	// note that on reset, PCR bit 1 is cleared; when set by this
	// class, PCR bit 7 is always 0
	if(inportb(PCR_L) == 0xFF)
		org = 0;

	// init PCR with compatible data, preserving its current contents
	if(org != 0)
		pcr(pcr());
}

// resets the DSP and holds it in reset
void DspBoard::reset()
{
	if(org) {
		pcr(PCR_HALT);	// reset DSP and init PCR
		emr(0xFFFF);	// mask off all errors
		pcr(PCR_HALT);	// do it again for good luck
	}
}

// returns current contents of PCR (returns -1 if not present)
int DspBoard::pcr()
{
	if(org) {
		int v = inportb(PCR_L);
		v |= (int)inportb(PCR_H) << 8;
		return v;
	}
	return -1;
}

// sets PCR
void DspBoard::pcr(int value)
{
	value &= 0x051F;	// zero reserved bits, force 8-bit PIO
	value |= 0x0402;	// force extended regs, trailing edge

	if(org) {
		outportb(PCR_L,value);
		outportb(PCR_H,value>>8);
	}
}

// contents of ESR (returns -1 if not present)
int DspBoard::esr()
{
	return (org ? (int)inportb(ESR) : -1);
}

// returns current contents of EMR (returns -1 if not present)
int DspBoard::emr()
{
	if(org) {
		int v = inportb(EMR_L);
		v |= (int)inportb(EMR_H) << 8;
		return v;
	}
	return -1;
}

// sets EMR
void DspBoard::emr(int value)
{
	if(org) {
		outportb(EMR_L,value);
		outportb(EMR_H,value>>8);
	}
}

// returns current contents of PIR (returns -1 if not present)
int DspBoard::pir()
{
	if(org) {
		int v = inportb(PIR_L);
		v |= (int)inportb(PIR_H) << 8;
		return v;
	}
	return -1;
}

// sets PIR
void DspBoard::pir(int value)
{
	if(org) {
		outportb(PIR_L,value);
		outportb(PIR_H,value>>8);
	}
}

// returns current contents of PDR,PDR2 (returns -1L if not present)
long DspBoard::pdr32()
{
	if(org) {
		long v = inportb(PDR2_L);
		v |= (long)inportb(PDR2_H) << 8;
		v |= (long)inportb(PDR_L) << 16;
		v |= (long)inportb(PDR_H) << 24;
		return v;
	}
	return -1L;
}

// sets PDR,PDR2
void DspBoard::pdr32(long value)
{
	if(org) {
		outportb(PDR2_L,value);
		outportb(PDR2_H,value>>8);
		outportb(PDR_L,value>>16);
		outportb(PDR_H,value>>24);
	}
}

// returns current contents of PDR (returns -1 if not present)
int DspBoard::pdr()
{
	if(org) {
		int v = inportb(PDR_L);
		v |= (int)inportb(PDR_H) << 8;
		return v;
	}
	return -1L;
}

// sets PDR
void DspBoard::pdr(int value)
{
	if(org) {
		outportb(PDR_L,value);
		outportb(PDR_H,value>>8);
	}
}

// returns current contents of PAR (returns -1L if not present)
// note that this routine clears PAR bit 0; it always reads as 1, but is
// always used as 0 during DMA. This makes it look correct to the PC prog.
long DspBoard::par()
{
	if(org) {
		long v = inportb(PAR_L);
		v |= (long)inportb(PARE) << 16;
		v |= (long)inportb(PAR_H) << 8;
		return v & 0x00FFFFFEL; // clear bit 0
	}
	return -1L;
}

// sets PAR
void DspBoard::par(long value)
{
	if(org) {
		outportb(PAR_L,value);
		outportb(PARE,value>>16);
		outportb(PAR_H,value>>8);
	}
}

// starts DSP executing; does reset first
void DspBoard::run()
{
	if(org) {
		reset();
		pcr(PCR_RUN);
		pcr(PCR_RUN);
	}
}

// reads data from DSP memory
// Does not reset the DSP. Note that if the DSP program writes to PDR,
// the data returned by this routine will be wrong!
void DspBoard::read(long addr, char *buf, int nbuf)
{
	if(!org)
		return;

	// round nbuf up to be even
	if((nbuf&1) == 1)
		++nbuf;

	// set DMA address
	outportb(PARE,addr>>16);
	outportb(PAR_L,addr);
	outportb(PAR_H,addr>>8);

	// enable DMA from DSP memory
	int org_pcr = set_dma(DSP_DMA16INCR);

	// do initial DMA (ignore stale data)
	inportb(PDR_H);

	// read the data - DSP32C is faster, always ready
	while(nbuf > 0) {
		*buf++ = inportb(PDR_L);
		*buf++ = inportb(PDR_H);
		nbuf -= 2;
	}

	// restore PCR
	pcr(org_pcr);
}

// writes data to DSP memory
void DspBoard::write(long addr, char *buf, int nbuf)
{
	if(!org)
		return;

	// round nbuf up to be even
	if((nbuf&1) == 1)
		++nbuf;

	// set DMA address
	outportb(PAR_L,addr);
	outportb(PAR_H,addr>>8);
	outportb(PARE,addr>>16);

	// enable DMA to DSP memory
	int org_pcr = set_dma(DSP_DMA16INCR);

	// transfer the data - DSP32C is faster, always ready
	while(nbuf > 0) {
		outportb(PDR_L,*buf++);
		outportb(PDR_H,*buf++);
		nbuf -= 2;
	}

	// restore PCR
	pcr(org_pcr);
}

// loads a file into DSP memory  returns 0 if OK, -1 if error
int DspBoard::load(char *filename, long addr)
{
	const int BLKSIZE=2048;
	int retval = -1;

	if(org) {
		char *buf = new char[BLKSIZE];
		int in = open(filename,O_RDONLY|O_BINARY);
		if(in >= 0) {
			int n;
			do {
				n = ::read(in,buf,BLKSIZE);
				if(n > 0) {
					write(addr,buf,n);
					addr += n;
				}
			} while(n > 0);
			close(in);
			if(n == 0)
				retval = 0;
		}
		delete buf;
	}

	return retval;
}

// writes data to DSP memory
void DspBoard::write32(long addr, long value)
{
	if(!org)
		return;

	// round addr down to be even
	addr &= ~3;

	// set DMA address
	outportb(PAR_L,addr);
	outportb(PAR_H,addr>>8);
	outportb(PARE,addr>>16);

	// enable DMA to DSP memory
	int org_pcr = set_dma(DSP_DMA32);

	// write data
	pdr32(value);

	// restore PCR
	pcr(org_pcr);
}

