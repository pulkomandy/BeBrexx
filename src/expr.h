/*
 * $Header: expr.h!v 1.1 1995/09/11 03:36:39 bill Exp bill $
 * $Log: expr.h!v $
 * Revision 1.1  1995/09/11 03:36:39  bill
 * Initial revision
 *
 */

#ifndef __EXPR_H__
#define __EXPR_H__

int   O_bool (Lstr *A);

Lstr *O_eq   (Lstr *A, Lstr *B);
Lstr *O_deq  (Lstr *A, Lstr *B);
Lstr *O_ne   (Lstr *A, Lstr *B);
Lstr *O_nde  (Lstr *A, Lstr *B);
Lstr *O_gt   (Lstr *A, Lstr *B);
Lstr *O_ge   (Lstr *A, Lstr *B);
Lstr *O_lt   (Lstr *A, Lstr *B);
Lstr *O_le   (Lstr *A, Lstr *B);

Lstr *O_concat (Lstr *A, Lstr *B);
Lstr *O_bconcat(Lstr *A, Lstr *B);

Lstr *O_add (Lstr *A, Lstr *B);
Lstr *O_sub (Lstr *A, Lstr *B);

Lstr *O_mult    (Lstr *A, Lstr *B);
Lstr *O_div     (Lstr *A, Lstr *B);
Lstr *O_intdiv  (Lstr *A, Lstr *B);
Lstr *O_mod     (Lstr *A, Lstr *B);

Lstr *O_expose (Lstr *A, Lstr *B);

Lstr *O_minus (Lstr *A);
Lstr *I_function (Lstr *A);
Lstr *I_expression(Lstr *A);

#endif
