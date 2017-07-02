/*
 * $Header: convert.h!v 1.1 1995/09/11 03:36:15 bill Exp bill $
 * $Log: convert.h!v $
 * Revision 1.1  1995/09/11 03:36:15  bill
 * Initial revision
 *
 */
#ifndef __CONVERT_H__
#define __CONVERT_H__

void  init_ul ( void );

char  datatype( Lstr *s );

Lstr *L2INT   ( Lstr **to );
Lstr *L2REAL  ( Lstr **to );
Lstr *L2int   ( Lstr **to );
Lstr *L2real  ( Lstr **to );
Lstr *L2num   ( Lstr **to );
Lstr *L2str   ( Lstr **to );
Lstr *L2upper ( Lstr  *to );
Lstr *L2lower ( Lstr  *to );
Lstr *Lx2c    ( Lstr **to, Lstr *from );
Lstr *Lreverse( Lstr *A );


#ifndef __CONVERT_C__
extern char u2l[256];
extern char l2u[256];
#endif

#endif
