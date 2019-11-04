#	makefile for tsynth

# memory model = large
M=l

tsynth.exe:	tsynth.cpp dsp32c.h dsp32c.obj sample.obj sample.dat playsamp.dat memtest.dat
	bcc -m$(M) -v -I$(TC) -L$(TC) tsynth.cpp dsp32c.obj sample.obj
	
dsp32c.obj:	dsp32c.cpp dsp32c.h
	bcc -m$(M) -c -v -I$(TC) dsp32c.cpp

sample.obj:	sample.cpp dsp32c.h sample.dat playsamp.dat
	bcc -m$(M) -c -v -I$(TC) sample.cpp

sample.dat:	sample.s
	..\cpp -C sample.s >a32.tmp
	..\asm32 -d sample.s a32.tmp
	del a32.tmp

playsamp.dat:	playsamp.s
	..\cpp -C playsamp.s >a32.tmp
	..\asm32 -d playsamp.s a32.tmp
	del a32.tmp

memtest.dat:	memtest.s
	..\cpp -C memtest.s >a32.tmp
	..\asm32 -d memtest.s a32.tmp
	del a32.tmp
