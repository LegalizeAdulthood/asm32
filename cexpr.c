/*	cexpr.c - constant-expression parser	*/

/*
	cexpr(string,symb) will evaluate any valid expression containing
	constants and the C-language arithmetic operators. The expression
	is terminated by ',', ';', '\n', or '\0'.
	Character constants ('C') are permitted.
	The value of the expression is returned in value, cexpr() returns
	0 for success, ERR for errors, UNDEF for un-defined symbols.
	Square brackets ('[', ']') are identical to parentheses ('(', ')').
	symb=0 indicates symbols are not allowed (used by the pre-processor),
	symb!=0 indicates symbols are allowed.

	Interface:
		The following extern-s are used:

			long value;			all values passed here
			int sym_val(string,&ptr)	gets symbol values
			int isident(char)		test for legal chars
							in identifier
		The follwing extern-s are defined:

			char *expr_end;			set to point to the
							char that
							ended the expression
		


	Known Bug: the ternary conditional operator (a?b:c) CANNOT be
	nested without parentheses. This is an inherent limitation
	of the parsing method.

	Token		Value	Precedence	Operation
	-----		-----	----------	---------
	( [		28	28		parenthesis
	!		27	25		logical NOT
	~		26	25		bitwise NOT
	- (unary)	25	25		arithmetic NEG
	*		24	22		multiply
	/		23	22		divide
	%		22	22		remainder
	+		20	20		addition
	-		20	20		subtraction
	<<		19	18		left-shift (0 fill)
	>>		18	18		right-shift (sign extend)
	<		17	14		less-than
	<=		16	14		less-than-or-equal
	>		15	14		greater-than
	>=		14	14		greater-than-or-equal
	==		13	12		equal-to
	!=		12	12		not-equal-to
	&		11	11		bitwise AND
	^		10	10		bitwise XOR
	|		9	9		bitwise OR
	&&		8	8		logical AND
	||		7	7		logical OR
	:		6	6		conditional-op (a ? b : c)
	?		5	5
	:-delayed	4	4			"	    "
	?-delayed	3	3			"	    "
	) ]		2	2		parenthesis
	BOS		1	1		Beginning-Of-Statement
	EOS		0	0		End-Of-Statement
	<constant>	-1	<none>		integer (decimal, octal, hex)
	<symbol>	-2	<none>		symbol (value treated as const)
	<undef-symb>	-3	<none>		undefined symbol (treated as 0)
	<error>		-99	<none>		illegal token

	Note that '(' is pushed onto the operator-stack as a ')',
	so the precedence is correct when it is interpreted.
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

#define BOS 1
#define EOS 0
#define CONST (-1)
#define SYMB (-2)
#define UNDEF (-3)
#define ERR (-99)

#define MAX_STACK 16

extern long value;	/* all values are passed here */

char *expr_end;		/* set to terminating char */


static char prec[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,12,14,14,14,14,18,18,
		      20,20,22,22,22,25,25,25,28};
static char op_stack[MAX_STACK+1], *op_sp;
static long val_stack[MAX_STACK+1], *val_sp;
static char *nextch;
static int last_token;
static int symb_ok;
static int cexpr_lex();

int
cexpr(string,symb)
char *string;
int symb;
{
	BEST int op;
	long temp, temp1;
	BEST int iop;
	BEST int undef_sym;

	/* permit an initial '+' */
	if(*string == '+')
		++string;
	undef_sym = 0;
	nextch = string;
	symb_ok = symb;
	expr_end = "";

	op_sp = op_stack;
	val_sp = val_stack;

	op = *op_sp = BOS;
	*val_sp = 0;

	for(;;) {
next_op:	last_token = op;
		if( (op_sp-op_stack) < 0 || (val_sp-val_stack) < 0)
			goto err;
		op = cexpr_lex();
		if(op == CONST || op == SYMB || op == UNDEF) {
			if(val_sp < &val_stack[MAX_STACK])
				*(++val_sp) = value;
			else
				goto err;
			if(op == UNDEF)
				++undef_sym;
		} else if(op < 0) {
			goto err;
		} else {
		    while (prec[op] <= prec[*op_sp]) {
			iop = *op_sp--;
			switch(iop) {
			case 0: goto err;
				/* allow null-string (return 0) */
			case 1: if(val_sp != &val_stack[1] && nextch != string)
					goto err;
				value = *val_sp;
				expr_end = nextch;
				if(undef_sym) {
					value = 0x80000000L;
					return(UNDEF);
				}
				return(0);
			case 2:	if(op != 2) goto err;
				goto next_op;
			case 3:	goto err;
			case 4:	if(*op_sp-- != 3) goto err;
				temp = *val_sp--;
				temp1= *val_sp--;
				*val_sp = *val_sp ? temp1 : temp;
				break;
			case 5:	goto err;
			case 6:	goto err;
			case 7:	temp = *val_sp--; *val_sp = *val_sp || temp;
				break;
			case 8:	temp = *val_sp--; *val_sp = *val_sp && temp;
				break;
			case 9:	temp = *val_sp--; *val_sp |= temp; break;
			case 10:temp = *val_sp--; *val_sp ^= temp; break;
			case 11:temp = *val_sp--; *val_sp &= temp; break;
			case 12:temp = *val_sp--; *val_sp = *val_sp != temp;
				break;
			case 13:temp = *val_sp--; *val_sp = *val_sp == temp;
				break;
			case 14:temp = *val_sp--; *val_sp = *val_sp >= temp;
				break;
			case 15:temp = *val_sp--; *val_sp = *val_sp > temp;
				break;
			case 16:temp = *val_sp--; *val_sp = *val_sp <= temp;
				break;
			case 17:temp = *val_sp--; *val_sp = *val_sp < temp;
				break;
			case 18:temp = *val_sp--; *val_sp >>= temp; break;
			case 19:temp = *val_sp--; *val_sp <<= temp; break;
			case 20:temp = *val_sp--; *val_sp -= temp; break;
			case 21:temp = *val_sp--; *val_sp += temp; break;
			case 22:temp = *val_sp--; if(temp == 0l) goto div_0;
						  *val_sp %= temp; break;
			case 23:temp = *val_sp--; if(temp == 0l) goto div_0;
						  *val_sp /= temp; break;
			case 24:temp = *val_sp--; *val_sp *= temp; break;
			case 25:*val_sp = - *val_sp; break;
			case 26:*val_sp = ~ *val_sp; break;
			case 27:*val_sp = ! *val_sp; break;
			case 28:goto err;
			default:goto err;
			}
		    }
		    if(op_sp < &op_stack[MAX_STACK]) {
			if(op == 28)	/* "(" -> ")" */
				op = 2;
			if(op == 5)	/* "?" -> "?-delayed" */
				op = 3;
			if(op == 6)	/* ":" -> ":-delayed" */
				op = 4;
			*(++op_sp) = op;
		    } else {
			goto err;
		    }
		}
	}
err:
	value = 0l;
	return(ERR);
div_0:
	value = 0l;
	return(ERR);
}

static int
cexpr_lex()
{
	extern long strtol();
	BEST int ich;

	while(isspace(*nextch)) {
		if(*nextch == '\n')
			return(EOS);
		++nextch;
	}

	if(isdigit(*nextch)) {
		value = strtol(nextch,&nextch,0);
		return(CONST);
	} else if(isident(*nextch)) {
		if(!symb_ok)
			return(ERR);
		if(sym_val(nextch,&nextch))
			return(UNDEF);
		return(SYMB);
	}

	ich = *nextch++;
	switch(ich) {
	/* '\n' detected above */
	case ';':
	case ',':
	case '\0':	--nextch; return(EOS);
	case ']':
	case ')':	return(2);
	case '?':	return(5);
	case ':':	return(6);
	case '|':	if(*nextch == '|') {++nextch; return(7);}
			return(9);
	case '^':	return(10);
	case '&':	if(*nextch == '&') {++nextch; return(8);}
			return(11);
	case '=':	if(*nextch++ == '=') return(13);
			goto err;
	case '>':	if(*nextch == '=') {++nextch; return(14);}
			if(*nextch == '>') {++nextch; return(18);}
			return(15);
	case '<':	if(*nextch == '=') {++nextch; return(16);}
			if(*nextch == '<') {++nextch; return(19);}
			return(17);
	case '-':	if(last_token > 0)
				return(25);
			return(20);
	case '+':	return(21);
	case '%':	return(22);
	case '/':	return(23);
	case '*':	return(24);
	case '~':	return(26);
	case '!':	if(*nextch == '=') {++nextch; return(12);}
			return(27);
	case '[':
	case '(':	return(28);
	case '\'':	ich = *nextch++; ich &= 0xFF;
			if(ich == '\\') {
				ich = *nextch++; ich &= 0xFF;
				switch(ich) {
				case 'n':  ich = '\n'; break;
				case 'r':  ich = '\r'; break;
				case 'f':  ich = '\f'; break;
				case '0':  ich = '\0'; break;
				}
			}
			if(*nextch++ != '\'')
				goto err;
			value = ich;
			return(CONST);
	}
err:	return(ERR);
}
