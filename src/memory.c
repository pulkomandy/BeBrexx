/*
 * $Header: c:/usr/rexx/MEMORY.C!v 1.2 1997/07/04 20:32:14 bill Exp bill $
 * $Log: MEMORY.C!v $
 * Revision 1.2  1997/07/04  20:32:14  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11 03:31:36  bill
 * Initial revision
 *
 */

#include <string.h>
#include <stdlib.h>

#ifdef __MSDOS__
#include <alloc.h>
#elif defined(__MPW__)
#else
#include <malloc.h>
#endif

#include "rexx.h"
#include "error.h"

#ifdef __MSDOS__
#  define MALLOC(MM)      farmalloc(MM)
#  define REALLOC(PP,MM)  farrealloc(PP,MM)
#  define FREE(PP)        farfree(PP)
#else
#  define MALLOC(MM)      malloc(MM)
#  define REALLOC(PP,MM)  realloc(PP,MM)
#  define FREE(PP)        free(PP)
#endif


#define MAGIC_NUMBER  'b'

/* ---------------------* *------------------- */

typedef struct memblock_st memblock;
struct memblock_st {
       size_t count;
#ifdef DEBUGING
       char   magic;
       char   type[5];
#endif
       struct memblock_st *prev;
       struct memblock_st *next;
} ;

#ifdef DEBUGING
static memblock *Mhead=NULL;
static memblock *Mtail=NULL;
#endif

/* -----------------* m_malloc *-------------- */
#ifdef DEBUGING
void *m_Malloc( size_t size, char *type )
{
   memblock *ptr;
   size_t   count;

   count = sizeof(memblock) + size;
   if ((ptr = MALLOC(count))==NULL) error(ERR_STORAGE_EXHAUSTED);
   if (Mtail != NULL) Mtail->next = ptr;
   ptr->count = count;
   ptr->next = NULL;
   ptr->prev = Mtail;
   Mtail = ptr;
   if (Mhead==NULL) Mhead = ptr;

   ptr->magic = MAGIC_NUMBER;
   strcpy(ptr->type,type);

   return (void *)(ptr+1);
}  /* m_Malloc */
#else
void *m_Malloc( size_t size)
{
   void *ptr;
   if ((ptr = MALLOC(size))==NULL) error(ERR_STORAGE_EXHAUSTED);
   return ptr;
}
#endif

/* --------------------* m_realloc *--------------- */
#ifdef DEBUGING
void *m_Realloc( void *block, size_t size, char *type )
{
   int  h,t;
   memblock *prv;
   memblock *nxt;
   memblock *ptr;

   if (block==NULL)  return m_malloc(size,type);
   ptr = (memblock *)block - 1;
   prv = ptr->prev;
   nxt = ptr->next;
   h = (ptr==Mhead);
   t = (ptr==Mtail);
   if (ptr->magic != MAGIC_NUMBER) error( ERR_INTERPRETER_FAILURE );
   size = size + sizeof(memblock);
   if ((ptr = REALLOC(ptr,size))==NULL) error(ERR_STORAGE_EXHAUSTED);
   ptr->count = size;
   if (prv!=NULL) prv->next = ptr;
   if (nxt!=NULL) nxt->prev = ptr;
   if (h) Mhead = ptr;
   if (t) Mtail = ptr;
   return (void *)(ptr + 1);
}  /* m_Realloc */
#else
void *m_Realloc( void *block, size_t size )
{
   if ((block = REALLOC(block,size))==NULL) error(ERR_STORAGE_EXHAUSTED); 
   return block;
}
#endif


/* --------------------* m_Free *----------------- */
void m_Free ( void *p )
{
#ifdef DEBUGING
    memblock *ptr;
    memblock *prv, *nxt;

    if (p == NULL) return;

    ptr = (memblock *)p - 1;
    if (ptr->magic != MAGIC_NUMBER) error( ERR_INTERPRETER_FAILURE );
    ptr->magic = '0';

    prv = ptr->prev;
    nxt = ptr->next;

    if (prv != NULL)  prv->next = nxt;
    if (nxt != NULL)  nxt->prev = prv;

    if (ptr == Mhead) Mhead = nxt;
    if (ptr == Mtail) Mtail = prv;
    FREE(ptr);
#else
    if (p != NULL) free(p);
#endif
} /* m_Free */
/* -------------------* m_mark *------------------- */
/* allocates only the header                        */
/* ------------------------------------------------ */
#ifdef DEBUGING
void *m_Mark( void )
{
   memblock *ptr;
   ptr = m_malloc(1,"MARK");
   return (void *)(ptr - 1);
} /* m_Mark */

/* ------------------* m_release *----------------- */
/* releases all memory blocks allocated after the   */
/* specified mark point                             */
/* ------------------------------------------------ */
void m_Release( void *mark )
{
    memblock *prv;
    memblock *tail;

    tail = Mtail;
    while (tail != (memblock *)mark) {
       prv = tail->prev;
       if (tail->magic != MAGIC_NUMBER) {
          bprintf("Magic number failure \'%c\'\n",tail->magic);
          error(ERR_INTERPRETER_FAILURE);
       }
       m_Free( tail + 1 );
       tail = prv;
    }
    if (tail==(memblock *)mark)
           m_Free(tail + 1);
}  /* m_Release */

void printmemlist ( void )
{
   memblock *ptr;
   int      i;

   ptr = Mhead;

   i = 0;
   while (ptr != Mtail ) {
      bprintf("%3d  type = \"%s\"  size = %4d\n",++i, ptr->type,
                                              ptr->count);
      if (!strcmp(ptr->type,"Lstr"))
          tracevar("-->",(Lstr *)(ptr+1));
      if (!strcmp(ptr->type,"VREC")) {
          tracevar("-n>", ((Idrec *)(ptr+1))->name);
          tracevar("-v>", ((Idrec *)(ptr+1))->var);
      }

      if (ptr->magic != MAGIC_NUMBER) {
         bprintf("Magic number failure \'%c\'\n",ptr->magic);
         error(ERR_INTERPRETER_FAILURE);
      }
      ptr = ptr->next;
   }
} /* printmemlist */
#endif
