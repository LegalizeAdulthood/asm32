//	sample.cpp - DSP record/play samples
/***************

THIS SOFTWARE IS RELEASED "AS IS", WITH NO WARRANTY EXPRESSED OR IMPLIED.
This software is copyright 1984, 1991, 1992, 1993, 1994 by Tom Roberts.
License is granted for unlimited distribution, as long as the following
conditions are met:
  A. This notice is preserved intact.
  B. No charge is made for the software (other than a reasonable
     distribution/duplication fee).

***************/


#include <conio.h>

#include "dsp32c.h"

#pragma argsused
static void delay(int ms)
{
	for(long l=0; l<10000L; ++l)
		;
}

/*	playsample() - download playsamp program to DSP, execute it, and
	send samples to the DSP
*/
static char Playsample_dsp[] = {
#include "playsamp.dat"
};
void playsample(DspBoard dsp, int huge *buf, long n_buf)
{
	int i,j;

	if(n_buf <= 128)
		return;

	dsp.reset();
	dsp.write(0L,Playsample_dsp,sizeof(Playsample_dsp));
	dsp.run();

	// output the first 128 samples without interrupts, to guarantee
	// that the PC is ahead of the DSP output
	disable();
	for(i=0; i<128; ++i) {
		j = 10000L;
		while(dsp.is_pir_full()) {
			if(--j <= 0L)
				goto done;
		}
		dsp.pir(*buf++);
		--n_buf;
	}
	enable();

	while(n_buf-- > 0) {
		j = 10000L;
		while(dsp.is_pir_full()) {
			if(--j <= 0L || kbhit())
				goto done;
		}
		dsp.pir(*buf++);
	}

	// output 4000 zeros to completely fill the buffer with 0
done:	for(i=0; i<4199; ++i) {
		j = 10000L;
		while(dsp.is_pir_full()) {
			if(--j <= 0L)
				goto error;
		}
		dsp.pir(0);
	}

error:
	dsp.pdr(0);	// writing to PDR silences the DAC (see playsamp.s)
	delay(2);
	dsp.reset();

	return;
}


/*	sample() - download sample program to DSP, execute it, and
	collect samples from the DSP
*/
static char Sample_dsp[] = {
#include "sample.dat"
};
void sample(DspBoard dsp, int huge *buf, long n_buf)
{
	int j;

	dsp.reset();
	dsp.write(0L,Sample_dsp,sizeof(Sample_dsp));
	dsp.run();

	do {
		j = 10000;
		while(!dsp.is_pir_full()) {
			if(--j <= 0 || kbhit())
				goto done;
		}
		*buf++ = dsp.pir();
	} while(--n_buf > 0L);
done:
	dsp.pdr(0);	// writing to PDR silences the DAC (see sample.s)
	delay(2);
	dsp.reset();

	while(n_buf-- > 0L)
		*buf++ = 0;

	return;
}
