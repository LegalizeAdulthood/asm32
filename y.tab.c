
# line 100 "asm32.y"
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif

{
	long value;
} YYSTYPE;
# define PLSPLS 257
# define MINUSMINUS 258
# define REG 259
# define REGR 260
# define REGE 261
# define REGHL 262
# define REGIO 263
# define ACCUM 264
# define PC 265
# define PIOP 266
# define PIR 267
# define PCSH 268
# define DAUC 269
# define IOC 270
# define IBUF 271
# define OBUF 272
# define PDR 273
# define PCW 274
# define SYMBOL 275
# define LABEL 276
# define CONST 277
# define FCONST 278
# define CONST1 279
# define CONST2 280
# define ORG 281
# define END 282
# define ALIGN 283
# define BYTE 284
# define PAGE 285
# define EQU 286
# define LIST 287
# define INT 288
# define INT24 289
# define FLOAT 290
# define DA_FUN 291
# define IF 292
# define GOTO 293
# define CALL 294
# define RETURN 295
# define IRETURN 296
# define DO 297
# define NOP 298
# define CA_COND 299
# define DA_COND 300
# define IO_COND 301
# define EQUEQU 302
# define NOTEQU 303
# define LE 304
# define GE 305
# define LSHIFT 306
# define RSHIFT 307
# define RROTATE 308
# define LROTATE 309

# line 143 "asm32.y"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mem.h>
#include <alloc.h>
#include <io.h>
#include <fcntl.h>
#include <dir.h>
#include <process.h>
/* the following headers are only needed for creating Title[] */
#include <time.h>
#include <dos.h>
#include <sys\stat.h>
	
#define REV 0x02000000L		/* bit-reversed flag */
#define EXT 0x80000000L		/* 24-bit extended flag */
#define LOW 0x40000000L		/* 8-bit low-byte flag */
#define HIGH 0x20000000L	/* 8-bit high-byte flag */

long Org = 0L;			/* current address */
int Page_length = 63;
int Page_width = 80;
int List_type = 3;		/* main file & generated code */

#define MAX_DATA 	4096	/* MAX # program bytes */

#define MAX_SYMBOL	17	/* Max # chars in a SYMBOL */

#define UNDEFINED	0x80000000L	/* undefined SYMBOL value */

typedef struct Symbol {
	struct Symbol *next;
	char name[MAX_SYMBOL+1];
	int type;
	long value;
} Symbol;

/* temp pointers for SYMBOL definition (pipelined) */
Symbol *Last_symbol = 0;
Symbol *Symbol_def = 0;

/* prototypes for C functions */
void endit(void);
void yyerror(char *s);
void errormsg(char *s);
void error_symbol(char *msg, Symbol *s);
void warning(char *s);
void warn_symbol(char *msg, Symbol *s);
void fatal(char *s);
int yylex();
long dspfloat(float f);
long dspnegfloat(long f);
int getinput();
void list_address(long addr);
void list_data(long data, int offset, int size);
void prepare_list();
void prepare_list_line(int lineno, char *input, char flag);
void list_print();
void list_page(int omit_list);
void end_stmt();
void define_data(long value, int size);
void define_array(int n, long value, int size);
int hash(char *name);
Symbol *symbol(char *name);
void define_label(long value);
void symbol_init();
void undefine_symbols(int type);
long mem_ref(long reg, long incr);
long x_ref(long mem_ref);
long y_ref(long mem_ref);
long z_ref(long mem_ref);
void da_1(long fmt, long m, long f, long s, long da_dest, long x, long y, long z);
void da_5(long g, long da_dest, long y);
long goto_dest(long reg, long n);
void ca_0(long cond, long goto_dest);
void ca_3a(long m, long goto_dest);
void ca_4(long m, long h, long n);
void ca_5(long d, long h, long n);
long ca_6a(long f, long d, long s1, long s2);
void ca_6c(long f, long d, long n, long s);
void ca_7a(long t, long h, long n);
void ca_7b(long t, long h, long mem_ref);
void ca_7d(long t, long r, long mem_ref);
void ca_8(int type, long reg, long expr);

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#else
#include <malloc.h>
#include <memory.h>
#endif

#include <values.h>

#ifdef __cplusplus

#ifndef yyerror
	void yyerror(const char *);
#endif

#ifndef yylex
#ifdef __EXTERN_C__
	extern "C" { int yylex(void); }
#else
	int yylex(void);
#endif
#endif
	int yyparse(void);

#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 560 "asm32.y"


/* stuff for listing */
int Line_on_page = 9999;
int Page_number = 0;
char Title[82] = "ASM32 - DSP32C Assembler     ";
/* List[] contains line to be listed */
char List[257] = "";
#define ADDR_COL	7	/* Col # for address in List[] */
#define DATA_COL	14	/* Col # for 4 data bytes in List[] */
#define SOURCE_COL	26	/* Col # for source line in List[] */
FILE *List_file = 0;            /* List output file */
FILE *List_in = 0;		/* original input file for listing */
char Input2[257] = "";		/* second ary input buffer for listing */
int List_line = 0;		/* line # on secondary input file */
int Block_comment = 0;		/* flag for block comments */

/* Input[] contains input line, Index points to next char */
char Input[257] = "";
int Index = 0;
char Filename[65] = "";
char Main_filename[65] = "";
FILE *Input_file = 0;
int Line_number = 0;

/* Data[] for program */
unsigned char *Data = 0;

/* Misc control variables */
int Pass = 1;
int Dsp_handle = -1;
int Errors = 0;
int Warnings = 0;
int D_flag = 0;

void main(int argc, char *argv[])
{
	struct stat stat_struct;
	int i;
	char file[MAXFILE],ext[MAXEXT];
	char buf[80];
	unsigned char c;

	if(argc >= 2 && strcmp(argv[1],"-d") == 0) {
		D_flag = 1;
		--argc, ++argv;
	}

	if(argc < 2 || argc > 3 || strlen(argv[1]) >= sizeof(Filename)-1)
	    fatal("No Input File/Illegal Filename/Illegal # of Parameters");

	atexit(endit);

	/* split up the filename */
	strcpy(Filename,argv[1]);
	strcpy(Main_filename,Filename);
	fnsplit(Filename,(char *)0,(char *)0,file,ext);

	/* open input file */
	if(argc == 3) {
		List_in = fopen(Main_filename,"r");
		List_line = 0;
		strcpy(Filename,argv[2]);
	}
	Input_file = fopen(Filename,"r");
	if(!Input_file) {
	    fprintf(stderr,"asm32: Cannot Open '%s'\n",Filename);
	    exit(1);
	}

	/* get Dsp_handle filename and List_file filename */
	strcpy(buf,file);
	strcat(buf,".lst");
	List_file = fopen(buf,"w"); // ignore failure for List_file
	strcpy(buf,file);
	if(D_flag) {
		strcat(buf,".dat");
		Dsp_handle = open(buf,O_WRONLY|O_CREAT|O_TRUNC,0666);
	} else {
		strcat(buf,".dsp");
		Dsp_handle = open(buf,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0666);
	}
	if(Dsp_handle < 0) {
		fprintf(stderr,"asm32: Cannot Create/Write DSP-file '%s'\n",buf);
		exit(3);
	}

	/* get Title[] */
	if(stat(Filename,&stat_struct)) {
		strftime(buf,sizeof(buf),"%m/%d/%y %H:%M     ",
					localtime(&stat_struct.st_mtime));
		strcat(Title,buf);
	}
	strcat(Title,file);
	strcat(Title,ext);
	while(strlen(Title) < 72)
		strcat(Title," ");
	if(strlen(Title) == 72)
		strcat(Title,"Page");

	/* initialize the assembler */
	Errors = 0;
	Warnings = 0;
	symbol_init();
	Data = malloc(MAX_DATA);
	if(!Data)
		fatal("Out of Memory");
	memset(Data,0,MAX_DATA);

	/* pass 1 */
	Pass = 1;
	Line_number = 0;
	Org = 0L;
	Input[0] = '\0';
	Index = 0;
	yyparse();

	/* pass 2 */
	Pass = 2;
	Line_number = 0;
	Org = 0L;
	Input[0] = '\0';
	Index = 0;
	rewind(Input_file);
	undefine_symbols(SYMBOL);
	yyparse();

	if(Errors > 0 || Warnings > 0) {
		sprintf(buf,"%d Warnings, %d Errors",Warnings,Errors);
		fprintf(List_file,"asm32: %s\n",buf);
		if(Errors > 0)
			fatal(buf);
		fprintf(stderr,"asm32: %s\n",buf);
	}	

	if(Org > MAX_DATA)
		fatal("Program Too Large");

	/* output assembled program */
	if(D_flag) {
		for(i=0; i<Org; ++i) {
			c = Data[i];
			if(c >= 100) {
				buf[0] = c/100 + '0';
				c = c % 100;
			} else {
				buf[0] = ' ';
			}
			if(c >= 10) {
				buf[1] = c/10 + '0';
				c = c % 10;
			} else {
				buf[1] = (buf[0]==' ') ? ' ' : '0';
			}
			buf[2] = c + '0';
			buf[3] = ',';
			if(write(Dsp_handle,buf,4) != 4)
				fatal("Write Error on DSP file!");
			if((i&15) == 15)
				write(Dsp_handle,"\n",1);
		}
	} else {
		if(write(Dsp_handle,Data,(int)Org) != (int)Org)
			fatal("Write Error on DSP file!");
	}
	
	/* done */
	exit(0);
}

void endit(void)
{
	close(Dsp_handle);

	fputs("\f",List_file);
	fclose(List_file);
}

#pragma argsused
void yyerror(char *s)
{
}

void errormsg(char *s)
{
	if(Pass != 2)
		return;
	list_print();
	if(++Line_on_page > Page_length)
		list_page(0);
	fprintf(List_file,"asm32 Error: %s in '%s' Line %d\n",
						s,Filename,Line_number);
	fprintf(stderr,"asm32 Error: %s in '%s' Line %d\n",
						s,Filename,Line_number);
	if(++Errors >= 25)
		fatal("Too Many Errors");
}

void error_symbol(char *msg, Symbol *s)
{
	char buf[99];
	sprintf(buf,"%s '%s'",msg,s->name);
	errormsg(buf);
}

void warning(char *s)
{
	if(Pass != 2)
		return;
	list_print();
	if(++Line_on_page > Page_length)
		list_page(0);
	fprintf(List_file,"asm32 Warning: %s in '%s' Line %d\n",
						s,Filename,Line_number);
	fprintf(stderr,"asm32 Warning: %s in '%s' Line %d\n",
						s,Filename,Line_number);
	++Warnings;
}

void warn_symbol(char *msg, Symbol *s)
{
	char buf[99];
	sprintf(buf,"%s '%s'",msg,s->name);
	warning(buf);
}

void fatal(char *s)
{
	fprintf(List_file,"asm32 Fatal: %s in '%s' Line %d\n",
						s,Filename,Line_number);
	fprintf(stderr,"asm32 Fatal: %s in '%s' Line %d\n",
						s,Filename,Line_number);
	exit(1);
}

int yylex()
{
	int i,j, c,d, radix, type;
	Symbol *s;
	char name[40];
#define isalnum_(c) (isalnum(c) || c == '_')

	yylval.value = UNDEFINED;
again:
	c = getinput();
	if(c == EOF) {
		type = 0;
	} else if(isdigit(c)) {
		name[0] = c;
		j = 1;
		if(c == '0' && toupper(Input[Index]) == 'X') {
			name[j++] = Input[Index++];
			while(isxdigit(Input[Index]))
				name[j++] = Input[Index++];
		} else {
			while(isdigit(Input[Index]))
				name[j++] = Input[Index++];
		}
		if(Input[Index] == '.') {	/* floating-point constant */
			name[j++] = Input[Index++];
			while(isdigit(Input[Index]))
				name[j++] = Input[Index++];
			if(toupper(Input[Index]) == 'E') {
				name[j++] = Input[Index++];
				if(Input[Index] == '+' || Input[Index] == '-')
					name[j++] = Input[Index++];
				while(isdigit(Input[Index]))
					name[j++] = Input[Index++];
			}
			name[j] = '\0';
			yylval.value = dspfloat(atof(name));
			type = FCONST;
		} else {			/* integer constant */
			name[j] = '\0';
			yylval.value = strtol(name,0,0);
			if(yylval.value == 1L)
				type = CONST1;
			else if(yylval.value == 2L)
				type = CONST2;
			else
				type = CONST;
		}
	} else if(isalnum_(c)) {
		name[0] = c;
		i = 1;
		while(isalnum_(Input[Index]) && i<MAX_SYMBOL)
			name[i++] = Input[Index++];
		name[i] = '\0';
		s = symbol(name);
		yylval.value = s->value;
		type = s->type;
	} else switch(c) {
	case '\n':
		type = c;
		break;
	case '/':
		c = Input[Index];
		if(c == '/') {
			Input[Index] = '\0';
			type = '\n';
			break;
		} else if(c == '*') {
			do {
				while(getinput() != '*')
					;
			} while(getinput() != '/');
			goto again;
		}
		type = '/';
		break;
	case '+':
		if(Input[Index] == '+') {
			++Index;
			type = PLSPLS;
			break;
		}
		type = '+';
		break;
	case '-':
		if(Input[Index] == '-') {
			++Index;
			type = MINUSMINUS;
			break;
		}
		type = '-';
		break;
	case '=':
		if(Input[Index] == '=') {
			++Index;
			type = EQUEQU;
			break;
		}
		type = '=';
		break;
	case '!':
		if(Input[Index] == '=') {
			++Index;
			type = NOTEQU;
			break;
		}
		type =  '!';
		break;
	case '<':
		if(Input[Index] == '=') {
			++Index;
			type = LE;
			break;
		}
		if(Input[Index] == '<') {
			if(Input[Index+1] == '<') {
				Index += 2;
				type = LROTATE;
				break;
			}
			++Index;
			type = LSHIFT;
			break;
		}
		type = '<';
		break;
	case '>':
		if(Input[Index] == '=') {
			++Index;
			type = GE;
			break;
		}
		if(Input[Index] == '>') {
			if(Input[Index+1] == '>') {
				Index += 2;
				type = RROTATE;
				break;
			}
			++Index;
			type = RSHIFT;
			break;
		}
		type =  '>';
		break;
	case '$':
		yylval.value = Org;
		type = CONST;
		break;
	default:
		if(isspace(c))
			goto again;
		type = c;
	}
done:
	return type;
}

int getinput()
{
	int i;

	while(Input[Index] == '\0') {
		list_print();
		Index = 0;
		++Line_number;
		for(;;) {
			if(!fgets(Input,sizeof(Input),Input_file)) {
				Input[0] = '\0';
				return EOF;
			}
			if(strncmp(Input,"#line",5) == 0) {
				char *p;
				if(Pass == 2) {
					--Line_number;
					prepare_list();
					++Line_number;
				}
				i = strtol(Input+6,&p,10);
				while(*p != '\0' && isspace(*p))
					++p;
				if(Pass == 2 && strncmp(Filename,p,strlen(Filename)) != 0
					     && strcmp(Filename,Main_filename) == 0) {
					prepare_list();
					list_print(); // print the #include
				}
				Line_number = i;
				if(*p) {
					for(i=0; i<sizeof(Filename)-1; ++i) {
						if(*p == '\0' || isspace(*p))
							break;
						Filename[i] = *p++;
					}
					Filename[i] = '\0';
				}
			} else {
				break;
			}
		}
		if(Pass == 2)
			prepare_list();
	}

	return Input[Index++];
}

/*	dspfloat() converts an 8087 float to a DSP32C float,
	which is returned as a long value.	*/
long dspfloat(float f)
{
	int neg = 0;
	long m,e;
	union {long l; float f;} u;

	if(f < 0.0) {
		f = -f;
		neg = 1;
	}
	u.f = f;

	m = u.l & 0x007FFFFFL;
	e = (u.l >> 23) & 0x000000FFL;

	if(e != 0)
		e += 1;
	else
		m = 0L; // unnormalized underflows ==> 0.0
	if(e > 0xFF) {
		e = 0xFF;
		m = 0x007FFFFFL;
	}
	m = (m<<8) | e;

	return neg ? dspnegfloat(m) : m;
}

/*	dspnegfloat() converts a positive DSP32C float to a
	negative one. Both are passed as longs.	*/
long dspnegfloat(long f)
{
	long m,e;

	if(f == 0L)
		return f;

	m = f >> 8;
	e = f & 0xFFL;

	m = -m;
	if(f > 0L && m == 0L)
		e -= 1, m = 0x00800000L;
	else if(f < 0L && m == 0L)
		e += 1;

	return (m<<8) | e;
}

void list_address(long addr)
{
	char buf[10];

	if(Pass != 2 || List[0] == '\0')
		return;
	sprintf(buf,"%06lX:",addr);
	memcpy(List+ADDR_COL-1,buf,7);
}

void list_data(long data, int offset, int size)
{
	char buf[24];
	int more = 0;
	int i,j,n;
	static long mask[5] = {0,0xFFL,0xFFFFL,0xFFFFFFL,0xFFFFFFFFL};

	if(Pass != 2)
		return;
	if(size < 0) {
		size = -size;
		more = 3;
	}

	if(List[0] == '\0') {
		memset(List,' ',SOURCE_COL);
		List[SOURCE_COL-1] = '\n';
		List[SOURCE_COL] = '\0';
	}
	if(offset == 0)
		list_address(Org);
	sprintf(buf,"%0*lX...",size+size,data&mask[size]);
	n = size+size+more;
	for(i=0, j=DATA_COL-1+offset*3; i<n; ++i,++j) {
		if(List[j] != ' ')
			break;
		List[j] = buf[i];
	}
	if(offset+size >= 4)
		list_print();
}

void prepare_list()
{
	if(List_in && strcmp(Filename,Main_filename)==0 && List_line < Line_number) {
		while(List_line < Line_number) {
			if(List[0])
				list_print();
			if(!fgets(Input2,sizeof(Input2),List_in)) {
				List_in = 0;
				break;
			}
			prepare_list_line(++List_line,Input2,' ');
		}
	} else {
		prepare_list_line(Line_number,Input,'X');
	}
}

void prepare_list_line(int lineno, char *input, char flag)
{
	int i,j;

	if(strncmp(input,"#line",5) == 0) {
		List[0] = '\0';
		return;
	}

	if(strcmp(Filename,Main_filename) != 0)
		flag = 'I';

	sprintf(List,"%4d%c",lineno,flag);
	memset(List+5,' ',SOURCE_COL-1);

	if(Block_comment || (input[0] == '/' && input[1] == '*')) {
		i = ADDR_COL - 1;
		Block_comment = 1;
	} else {
		i = SOURCE_COL-1;
	}

	for(j=0; input[j]; ++j) {
		if(i >= sizeof(List)-2)
			break;
		if(input[j] == '\t') {
			do {
				List[i++] = ' ';
			} while(((i-SOURCE_COL)&7) != 7);
		} else {
			List[i++] = input[j];
		}
	}
	List[i] = '\0';

	for(j=0; input[j]; ++j) {
		if(input[j] == '*' && input[j+1] == '/') {
			Block_comment = 0;
			break;
		}
	}
}

void list_print()
{
	if(Pass != 2 || List[0] == '\0')
		return;
	if(((List_type&1)!=0 && strcmp(Filename,Main_filename) == 0) ||
	   ((List_type&2)!=0 && List[DATA_COL] != ' ' && !Block_comment) ||
	   ((List_type&4)!=0) ){
		if(++Line_on_page > Page_length)
			list_page(0);
		if(strlen(List) >= Page_width) {
			List[Page_width-1] = '\n';
			List[Page_width] = '\0';
		}
		fputs(List,List_file);
	}
	List[0] = '\0';
}

void list_page(int omit_list)
{
	if(Pass != 2)
		return;

	if(++Page_number > 1)
		fputc('\f',List_file);

	fprintf(List_file,"\n\n\n%s%3d\n\n",Title,Page_number);

	Line_on_page = 6;

	if(omit_list)
		List[0] = '\0';
}

void end_stmt()
{
}

void define_data(long value, int size)
{
	int list = 1;

	if(size < 0) {
		list = 0;
		size = -size;
	}

	if(Org%size != 0)
		errormsg("Alignment Error");

	if(Org+size >= MAX_DATA) {
		errormsg("Program Exceeded MAX_DATA in Size");
		Org += size;
	} else {
		if(Pass == 1) {
			Org += size;
		} else {
			if(list)
				list_data(value,(int)Org&3,size);
			if((size==1 && (value > 127L   || value < -128L))  ||
			   (size==2 && (value > 65536L || value < -32768L)))
				warning("Value Exceeds Data Size");
			while(size--) {
				Data[Org++] = value;
				value >>= 8;
			}
		}
	}
}

void define_array(int n, long value, int size)
{
	list_address(Org);
	list_data(value,(int)Org&3,-size);
	while(n-- > 0)
		define_data(value,-size);
}

/*	SYMBOL TABLE ROUTINES	*/

#define N_HASH	128	/* power of 2 */
Symbol *Symbol_table[N_HASH] = {0};

int hash(char *name)
{
	int h;
	
	h = 0;
	while(*name) {
		h = (h << 1) | ((h>>7) & 1);
		h += *name++;
	}
	
	return h & (N_HASH-1);
}

Symbol *symbol(char *name)
{
	Symbol *s;
	int h;
	
	h = hash(name);
	for(s=Symbol_table[h]; s; s=s->next) {
		if(strncmp(name,s->name,sizeof(s->name)) == 0) {
			if(s->type == SYMBOL || s->type == LABEL)
				Last_symbol = s;
			return s;
		}
	}

	/* create a new Symbol entry */
	s = (Symbol *)malloc(sizeof(Symbol));
	if(!s)
		fatal("Out of Memory");
	s->next = Symbol_table[h];
	Symbol_table[h] = s;
	strncpy(s->name,name,sizeof(s->name));
	s->type = SYMBOL;
	s->value = UNDEFINED;
	Last_symbol = s;
	return s;
}

void define_label(long value)
{
	if(!Symbol_def) {
		errormsg("Undefined Symbol");
		return;
	}
	
	if(Pass == 1) {
		if(Symbol_def->type == SYMBOL) {
			Symbol_def->type = LABEL;
			Symbol_def->value = value;
		}
	} else {
		if(Symbol_def->value != value)
			error_symbol("Phase Error or Redefinition of",
								Symbol_def);
		else if(value == UNDEFINED)
			error_symbol("Undefined value in",Symbol_def);
	}
	
	list_address(Symbol_def->value);
}

void define_symbol(long value)
{
	char buf[80];

	if(!Symbol_def) {
		errormsg("Undefined Symbol");
		return;
	}

	if(Symbol_def->type != SYMBOL) {
		error_symbol("Attempt to re-define Reserved Word or Label",
						Symbol_def);
	} else {
		Symbol_def->value = value;
		if(List[0] != '\0') {
			sprintf(buf,"%08lx",value);
			memcpy(List+ADDR_COL-1,buf,8);
		}
	}
}

void symbol_init()
{
	int i,j, h;
	char buf[16];
	Symbol *s;
	static Symbol reserved[] = {
		{0,"a0",ACCUM,0L},	/* accumulators */
		{0,"a1",ACCUM,1L},
		{0,"a2",ACCUM,2L},
		{0,"a3",ACCUM,3L},
		{0,"ibuf",IBUF,4L},	/* other data registers */
		{0,"ibufe",IBUF,4L|EXT},
		{0,"ibufl",IBUF,4L|LOW},
		{0,"obuf",OBUF,5L},
		{0,"obufe",OBUF,5L|EXT},
		{0,"obufl",OBUF,5L|LOW},
		{0,"pdr",PDR,6L},
		{0,"pdre",PDR,6L|EXT},
		{0,"pdr2",PDR,20L},
		{0,"pc",PC,15L},
		{0,"pce",PC,15L|EXT},
		{0,"pin",REGIO,24L|EXT},
		{0,"pout",REGIO,25L|EXT},
		{0,"dauc",DAUC,26L},
		{0,"ioc",IOC,27L},
		{0,"ivtp",REGIO,29L|EXT},
		{0,"pcsh",PCSH,30L},
		{0,"pcshe",PCSH,30L|EXT},
		{0,"piop",PIOP,14L|LOW},
		{0,"pir",PIR,22L},
		{0,"pcw",PCW,30L},
		{0,"org",ORG,UNDEFINED}, /* psuedo-ops */
		{0,"end",END,UNDEFINED},
		{0,"align",ALIGN,UNDEFINED},
		{0,"byte",BYTE,UNDEFINED},
		{0,"char",BYTE,UNDEFINED},
		{0,"word",INT,UNDEFINED},
		{0,"long",INT24,UNDEFINED},
		{0,"page",PAGE,UNDEFINED},
		{0,"list",LIST,UNDEFINED},
		{0,"equ",EQU,UNDEFINED},
		{0,"ic",DA_FUN,0L},	/* DA special instructions */
		{0,"oc",DA_FUN,1L},
		{0,"float",FLOAT,2L},
		{0,"int",INT,3L},
		{0,"round",DA_FUN,4L},
		{0,"ifalt",DA_FUN,5L},
		{0,"ifaeq",DA_FUN,6L},
		{0,"ifagt",DA_FUN,7L},
		{0,"float24",DA_FUN,10},
		{0,"int24",INT24,11},
		{0,"ieee",DA_FUN,12},
		{0,"dsp",DA_FUN,13},
		{0,"seed",DA_FUN,14},
		{0,"if",IF,UNDEFINED},	/* CA Special instructions */
		{0,"goto",GOTO,UNDEFINED},
		{0,"call",CALL,UNDEFINED},
		{0,"return",RETURN,UNDEFINED},
		{0,"ireturn",IRETURN,UNDEFINED},
		{0,"do",DO,UNDEFINED},
		{0,"nop",NOP,UNDEFINED},
		{0,"pl",CA_COND,2},		/* CA conditions */
		{0,"mi",CA_COND,3},
		{0,"ne",CA_COND,4},
		{0,"eq",CA_COND,5},
		{0,"vc",CA_COND,6},
		{0,"vs",CA_COND,7},
		{0,"cc",CA_COND,8},
		{0,"cs",CA_COND,9},
		{0,"ge",CA_COND,10},
		{0,"lt",CA_COND,11},
		{0,"gt",CA_COND,12},
		{0,"le",CA_COND,13},
		{0,"hi",CA_COND,14},
		{0,"ls",CA_COND,15},
		{0,"ane",DA_COND,20},		/* DA conditions */
		{0,"aeq",DA_COND,21},
		{0,"age",DA_COND,18},
		{0,"alt",DA_COND,19},
		{0,"avc",DA_COND,22},
		{0,"avs",DA_COND,23},
		{0,"auc",DA_COND,16},
		{0,"aus",DA_COND,17},
		{0,"agt",DA_COND,24},
		{0,"ale",DA_COND,25},
		{0,"ibe",IO_COND,32},		/* I/O conditions */
		{0,"ibf",IO_COND,33},
		{0,"obe",IO_COND,35},
		{0,"obf",IO_COND,34},
		{0,"pde",IO_COND,36},
		{0,"pdf",IO_COND,37},
		{0,"pie",IO_COND,38},
		{0,"pif",IO_COND,39},
		{0,"syc",IO_COND,40},
		{0,"sys",IO_COND,41},
		{0,"fbc",IO_COND,42},
		{0,"fbs",IO_COND,43},
		{0,"ireq1_hi",IO_COND,45},
		{0,"ireq1_lo",IO_COND,44},
		{0,"ireq2_hi",IO_COND,46},
		{0,"ireq2_lo",IO_COND,47},
	};

	for(i=0; i<N_HASH; ++i)
		Symbol_table[i] = (Symbol *)0;

	for(i=0; i<sizeof(reserved)/sizeof(reserved[0]); ++i) {
		h = hash(reserved[i].name);
		reserved[i].next = Symbol_table[h];
		Symbol_table[h] = &reserved[i];
	}
	
	/* add registers */
	for(i=1; i<=22; ++i) {
		if(i <= 14)
			j = i;
		else if(i <= 19)
			j = i + 2;
		else if(i <= 21)
			j = i + 4;
		else
			j = 29;
		sprintf(buf,"r%d",i);
		s = symbol(buf);
		s->type = REG;
		s->value = j;
		sprintf(buf,"r%dr",i);
		s = symbol(buf);
		s->type = REGR;
		s->value = j | REV;
		sprintf(buf,"r%de",i);
		s = symbol(buf);
		s->type = REGE;
		s->value = j | EXT;
		sprintf(buf,"r%dl",i);
		s = symbol(buf);
		s->type = REGHL;
		s->value = j | LOW;
		sprintf(buf,"r%dh",i);
		s = symbol(buf);
		s->type = REGHL;
		s->value = j | HIGH;
	}	
}

void undefine_symbols(int type)
{
	int i;
	Symbol *s;

	for(i=0; i<N_HASH; ++i) {
		for(s=Symbol_table[i]; s; s=s->next) {
			if(s->type == type)
				s->value = UNDEFINED;
		}
	}
}

/*	mem_ref() - prepare a memory-reference via regs
	result is for CA format 7b (or 7d), r,P,I fields
	NO VALIDITY CHECKS ARE PERFORMED!
*/
long mem_ref(long reg, long incr)
{
	long v = ((reg&0x1FL)<<5) | (incr&0x1FL);
	if((incr&REV) != 0)
		v |= (1<<10);
	return v;
}

/*	x_ref() converts a mem_ref() to X.	*/
long x_ref(long mem_ref)
{
	unsigned int p = (mem_ref>>5) & 0x1FL;
	unsigned int i = mem_ref & 0x1FL;

	if((i >= 16 && i <= 23) || (p == 0 && i <= 4))
		i &= 7;
	else
		i = 99;	
	if((mem_ref&(1L<<10)) != 0)
		errormsg("X-field cannot use bit-reversed increment");

	if((p == 0 && i >= 5) || i > 7 || p >= 15)
		errormsg("Illegal reference in X field");

	return ((long)i << 14) | ((long)p << 17);		
}

/*	y_ref() converts a mem_ref() to Y.	*/
long y_ref(long mem_ref)
{
	unsigned int p = (mem_ref>>5) & 0x1FL;
	unsigned int i = mem_ref & 0x1FL;

	if((i >= 16 && i <= 23) || (p == 0 && (i <= 4 || i == 6)))
		i &= 7;
	else
		i = 99;
	if((mem_ref&(1L<<10)) != 0)
		errormsg("Y-field cannot use bit-reversed increment");

	if((p == 0 && i >= 5 && i != 6) || i > 7 || p >= 15)
		errormsg("Illegal reference in Y field");

	return ((long)i << 7) | ((long)p << 10);		
}

/*	z_ref() converts a mem_ref() to Z.	*/
long z_ref(long mem_ref)
{
	unsigned int p = (mem_ref>>5) & 0x1FL;
	unsigned int i = mem_ref & 0x1FL;
	long r = (mem_ref&(1L<<10)) ? (1L<<25) : 0L;

	if(p != 0) {
		if(i >= 16 && i <= 23)
			i &= 7;
		else if(i != 7)
			i = 99;
	} else {
		if(i <= 4 || i > 7)
			i = 99;
	}
	if(p > 15 || i > 7)
		errormsg("Illegal reference in Z field");

	return (long)i | ((long)p<<3) | r;
}

void da_1(long fmt, long m, long f, long s, long da_dest, long x, long y, long z)
{
	long n,v;
	
	if(z == 7L)
		z = da_dest & 0x7FFL;
	else if((da_dest&0x7FFL) != 7)
		errormsg("Two Z= fields is Illegal");

	n = (da_dest>>21) & 3L;
	v = (fmt<<29) | (m<<26) | (f<<24) | (s<<23) | (n<<21) |
						 x_ref(x) | y_ref(y) | z_ref(z);
	define_data(v,4);
}

void da_5(long g, long da_dest, long y)
{
	long v = 0x78000000L | (g<<23) | (da_dest&(3L<<21)) | y_ref(y) |
							z_ref(da_dest&0x7FF);
	define_data(v,4);							
}

long goto_dest(long reg, long n)
{
	if(n > 65536L || n < -32768L)
		warning("Offset Exceeds Field Width");
	if((reg&~(EXT|0x1FL)) != 0L)
		errormsg("Illegal register in goto");
	return ((reg&0x1FL)<<16) | (n & 0x0000FFFFL);
}

void ca_0(long cond, long goto_dest)
{
	long v = ((cond&0x3FL)<<21) | goto_dest;

        define_data(v,4);
}

void ca_3a(long m, long goto_dest)
{
	long v = 0x0C000000L | ((m&0x1FL)<<21) | goto_dest;

	define_data(v,4);
}

void ca_4(long m, long h, long n)
{
	long v;
	if(n > 65536L || n < -32768L)
		warning("Offset Exceeds Field Width");
	v = 0x10000000L | ((m&0x1FL)<<21) | ((h&0x1FL)<<16) | (n&0x0000FFFFL);
	define_data(v,4);
}

void ca_5(long d, long h, long n)
{
	long v = (d & EXT) | 0x14000000L;

	if(n > 65536L || n < -32768L)
		warning("Offset Exceeds Field Width");
	v |= ((d&0x1FL)<<21) | ((h&0x1FL)<<16) | (n&0x0000FFFFL);
	define_data(v,4);
}

long ca_6a(long f, long d, long s1, long s2)
{
	long v = (d & EXT) | 0x18000000L;

	v |= ((f&0xFL)<<21) | ((d&0x1FL)<<16) | ((s1&0x1FL)<<5) | (s2&0x1FL);
	if(s2 != 0)
		v |= 0x00000800L;	/* E */
	return v;
}

void ca_6c(long f, long d, long n, long s)
{
	long v = (d & EXT) | 0x1A000000L;

	if(s != 0L && s != d)
		errormsg("Illegal 3-operand Instruction with Constant");
	if(n > 65536L || n < -32768L)
		warning("Offset Exceeds Field Width");
	v |= ((f&0xfL)<<21) | ((d&0x1FL)<<16) | (n&0x0000FFFFL);
	define_data(v,4);
}

void ca_7a(long t, long h, long n)
{
	long v = 0x1C000000L;

	if(n > 65536L || n < -32768L)
		warning("Offset Exceeds Field Width");
	v |= (t<<24) | ((h&0x1FL)<<16) | (n&0x0000FFFFL);
	if((h&EXT) != 0)
		v |= 3L<<22;
	else if((h&LOW) != 0)
		v |= 1L<<22;
	else if((h&HIGH) != 0)
		v |= 0L<<22;
	else
		v |= 2L<<22;
	define_data(v,4);
}

void ca_7b(long t, long h, long mem_ref)
{
	long v = 0x1E000000L | (t<<24) | ((h&0x1FL)<<16) | (mem_ref&0x7FFL);
	if((mem_ref&0x3E0L) != 0L)
		v |= (1L<<21);     /* NOT I/O instr */
	else if((h&EXT) != (mem_ref&EXT) ||
				!!(h&(LOW|HIGH)) != !!(mem_ref&(LOW|HIGH)))
		errormsg("Inconsistent Size in I/O Instruction");
	if((h&EXT) != 0)
		v |= 3L<<22;
	else if((h&LOW) != 0)
		v |= 1L<<22;
	else if((h&HIGH) != 0)
		v |= 0L<<22;
	else
		v |= 2L<<22;
	define_data(v,4);
}

void ca_7d(long t, long r, long mem_ref)
{
	long v = 0x1C200000L | (t<<24) | ((r&0x1FL)<<16) | (mem_ref&0x7FFL);
	if((r&EXT) != 0)
		v |= 3L<<22;
	else if((r&LOW) != 0)
		v |= 1L<<22;
	else if((r&HIGH) != 0)
		v |= 0L<<22;
	else
		v |= 2L<<22;
	define_data(v,4);
}

void ca_8(int type, long reg, long expr)
{
	long v;

	reg &= 0x0000001FL;  /* mask off size bits */
	v = ((long)type<<29) | (reg<<16) | ((expr&0x00FF0000L)<<5) |
						(expr&0x0000FFFFL);
        define_data(v,4);
}
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 9,
	286, 28,
	61, 24,
	-2, 181,
-1, 10,
	286, 30,
	61, 26,
	-2, 182,
-1, 64,
	286, 28,
	61, 24,
	-2, 181,
-1, 65,
	286, 30,
	61, 26,
	-2, 182,
-1, 345,
	42, 72,
	-2, 69,
-1, 346,
	42, 73,
	-2, 70,
-1, 347,
	42, 74,
	-2, 71,
	};
# define YYNPROD 208
# define YYLAST 1338
yytabelem yyact[]={

    20,   337,   291,   310,   297,   338,   197,    32,   304,    32,
    27,   307,   133,   286,    76,   143,    79,    82,    85,    88,
   141,   332,   112,   113,   178,    49,   179,    50,    51,   115,
   116,   117,   118,   328,   331,   330,    45,   329,   150,    34,
    33,   125,   126,    94,   149,    93,   194,   195,   196,   134,
   111,    99,   135,   278,   136,   144,   108,   244,   109,   139,
   110,   253,   111,    99,   135,   315,   136,   144,   108,    90,
   109,    43,   110,   102,   342,   103,   244,   123,   382,    59,
    84,    60,    61,    62,   244,   102,    91,   103,    96,    59,
   348,    60,    61,    62,    87,   137,   155,   156,   157,   158,
   159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
   169,   170,   171,   307,   244,   243,    81,   244,    59,    98,
    60,    61,    62,   142,   371,   349,   220,   191,   140,   208,
   214,   216,   217,   218,   209,   244,   219,   220,   400,   132,
    73,   226,   227,   228,   229,   155,    82,    85,    88,    97,
   131,   121,   234,   235,   236,   120,   182,   186,   190,   184,
   188,   119,   192,   114,   205,   206,   213,   215,   211,   210,
   296,    12,   193,   239,    70,   111,   111,   148,   241,   240,
   144,   144,   108,   405,   109,   110,   110,    59,   356,    60,
    61,    62,   130,   147,   193,   355,   402,   146,    12,   129,
   255,   256,   257,   258,   111,   260,   145,   399,   118,   144,
   108,   354,   109,   276,   110,   128,   397,   288,   233,   289,
    11,    29,   368,   281,   282,   283,   284,   231,    28,   374,
    48,   375,   277,    31,    59,   254,    60,    61,    62,   285,
   259,   237,   232,   280,   274,   287,   288,    11,   289,   292,
    12,   366,   299,   299,   299,   299,   290,   295,   279,   384,
   308,   309,   230,   380,   318,   320,   323,   325,   327,   175,
   176,   177,   174,    29,   334,   372,   169,   373,    92,   300,
    28,   249,    48,   294,   293,    31,   245,   306,   298,   340,
   379,   288,   344,   289,   378,   343,   345,   401,   346,    11,
   317,   319,   321,   324,   326,   347,   341,   224,   359,   225,
   333,   377,   335,   339,    30,   100,   101,   104,   105,   106,
   107,   360,   305,   117,   222,   376,   223,   100,   101,   104,
   105,   106,   107,   201,   353,   202,   300,   352,    59,   242,
    60,    61,    62,   245,   357,   367,   245,   351,   292,   361,
   362,   363,   364,   369,   299,   111,    99,   336,   314,   365,
   144,   108,   277,   109,   339,   110,    30,   350,   381,   316,
   383,   311,   262,   391,   392,   393,   394,   252,   102,   251,
   103,    96,   294,   293,   250,   395,   248,   396,   299,   247,
   246,   370,   213,   215,   127,   122,    54,    56,   398,   403,
   404,    52,    53,    55,    57,     2,    75,   406,   274,    63,
    72,    59,    98,    60,    61,    62,   189,    74,   301,   302,
   303,    71,   317,   319,   321,   324,   326,   333,    89,    86,
    59,    83,    60,    61,    62,    80,   198,     6,   365,   199,
     5,   200,    97,     3,     8,   106,   107,    59,     1,    60,
    61,    62,    35,    37,    54,    56,    44,    47,    46,    52,
    53,    55,    57,    64,    65,    49,    26,    50,    51,    13,
    14,    15,    16,    21,    25,    22,    17,    18,    19,    24,
    36,    58,    40,    42,    38,    41,    39,     7,    23,   173,
     0,     0,     0,     0,     0,     0,     8,     0,     0,    59,
     0,    60,    61,    62,    35,     0,    54,    56,     0,    47,
    46,    52,    53,    55,    57,     9,    10,    49,     0,    50,
    51,    13,    14,    15,    16,    21,     0,    22,    17,    18,
    19,     0,    36,    58,    40,    42,    38,    41,    39,   111,
    99,     0,   313,     0,   144,   108,     0,   109,     0,   110,
   111,    99,     0,   312,     0,   144,   108,     0,   109,     0,
   110,     4,   102,     0,   103,    96,     0,    66,    67,    68,
    69,     0,     0,   102,     0,   103,    96,   111,    99,     0,
     0,     0,   144,   108,     0,   109,     0,   110,     0,   111,
    99,     0,     0,     0,   144,   108,    98,   109,   238,   110,
   102,     0,   103,    96,     0,     0,     0,    98,     0,     0,
     0,     0,   102,   221,   103,    96,     0,     0,     0,     0,
   100,   101,   104,   105,   106,   107,    97,     0,     0,     0,
     0,     0,     0,     0,    98,   111,    99,    97,     0,     0,
   144,   108,   204,   109,     0,   110,    98,   111,    99,     0,
   203,     0,   144,   108,     0,   109,     0,   110,   102,   390,
   103,    96,   387,     0,    97,     0,   272,   385,     0,   386,
   102,   268,   103,    96,   111,    99,    97,     0,   180,   144,
   108,     0,   109,     0,   110,     0,     0,     0,   111,    99,
     0,     0,    98,   144,   108,   172,   109,   102,   110,   103,
    96,     0,   111,    99,    98,     0,     0,   144,   108,     0,
   109,   102,   110,   103,    96,     0,     0,    29,   389,     0,
     0,     0,    97,     0,    28,   102,     0,   103,    96,    31,
   111,    98,    29,     0,    97,   144,   108,     0,   109,    28,
   110,   111,    99,     0,    31,    98,   144,   108,   388,   275,
     0,   110,     0,   102,     0,   103,     0,     0,     0,    98,
     0,    97,     0,     0,   102,     0,   103,    96,     0,     0,
     0,     0,     0,     0,     0,    97,   111,    99,     0,     0,
   273,    95,   108,   265,   109,     0,   110,   272,   263,    97,
   264,     0,   268,     0,     0,     0,     0,     0,    98,   102,
     0,   103,    96,     0,   100,   101,   104,   105,   106,   107,
    30,     0,     0,     0,     0,   100,   101,   104,   105,   106,
   107,     0,     0,     0,     0,    30,   111,    99,    97,     0,
     0,   144,   108,    98,   109,     0,   110,     0,     0,   267,
     0,    29,   100,   101,   104,   105,   106,   107,    28,   102,
   212,   103,     0,   207,   100,   101,   104,   105,   106,   107,
     0,     0,   111,    97,     0,     0,     0,   144,   108,   266,
   109,   111,   110,     0,     0,     0,   144,   108,     0,   109,
     0,   110,     0,    98,     0,   102,     0,   103,     0,     0,
     0,     0,     0,     0,   102,     0,   103,     0,     0,     0,
   100,   101,   104,   105,   106,   107,     0,     0,     0,     0,
     0,     0,   100,   101,   104,   105,   106,   107,     0,     0,
     0,     0,     0,    29,     0,     0,     0,     0,     0,     0,
    28,   269,   270,   271,    30,    31,     0,     0,    29,   100,
   101,   104,   105,   106,   107,    28,     0,     0,     0,     0,
    31,     0,     0,   100,   101,   104,   105,   106,   107,    77,
    78,    49,     0,    50,    51,     0,     0,   100,   101,   104,
   105,   106,   107,    29,    77,    78,    49,     0,    50,    51,
    28,     0,     0,   151,     0,    31,     0,   152,   153,   154,
     0,     0,     0,     0,     0,   100,   101,   104,   105,   106,
   107,     0,     0,     0,     0,    29,   100,   101,   104,   105,
   106,   107,    28,    29,     0,     0,    30,    31,     0,     0,
    28,     0,     0,     0,     0,    31,     0,     0,     0,     0,
     0,    30,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   100,   101,   104,   105,   106,   107,     0,     0,    29,
     0,     0,   269,   270,   271,     0,    28,     0,     0,     0,
     0,    31,     0,     0,     0,     0,    30,    59,     0,    60,
    61,    62,     0,     0,    54,    56,     0,     0,     0,    52,
    53,    55,    57,    77,    78,    49,     0,    50,    51,     0,
    29,   100,   101,   104,   105,   106,   107,    28,    30,     0,
     0,     0,    31,     0,     0,    59,    30,    60,    61,    62,
   185,     0,    54,    56,   187,     0,     0,    52,    53,    55,
    57,     0,     0,     0,     0,     0,     0,     0,   101,   104,
   105,   106,   107,     0,     0,     0,     0,     0,   104,   105,
   106,   107,   322,     0,     0,     0,     0,     0,     0,    59,
     0,    60,    61,    62,     0,   358,     0,     0,     0,     0,
     0,     0,     0,     0,   261,    77,    78,    49,     0,    50,
    51,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    77,    78,    49,    30,    50,    51,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
     0,    60,    61,    62,     0,   138,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    77,    78,    49,     0,    50,
    51,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    59,     0,    60,    61,    62,     0,   124,     0,    59,
     0,    60,    61,    62,     0,     0,     0,    77,    78,    49,
     0,    50,    51,     0,     0,    77,    78,    49,     0,    50,
    51,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    59,     0,    60,    61,    62,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    77,    78,    49,     0,    50,    51,    59,     0,    60,
    61,    62,   181,     0,    54,    56,   183,     0,     0,    52,
    53,    55,    57,     0,     0,     0,   135,     0,   136,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    77,    78,    49,     0,    50,    51 };
yytabelem yypact[]={

-10000000,   240,-10000000,   188,-10000000,   161,   161,   161,   161,   116,
    82,-10000000,-10000000,   684,-10000000,   684,   684,   684,   684,     0,
   739,   684,   684,   102,-10000000,-10000000,-10000000,-10000000,   684,   684,
   684,   684,   100,    94,    90,-10000000,   355,-10000000,-10000000,-10000000,
   972,   684,   354,-10000000,-10000000,   154,    89,    78,  1057,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   940,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,    67,  -266,-10000000,    62,  -271,   665,-10000000,-10000000,   665,
   162,-10000000,   665,   153,-10000000,   665,   149,-10000000,   665,   133,
-10000000,-10000000,-10000000,  -234,  -240,   699,   684,   684,   684,   684,
   684,   684,   684,   684,   684,   684,   684,   684,   684,   684,
   684,   684,   651,   665,   -19,   637,-10000000,-10000000,-10000000,  1038,
   846,   152,  -253,   396,   290,   610,   598,  -170,   808,   980,
   980,   684,   684,  -121,   552,-10000000,-10000000,   281,   264,   665,
   684,   684,   684,   684,   684,   684,   684,   684,     0,-10000000,
-10000000,   684,   684,   684,     0,-10000000,   540,   789,    13,   693,
   825,   834,   139,   139,   139,   139,   167,   167,   138,   138,
-10000000,-10000000,   684,    75,   350,   349,   346,   241,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,  -195,   343,   338,   336,  -197,  -170,   684,
   684,   684,   684,  -170,   905,   331,   745,   980,   704,-10000000,
-10000000,-10000000,  1057,-10000000,   665,-10000000,   665,   665,   665,  -207,
-10000000,   -25,   684,   684,   684,   684,   665,   665,   665,   665,
-10000000,-10000000,-10000000,-10000000,   665,   665,   665,-10000000,   684,   665,
   203,-10000000,   248,   130,  -195,-10000000,    15,    15,    15,    15,
  -180,  -282,  -282,  -302,   330,   513,   502,   318,    25,   328,
   665,-10000000,-10000000,   980,   980,  1016,   980,   980,  -247,  -242,
  -244,  -245,  -259,   980,-10000000,   980,   665,-10000000,-10000000,-10000000,
-10000000,   665,   665,   665,   665,   665,    93,    42,-10000000,-10000000,
    34,    29,-10000000,-10000000,-10000000,  -132,   326,-10000000,-10000000,-10000000,
-10000000,   306,   296,   293,-10000000,-10000000,   150,   890,-10000000,-10000000,
  -252,-10000000,  -170,  -170,  -170,  -170,-10000000,-10000000,   665,-10000000,
   665,-10000000,   980,   665,-10000000,   665,-10000000,   665,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,   665,-10000000,   209,-10000000,-10000000,-10000000,
-10000000,-10000000,   130,   180,-10000000,-10000000,-10000000,-10000000,    72,  -195,
-10000000,-10000000,-10000000,-10000000,    79,  -170,  -170,   232,   186,   665,
   284,   270,   253,   249,   222,-10000000,    42,    17,    42,   218,
   624,  -170,   684,   684,   684,   684,  -282,-10000000,-10000000,-10000000,
-10000000,-10000000,    72,-10000000,   174,   -72,  -141,   171,  -170,  -170,
  -170,   665,   665,   665,   665,-10000000,   155,    42,    42,-10000000,
-10000000,  -170,   141,-10000000,-10000000,    42,-10000000 };
yytabelem yypgo[]={

     0,     0,    86,    10,   489,    13,     1,     4,   170,     2,
   488,     5,    12,    40,   487,   479,   474,   466,    71,   456,
     8,   453,    36,    39,   448,   405,   443,   561,   440,   437,
   435,   431,   429,   428,   421,   417,   410,   406,   116,    80,
    94,    69 };
yytabelem yyr1[]={

     0,    24,    24,    24,    25,    25,    25,    25,    25,    27,
    27,    26,    26,    28,    28,    28,    28,    28,    28,    28,
    28,    28,    28,    28,    34,    28,    35,    28,    36,    28,
    37,    28,    28,    28,    28,    28,    30,    30,    38,    31,
    31,    39,    32,    32,    40,    33,    33,    41,     2,     2,
     2,    29,    29,    29,    29,    29,    29,    29,    29,    29,
    29,    29,    29,    29,    29,    10,    10,    10,    10,     6,
     6,     6,     7,     7,     7,     8,     8,     9,     9,     9,
    11,    11,    11,    11,    12,    12,    13,     4,     4,     4,
     5,     5,    14,    14,    14,    15,    15,    15,    15,    15,
    15,    15,    15,    15,    15,    15,    15,    15,    15,    15,
    15,    20,    20,    20,    20,    20,    20,    21,    21,    21,
    21,    21,    21,    22,    22,    22,    22,    16,    16,    16,
    16,    16,    18,    18,    18,    18,    18,    18,    18,    18,
    18,    18,    18,    18,    18,    18,    18,    18,    19,    19,
    19,    19,    19,    19,    19,    19,    19,    17,    17,    17,
    17,    17,    17,    17,    17,    17,    17,    17,    17,    17,
    17,    17,    17,    17,    17,    23,    23,    23,    23,    23,
    23,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     3,     3,     3 };
yytabelem yyr2[]={

     0,     0,     5,     7,     2,     4,     4,     4,     5,     2,
     2,     5,     5,     5,     3,     5,     4,     4,     4,     4,
     9,     9,     9,     9,     1,     9,     1,     9,     1,     9,
     1,     9,     3,     5,     9,     5,     2,     6,     3,     2,
     6,     3,     2,     6,     3,     2,     6,     3,     2,     5,
     5,     3,    17,    13,     9,    25,    21,    17,    13,    13,
    21,    13,    13,    13,    13,     7,     7,     7,     3,     2,
     3,     3,     2,     3,     3,     2,     3,     2,     2,     2,
     5,     7,     7,     9,     2,     2,     9,     3,     3,     1,
     3,     3,     2,     2,     2,    11,    11,    11,     3,     3,
     3,    17,    11,    15,    15,    15,    15,    11,     9,     9,
     9,     5,     9,     9,     9,     9,     5,     5,     9,     9,
     9,     9,     5,     2,     2,     2,     2,     3,     3,    11,
    19,    19,    11,    11,     7,    11,     7,    11,    11,    11,
    11,    11,    11,     9,    11,    11,    13,     7,    11,    11,
     7,    11,    11,     7,    11,    11,    11,     7,     7,     7,
     7,     7,     9,     7,     7,     9,     9,     7,     7,     7,
     7,     7,     7,     7,     7,     2,     2,     2,     2,     2,
     2,     3,     3,     2,     7,    11,     7,     7,     7,     7,
     7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
     7,     7,     5,     5,     5,     2,     2,     2 };
yytabelem yychk[]={

-10000000,   -24,   -25,   -26,   -27,   -28,   -29,   -14,   256,   275,
   276,    59,    10,   281,   282,   283,   284,   288,   289,   290,
    -1,   285,   287,   -10,   -15,   -16,   -17,    -3,    40,    33,
   126,    45,   -11,   -13,   -23,   264,   292,   -21,   296,   298,
   294,   297,   295,   -18,   -19,   -22,   270,   269,    42,   277,
   279,   280,   271,   272,   266,   273,   267,   274,   293,   259,
   261,   262,   263,   -25,   275,   276,   -27,   -27,   -27,   -27,
    58,   -34,   -36,    58,   -35,   -37,    -1,   275,   276,    -1,
   -30,   -38,    -1,   -31,   -39,    -1,   -32,   -40,    -1,   -33,
   -41,    -2,   278,    45,    43,    42,    63,   124,    94,    38,
   302,   303,    60,    62,   304,   305,   306,   307,    43,    45,
    47,    37,    -1,    -1,    61,    -1,    -1,    -1,    -1,    61,
    61,    61,    40,   -22,   265,    -1,    -1,    40,    61,    45,
    38,    61,    61,   -12,    -1,   259,   261,   -22,   265,    -1,
    61,   286,    61,   286,    42,    44,    44,    44,    44,   278,
   278,   284,   288,   289,   290,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    44,    -4,   291,   288,   289,   290,    43,    45,
    41,   264,   -22,   268,   -23,   264,   -22,   268,   -23,   264,
   -22,   -11,   -13,    42,   299,   300,   301,   259,    40,    43,
    45,    43,    45,    40,    44,   -22,   -22,    45,    -1,   -11,
   -13,   -23,    42,   -22,    -1,   -22,    -1,    -1,    -1,   257,
   258,    61,    43,    45,    43,    45,    -1,    -1,    -1,    -1,
   -38,   -39,   -40,   -41,    -1,    -1,    -1,    -2,    58,    -1,
   -11,    -7,   264,    40,    42,   271,    40,    40,    40,    40,
    41,    41,    41,   258,   -22,    -1,    -1,    -1,    -1,   -22,
    -1,   259,    41,    43,    45,    38,   124,    94,    47,   307,
   308,   309,    42,    35,   -22,    45,    -1,   -12,   260,   -22,
   268,    -1,    -1,    -1,    -1,    -1,    -5,    42,    43,    45,
    -5,    -9,   -11,   -13,   -23,   -12,    -8,    -7,   273,   -11,
   264,    -8,    -8,    -8,   -20,   -18,   -22,   293,   -20,   -20,
   305,    41,    40,    40,    40,    40,    41,   -22,    -1,   -22,
    -1,   -22,   126,    -1,   -22,    -1,   -22,    -1,   280,   279,
   279,   279,   280,   -22,    -1,   -22,   264,    -6,   -11,   271,
    -6,   264,    40,    -7,    -6,   -11,   264,   271,    61,   257,
    41,    41,    41,    41,    61,    45,    38,   -22,   265,    -1,
    -3,   -22,   -22,   -22,   -22,   -22,    42,    -9,    42,    -7,
   -22,    45,    43,    45,    43,    45,    41,    41,    41,    41,
    41,    -6,    61,    -6,    41,    43,    45,    38,   124,    94,
    35,    -1,    -1,    -1,    -1,   -20,    -7,    42,    -5,   279,
   279,   126,    41,    -6,    -6,    42,    -6 };
yytabelem yydef[]={

     1,    -2,     2,     0,     4,     0,     0,     0,     0,    -2,
    -2,     9,    10,     0,    14,     0,     0,     0,     0,     0,
     0,    32,     0,    51,    92,    93,    94,   183,     0,     0,
     0,     0,     0,     0,     0,    68,     0,    98,    99,   100,
     0,     0,     0,   127,   128,     0,     0,     0,     0,   205,
   206,   207,   175,   176,   177,   178,   179,   180,     0,   123,
   124,   125,   126,     3,    -2,    -2,     5,     6,     7,     8,
    11,     0,     0,    12,     0,     0,    13,   181,   182,    15,
    16,    36,    38,    17,    39,    41,    18,    42,    44,    19,
    45,    47,    48,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    33,    35,    89,     0,   202,   203,   204,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    80,     0,    84,    85,   117,     0,   122,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
    50,     0,     0,     0,     0,   199,     0,   186,   187,   188,
   189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
   200,   201,     0,     0,     0,     0,     0,     0,    87,    88,
   184,    65,   167,   169,   171,    66,   168,   170,   172,    67,
   161,   173,   174,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   147,     0,   157,   158,
   159,   160,     0,   134,   150,   136,   153,   163,   164,    81,
    82,     0,     0,     0,     0,     0,    25,    29,    27,    31,
    37,    40,    43,    46,    20,    21,    22,    23,     0,    34,
    72,    54,    73,     0,     0,    74,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   108,   109,   110,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   143,     0,   162,    83,    86,   165,
   166,   118,   119,   120,   121,   185,     0,     0,    90,    91,
     0,     0,    77,    78,    79,    80,     0,    75,    76,    72,
    73,     0,     0,     0,    95,   129,     0,     0,    96,    97,
     0,   102,     0,     0,     0,     0,   107,   132,   148,   133,
   149,   135,     0,   152,   137,   154,   138,   155,   139,   140,
   141,   142,   144,   145,   156,   151,    70,    53,    69,    71,
    59,    70,     0,     0,    58,    -2,    -2,    -2,     0,    81,
    61,    62,    63,    64,     0,     0,     0,   111,     0,   116,
     0,     0,     0,     0,     0,   146,     0,     0,     0,     0,
   147,     0,     0,     0,     0,     0,     0,   103,   104,   105,
   106,    52,     0,    57,     0,     0,     0,     0,     0,     0,
     0,   112,   113,   114,   115,   101,     0,     0,     0,   130,
   131,     0,     0,    56,    60,     0,    55 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"PLSPLS",	257,
	"MINUSMINUS",	258,
	"REG",	259,
	"REGR",	260,
	"REGE",	261,
	"REGHL",	262,
	"REGIO",	263,
	"ACCUM",	264,
	"PC",	265,
	"PIOP",	266,
	"PIR",	267,
	"PCSH",	268,
	"DAUC",	269,
	"IOC",	270,
	"IBUF",	271,
	"OBUF",	272,
	"PDR",	273,
	"PCW",	274,
	"SYMBOL",	275,
	"LABEL",	276,
	"CONST",	277,
	"FCONST",	278,
	"CONST1",	279,
	"CONST2",	280,
	"ORG",	281,
	"END",	282,
	"ALIGN",	283,
	"BYTE",	284,
	"PAGE",	285,
	"EQU",	286,
	"LIST",	287,
	"INT",	288,
	"INT24",	289,
	"FLOAT",	290,
	"DA_FUN",	291,
	"IF",	292,
	"GOTO",	293,
	"CALL",	294,
	"RETURN",	295,
	"IRETURN",	296,
	"DO",	297,
	"NOP",	298,
	"CA_COND",	299,
	"DA_COND",	300,
	"IO_COND",	301,
	"=",	61,
	"(",	40,
	")",	41,
	"?",	63,
	":",	58,
	"|",	124,
	"^",	94,
	"&",	38,
	"EQUEQU",	302,
	"NOTEQU",	303,
	"<",	60,
	">",	62,
	"LE",	304,
	"GE",	305,
	"LSHIFT",	306,
	"RSHIFT",	307,
	"RROTATE",	308,
	"LROTATE",	309,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"!",	33,
	"~",	126,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"program : /* empty */",
	"program : program stmt",
	"program : program label stmt",
	"stmt : end_stmt",
	"stmt : psuedo_op end_stmt",
	"stmt : da_instr end_stmt",
	"stmt : ca_instr end_stmt",
	"stmt : error end_stmt",
	"end_stmt : ';'",
	"end_stmt : '\n'",
	"label : SYMBOL ':'",
	"label : LABEL ':'",
	"psuedo_op : ORG expr",
	"psuedo_op : END",
	"psuedo_op : ALIGN expr",
	"psuedo_op : BYTE byte_list",
	"psuedo_op : INT word_list",
	"psuedo_op : INT24 quad_list",
	"psuedo_op : FLOAT float_list",
	"psuedo_op : expr '*' BYTE expr",
	"psuedo_op : expr '*' INT expr",
	"psuedo_op : expr '*' INT24 expr",
	"psuedo_op : expr '*' FLOAT fconst",
	"psuedo_op : SYMBOL",
	"psuedo_op : SYMBOL '=' expr",
	"psuedo_op : LABEL",
	"psuedo_op : LABEL '=' expr",
	"psuedo_op : SYMBOL",
	"psuedo_op : SYMBOL EQU expr",
	"psuedo_op : LABEL",
	"psuedo_op : LABEL EQU expr",
	"psuedo_op : PAGE",
	"psuedo_op : PAGE expr",
	"psuedo_op : PAGE expr ',' expr",
	"psuedo_op : LIST expr",
	"byte_list : byte",
	"byte_list : byte_list ',' byte",
	"byte : expr",
	"word_list : word",
	"word_list : word_list ',' word",
	"word : expr",
	"quad_list : quad",
	"quad_list : quad_list ',' quad",
	"quad : expr",
	"float_list : float",
	"float_list : float_list ',' float",
	"float : fconst",
	"fconst : FCONST",
	"fconst : '-' FCONST",
	"fconst : '+' FCONST",
	"da_instr : da_dest",
	"da_instr : da_dest '=' f mem s ACCUM '*' x",
	"da_instr : da_dest '=' f mem s x",
	"da_instr : da_dest '=' f y",
	"da_instr : da_dest '=' f ACCUM s '(' z '=' y ')' '*' x",
	"da_instr : da_dest '=' f '(' z '=' y ')' '*' x",
	"da_instr : da_dest '=' f ACCUM s y '*' x",
	"da_instr : da_dest '=' f ACCUM s x",
	"da_instr : da_dest '=' f mem '*' x",
	"da_instr : da_dest '=' f '(' z '=' y ')' s x",
	"da_instr : da_dest '=' DA_FUN '(' y_spec ')'",
	"da_instr : da_dest '=' INT '(' y_spec ')'",
	"da_instr : da_dest '=' INT24 '(' y_spec ')'",
	"da_instr : da_dest '=' FLOAT '(' y_spec ')'",
	"da_dest : mem '=' ACCUM",
	"da_dest : mem_r '=' ACCUM",
	"da_dest : io_port '=' ACCUM",
	"da_dest : ACCUM",
	"x : mem",
	"x : ACCUM",
	"x : IBUF",
	"y : mem",
	"y : ACCUM",
	"y : IBUF",
	"y_spec : y",
	"y_spec : PDR",
	"z : mem",
	"z : mem_r",
	"z : io_port",
	"mem : '*' reg_e",
	"mem : '*' reg_e PLSPLS",
	"mem : '*' reg_e MINUSMINUS",
	"mem : '*' reg_e PLSPLS reg_e",
	"reg_e : REG",
	"reg_e : REGE",
	"mem_r : '*' reg_e PLSPLS REGR",
	"f : '+'",
	"f : '-'",
	"f : /* empty */",
	"s : '+'",
	"s : '-'",
	"ca_instr : control",
	"ca_instr : arith_logic",
	"ca_instr : data_move",
	"control : IF '(' CA_COND ')' goto16",
	"control : IF '(' DA_COND ')' goto16",
	"control : IF '(' IO_COND ')' goto16",
	"control : goto24",
	"control : IRETURN",
	"control : NOP",
	"control : IF '(' REG MINUSMINUS GE const ')' goto16",
	"control : CALL any_reg '(' any_reg ')'",
	"control : CALL any_reg '+' expr '(' any_reg ')'",
	"control : CALL any_reg '-' expr '(' any_reg ')'",
	"control : CALL PC '+' expr '(' any_reg ')'",
	"control : CALL PC '-' expr '(' any_reg ')'",
	"control : CALL expr '(' any_reg ')'",
	"control : DO expr ',' expr",
	"control : DO expr ',' REG",
	"control : RETURN '(' any_reg ')'",
	"goto16 : GOTO any_reg",
	"goto16 : GOTO any_reg '+' expr",
	"goto16 : GOTO any_reg '-' expr",
	"goto16 : GOTO PC '+' expr",
	"goto16 : GOTO PC '-' expr",
	"goto16 : GOTO expr",
	"goto24 : GOTO any_reg",
	"goto24 : GOTO any_reg '+' expr",
	"goto24 : GOTO any_reg '-' expr",
	"goto24 : GOTO PC '+' expr",
	"goto24 : GOTO PC '-' expr",
	"goto24 : GOTO expr",
	"any_reg : REG",
	"any_reg : REGE",
	"any_reg : REGHL",
	"any_reg : REGIO",
	"arith_logic : a_l_reg",
	"arith_logic : a_l_immed",
	"arith_logic : IF '(' CA_COND ')' a_l_reg",
	"arith_logic : IF '(' CA_COND ')' any_reg '=' any_reg '+' CONST1",
	"arith_logic : IF '(' CA_COND ')' any_reg '=' any_reg '-' CONST1",
	"a_l_reg : any_reg '=' any_reg '+' any_reg",
	"a_l_reg : any_reg '=' any_reg '-' any_reg",
	"a_l_reg : any_reg '-' any_reg",
	"a_l_reg : any_reg '=' any_reg '&' any_reg",
	"a_l_reg : any_reg '&' any_reg",
	"a_l_reg : any_reg '=' any_reg '|' any_reg",
	"a_l_reg : any_reg '=' any_reg '^' any_reg",
	"a_l_reg : any_reg '=' any_reg '/' CONST2",
	"a_l_reg : any_reg '=' any_reg RSHIFT CONST1",
	"a_l_reg : any_reg '=' any_reg RROTATE CONST1",
	"a_l_reg : any_reg '=' any_reg LROTATE CONST1",
	"a_l_reg : any_reg '=' '-' any_reg",
	"a_l_reg : any_reg '=' any_reg '*' CONST2",
	"a_l_reg : any_reg '=' any_reg '#' any_reg",
	"a_l_reg : any_reg '=' any_reg '&' '~' any_reg",
	"a_l_reg : any_reg '=' any_reg",
	"a_l_immed : any_reg '=' any_reg '+' expr",
	"a_l_immed : any_reg '=' any_reg '-' expr",
	"a_l_immed : any_reg '-' expr",
	"a_l_immed : any_reg '=' expr '-' any_reg",
	"a_l_immed : any_reg '=' any_reg '&' expr",
	"a_l_immed : any_reg '&' expr",
	"a_l_immed : any_reg '=' any_reg '|' expr",
	"a_l_immed : any_reg '=' any_reg '^' expr",
	"a_l_immed : any_reg '=' any_reg '#' expr",
	"data_move : any_reg '=' expr",
	"data_move : any_reg '=' mem",
	"data_move : any_reg '=' mem_r",
	"data_move : any_reg '=' io_port",
	"data_move : io_port '=' any_reg",
	"data_move : any_reg '=' '*' expr",
	"data_move : IOC '=' expr",
	"data_move : DAUC '=' expr",
	"data_move : '*' expr '=' any_reg",
	"data_move : '*' expr '=' PCSH",
	"data_move : mem '=' any_reg",
	"data_move : mem_r '=' any_reg",
	"data_move : mem '=' PCSH",
	"data_move : mem_r '=' PCSH",
	"data_move : mem '=' io_port",
	"data_move : mem_r '=' io_port",
	"data_move : io_port '=' mem",
	"data_move : io_port '=' mem_r",
	"io_port : IBUF",
	"io_port : OBUF",
	"io_port : PIOP",
	"io_port : PDR",
	"io_port : PIR",
	"io_port : PCW",
	"expr : SYMBOL",
	"expr : LABEL",
	"expr : const",
	"expr : '(' expr ')'",
	"expr : expr '?' expr ':' expr",
	"expr : expr '|' expr",
	"expr : expr '^' expr",
	"expr : expr '&' expr",
	"expr : expr EQUEQU expr",
	"expr : expr NOTEQU expr",
	"expr : expr '<' expr",
	"expr : expr '>' expr",
	"expr : expr LE expr",
	"expr : expr GE expr",
	"expr : expr LSHIFT expr",
	"expr : expr RSHIFT expr",
	"expr : expr '+' expr",
	"expr : expr '-' expr",
	"expr : expr '*' expr",
	"expr : expr '/' expr",
	"expr : expr '%' expr",
	"expr : '!' expr",
	"expr : '~' expr",
	"expr : '-' expr",
	"const : CONST",
	"const : CONST1",
	"const : CONST2",
};
#endif /* YYDEBUG */
# line	1 "/usr/ccs/bin/yaccpar"
/*
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */

#pragma ident	"@(#)yaccpar	6.12	93/06/07 SMI"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yynewmax * sizeof(type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside switch should never be
	executed; yypvt is set to 0 to avoid "used before set" warning.
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
	yypvt = 0;
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 2:
# line 233 "asm32.y"
{ end_stmt(); } break;
case 3:
# line 234 "asm32.y"
{ end_stmt(); } break;
case 8:
# line 241 "asm32.y"
{ yyerrok; errormsg("Syntax Error"); } break;
case 11:
# line 248 "asm32.y"
{ Symbol_def = Last_symbol; define_label(Org); } break;
case 12:
# line 249 "asm32.y"
{ Symbol_def = Last_symbol; define_label(Org); } break;
case 13:
# line 252 "asm32.y"
{ if(yypvt[-0].value >= 0L && yypvt[-0].value < MAX_DATA)
						Org = yypvt[-0].value;
					  else
						errormsg("Illegal Origin");
					  list_address(Org);
					} break;
case 14:
# line 258 "asm32.y"
{ list_address(Org); list_print();
					  if(Org != UNDEFINED) YYACCEPT;
					} break;
case 15:
# line 261 "asm32.y"
{ if(yypvt[-0].value > 1024L || yypvt[-0].value < 1L)
						errormsg("Illegal Alignment");
					  else
						while(Org % yypvt[-0].value)
							define_data(0L,1);
					} break;
case 20:
# line 271 "asm32.y"
{ define_array(yypvt[-3].value,yypvt[-0].value,1); } break;
case 21:
# line 272 "asm32.y"
{ define_array(yypvt[-3].value,yypvt[-0].value,2); } break;
case 22:
# line 273 "asm32.y"
{ define_array(yypvt[-3].value,yypvt[-0].value,4); } break;
case 23:
# line 274 "asm32.y"
{ define_array(yypvt[-3].value,yypvt[-0].value,4); } break;
case 24:
# line 275 "asm32.y"
{ Symbol_def = Last_symbol; } break;
case 25:
# line 276 "asm32.y"
{ define_symbol(yypvt[-0].value); } break;
case 26:
# line 277 "asm32.y"
{ Symbol_def = Last_symbol; } break;
case 27:
# line 278 "asm32.y"
{ error_symbol("Attempt to Re-Define Label",
								Symbol_def); } break;
case 28:
# line 280 "asm32.y"
{ Symbol_def = Last_symbol; } break;
case 29:
# line 281 "asm32.y"
{ define_label(yypvt[-0].value); } break;
case 30:
# line 282 "asm32.y"
{ Symbol_def = Last_symbol; } break;
case 31:
# line 283 "asm32.y"
{ define_label(yypvt[-0].value); } break;
case 32:
# line 284 "asm32.y"
{ list_page(1); } break;
case 33:
# line 285 "asm32.y"
{ Page_length = yypvt[-0].value; } break;
case 34:
# line 286 "asm32.y"
{ Page_length = yypvt[-2].value; Page_width = yypvt[-0].value; } break;
case 35:
# line 287 "asm32.y"
{ List_type = yypvt[-0].value; } break;
case 38:
# line 293 "asm32.y"
{ define_data(yypvt[-0].value,1); } break;
case 41:
# line 299 "asm32.y"
{ define_data(yypvt[-0].value,2); } break;
case 44:
# line 305 "asm32.y"
{ define_data(yypvt[-0].value,4); } break;
case 47:
# line 311 "asm32.y"
{ define_data(yypvt[-0].value,4); } break;
case 49:
# line 314 "asm32.y"
{ yyval.value = dspnegfloat(yypvt[-0].value); } break;
case 50:
# line 315 "asm32.y"
{ yyval.value = yypvt[-0].value; } break;
case 51:
# line 319 "asm32.y"
{ da_1(1,4,0,0,yypvt[-0].value,0,yypvt[-0].value&0x3L,7); } break;
case 52:
# line 321 "asm32.y"
{ da_1(1,yypvt[-2].value,yypvt[-5].value,yypvt[-3].value,yypvt[-7].value,yypvt[-0].value,yypvt[-4].value,7); } break;
case 53:
# line 323 "asm32.y"
{ da_1(1,5,yypvt[-3].value,yypvt[-1].value,yypvt[-5].value,yypvt[-0].value,yypvt[-2].value,7); } break;
case 54:
# line 325 "asm32.y"
{ da_1(1,4,yypvt[-1].value,0,yypvt[-3].value,0,yypvt[-0].value,7); } break;
case 55:
# line 327 "asm32.y"
{ da_1(2,yypvt[-8].value,yypvt[-9].value,yypvt[-7].value,yypvt[-11].value,yypvt[-0].value,yypvt[-3].value,yypvt[-5].value); } break;
case 56:
# line 329 "asm32.y"
{ da_1(2,4,0,yypvt[-7].value,yypvt[-9].value,yypvt[-0].value,yypvt[-3].value,yypvt[-5].value); } break;
case 57:
# line 331 "asm32.y"
{ da_1(3,yypvt[-4].value,yypvt[-5].value,yypvt[-3].value,yypvt[-7].value,yypvt[-0].value,yypvt[-2].value,7); } break;
case 58:
# line 333 "asm32.y"
{ da_1(1,5,yypvt[-3].value,yypvt[-1].value,yypvt[-5].value,yypvt[-0].value,yypvt[-2].value,7); } break;
case 59:
# line 335 "asm32.y"
{ da_1(3,4,0,yypvt[-3].value,yypvt[-5].value,yypvt[-0].value,yypvt[-2].value,7); } break;
case 60:
# line 337 "asm32.y"
{ da_1(1,6,yypvt[-7].value,yypvt[-1].value,yypvt[-9].value,yypvt[-0].value,yypvt[-3].value,yypvt[-5].value); } break;
case 61:
# line 339 "asm32.y"
{ da_5(yypvt[-3].value,yypvt[-5].value,yypvt[-1].value); } break;
case 62:
# line 341 "asm32.y"
{ da_5(yypvt[-3].value,yypvt[-5].value,yypvt[-1].value); } break;
case 63:
# line 343 "asm32.y"
{ da_5(yypvt[-3].value,yypvt[-5].value,yypvt[-1].value); } break;
case 64:
# line 345 "asm32.y"
{ da_5(yypvt[-3].value,yypvt[-5].value,yypvt[-1].value); } break;
case 65:
# line 348 "asm32.y"
{ yyval.value = (yypvt[-0].value<<21) | yypvt[-2].value; } break;
case 66:
# line 349 "asm32.y"
{ yyval.value = (yypvt[-0].value<<21) | yypvt[-2].value; } break;
case 67:
# line 350 "asm32.y"
{ yyval.value = (yypvt[-0].value<<21) | yypvt[-2].value; } break;
case 68:
# line 351 "asm32.y"
{ yyval.value = (yypvt[-0].value<<21) | 7; } break;
case 70:
# line 355 "asm32.y"
{yyval.value = mem_ref(0,yypvt[-0].value); } break;
case 71:
# line 356 "asm32.y"
{yyval.value = mem_ref(0,yypvt[-0].value); } break;
case 73:
# line 360 "asm32.y"
{yyval.value = mem_ref(0,yypvt[-0].value); } break;
case 74:
# line 361 "asm32.y"
{yyval.value = mem_ref(0,yypvt[-0].value); } break;
case 76:
# line 365 "asm32.y"
{yyval.value = mem_ref(0,yypvt[-0].value); } break;
case 80:
# line 373 "asm32.y"
{yyval.value = mem_ref(yypvt[-0].value,16); } break;
case 81:
# line 374 "asm32.y"
{yyval.value = mem_ref(yypvt[-1].value,23); } break;
case 82:
# line 375 "asm32.y"
{yyval.value = mem_ref(yypvt[-1].value,22); } break;
case 83:
# line 376 "asm32.y"
{yyval.value = mem_ref(yypvt[-2].value,yypvt[-0].value); } break;
case 86:
# line 383 "asm32.y"
{yyval.value = mem_ref(yypvt[-2].value,yypvt[-0].value); } break;
case 87:
# line 386 "asm32.y"
{yyval.value = 0L;} break;
case 88:
# line 387 "asm32.y"
{yyval.value = 1L;} break;
case 89:
# line 388 "asm32.y"
{yyval.value = 0L;} break;
case 90:
# line 391 "asm32.y"
{yyval.value = 0;} break;
case 91:
# line 392 "asm32.y"
{yyval.value = 1;} break;
case 95:
# line 400 "asm32.y"
{ ca_0(yypvt[-2].value,yypvt[-0].value); } break;
case 96:
# line 401 "asm32.y"
{ ca_0(yypvt[-2].value,yypvt[-0].value); } break;
case 97:
# line 402 "asm32.y"
{ ca_0(yypvt[-2].value,yypvt[-0].value); } break;
case 98:
# line 403 "asm32.y"
{ ; } break;
case 99:
# line 404 "asm32.y"
{ define_data(0x003E0000L,4); } break;
case 100:
# line 405 "asm32.y"
{ define_data(0L,4); } break;
case 101:
# line 406 "asm32.y"
{ ca_3a(yypvt[-5].value,yypvt[-0].value); } break;
case 102:
# line 407 "asm32.y"
{ ca_4(yypvt[-1].value,yypvt[-3].value,0L); } break;
case 103:
# line 408 "asm32.y"
{ ca_4(yypvt[-1].value,yypvt[-5].value,yypvt[-3].value); } break;
case 104:
# line 409 "asm32.y"
{ ca_4(yypvt[-1].value,yypvt[-5].value,-yypvt[-3].value); } break;
case 105:
# line 410 "asm32.y"
{ ca_4(yypvt[-1].value,yypvt[-5].value,yypvt[-3].value); } break;
case 106:
# line 411 "asm32.y"
{ ca_4(yypvt[-1].value,yypvt[-5].value,-yypvt[-3].value); } break;
case 107:
# line 412 "asm32.y"
{ ca_8(7,yypvt[-1].value,yypvt[-3].value); } break;
case 108:
# line 413 "asm32.y"
{define_data(0x8C000000L|(yypvt[-2].value<<16)|(yypvt[-0].value&0x7FFL),4); } break;
case 109:
# line 414 "asm32.y"
{define_data(0x8C200000L|(yypvt[-2].value<<16)|(yypvt[-0].value&0x1FL),4); } break;
case 110:
# line 415 "asm32.y"
{ define_data(goto_dest(yypvt[-1].value,0L),4); } break;
case 111:
# line 418 "asm32.y"
{ yyval.value = goto_dest(yypvt[-0].value,0L); } break;
case 112:
# line 419 "asm32.y"
{ yyval.value = goto_dest(yypvt[-2].value,yypvt[-0].value); } break;
case 113:
# line 420 "asm32.y"
{ yyval.value = goto_dest(yypvt[-2].value,-yypvt[-0].value); } break;
case 114:
# line 421 "asm32.y"
{ yyval.value = goto_dest(yypvt[-2].value,yypvt[-0].value); } break;
case 115:
# line 422 "asm32.y"
{ yyval.value = goto_dest(yypvt[-2].value,-yypvt[-0].value); } break;
case 116:
# line 423 "asm32.y"
{ yyval.value = goto_dest(0,yypvt[-0].value); } break;
case 117:
# line 426 "asm32.y"
{ ca_8(5,yypvt[-0].value,0L); } break;
case 118:
# line 427 "asm32.y"
{ ca_8(5,yypvt[-2].value,yypvt[-0].value); } break;
case 119:
# line 428 "asm32.y"
{ ca_8(5,yypvt[-2].value,-yypvt[-0].value); } break;
case 120:
# line 429 "asm32.y"
{ ca_8(5,yypvt[-2].value,yypvt[-0].value); } break;
case 121:
# line 430 "asm32.y"
{ ca_8(5,yypvt[-2].value,-yypvt[-0].value); } break;
case 122:
# line 431 "asm32.y"
{ ca_8(5,0,yypvt[-0].value); } break;
case 127:
# line 440 "asm32.y"
{ define_data(yypvt[-0].value,4); } break;
case 128:
# line 441 "asm32.y"
{ ; } break;
case 129:
# line 442 "asm32.y"
{ define_data(yypvt[-0].value|(yypvt[-2].value<<12)|0x400L,4); } break;
case 130:
# line 444 "asm32.y"
{ long v = ca_6a(0,yypvt[-4].value,23,yypvt[-2].value);
						  define_data(v|(yypvt[-6].value<<12)|0x400L,4); } break;
case 131:
# line 447 "asm32.y"
{ long v = ca_6a(0,yypvt[-4].value,22,yypvt[-2].value);
						  define_data(v|(yypvt[-6].value<<12)|0x400L,4); } break;
case 132:
# line 452 "asm32.y"
{ yyval.value = ca_6a(0,yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 133:
# line 454 "asm32.y"
{ yyval.value = ca_6a(2,yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 134:
# line 456 "asm32.y"
{ yyval.value = ca_6a(7,yypvt[-2].value,yypvt[-0].value,0); } break;
case 135:
# line 458 "asm32.y"
{ yyval.value = ca_6a(14,yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 136:
# line 460 "asm32.y"
{ yyval.value = ca_6a(15,yypvt[-2].value,yypvt[-0].value,0); } break;
case 137:
# line 462 "asm32.y"
{ yyval.value = ca_6a(10,yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 138:
# line 464 "asm32.y"
{ yyval.value = ca_6a(8,yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 139:
# line 466 "asm32.y"
{ yyval.value = ca_6a(13,yypvt[-4].value,yypvt[-2].value,0); } break;
case 140:
# line 468 "asm32.y"
{ yyval.value = ca_6a(12,yypvt[-4].value,yypvt[-2].value,0); } break;
case 141:
# line 470 "asm32.y"
{ yyval.value = ca_6a(9,yypvt[-4].value,yypvt[-2].value,0); } break;
case 142:
# line 472 "asm32.y"
{ yyval.value = ca_6a(11,yypvt[-4].value,yypvt[-2].value,0); } break;
case 143:
# line 474 "asm32.y"
{ yyval.value = ca_6a(5,yypvt[-3].value,yypvt[-0].value,0); } break;
case 144:
# line 476 "asm32.y"
{ yyval.value = ca_6a(1,yypvt[-4].value,yypvt[-2].value,0); } break;
case 145:
# line 478 "asm32.y"
{ yyval.value = ca_6a(3,yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 146:
# line 480 "asm32.y"
{ yyval.value = ca_6a(6,yypvt[-5].value,yypvt[-3].value,yypvt[-0].value); } break;
case 147:
# line 482 "asm32.y"
{ yyval.value = ca_6a(10,yypvt[-2].value,yypvt[-0].value,yypvt[-0].value); } break;
case 148:
# line 485 "asm32.y"
{ ca_5(yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 149:
# line 486 "asm32.y"
{ ca_5(yypvt[-4].value,yypvt[-2].value,-yypvt[-0].value); } break;
case 150:
# line 487 "asm32.y"
{ ca_6c(7,yypvt[-2].value,yypvt[-0].value,0L); } break;
case 151:
# line 488 "asm32.y"
{ ca_6c(2,yypvt[-4].value,yypvt[-2].value,yypvt[-0].value); } break;
case 152:
# line 489 "asm32.y"
{ ca_6c(14,yypvt[-4].value,yypvt[-0].value,yypvt[-2].value); } break;
case 153:
# line 490 "asm32.y"
{ ca_6c(15,yypvt[-2].value,yypvt[-0].value,0L); } break;
case 154:
# line 491 "asm32.y"
{ ca_6c(10,yypvt[-4].value,yypvt[-0].value,yypvt[-2].value); } break;
case 155:
# line 492 "asm32.y"
{ ca_6c(8,yypvt[-4].value,yypvt[-0].value,yypvt[-2].value); } break;
case 156:
# line 493 "asm32.y"
{ ca_6c(3,yypvt[-4].value,yypvt[-0].value,yypvt[-2].value); } break;
case 157:
# line 496 "asm32.y"
{ if((yypvt[-2].value&EXT) == 0) ca_5(yypvt[-2].value,0,yypvt[-0].value);
						else ca_8(6,yypvt[-2].value,yypvt[-0].value); } break;
case 158:
# line 498 "asm32.y"
{ ca_7b(0,yypvt[-2].value,yypvt[-0].value); } break;
case 159:
# line 499 "asm32.y"
{ ca_7b(0,yypvt[-2].value,yypvt[-0].value); } break;
case 160:
# line 500 "asm32.y"
{ ca_7b(0,yypvt[-2].value,yypvt[-0].value); } break;
case 161:
# line 501 "asm32.y"
{ ca_7b(1,yypvt[-0].value,yypvt[-2].value); } break;
case 162:
# line 502 "asm32.y"
{ ca_7a(0,yypvt[-3].value,yypvt[-0].value); } break;
case 163:
# line 503 "asm32.y"
{ define_data(0x17600000L|(yypvt[-0].value&0x1FFFFFL),4); } break;
case 164:
# line 504 "asm32.y"
{ define_data(0x17400000L|((yypvt[-0].value&0x1FL)<<16),4); } break;
case 165:
# line 505 "asm32.y"
{ ca_7a(1,yypvt[-0].value,yypvt[-2].value); } break;
case 166:
# line 506 "asm32.y"
{ ca_7a(1,yypvt[-0].value,yypvt[-2].value); } break;
case 167:
# line 507 "asm32.y"
{ ca_7b(1,yypvt[-0].value,yypvt[-2].value); } break;
case 168:
# line 508 "asm32.y"
{ ca_7b(1,yypvt[-0].value,yypvt[-2].value); } break;
case 169:
# line 509 "asm32.y"
{ ca_7b(1,yypvt[-0].value,yypvt[-2].value); } break;
case 170:
# line 510 "asm32.y"
{ ca_7b(1,yypvt[-0].value,yypvt[-2].value); } break;
case 171:
# line 511 "asm32.y"
{ ca_7d(1,yypvt[-0].value,yypvt[-2].value); } break;
case 172:
# line 512 "asm32.y"
{ ca_7d(1,yypvt[-0].value,yypvt[-2].value); } break;
case 173:
# line 513 "asm32.y"
{ ca_7d(0,yypvt[-2].value,yypvt[-0].value); } break;
case 174:
# line 514 "asm32.y"
{ ca_7d(0,yypvt[-2].value,yypvt[-0].value); } break;
case 181:
# line 525 "asm32.y"
{ if(yypvt[-0].value == UNDEFINED) warn_symbol("Undefined Symbol",Last_symbol); } break;
case 182:
# line 526 "asm32.y"
{ if(yypvt[-0].value == UNDEFINED) error_symbol("Undefined Label",Last_symbol); } break;
case 184:
# line 528 "asm32.y"
{ yyval.value = yypvt[-1].value; } break;
case 185:
# line 529 "asm32.y"
{ yyval.value = (yypvt[-4].value ? yypvt[-2].value : yypvt[-0].value); if(yypvt[-4].value==UNDEFINED) yyval.value = UNDEFINED; } break;
case 186:
# line 530 "asm32.y"
{ yyval.value = yypvt[-2].value | yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 187:
# line 531 "asm32.y"
{ yyval.value = yypvt[-2].value ^ yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 188:
# line 532 "asm32.y"
{ yyval.value = yypvt[-2].value & yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 189:
# line 533 "asm32.y"
{ yyval.value = yypvt[-2].value == yypvt[-0].value;   if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 190:
# line 534 "asm32.y"
{ yyval.value = yypvt[-2].value != yypvt[-0].value;   if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 191:
# line 535 "asm32.y"
{ yyval.value = yypvt[-2].value < yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 192:
# line 536 "asm32.y"
{ yyval.value = yypvt[-2].value > yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 193:
# line 537 "asm32.y"
{ yyval.value = yypvt[-2].value <= yypvt[-0].value;   if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 194:
# line 538 "asm32.y"
{ yyval.value = yypvt[-2].value >= yypvt[-0].value;   if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 195:
# line 539 "asm32.y"
{ yyval.value = yypvt[-2].value << yypvt[-0].value;   if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 196:
# line 540 "asm32.y"
{ yyval.value = yypvt[-2].value >> yypvt[-0].value;   if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 197:
# line 541 "asm32.y"
{ yyval.value = yypvt[-2].value + yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 198:
# line 542 "asm32.y"
{ yyval.value = yypvt[-2].value - yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 199:
# line 543 "asm32.y"
{ yyval.value = yypvt[-2].value * yypvt[-0].value;    if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 200:
# line 544 "asm32.y"
{ if(yypvt[-0].value != 0L) yyval.value = yypvt[-2].value / yypvt[-0].value;
					  else yyval.value = UNDEFINED;
					  if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 201:
# line 547 "asm32.y"
{ if(yypvt[-0].value != 0L) yyval.value = yypvt[-2].value % yypvt[-0].value;
					  else yyval.value = UNDEFINED;
					  if(yypvt[-2].value == UNDEFINED || yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 202:
# line 550 "asm32.y"
{ yyval.value = ! yypvt[-0].value;       if(yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 203:
# line 551 "asm32.y"
{ yyval.value = ~ yypvt[-0].value;       if(yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
case 204:
# line 552 "asm32.y"
{ yyval.value = - yypvt[-0].value;       if(yypvt[-0].value == UNDEFINED) yyval.value = UNDEFINED; } break;
# line	532 "/usr/ccs/bin/yaccpar"
	}
	goto yystack;		/* reset registers in driver code */
}

