//	tsynth.cpp - test DSP-32C Synthesizer Board
//
//	tsynth is a command-oriented tool to exercise the functions
//	in dsp32c.cpp.
//
//	USAGE:
//		tsynth [1]
//
//	The optional argument specifies which DSP board to use (default=1).
//
//	See help() for list of commands.
//
/*	Before compiling this program, you must assemble the following
	DSP32C programs with the "-d" option:
		memtest.s
		playsamp.s
		sample.s
	They will be included as data into this executable program.
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


#ifdef _Windows
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <alloc.h>
#include <conio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>

#include "dsp32c.h"

const int N_VALUES=32;
long Value[N_VALUES];
int N_values;
char Command;
char *Arg1;
int Radix=0;

int huge *Sample_buf = 0;
long N_sample_buf = 0L;

DspBoard Dsp(1);

void help();
void display_dsp();
void write_dsp();
void fill_dsp_mem(long addr, unsigned nbytes, unsigned value);
int memory_test(long addr, unsigned nbytes);
void parse(char *cmd);
void beep();

long memfree()
{
#ifdef _Windows
	return GlobalCompact(0L);
#else
	return farcoreleft();
#endif
}

int main(int argc, char *argv[])
{
	char cmd[128];
	long l;
	int i;

	if(argc > 1) {
		i = atoi(argv[1]);
		Dsp = DspBoard(i);
	}

	if(!Dsp.is_present()) {
		printf("DSP NOT AVAILABLE!\n");
		return 1;
	}

	// allocate Sample_buf
	N_sample_buf = (memfree() - 10*1024L) / sizeof(int);
	if(N_sample_buf < 0L)
		N_sample_buf = 0L;
	if(N_sample_buf > 500000L)
		N_sample_buf = 500000L;
	if(N_sample_buf)
		Sample_buf = (int huge *)farmalloc(N_sample_buf*sizeof(int));
	if(Sample_buf) {
		int sinewave[32];
		for(i=0; i<32; ++i)
			sinewave[i] = 32767.0 * sin(2.0*M_PI*i/32.0);
		for(l=0; l<N_sample_buf; ++l)
			Sample_buf[l] = sinewave[l&31];
		printf("Sample buffer is %ld words, filled with 1kHz sinewave\n",
								N_sample_buf);
	}

////	Dsp.reset();
	help();

	while(printf("CMD: "), fgets(cmd,sizeof(cmd),stdin)) {
		parse(cmd);
		switch(Command) {
		case 'a':	// display or set PAR
			if(N_values < 1)
				printf("PAR=%06lX\n",Dsp.par());
			else
				Dsp.par(Value[1]);
			break;
		case 'c':
			printf("PCR=%04X\n",Dsp.pcr());
			break;
		case 'D':     	// set DMA bits
			if(N_values < 1)
				beep();
			else
				Dsp.set_dma(Value[1]);
			break;
		case 'd':	// display or set PDR
			if(N_values < 1) {
				if(Radix == 10)
					printf("PDR=%8ld\n",Dsp.pdr32());
				else
					printf("PDR=%08lX\n",Dsp.pdr32());
			} else {
				Dsp.pdr32(Value[1]);
			}
			break;
		case 'F':	// fill memory
			fill_dsp_mem(Value[1],Value[2],Value[3]);
			break;
		case 'f':	// set display format (Radix)
			if(N_values < 1)
				beep();
			else
				Radix = Value[1];
			break;
		case 'G':	// start DSP executing
			Dsp.run();
			break;
		case 'h':	// type help message
			help();
			break;
		case 'i':	// display or set PIR
			if(N_values < 1) {
				if(Radix == 10)
					printf("PIR=%6d\n",Dsp.pir());
				else
					printf("PIR=%04X\n",Dsp.pir());
			} else {
				Dsp.pir(Value[1]);
			}
			break;
		case 'L':	// load DSP memory from file, execute it
			Dsp.reset();
			if(Dsp.load(Arg1))
				printf("\aCannot load file '%s' - Execution Aborted\n",
								Arg1);
			else
				Dsp.run();
			break;
		case 'l':	// load DSP memory from file
			Dsp.reset();
			if(Dsp.load(Arg1))
				printf("\aCannot load file '%s'\n",Arg1);
			break;
		case 'M':	// Continuous Memory test
			i = 0;
			for(l=0; ; ++l) {
				if(kbhit())
					break;
				if(memory_test(Value[1],Value[2]))
					++i;
				printf("Test%6ld  %5d errors\n",l,i);
			}
			break;
		case 'm':	// Memory test
			memory_test(Value[1],Value[2]);
			break;
		case 'P':	// play sample
			if(Sample_buf)
				playsample(Dsp,Sample_buf,N_sample_buf);
			break;
		case 'Q':	// Quit
			goto quit;
		case 'R':	// Reset the DSP
			Dsp.reset();
			break;
		case 'r':	// read & display DSP memory
			display_dsp();
			break;
		case 'S':	// acquire a Sample
			if(Sample_buf)
				sample(Dsp,Sample_buf,N_sample_buf);
			break;
		case 's':	// status display
			long par=Dsp.par();
			i = Dsp.pcr();
			if(Radix == 10)
				printf("PCR=%04X ESR=%02X EMR=%04X"
					" PIR=%6d PAR=%06lX PDR=%8d\n",
					i,Dsp.esr(),Dsp.emr(),
					Dsp.pir(),par,Dsp.pdr32());
			else
				printf("PCR=%04X ESR=%02X EMR=%04X"
					" PIR=%04X PAR=%06lX PDR=%08lX\n",
					i,Dsp.esr(),Dsp.emr(),
					Dsp.pir(),par,Dsp.pdr32());
			break;
		case 'W':
			if(Sample_buf) {
				i = open("tsynth.adc",
					O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0666);
				if(i >= 0) {
					long l;
					for(l=0; (l+128)<N_sample_buf; l+=128) {
						char buf[256];
						_fmemcpy(buf,&Sample_buf[l],256);
						write(i,buf,256);
					}
					close(i);
				}
			}
			break;
		case 'w':	// write DSP memory
			write_dsp();
			break;
		default:
			beep();
			help();
			break;
		}
	}
quit:
	return 0;
}

void help()
{
	printf("tsynth - test the DSP32C synthesizer hardware\n"
		"Commands:\n"
		"\ta [val]\t\tDisplay [Set] PAR.\n"
		"\tc\t\tDisplay PCR.\n"
		"\tD v\t\tSet DMA bits.  8=enable, 16=incr, 256=32-bits\n"
		"\td [val]\t\tDisplay [Set] PDR.\n"
		"\tF addr nb val\tFill memory.\n"
		"\tf radix\t\tSet display format to radix.\r"
		"\tG\t\tStart the DSP executing at addr 0.\n"
		"\th\t\tHelp.\n"
		"\ti [val]\t\tDisplay [Set] PIR.\n"
		"\tL filename\tLoad and execute filename at addr 0.\n"
		"\tl filename\tLoad filename into DSP memory at addr 0.\n"
		"\tm addr,nbytes\tTest memory at addr, block of 256*nbytes.\n"
		"\tM addr,nbytes\tContinuously test memory at addr, block of 256*nbytes.\n"
		"\tQ\t\tQuit.\n"
		"\tP\t\tPlay a sample.\n"
		"\tR\t\tReset the DSP32C, and hold it in reset.\n"
		"\tr a n\t\tRead & display n/2 words from DSP memory at address a.\n"
		"\tS\t\tAcquire a sample.\n"
		"\ts\t\tDisplay DSP32C status.\n"
		"\tW\t\tWrite sample to file TSYNTH.ADC.\n"
		"\tw a v1 v2 ...\tWrite v1,v2,... words to DSP memory at addr a.\n"
	);
}

void display_dsp()
{
	const int INTS_PER_LINE = 8;
	long addr=0L, n=INTS_PER_LINE;
	int data[INTS_PER_LINE];
	char *fmt;

	if(Radix == 10)
		fmt = "%06lX: %6d %6d %6d %6d %6d %6d %6d %6d\n";
	else
		fmt = "%06lX: %04X %04X %04X %04X %04X %04X %04X %04X\n";

	if(N_values >= 1)
		addr = Value[1];
	if(N_values >= 2)
		n = Value[2];

	while(n > 0L) {
		Dsp.read(addr,(char *)data,INTS_PER_LINE*2);
		printf(fmt,
		       addr,data[0],data[1],data[2],data[3],data[4],data[5],
		       data[6],data[7]);
		n -= INTS_PER_LINE;
		addr += INTS_PER_LINE*2;
		if(kbhit())
			break;
	}
}

void write_dsp()
{
	long addr;
	int n, data;

	addr = Value[1];
	for(int i=2; i<=N_values; ++i) {
		data = Value[i];
		Dsp.write(addr,(char *)&data,2);
		addr += 2L;
	}
}

void fill_dsp_mem(long addr, unsigned nbytes, unsigned value)
{
	for(int i=0; i<nbytes; i+=2) {
		Dsp.write(addr,(char *)&value,2);
		addr += 2;
	}
}

char Memtest[] = {
#include "memtest.dat"
};
int memory_test(long addr, unsigned nbytes)
{
	int i;

	Dsp.reset();
	Dsp.write(0L,Memtest,sizeof(Memtest));
	Dsp.pir(0);	// set PIF
	Dsp.run();	// program will clear PIF when ready

	for(i=0; i<10000; ++i) {
		if(!Dsp.is_pir_full())
			break;
	}
	if(Dsp.is_pir_full()) {
		printf("Memory-Test: DSP not responding\n");
		return 1;
	}

	Dsp.pdr32(addr);
	Dsp.pir(nbytes);
	for(i=0; i<10000; ++i) {
		if(!Dsp.is_pir_full())
			break;
	}

	while(!Dsp.is_pir_full()) {
		if(kbhit())
			break;
	}

	addr = Dsp.pdr32() & 0x00FFFFFFL;
	nbytes = Dsp.pir();
	if(nbytes == 0) {
		printf("Memory-Test: Success\n"
			"   (Program keeps DRAM refreshed; test refresh with w and r cmds).\n");
		return 0;
	} else {
		printf("Memory-Test: FAILURE AT %06lX:%04X\n",addr,nbytes);
	}

	return 1;	// failure
}

void parse(char *cmd)
{
	Command = *cmd++;
	Arg1 = 0;

	char *p = strchr(cmd,'\n');
	if(p)
		*p = '\0';

	N_values = 0;
	for(;;) {
		while(*cmd && isspace(*cmd))
			++cmd;
		if(N_values == 0)
			Arg1 = cmd;
		if(N_values >= N_VALUES-1 || *cmd == '\0' ||
					(!isdigit(*cmd) && *cmd != '-'))
			break;
		++N_values;
		Value[N_values] = strtol(cmd,&cmd,Radix);
	}
}

void beep()
{
	printf("\a");
}
