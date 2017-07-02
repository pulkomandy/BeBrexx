/*
 * $Header: c:/usr/rexx/VARIABLE.C!v 1.2 1997/07/04 20:32:32 bill Exp bill $
 * $Log: VARIABLE.C!v $
 * Revision 1.2  1997/07/04  20:32:32  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11 03:32:32  bill
 * Initial revision
 *
 */

#include <string.h>

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "lstring.h"
#include "variable.h"
#include "convert.h"

/* -----------------* V_create *------------------ */
Idrec *V_create( int isarray, Lstr *name, Idrec **Scope )
{
   if (isarray)  return C_create(name,Scope);
   else          return createId(name,Scope);
}  /* V_create */

/* -----------------* V_search *------------------ */
Idrec *V_search( int isarray, Lstr *name, Idrec *Scope )
{
   if (isarray)  return C_search(name,Scope);
   else          return searchId(name,Scope);
}  /* V_search */

/* --------------------* V_eval *------------------ */
Lstr *V_eval ( int isarray, Lstr **to, Lstr *name, Idrec *Scope )
{
   if (isarray)  return C_eval(to,name,Scope);
   else          return evalId(to,name,Scope);
}  /* V_eval */

/* -------------------------------------------------------------- */
/* Search the 'Name'  identifier... if not found then             */
/* create a new one as string with value 'Name'                   */
/* return PTR pointing that record                                */
/* -------------------------------------------------------------- */
Idrec *createId( Lstr *name, Idrec **Scope)
{
   Idrec     *ThisEntry;
   Idrec     *LastEntry;
   Idrec     *ptr;
   boolean    LeftTaken;
   int        r;

   ThisEntry = *Scope;
   while (ThisEntry != NULL) {
      LastEntry = ThisEntry;
      r = Lstrcmp(ThisEntry->name, name);
      if ( r > 0 ) {
         ThisEntry = ThisEntry->left;
         LeftTaken = TRUE;
      } else
      if ( r < 0 ) {
         ThisEntry = ThisEntry->right;
         LeftTaken = FALSE;
      } else {
     ptr = ThisEntry;
     return ptr;
      }
   }       /* else create a new entry */

   ptr = m_malloc( sizeof(Idrec), "VREC" );
   ptr->name = NULL;
   Lstrcpy(&(ptr->name), name);
   ptr->var = NULL;
   ptr->ara = NULL;
   ptr->left = NULL;
   ptr->right = NULL;

   if (*Scope == NULL)
    *Scope = ptr;
   else {
      if (LeftTaken) LastEntry->left = ptr;
        else LastEntry->right = ptr;
   }
   return ptr;
}

/* -------------------------------------------------------------- */
/* Search the 'Name'  identifier and return PTR pointing          */
/* the variable record.                                           */
/* return PTR -> NULL                                             */
/* -------------------------------------------------------------- */
Idrec *searchId ( Lstr *name, Idrec *Scope )
{
   Idrec  *ptr;
   int    r;

   ptr  = Scope;
   while (ptr != NULL ) {
      r = Lstrcmp(ptr->name, name);
      if (r > 0) ptr = ptr->left;
      else
      if (r < 0) ptr = ptr->right;
      else
     return ptr;
   }
   return NULL;
} /* searchId */

/* -------------------------------------------------------------- */
/* Search the 'name'  identifier and return a new allocated Lstr  */
/* with its value. If the variable is not found then 'name' in    */
/* Lstring is returned.                                           */
/* -------------------------------------------------------------- */
Lstr *evalId ( Lstr **to, Lstr *name, Idrec *Scope )
{
   Idrec  *ptr;
   int    r;
   Lstr   *NOVAL=NULL;
   Label  *Lab;
   Lstr   *retcode=NULL;
   Idrec  *retc;

   ptr  = Scope;
   while (ptr != NULL ) {
      r = Lstrcmp(ptr->name, name);
      if (r > 0) ptr = ptr->left;
      else
      if (r < 0) ptr = ptr->right;
      else {
     Lstrcpy(to,ptr->var);
         return *to;
      }
   }
   if (Novalue) {
      Lscpy(&NOVAL,"NOVALUE");
      Lab = searchlabel( NOVAL, &label_scope );
      m_free(NOVAL);

      Lscpy(&retcode,"RC");  retc = createId(retcode,&scope);
      Lstrcpy(&(retc->var),name);
      m_free(retcode);

      Signal(Lab);
   }
   Lstrcpy(to, name);
   return *to;
} /* evalId */

/* ----------------------------------------------------- o o ---- */
/*                                                     ____oo_    */
/*                                          (c)1992   /+     |\   */
/*  Compound Variable Names                   BNV      +     |    */
/*  ~~~~~~~~~~~~~~~~~~~~~~~                            '.___.'    */
/*                                                     MARMITA    */
/*  NAME may be "compound" in that it may be composed of          */
/*  several parts (seperated by periods) some of which may have   */
/*  variable values. The parts are then substituted independently,*/
/*  to generate a fully resolved name. In general,                */
/*                                                                */
/*  s0.s1.s2.---.sn         is substituted to form                */
/*                                                                */
/*  d0.v1.v2.---.vn         where d0 is uppercase of s0, and      */
/*                          v1-vn are values of s1-sn             */
/*                                                                */
/*  This facility may be used for traditional arrays,             */
/*  content-addressable arrays, and other indirect addressing     */
/*  modes. As an example, the sequence:                           */
/*                                                                */
/*    J = 5;  a.j = "fred";                                       */
/*                                                                */
/*  would assign "fred" to the variable "A.5".                    */
/*                                                                */
/*  The stem of NAME (i.e. that part up to and including the      */
/*  first ".") may be specified on the DROP and PROCEDURE         */
/*  EXPOSE instructions and affect all variables starting with    */
/*  that stem. An assigment to a stem assigns the new value to    */
/*  all possible variables with that stem.                        */
/*                                                                */
/* -------------------------------------------------------------- */
Lstr *C_compound_substitute( Lstr **d, Lstr *s, Idrec *Scope)
{
   char    *p1, *p2;
   size_t   l,l1=0,l2;
   Lstr    *subn=NULL;
   Lstr    *subv=NULL;
   boolean  _NOVALUE;

   l = LEN(s);
   p1 = memchr( STR(s), '.', (size_t)l);
   l2 = (size_t)p1-(size_t)STR(s) + 1;
   l -= l2;
   Lfx(d,LEN(s));
   memcpy(STR((*d)),STR(s), (size_t)l2);
   LEN((*d)) = l2;

   stem = (l2==LEN(s));
   STR((*d))[ l2 ] = '\0';      /* Make it ASCIIZ string */
   ArrayHead = searchId( *d, Scope );

   p2 = p1;

   _NOVALUE = Novalue;
   Novalue = OFF;
   do {
      p1 = p2 + 1;
      l1 = LEN(s)-l;
      p2 = memchr( p1, '.', (size_t)l);
      if (p2==NULL) p2 = STR(s) + LEN(s);
      l2 = (size_t)p2 - (size_t)p1;
      l -= l2;

      if (l2!=0) {
         Lstrsub(&subn, s, l1, l2);
         STR(subn)[LEN(subn)] = '\0';
         evalId(&subv,subn,Scope);
         Lstrcat(d,subv);
      }
      if (l)  {
         STR((*d)) [ LEN((*d)) ] = '.';
         LEN((*d)) += 1;
         l--;
      }
   } while (l);
   Novalue = _NOVALUE;
   m_free(subv);
   m_free(subn);
   return *d;
}  /* C_compound_substitue */

/* -------------------------------------------------------------- */
/*  Create a new stem variable and return Aptr pointing that      */
/*  variable                                                      */
/* -------------------------------------------------------------- */
Idrec *C_create( Lstr *name, Idrec **Scope)
{
    Lstr   *id=NULL;
    Lstr   *d=NULL;
    char   *p;
    size_t  i;
    Idrec  *A;

   C_compound_substitute(&id, name, *Scope);
   if (ArrayHead == NULL) {
      if (stem)  Lstrcpy(&d,id);
      else {
         p = memchr(STR(id),'.',LEN(id));
         i = (size_t)p - (size_t)STR(id) + 1;
         Lstrsub(&d, id, 0, i);
      }
      ArrayHead = createId(d,Scope);
   }
   if (tracing == intermediates_trace) tracevar(">C>", id);
   if (stem)  A = ArrayHead;
   else {
      A = searchId( id, ArrayHead->ara );
      if (A==NULL) {
         A = createId( id, &(ArrayHead->ara) );
         if ((A->var==NULL) && (ArrayHead->var!=NULL))
            Lstrcpy(&(A->var), ArrayHead->var);
      }
   }
   m_free(id);
   m_free(d);
   return A;
}   /* C_create */
/* -------------------------------------------------------------- */
/*  Search for NAME in Scope and return a pointer to  that        */
/*  variable.                                                     */
/* -------------------------------------------------------------- */
Idrec *C_search ( Lstr *name, Idrec *Scope )
{
   Lstr   *id=NULL;
   Idrec  *A;

   C_compound_substitute (&id, name, Scope);
   if (ArrayHead == NULL) A = NULL;
   else {
      if (tracing == intermediates_trace) tracevar(">C>", id);
      if (stem)  A = ArrayHead;
      else       A = searchId( id, ArrayHead->ara );
   }
   m_free(id);
   return A;
}  /* C_search */

/* -------------------------------------------------------------- */
/* Search the 'name'  identifier and return a new allocated Lstr  */
/* with its value. If the variable is not found then 'name' in    */
/* Lstring is returned.                                           */
/* -------------------------------------------------------------- */
Lstr *C_eval ( Lstr **to, Lstr *name, Idrec *Scope )
{
   Lstr   *id=NULL;
   Idrec  *A;
   Lstr   *NOVAL=NULL;
   Label  *Lab;
   Lstr   *retcode=NULL;
   Idrec  *retc;
   int     found=FALSE;

   C_compound_substitute (&id, name, Scope);
   if (ArrayHead == NULL) Lstrcpy(to,id);
   else {
      if (stem) {
         found = TRUE;
         evalId(to, name, Scope);
      } else {
         A = searchId(id,ArrayHead->ara);
         if (A==NULL) {
             if (ArrayHead->var != NULL) {
                found = TRUE;
                Lstrcpy(to, ArrayHead->var);
             } else
                Lstrcpy(to, id);
         } else {
            found = TRUE;
            Lstrcpy (to, A->var);
         }
      }
   }
   m_free(id);
   if (Novalue && !found) {
      Lscpy(&NOVAL,"NOVALUE");
      Lab = searchlabel( NOVAL, &label_scope );
      m_free(NOVAL);
      Lscpy(&retcode,"RC");  retc = createId(retcode,&scope);
      Lstrcpy(&(retc->var),*to);
      m_free(retcode);
      Signal(Lab);
   }
   return *to;
}  /* C_eval */

/* -------------------------------------------------------------- */
void getbintree( Lstr **A, int dep, int option, Idrec *head )
{
   Lstr  *B=NULL;

   if (head == NULL) return;
   getbintree( A, dep+1, option, head->right);

   if (option) {
      Licpy(&B,dep);  Lstrcat(A,B);
      m_free(B);      Lcat(A," ");
   }
   Lstrcat(A, head->name);       Lcat(A," = ");
   if (head->var != NULL) {
      Lcat(A,"\"");
      Lstrcat(A,head->var);
      Lcat(A,"\"\n");
   } else Lcat(A,"NULL\n");
   getbintree( A, dep+1, option, head->left);
} /* printbintree */

/* -------------------------------------------------------------- */
/*                  Dispose  a  specified id.                     */
/* -------------------------------------------------------------- */
void disposeId( Idrec *thisid )
{
   if (thisid == NULL) return;
   if (thisid->name != NULL) m_free(thisid->name);
   if (thisid->var  != NULL) m_free(thisid->var);
   if (thisid->ara  != NULL) disposetree( thisid->ara );
   m_free(thisid);
} /* disposeId */
/* -------------------------------------------------------------- */
/*                  Dispose  a  binary  tree.                     */
/* -------------------------------------------------------------- */
void disposetree( Idrec *root )
{
   if (root != NULL) {
      disposetree(root->left);
      disposetree(root->right);
      disposeId(root);
   }
} /* disposetree */
/* -------------------------------------------------------------- */
/* Search the 'Name'  identifier...                               */
/* and delete it from Local Scope                                 */
/* Ž«˜¤ › ˜š¨˜­¦¬£œ œ¤˜ §¦ ¤«œ¨ ˜§¦ «¦ ›œ¤«¨¦ ›œ¤ §¨œ§œ  ¤˜       */
/* ˜¢¢˜¥¦¬£œ «ž¤ ›¦£ž «¦¬ ›œ¤«¨¦¬! ‚ ˜ ¤˜ «¦ §œ«¬®¦¬£œ ˜¬«¦       */
/* ˜¤« ¡˜Ÿ ©«¦¬£œ «¦¤ §¦ä¤«œ¨ §¦¬ §¨œ§œ  ¤˜ ©™¬©¦¬£œ £œ «¦¤       */
/* ˜£œ©àª §¦ä¤«œ¨ £œ «ž¤ £ ¡¨¦«œ¨ž « £ž, ›ž¢˜›ž £œ «¦¤            */
/* ›œ¥ ¦«œ¨¦ ˜§¦ «¦ ˜¨ ©«œ¨¦ ¬§¦-›œ¤«¨¦. Ž§àª ›œ ®¤œ  §˜¨˜¡˜«à    */
/* «¦ ©®ž£˜...                                                    */
/* -------------------------------------------------------------- */
/*                     ...                                        */
/*                    /                                           */
/*                 (name)   <-- to be dropped                     */
/*                 /    \                                         */
/*              (a)      (d)       (c)= dominator the leftmost    */
/*             /   \       ...          one, where b^.right=NIL   */
/*          ...     (c)            (a)= predo (before dominaror)  */
/*                 /   \                                          */
/*              (b)     NIL       dominator will become the new   */
/*             ...                sub-head when (name) is dropped */
/*                    |                                           */
/*                    |                                           */
/*                  \ | /                                         */
/*                    \/  ...                                     */
/*                      /                                         */
/*                    (c)           but in the case that          */
/*                   /   \          (a)=(c) when a^.right = NIL   */
/*                 (a)    (d)       then the tree is very simple  */
/*                /   \    ....     we simply replace a^.right=d  */
/*              ...   (b)                                         */
/*                    ....                                        */
/*                                                                */
/* -------------------------------------------------------------- */
void drop( Lstr *name, Idrec **head )
{
   Idrec  *previous,  *predo,  *thisid, *dominator;
   int    lefttaken;
   int    cmp;

   thisid = *head;
   while (thisid != NULL) {
    if ((cmp=Lstrcmp(name,thisid->name)) < 0) {
      previous = thisid;
      thisid = thisid->left;
      lefttaken = TRUE;
    } else
    if (cmp > 0) {
      previous = thisid;
      thisid = thisid->right;
      lefttaken = FALSE;
    } else {
       if (thisid->right == NULL) dominator = thisid->left;
       else
       if (thisid->left  == NULL) dominator = thisid->right;
       else {           /* when no node is empty */
          predo = thisid;            /* find the right most id of the */
          dominator = thisid->left;  /* left branch of thisid         */
          while (dominator->right != NULL) {
             predo = dominator;
             dominator = dominator->right;
          }

          dominator->right = thisid->right;

          if (predo != thisid) {
             predo->right = dominator->left;
             dominator->left = thisid->left;
          }
       }

       if (thisid == *head)   *head = dominator;
       else {
          if (lefttaken) previous->left  = dominator;
                    else previous->right = dominator;
       }
       disposeId(thisid);
       thisid=NULL;
    }
  }
}  /* drop */

/* ----------------------* V_drop *------------------------ */
void V_drop( int isarray, Lstr *name, Idrec **head )
{
   Lstr  *sub;         /* NULLED BELOW */

   if (isarray) {
      sub = NULL;
      C_compound_substitute (&sub, name, *head);
      if (stem) drop( name, head );
      else      drop( sub , &(ArrayHead->ara) );
      m_free(sub);
   } else
      drop(name,head);
}  /* V_drop */
