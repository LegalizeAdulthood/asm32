/*	memtest.s - DSP32C memory test

	Waits for data to be put into PIR from the host, then
	does a memory test of the memory at address (PDRE), and
	size (256*PIR). When started, it will zero PDRE. When
	completed, it will output (actual) ^ (correct) to PIR, and
	the address to PDRE. If no errors are found, both PIR and PDRE
	will be zeroed.

	Thus, the host writing to PIR will start the test, and the DSP32C
	writing to PIR will indicate its completion. The host must write
	PDRE with the address BEFORE writing PIR; similarly, once PIF is
	set, the host should read PDRE before PIR.

	This program uses r19 AND r18 for REFRESH at FRAME-INT level.
	The program itself does NOT need to do any REFRESH() calls at all.
*/

/*
	The INITIAL_PAT defines how good a test this is for detecting
	cross-writes between data-bits, and how fast it runs.
	0x0001 is the best, and the slowest, 0x5555 is 16 times faster,
	and will still catch adjacent data-bit cross-writes
	(x8 because 8 bits are on, x2 because ROTATE(0x5555)==~0x5555).

	The algorithm is loosely based on the article "Fast Memory Test
	Checks Individual Bits", by E. J. Milner, EDN, Oct 13, 1983.
*/
/***************

THIS SOFTWARE IS RELEASED "AS IS", WITH NO WARRANTY EXPRESSED OR IMPLIED.
This software is copyright 1984, 1991, 1992, 1993, 1994 by Tom Roberts.
License is granted for unlimited distribution, as long as the following
conditions are met:
  A. This notice is preserved intact.
  B. No charge is made for the software (other than a reasonable
     distribution/duplication fee).

***************/

#define INITIAL_PAT	0x0001


#include "dsp32c.i"	// DSP32C board definitions

	DSP_INIT
	DAC_DISABLE

	FRAME_INT(frame)// used for REFRESH only
	
	r1 = pir	// empty any stale data

//	Registers

//	size = size of block to test, max = addr of last word to test
//	adr = current address within block, pat = current pattern
#define size	r9
#define sizeE	r9e
#define block	r10e
#define adr	r11e
#define pat	r8
#define key_pat	r7
#define fill_pat r6
#define max	r12e


start:	nop			// wait for start command/data from host
	r1e = 1
	pdre = r1e

start1:	nop
	if(pie) goto start1
	nop

	block = pdre		// beginning address
	r1e = 0xFFFFFE
	block = block & r1e
	r1e = end_prog		// (cannot test below program code)
	block - r1e
	if(ls) block = r1e

	size = pir		// 256*byte count to test
	nop
	sizeE = sizeE + sizeE
	sizeE = sizeE + sizeE
	sizeE = sizeE + sizeE
	sizeE = sizeE + sizeE
	sizeE = sizeE + sizeE
	sizeE = sizeE + sizeE
	sizeE = sizeE + sizeE
	sizeE = sizeE + sizeE

	r1e = 0xFFFFFE		// (cannot test last word - addr wrap)
	max = block + sizeE	// max = addr of last word in block
	if(cs) max = r1e
	max = max - 2

	key_pat = INITIAL_PAT
	fill_pat = 0xFFFF

	r1e = 0
	pdre = max

fill:
	pat = fill_pat
	adr = block
fill_1:	adr - max
	if(ls) goto fill_1
	*adr++ = pat

loop:
	pat = key_pat
	adr = block
loop_1:	adr - max
	if(hi) goto loop_2
	r1 = *adr
	nop
	r1 = r1 ^ fill_pat
	if(ne) goto error
	nop
	*adr++ = pat
	pat = pat * 2
	if(cs) pat = pat + 1
	goto loop_1
	nop
loop_2:

	pat = key_pat
	adr = block
loop_3:	adr - max
	if(hi) goto loop_4
	r1 = *adr
	nop
	r1 = r1 ^ pat
	if(ne) goto error
	nop
	*adr++ = fill_pat
	pat = pat * 2
	if(cs) pat = pat + 1
	goto loop_3
	nop
loop_4:

	pat = key_pat
	adr = max
loop_5:	adr - block
	if(ls) goto loop_6
	r1 = *adr
	nop
	r1 = r1 ^ fill_pat
	if(ne) goto error
	nop
	*adr-- = pat
	pat = pat * 2
	if(cs) pat = pat + 1
	goto loop_5
	nop
loop_6:

	pat = key_pat
	adr = max
loop_7:	adr - block
	if(ls) goto loop_8
	r1 = *adr
	nop
	r1 = r1 ^ pat
	if(ne) goto error
	nop
	*adr-- = fill_pat
	pat = pat * 2
	if(cs) pat = pat + 1
	goto loop_7
	nop
loop_8:

	pat = key_pat
	pat = pat * 2
	if(cs) pat = pat + 1
	key_pat = pat

	key_pat - INITIAL_PAT
	if(eq) goto _1
	key_pat - ~INITIAL_PAT
	if(ne) goto loop
	nop
_1:
	fill_pat - 0xFFFF
	if(ne) goto _2
	nop
		fill_pat = 0x0000
		goto fill
		nop
_2:
	key_pat - INITIAL_PAT
	if(ne) goto _3
		fill_pat = 0xFFFF
		key_pat = ~INITIAL_PAT
		goto fill
		nop
_3:
	r1 = 0
	adr = 0

//	Here when error is detected, or when finished
//
//	adr= address
//	r1 = actual ^ correct
//
error:
	pdre = adr
	pir = r1
done_1:	nop
	if(pif) goto done_1
	nop
	goto start
	nop

	page
//	FRAME interrupt
frame:
	REFRESH(r18e)
	MASK_REFRESH
	REFRESH(r18e)
	ireturn
end_prog:
