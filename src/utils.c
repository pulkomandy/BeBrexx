/*
 * $Header: c:/usr/rexx/UTILS.C!v 1.2 1997/07/04 20:32:07 bill Exp bill $
 * $Log: UTILS.C!v $
 * Revision 1.2  1997/07/04  20:32:07  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11 03:31:27  bill
 * Initial revision
 *
 */

#include <string.h>

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "lstring.h"
#include "variable.h"

typedef struct status_queue_st status_queue;
struct status_queue_st {
       status_queue *prev;
       struct rexx_vars_st v;
};

static status_queue *pgm_status=NULL;

/* -------------------* mallocargs *---------------------- */
/* args *mallocargs(void)                                  */
/* allocates some space for arguments, initializes them    */
/* and returns the argument pointer                        */
/*-------------------------------------------------------- */
args *mallocargs( void )
{
   int i;
   args *arg;
   arg = m_malloc(sizeof(args),"ARGS");
   arg->n = 0;
   arg->r = NULL;
   for (i=0; i<maxargs; i++)  arg->a[i] = NULL;
   return arg;
} /* mallocargs */
/* --------------------* freeargs *----------------------- */
/* void  *freeargs(void)                                   */
/*-------------------------------------------------------- */
void freeargs( args *arg )
{
   int i;
   if (arg == NULL) return;
   m_free(arg->r);
   for (i=0; i<maxargs; i++)  m_free(arg->a[i]);
   m_free(arg);
} /* freeargs */

/* ----------------------* addlabel *------------------------ */
/* addlabel(Lstr *lab, char *ptr, Label **label_Scope);       */
/* adds a label to label binary tree                          */
/* ---------------------------------------------------------- */
Label *addlabel( Lstr *lab, Label **label_Scope )
{
   Label    *ThisLabel, *LastLabel;
   boolean  LeftTaken;
   int      cmp;

   ThisLabel = *label_Scope;
   while (ThisLabel != NULL) {
      LastLabel = ThisLabel;
      if ( (cmp=Lstrcmp(ThisLabel->name, lab)) > 0 ) {
         ThisLabel = ThisLabel->left;
         LeftTaken = TRUE;
      } else
      if ( cmp < 0 ) {
         ThisLabel = ThisLabel->right;
         LeftTaken = FALSE;
      }
      else return ThisLabel ;
   }

   ThisLabel = (Label *) m_malloc( sizeof(Label) ,"LAB");
   ThisLabel->name = NULL;
   Lstrcpy(&(ThisLabel->name), lab);
   ThisLabel->ptr = Bptr;    /* Current program pointer /// Maybe Pptr? */
   ThisLabel->nnn = Blineno;
   ThisLabel->left = NULL;
   ThisLabel->right = NULL;

   if (*label_Scope == NULL) *label_Scope = ThisLabel;
   else {
      if (LeftTaken)  LastLabel->left  = ThisLabel;
      else            LastLabel->right = ThisLabel;
   }
   return ThisLabel;
}   /* addlabel */


/* ------------------------------------------------ (c)BNV 1992 - */
/*  Search for a specified Label in the binary tree               */
/*  return the pointer in the execfile where the label was found  */
/*  return NULL if label is not found!                            */
/* -------------------------------------------------------------- */
Label *searchlabel( Lstr *Lab, Label **label_Scope )
{
   int       cmp;
   char     *pp;
   Label    *ThisLabel;
   int       oldflag,lno;

   ThisLabel = *label_Scope;
   while (ThisLabel != NULL) {
      if ((cmp=Lstrcmp(ThisLabel->name, Lab)) > 0)
         ThisLabel = ThisLabel->left;
      else
      if (cmp<0)
         ThisLabel = ThisLabel->right;
      else
         return ThisLabel;
   }

   /* no search is done when we are in a execloaded file */
   if (file_type != COMMAND) return NULL;

   /* If label is not in the label tree, then search in the program */
   if (lastlabelptr == NULL) {
      lastlabelptr = orig_exec;
      lastlineno = 1;
   };
   push_program_status();

   RxPtr = lastlabelptr;
   lineno = lastlineno;

   oldflag = flag;
   flag = f_stopped;
   statement = nothing_st;
   do {
      do {
         Bptr = RxPtr; Blineno = lineno;
         nextsymbol();
         if (symbol==exit_sy)  { ThisLabel = NULL; goto fin; }
      } while (symbol != label_sy);
      ThisLabel = addlabel(symbolstr, label_Scope);
   }  while (Lstrcmp(symbolstr,Lab));
fin:
   flag = oldflag;

   pp = RxPtr;    lno = lineno;
   pop_program_status();
   lastlabelptr = pp;
   lastlineno = lno;

   return ThisLabel;
}  /* searchlabel */

/* ----------------------* mustbe *-------------------------- */
/* mustbe(symbol,errorno)                                     */
/* if current symbol is equal to 'symbol' then get nextsymbol */
/* and continue, otherwise call an error(errorno)             */
/* ---------------------------------------------------------- */
void mustbe( enum symboltype sym, int err )
{
   if (symbol==sym) nextsymbol();
   else
      error(err);
}  /* mustbe */

/* ---------------------------------------------------------- */
/* setvar(char *name, long value )                            */
/* creates a new variable with name 'name' and value the      */
/* integer value of 'value'                                   */
/* ---------------------------------------------------------- */
void setvar( char *name, long value )
{
    Lstr   *varname=NULL;
    Idrec  *nameId;

    Lscpy(&varname,name);
    nameId = createId(varname,&scope);
    Licpy(&(nameId->var),value);
    m_free(varname);
} /* setvar */
/* --------------------- set_trace -------------------------- */
/* set_trace( char *str )                                     */
/* sets the current tracing options according to str          */
/* ---------------------------------------------------------- */
void set_trace( char *str )
{
   int     i;
   boolean more;

   i = 0 ; more = TRUE;
   while (str[i] && more)
     if (str[i] == '?') {
        interactive_debug = !interactive_debug;
        if (interactive_debug)
           bputs(interactive_msg);
           i++;
        } else
        if (str[i] == '!') {
           command_inhibition = !command_inhibition;
           i++;
        } else
           more = FALSE;

   if (!more)
      switch (l2u[str[i]])  {
        case 'A' : tracing = all_trace;              break;
        case 'C' : tracing = commands_trace;         break;
        case 'E' : tracing = error_trace;            break;
        case 'I' : tracing = intermediates_trace;    break;
        case 'L' : tracing = labels_trace;           break;
        case 'N' : tracing = normal_trace;           break;
        case 'O' : tracing = off_trace;
                     interactive_debug  =  FALSE;
                     command_inhibition =  FALSE;
                     break;
        case 'R' : tracing = results_trace;          break;
        case 'S' : tracing = scan_trace;             break;
        default:   error(ERR_INVALID_TRACE);
     }
} /* set_trace */

/* ---------------------------------------------------------- */
/* readline( Lstr **A )                                       */
/* reads a line from stdin                                    */
/* ---------------------------------------------------------- */
Lstr *readline ( Lstr **A)
{
#ifdef __BEOS__
    extern char *bgetline(char *, long);
    char buff[4096];
    int i, l;
    char *line = bgetline(buff, 4096), *c;

    if (line == 0) {
       longjmp(mainblock, 2);
    }

    l = strlen(line);
    Lfx(A, l + 50);
    for (i = 0; i < l; i++) {
       c = STR((*A)) + i;
       if (line[i] == '\n') {
           l = i;
           break;
       }
       *c = line[i];
    }
    LEN((*A)) = l;
#else
    char *c,ci;
    int   l;

    Lfx(A,50);

    l = 0;
    while ((ci=bgetchar())!='\n') {
      c = STR((*A)) + l;
      *c = ci;  l++;
      if (l > MAXL((*A)))  Lfx(A, l + 50);
    }
    LEN((*A)) = l;
#endif
    return *A;
} /* readline */
/* ---------------------------------------------------------- */
/*  push_program_status(void)                                 */
/*  saves all the data in a temporary allocated array         */
/* ---------------------------------------------------------- */
void push_program_status ( void )
{
   status_queue *n;

   n = (status_queue *) m_malloc( sizeof(status_queue) ,"PRGS" );
   n->prev = pgm_status;
   memcpy(&(n->v), &V, sizeof(struct rexx_vars_st));
   symbolstr = NULL;
   Lfx(&symbolstr,250);
   Lstrcpy(&symbolstr, n->v.SYMBOLSTR);
   environment = NULL;
   Lstrcpy(&environment, n->v.ENVIRONMENT);
   pgm_status = n;
}  /* push_program_status */

/* ---------------------------------------------------------- */
/*  pop_program_status(void)                                  */
/*  saves all the data in a temporary allocated array         */
/* ---------------------------------------------------------- */
void pop_program_status ( void )
{
   status_queue *o;

   if (pgm_status == NULL) return;
   m_free(symbolstr);
   m_free(environment);
   memcpy(&V, &(pgm_status->v), sizeof(struct rexx_vars_st));
   o = pgm_status;
   pgm_status = pgm_status->prev;
   m_free(o);
}  /* pop_program_status */

/* ---------------------------------------------------------- */
/*  queue_Lstr( Lstr *s, Lstrlist **head, **tail );           */
/*  queues one Lstr to a double linked list                   */
/* ---------------------------------------------------------- */
Lstrlist *queue_Lstr(Lstr *s, Lstrlist **head, Lstrlist **tail)
{
   Lstrlist *new;

   new = (Lstrlist *)m_malloc(sizeof(Lstrlist),"FIFO");
   new->s = NULL;
   Lstrcpy(&(new->s),s);

   new->prev = NULL;          /* will be first item in list */
   new->next = *head;
   if (*head != NULL) (*head)->prev = new;
   *head = new;
   if (*tail==NULL) *tail=*head;
   return new;
} /* queue_Lstr */

/* ---------------------------------------------------------- */
/*  push_Lstr( Lstr *s, Lstrlist **head, **tail );            */
/*  pushes one Lstr to a double linked list                   */
/* ---------------------------------------------------------- */
Lstrlist *push_Lstr(Lstr *s, Lstrlist **head, Lstrlist **tail)
{
   Lstrlist *new;
   new = (Lstrlist *)m_malloc(sizeof(Lstrlist),"LIFO");
   new->s = NULL;
   Lstrcpy(&(new->s),s);
   new->next = NULL;          /* will be first item in list */
   new->prev = *tail;
   if (*tail != NULL) (*tail)->next = new;
   *tail = new;
   if (*head==NULL) *head=*tail;
   return new;
} /* push_Lstr */

/* ---------------------------------------------------------- */
/*  Lstr *pop_lstr( Lstrlist **head, **tail );                */
/*  pops the last Lstr from a double linked list              */
/*  NOTE! returned Lstr MUST be freed                         */
/* ---------------------------------------------------------- */
Lstr *pop_Lstr(Lstrlist **head, Lstrlist **tail)
{
   Lstr *s;                 /* PUSH */
   Lstrlist *l;

   if (*tail == NULL) return NULL;
   l = *tail;
   *tail = (*tail)->prev;
   s = l->s;
   m_free(l);
   if (*tail != NULL) (*tail)->next = NULL;
   else                *head = NULL;

   return s;
} /* pop_Lstr */
