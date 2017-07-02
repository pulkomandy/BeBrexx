/*
 * $Header: c:/usr/rexx/I_DO.C!v 1.3 1997/07/04 20:32:45 bill Exp bill $
 * $Log: I_DO.C!v $
 * Revision 1.3  1997/07/04  20:32:45  bill
 * *** empty log message ***
 *
 * Revision 1.2  1996/12/13 01:04:05  bill
 * Volatile added to functions with setjmp()
 *
 * Revision 1.1  1995/09/11  03:33:46  bill
 * Initial revision
 *
 */

#include "instruct.h"

#define NORMAL   0
#define WHILE    1
#define UNTIL    2
#define FOREVER  3
#define SINGLE   4
#define FOR      5

/* -------------------------------------------------------------- */
/*  DO      [ [name=expri  [TO exprt] [BY exprb]                  */
/*            [FOR exprf]] | [ FOREVER | exprr ]                  */
/*          [UNTIL expru | WHILE exprw] ;                         */
/*          [instr]... ;                                          */
/*  END [symbol] ;                                                */
/*      group instructions together with optional repetition and  */
/*      condition. NAME is stepped from EXPRI to EXPRT in         */
/*      steps of EXPRB, for a maximum of EXPRF iterations.        */
/*      These variables are evaluated only once at the top of     */
/*      the loop and must result in numbers. The iterative        */
/*      phrase may be replaced by EXPRR, which is a loop          */
/*      count (no variable used), or by the keyword               */
/*      FOREVER. If a WHILE or UNTIL is given, EXPRU or           */
/*      EXPRW must evaluate to "0" or "1". The condition is       */
/*      tested at the top of the loop if WHILE or at the bottom   */
/*      if UNTIL.                                                 */
/* -------------------------------------------------------------- */
void I_do_times (int,long);
void I_do_loop  (int);

void I_do ( void )
{
   Lstr *A=NULL;
   long  times;
   char  *old_DO_BPTR;

   depth++;
   if (depth > 250) error(ERR_FULL_CTRL_STACK);
   nextsymbol();

   if (flag == f_running) {
      old_DO_BPTR = DO_BPTR;
      DO_BPTR = Bptr;
      statement = in_do;
      if ((symbol==ident_sy) && (*RxPtr == '='))
         I_do_loop(NORMAL);
      else
      if ((symbol==ident_sy) && (!Lcmp(symbolstr,"FOREVER"))) {
         nextsymbol();
         I_do_times(FOREVER,0);
      }  else
      if ((symbol==ident_sy) && (!Lcmp(symbolstr,"FOR"))) {
         symbol = for_sy;
         I_do_loop(FOR);
      }  else
      if ((symbol==ident_sy) && (!Lcmp(symbolstr,"WHILE")))
         I_do_loop(WHILE);
      else
      if ((symbol==ident_sy) && (!Lcmp(symbolstr,"UNTIL")))
         I_do_loop(UNTIL);
      else
      if (symbol==semicolon_sy)
         I_do_times(SINGLE,1);
      else {
         A = I_expression(A);
         L2int(&A);
         if (tracing==results_trace) tracevar(">>>",A);
         times = INT(A);
         m_free(A);
         I_do_times(FALSE,times);
      }
      DO_BPTR = old_DO_BPTR;
      depth--;
   } else {             /* stopped */
      skiptill(semicolon_sy);
      I_instr_till_end;
      nextsymbol();
      if (symbol==ident_sy) nextsymbol();
      mustbe_semicolon;
      depth--;
   }
}  /* I_do */

/* --------------------* I_do_times *---------------------------- */
void I_do_times ( int type, long Times )
{
   volatile char    *do_ptr;
   volatile int      do_lineno;
   volatile int      old_IN_DO, IN_DO_SET;
   volatile int      old_depth;
   volatile long     times;
   volatile int      result;
   jmp_buf  old_do;
                    /* Maybe on MSDOS, this problem doesn't exist */
   times = Times;   /* WITH LONG JUMP Times doesn't change it's value */

   do_ptr = RxPtr;
   do_lineno = lineno;
   mustbe_semicolon;

   old_depth = depth;
   if (type != SINGLE) {
      old_IN_DO = IN_DO;
      IN_DO = TRUE;
      IN_DO_SET = TRUE;
   } else
      IN_DO_SET = FALSE;
   Jcpy(old_do, do_instr);
setjumper:
   result = setjmp(do_instr);
   if (result) {
      depth = old_depth;
      if ((ControlVar != NULL) || (type==SINGLE))
         longjmp(old_do, result);  /* try prev do */
      if (result==2) goto FINITO;  /* from leave .... */
      goto setjumper;
   }
   while ((times>0)  || (type==FOREVER) ) {
       RxPtr = (char *)do_ptr;
       Bptr = DO_BPTR;  Blineno = lineno; AIR_print;
       lineno = do_lineno;
       nextsymbol();        /* semicolon */
       --times;
       I_instr_till_end;
       AIR_print;
   }

FINITO:
       Bptr = DO_BPTR;   Blineno = lineno; AIR_print;
       RxPtr = (char *)do_ptr;
       lineno = do_lineno;
       flag = f_stopped;
       nextsymbol();
       I_instr_till_end;
       flag = f_running;
   nextsymbol();
   mustbe_semicolon;

   Jcpy(do_instr, old_do);
   if (IN_DO_SET) IN_DO = old_IN_DO;
}  /* I_do_times */

/* --------------------* I_do_var *---------------------------- */
void I_do_loop ( int type )
{
   volatile char    *do_ptr;         Idrec *Id;           Lstr *name=NULL;
   volatile int      do_lineno;      Lstr  *expri=NULL;
   volatile int      old_IN_DO;      Lstr  *exprt=NULL;   volatile int t=FALSE;
   volatile int      old_depth;      Lstr  *exprb=NULL;   volatile int b=FALSE;
                                     Lstr  *exprf=NULL;   volatile int f=FALSE;
   volatile int      result;                              volatile int decrement=FALSE;
   volatile long     dofor;
   volatile int      isarray;
   volatile int      firsttime=TRUE; Lstr  *A=NULL;
   jmp_buf  old_do;

   statement = in_do_init;
   AIR_print;

   if (type==NORMAL) {
      isarray = symbolisarray;
      Lstrcpy(&name,symbolstr);
      Id = V_create( isarray, name, &scope );
      nextsymbol();
      mustbe(eq_sy, ERR_INVALID_EXPRESSION);
      expri = I_expression(expri);
      if (tracing == results_trace) tracevar(">>>",expri);
      assign(Id,expri);
      m_free(expri);
   }

   Licpy(&exprb,1);

   for (;;)
    switch (symbol) {
      case to_sy:   if (t || (name==NULL))
                       error(ERR_INVALID_DO_SYNTAX);
                    nextsymbol();
                    exprt = I_expression(exprt);
                    L2num(&exprt);
                    if (tracing==results_trace) tracevar(">>>",exprt);
                    t = TRUE;
                    break;
      case by_sy:   if (b || (name==NULL))
                       error(ERR_INVALID_DO_SYNTAX);
                    nextsymbol();
                    exprb = I_expression(exprb);
                    L2num(&exprb);
                    if (tracing==results_trace) tracevar(">>>",exprb);
                    if (TYPE(exprb)==INT_TY)
                       decrement = (INT(exprb) < 0);
                    else
                       decrement = (REAL(exprb) < 0);
                    b = TRUE;
                    break;
      case for_sy:  if (f) error(ERR_INVALID_DO_SYNTAX);
                    nextsymbol();
                    exprf = I_expression(exprf);
                    L2int(&exprf); dofor = INT(exprf);
                    if (tracing==results_trace) tracevar(">>>",exprf);
                    m_free(exprf);
                    if (dofor<0) error(ERR_INVALID_INTEGER);
                    f = TRUE;
                    break;
      default:      goto leave;
    }
leave:

   do_ptr = RxPtr;
   do_lineno = lineno;

   if (symbol==while_sy) {
      type = WHILE; nextsymbol();
   } else
   if (symbol==until_sy) {
      type = UNTIL; nextsymbol();
   }


   old_depth = depth;
   old_IN_DO = IN_DO;
   IN_DO = TRUE;

   statement = in_do;
   Jcpy(old_do, do_instr);
setjumper:
   result = setjmp(do_instr);
   if (result) {
      if (ControlVar != NULL) {
         if (name==NULL) longjmp(old_do, result);  /* try prev do */
         if (Lstrcmp(name,ControlVar)) {
             m_free(name);
             m_free(exprb);
             m_free(exprt);
             m_free(A);
             longjmp(old_do, result);  /* try prev do */
          }
         m_free(ControlVar);
      }
      depth = old_depth;
      if (result==2) {   /* from leave .... and should leave  */
         RxPtr = (char *)do_ptr;
         lineno = do_lineno;
         goto FINITO;
      }
      goto setjumper;
   }

   for (;;) {
       RxPtr = (char *)do_ptr;
       Bptr = DO_BPTR;   Blineno = lineno;  AIR_print;
       lineno = do_lineno;

       if (type==UNTIL) {
          nextsymbol();
          if (firsttime) skiptill(semicolon_sy);
          else {
             A = I_expression(A);   result = O_bool(A);
             if (tracing==results_trace) tracevar(">>>",A);
             m_free(A);
             if (result) goto FINITO;
          }
       }

       if (name!=NULL) {
          Id = V_create( isarray, name, &scope );
          if (!firsttime)  Id->var = O_add(Id->var, exprb);

          if (tracing==results_trace) tracevar(">>>",Id->var);
          if (t) {
             Lstrcpy(&A, Id->var );
             if (decrement) A = O_lt(A, exprt);
             else           A = O_gt(A, exprt);
             if (INT(A)==1) goto FINITO;
          }
       }

       if (f) if (!dofor--) goto FINITO;

       if (type==WHILE) {
          nextsymbol();
          A = I_expression(A);    result = O_bool(A);
          if (tracing==results_trace) tracevar(">>>",A);
          m_free(A);
          if (!result) goto FINITO;
       }

       if (type!=NORMAL)  mustbe_semicolon;
       else               symbol = semicolon_sy;

       firsttime = FALSE;
       I_instr_till_end;
       AIR_print;
       nextsymbol();
       if (symbol==ident_sy) {
         if ((name!=NULL) && Lstrcmp(name,symbolstr))
            error(ERR_UNMATCHED_END);
         nextsymbol();
       }
   }
FINITO:
         flag = f_stopped;
         symbol = semicolon_sy; /*skiptill(semicolon_sy);*/
         I_instr_till_end;
         flag = f_running;

   nextsymbol();
   if (symbol==ident_sy) {
      if ((name!=NULL) && Lstrcmp(name,symbolstr))
          error(ERR_UNMATCHED_END);
      nextsymbol();
   }
   mustbe_semicolon;

   m_free(name);
   m_free(A);
   m_free(exprb);
   m_free(exprt);

   Jcpy(do_instr, old_do);
   IN_DO = old_IN_DO;
}  /* I_do_loop */

/* -------------------------------------------------------------- */
/*  ITERATE   [name] ;                                            */
/*      start next iteration of the innermost repetitive loop     */
/*      (or loop with control variable name).                     */
/* -------------------------------------------------------------- */
void I_iterate ( void )
{
   if (flag==f_running) {
      AIR_print;
      if (!IN_DO) error(ERR_INVALID_LEAVE);
      nextsymbol();
      if (symbol == ident_sy) {
         ControlVar = NULL;
	 Lstrcpy(&ControlVar,symbolstr);
	 nextsymbol();
      } else ControlVar = NULL;
      mustbe_semicolon;
      longjmp( do_instr, 1 );
   } else  skiptill(semicolon_sy);
} /* I_iterate */
/* -------------------------------------------------------------- */
/*  LEAVE     [name] ;                                            */
/*      terminate innermost loop (or loop with control            */
/*      variable name).                                           */
/* -------------------------------------------------------------- */
void I_leave ( void )
{
   if (flag==f_running) {
      AIR_print;
      if (!IN_DO) error(ERR_INVALID_LEAVE);
      nextsymbol();
      if (symbol == ident_sy) {
         ControlVar = NULL;
	 Lstrcpy(&ControlVar,symbolstr);
	 nextsymbol();
      } else ControlVar = NULL;
      mustbe_semicolon;
      longjmp( do_instr, 2 );
   } else  skiptill(semicolon_sy);
} /* I_leave */
