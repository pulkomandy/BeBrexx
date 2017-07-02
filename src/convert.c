/*
 * $Header: c:/usr/rexx/CONVERT.C!v 1.3 1997/07/04 20:32:22 bill Exp bill $
 * $Log: CONVERT.C!v $
 * Revision 1.3  1997/07/04  20:32:22  bill
 * *** empty log message ***
 *
 * Revision 1.2  1997/02/01 12:53:01  bill
 * Changed: in L2str from free format "lG" to have at least 7 decimal digits
 * "8lG"
 *
 * Revision 1.1  1995/09/11  03:31:51  bill
 * Initial revision
 *
 */

#define __CONVERT_C__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rexx.h"
#include "error.h"
#include "common.h"
#include "convert.h"



/* ------------------* initialize u2l and l2u *---------------- */

byte u2l[256];
byte l2u[256];

void init_ul ( void )
{
   int  i;
   for (i=0; i<256; i++)  u2l[i] = l2u[i] = i;
   for (i=0; clower[i]; i++) {
      l2u[ (unsigned char)clower[i] ] = cUPPER [i];
      u2l[ (unsigned char)cUPPER[i] ] = clower [i];
   }
} /* init_ul */


static enum  CharSets {
      Blanks,      /* ' '      */
      Dot,         /* '.'      */
      Sign,        /* '+','-'  */
      Digits,      /* '0'..'9' */
      E            /* 'e','E'  */
};

#define One   0
#define Many  1

static  Lstr   *S=NULL;
static  int     p=0;
static  boolean found=FALSE;

/* --------------------- in_set ----------------- */
static int  in_set( char c, enum CharSets set )
{
    switch (set) {
        case Blanks:   return  c==' ';
        case Dot:      return  c=='.';
        case Sign:     return  ((c=='+') || (c=='-'));
        case Digits:   return  IN_RANGE('0',c,'9');
        case E:        return  ((c=='e') || (c=='E'));
    }
    return 0;       /* just to keep compiler happy */
} /* in_set */

/* --------------------- Accept ---------------- */
static void Accept( boolean many, int C )
{
   boolean more;
   int     pold;

   pold = p;
   if (many) {
       more = TRUE;
       while ( (p<LEN(S)) &&  more)
           if (in_set(STR(S)[p], C))  p++;
                             else     more = FALSE;
   } else
   if ( (p<LEN(S)) &&  in_set(STR(S)[p], C))  p++;
   found = (p != pold);
} /* Accept */


/* --------------------- datatype --------------------- */
char datatype( Lstr *s )
{
   int   F, R;

  if (TYPE(s)==INT_TY)  return 'I';
  if (TYPE(s)==REAL_TY) return 'R';

  S = s;
  p = 0;
  Accept( Many , Blanks );  if (p >= LEN(s))  goto alphabetic;
  Accept( One  , Sign   );  if (p >= LEN(s))  goto alphabetic;
  Accept( Many , Blanks );  if (p >= LEN(s))  goto alphabetic;
  Accept( Many , Digits );  F = found;
  Accept( One  , Dot    );  R = found;
  Accept( Many , Digits );  if (!F && !found) goto alphabetic;
  Accept( One  , E      );
    if (found) {
       Accept( One  , Sign   ); if (p >= LEN(s)) goto alphabetic;
       Accept( Many , Digits ); if (!found)     goto alphabetic;
       R = TRUE;
    }
  Accept( Many , Blanks );

  if (p < LEN(s)) goto alphabetic;
  if (R)  return 'R';
  else    return 'I';

alphabetic:
  return 'C';
}  /* datatype */

/* ------------------* L2INT *-------------------- */
/* No checking is done for errors                  */
Lstr *L2INT ( Lstr **to )
{
   switch (TYPE((*to))) {
      case REAL_TY:   INT((*to)) = (long) REAL((*to));
                      break;
      case STR_TY:    STR((*to)) [LEN((*to))] = '\0';
                      INT((*to)) = atol( STR((*to)));
                      break;
   }
   LEN((*to)) = sizeof(long);
   TYPE((*to)) = INT_TY;

   return *to;
}  /* L2INT */

/* ------------------* L2REAL *-------------------- */
/* No checking is done for errors */
Lstr *L2REAL ( Lstr **to )
{
   switch (TYPE((*to))) {
      case INT_TY:    REAL((*to)) = (double) INT((*to));
                      break;
      case STR_TY:    STR((*to)) [LEN((*to))] = '\0';
                      REAL((*to)) = atof( STR((*to)));
                      break;
   }
   LEN((*to)) = sizeof(double);
   TYPE((*to)) = REAL_TY;

   return *to;
}  /* L2REAL */

/* ------------------* L2int *-------------------- */
Lstr *L2int ( Lstr **to )
{
    switch (datatype(*to)) {
      case 'I':      L2INT(to);  break;
      case 'R':      L2REAL(to);
                     if ((double)((long)REAL(*to)) == REAL(*to)) {
                        INT(*to) = (long)REAL(*to);
                        TYPE(*to) = INT_TY;
                        break;
                     }
      default:       error(ERR_INVALID_INTEGER);
    }
    return *to;
}  /* L2int */

/* ------------------* L2real *-------------------- */
Lstr *L2real ( Lstr **to )
{
    switch (datatype(*to)) {
      case 'I':      L2INT(to);
                     REAL((*to)) = INT((*to));
                     TYPE((*to)) = REAL_TY;
                     break;
      case 'R':      L2REAL(to); break;
      default:       error(ERR_BAD_ARITHMETRIC);
    }
    return *to;
}  /* L2real */
/* ------------------* L2num *-------------------- */
Lstr *L2num ( Lstr **to )
{
    switch (datatype(*to)) {
      case 'I':      L2INT(to);  break;
      case 'R':      L2REAL(to); break;
      default:       error(ERR_BAD_ARITHMETRIC);
    }
    return *to;
}  /* L2num */

/* ------------------* L2str *-------------------- */
Lstr *L2str ( Lstr **to )
{
   switch (TYPE((*to))) {
      case INT_TY:    sprintf(STR((*to)), "%ld", (long)INT((*to)));
                      LEN((*to)) = strlen(STR((*to)));
                      break;
#if defined(__BEOS__)
      case REAL_TY:   sprintf(STR((*to)), "%.8G", REAL((*to)));
#else
      case REAL_TY:   sprintf(STR((*to)), "%.8lG", REAL((*to)));
#endif
                      LEN((*to)) = strlen(STR((*to)));
                      break;
   }
   TYPE((*to)) = STR_TY;

   return *to;
}  /* L2str */

/* ------------------* L2upper *---------------------- */
Lstr *L2upper ( Lstr *to )
{
   int i;
   if (to == NULL) return to;
   if (TYPE(to)==STR_TY)
     for (i=0; i<LEN(to); i++)
        STR(to)[i] = l2u[ (unsigned char) STR(to)[i] ];
   return to;
} /* L2upper */

/* ------------------* L2lower *---------------------- */
Lstr *L2lower ( Lstr *to )
{
   int i;
   if (to == NULL) return to;
   if (TYPE(to)==STR_TY)
     for (i=0; i<LEN(to); i++)
          STR(to)[i] = u2l[ (unsigned char)STR(to)[i] ];
   return to;
} /* L2lower */

/* --------------------* Lx2c *----------------------- */
Lstr *Lx2c ( Lstr **to, Lstr *from )
{
   size_t i,j,r;
   char   *t,*f;

   Lfx(to,LEN(from)/2 +1);

   t = STR(*to);    f = STR(from);

   for (i=r=0; i<LEN(from); )  {
      for (; ISSPACE(f[i]) && (i<LEN(from)); i++) ;; /*skip spaces*/
      for (j=i; isxdigit(f[j]) && (j<LEN(from)); j++) ;; /* find hexdigits */

      if ((i<LEN(from)) && (j==i)) { /* Ooops wrong character */
         m_free(*to); *to = NULL;    /* return null when error occures */
         return *to;
      }

      if ((j-i)&1)  { t[r++] = HEXVAL(f[i]); i++ ;}
      for (; i<j; i+=2)
         t[r++] = (HEXVAL(f[i])<<4) | HEXVAL(f[i+1]);
   }
   TYPE(*to) = STR_TY;
   LEN(*to) = r;
   return *to;
} /* Lx2c */
/* --------------------* Lreverse *-------------------- */
Lstr *Lreverse( Lstr *A )
{
   size_t i;
   char   c,*cf,*cl;
   cf = STR(A);   cl = STR(A) + LEN(A) - 1;
   i = LEN(A) / 2;
   while (i--) {
     c = *cf;
     *cf++ = *cl;
     *cl-- = c;
   }
   return A;
} /* Lreverse */
