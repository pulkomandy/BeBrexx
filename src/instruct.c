/* ----------------------------------------------------- o o ---- */
/*                                                     ____oo_    */
/*                                          (c)1992   /||    |\   */
/*  Instructions                              BNV      ||    |    */
/*  ~~~~~~~~~~~~                                       '.___.'    */
/*                                                     MARMITA    */
/*  REXX instructions are built out of clauses consisting of a    */
/*  series of items, operators, etc.  expr, used in the following */
/*  instructions, refer to expressions discussed previously. The  */
/*  semicolon at the end of each clause if often not required,    */
/*  being implied by line ends and after the keywords THEN,       */
/*  ELSE, or OTHERWISE. A clause may be continued from one        */
/*  line to the next by using a comma at the end of the line.     */
/*  This then acts like a blank. Open strings or comments are not */
/*  affected by line ends, and do not require a continuation      */
/*  character.                                                    */
/*                                                                */
/*  Keywords are shown in uppercase in this list; however, they   */
/*  may appear in either case (upper or mixed). Keywords are      */
/*  only reserved when they are found in the correct context.     */
/*                                                                */
/*                                                                */
/*  The General Guidelines are:                                   */
/*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~                                   */
/*                                                                */
/*  name                                                          */
/*       refers to a variable, which can be assigned any value.   */
/*       name is a symbol with the following exception: the       */
/*       first character may not be a digit or a period. The      */
/*       value of name is translated to uppercase before use, and */
/*       forms the initial value of the value of the variable.    */
/*       Some valid names are:                                    */
/*            Fred  COST?  next  index   A.j                      */
/*                                                                */
/*  name:                                                         */
/*       is a form of labels for CALL instructions, SIGNAL        */
/*       instructions, and internal function calls. The colon     */
/*       acts as a clause separator.                              */
/*                                                                */
/*  template                                                      */
/*       is a parsing template, described in a later section.     */
/*                                                                */
/*  instr                                                         */
/*       is any one of the listed instructions.                   */
/*                                                                */
/* -------------------------------------------------------------- */

/* ----------------------------------------------------- o o ---- */
/*                                                     ____oo_    */
/*                                          (c)1992   /||    |\   */
/*  Special Variables                         BNV      ||    |    */
/*  ~~~~~~~~~~~~~~~~~                                  '.___.'    */
/*                                                     MARMITA    */
/*  There are three special variables:                            */
/*                                                                */
/*  SIGL                                                          */
/*         contains the line number of the clause currently       */
/*         executing when the last transfer of control to a       */
/*         label took place. This can be caused by a SIGNAL       */
/*         instruction, a CALL instruction, an internal function  */
/*         invocation, or a trapped error condition.              */
/*                                                                */
/*  RC                                                            */
/*         is set to the return code after each executed command. */
/*                                                                */
/*  RESULT                                                        */
/*         is set by a RETURN instruction in a CALLed             */
/*         subroutine, or is dropped if the RETURN instruction    */
/*         did not specify an expression.                         */
/*                                                                */
/* -------------------------------------------------------------- */
/*
 * $Header: c:/usr/rexx/instruct.c!v 1.4 1997/09/13 15:18:22 bill Exp bill $
 * $Log: instruct.c!v $
 * Revision 1.4  1997/09/13  15:18:22  bill
 * Changed: Multiple stacks support
 *
 * Revision 1.3  1997/07/04  20:06:52  bill
 * Correct: Now can handle host cmds starting with ( or any other symbol.
 *
 * Revision 1.2  1996/12/13 01:03:24  bill
 * Volatile added to variables in functions with setjmp()
 *
 * Revision 1.1  1995/09/11  03:33:11  bill
 * Initial revision
 *
 */

#define __INSTRUCT_C__

#include <stdlib.h>
#include "instruct.h"

/* -------------------------------------------------------------- */
/*  ADDRESS [<symbol | string> [expr]] ;                          */
/*      redirect commands or a single command to a new            */
/*      environment. ADDRESS VALUE expr may be used               */
/*      for an evaluated enviroment name.                         */
/* -------------------------------------------------------------- */
void  I_address( void )
{
   Lstr *A=NULL;
   Lstr *env=NULL;

   if (flag == f_running) {
      AIR_print;   nextsymbol();
      if ((symbol != ident_sy) &&
	  (symbol != literal_sy)) error(ERR_INVALID_EXPRESSION);
      if (!Lcmp(symbolstr,"VALUE")) {
	 nextsymbol();
	 A = I_expression(A);  L2str(&A);
	 if (tracing==results_trace) tracevar(">>>",A);
#ifdef __MSDOS__
	 L2upper(A);
#endif
	 if (LEN(A) > maxenvi) error(ERR_ENVIRON_TOO_LONG);
	 Lstrcpy(&environment,A);
	 m_free(A);
      }  else {
	  if (LEN(symbolstr) > maxenvi) error(ERR_ENVIRON_TOO_LONG);
	  Lstrcpy(&env,symbolstr);
#ifdef __MSDOS__
	  L2upper(env);
#endif
	  if (tracing==results_trace) tracevar(">E>",env);
	  nextsymbol();
	  if (symbol!=semicolon_sy) {
	     A = I_expression(A);
	     if (tracing==results_trace) tracevar(">>>",A);
	     I_cmd(A,env);
	     m_free(A);
	  } else Lstrcpy(&environment,env);
	  m_free(env);
	  mustbe_semicolon;
      }
   } else skiptill(semicolon_sy);
} /* I_address */
/* -------------------------------------------------------------- */
/*  ARG     <template> ;                                          */
/*      parse argument string(s), translated into uppercase,      */
/*      into variables. Short for PARSE UPPER ARG.                */
/* -------------------------------------------------------------- */
void  I_arg ( void )
{
   Lstr *A=NULL;
   int   i;
   if (flag == f_running) {
      AIR_print;
      i = 0;
      do {
         nextsymbol();
         if (i >= arg->n)  Lscpy(&A,"");
         else {
            Lstrcpy(&A, arg->a[i]);
            L2upper( A );
         }
         I_template( A );
         i++;
      } while (symbol==comma_sy);
      m_free(A);
      mustbe_semicolon;
   }  else  skiptill(semicolon_sy);
} /* I_arg */
/* -------------------------------------------------------------- */
/*  CALL    [symbol | string] [<expr>] [,<expr>]... ;             */
/*      call an internal routine, an external routine or program, */
/*      or a built-in function. Depending on the type of          */
/*      routine called, the variable RESULT contains the result   */
/*      of the routine. RESULT is uninitialized if no result is   */
/*      returned.                                                 */
/* -------------------------------------------------------------- */
void  I_call ( void )
{
   Lstr   *funcname=NULL;
   args   *_arg;
   args   *Carg;
   int     n,isstr;
   Lstr   *resultid=NULL;
   Idrec  *Aptr;
   int     func;

   if (flag == f_running) {
      AIR_print;  nextsymbol();
      if ((symbol!=ident_sy) && (symbol!=literal_sy) && (symbol!=function_sy))
      	error(ERR_STRING_EXPECTED);
      func = (symbol==function_sy);
      Lstrcpy(&funcname, symbolstr);  isstr = symbolisstr;
      nextsymbol();
      Carg = mallocargs();
      n = 0;
      if (symbol!=semicolon_sy) {
         if (symbol != comma_sy)
            Carg->a[n] = I_expression( Carg->a[n] );
         n++;
         while (symbol==comma_sy) {
            nextsymbol();
            if (n >= maxargs) error(ERR_INCORRECT_CALL);
            if (! ((symbol==comma_sy) || (symbol==semicolon_sy)))
               Carg->a[n] = I_expression( Carg->a[n] );
            n++;
         }
      }
      Carg->n = n;
/* push arguments */
      depth++;
      if (depth>250) error(ERR_FULL_CTRL_STACK);

      _arg = oarg;
      oarg =  arg;
       arg = Carg;

      call_type = SUBROUTINE;

      if (isstr) System_Function( &funcname);
      else       Rexx_Function( funcname );
      m_free(funcname);

      if (arg->n) {
         Lscpy(&resultid,"RESULT");
         Aptr = createId(resultid, &scope );
         assign(Aptr, arg->r);
         m_free(resultid);
      }

/* Pop old arguments */
      freeargs( arg );
       arg = oarg;
      oarg = _arg;
      --depth;
      if (func)
      	mustbe(ri_parent,ERR_UNMATCHED_PARAN);
      mustbe_semicolon;
   } else  skiptill(semicolon_sy);
} /* I_call */
/* -------------------------------------------------------------- */
/*  DROP name [name]... ;                                         */
/*      drop (reset) the named variables or group of variables.   */
/*                                                                */
/*  *** if an exposed variable is named, the variable itself      */
/*      in the older generation will be dropped!                  */
/* -------------------------------------------------------------- */
void  I_drop ( void )
{
   if (flag == f_running) {
      AIR_print;
      nextsymbol();
      while (symbol == ident_sy) {
         V_drop(symbolisarray,symbolstr, &scope);
         nextsymbol();
      }
      mustbe_semicolon;
   } else
      skiptill(semicolon_sy);
} /* I_drop */
/* -------------------------------------------------------------- */
/*  EXIT [expr] ;                                                 */
/*      leave the program (with return data, EXPR). EXIT is       */
/*      the same as RETURN except that all internal routines      */
/*      are terminated                                            */
/* -------------------------------------------------------------- */
void  I_exit ( void )
{
   Lstr *A=NULL;

   if (flag == f_running) {
      AIR_print;
      if (symbol != exit_sy) {
         nextsymbol();
         if (symbol == semicolon_sy) rc = 0;
         else {
            A = I_expression(A);
            if (tracing == results_trace) tracevar(">>>",A);
            L2int(&A);
            rc = (int)INT(A);
            m_free(A);
         }
         EXIT_FROMPRG = 1;
         mustbe_semicolon;
      } else
         EXIT_FROMPRG = 0;  /* if exit_sy is reached, and we are in interpret then */

      longjmp(mainblock,2);
   } else  skiptill(semicolon_sy);
} /* I_exit */
/* -------------------------------------------------------------- */
/*  IF expr [;] THEN [;] instr ;                                  */
/*             [ELSE [;] instr];                                  */
/*      if EXPR evaluates to "1", execute the instruction         */
/*      following the THEN. Otherwise, (EXPR evaluates to         */
/*      "0") skip that instruction and execute the one            */
/*      following the ELSE clause, if present.                    */
/* -------------------------------------------------------------- */
void  I_if ( void )
{
   Lstr *A=NULL;
   int  _exec;

   nextsymbol();
   depth++;
   if (depth > 250) error(ERR_FULL_CTRL_STACK);
   if (flag == f_running) {
      AIR_print;
      statement = in_if;
      A = I_expression(A);
      _exec = O_bool(A);
      if (tracing==results_trace) tracevar(">>>",A);
      m_free(A);
      skip_semicolons;
      statement = normal_st;
      Bptr = Pptr;  Blineno = lineno;    /* Mark current ptr for error msg */
      if (symbol != then_sy)  error(ERR_THEN_EXPECTED );
      if (_exec) AIR_print;
      nextsymbol();
      if (_exec) flag = f_running;
            else flag = f_stopped;
      I_instr( FALSE );
      skip_semicolons;

      if (symbol==ident_sy)
        Bptr = Pptr; Blineno = lineno;
        if (!Lcmp(symbolstr,"ELSE")) {
         if (!_exec) AIR_print;
         nextsymbol();
         if (_exec) flag = f_stopped;
               else flag = f_running;
         I_instr( FALSE );
      }
      flag = f_running;
   } else {
      statement = in_if;
      skiptill(then_sy);
      nextsymbol();
      statement = normal_st;
      I_instr(FALSE);
      skip_semicolons;
      if (symbol==ident_sy)
        if (!Lcmp(symbolstr,"ELSE")) {
         nextsymbol();
         I_instr(FALSE);
      }
   }
   depth--;
}  /* I_if */
/* -------------------------------------------------------------- */
/*  INTERPRET  expr ;                                             */
/*      evaluate EXPR and then execute the resultant string as    */
/*      if it was part of the original program.                   */
/* -------------------------------------------------------------- */
void  I_interpret ( void )
{
   Lstr *A=NULL;

   nextsymbol();
   if (flag==f_running) {
      AIR_print;
      A = I_expression(A);
      mustbe_semicolon;
      if (tracing==results_trace) tracevar(">>>",A);
      I_interpret_string ( &A );
   } else
     skiptill(semicolon_sy);
} /* I_interpret */
/* ----------------* I_interpret_string *---------------- */
void  I_interpret_string ( Lstr **A)
{
   int  trace, inter, cmdinh;
   int  lno;
   char *ptr;
   Lstr *env=NULL;
   char *_exectext;

   if (*A==NULL) return;

   depth++;
   if (depth > 250) error(ERR_FULL_CTRL_STACK);
   INTERPRET_LEVEL++;
   _exectext = exectext;
   push_program_status();
   L2str(A);
   Lfx(A,LEN(*A)+4);
   exectext = STR(*A);
   exectext[ LEN(*A)+1 ] = exectext[ LEN(*A) ] = '\n';
   exectext[ LEN(*A)+2 ] = '\0';
   symbol = semicolon_sy;
   Bptr = RxPtr = exectext;

   Main_block();

   Lstrcpy(&env,environment);
   trace = tracing; inter = interactive_debug; cmdinh = command_inhibition;
   ptr = RxPtr;  lno = lineno;
   pop_program_status();        /* Tracing is restored */
   tracing = trace; interactive_debug = inter; command_inhibition = cmdinh;
   Lstrcpy(&environment,env);
   m_free(env);

   --INTERPRET_LEVEL;
   exectext = _exectext;
   --depth;
   m_free(*A);                /* !!!!! will free the string */
   if (IN_SIGNAL) {
      RxPtr = ptr;          /* Preserve these two values in case of signal */
      lineno = lno;
      longjmp(mainblock,1);
   } else
   if (EXIT_FROMPRG) longjmp(mainblock,2);
} /* I_interpret_string */
/* -------------------------------------------------------------- */
/*  LOWER name [name]...                                          */
/*      translate the values of the specified individual          */
/*      variables to uppercase.                                   */
/* -------------------------------------------------------------- */
void  I_lower ( void )
{
   Idrec *A = NULL;

   if (flag == f_running) {
      AIR_print;    nextsymbol();
      while (symbol == ident_sy) {
	 A = V_search( symbolisarray, symbolstr, scope );
	 if (symbolisarray && stem) error(ERR_INVALID_STEM);
	 if (A != NULL) {
	    L2lower(A->var);
	    if (tracing==results_trace) tracevar(">>>",A->var);
	 }
	 nextsymbol();
      }
      mustbe_semicolon;
   } else  skiptill(semicolon_sy);
} /* I_lower */
/* -------------------------------------------------------------- */
/*  NOP  ;                                                        */
/*      dummy instruction, has no side-effects.                   */
/* -------------------------------------------------------------- */
void  I_nop ( void )
{
   if (flag==f_running) AIR_print;
   nextsymbol();
   mustbe_semicolon;
} /* I_nop */
/* -------------------------------------------------------------- *)
(*  NUMERIC   DIGITS [expr]  |                                    *)
(*            FORM   [SCIENTIFIC | ENGINEERING] |                 *)
(*            FUZZ   [expr]  ;                                    *)
(*   o  DIGITS, carry out arithmetic operations to EXPR           *)
(*      significant digits.                                       *)
(*   o  FORM, set new exponential format.                         *)
(*   o  FUZZ, ignore up to EXPR least significant digits          *)
(*      during arithmetic comparisons.                            *)
(* -------------------------------------------------------------- */
void  I_numeric( void )
{
   Lstr *A=NULL;

   if (flag==f_running) {
      AIR_print;  nextsymbol();
      if (symbol==ident_sy) {
         if (!Lcmp(symbolstr,"DIGITS")) {
            nextsymbol();
            A = I_expression(A);
            L2int(&A);
            digits = (int)INT(A);
            if (digits<=0 || digits>=9) error(ERR_INVALID_INTEGER);
         } else
         if (!Lcmp(symbolstr,"FORM")) {
            nextsymbol();
            if (symbol==ident_sy) {
               if (!Lcmp(symbolstr,"SCIENTIFIC"))  form = SCIENTIFIC;
               else
               if (!Lcmp(symbolstr,"ENGINEERING"))  form = ENGINEERING;
               else error(ERR_INVALID_EXPRESSION);
            } else error(ERR_INVALID_EXPRESSION);
         } else
         if (!Lcmp(symbolstr,"FUZZ")) {
            nextsymbol();
            A = I_expression(A);
            L2int(&A);
            fuzz = (int)INT(A);
            if (digits<=0 || digits>=9) error(ERR_INVALID_INTEGER);
         } else error(ERR_INVALID_EXPRESSION);
      } else error(ERR_INVALID_EXPRESSION);
      mustbe_semicolon;
   } else  skiptill(semicolon_sy);
} /* I_numeric */
/* -------------------------------------------------------------- */
/*  PARSE   [UPPER]  + ARG               | [template] ;           */
/*                   | AUTHOR            |                        */
/*                   | EXTERNAL          |                        */
/*                   | NUMERIC           |                        */
/*                   | PULL              |                        */
/*                   | SOURCE            |                        */
/*                   | VALUE [expr] WITH |                        */
/*                   | VAR name          |                        */
/*                   + VERSION           +                        */
/*   o  ARG, parses the argument string(s) to the program,        */
/*      subroutine, or function. UPPER first translates the       */
/*      strings to uppercase. See also the ARG instruction        */
/*   o  EXTERNAL, read and parse the next string from the         */
/*      terminal input buffer (system external queue).            */
/*   o  NUMERIC, parse the current NUMERIC settings.              */
/*   o  PULL, read and parse the next string from the             */
/*      program stack (system data queue). See the PULL           */
/*      instruction.                                              */
/*   o  SOURCE, parse the program source description e.g.         */
/*      "CMS COMMAND FRED EXEC A fred CMS".                       */
/*   o  VALUE, parse the value of EXPR.                           */
/*   o  VAR, parse the value of NAME.                             */
/*   o  VERSION, parse the data describing the language level     */
/*      and the date of the interpreter.                          */
/* -------------------------------------------------------------- */
void  I_parse ( void )
{
   Lstr     *A = NULL;
   Lstr     *B = NULL;
   boolean   UP;
   int       i;

   if (flag == f_running) {
      AIR_print;   nextsymbol();
      if (!Lcmp(symbolstr,"UPPER")) {
	 UP = TRUE; nextsymbol();
      } else  UP = FALSE;
      if (!Lcmp(symbolstr,"ARG")) {
	 i = 0;
	 do {
	    nextsymbol();
	    if (i >= arg->n)  Lscpy(&A,"");
	    else {
	       Lstrcpy(&A, arg->a[i]);
	       if (UP) L2upper( A );
	    }
	    I_template( A );
	   i++;
	 } while (symbol==comma_sy);
      } else
      if (!Lcmp(symbolstr,"EXTERNAL")) {
	 nextsymbol();
	 readline(&A);
	 if (UP) L2upper( A );
	 I_template( A );
      } else
      if (!Lcmp(symbolstr,"NUMERIC")) {
	 nextsymbol();
         Licpy(&A, digits);   Lcat(&A," ");
         Licpy(&B, fuzz);     Lstrcat(&A,B);  Lcat(&A," ");
         Lcat(&A, (form==SCIENTIFIC)?"SCIENTIFIC":"ENGINEERING");
         m_free(B);
	 I_template( A );
      } else
      if (!Lcmp(symbolstr,"PULL")) {
	 nextsymbol();
         if (stack->tail!=NULL) A = pop_Lstr(&(stack->head),&(stack->tail));
         else                   readline(&A);
	 if (UP) L2upper( A );
	 I_template( A );
      } else
      if (!Lcmp(symbolstr,"VAR")) {
	 nextsymbol();
	 if (symbol != ident_sy) error(ERR_INVALID_TEMPLATE);
	 V_eval(symbolisarray, &A, symbolstr, scope);
	 nextsymbol();
	 if (UP) L2upper( A );
	 I_template( A );
      } else
      if (!Lcmp(symbolstr,"SOURCE")) {
	 nextsymbol();
         Lscpy(&A,OS);
         if (file_type==COMMAND)  Lcat(&A," COMMAND ");
         else
         if (file_type==FUNCTION) Lcat(&A," FUNCTION ");
         else                     Lcat(&A," SUBROUTINE ");
         Lstrcat(&A,execfile);    
         Lcat(&A," * * ");
         Lstrcat(&A,execfile);
         Lcat(&A," ");
         Lcat(&A,getenv(SHELL));
	 if (UP) L2upper( A );
	 I_template( A );
      }  else
      if (!Lcmp(symbolstr,"VALUE")) {
	 nextsymbol();
	 statement = in_parse_value;
	 A = I_expression( A );
	 statement = normal_st;
	 mustbe( with_sy, ERR_INVALID_EXPRESSION );
	 if (UP) L2upper( A );
	 I_template( A );
      }  else
      if (!Lcmp(symbolstr,"AUTHOR")) {  
	 nextsymbol();         Lscpy(&A, author );
	 if (UP) L2upper( A );
	 I_template( A );
      }  else
      if (!Lcmp(symbolstr,"VERSION")) {
	 nextsymbol();         Lscpy(&A, rx_version );
	 if (UP) L2upper( A );
	 I_template( A );
      };
      m_free(A);
      mustbe_semicolon;
   }  else   skiptill(semicolon_sy);
} /* I_parse */
/* -------------------------------------------------------------- */
/*  PULL   [template] ;                                           */
/*      read the next string from the program stack (system       */
/*      data queue), uppercase it, and parse it into variables.   */
/*      If the queue is empty, then data is read from the         */
/*      terminal input buffer. Short for PARSE UPPER PULL.        */
/* -------------------------------------------------------------- */
void  I_pull ( void )
{
   Lstr *A=NULL;

   if (flag == f_running) {
      AIR_print;   nextsymbol();
      if (stack->tail!=NULL)  A = pop_Lstr(&(stack->head),&(stack->tail));
      else                    readline(&A);
      L2upper(A);
      I_template( A );
      m_free(A);
      mustbe_semicolon;
   } else  skiptill(semicolon_sy);
} /* I_pull */
/* -------------------------------------------------------------- */
/*  PUSH   [expr] ;                                               */
/*      push EXPR onto head of the system queue (stack LIFO)      */
/* -------------------------------------------------------------- */
void  I_push ( void )
{
   Lstr *A = NULL;

   if (flag == f_running) {
      AIR_print;   nextsymbol();
      A = I_expression(A);
      if (tracing == results_trace) tracevar(">>>",A);
      L2str(&A);
      push_Lstr(A,&(stack->head), &(stack->tail));
      m_free(A);
      mustbe_semicolon;
   } else  skiptill(semicolon_sy);
} /* I_push */
/* -------------------------------------------------------------- */
/*  QUEUE  [expr] ;                                               */
/*      add EXPR to the tail of the system queue (stack FIFO)     */
/* -------------------------------------------------------------- */
void  I_queue ( void )
{
   Lstr *A = NULL;

   if (flag == f_running) {
      AIR_print;    nextsymbol();
      A = I_expression(A);
      if (tracing == results_trace) tracevar(">>>",A);
      L2str(&A);
      queue_Lstr(A,&(stack->head), &(stack->tail));
      m_free(A);
      mustbe_semicolon;
   } else
      skiptill(semicolon_sy);
} /* I_push */
/* -------------------------------------------------------------- */
/*  RETURN [expr] ;                                               */
/*      evaluate EXOR and then return the value to the caller.    */
/*      (RETURN has the same effect as EXIT if it is not in an    */
/*      internal routine.)                                        */
/* -------------------------------------------------------------- */
void  I_return( void )
{
   Lstr *A=NULL;

   if (flag == f_running) {
      AIR_print;   nextsymbol();
      if (symbol != semicolon_sy) {
	 A = I_expression(A);
	 arg->n = 1;
	 Lstrcpy(&(arg->r),A);
	 m_free(A);
      }  else arg->n = 0;
      mustbe_semicolon;
      longjmp(mainblock,3);
   } else  skiptill(semicolon_sy);
}  /* I_return */
/* -------------------------------------------------------------- */
/*  SAY [expr];                                                   */
/*      evaluate EXPR and then display the result on the user's   */
/*      console.                                                  */
/* -------------------------------------------------------------- */
void  I_say ( void )
{
   Lstr *A=NULL;
   int   i;

   nextsymbol();
   if (flag==f_running) {
      AIR_print;
      A = I_expression(A);
      L2str(&A);
      if (A!=NULL) {
         ASCIIZ(A);
         bputs(STR(A));
/*
*   The above is faster
*
*	 for (i=0; i < LEN(A); i++)  bputchar(STR(A)[i]);  bputchar('\n');
*/
         m_free(A);
      }
      else {
         bputchar('\n');
      }
      mustbe_semicolon;
   } else  skiptill(semicolon_sy);
}  /* I_say */
/* -------------------------------------------------------------- */
/*  SELECT ;                                                      */
/*     WHEN expr [;] THEN [;] inst ;                              */
/*   [ WHEN expr [;] THEN [;] inst ; ]                            */
/*   [ OTHERWISE [;] [inst]... ];                                 */
/*  END ;                                                         */
/*      then WHEN expressions are evaluated in sequence until     */
/*      one results in "1". INSTR, immediately following it, is   */
/*      executed and control leaves the construct. If no          */
/*      EXPR evaluated to "1", control passes to the              */
/*      instructions following the OTHERWISE expression           */
/*      that must then be present.                                */
/* -------------------------------------------------------------- */
void  I_skipselect ( void );
void  I_select ( void )
{
   Lstr *A = NULL;
   int  _exec;

   depth++;
   if (depth > 250) error(ERR_FULL_CTRL_STACK);

   AIR_print;   nextsymbol();

   statement = in_select;
   if (flag == f_running) {
      for (;;) {
         skip_semicolons;
         Bptr = Pptr;  Blineno = lineno;
         if (symbol==ident_sy) {
           if (!Lcmp(symbolstr,"WHEN")) {
	      AIR_print;
	      nextsymbol();
	      statement = in_if;
              flag = f_running;
	      A = I_expression(A);
	      if (tracing==results_trace) tracevar(">>>",A);
	      _exec = O_bool(A);
	      skip_semicolons;
	      statement = in_select;
	      Bptr = Pptr;  Blineno = lineno;
	      if (symbol != then_sy)  error( ERR_THEN_EXPECTED );
	      if (_exec) AIR_print;
	      nextsymbol();
	      if (_exec) flag = f_running;
		    else flag = f_stopped;
	      I_instr( FALSE );
	      skip_semicolons;
              flag = f_running;
              if (_exec) {
                 m_free(A);
                 flag = f_stopped;
                 I_skipselect();
                 flag = f_running;
                 mustbe_semicolon;
                 return;
	      }
	      skip_semicolons;
	   }  else
	   if (!Lcmp(symbolstr,"OTHERWISE")) {
              flag = f_running;
	      nextsymbol();
	      I_instr_till_end;
              nextsymbol();   mustbe_semicolon;
              depth--;        m_free(A);
	      return;
	   }  else
	   if (!Lcmp(symbolstr,"END")) {
              AIR_print;   nextsymbol();
	      mustbe_semicolon;
              depth--;     m_free(A);
	      return;
	   }  else  error(ERR_WHEN_EXCEPTED);
	 } else error(ERR_WHEN_EXCEPTED);
      }
   }
               /* stopped */
   I_skipselect();
   mustbe_semicolon;
}  /* I_select */

void  I_skipselect ( void )
{
   depth--;
   for (;;) {
     skip_semicolons;
     if (symbol==ident_sy) {
        if (!Lcmp(symbolstr,"WHEN")) {
           statement = in_if;  skiptill(then_sy);
           nextsymbol();       statement = normal_st;
           I_instr(FALSE);     skip_semicolons;
        } else
        if (!Lcmp(symbolstr,"OTHERWISE")) {
           nextsymbol();       I_instr_till_end;
           nextsymbol();       return;
        } else
        if (!Lcmp(symbolstr,"END")) { nextsymbol(); return; }
        else error(ERR_WHEN_EXCEPTED);
     } else error(ERR_WHEN_EXCEPTED);
   }
} /* I_skipselect */
/* -------------------------------------------------------------- */
/*  SIGNAL [name] |                                               */
/*         [VALUE] expr |                                         */
/*         <ON | OFF> + ERROR   +      ;                          */
/*                    | HALT    |                                 */
/*                    | NOVALUE |                                 */
/*                    + SYNTAX  +                                 */
/*   o  NAME, jump to the label NAME specified. Any pending       */
/*      instructions, DO ... END, IF, SELECT, and INTERPRET       */
/*      are terminated.                                           */
/*   o  VALUE, may be used for an evaluated label name.           */
/*   o  ON|OFF, enable or disable exception traps. CONDITION      */
/*      must be ERROR, HALT, NOVALUE, or SYNTAX. Control          */
/*      passes to the label of the condition name if the event    */
/*      occurs whIle ON                                           */
/* -------------------------------------------------------------- */
void Signal( Label *Lab )
{
    if (Lab == NULL) error(ERR_UNEXISTENT_LABEL);

    setvar("SIGL",Blineno);

    Bptr = RxPtr = Lab->ptr;
    Blineno = lineno = Lab->nnn;
    if (tracing==labels_trace) {
       Bptr = RxPtr;    printcurline();
    }
   longjmp(mainblock,1);
} /* Signal */

void label( int );
/* -----------------------* I_signal *--------------------- */
void  I_signal ( void )
{
    Label  *Lab;
    Lstr   *A=NULL;

    if (flag==f_running) {
       AIR_print;   nextsymbol();
       if (symbol==ident_sy) {
	  if (!Lcmp(symbolstr,"OFF")) {
	     label(OFF); goto LABEL_SET;
	  }  else
	  if (!Lcmp(symbolstr,"ON"))  {
	     label(ON); goto LABEL_SET;
	  }  else
	  if (!Lcmp(symbolstr,"VALUE")) {
	     nextsymbol();
	     A = I_expression(A);
	     L2str(&A);
	     Lab = searchlabel( A, &label_scope );
	     m_free(A);
	  }
	  else
	     Lab = searchlabel( symbolstr, &label_scope );
       }
       else error(ERR_EXTRAD_DATA);
       nextsymbol();
       mustbe_semicolon;

       Signal(Lab);

LABEL_SET:
       nextsymbol();
       mustbe_semicolon;
    } else skiptill(semicolon_sy);
}  /* I_signal */

/* -----------------------* label *------------------------ */
void label(int state)
{
   nextsymbol();
   if (symbol==ident_sy) {
      if (!Lcmp(symbolstr,"ERROR"))      Error = state;
      else
      if (!Lcmp(symbolstr,"HALT"))       Halt = state;
      else
      if (!Lcmp(symbolstr,"NOVALUE"))    Novalue = state;
      else
      if (!Lcmp(symbolstr,"SYNTAX"))     Syntax = state;
      else error(ERR_EXTRAD_DATA);
   }
   else error(ERR_EXTRAD_DATA);
}  /* label */
/* -------------------------------------------------------------- */
/*        + +   +      + + +                   + +                */
/*        | | ? | ?... | | | All               | |;               */
/*  TRACE | |   | !... | | | Commands          | |                */
/*        | |   +      + | | Errors            | |                */
/*        | |              | Failure           | |                */
/*        | |   +      + | | Intermediates     | |                */
/*        | | ! | ?... | | | Labels            | |                */
/*        | |   | !... | | | Normal            | |                */
/*        | +   +      + + | Off               | |                */
/*        |                | Results           | |                */
/*        |                | Scan              | |                */
/*        |                +                   + |                */
/*        | (number)                             |                */
/*        +                                      +                */
/*                                                                */
/*   ______________                                               */
/*                                                                */
/*  Or, alternatively:                                            */
/*  TRACE [ string ] ;                                            */
/*                                                                */
/*      if OPTION is numeric, then (if negative) inhibit tracing  */
/*      for a number of clauses, else (if positive) inhibit debug */
/*      mode for a number of clauses. Otherwise, trace            */
/*      according to first character or OPTION:                   */
/*                                                                */
/*      A   (All) trace all clauses.                              */
/*      C   (Commands) trace all commands.                        */
/*      E   (Error) trace commands with non-zero return codes     */
/*          after execution.                                      */
/*      I   (Intermediates) trace intermediates evaluation        */
/*          results and name substitutions also.                  */
/*      L   (Labels) trace only labels.                           */
/*      N   (Negative or Normal) trace commands with              */
/*          negative return codes after execution (default        */
/*          setting).                                             */
/*      O   (Off) no trace.                                       */
/*      R   (Results) trace all clauses and exceptions.           */
/*      S   (Scan) display rest of program without any            */
/*          execution (shows control nesting).                    */
/*      !   turn command inhibition on or off, and trace          */
/*          according to next character.                          */
/*     null restores the default tracing actions.                 */
/*                                                                */
/*     TRACE VALUE expr may be used for an evaluated              */
/*     trace setting.                                             */
/* -------------------------------------------------------------- */
void  I_trace ( void )
{
   Lstr *A=NULL;

   if (flag==f_running) {
      AIR_print; nextsymbol();
      if (symbol==ident_sy || symbol==literal_sy) {
          if (!Lcmp(symbolstr,"VALUE")) {
              nextsymbol();
              A = I_expression(A);
              L2str(&A);
         } else {
              Lstrcpy(&A,symbolstr);
              nextsymbol();
         }

         ASCIIZ(A);
         set_trace(STR(A));
         m_free(A);
      }  else  error(ERR_INVALID_TRACE);
      mustbe_semicolon;
   } else  skiptill( semicolon_sy );
} /* I_trace */
/* -------------------------------------------------------------- */
/*  UPPER name [name]...                                          */
/*      translate the values of the specified individual          */
/*      variables to uppercase.                                   */
/* -------------------------------------------------------------- */
void  I_upper ( void )
{
   Idrec *A = NULL;

   if (flag == f_running) {
      AIR_print;   nextsymbol();
      while (symbol == ident_sy) {
         A = V_search( symbolisarray, symbolstr, scope );
         if (symbolisarray && stem) error(ERR_INVALID_STEM);
         if (A != NULL) {
            L2upper(A->var);
            if (tracing==results_trace) tracevar(">>>",A->var);
         }
         nextsymbol();
      }
      mustbe_semicolon;
   } else  skiptill(semicolon_sy);
} /* I_upper */
/* -------------------------------------------------------------- */
/* I_error,  reports an error, when an illegal symbol is found.   */
/* -------------------------------------------------------------- */
void  I_error ( void )
{
   if (flag==f_running) {
      if (!Lcmp(symbolstr,"ELSE")) error(ERR_THEN_UNEXCEPTED);
      else
      if (!Lcmp(symbolstr,"OTHERWISE")) error(ERR_WHEN_UNCEPTED);
      else
      if (!Lcmp(symbolstr,"PROCEDURE")) error(ERR_UNEXPECTED_PROC);
      else
      if (!Lcmp(symbolstr,"THEN")) error(ERR_THEN_UNEXCEPTED);
      else
      if (!Lcmp(symbolstr,"WHEN")) error(ERR_WHEN_UNCEPTED);
   } else
     skiptill(semicolon_sy);
}  /* I_error */
/* -------------------------------------------------------------- */
/*  HostCmd                                                       */
/* Execute a host command according to environment                */
/* -------------------------------------------------------------- */
void HostCmd( void )
{
   Lstr *A=NULL;

   if (flag==f_running) {
      AIR_print;
      A = I_expression(A);
      if (tracing==results_trace) tracevar(">>>",A);
      I_cmd(A,environment);
      m_free(A);
      mustbe_semicolon;
   }  else  skiptill(semicolon_sy);
}  /* HostCmd */
/* -------------------------------------------------------------- */
/*  name = [expr];                                                */
/*       is an assignment: the variable name is set to the value  */
/*       of expr.                                                 */
/* -------------------------------------------------------------- */
I_cmd_or_assign ( void )
{
   Idrec  *A=NULL;
   Lstr   *B=NULL;

#ifdef __BEOS__
      if (be_was_interrupted()) longjmp(mainblock,2);
#endif

   if ((symbol==literal_sy) || (symbol==function_sy) || (symbol==le_parent)) {
      HostCmd();
      return TRUE;
   } else
   if (*RxPtr == '=')  {   /*  if the following character is '=' */
     if (flag==f_running) {
        AIR_print;
        A = V_create( symbolisarray, symbolstr, &scope );
        nextsymbol();
        mustbe(eq_sy, ERR_INVALID_EXPRESSION);
        B = I_expression(B);
        if (tracing == results_trace) tracevar(">>>",B);
        assign(A,B);
        m_free(B);
        mustbe_semicolon;
     } else
         skiptill(semicolon_sy);
     return TRUE;
   } else return FALSE;
}  /* I_cmd_or_assign */
/* ---------------------------------------------------------- */
struct sort_list_st {
           char *name;
           void (*func)(void);
       }
/* //// WARNING THE LIST MUST BE SORTED!!!!!!!!!!!! */
statements_list[] = {
           {"ADDRESS",   I_address   },
           {"ARG",       I_arg       },
           {"CALL",      I_call      },
           {"DO",        I_do        },
           {"DROP",      I_drop      },
           {"ELSE",      I_error     },
           {"EXIT",      I_exit      },
           {"IF",        I_if        },
           {"INTERPRET", I_interpret },
           {"ITERATE",   I_iterate   },
           {"LEAVE",     I_leave     },
           {"LOWER",     I_lower     },
           {"NOP",       I_nop       },
           {"NUMERIC",   I_numeric   },
           {"OTHERWISE", I_error     },
           {"PARSE",     I_parse     },
           {"PROCEDURE", I_error     },
           {"PULL",      I_pull      },
           {"PUSH",      I_push      },
           {"QUEUE",     I_queue     },
           {"RETURN",    I_return    },
           {"SAY",       I_say       },
           {"SELECT",    I_select    },
           {"SIGNAL",    I_signal    },
           {"THEN",      I_error     },
           {"TRACE",     I_trace     },
           {"UPPER",     I_upper     },
           {"WHEN",      I_error     }
};

#define LISTSIZE(a) sizeof((a)) / sizeof(struct sort_list_st) -1
/* -------------------------------------------------------------- */
/*  [ Instr ] ::=  [ All acceptable commands ] | [ Expression ]   */
/* -------------------------------------------------------------- */
int I_instr ( boolean return_with_end )
{
   int first, middle, last, cmp, found;

   statement = nothing_st;
   /* skip null lines, comments and labels.... */
   Bptr = RxPtr;   Blineno = 1;
   while ((symbol==semicolon_sy) || (symbol==label_sy)) nextsymbol();

   if (symbol==exit_sy) I_exit();

   statement = normal_st;
   Bptr = Pptr;   Blineno = lineno;
   if (!I_cmd_or_assign()) {
      if (symbol!=ident_sy) HostCmd(); /*error(ERR_INVALID_EXPRESSION);*/
      else {
         if (!Lcmp(symbolstr,"END")) {
	    if (return_with_end) return TRUE;
	    else error(ERR_UNMATCHED_END);
         }
         first = found = 0;    /* Use binary search to find intruction */
         last  = LISTSIZE(statements_list);
         while (first<=last)   {
            middle = (first+last)/2;
            if ((cmp=Lcmp(symbolstr,statements_list[middle].name))==0)  {
               found=1;
               break;
            }  else
           if (cmp<0) last = middle-1;
           else      first = middle+1;
         }

         if (found) (*statements_list[middle].func)();
         else       HostCmd();
      }
   }
   statement = nothing_st;
   return FALSE;
}  /* I_instr */

/* -------------------------------------------------------------- */
/*  Executes a block of instructions                              */
/* -------------------------------------------------------------- */
void  I_block ( void )
{
   for (;;) {       /* do forever */
      I_instr( FALSE );
   }
} /* I_block */

/* -------------------------------------------------------------- */
/*  Executes a block of instructions and takes care of jmp control*/
/* -------------------------------------------------------------- */
int Main_block ( void )
{
   volatile jmp_buf  old_mainblock;
   volatile jmp_buf  old_do_instr;
   volatile int      old_depth;
/*//   volatile int      old_lineno; */
   volatile int      result;

   old_depth = depth;
   Jcpy(old_mainblock, mainblock);
   Jcpy(old_do_instr, do_instr);

setjumper:
   if ((result=setjmp(mainblock))!=0) {    /* //////////////// */
      depth = old_depth;
      if (result == 1) {       /* from signal */
         if (INTERPRET_LEVEL) {
            IN_SIGNAL = TRUE;
            goto telos_kalo;
         }
      } else
      if (result == 2) goto telos_kalo;  /* from exit */
      if (result == 3) goto telos_kalo;  /* from return */
      symbol=semicolon_sy;
      AIR_print;
      goto setjumper;
   }
   if (setjmp(do_instr)) error(ERR_INVALID_LEAVE);
   IN_SIGNAL = FALSE;

   I_block();
   result = 0;

telos_kalo:
   Jcpy(mainblock, old_mainblock);
   Jcpy(do_instr, old_do_instr);
   return result;
} /* I_block */
