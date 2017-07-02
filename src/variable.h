/*
 * $Header: variable.h!v 1.1 1995/09/11 03:36:31 bill Exp bill $
 * $Log: variable.h!v $
 * Revision 1.1  1995/09/11 03:36:31  bill
 * Initial revision
 *
 */

#ifndef __VARIABLE_H__
#define __VARIABLE_H__

/* ----------- function prototypes for variable.c ------------ */
Idrec *V_create( int isarray, Lstr *name, Idrec **Scope );
Idrec *V_search( int isarray, Lstr *name, Idrec *Scope );
Lstr  *V_eval ( int isarray, Lstr **to, Lstr *name, Idrec *Scope );

Idrec *createId( Lstr *name, Idrec **Scope);
Idrec *searchId ( Lstr *name, Idrec *Scope );
Lstr  *evalId ( Lstr **to, Lstr *name, Idrec *Scope );

Lstr  *C_compound_substitute( Lstr **d, Lstr *s, Idrec *Scope);
Idrec *C_create ( Lstr *name, Idrec **Scope);
Idrec *C_search ( Lstr *name, Idrec *Scope );
Lstr  *C_eval   ( Lstr **to, Lstr *name, Idrec *Scope );

void   V_drop( int isarray, Lstr *name, Idrec **head);

void   disposeId   ( Idrec *thisid );
void   disposetree ( Idrec *root );

void getbintree( Lstr **, int, int, Idrec * );

#endif
