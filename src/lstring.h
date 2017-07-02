/*
 * $Header: lstring.h!v 1.1 1995/09/11 03:35:40 bill Exp bill $
 * $Log: lstring.h!v $
 * Revision 1.1  1995/09/11 03:35:40  bill
 * Initial revision
 *
 */

#ifndef __LSTRING_H__
#define __LSTRING_H__

enum   TYPES { INT_TY, REAL_TY, STR_TY };

/* Structure for length prefixed strings used in REXX */
typedef struct lstr {
       size_t   l;
       size_t   m;
       char     t;
       union  {
          long     i;
          double   r;
          char     s[11];
       } v;
} Lstr;

/* !!! WARNING !!! assign the correct value to LSTRHEAD below
       according to sizeof(size_t)    */
#define LSTRHEAD (sizeof(size_t) + sizeof(size_t) + sizeof(char))

#define  LEN(L)     ((L)->l)
#define  MAXL(L)    ((L)->m)
#define  TYPE(L)    ((L)->t)
#define  INT(L)     ((L)->v.i)
#define  REAL(L)    ((L)->v.r)
#define  STR(L)     ((L)->v.s)

#define  ASCIIZ(s)  {STR(s)[LEN(s)] = '\0';}

/*#define  Lnorm(size) ( (size & (~0x0f)) + 16 )*/
#define  Lnorm(size) ( (size) | 0x0000000f)


/* --------------- function prototypes --------------- */
Lstr  *Lfx ( Lstr **s, size_t len );
Lstr  *Licpy (Lstr **to, long from );
Lstr  *Lrcpy (Lstr **to, double from );
Lstr  *Lscpy (Lstr **to, char *from );
Lstr  *Lcat  (Lstr **to, char *from );
int    Lcmp  ( Lstr *a, char *b );
Lstr  *Lstrcpy ( Lstr **to, Lstr *from );
Lstr  *Lstrcat ( Lstr **to, Lstr *from );
Lstr  *Lstrsub ( Lstr **to, Lstr *from, size_t start, size_t length );
Lstr  *Lstrset ( Lstr **to, size_t length, char value);
int    Lstrcmp ( Lstr *a, Lstr *b );
Lstr  *Lstrip( Lstr **s );
long   Lindex(Lstr *haystack, Lstr *needle, size_t p);
long   Lverify( Lstr *str, Lstr *ref, char match, size_t s );

#endif
