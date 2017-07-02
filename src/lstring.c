/*
 * $Header: lstring.c!v 1.3 1997/04/15 13:46:12 bill Exp bill $
 * $Log: lstring.c!v $
 * Revision 1.3  1997/04/15 13:46:12  bill
 * Fixed bug, in Lscpy when from is NULL string
 *
 * Revision 1.2  1996/12/13  01:01:54  bill
 * Minnor changes
 *
 * Revision 1.1  1995/09/11  03:30:24  bill
 * Initial revision
 *
 */

#include <stdio.h>
#include <string.h>

#include "rexx.h"
#include "lstring.h"
#include "convert.h"

/* -----------------* Lfx *---------------- */
/* fix the size of the Length prefixed string */
Lstr *Lfx( Lstr **s, size_t len )
{
   size_t max;

   if (*s==NULL) {
      *s = (Lstr *) m_malloc( (max = Lnorm(len))+LSTRHEAD+4, "Lstr" );
      LEN(*s) = 0;
      MAXL(*s) = max;
      TYPE(*s) = STR_TY;
   }
   else
   if (MAXL(*s)-LSTRHEAD < len) {
      *s = (Lstr *) m_realloc( *s, (max=Lnorm(len))+LSTRHEAD+4, "Rstr" );
      MAXL(*s) = max;
   }

   return *s;
}  /* Lfx */

/* ------------------* Licpy *-------------------- */
Lstr *Licpy (Lstr **to, long from )
{
   size_t l;
   Lfx(to, (l=sizeof(from)));
   INT(*to) = from;
   LEN(*to) = l;
   TYPE(*to) = INT_TY;
   return *to;
} /* Licpy */

/* ------------------* Lrcpy *-------------------- */
Lstr *Lrcpy (Lstr **to, double from )
{
   size_t l;
   Lfx(to, (l=sizeof(from)));
   REAL(*to) =  from ;
   LEN(*to) = l;
   TYPE(*to) = REAL_TY;
   return *to;
} /* Lrcpy */

/* ------------------* Lcpy *-------------------- */
Lstr *Lscpy (Lstr **to, char *from )
{
   size_t l;

   if (from==NULL) {
      Lfx(to,0);
      return *to;
   }
   Lfx(to,(l=strlen(from)));
   strcpy( STR(*to), from );
   LEN(*to) = l;
   TYPE(*to) = STR_TY;
   return *to;
} /* Lcpy */

/* ------------------* Lcat *-------append------------- */
Lstr *Lcat (Lstr **to, char *from )
{
   size_t l;

   if (from==NULL) return *to;

   if ((*to)==NULL) return Lscpy( to, from );

   L2str(to);
   Lfx(to,(l=LEN(*to) + strlen(from)));
   strcpy( STR(*to) + LEN(*to), from );
   LEN(*to) = l;
   return *to;
} /* Lcat */

/* ------------------* Lcmp *------------------- */
int   Lcmp ( Lstr *a, char *b )
{
   int r,lb;

   lb = strlen(b);
   if ( (r=memcmp( STR(a), b, MIN(LEN(a),lb))) !=0 )
      return r;
   else {
      if (LEN(a) > lb)
	 return 1;
      else
      if (LEN(a) == lb)
	 return 0;
      else
	 return -1;
   }
}  /* Lcmp */
/* ------------------* Lstrcpy *------------------- */
Lstr *Lstrcpy ( Lstr **to, Lstr *from )
{
   if (from != NULL) {
     
     switch ( TYPE(from) ) {
       case INT_TY:     Lfx(to,8);
                        INT(*to) = INT(from);
                        break;
       case REAL_TY:    Lfx(to,8);
                        REAL(*to) = REAL(from);
                        break;
       case STR_TY:     Lfx(to, LEN(from));
                        memcpy( STR(*to), STR(from), LEN(from) );
                        break;
     }
     TYPE(*to) = TYPE(from);
     LEN(*to) = LEN(from);
   } else {
     Lfx(to,1);
     LEN(*to) = 0;
     TYPE(*to) = STR_TY;
   }
   return *to;
} /* Lstrcpy */

/* ------------------* Lstrcat *------------------- */
Lstr *Lstrcat ( Lstr **to, Lstr *from )
{
   if ((*to)==NULL) return Lstrcpy( to, from );

   L2str(to);
   L2str(&from);

   Lfx(to, LEN(*to) + LEN(from));
   memcpy( STR(*to) + LEN(*to), STR(from), LEN(from) );
   LEN(*to) = LEN(*to) + LEN(from);
   return *to;
} /* Lstrcat */

/* ------------------* Lstrsub *------------------- */
/* WARNING!!! length is size_t type DO NOT PASS A NEGATIVE value */
Lstr *Lstrsub ( Lstr **to, Lstr *from, size_t start, size_t length )
{
   L2str(&from);
   if ((length==0) || (length+start>LEN(from)))
      length = LEN(from) - start;

   if (start<LEN(from)) {
      Lfx(to, length);
      memcpy( STR(*to), STR(from)+start, length );
      LEN(*to) = length;
   } else
      Lscpy(to,"");
   TYPE(*to) = STR_TY;
   return *to;
}  /* Lstrsub */

/* ------------------* Lstrcmp *------------------- */
int Lstrcmp ( Lstr *a, Lstr *b )
{
   int r;

   if ( (r=memcmp( STR(a), STR(b), MIN(LEN(a),LEN(b))))!=0 )
      return r;
   else {
      if (LEN(a) > LEN(b))
	 return 1;
      else
      if (LEN(a) == LEN(b))
	 return 0;
      else
	 return -1;
   }
}  /* Lstrcmp */

/* ------------------* Lstrset *------------------- */
Lstr *Lstrset ( Lstr **to, size_t length, char value)
{
   Lfx(to,length);
   TYPE(*to) = STR_TY;
   LEN(*to) = length;
   memset(STR(*to),value,length);
   return *to;
}  /* Lstrset */

/* --------------* Lstrip *------------------- */
Lstr *Lstrip( Lstr **s )
{
   char *cf,*cl;
   size_t l;

   if (TYPE(*s)!=STR_TY) return (*s);
   l = LEN(*s);
   cf = STR(*s);
   while (l && (*cf==' ')) {cf++; l--;}  /* Search for first non blank char */
   if (!l) {                             /* Only blanks */
      LEN(*s) = 0;
      return *s;
   }
   cl = STR(*s) + (LEN(*s) - 1);
   while (*cl==' ') cl--;   /* No need to check for len, since there is one */
			    /* at least non-blank character */

   l = (size_t)((char HUGE *)cl - (char HUGE *)cf + 1);
   if (l<LEN(*s)) {
      memcpy(STR(*s),cf,l);
      LEN(*s) = l;
   }

   return *s;
}  /* Lstrip */

/* --------------------* index *-------------------------- */
long Lindex(Lstr *haystack, Lstr *needle, size_t p)
{
   size_t   n,lp;

   if (!LEN(needle)) {
     if (!LEN(haystack)) return 0;
     return (!p)?1:p;
   }
   if (LEN(needle) > LEN(haystack)) return 0;
   if (!p) p = 1;
   p--;  lp = p;
   do {
      n = 0; p = lp;
      if (p >= LEN(haystack)) return 0;
      while (STR(haystack)[p] != STR(needle)[0]) {
         p++;
         if (p >= LEN(haystack)) return 0;
      }
      lp = p+1;
      while ( (STR(haystack)[p]==STR(needle)[n]) && (n<LEN(needle))) {
         if ((++n) >= LEN(needle)) return lp;
         p++;
         if (p >= LEN(haystack)) return 0;
      }
   } while (n<LEN(needle));
   return lp;
} /* index */
/* ------------------------* verify *----------------- */
long Lverify( Lstr *str, Lstr *ref, char match, size_t s )
{
   boolean found;

   if (s > LEN(str)) return -1;

   for (; s<LEN(str); s++) {
     found = (memchr(STR(ref), STR(str)[s], LEN(ref)) == NULL);
     if (found ^ match) return s;
   }
   return -1;
} /* Lverify */
