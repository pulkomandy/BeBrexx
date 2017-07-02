/*
 * $Header: c:/usr/rexx/RXMATH.C!v 1.2 1997/07/04 20:32:39 bill Exp bill $
 * $Log: RXMATH.C!v $
 * Revision 1.2  1997/07/04  20:32:39  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11 03:32:59  bill
 * Initial revision
 *
 */

#include <math.h>
#include <stdlib.h>

#include "rexx.h"
#include "error.h"
#include "convert.h"

#include "rxdefs.h"

/* -------------------------------------------------------------- */
/*  ABS(number)                                                   */
/*       return absolute value of number                          */
/* -------------------------------------------------------------- */
/*  SIGN(number)                                                  */
/*       return the sign of NUMBER ("-1","0" or "1").             */
/* -------------------------------------------------------------- */
void R_abs_sign( args *arg, int f )
{
   int s;
   if (ARGN!=1) error(ERR_INCORRECT_CALL);
   L2num(&ARG1);
   if (f==f_abs) {
      if (TYPE(ARG1)==REAL_TY)
          Lrcpy(&ARGR, fabs(REAL(ARG1)));
      else
          Licpy(&ARGR, labs(INT(ARG1)));
   } else {
      if (TYPE(ARG1)==REAL_TY) {
         if (REAL(ARG1)<0) s = -1;
         else
         if (REAL(ARG1)>0) s =  1;
         else  s = 0;
      } else {
         if (INT(ARG1)<0) s = -1;
         else
         if (INT(ARG1)>0) s =  1;
         else  s = 0;
      }
      Licpy(&ARGR, s);
   }
} /* R_abs_sign */

#ifndef __BORLANDC__
double pow10(double f)
{
   return pow(10.0,f);
}
#endif
#ifdef MATH
double R_asinh(double x)
{
   if (x > 0.0) return( log(sqrt(1.0 + x * x) + x));
   return(-log(sqrt(1.0 + x * x) - x));
}

double R_acosh(double x)
{
   return(log(sqrt(x * x - 1.0) + x));
}

double R_atanh(double x)
{
   return(0.5 * log((1.0 + x)/(1.0 - x)));
}

/* ------------------* common math functions *------------- */
void R_math( args *arg, int func )
{
   if (ARGN!=1) error(ERR_INCORRECT_CALL);
   L2real(&ARG1);  ARGR = ARG1; ARG1=NULL;
   switch (func) {
     case f_acos:  REAL(ARGR) = acos(REAL(ARGR)); break;
     case f_asin:  REAL(ARGR) = asin(REAL(ARGR)); break;
     case f_atan:  REAL(ARGR) = atan(REAL(ARGR)); break;
     case f_cos :  REAL(ARGR) =  cos(REAL(ARGR)); break;
     case f_cosh:  REAL(ARGR) = cosh(REAL(ARGR)); break;
     case f_exp :  REAL(ARGR) =  exp(REAL(ARGR)); break;
     case f_log :  REAL(ARGR) =  log(REAL(ARGR)); break;
     case f_log10: REAL(ARGR) = log10(REAL(ARGR)); break;
     case f_pow10: REAL(ARGR) = pow10(REAL(ARGR)); break;
     case f_sin :  REAL(ARGR) =  sin(REAL(ARGR)); break;
     case f_sinh:  REAL(ARGR) = sinh(REAL(ARGR)); break;
     case f_sqrt:  REAL(ARGR) = sqrt(REAL(ARGR)); break;
     case f_tan :  REAL(ARGR) =  tan(REAL(ARGR)); break;
     case f_tanh:  REAL(ARGR) = tanh(REAL(ARGR)); break;
     case f_ceil:  REAL(ARGR) = ceil(REAL(ARGR)); break;
     case f_floor: REAL(ARGR) = floor(REAL(ARGR)); break;
     case f_asinh: REAL(ARGR) = R_asinh(REAL(ARGR)); break;
     case f_acosh: REAL(ARGR) = R_acosh(REAL(ARGR)); break;
     case f_atanh: REAL(ARGR) = R_atanh(REAL(ARGR)); break;
     default: error(ERR_INTERPRETER_FAILURE);
   }
} /* R_math */

/* ----------* raise to power *-------------- */
void R_pow( args *arg )
{
   if (ARGN!=2) error(ERR_INCORRECT_CALL);
   L2real(&ARG1);  ARGR = ARG1; ARG1=NULL;
   L2real(&ARG2);
   REAL(ARGR) = pow(REAL(ARGR),REAL(ARG2));
} /* R_pow */

void R_atan2( args *arg )
{
   if (ARGN!=2) error(ERR_INCORRECT_CALL);
   L2real(&ARG1);  ARGR = ARG1; ARG1=NULL;
   L2real(&ARG2);
   REAL(ARGR) = atan2(REAL(ARGR),REAL(ARG2));
} /* R_pow */
#endif
