/*
 * $Header: utils.h!v 1.1 1995/09/11 03:37:04 bill Exp bill $
 * $Log: utils.h!v $
 * Revision 1.1  1995/09/11 03:37:04  bill
 * Initial revision
 *
 */

#ifndef __UTILS_H__
#define __UTILS_H__

/* --------------------* function prototypes *----------------- */
args  *mallocargs( void );
void   freeargs( args *arg );
Label *addlabel( Lstr *lab, Label **label_Scope );
Label *searchlabel( Lstr *Lab, Label **label_Scope );
void   mustbe (enum symboltype sym, int err );
void   setvar( char *name, long value );
void   set_trace( char *str );
Lstr  *readline ( Lstr **A );
void   push_program_status ( void );
void   pop_program_status ( void );

Lstrlist *queue_Lstr( Lstr *, Lstrlist **, Lstrlist **);
Lstrlist *push_Lstr( Lstr *, Lstrlist **, Lstrlist **);
Lstr     *pop_Lstr( Lstrlist **, Lstrlist **);

#endif
