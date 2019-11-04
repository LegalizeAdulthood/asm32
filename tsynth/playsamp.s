//	playsamp.s - play samples from PC (via PIR) on DAC-0
//
//	writing to PDR will silence the DAC
/***************

THIS SOFTWARE IS RELEASED "AS IS", WITH NO WARRANTY EXPRESSED OR IMPLIED.
This software is copyright 1984, 1991, 1992, 1993, 1994 by Tom Roberts.
License is granted for unlimited distribution, as long as the following
conditions are met:
  A. This notice is preserved intact.
  B. No charge is made for the software (other than a reasonable
     distribution/duplication fee).

***************/


#include "dsp32c.i"	// DSP32C board definitions

	/* Init the DSP */
	org LOW_RAM
	DSP_INIT
	SIO_DMA(i_buf,o_buf,1)

	// flush any residual data
	r1 = pir
	r1 = pdr

	r10e = buffer	// get pointer
	r11e = buffer	// put pointer
	r12e = buffer	// start-of-buffer pointer

	// collect the first 32 samples (output is 0)
	r2 = 32-2
fill:
	if(syc) goto fill0
	REFRESH(r1e)
	SIO_DMA_MASK
	MASK_REFRESH
fill0:	if(pie) goto fill
	REFRESH(r1e)
	*r11++ = pir
	if(r2-- >= 0) goto fill
	REFRESH(r1e)

	// enable interrupts
	FRAME_INT(frame)

	DAC_ENABLE

loop:	nop
	if(pie) goto loop
	REFRESH(r1e)

	*r11++ = pir
	r11e - buffer_end
	if(ge) r11e = r12e

full:	r10e - r11e
	if(eq) goto full
	REFRESH(r1e)

	goto loop
	REFRESH(r1e)

// FRAME interrupt routine
frame:
	SIO_DMA_MASK
	MASK_REFRESH

	if(pdf) goto silence
	nop

	r1 = *r10++
	r10e - buffer_end
	if(ge) r10e = r12e

	*o_buf = r1
	ireturn
silence:
	r1 = 0
	*o_buf = r1
	ireturn

// SIO I/O buffers for DMA
	align	8
i_buf:	4*word	0
o_buf:	4*word	0

// transfer buffer from PC
buffer:
buffer_end equ MED_RAM_END

	end
