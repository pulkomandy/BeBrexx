/* ----------------------------------------------------- o o ---- */
/*                                                     ____oo_    */
/*                                          (c)1992   /||    |\   */
/*  Expressions                               BNV      ||    |    */
/*  ~~~~~~~~~~~                                        '.___.'    */
/*                                                     MARMITA    */
/*  Most REXX instructions permit the use of expressions,         */
/*  following normal algebraic style. Expressions can consists    */
/*  of strings, symbols, or functions calls. Expressions are      */
/*  evaluated from left to right, modified by parentheses and     */
/*  the priority of the operators (as ordered below). Parenteses  */
/*  may be used to change the order of evaluation.                */
/*                                                                */
/*  All operations (except prefix operations) act on two items    */
/*  and result in a character string.                             */
/*                                                                */
/*  Prefix  + - ^                                                 */
/*        Prefix operations: Plus; Minus; and Not. (For + and -,  */
/*        the item must evaluate to a number; for ^, the item     */
/*        must evaluate to "1" or "0".)                           */
/*                                                                */
/*  **                                                            */
/*        Exponentiate. (Both items must evaluate to numbers,     */
/*        and the right-hand item must be a whole number.)        */
/*                                                                */
/*  * / % //                                                      */
/*        Multiply; Divide; Integer Divide; Divide and return the */
/*        remainder. (Both items must evaluate to numbers.)       */
/*                                                                */
/*  + -                                                           */
/*        Add; Subtract. (Both items must evaluate to numbers.)   */
/*                                                                */
/*  (blank) ||                                                    */
/*        Concatenate: with or without a blank. Abuttal of items  */
/*        causes direct concatenation.                            */
/*                                                                */
/*  = == ^= ^== /= /== >                                          */
/*  < ^> ^< >= <= <> ><                                           */
/*        Comparisons (arithmetic compare if both items evaluate  */
/*        to a number.) The == operator checks for an EXACT       */
/*        match.                                                  */
/*                                                                */
/*  &                                                             */
/*        Logical And. (Both items must evaluate to "0" or "1".)  */
/*                                                                */
/*  | &&                                                          */
/*        Logical Or; Logical Exclusive Or. (Both items must      */
/*        evaluate to "0" or "1".)                                */
/*                                                                */
/*                                                                */
/*  The results of arithmetic operations are rounded (if          */
/*  necessary) to 9 significant figures. Greater accuracy may     */
/*  be requested by using the NUMERIC DIGITS instruction.         */
/*  Trailing zeros are not removed except after division and      */
/*  exponentiation. For example, 3.25 * 2 results in 6.50.        */
/*                                                                */
/* -------------------------------------------------------------- */
/*
 * $Header: expr.c!v 1.1 1995/09/11 03:32:12 bill Exp bill $
 * $Log: expr.c,v $
 * Revision 1.1  1995/09/11 03:32:12  bill
 * Initial revision
 *
 */

#include <string.h>

#include "rexx.h"
#include "expr.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"

/*-----------------* local function prototypes *------------ */

Lstr *Exp1( Lstr *A);
Lstr *Exp2( Lstr *A);
Lstr *Exp3( Lstr *A);
Lstr *Exp4( Lstr *A);
Lstr *Exp5( Lstr *A);
Lstr *Exp6( Lstr *A);
Lstr *Exp7( Lstr *A);
Lstr *Exp8( Lstr *A);
Lstr *I_function( Lstr *A);

/* -------------------------------------------------------------- */
/*   [ Expression ] ::= ?????                                     */
/* -------------------------------------------------------------- */
Lstr *I_expression( Lstr *A )
{
   Lstr *B = NULL;
   enum symboltype _symbol;

   A = Exp1(A);

   _symbol = (symbol==xor_sy);

   while ((symbol==or_sy) || (symbol==xor_sy)) {  /* Logical OR; XOR */
      nextsymbol();
      B = Exp1(B);
      if (_symbol)  INT(A) = O_bool(A) ^ O_bool(B);
      else          INT(A) = O_bool(A) | O_bool(B);
      _symbol = (symbol==xor_sy);
      if (tracing == intermediates_trace) tracevar(">O>",A);
   }
   m_free(B);
   return A;
}  /* Expression */
/* -------------------- Exp1 -------------- */
Lstr *Exp1( Lstr *A)
{
   Lstr *B = NULL;

   A = Exp2(A);
   while (symbol == and_sy) {         /* Logical AND  */
      nextsymbol();
      B = Exp2(B);
      INT(A) = O_bool(A) & O_bool(B);
      if (tracing == intermediates_trace)  tracevar(">O>",A);
   }
   m_free(B);
   return A;
}  /* Exp1 */
/* --------------------- Exp2 ------------------ */
Lstr *Exp2( Lstr *A )
{
   enum  symboltype  _symbol;
   Lstr  *B = NULL;

   A = Exp3(A);
   _symbol = symbol;

   if  ((symbol >= eq_sy)  && (symbol <= gt_sy)) {
       nextsymbol();
       B = Exp3(B);
       switch (_symbol) {
	  case eq_sy  : A = O_eq(A,B);    break;
	  case deq_sy : A = O_deq(A,B);   break;
	  case ne_sy  : A = O_ne(A,B);    break;
	  case nde_sy : A = O_nde(A,B);   break;
	  case gt_sy  : A = O_gt(A,B);    break;
	  case ge_sy  : A = O_ge(A,B);    break;
	  case lt_sy  : A = O_lt(A,B);    break;
	  case le_sy  : A = O_le(A,B);    break;
       }
       if (tracing == intermediates_trace) tracevar(">O>",A);
       m_free(B);
   }
   return A;
}  /* Exp2 */
/* ------------------- Exp3 ----------------- */
Lstr *Exp3( Lstr *A)
{
   int   _Concat;
   int   _Pblank;
   Lstr	 *B = NULL;

   A = Exp4(A);
   _Concat = (symbol==concat_sy);
   _Pblank = Pblank;

   while ((symbol <= not_sy) ||  _Concat) {
      if (_Concat) nextsymbol();
      B = Exp4(B);
      if (_Concat || !_Pblank)  A = O_concat(A,B);
      else                      A = O_bconcat(A,B);

      _Concat = (symbol==concat_sy);
      _Pblank = Pblank;
      if (tracing == intermediates_trace) tracevar(">O>",A);
   }
   m_free(B);
   return A;
}  /* Exp3 */
/* ------------------- Exp4 ------------------ */
Lstr *Exp4( Lstr *A )
{
   enum symboltype _symbol;
   Lstr *B = NULL;

   A = Exp5(A);
   _symbol = symbol;

   while ((symbol==plus_sy) || (symbol==minus_sy)) {
      nextsymbol();
      B = Exp5(B);
      if (_symbol==plus_sy) A = O_add(A,B);
      else		    A = O_sub(A,B);
      _symbol = symbol;
      if (tracing == intermediates_trace) tracevar(">O>",A);
   }
   m_free(B);
   return A;
}  /* Exp4 */
/* --------------------- Exp5 -------------------- */
Lstr *Exp5( Lstr *A )
{
   enum  symboltype _symbol;
   Lstr  *B=NULL;

   A = Exp6(A);
   _symbol = symbol;
   while ((symbol >= times_sy) && (symbol<=intdiv_sy)) {
     nextsymbol();
     B = Exp6(B);
     switch (_symbol) {
	case times_sy  : A = O_mult(A,B);    break;
	case div_sy    : A = O_div(A,B);     break;
	case intdiv_sy : A = O_intdiv(A,B);  break;
	case mod_sy    : A = O_mod(A,B);     break;
     }
     if (tracing == intermediates_trace) tracevar(">O>",A);
     _symbol = symbol;
   }
   m_free(B);
   return A;
}  /* Exp5 */
/* -------------- Exp6 --------------- */
Lstr *Exp6( Lstr *A )
{
   Lstr *B = NULL;

   A = Exp7(A);
   while (symbol==power_sy) {
      nextsymbol();
      B = Exp7(B);
      A = O_expose(A,B);
      if (tracing == intermediates_trace) tracevar(">O>",A);
   }
   m_free(B);
   return A;
} /* Exp6 */
/* ---------------- Exp7 ------------- */
Lstr *Exp7 ( Lstr *A )
{
   enum symboltype _symbol;
   int  prefix;

   _symbol = symbol;

   if ((symbol==not_sy) || (symbol==minus_sy)) {
	nextsymbol();
	prefix = TRUE;
   } else  prefix = FALSE;

   if (!prefix && (symbol==plus_sy)) nextsymbol();

   A = Exp8(A);
   if (prefix) {
      if (_symbol==not_sy) INT(A) = !O_bool(A);
      else                 A = O_minus(A);
      if (tracing == intermediates_trace) tracevar(">O>",A);
  }
  return A;
} /* Exp7 */
/* ---------------- Exp8 ------------- */
Lstr *Exp8( Lstr *A )
{
   if (symbol == ident_sy) {
     V_eval(symbolisarray, &A, symbolstr, scope );
     if (tracing == intermediates_trace) {
    	if (V_search(symbolisarray,symbolstr,scope)!=NULL)
              tracevar(">V>", A);
    	else  tracevar(">L>", A);
     }
     nextsymbol();
   } else
   if (symbol == function_sy)
      A = I_function(A);
   else
   if (symbol==literal_sy) {
      Lstrcpy( &A, symbolstr );
      if (tracing==intermediates_trace) tracevar(">L>", A);
      nextsymbol();
   } else
   if (symbol==le_parent) {
      nextsymbol();
      A = I_expression(A);
      mustbe(ri_parent,ERR_UNMATCHED_PARAN);
   } else
      Lscpy(&A,"");
   return A;
} /* Exp8 */
/* -------------------------------------------------------------- */
/*  [Function] ::= [Identifier]([[Expression][,[Expression]]...]) */
/* -------------------------------------------------------------- */
Lstr *I_function ( Lstr *A )
{
   Lstr   *funcname=NULL;
   args   *_arg;
   args   *Carg;
   int     n,isstr;

   Lstrcpy(&funcname, symbolstr); isstr = symbolisstr;
   nextsymbol();
   Carg = mallocargs();
   n = 0;
   if (symbol!=ri_parent) {
      if (symbol != comma_sy)
         Carg->a[n] = I_expression( Carg->a[n] );
      n++;
      while (symbol==comma_sy) {
         nextsymbol();
         if (n >= maxargs) error(ERR_INCORRECT_CALL);
         if (! ((symbol==comma_sy) || (symbol==ri_parent)))
            Carg->a[n] = I_expression( Carg->a[n] );
         n++;
      }
   }
   Carg->n = n;

/* push arguments */
   depth++;
   if (depth>250) error(ERR_FULL_CTRL_STACK);

   _arg = oarg;
   oarg =  arg;
   arg = Carg;

   call_type = FUNCTION;
   if (isstr) System_Function( &funcname);
   else       Rexx_Function( funcname );
   m_free(funcname);

   if (arg->n)  Lstrcpy(&A, arg->r);
   else         error(ERR_NO_DATA_RETURNED);

/* Pop old arguments */
   freeargs( arg );
   arg = oarg;
   oarg = _arg;
   --depth;
   if (tracing == intermediates_trace) tracevar(">F>",A);
   mustbe(ri_parent, ERR_UNMATCHED_PARAN);
   return A;
} /* I_function */

/* ----------------------------------------------------- o o ---- */
/*                                                     ____oo_    */
/*                                          (c)1992   /||    |\   */
/*  Operator                                  BNV      ||    |    */
/*  ~~~~~~~~                                           '.___.'    */
/*                                                     MARMITA    */
/*  Procedures used as operators in I_Expression                  */
/* -------------------------------------------------------------- */

#define Is_Number(A)  (datatype(A) != 'C')
#define Is_Int(AA)   (REAL(AA) == (double)(long)REAL(AA))
#define toreal(AA)   ((TYPE(AA)==INT_TY)? (double) INT(AA):  REAL(AA) )
#define toint(AA)    ((TYPE(AA)==INT_TY)? INT(AA):  (long)REAL(AA) )
#define odd(n)       (n&0x1)

/* -------------------------------------------------------------- */
Lstr *O_add( Lstr *A, Lstr *B )
{
  L2num( &A );
  L2num( &B );

  if ( (TYPE(A) == INT_TY) && (TYPE(B)==INT_TY))
     INT(A) += INT(B);
  else {
     REAL(A) = toreal(A) + toreal(B);
     TYPE(A) = REAL_TY;
  }
  return A;
} /* O_add */
/* -------------------------------------------------------------- */
int O_bool( Lstr *A )
{
   L2num( &A );
   if (TYPE(A) == REAL_TY)
      if (Is_Int(A)) INT(A) = (long)REAL(A);
      else         error(ERR_UNLOGICAL_VALUE);
   TYPE(A) = INT_TY;
   if (INT(A) == 0) return FALSE;
   if (INT(A) == 1) return TRUE;
   error(ERR_UNLOGICAL_VALUE);
   return -1;             /* Never gets here but keep compiler happy :-) */
} /* O_bool */
/* -------------------------------------------------------------- */
Lstr *O_bconcat( Lstr *A, Lstr *B )
{
   L2str( &A );
   L2str( &B );
   Lcat(&A," ");
   Lstrcat(&A,B);
   return A;
} /* O_bconcat */
/* -------------------------------------------------------------- */
Lstr *O_concat( Lstr *A, Lstr *B )
{
   L2str( &A );
   L2str( &B );
   Lstrcat(&A,B);
   return A;
} /* O_concat */
/* -------------------------------------------------------------- */
Lstr *O_deq ( Lstr *A, Lstr *B )
{
   L2str ( &A );
   L2str ( &B );
   if (LEN(A) != LEN(B))     INT(A) = FALSE;
   else                      INT(A) = !memcmp(STR(A),STR(B),LEN(A));
   TYPE(A) = INT_TY;
   return A;
} /* O_deq */
/* -------------------------------------------------------------- */
Lstr *O_sub( Lstr *A, Lstr *B )
{
  L2num( &A );
  L2num( &B );

  if ( (TYPE(A) == INT_TY) && (TYPE(B)==INT_TY))
     INT(A) -= INT(B);
  else {
     REAL(A) = toreal(A) - toreal(B);
     TYPE(A) = REAL_TY;
  }
  return A;
} /* O_sub */
/* -------------------------------------------------------------- */
Lstr *O_div ( Lstr *A, Lstr *B )
{
   L2num( &A );
   L2num( &B );

   if (TYPE(B) == REAL_TY)   { if (REAL (B) == 0) error(ERR_ARITH_OVERFLOW); }
   else                      { if (INT  (B) == 0) error(ERR_ARITH_OVERFLOW); }

   REAL (A) = toreal(A) / toreal(B);
   TYPE (A) = REAL_TY;

   return A;
} /* O_div */
/* -------------------------------------------------------------- */
Lstr *O_eq (Lstr *A, Lstr *B)
{
   if (Is_Number( A )  && Is_Number( B )) {
      L2num( &A );
      L2num( &B );
      if ( (TYPE (A)==INT_TY) && (TYPE (B) == INT_TY) )
	 INT (A) = (INT(A) == INT(B));
      else
	 INT (A) = (toreal(A) == toreal(B));
   } else {
      L2str( &A );
      L2str( &B );
      Lstrip( &A );
      Lstrip( &B );
      INT(A) = !Lstrcmp(A,B);
   }
   TYPE (A) = INT_TY;
   return A;
}  /* O_eq */
/* -------------------------------------------------------------- */
Lstr *O_expose ( Lstr *A, Lstr *B)
{
   double   ar,r;
   long     bi;
   boolean  minusA;
   boolean  minusB;

   L2real( &A );     ar = REAL(A);
   L2int( &B );      bi = INT(B);

   if (ar < 0 ) {
      if (odd(bi)) minusA = TRUE;
      else         minusA = FALSE;
      ar = - ar;
   } else minusA = FALSE;
   if (bi < 0 ) {
      minusB = TRUE;
      bi = - bi;
   } else minusB = FALSE;
   r = 1;
   while (bi != 0) {
      if (odd(bi)) r *= ar;
      ar *= ar;
      bi /= 2;
   }
   if (minusA) r = -r;
   if (minusB) REAL(A) = 1/r;
   else        REAL(A) = r;
   TYPE(A) = REAL_TY;

   return A;
}  /* O_expose */
/* -------------------------------------------------------------- */
Lstr *O_ge( Lstr *A, Lstr *B)
{
   if (Is_Number( A ) && Is_Number( B )) {
      L2num( &A );
      L2num( &B );
      if ( (TYPE (A)==INT_TY) && (TYPE (B) == INT_TY) )
	 INT (A) = (INT(A) >= INT(B));
      else
	 INT (A) = (toreal(A) >= toreal(B));
   } else {
      L2str( &A );
      L2str( &B );
      INT (A) = (Lstrcmp(A,B) >= 0);
   }
   TYPE(A) = INT_TY;
   return A;
}  /* O_ge */
/* -------------------------------------------------------------- */
Lstr *O_gt( Lstr *A, Lstr *B)
{
   if (Is_Number( A ) && Is_Number( B )) {
      L2num( &A );
      L2num( &B );
      if ( (TYPE (A)==INT_TY) && (TYPE (B) == INT_TY) )
	 INT (A) = (INT(A) > INT(B));
      else
	 INT (A) = (toreal(A) > toreal(B));
   } else {
      L2str( &A );
      L2str( &B );
      INT (A) = (Lstrcmp(A,B) > 0);
   }
   TYPE(A) = INT_TY;
   return A;
}  /* O_gt */
/* -------------------------------------------------------------- */
Lstr *O_intdiv ( Lstr *A, Lstr *B )
{
   L2num( &A );
   L2num( &B );

   if (TYPE(B) == REAL_TY)   { if (REAL (B) == 0) error(ERR_ARITH_OVERFLOW); }
   else                      { if (INT  (B) == 0) error(ERR_ARITH_OVERFLOW); }

   INT(A) = (long) (toreal(A) / toreal(B));
   TYPE(A) = INT_TY;

   return A;
} /* O_intdiv */
/* -------------------------------------------------------------- */
Lstr *O_le( Lstr *A, Lstr *B)
{
   A = O_gt(A,B);
   INT(A) = !INT(A);
   return A;
}  /* O_le */
/* -------------------------------------------------------------- */
Lstr *O_lt( Lstr *A, Lstr *B)
{
   A = O_ge(A,B);
   INT(A) = !INT(A);
   return A;
}  /* O_lt */
/* -------------------------------------------------------------- */
Lstr *O_minus( Lstr *A )
{
   L2num( &A );
   if (TYPE(A) == INT_TY) INT(A) = - INT(A);
   else                  REAL(A) = - REAL(A);
   return A;
} /* O_minus */
/* -------------------------------------------------------------- */
Lstr *O_mod( Lstr *A, Lstr *B )
{
   double ra,rb;

   L2num( &A );
   L2num( &B );

   if (TYPE(B) == REAL_TY)   { if (REAL (B) == 0) error(ERR_ARITH_OVERFLOW); }
   else                      { if (INT  (B) == 0) error(ERR_ARITH_OVERFLOW); }

   if ((TYPE(A)==INT_TY) && (TYPE(B)==INT_TY))
      INT(A) = INT(A) % INT(B);
   else  {
      ra = toreal(A);
      rb = toreal(B);
      REAL(A) = ra - (long)(ra / rb) * rb;
      TYPE(A) = REAL_TY;
   }

   return A;
} /* O_mod */
/* -------------------------------------------------------------- */
Lstr *O_mult( Lstr *A, Lstr *B)
{
   L2num( &A );
   L2num( &B );
   if ((TYPE(A)==INT_TY) && (TYPE(B)==INT_TY))
      INT(A) *= INT(B);
   else {
      REAL(A) = toreal(A) * toreal(B);
      TYPE(A) = REAL_TY;
   }
   return A;
}  /* O_mult */
/* -------------------------------------------------------------- */
Lstr *O_nde( Lstr *A, Lstr *B)
{
   A = O_deq(A,B);
   INT(A) = !INT(A);
   return A;
} /* O_nde */
/* -------------------------------------------------------------- */
Lstr *O_ne( Lstr *A, Lstr *B)
{
   A = O_eq(A,B);
   INT(A) = !INT(A);
   return A;
} /* O_ne */
