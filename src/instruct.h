/*
 * $Header: instruct.h!v 1.1 1995/09/11 03:36:44 bill Exp bill $
 * $Log: instruct.h!v $
 * Revision 1.1  1995/09/11 03:36:44  bill
 * Initial revision
 *
 */

#ifndef __INSTRUCT_H__
#define __INSTRUCT_H__

#include <string.h>

#include "rexx.h"
#include "expr.h"
#include "error.h"
#ifdef __MSDOS__
#include "int2e.h"
#endif
#include "utils.h"
#include "convert.h"
#include "variable.h"

#define  assign(A,B)       Lstrcpy(&(A->var),B)
#define  mustbe_semicolon  mustbe(semicolon_sy, ERR_INVALID_EXPRESSION)
#define  skiptill(S)       while (symbol!=S) nextsymbol()
#define  skip_semicolons   while (symbol==semicolon_sy) nextsymbol()
#define  I_instr_till_end  {while (!I_instr( TRUE)) /*do-nothing*/;}

#define  AIR_print        { if (tracing & AIR_trace) printcurline();}

#define  Jcpy(A,B)      memcpy(&A,&B,sizeof(A));

int redirect_cmd( char *cmd, int in, int out, int isfunc );
void  I_cmd( Lstr *A, Lstr *env );
void  I_do ( void );
void  I_iterate ( void );
void  I_leave ( void );
int   I_instr ( boolean return_with_end );


#ifdef __INSTRUCT_C__
#   define EXTERN
#else
#   define EXTERN extern
#endif

EXTERN int   IN_DO;
EXTERN int   IN_FUNCTION;
EXTERN char *DO_BPTR;
EXTERN Lstr *ControlVar;

EXTERN jmp_buf do_instr;

EXTERN int   IN_SIGNAL;
EXTERN int   INTERPRET_LEVEL;
EXTERN int   EXIT_FROMPRG;

#undef EXTERN

#endif
