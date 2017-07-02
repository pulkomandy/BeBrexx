/*
 * $Header: common.h!v 1.1 1995/09/11 03:36:09 bill Exp bill $
 *
 * $Log: common.h!v $
 * Revision 1.1  1995/09/11 03:36:09  bill
 * Initial revision
 *
 * Revision 1.1.1.1  1992/11/13  23:26:28  bill
 * Some more added.
 *
 * Revision 1.1  1992/11/13  23:22:01  bill
 * Initial revision
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

/* ----------- some global data type definitions ------------------ */
typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long  dword;

typedef int boolean;

/* ---------------- comonly used definitions ----------------- */
#define TRUE  1
#define FALSE 0
#define ON    1
#define OFF   0
#define YES   1
#define NO    0

/* ------------------ comonly used macros -------------------- */
#define MAX(a,b)  (((a)>(b))?(a):(b))
#define MIN(a,b)  (((a)<(b))?(a):(b))
#define RANGE(a,x,b)  (((x)<(a))?(a): ( ((x)>(b))?(b):(x)))
#define IN_RANGE(a,x,b)  (((a) <= (x)) && ((x) <= (b)))

#define HEXVAL(x) (((x)>='A')?((((x)>='a')?((x)&(0xdf)):(x))-'A'+10):((x)-'0'))
#define CTL(A)    ((A) & 0x1f)
#define SQR(x)    ((x)*(x))

#endif
