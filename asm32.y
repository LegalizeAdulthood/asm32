/*	asm32.y - DSP32C assembler

	USAGE:
		asm32 [-d] file.s [infile]

	Assembles file.s, producing file.dsp and file.lst.
	file.dsp is a direct image of the program, in byte sequence
	for the DSP32C, starting at address 0. file.lst is the listing.
	Output files are in the current directory, regardless of where the
	source file is located.

	If infile is present, it is used instead of file.s for input -
	file.s is used for titles, .dsp and .lst filenames, etc. This
	is useful when cpp is used before asm32 - the #line directives
	from cpp keep error messages synchronized to the cpp input
	file.s, not to the intermediate temporary file infile. Use the
	"-c" flag to cpp to keep comments in file.lst (cpp directives
	and #line directives WILL NOT appear in file.lst).

	If "-d" is present, the output file will be file.dat, and is
	a comma-separated series of bytes, suitable for inclusion into
	a C program.
	
	All DSP32C instructions are supported. Note that for ambiguous
	instructions (e.g. a0 = a1;), a particular choice was chosen
	(by selection of grammar rules implemented).

	Enhancement: 24-bit register names can be used anywhere. In
	particular, *r1e++ = r2e is OK (the AT&T assembler will only
	accept *r1++ = r2e for this instruction). This makes it easy
	to #define real names for registers, and use them throughout
	your code.
	
	The following psuedo-ops are implemented:

		org	expr		sets the current memory origin
		end			ends the assembly (EOF also works)
		align	expr		align to (expr) bytes
		byte	expr,expr, ...	define data bytes
		char	expr,expr, ...		"
		int	expr,expr, ...	define 2-byte words
		word	expr,expr, ...		"
		int24	expr,expr, ...	define 4-byte data
		long	expr,expr, ...		"
		float	fconst,fconst.. define floating-point constants
		N*byte	expr		define N bytes (N is an expr)
		... same for char, int, word, int24, long, float
		symbol = expr		define a SYMBOL
		label equ expr		define a label (also label:)
		page			eject a page in listing
		page	expr		define page length
		page	expr,expr	define page length & width
		list	expr		set listing type (default=3):
						1 = main file
						2 = generated code
						4 = include files

	Comments are as in C++.

	Labels are ABSOLUTE, and are indicated by "name:" before a
	(possibly empty) statement.

	Statements are terminated by ; and \n; empty statements are
	allowed.

	Symbols can be re-defined; labels cannot. Both are kept as 32-bit
	long integers.

	expr is an integer expression, using C operators, labels,
	symbols, or integer constants. Integer constants are as in
	C ([+-]ddd, 0xddd, 0ddd), but character ('X') and string ("X")
	constants are NOT allowed.

	fconst is a floating-point constant ([+-]ddd.ddd[E[+-]ddd]),
	where the initial digit and the '.' MUST be present.

	"#line 123 filename" is also recognized (# in col 1), and sets
	Line_number and Filename.


	Uses yacc (ported to a PC) and Turbo C++ ver 1.01 (in C mode).
	Has 0 Shift/Reduce conflicts, and 0 Reduce/Reduce conflicts (Hurrah!).


	Written by Tom Roberts, 1/3/91. Listings fixed 1/15/92  TJR.
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


%union
{
	long value;
}

%token <value>	PLSPLS MINUSMINUS		/* ++, -- */
%token <value>	REG				/* rX  (X=1..22) */
%token <value>	REGR				/* rXr */
%token <value>	REGE				/* rXe */
%token <value>	REGHL				/* rXl, rXh */
%token <value>	REGIO				/* pir, pdr, ... */
%token <value>	ACCUM PC PIOP PIR		/* special registers */
%token <value>	PCSH DAUC IOC IBUF OBUF PDR PCW	/* special registers */
%token <value>	SYMBOL LABEL CONST FCONST	/* literal data values */
%token <value>	CONST1 CONST2			/* special constants (1,2) */
%token <value>	ORG END ALIGN BYTE PAGE EQU LIST/* psuedo-ops */
%token <value>	INT INT24 FLOAT                 /* psuedo_ops AND da-funs */
%token <value>	DA_FUN				/* DA special functions */
%token <value>	IF GOTO CALL RETURN IRETURN DO	/* CA special functions */
%token <value>	NOP CA_COND DA_COND IO_COND	/* CA special functions */
%token <value>	'=' '(' ')'			/* tokens which need type */

%type <value>	expr fconst const
%type <value>	f s x y y_spec z da_dest mem reg_e mem_r
%type <value>	ca_instr control arith_logic data_move a_l_reg a_l_immed
%type <value>	goto16 goto24 any_reg io_port

/*	arithmetic operators for expr */
%right <value> '?' ':'
%left <value> '|'
%left <value> '^'
%left <value> '&'
%left <value> EQUEQU
%left <value> NOTEQU
%left <value> '<' '>' LE GE
%left <value> LSHIFT RSHIFT RROTATE LROTATE
%left <value> '+' '-'
%left <value> '*' '/' '%'
%right <value> '!' '~'

%start program

%{
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
%}

%%

program	:	/* EMPTY */
	|	program stmt		{ end_stmt(); }
	|	program label stmt	{ end_stmt(); }
	;

stmt	:	end_stmt
	|	psuedo_op end_stmt
	|	da_instr end_stmt
	|	ca_instr end_stmt
	|	error end_stmt 		{ yyerrok; errormsg("Syntax Error"); }
	;

end_stmt :	';'
	|	'\n'
	;

label	:	SYMBOL ':'      { Symbol_def = Last_symbol; define_label(Org); }
	|	LABEL ':'       { Symbol_def = Last_symbol; define_label(Org); }
	;

psuedo_op :	ORG expr 		{ if($2 >= 0L && $2 < MAX_DATA)
						Org = $2;
					  else
						errormsg("Illegal Origin");
					  list_address(Org);
					}
	|	END			{ list_address(Org); list_print();
					  if(Org != UNDEFINED) YYACCEPT;
					}
	|	ALIGN expr		{ if($2 > 1024L || $2 < 1L)
						errormsg("Illegal Alignment");
					  else
						while(Org % $2)
							define_data(0L,1);
					}
	|	BYTE byte_list
	|	INT word_list
	|	INT24 quad_list
	|	FLOAT float_list
	|	expr '*' BYTE expr	{ define_array($1,$4,1); }
	|	expr '*' INT expr	{ define_array($1,$4,2); }
	|	expr '*' INT24 expr	{ define_array($1,$4,4); }
	|	expr '*' FLOAT fconst	{ define_array($1,$4,4); }
	|	SYMBOL { Symbol_def = Last_symbol; } '=' expr
					{ define_symbol($4); }
	|	LABEL { Symbol_def = Last_symbol; } '=' expr
					{ error_symbol("Attempt to Re-Define Label",
								Symbol_def); }
	|	SYMBOL { Symbol_def = Last_symbol; } EQU expr
					{ define_label($4); }
	|	LABEL { Symbol_def = Last_symbol; } EQU expr
					{ define_label($4); }
	|	PAGE			{ list_page(1); }
	|	PAGE expr		{ Page_length = $2; }
	|	PAGE expr ',' expr	{ Page_length = $2; Page_width = $4; }
	|	LIST expr		{ List_type = $2; }
	;

byte_list :	byte
	|	byte_list ',' byte
	;
byte	:	expr 			{ define_data($1,1); }
	;

word_list :	word
	|	word_list ',' word
	;
word	:	expr 			{ define_data($1,2); }
	;

quad_list :	quad
	|	quad_list ',' quad
	;
quad	:	expr 			{ define_data($1,4); }
	;

float_list :	float
	|	float_list ',' float
	;
float	:	fconst			{ define_data($1,4); }
	;
fconst	:	FCONST
	|	'-' FCONST		{ $$ = dspnegfloat($2); }
	|	'+' FCONST		{ $$ = $2; }
	;

da_instr :      da_dest
					{ da_1(1,4,0,0,$1,0,$1&0x3L,7); }
	|	da_dest '=' f mem s ACCUM '*' x
					{ da_1(1,$6,$3,$5,$1,$8,$4,7); }
	|	da_dest '=' f mem s x
					{ da_1(1,5,$3,$5,$1,$6,$4,7); }
	|	da_dest '=' f y
					{ da_1(1,4,$3,0,$1,0,$4,7); }
	|       da_dest '=' f ACCUM s '(' z '=' y ')' '*' x
					{ da_1(2,$4,$3,$5,$1,$12,$9,$7); }
	|	da_dest '=' f '(' z '=' y ')' '*' x
					{ da_1(2,4,0,$3,$1,$10,$7,$5); }
	|	da_dest '=' f ACCUM s y '*' x
					{ da_1(3,$4,$3,$5,$1,$8,$6,7); }
	|	da_dest '=' f ACCUM s x
					{ da_1(1,5,$3,$5,$1,$6,$4,7); }
	|	da_dest '=' f mem '*' x
					{ da_1(3,4,0,$3,$1,$6,$4,7); }
	|	da_dest '=' f '(' z '=' y ')' s x
					{ da_1(1,6,$3,$9,$1,$10,$7,$5); }
	|	da_dest '=' DA_FUN '(' y_spec ')'
					{ da_5($3,$1,$5); }
	|	da_dest '=' INT '(' y_spec ')'
					{ da_5($3,$1,$5); }
	|	da_dest '=' INT24 '(' y_spec ')'
					{ da_5($3,$1,$5); }
	|	da_dest '=' FLOAT '(' y_spec ')'
					{ da_5($3,$1,$5); }
	;

da_dest	:	mem '=' ACCUM		{ $$ = ($3<<21) | $1; }
	|	mem_r '=' ACCUM		{ $$ = ($3<<21) | $1; }
	|	io_port '=' ACCUM	{ $$ = ($3<<21) | $1; }
	|	ACCUM			{ $$ = ($1<<21) | 7; }
	;

x       :	mem
	|	ACCUM			{$$ = mem_ref(0,$1); }
	|	IBUF			{$$ = mem_ref(0,$1); }
	;

y	:	mem
	|	ACCUM			{$$ = mem_ref(0,$1); }
	|	IBUF			{$$ = mem_ref(0,$1); }
	;

y_spec	:	y
	|	PDR			{$$ = mem_ref(0,$1); }
	;

z	:	mem
	|	mem_r
	|	io_port           /* OBUF,PDR only (enforced in z_ref()) */
	;

mem	:	'*' reg_e 		{$$ = mem_ref($2,16); }
	|	'*' reg_e PLSPLS	{$$ = mem_ref($2,23); }
	|	'*' reg_e MINUSMINUS	{$$ = mem_ref($2,22); }
	|	'*' reg_e PLSPLS reg_e	{$$ = mem_ref($2,$4); }
	;

reg_e	:	REG
	|	REGE
	;

mem_r	:	'*' reg_e PLSPLS REGR	{$$ = mem_ref($2,$4); }
	;

f	:	'+'				{$$ = 0L;}
	|	'-'				{$$ = 1L;}
	|	/* EMPTY */			{$$ = 0L;}
	;

s	:	'+'				{$$ = 0;}
	|	'-'				{$$ = 1;}
	;

ca_instr :	control
	|	arith_logic
	|	data_move
	;

control	:	IF '(' CA_COND ')' goto16		{ ca_0($3,$5); }
	|	IF '(' DA_COND ')' goto16		{ ca_0($3,$5); }
	|	IF '(' IO_COND ')' goto16		{ ca_0($3,$5); }
	|	goto24					{ ; }
	|	IRETURN					{ define_data(0x003E0000L,4); }
	|	NOP                     		{ define_data(0L,4); }
	|	IF '(' REG MINUSMINUS GE const ')' goto16 { ca_3a($3,$8); }
	|	CALL any_reg '(' any_reg ')'   		{ ca_4($4,$2,0L); }
	|	CALL any_reg '+' expr '(' any_reg ')'	{ ca_4($6,$2,$4); }
	|	CALL any_reg '-' expr '(' any_reg ')'	{ ca_4($6,$2,-$4); }
	|	CALL PC '+' expr '(' any_reg ')'	{ ca_4($6,$2,$4); }
	|	CALL PC '-' expr '(' any_reg ')'	{ ca_4($6,$2,-$4); }
	|	CALL expr '(' any_reg ')'		{ ca_8(7,$4,$2); }
	|	DO expr ',' expr  {define_data(0x8C000000L|($2<<16)|($4&0x7FFL),4); }
	|	DO expr ',' REG   {define_data(0x8C200000L|($2<<16)|($4&0x1FL),4); }
	|	RETURN '(' any_reg ')'	{ define_data(goto_dest($3,0L),4); }
	;

goto16	:	GOTO any_reg		{ $$ = goto_dest($2,0L); }
	|	GOTO any_reg '+' expr	{ $$ = goto_dest($2,$4); }
	|	GOTO any_reg '-' expr	{ $$ = goto_dest($2,-$4); }
	|	GOTO PC '+' expr 	{ $$ = goto_dest($2,$4); }
	|	GOTO PC '-' expr 	{ $$ = goto_dest($2,-$4); }
	|	GOTO expr		{ $$ = goto_dest(0,$2); }
	;

goto24	:	GOTO any_reg		{ ca_8(5,$2,0L); }
	|	GOTO any_reg '+' expr	{ ca_8(5,$2,$4); }
	|	GOTO any_reg '-' expr	{ ca_8(5,$2,-$4); }
	|	GOTO PC '+' expr 	{ ca_8(5,$2,$4); }
	|	GOTO PC '-' expr 	{ ca_8(5,$2,-$4); }
	|	GOTO expr		{ ca_8(5,0,$2); }
	;

any_reg	:	REG
	|	REGE
	|	REGHL
	|	REGIO
	;

arith_logic :	a_l_reg				{ define_data($1,4); }
	|	a_l_immed                       { ; }
	|	IF '(' CA_COND ')' a_l_reg      { define_data($5|($3<<12)|0x400L,4); }
	|	IF '(' CA_COND ')' any_reg '=' any_reg '+' CONST1
						{ long v = ca_6a(0,$5,23,$7);
						  define_data(v|($3<<12)|0x400L,4); }
	|	IF '(' CA_COND ')' any_reg '=' any_reg '-' CONST1
						{ long v = ca_6a(0,$5,22,$7);
						  define_data(v|($3<<12)|0x400L,4); }
	;

a_l_reg	:	any_reg '=' any_reg '+' any_reg
						{ $$ = ca_6a(0,$1,$3,$5); }
	|	any_reg '=' any_reg '-' any_reg
						{ $$ = ca_6a(2,$1,$3,$5); }
	|	any_reg '-' any_reg
						{ $$ = ca_6a(7,$1,$3,0); }
	|	any_reg '=' any_reg '&' any_reg
						{ $$ = ca_6a(14,$1,$3,$5); }
	|	any_reg '&' any_reg
						{ $$ = ca_6a(15,$1,$3,0); }
	|	any_reg '=' any_reg '|' any_reg
						{ $$ = ca_6a(10,$1,$3,$5); }
	|	any_reg '=' any_reg '^' any_reg
						{ $$ = ca_6a(8,$1,$3,$5); }
	|	any_reg '=' any_reg '/' CONST2
						{ $$ = ca_6a(13,$1,$3,0); }
	|	any_reg '=' any_reg RSHIFT CONST1
						{ $$ = ca_6a(12,$1,$3,0); }
	|       any_reg '=' any_reg RROTATE CONST1
						{ $$ = ca_6a(9,$1,$3,0); }
	|	any_reg '=' any_reg LROTATE CONST1
						{ $$ = ca_6a(11,$1,$3,0); }
	|	any_reg '=' '-' any_reg
						{ $$ = ca_6a(5,$1,$4,0); }
	|	any_reg '=' any_reg '*' CONST2
						{ $$ = ca_6a(1,$1,$3,0); }
	|	any_reg '=' any_reg '#' any_reg
						{ $$ = ca_6a(3,$1,$3,$5); }
	|	any_reg '=' any_reg '&' '~' any_reg
						{ $$ = ca_6a(6,$1,$3,$6); }
	|	any_reg '=' any_reg
						{ $$ = ca_6a(10,$1,$3,$3); }
	;

a_l_immed :	any_reg '=' any_reg '+' expr	{ ca_5($1,$3,$5); }
	|	any_reg '=' any_reg '-' expr	{ ca_5($1,$3,-$5); }
	|	any_reg '-' expr		{ ca_6c(7,$1,$3,0L); }
	|	any_reg '=' expr '-' any_reg	{ ca_6c(2,$1,$3,$5); }
	|	any_reg '=' any_reg '&' expr	{ ca_6c(14,$1,$5,$3); }
	|	any_reg '&' expr		{ ca_6c(15,$1,$3,0L); }
	|	any_reg '=' any_reg '|' expr	{ ca_6c(10,$1,$5,$3); }
	|	any_reg '=' any_reg '^' expr	{ ca_6c(8,$1,$5,$3); }
	|       any_reg '=' any_reg '#' expr	{ ca_6c(3,$1,$5,$3); }
	;

data_move :	any_reg '=' expr	{ if(($1&EXT) == 0) ca_5($1,0,$3);
						else ca_8(6,$1,$3); }
	|	any_reg '=' mem		{ ca_7b(0,$1,$3); }
	|	any_reg '=' mem_r       { ca_7b(0,$1,$3); }
	|	any_reg '=' io_port	{ ca_7b(0,$1,$3); }
	|	io_port '=' any_reg	{ ca_7b(1,$3,$1); }
	|	any_reg '=' '*' expr	{ ca_7a(0,$1,$4); }
	|	IOC '=' expr            { define_data(0x17600000L|($3&0x1FFFFFL),4); }
	|	DAUC '=' expr		{ define_data(0x17400000L|(($3&0x1FL)<<16),4); }
	|	'*' expr '=' any_reg	{ ca_7a(1,$4,$2); }
	|	'*' expr '=' PCSH	{ ca_7a(1,$4,$2); }
	|       mem '=' any_reg		{ ca_7b(1,$3,$1); }
	|       mem_r '=' any_reg	{ ca_7b(1,$3,$1); }
	|	mem '=' PCSH		{ ca_7b(1,$3,$1); }
	|	mem_r '=' PCSH		{ ca_7b(1,$3,$1); }
	|	mem '=' io_port		{ ca_7d(1,$3,$1); }
	|	mem_r '=' io_port	{ ca_7d(1,$3,$1); }
	|	io_port '=' mem		{ ca_7d(0,$1,$3); }
	|	io_port '=' mem_r	{ ca_7d(0,$1,$3); }
	;

io_port	:	IBUF
	|	OBUF
	|	PIOP
	|	PDR
	|	PIR
	|	PCW
	;

expr	:	SYMBOL	{ if($1 == UNDEFINED) warn_symbol("Undefined Symbol",Last_symbol); }
	|	LABEL 	{ if($1 == UNDEFINED) error_symbol("Undefined Label",Last_symbol); }
	|	const
	|	'(' expr ')'  		{ $$ = $2; }
	|	expr '?' expr ':' expr	{ $$ = ($1 ? $3 : $5); if($1==UNDEFINED) $$ = UNDEFINED; }
	|	expr '|' expr 		{ $$ = $1 | $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '^' expr 		{ $$ = $1 ^ $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '&' expr 		{ $$ = $1 & $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr EQUEQU expr 	{ $$ = $1 == $3;   if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr NOTEQU expr 	{ $$ = $1 != $3;   if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '<' expr 		{ $$ = $1 < $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '>' expr 		{ $$ = $1 > $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr LE expr 		{ $$ = $1 <= $3;   if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr GE expr 		{ $$ = $1 >= $3;   if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr LSHIFT expr 	{ $$ = $1 << $3;   if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr RSHIFT expr 	{ $$ = $1 >> $3;   if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '+' expr 		{ $$ = $1 + $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '-' expr 		{ $$ = $1 - $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '*' expr 		{ $$ = $1 * $3;    if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '/' expr 		{ if($3 != 0L) $$ = $1 / $3;
					  else $$ = UNDEFINED;
					  if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	expr '%' expr 		{ if($3 != 0L) $$ = $1 % $3;
					  else $$ = UNDEFINED;
					  if($1 == UNDEFINED || $3 == UNDEFINED) $$ = UNDEFINED; }
	|	'!' expr      		{ $$ = ! $2;       if($2 == UNDEFINED) $$ = UNDEFINED; }
	|	'~' expr      		{ $$ = ~ $2;       if($2 == UNDEFINED) $$ = UNDEFINED; }
	|	'-' expr %prec '!' 	{ $$ = - $2;       if($2 == UNDEFINED) $$ = UNDEFINED; }
	;

const	:	CONST
	|	CONST1
	|	CONST2
	;

%%

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
