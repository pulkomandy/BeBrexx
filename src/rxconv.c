/*
 * $Header: c:/usr/rexx/RXCONV.C!v 1.3 1997/07/04 20:32:28 bill Exp bill $
 * $Log: RXCONV.C!v $
 * Revision 1.3  1997/07/04  20:32:28  bill
 * *** empty log message ***
 *
 * Revision 1.2  1996/12/13 01:02:49  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11  03:32:21  bill
 * Initial revision
 *
 */

#include <math.h>
#include <stdlib.h>

#include "rexx.h"
#include "error.h"
#include "convert.h"

#include <string.h>

#include "rxdefs.h"

/* ------------------* bitand *-------------------- */
Lstr *lbitand( Lstr *s1, Lstr *s2, boolean ommit, char pad )
{  size_t i;
   for (i=0; i<LEN(s1); i++)    STR(s2)[i] &= STR(s1)[i];
   if (!ommit)  for (; i<LEN(s2); i++)    STR(s2)[i] &= pad;
   return s2;
} /* lbitand */
/* --------------------* bitor *------------------ */
Lstr *lbitor( Lstr *s1, Lstr *s2, boolean ommit, char pad )
{  size_t i;
   for (i=0; i<LEN(s1); i++)    STR(s2)[i] |= STR(s1)[i];
   if (!ommit)  for (; i<LEN(s2); i++)    STR(s2)[i] |= pad;
   return s2;
} /* lbitor */
/* -------------------* bitxor *---------------------- */
Lstr *lbitxor( Lstr *s1, Lstr *s2, boolean ommit, char pad )
{  size_t i;
   for (i=0; i<LEN(s1); i++)    STR(s2)[i] ^= STR(s1)[i];
   if (!ommit)  for (; i<LEN(s2); i++)    STR(s2)[i] ^= pad;
   return s2;
} /* lbitxor */
/* -------------------------------------------------------------- */
/* BITAND(string1[,[string2][,pad]])                              */
/*      logically AND the strings, bit by bit                     */
/* -------------------------------------------------------------- */
/* BITOR(string1[,[string2][,pad]])                               */
/*      logically OR  the strings, bit by bit                     */
/* -------------------------------------------------------------- */
/* BITXOR(string1[,[string2][,pad]])                              */
/*      logically XOR the strings, bit by bit                     */
/* -------------------------------------------------------------- */
void R_SoSoC( args *arg, int func )
{
   char    pad;
   boolean ommit;

   if (!IN_RANGE(1,ARGN,3)) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   if (exist(2)) L2str(&ARG2);
   else          Lscpy(&ARG2,"");
   if (exist(3)) {
      ommit = FALSE;
      get_pad(3);
   }  else ommit = TRUE;

   switch (func) {
     case f_bitand:
             if (LEN(ARG1) <= LEN(ARG2))
                   { ARGR = lbitand(ARG1,ARG2,ommit,pad);  ARG2 = NULL; }
             else  { ARGR = lbitand(ARG2,ARG1,ommit,pad);  ARG1 = NULL; }
             break;

     case f_bitor:
             if (LEN(ARG1) <= LEN(ARG2))
                   { ARGR = lbitor(ARG1,ARG2,ommit,pad);  ARG2 = NULL; }
             else  { ARGR = lbitor(ARG2,ARG1,ommit,pad);  ARG1 = NULL; }
             break;

     case f_bitxor:
             if (LEN(ARG1) <= LEN(ARG2))
                   { ARGR = lbitxor(ARG1,ARG2,ommit,pad);  ARG2 = NULL; }
             else  { ARGR = lbitxor(ARG2,ARG1,ommit,pad);  ARG1 = NULL; }
             break;

     default: error (ERR_INTERPRETER_FAILURE);
   }
} /* R_SoSoC */

/* --------------------* todecimal *------------------ */
void todecimal(Lstr *A, long n)
{
   int  i;
   boolean negative;
#ifndef __MSDOS__
   long    num;
#endif

   if (!LEN(A)) {
      Licpy(&A,0); return;
   }

   Lreverse(A);
   if (n<1 || n>sizeof(long)) n=sizeof(long);  /* ////// numeric */
   if (n <= LEN(A))
	 negative =  STR(A)[(size_t)n-1] & 0x80;  /* msb = 1 */
   else  negative = FALSE;
#ifdef __MSDOS__
   if (negative) negative=0xFF;
   for (i=MIN((size_t)n,LEN(A));i<sizeof(long);i++)
      STR(A)[i] = (char)negative;
#else
   n = MIN(n,LEN(A));
   num = 0;
   if (!negative)
      for (i=MIN((size_t)n,LEN(A))-1; i>=0; i--)
        num = (num<<8) | (STR(A)[i] & 0xFF);
   else {
      for (i=MIN((size_t)n,LEN(A))-1; i>=0; i--)
        num = (num<<8) | ((STR(A)[i] & 0xFF) ^ 0xFF);
      num = -(num + 1);
   }
   INT(A) = num;
#endif
   TYPE(A) = INT_TY;
} /* dodecimal */
/* -------------------------------------------------------------- */
/* C2D(string[,n])                                                */
/*     Character to Decimal. The binary representation of         */
/*     STRING is converted to a number (unsigned unless the       */
/*     length n is specified).                                    */
/* -------------------------------------------------------------- */
/* X2D(hex-string[,n])                                            */
/*     Hexadecimal to Decimal. HEX-STRING is converted to a       */
/*     number (unsigned unless the length n is specified)         */
/* -------------------------------------------------------------- */
void R_SoI ( args *arg, int func )
{
   long    n;
   if (!IN_RANGE(1,ARGN,2)) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   get_oi(2,n);
   switch (func) {
      case f_c2d:
              ARGR = ARG1; ARG1 = NULL;
              break;
      case f_x2d:
              Lx2c(&ARGR,ARG1);
              if (ARGR==NULL) error(ERR_INVALID_HEX_CONST);
              break;

     default: error (ERR_INTERPRETER_FAILURE);
   }
   todecimal(ARGR,n);
} /* R_SoI */

/* -------------------------------------------------------------- */
/* D2C(wholenumber[,n])                                           */
/*      Decimal to Character. Return a string of length n,        */
/*      which is the binary represantation of the number.         */
/* -------------------------------------------------------------- */
/* D2X(wholenumber[,n])                                           */
/*      Decimal to Hexadecimal. Return a string of length n,      */
/*      which is the hexadecimal represantation of the number.    */
/* -------------------------------------------------------------- */
void R_IoI ( args *arg, int func )
{
   long  n;
   int   i,r;
   char  *re,*ar,c;
#ifndef __MSDOS__
   long  num;
   int   rm,neg;
#endif

   if (!IN_RANGE(1,ARGN,2)) error(ERR_INCORRECT_CALL);
   must_exist(1); L2int(&ARG1);
   if (exist(2)) {
      L2int(&ARG2); n = INT(ARG2);
      if (n<0) error(ERR_INCORRECT_CALL);
   } else n = -1;

#ifndef __MSDOS__
   num = INT(ARG1);
   if (num<0) {
      neg = TRUE;
      num = -num - 1;
   } else neg = FALSE;
   i = 0;
   while (num>0) {
      rm = num % 0x100;
      num /= 0x100;
      if (neg) rm ^= 0xFF;
      STR(ARG1)[i++] = (char)rm;
   }
   rm = ((neg)?0xFF:0x00);
   for (;i<sizeof(long);i++) STR(ARG1)[i] = (char)rm;
#endif
   switch (func) {
      case f_d2c:
              if (n==-1) {  /* determine length */
                 i=3;   c = STR(ARG1)[i];
                 if ((c==0x00) || (c==(char)0xFF))
                 while (i && (STR(ARG1)[i]==c)) i--;
                 n = i+1;
              }
              if (n>4) n=4;
              TYPE(ARG1) = STR_TY;   LEN(ARG1) = (size_t)n;
              Lreverse(ARG1);
              ARGR = ARG1; ARG1 = NULL;
              break;

      case f_d2x:
              TYPE(ARG1) = STR_TY;
              LEN(ARG1)  = sizeof(long);

              Lfx(&ARGR,2*sizeof(long));
              re = STR(ARGR); ar = STR(ARG1);
              for (i=r=0; i<sizeof(long); i++) {
                  re[r++] = chex[ar[i] & 0x0F];
                  re[r++] = chex[(ar[i] >> 4) & 0x0F];
              }
              TYPE(ARGR) = STR_TY;

              if (n==-1) {
                 i = 2*sizeof(long)-1;  c = STR(ARGR)[i];
                 if (c=='0' || c == 'F')
                 while (i && (STR(ARGR)[i]==c)) i--;
                 n = i+1;
              }
              LEN(ARGR) = (size_t)n;
              Lreverse(ARGR);
              break;

      default: error (ERR_INTERPRETER_FAILURE);
   }
} /* R_IoI */
/* -------------------------------------------------------------- */
/* FORMAT(number(,(before)(,(after)(,(expp)(,expt)))))            */
/*      rounds and formats number.                                */
/* -------------------------------------------------------------- */
void R_format( args *arg )
{
   long before, after, expp, expt;
   char fmt[15];

   if (!IN_RANGE(1,ARGN,5)) error(ERR_INCORRECT_CALL);
   must_exist(1); L2real(&ARG1);
   if (ARGN==1) {
      ARGR = ARG1; ARG1 = NULL;
      return;
   }
   get_oi0(2,before);
   get_oi0(3,after);
   get_oi0(4,expp);     /*  /// Not used right now  */
   get_oi0(5,expt);     /*  /// Neither this one    */

   ARGR = ARG1; ARG1=NULL;
   Lfx(&ARGR,50);
   if (after) before += (after+1);
   sprintf(fmt,"%%%d.%df",(int)before,(int)after);
   sprintf(STR(ARGR),fmt,REAL(ARGR));
   LEN(ARGR) = strlen(STR(ARGR));
   TYPE(ARGR) = STR_TY;
} /* R_format */
/* -------------------------------------------------------------- */
/* TRUNC(number(,n))                                              */
/*      returns the integer part of number, and n decimal places. */
/*      The default n is zero.                                    */
/* -------------------------------------------------------------- */
void R_trunc( args *arg )       
{                              
   long   n;                  

   if (!IN_RANGE(1,ARGN,2)) error(ERR_INCORRECT_CALL);
   must_exist(1); L2num(&ARG1);
   get_oi0(2,n);

   ARGR = ARG1; ARG1=NULL;
   if (!n) {
      if (TYPE(ARGR)==INT_TY) return;
      INT(ARGR) = (long)REAL(ARGR);
      TYPE(ARGR) = INT_TY;
   } else {
      Lfx(&ARGR,100);
      if (TYPE(ARGR)==INT_TY) REAL(ARGR) = (double)INT(ARGR);
      sprintf(STR(ARGR),"%.*f",(int)n,REAL(ARGR));
      TYPE(ARGR) = STR_TY;
      LEN(ARGR) = strlen(STR(ARGR));
   }
} /* R_trunc */
/* -------------------------------------------------------------- */
/* XRANGE([start][,end])                                          */
/* -------------------------------------------------------------- */
void R_xrange( args *arg )
{
   unsigned int c,start, stop;
   char *r;
   if (ARGN>2) error(ERR_INCORRECT_CALL);
   if (exist(1)) {
      L2str(&ARG1);
      if (LEN(ARG1)!=1) error(ERR_INCORRECT_CALL);
      start = (unsigned)STR(ARG1)[0] & 0xFF;
   } else start = 0;
   if (exist(2)) {
      L2str(&ARG2);
      if (LEN(ARG2)!=1) error(ERR_INCORRECT_CALL);
      stop = (unsigned)STR(ARG2)[0] & 0xFF;
   } else stop = 255;

   if (start <= stop) {
      Lfx(&ARGR,(size_t)labs((int)start-(int)stop)+1);   r = STR(ARGR);
      for (c=start; c<=stop; c++) *r++ = (char)c;
      LEN(ARGR) = (size_t)(labs((int)start-(int)stop)+1);
   } else {
      Lfx(&ARGR,256-(size_t)labs((int)start-(int)stop));   r = STR(ARGR);
      for (c=start; c<=0xFF; c++) *r++ = (char)c;
      for (c=0x00; c<=stop; c++) *r++ = (char)c;
      LEN(ARGR) = (size_t)(256-labs((int)start-(int)stop));
   }
   TYPE(ARGR) = STR_TY;
} /* R_xrange */
