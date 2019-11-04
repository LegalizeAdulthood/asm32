/*	cpp.c - C Preprocessor	*/
/*
	USAGE:
		cpp [-C] [-oout] file(s)
		
	Expands file(s) (- = stdin) and writes to stdout.
	If -oout is present, output is written to file out.
	If -C is present, comments are preserved.
	Puts "#line 123 file" lines out as appropriate.
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

#include "port.h"

#undef MX_LINE
#define MX_LINE 256

extern int Keep_comments;

char line[MX_LINE+1];
char rawline[MX_LINE+1];
char errbuf[60];
char *filename;
int lineno;
long value;
int outlineno;
char *outfile = "";
FILE *out;

main(argc,argv)
int argc;
char *argv[];
{
	int i,j;
	char *p;
	
	outlineno = 0;
	out = stdout;

	while(argc > 2 && argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'o':
			if(argv[1][2] != '\0') {
				p = argv[1]+2;
			} else {
				p = argv[2];
				--argc, ++argv;
			}
			out = fopen(p,"w");
			if(!out) {
			    fprintf(stderr,"cpp: Cannot write '%s'\n",p);
			    exit(1);
			}
			break;
		case 'C':
			Keep_comments = 1;
			break;
		}
		--argc, ++argv;
	}
	
	for(i=1; i<argc; ++i) {
		ppinit(argv[i]);
		while(!ppgetline(line,MX_LINE)) {
		    if(lineno!=outlineno || strcmp(outfile,filename)!=0)
			prtlineno();
		    fprintf(out,"%s",line);
		    for(j=0; line[j]; ++j) {
			if(line[j] == '\n')
				++outlineno;
		    }
		}
	}
	
	fclose(out);
}

prtlineno()
{
	fprintf(out,"#line %d %s\n",lineno,filename);
	outfile = filename;
	outlineno = lineno;
}

error(string,exit_code)
char *string;
int exit_code;
{

	fprintf(stderr,"cpp: '%s' line %d: %s\n",filename,lineno,string);
	if(exit_code > 0) {
		fclose(stdout);
		exit(exit_code);
	}
}

listit(inlevel,inskip)
int inlevel,inskip;
{
}

isident(ch)
int ch;
{
	return(isalnum(ch) || ch == '_');
}

sym_val(ptr,pptr)
char *ptr,**pptr;
{
	return(-1);
}
