/*
 * $Header: rxword.c!v 1.2 1996/12/13 01:03:02 bill Exp bill $
 * $Log: rxword.c!v $
 * Revision 1.2  1996/12/13 01:03:02  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11  03:32:40  bill
 * Initial revision
 *
 */
#include <string.h>
#include "rexx.h"
#include "error.h"
#include "convert.h"
#include "variable.h"

#include "rxdefs.h"

/* -------------------* skipblanks *---------------------- */
size_t skipblanks( Lstr *S, size_t p )
{
  while ((p<LEN(S)) && ISSPACE(STR(S)[p])) p++;
  return p;
} /* skipblanks */
/* -------------------* skipblanks *---------------------- */
size_t skipword( Lstr *S, size_t p )
{
  while ((p<LEN(S)) && !ISSPACE(STR(S)[p])) p++;
  return p;
} /* skipblanks */

/* -------------------* Lwordindex *---------------------- */
long Lwordindex( Lstr *s, size_t n )
{
   size_t i;
   if ((LEN(s)==0) || (n<=0))  return -1;
   for (i=0;;) {
      i = skipblanks(s,i);
      if (i>=LEN(s)) return -1;
      n--;
      if (!n) return i;
      i = skipword(s,i);
   }
}  /* Lwordindex */

/* -------------------* Lwordpos *----------------------- */
long Lwordpos( Lstr *phrase, Lstr *s, size_t n )
{
   long    idx;
   size_t  lp,p;
   size_t  lk,k;

   if (!n) n = 1;
   idx = Lwordindex(s,n);  if (idx == -1) return 0;
   lp = (size_t)idx;
   lk = skipblanks(phrase,0);  if  (lk>=LEN(phrase)) return 0;
   k = lk;  p = lp;
   while (1) {
     if (p >= LEN(s)) {
        k = skipblanks(phrase,k);
        if (k>=LEN(phrase)) return n;
        else                return 0;
     }
     if (k >= LEN(phrase)) {
        if (p>=LEN(s)) return n;
        if (STR(s)[p]==' ') return n;
        k = lk;
        lp = skipword(s,lp);   lp = skipblanks(s,lp);
        if (lp>=LEN(s)) return 0;
        p = lp;  n++;
     } else
     if (STR(phrase)[k] == ' ') {
        if (STR(s)[p] == ' ') {
           k = skipblanks(phrase,k);
           if (k>=LEN(phrase)) return n;
           p = skipblanks(s,p);
           if (p>=LEN(s)) return 0;
        } else {
           k = lk;
           lp = skipword(s,lp);   lp = skipblanks(s,lp);
           if (lp>=LEN(s)) return 0;
           p = lp;  n++;
        }
     } else {
        if (STR(phrase)[k] == STR(s)[p]) {
           k++; p++;
        }  else {
           k = lk;
           lp = skipword(s,lp); lp = skipblanks(s,lp);
           if (lp >= LEN(s)) return 0;
           p = lp;  n++;
        }
     }
   }
} /* Lwordpos */

/* --------------------* Lspace *------------------ */
void Lspace(Lstr **R, Lstr *A, long n, char pad )
{
   size_t  p,lp;
   Lstr   *space=NULL;
   Lstr   *sub=NULL;

   Lstrip(&A);   p = 0;
   Lstrset(&space,(size_t)n,pad);

   for (;;) {
      lp = p;
      p = skipword(A,p);
      Lstrsub(&sub,A,lp,p-lp);
      Lstrcat(R, sub);
      if (p>=LEN(A)) {
         m_free(space);
         m_free(sub);
         return;
      }
      Lstrcat(R, space);
      p = skipblanks(A,p);
   }
}  /* Lspace */

/* -------------------------------------------------------------- */
/* FIND(string,phrase)                                            */
/*       returns the word number of the first word of phrase      */
/*       in string. Returns "0" if phrase is not found            */
/* -------------------------------------------------------------- */
void R_find( args *arg )
{
   if (ARGN!=2) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   must_exist(2);  L2str(&ARG1);
   Licpy(&ARGR, Lwordpos(ARG2,ARG1, 1));
} /* R_find */
/* -------------------------------------------------------------- */
/* SPACE(string(,(n)(,pad)))                                      */
/*       formats the blank-delimited words in string with n pad   */
/*       characters between each word.                            */
/* -------------------------------------------------------------- */
void R_space( args *arg )
{
   long    n;
   char    pad;

   if (!IN_RANGE(1,ARGN,3)) error(ERR_INCORRECT_CALL);
   must_exist(1); L2str(&ARG1);
   if (exist(2)) {
      L2int(&ARG2); n = INT(ARG2);
      if (n<0) error(ERR_INCORRECT_CALL);
   } else n = 1;
   get_pad(3);

   Lspace(&ARGR, ARG1, n, pad);
} /* R_space */
/* -------------------------------------------------------------- */
/*  WORD(string,n)                                                */
/*       return the nth word in string                            */
/* -------------------------------------------------------------- */
/*  WORDINDEX(string,n)                                           */
/*       return the position of the nth word in string            */
/* -------------------------------------------------------------- */
/*  WORDLENGTH(string,i)                                          */
/*       return the length of the nth word in string              */
/* -------------------------------------------------------------- */
void R_SI ( args *arg, int func )
{
   long     i,n;

   if (ARGN!=2) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   get_i(2,n);

   switch (func) {
      case f_word:
              i = Lwordindex(ARG1,(size_t)n);
              if (i==-1) {
                 Lscpy(&ARGR,"");
                 return;
              }
              n = i;
              i = skipword(ARG1,(size_t)i);
              Lstrsub(&ARGR,ARG1,(size_t)n,(size_t)(i-n));
              break;

      case f_wordlength:
              i = Lwordindex(ARG1,(size_t)n);
              if (i==-1) {
                 Licpy(&ARGR,0);
                 return;
              }
              n = i;
              i = skipword(ARG1,(size_t)i);
              Licpy(&ARGR,i-n);
              break;

      case f_wordindex:
              Licpy(&ARGR, Lwordindex(ARG1,(size_t)n)+1);
              break;

      default:   error(ERR_INTERPRETER_FAILURE);
   } /* switch */
} /* R_SI */
