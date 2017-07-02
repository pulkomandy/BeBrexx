/* ----------------------------------------------------- o o ---- */
/*                                                     ____oo_    */
/*                                          (c)1992   /||    |\   */
/*  Templates for ARG, PULL, and PARSE        BNV      ||    |    */
/*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                 `.___.'    */
/*                                                     MARMITA    */
/*  The PULL, ARG and PARSE instructions use a template to        */
/*  parse a string. The template specifies the name of variables  */
/*  that are to be given new values. Each name in the template is */
/*  assigned one word (without any leading or trailing blanks)    */
/*  from the input string in sequence, except that the last name  */
/*  is assigned the remainder of the string (in any) unedited. If */
/*  there are fewer words in the string than names in the         */
/*  template, all excess variables are set to null. In all cases, */
/*  all the variables in the template are given a new value.      */
/*                                                                */
/*  If PULL, ARG or PARSE UPPER is used, then the whole string    */
/*  is translated to uppercase before parsing starts. When        */
/*  this translation is not desired, use the PARSE instruction    */
/*  (without the UPPER option).                                   */
/*                                                                */
/*  The parsing algorithm also allows some pattern matching, in   */
/*  which you may "trigger" on either a string (possibly a        */
/*  variable) or on a column number.                              */
/*                                                                */
/*  If the template contains such trigger, then alignment will    */
/*  occur at the next point where the trigger exactly matches     */
/*  the data. A trigger match splits the string up into           */
/*  seperate parts, each of which is parsed in the same way       */
/*  as a complete string would be if no triggers were used.       */
/*                                                                */
/*  A comma can be used as a "trigger" to move to the next        */
/*  string when there is more than one to be parsed (e.g. when    */
/*  there is more than one argument string to a routine).         */
/*                                                                */
/* -------------------------------------------------------------- */
/*
 * $Header: c:/usr/rexx/template.c!v 1.5 1997/09/15 23:54:18 bill Exp bill $
 * $Log: template.c!v $
 * Revision 1.5  1997/09/15  23:54:18  bill
 * Changed to handle correct the
 * template a b 1 a b
 * where b should have the rest of the parsing string
 *
 * Revision 1.4  1996/12/13 01:04:24  bill
 * *** empty log message ***
 *
 * Revision 1.3  1995/10/02  19:51:18  bill
 * A minor correction in Lstrsub to accept zero values for length
 *
 * Revision 1.2  1995/10/01  20:57:23  bill
 * Lstrsub can lead to fatal error if length is less than zero
 *
 * Revision 1.1  1995/09/11  03:31:16  bill
 * Initial revision
 *
 */

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"

void I_template ( Lstr *A )
{
   enum     symboltype oldsymbol;
   int      oldsymbolwasarray;
   Lstr    *oldsymbolstr=NULL;
   Idrec   *Id;
   int      P, NP, LP, NLP;
   boolean  more;
   Lstr    *B=NULL;

   L2str(&A);
   NP = LP = NLP = 0;
   oldsymbol = symbol;
   oldsymbolwasarray = symbolisarray;
   Lstrcpy(&oldsymbolstr, symbolstr);

   if ((symbol==ident_sy) || (symbol==dot_sy)) nextsymbol();

   while ((symbol!=semicolon_sy) &&
          (symbol!=comma_sy))
   {
      if (LP>LEN(A)) LP = LEN(A);
      P = LP;
      switch (symbol) {
        case ident_sy:
        case dot_sy:    if ((oldsymbol == ident_sy) ||
                            (oldsymbol == dot_sy)) {
                           more = TRUE;  LP = P;
                           while ((LP < LEN(A)) && more)
                             if (STR(A)[LP]==' ') LP++;
                             else     more = FALSE;
                           more = TRUE;  P = LP;
                           while ((P<LEN(A)) && more)
                             if (STR(A)[P]!=' ') P++;
                             else     more = FALSE;
                           NP = NLP = P;

                           more = TRUE;  
                           while ((NLP < LEN(A)) && more)
                             if (STR(A)[NLP]==' ') NLP++;
                             else     more = FALSE;
                        }
                        break;

        case minus_sy:
                        nextsymbol();
                        if ((symbol != literal_sy) ||
                            (datatype(symbolstr)!='I'))
                               error(ERR_INVALID_TEMPLATE);
                        L2int( &symbolstr );
                        P = NP - (int)INT(symbolstr);
                        if (P < 0) P = 0;
                        NP = NLP = P;
                        break;

        case plus_sy:
                        nextsymbol();
                        if ((symbol != literal_sy) ||
                            (datatype(symbolstr)!='I'))
                               error(ERR_INVALID_TEMPLATE);
                        L2int( &symbolstr );
                        P = NP + (int)INT(symbolstr);
                        NP = NLP = P;
                        break;

        case le_parent:
                        nextsymbol();
                        if (symbol != ident_sy) error(ERR_INVALID_TEMPLATE);
                        V_eval( symbolisarray, &B, symbolstr, scope );
                        L2str(&B);
                        P = (size_t)Lindex(A,B,P+1);
                        if (!P)  P = LEN(A);
                        else P--;
                        if (P==LP) LP=P-1; /* So Null string will be copied */
                        NP = P;
                        NLP = P + LEN(B);
                        nextsymbol();
                        if (symbol!=ri_parent) error(ERR_INVALID_TEMPLATE);
                        break;

        case literal_sy:
                        if (symbolisstr) {
                           P = (size_t)Lindex(A,symbolstr,P+1);
                           if (!P) P = LEN(A);
                           else P--;
                           if (P==LP) LP=P-1; /* So Null string will be copied */
                           NP = P;
                           NLP = P + LEN(symbolstr);
                        } else {
                           if (datatype(symbolstr) != 'I')
                              error (ERR_INVALID_INTEGER);
                           L2int(&symbolstr);
                           P = (int)INT(symbolstr)-1;
                           if (P<0) P = 0;
                           NP = NLP = P;
                        }
                        break;
        default:
                        error(ERR_INVALID_TEMPLATE);
      } /* end of switch */

      if (oldsymbol == ident_sy) {
         Id = V_create( oldsymbolwasarray, oldsymbolstr, &scope );

         /* WATCH FOR NEGATIVE VALUES IN P-LP OR ZERO */
         /* Lstrsub has funny results */

         if (P > LEN(A))
              Lstrsub( &(Id->var), A, LP, 0 );
         else if (P-LP>=0) Lstrsub( &(Id->var), A, LP, P-LP );
              else Lstrsub( &(Id->var), A, LP, 0 );
/*                     else Lscpy(&(Id->var),""); */
         if (tracing)
            if (tracing & AIR_trace) tracevar(">>>", Id->var);
      } else
      if (oldsymbol == dot_sy) {
         if (tracing & AIR_trace) {
            if (P > LEN(A))
                   Lstrsub( &B, A, LP, 0    );
              else if (P-LP>=0) Lstrsub( &B, A, LP, P-LP );
                    else Lstrsub( &B, A, LP, 0 );
/*                          else Lscpy( &B, "" ); */
            tracevar(">.>",B);
         }
      }
      oldsymbol = symbol;
      oldsymbolwasarray = symbolisarray;
      Lstrcpy(&oldsymbolstr, symbolstr);
      nextsymbol();
      LP = NLP;
   }
   P = NLP;
   if (oldsymbol == ident_sy)  {
      Id = V_create( oldsymbolwasarray, oldsymbolstr, &scope );
      Lstrsub( &(Id->var), A, P, 0 );
      if (tracing & AIR_trace) tracevar(">>>", Id->var);
   } else
   if (oldsymbol == dot_sy)
      if (tracing & AIR_trace) {
         Lstrsub( &B, A, LP, 0  );
         tracevar(">.>",B);
      }
   m_free(B);
   m_free(oldsymbolstr);
}  /* I_template */
