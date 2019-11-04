//	sample.s - sample ADC-1 and transfer to PC (via PIR)
//
//	Copies input to Channel-0 DAC.
//	writing to PDR will silence the DAC, and zero the input
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


#include "dsp32c.i"	// DSP32C board definitions

	/* Init the DSP */
	org LOW_RAM
	DSP_INIT
	SIO_DMA(i_buf,o_buf,1)

	r1 = pdr

	r10e = buffer	// put pointer
	r11e = buffer	// get pointer
	r12e = buffer	// start-of-buffer pointer
	FRAME_INT(frame)

	DAC_ENABLE

loop:	nop
	if(pif) goto loop
	REFRESH(r1e)

	r10e - r11e
	if(eq) goto loop
	REFRESH(r1e)

	pir = *r11++
	r11e - buffer_end
	if(ge) r11e = r12e

	goto loop
	REFRESH(r1e)

// FRAME interrupt routine
frame:
	SIO_DMA_MASK
	MASK_REFRESH

	r1 = 0
	if(pdf) goto silence
	nop

	r1 = *i_buf
silence:
	nop
	nop
	nop
	*r10++ = r1
	r10e - buffer_end
	if(ge) r10e = r12e

	*o_buf = r1

	ireturn


// SIO I/O buffers for DMA
	align	8
i_buf:	4*word	0
o_buf:	4*word	0

// transfer buffer to PC
buffer:
buffer_end equ	MED_RAM_END

	end
