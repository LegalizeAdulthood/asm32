#	makefile for asm32 - DSP32C assembler
#
#	Environment variable TC must be set to the Turbo-C
#	include/library directory.

# Memory model = large
M=l

all:	asm32.exe cpp.exe

asm32.exe:	y_tab.c
	bcc -m$(M) -v -I$(TC) -L$(TC) -easm32 y_tab.c

y_tab.c:	asm32.y
	yacc asm32.y

cpp.exe:	cpp.c cppsubs.c cexpr.c
	bcc -m$(M) -I$(TC) -L$(TC) -DMSDOS cpp.c cppsubs.c cexpr.c
