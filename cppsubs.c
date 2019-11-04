/*	CPPSUBS.C - "C" preprocessor 	*/
/*
	cppsubs - subroutines for the C pre-processor.

	Copyright 3/19/84 by Tom Roberts. All rights reserved.


	The C pre-processor implemented here has a few changes from the
	standard UNIX(TM) pre-processor:

	1) #line is not recognized.
	2) defined(symbol) is not recognized
	3) __FILE__ and __LINE__ are not recognized.
	4) #if/#else/#endif (all flavors) MAY be used inside a #define:
		#define m(a)	\
		#ifdef a	\
			a is defined\
		#else		\
			a is not defined\
		#endif
	   all of the #'s MUST follow an escaped newline.
	5) there is a new statement:
		#set name expression
	   the expression is expanded (for #define-d symbols),
	   name is #undef-ed, and the expression
	   is evaluated (must contain only operators and constants);
	   name is then #define-d to be the result (in ASCII, decimal radix).
	   All C operators are permitted.
	6) newlines in the input are not rigorously preserved (e.g. within
	   comments).
	7) comments are normally crunched out of #define macro-bodies; an
	   exception is made for the null comment (slash-star-slash -
	   indicated by <sss> below).
	   This permits generating unique labels inside #define-s:
		#define l 0
		#define m	\
			x<sss>l:	operation();\
		#set l l+1
	   The null comment can be used for any concatenation desired.
	   (I cannot use <sss> here, because it would end this comment!)
	8) In expressions, the conditional operator (a ? b : c) MUST
	   be surrounded by parentheses when nested within another
	   conditional operator.
	   In expressions, brackets ([]) are equivalent to parentheses (()).
	   Comma (,) and semicolon (;) will terminate the expression.
	   (#if and #set are the only places expressions are interpreted).


	Calling sequences:

	ppinit(filename) MUST be the first routine called.
	use filename="-" for stdin.

	ppgetline(line,MX_LINE) implements a line-oriented version of "cpp";
	line-orientation mainly refers to the ability to list
	the raw input-lines (via listit()).

	ppclose() will close the input-file, and free all space used by
	#defines. need not be called.


	externs required:

		char line[MX_LINE];	expanded line put here  (argument)
		char rawline[MX_LINE];	raw line read here
		char errbuf[];		space to form error-message
					  (at least 50 chars long)
		char *filename;		pointer for current file-name
					  (is set for each #include-file)
		int lineno;		int for current line-number
					  (of current file)
		error(string,code);	error-message print - should
					  prepend prog-name, filename, and
					  lineno to string, and add '\n'.
					  should call listit() internally.
					  code > 0 means fatal error (error
					  will call exit(code)).
		void listit(level,skip);routine to list rawline[], level =
					   nesting-level of #include files.
					   skip = 1 if text is skipped.
					   it should guarantee not to list the
					   same line twice. rawline[] may
					   be clobbered by listit().
					   (called just before a line is read)

	These will be given suitable defaults, if not #defined:

		#define PP_AUXNL ';'	to translate ';' to '\n' (leave
					   undefined, if no such char)
		#define N_HASH 32	number of hash-table entries - MUST
					  be a power of 2. defaults to 32.


	externs supplied to control the pre-processor:

		int pp_symb		symbol-argument to cexpr(); set
					non-zero to allow all symbols known
					to cexpr in #if and #set. zero
					(default) means only constants (and
					names #define-d to constants) can
					be used.

		int Keep_comments	set non-zero to preserve comments.
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


extern char line[];
extern char rawline[];
extern char errbuf[];
extern char *filename;
extern int lineno;
extern long value;


#define MX_INCLUDE	4	/* max nesting of #include files */
#define MX_TOKEN        128      /* max identifier, string, etc. */
#define MX_ARG          10      /* max # args to macro */
#ifndef MX_PPBUF
#define MX_PPBUF         2000    /* size of expand-buffer */
#endif

#ifndef N_HASH		/* size of hash-table */
#define N_HASH 32	/* MUST be a power of 2! */
#endif

#ifndef PP_AUXNL		/* char to be converted to '\n' */
#define PP_AUXNL 0
#endif

int pp_symb = 0;		/* set non-zero to enable symbols in pp */
int Keep_comments = 0;		/* set non-zero to preserve comments */

static char ppbuf[MX_PPBUF+2];  /* expand buffer */
static char *pparg[MX_ARG];     /* pointers to argument-values (last char) */
static char *pp_ptr;            /* current pointer into ppbuf[] */
static char *ppmacro;           /* pointer to macro-body (last char) */
static char *pptemp;		/* temporary pointer */
static char ppname[MX_TOKEN];   /* current token */
static FILE * infile[MX_INCLUDE];	/* #include file-pointers */
static int inlevel = 0;		/* current #include-file level */
static int inskip = 0;		/* 1 = skipping text, 0 = not skipping */
static int incomment = 0;	/* 1 = within comment (Keep_comments!=0) */
static int no_list = 0;		/* 1 = no list on next raw read */
static char inname[MX_INCLUDE][MX_FNAME]; /* file-names */
static int inline[MX_INCLUDE];	/* line numbers */
static int mx_line;		/* max size of line[] */
static int def_level = 0;	/* level of #define-s */


static struct PP_SYMBOL {
        struct PP_SYMBOL *ppnext;
        char ppname[MX_IDENT];
        char *ppstart;
        char *ppmacro;
} *pptable[N_HASH], *ppentry;


ppinit(name)				/* initialize ppgetline() */
char *name;
{
	extern char *strncpy();

	inlevel = 0;
	inskip = 0;
	if(strcmp(name,"-") == 0) {
		infile[0] = stdin;
	} else {
		infile[0] = fopen(name,"r");
		if(infile[0] == 0) {
			sprintf(errbuf,"cannot open file '%.16s'",name);
			error(errbuf,1);
		}
	}
	strncpy(inname[0],name,16);
	filename = inname[0];
	inline[0] = 0;
	lineno = 0;
	line[0] = '\0';
	rawline[0] = '\0';

	pp_ptr = &ppbuf[MX_PPBUF];
}

ppclose()
{
	register struct PP_SYMBOL *ptr, *ptr1;
	register int i;

	fclose(infile[0]);

	for(i=0; i<N_HASH; ++i) {
		for(ptr=pptable[i]; ptr; ptr=ptr1) {
			ptr1 = ptr->ppnext;
			if(ptr->ppstart)
				free(ptr->ppstart);
			free(ptr);
		}
		pptable[i] = (char *) 0;
	}
}


ppgetline(line,nline)		/* get expanded line into line[] */
char *line;			/* (recursive, through ppexpr()) */
int nline;
{
        register char *pp_put, *ptr, ch;
        register int i;

        pp_put = line;
	mx_line = nline - 2;	/* space for '\n' '\0' */

        for(;;) {
                if(pp_ptr >= &ppbuf[MX_PPBUF])
                        if(fillppbuf())
                                return(EOF);
                if(*pp_ptr == '#') {
                        if(pp_stmt())
				return(EOF);
                        continue;
                }

                while(pp_ptr < &ppbuf[MX_PPBUF]) {
                        if( (pp_put-line) >= mx_line) {
                                *pp_put++ = '\n';
				*pp_put = '\0';
                                return(0);
                        }
                        i = pptoken();
                        switch(i) {
#if PP_AUXNL
                        case PP_AUXNL:
#endif
                        case '\n':
                                *pp_put++ = '\n';
				*pp_put-- = '\0';
                                return(0);
			case 'c':	/* comment */
                                for(ptr=ppname; *ptr; ++ptr) {
					if( (pp_put-line) >= mx_line) {
						*pp_put++ = '\n';
						*pp_put = '\0';
						return(0);
					}   
					*pp_put++ = *ptr;
					if(*ptr == '\n') {
						*pp_put++ = '\0';
						return(0);
					}
				}
				break;
                        case 'a':       /* name */
                                if(ppsymb() && !incomment) {
                                        pparglist(1);
                                        while (ch = *ppmacro--) {
						if(pp_ptr < ppbuf)
							ppbufovfl();
						if(ISARG(ch)) {
                                                        ptr = pparg[ARGNO(ch)];
                                                        while(*ptr) {
							    if(pp_ptr < ppbuf)
								ppbufovfl();
                                                            *--pp_ptr = *ptr--;
							}
                                                } else {
                                                        *--pp_ptr = ch;
                                                }
                                        }
                                        break;
                                }
                                /* flow through */
                        default:
                                for(ptr=ppname; *ptr; ++ptr)
                                        *pp_put++ = *ptr;
                                break;
                        }
                }
        }
}

fillppbuf()     /* read into rawline[], copy to end of ppbuf[] */
{
        register  int n;
        register  char *ptr;
        extern char *fgets();

        for(;;) {
		if(!no_list)
	                listit(inlevel,inskip);
		else
			no_list = 0;

                while(!fgets(rawline,mx_line,infile[inlevel])) {
			fclose(infile[inlevel]);
			if(--inlevel < 0)
				return(EOF);
			filename = inname[inlevel];
			lineno = inline[inlevel];
		}
		/* increment abs(lineno) */
		lineno = ((lineno<0) ? -lineno : lineno) + 1;
        
                n = strlen(rawline);
                if(n) {
                        ptr = rawline + n;      /* this is the '\0' */
			if(*(ptr-2) == '\r' && *(ptr-1) == '\n') {
				*--ptr = '\0';
				*(ptr-1) = '\n';
				--n;
			}
                        pp_ptr = &ppbuf[MX_PPBUF];
			++n;
                        while(n--)
				*pp_ptr-- = *ptr--;
			++pp_ptr;
                        break;
                }
        }
        return(0);
}

int
pp_stmt()
{
	extern char *strncpy();
        register int i, level;
        register struct PP_SYMBOL *sym, **psym;
        register char *ptr, *ptr1;
	register int j, state;
	auto char t_name[MX_IDENT+1], t_line[80];
        static char *keyword[] = {"include","define","undef",
                                   "ifdef","ifndef","if","else","endif",
                                   "set","pushdef","pushset","popdef",NULL};
	FILE *f;

        ++pp_ptr;        /* (eat the '#') */

        while(isspace(*pp_ptr) && *pp_ptr != '\n')
                ++pp_ptr;
        if(pptoken() != 'a')
                goto err;
        while(isspace(*pp_ptr) && *pp_ptr != '\n')
                ++pp_ptr;

        for(i=0; keyword[i]; ++i) {
                if(strcmp(ppname,keyword[i]) == 0)
                        break;
        }
        switch(i) {

        case 0: /* include */
		if(inlevel >= MX_INCLUDE) {
			error("#include-files nested too deeply - ignored",0);
			break;
		}
		if(*pp_ptr == '"')
			i = '"';
		else if(*pp_ptr == '<')
			i = '>';
		else
			goto err;
		ptr = ++pp_ptr;
		while(*pp_ptr) {
			if(*pp_ptr++ == i)
				break;
		}
		*(pp_ptr-1) = '\0';
		f = fopen(ptr,"r");
		if(!f) {
			sprintf(errbuf,"cannot open #include-file '%.16s'",ptr);
			error(errbuf,0);
			break;
		}
		listit(inlevel,inskip);
		no_list = 1;
		inline[inlevel] = lineno;
		infile[++inlevel] = f;
		strncpy(inname[inlevel],ptr,16);
		filename = inname[inlevel];
		lineno = 0;
		break;

	case 9:	/* pushdef */
        case 1: /* define */
                if(pptoken() != 'a')
                        goto err;
                if(i == 1 && ppsymb()) {
                    sym = ppentry;
                    free(sym->ppstart);
		    sprintf(errbuf,"WARNING: re-definition of macro '%.8s'",
			ppname);
		    error(errbuf,0);
                } else {
                        sym =(struct PP_SYMBOL *)malloc(sizeof(struct PP_SYMBOL));
                        if(sym == 0)
				ppmemovfl();
                        strncpy(sym->ppname,ppname,MX_IDENT);
                        i = hash(ppname);
                        sym->ppnext = pptable[i];
                        pptable[i] = sym;
                }
		++def_level;
		pparglist(0);
		*pptemp++ = '\0';	/* initialized in pparglist() */
		while(isspace(*pp_ptr) && *pp_ptr != '\n')
			++pp_ptr;
		ptr = pptemp;
		for(;;) {
next:			if(ptr >= pp_ptr)
				ppbufovfl();
			switch(pptoken()) {
			case 'a':
				for(i=0; i<MX_ARG; ++i) {
					if(strncmp(ppname,pparg[i],MX_IDENT)
							 == 0) {
						*ptr++ = (ARGFLAG(i));
						goto next;
					}
				}
				goto copy;
			case '\n':
#if PP_AUXNL
				if(*(pp_ptr-1) == PP_AUXNL)
					ppname[0] = PP_AUXNL;
				else
#endif
				     if(*(ptr-1) == '\\')
					--ptr;
				else
					goto done;
				/* flow through */
			default:
copy:				for(ptr1=ppname; *ptr1;)
					*ptr++ = *ptr1++;
			}
		}
done:		--def_level;
		*ptr++ = '\0';
                ptr = malloc(ptr - pptemp + 1);
                sym->ppstart = ptr;
                if(!ptr)
			ppmemovfl();
		*ptr++ = '\0';	/* (end string when scanned backwards) */
                for(ptr1=pptemp; *ptr++ = *ptr1++;)
                        ;
                sym->ppmacro = ptr - 2;
                return(0);

	case 11:/* popdef */
        case 2: /* undef */
                if(pptoken() != 'a')
                        goto err;
                psym = &pptable[hash(ppname)];
		while(*psym) {
                        if(strncmp((*psym)->ppname,ppname,MX_IDENT) == 0) {
                                free((*psym)->ppstart);
                                ptr = (char *)*psym;
                                *psym = (*psym)->ppnext;
				free(ptr);
				if(i == 11)
	                                break;
				else
					continue;
                        }
			psym = &((*psym)->ppnext);
                }
                break;

        case 3: /* ifdef */
		if(pptoken() != 'a')
			goto err;
		if(!ppsymb())
			goto skip;
		break;

        case 4: /* ifndef */
		if(pptoken() != 'a')
			goto err;
		if(ppsymb())
			goto skip;
		break;

        case 5: /* if */
		i = ppexpr();
		if(i == EOF)
			return(EOF);
		if(i != 0)
			goto err;
		if(value == 0l)
			goto skip;
		break;

        case 6: /* else */
skip:		listit(inlevel,inskip);
		no_list = 1;
		level = 1;
		inskip = 1;
		state = 0;
		/* state=0: looking for '\n'; state=1: found '\n';
		   state=2: found '\n#' or '\n# \t', looking for name */
		while(level > 0) {
			i = pptoken();
			if(i == EOF)
				return(EOF);
			if(state == 1 && i == '#') {
				state = 2;
				continue;
			} else if(state == 2 && (i == ' ' || i == '\t')) {
				continue;
			} else if(state == 2 && i == 'a') {
				if(strcmp(ppname,"endif") == 0)
					--level;
				else if(level == 1 && strcmp(ppname,"else")==0)
					--level;
				else if(ppname[0] == 'i' && ppname[1] == 'f')
					++level;
			}
			if(i == '\n')
				state = 1;
			else
				state = 0;
		}
		inskip = 0;
		break;

        case 7: /* endif */
		break;

	case 10:/* pushset */
	case 8:	/* set */
		if(pptoken() != 'a')
			goto err;
		strncpy(t_name,ppname,MX_IDENT);
		t_name[MX_IDENT] = '\0';
		j = ppexpr();
		if(j == EOF)
			return(EOF);
		if(j != 0)
			goto err;
		if(i == 8)
			sprintf(t_line,"#undef %s\n#define %s 0%lo",
				t_name,t_name,value);
		else
			sprintf(t_line,"#pushdef %s 0%lo",
				t_name,value);
		i = strlen(t_line);
		while(i--)
			*--pp_ptr = t_line[i];
		return(0);

        default:
                goto err;
        }

	goto ret;

err:	sprintf(errbuf,"illegal cpp-statement  token='%.16s'",ppname);
	error(errbuf,0);

ret:	while(pptoken() != '\n')
                ;
        return(0);
}

int
ppexpr()	/* expand rest of line, and evaluate expression */
{
	register int t_mxline, i;
	auto char t_line[MX_TOKEN+MX_TOKEN];

	t_mxline = mx_line;
	t_line[0] = '\0';
	++def_level;
	i = ppgetline(t_line,MX_TOKEN+MX_TOKEN);
	if(i == EOF)
		return(EOF);
	--pp_ptr;	/* put back '\n' */
	--def_level;
	mx_line = t_mxline;
	if(i || cexpr(t_line,pp_symb))
		return(1);
	return(0);
}

ppsymb()        /* check if ppname[] is a cpp-known symbol */
{
        register struct PP_SYMBOL *sym;

        for(sym=pptable[hash(ppname)]; sym!=0; sym=sym->ppnext) {
                if(strncmp(ppname,sym->ppname,MX_IDENT) == 0) {
                        ppmacro = sym->ppmacro;
                        ppentry = sym;
                        return(1);
                }
        }
        return(0);
}


hash(name)              /* simple hash (N_HASH is a power of 2) */
register char *name;
{
        BEST int h, n;

	n = MX_IDENT;
        h = 0;
        while(*name && n--)
                h += *name++;
        
        return(h & (N_HASH-1));
}

pptoken()       /* return the next token from *pp_ptr */
{
	BEST int ch;
	BEST char *ptr;

loop:	while(pp_ptr >= &ppbuf[MX_PPBUF])
		if(fillppbuf())
			return(EOF);

	ch = *pp_ptr++;
	ppname[0] = ch;
	ptr = &ppname[1];
	if(ch == '/' && (*pp_ptr == '*' || *pp_ptr == '/') && !incomment) {
		if(*pp_ptr == '*') {
			if(Keep_comments) {
			    incomment = 1;
			} else if(*(pp_ptr+1)!='/' || def_level==0) {
			    while( !(*pp_ptr++ == '*' && *pp_ptr == '/') ){
				if(pp_ptr >= &ppbuf[MX_PPBUF])
					if(fillppbuf())
						return(EOF);
			    }
			    ++pp_ptr;
			    goto loop;
			}
		} else {
			if(Keep_comments) {
			    incomment = -1;
			} else {
			    while(*pp_ptr++ != '\n') {
				if(pp_ptr >= &ppbuf[MX_PPBUF])
					if(fillppbuf())
						return EOF;
			    }
			    --pp_ptr;
			    goto loop;
			}
		}
	}
	if((incomment == 1 && ch == '*' && *pp_ptr == '/') ||
	   (incomment == -1 && ch == '\n'))
		incomment = 0;
		
	if(ch == '"') {
		while(ch = *pp_ptr++) {
			*ptr++ = ch;
			if(ch == '\\')
				*ptr++ = *pp_ptr++;
			else if(ch == '"')
				break;
			if(ptr >= &ppname[MX_TOKEN]) {
				error("string too long",0);
				break;
			}
			if(pp_ptr >= &ppbuf[MX_PPBUF])
				if(fillppbuf())
					return(EOF);
		}
	} else if(ch == '\'') {
		while(ch = *pp_ptr++) {
			*ptr++ = ch;
			if(ch == '\\')
				*ptr++ = *pp_ptr++;
			else if(ch == '\'')
				break;
			if(ptr >= &ppname[MX_TOKEN]) {
				error("string too long",0);
				break;
			}
			if(pp_ptr >= &ppbuf[MX_PPBUF])
				if(fillppbuf())
					return(EOF);
		}
	} else if(isident(ch)) {
		while(isident(*pp_ptr))
			*ptr++ = *pp_ptr++;
		ch = 'a';
	}

	*ptr = '\0';
	return(ch);
}

pparglist(type)     /* collect macro arg-list */
int type;		/* 0=point to start, 1=point to end of arg */
{
	register char *ptr, *ptr1, *begin;
	register int level, i;

	for(i=0; i<MX_ARG; ++i)
		pparg[i] = "";
	pptemp = ppbuf;

	if(*pp_ptr != '(')
		return(0);

	level = 1;
	++pp_ptr;
	ptr = ppbuf;
	*ptr++ = '\0';	/* end string when scanned backwards */

	for(i=0; i<MX_ARG; ++i) {
	    begin = ptr;
	    while (level > 0) {
		switch(pptoken()) {
		case '\n':
#if PP_AUXNL
			ppname[0] = PP_AUXNL;
#else
			ppname[0] = ' ';
#endif
			break;
		case '(': case '[': case '{':
			++level;
			break;
		case ')': case ']': case '}':
			if(--level <= 0)
				goto end_arg;
			break;
		case ',':
			if(level == 1)
				goto end_arg;
		}
		for(ptr1=ppname; *ptr1;)
			*ptr++ = *ptr1++ & 0177;
	    }
end_arg:    *ptr++ = '\0';
	    if(type)
		pparg[i] = ptr-2;
	    else
		pparg[i] = begin;
	}
	pptemp = ptr;
}

ppbufovfl()
{
	error("macro expand-buffer overflow",1);
}

ppmemovfl()
{
	error("MEMORY OVERFLOW in cpp",1);
}
