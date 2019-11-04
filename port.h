/*	port.h - portability header-file	*/
/*
	This file contains definitions likely to depend upon the hardware
	and/or the operating system.


	Compatibility considerations:

	1) The following are not used:
		#line
		macros with arguments [some macros are written name(arg)
			so they can be implemented as functions, if needed -
			if arguments are implemented, these macros MUST be
			defined with their arguments]
		typedef
		bit fields
		enum-s
		(void) [functions with no return-value have no type given,
			those with return-values ALWAYS have a type specified]
	2) Arguments to functions must be rigorously correct. In particular,
	   a zero passed to an argument of type (type *) MUST always be
	   properly cast, so machines on which pointers are of different
	   sizes from integers will work.
	3) All arithmetic types except char, int, and long are #defined
	   appropriately in this header-file.
	4) (byte *) and (char *) must be cast when used together; same for
	   (word *) and (int *).
	5) Only the following routines from <stdio> are used:
		putc(char, FILE *)
		getc(FILE *)
		printf(char *, ...)
		sprintf(char *, char *, ...)
		fprintf(FILE *, char *, ...)
		fopen(char *, char *)	[name, type]
		fclose(FILE *)
	6) Global symbols have only 6 significant characters, upper/lower
	   case is not significant. Local symbols have 7 chars significant,
	   case is significant.



	Basic arithmetic types:

		char	ONLY used for character strings; should be the
			native char of the C compiler.
		byte	at least 8 bits, UNSIGNED. Used for char + flag-bit
			(0200); also used for (up to 8) individual bits.
		int	at least -32768 to +32767; should be the native
			int or short of the C compiler.
		uint	at least 0 to 65536; should be the native unsigned int
			or short.
		word	at least 16 bits, UNSIGNED.
		long	at least 32 bits, SIGNED.

	byte and word should normally be the smallest possible, because they
	are usually used in data structures and file-formats.



	#ifdef-names:

		Operating-Systems (_OS):
		CPM	indicates CP/M operating system, using the C/80
			compiler from The Software Toolworks and M80
			from Microsoft.
		unix	the UNIX(TM) operating system, any "normal"
			hardware.
		MSDOS	MS-DOS ver 2.1 or greater
		
		CPUs (_CPU):
		NORMCPU	indicates a normal, byte-oriented CPU.

	_OS and _CPU are used to indicate that the OS and/or CPU
	have been defined. If either _OS or _CPU is undefined,
	CPM and NORMCPU are used.

	The NORMCPU hardware is assumed to be a byte-addresses CPU with
	2-byte shorts, 2- or 4-byte ints, and 4-byte longs.
*/

/*
	Here are the things that MUST be defined under _CPU:

		byte, word, uint (as described above)
		BEST (static, register, auto, or <blank>) - best storage-
			allocation type.

	Here are the things that MUST be defined under _OS:

		MX_FNAME	max # chars in a file-name
		MX_LINE		max # chars in an input-line
		MX_IDENT	max # chars in identifier

	ARG flags for cppsubs.c: must fit in a char (not byte), and
	be different from any normal char.

		ISARG(ch)	returns != 0 if ch (char) is an ARGFLAG.
		ARGFLAG(i)	returns the char ARGFLAG for arg # i (int).
		ARGNO(ch)	returns the arg # (int) for which ch is the
				ARGFLAG (ISARG(ch) is known to be true).

*/
/**************

THIS SOFTWARE IS RELEASED "AS IS", WITH NO WARRANTY EXPRESSED OR IMPLIED.
This software is copyright 1984, 1991, 1992, 1993, 1994 by Tom Roberts.
License is granted for unlimited distribution, as long as the following
conditions are met:
  A. This notice is preserved intact.
  B. No charge is made for the software (other than a reasonable
     distribution/duplication fee).

***************/

/*
	Put Special cases Here.
*/

#ifdef unix
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <alloc.h>
#define MX_FNAME 80
#define MX_LINE 256
#define MX_IDENT 8
#define ISARG(CH) ((CH) & 0200)
#define ARGFLAG(I) ((I) | 0200)
#define ARGNO(CH) ((CH) & 0177)
#define _OS
#endif /* unix */

#ifdef MSDOS
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <alloc.h>
#define MX_FNAME 80
#define MX_LINE 256
#define MX_IDENT 8
#define ISARG(CH) ((CH) & 0200)
#define ARGFLAG(I) ((I) | 0200)
#define ARGNO(CH) ((CH) & 0177)
#define _OS
#endif /* MSDOS */

#ifndef _CPU		/* default to NORMCPU if no _CPU so far */
#define NORMCPU
#endif
#ifndef _OS		/* default to CPM if no _OS so far */
#define CPM
#endif
#undef _OS
#undef _CPU

#ifdef NORMCPU
#define byte unsigned char
#define uint unsigned int
#define word unsigned short
#define BEST register
#endif

#ifdef CPM
#include "A:CLIB.H"		/* C-library definitions */
#undef byte			/* c80 has no unsigned char */
#define byte char
#undef word			/* c80 has no unsigned short */
#define word unsigned int
#define ISARG 0200 &		/* ISARG(ch) indicates an arg-flag char */
#define ARGNO 0177 &		/* ARGNO(ch) returns the arg # indicated by
				   ch (for which ISARG(ch) is true) */
#define ARGFLAG 0200 |		/* ARGFLAG(i) returns the arg-flag char for
				   arg # i */
#define MX_FNAME 16		/* max # chars in file-name */
#define MX_LINE 80		/* max # chars in input-line */
#define MX_IDENT 8		/* max # chars in identifier */
#undef BEST
#define BEST static		/* best storage allocation */
#endif	/* CPM */

