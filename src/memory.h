/*
 * $Header: memory.h!v 1.1 1995/09/11 03:36:23 bill Exp bill $
 * $Log: memory.h!v $
 * Revision 1.1  1995/09/11 03:36:23  bill
 * Initial revision
 *
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdio.h>

void  m_Free ( void *p );
void *m_Mark( void );
void  m_Release( void *mark );

#ifdef DEBUGING

void *m_Malloc( size_t size, char *type );
void *m_Realloc( void *block, size_t size, char *type );
#   define m_malloc(s,t)     m_Malloc(s,t)
#   define m_realloc(b,s,t)  m_Realloc(b,s,t)
#   define m_mark()             m_Mark()
#   define m_release(M)         m_Release(M)
void printmemlist(void);

#else

void *m_Malloc( size_t size );
void *m_Realloc( void *block, size_t size );
#   define m_malloc(s,t)     m_Malloc(s)
#   define m_realloc(b,s,t)  m_Realloc(b,s)
#   define m_mark()          NULL
#   define m_release(M)
#endif

#define m_free(AA)           { m_Free((AA)); (AA) = NULL; }

#endif
