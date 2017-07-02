/*
 * $Header: c:/usr/rexx/ERROR.C!v 1.2 1997/07/04 20:32:42 bill Exp bill $
 * $Log: ERROR.C!v $
 * Revision 1.2  1997/07/04  20:32:42  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11 03:31:42  bill
 * Initial revision
 *
 */

#define __ERROR_C__

#include <stdio.h>

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "variable.h"

#ifdef PARADOX
# include <pxengine.h>
#endif

void typesym( char *);

boolean in_print=FALSE;
boolean in_error=FALSE;

extern INTERPRET_LEVEL; /* from instruct.h */

/* -------------------------------------------------------------- */
/* ERROR: place an error in the error array and point that error  */
/* -------------------------------------------------------------- */

void error(int errornum)
{
   Label  *Lab;
   Lstr   *SYNTAX=NULL;

   if (in_error) goto AFTER_TYPE;

   in_error = TRUE;
   if (Syntax) {
      Lscpy(&SYNTAX,"SYNTAX");
      Lab = searchlabel( SYNTAX, &label_scope );
      m_free(SYNTAX);
      if (Lab == NULL) {
         errornum = ERR_UNEXISTENT_LABEL;
         goto TYPE_ERR;
      }

      setvar("RC",errornum);

      in_error=FALSE;
      Signal(Lab);
   }

TYPE_ERR:
   if (!in_print) printcurline();
AFTER_TYPE:
#ifdef PARADOX
   if (errornum==ERR_PARADOX_ERROR) {
      beprintf("Error %d running %s, line %d: %s \"%s\"\n",
           errornum,STR(execfile),Blineno,errormsg[errornum], PXErrMsg(rc));
      rc += 30000;
   } else
#endif
   {
      beprintf("Error %d running %s, line %d: %s\n",
              errornum,STR(execfile),Blineno,errormsg[errornum]);
      rc = 20000 + errornum;
   }
   in_error = FALSE;
   longjmp(finito,rc);
} /* error */

/* -------------------------------------------------------------- */
/* tracevar(msg,Lstr) Dumps a variable to stdout                  */
/* -------------------------------------------------------------- */
void tracevar(char *msg, Lstr *Var)
{
   int i;
   bprintf("       %3s   ",msg);
   for (i=1; i < depth; i++) bputchar(' ');
   if (Var==NULL) bprintf("+++NULL+++\n");
   else
     switch (TYPE(Var)) {
        case INT_TY:  bprintf("\"%ld\"",INT(Var)); break;
#if defined(__BEOS__)
        case REAL_TY: bprintf("\"%G\"",REAL(Var)); break;
#else
        case REAL_TY: bprintf("\"%lG\"",REAL(Var)); break;
#endif
        case STR_TY:  bputchar('\"');
                      for (i=0;i<LEN(Var);i++) bputchar(STR(Var)[i]);
                      bputchar('\"');
                      break;
        default:      bprintf("Internal ERROR\n");
                      error(ERR_INTERPRETER_FAILURE);
   }
   bputchar('\n');
} /* tracevar */

/* ---------------- printcurline -------------- */
void printcurline( void )
{
   char      *s;
   int       i;
   Lstr      *A=NULL;
   int       _tracing;

   in_print = TRUE;
   if (Blineno!=TPrevLineno && !INTERPRET_LEVEL)
     /* this is the first command in the line so type and the linenum */
      bprintf("%6d *-* ",Blineno);
   else
      bprintf("       *-* ");
   TPrevLineno = Blineno;

   push_program_status();

   RxPtr = Bptr;

   for (i=1;i<depth;i++) bputchar(' ');

   /* skip first blanks */
   while (ISSPACE(*RxPtr)) RxPtr++;
   do { s=RxPtr;nextsymbol();} while (symbol==semicolon_sy);

   if (symbol==label_sy)  typesym(s);
   else {
     if (symbol==ident_sy &&
         (!Lcmp(symbolstr,"IF") ||
          !Lcmp(symbolstr,"WHEN")) )
         statement = in_if;
     else
         statement = normal_st;

     while (symbol!=semicolon_sy && symbol!=then_sy && (*RxPtr)) {
        typesym(s); s=RxPtr;
        nextsymbol();
     }
     statement = nothing_st;
     if (*RxPtr)
       while (symbol==semicolon_sy) {
         typesym(s); s=RxPtr;
         nextsymbol();
       }
   }

   bputchar('\n');
   pop_program_status();

   if (interactive_debug) {
      _tracing = tracing;
      tracing = 0;
      interactive_debug = FALSE;
      do {
        A = NULL;
        readline(&A);
        if ((i=LEN(A))!=0)
          I_interpret_string(&A); /* A is freed in interpret string */
        else m_free(A);
      } while (i!=0);
      interactive_debug = !interactive_debug;
      if (!tracing) tracing = _tracing;
      else if (tracing==off_trace) interactive_debug = FALSE;
   }
   in_print = FALSE;
} /* printcurline */

/* ---------- internal routines -------------- */
void  typesym(char *s)
{
   int nl=0;
   while ((char HUGE *)s < (char HUGE *)RxPtr)  {
     if (!(*s)) return;
     if (!((*s==',' && *(s+1)=='\n') || (*s=='\n'))) bputchar(*s);
     else bputchar(' ');
     if (*s=='\n') nl++;
     s++;
   }
} /* typesym */
