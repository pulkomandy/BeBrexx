/* ------------------------------------------------------ o o --- */
/*                                                      ____oo_   */
/*                                           (c)1992   /||    |\  */
/*  Tokens and Terms                           BNV      ||    |   */
/*  ~~~~~~~~~~~~~~~~                                    '.___.'   */
/*                                                      MARMITA   */
/*  REXX expressions and instructions may contain the following   */
/*  items:                                                        */
/*                                                                */
/*  comment                                                       */
/*      A comment is a sequence of characters (on one or          */
/*      more lines) delimited by \* and *\.                       */
/*                                                                */
/*  string                                                        */
/*      A string is a sequence of characters delimited by a       */
/*      single quote or double quote. Use two quotes to           */
/*      obtain one quote inside a string. A string may be         */
/*      specified in hexadecimal if the final quote is            */
/*      followed by an X. If it is followed by an H then          */
/*      is treated as a hexadecimal number.                       */
/*      Some valid strings are:                                   */
/*        "Next" 'Don''t touch'  '1de8'x   '10'h (=16)            */
/*                                                                */
/*  number                                                        */
/*      A number is a string of decimal digits before and/or      */
/*      after an (optional) decimal point. A number may be        */
/*      prefixed with a plus or minus sign, and/or written in     */
/*      exponential notation. Some valid numbers are:             */
/*        17  98.07  101 12.2e6                                   */
/*                                                                */
/*  symbol                                                        */
/*      A symbol refers to any group of characters from the       */
/*      following selection:                                      */
/*                                                                */
/*         A-Z, a-z, 0-9, @#$_.?!                                 */
/*                                                                */
/*      A symbol may consist of up to 250 characters.             */
/*                                                                */
/*  function-call                                                 */
/*      A function-call invokes an internal, external, or built-in*/
/*      routine with 0 to 10 argument strings. The called         */
/*      routine returns a character string. A function-call has   */
/*      the format:                                               */
/*                                                                */
/*      function-name( [expr][,[expr]...)                         */
/*                                                                */
/*      function-name must be adjacent to the left                */
/*      parenthesis, and may be a symbol or a string.             */
/*                                                                */
/* -------------------------------------------------------------- */
/* 
 * $Header: c:/usr/rexx/NEXTSYMB.C!v 1.3 1997/07/04 20:32:36 bill Exp bill $
 * $Log: NEXTSYMB.C!v $
 * Revision 1.3  1997/07/04  20:32:36  bill
 * *** empty log message ***
 *
 * Revision 1.2  1995/10/02 19:49:23  bill
 * Fixed problem with real numbers exponext sign
 * ie  1.6E-19 in the old version was treated as 1.6E - 19 leading in a
 * bad arithmetic conversion for 1.6E
 *
 * Revision 1.1  1995/09/11  03:32:49  bill
 * Initial revision
 *
 */
#define  __NEXTSYMB_C__

#include <stdio.h>

#include "rexx.h"
#include "error.h"
#include "convert.h"

/* -------------* function prototypes *---------------- */
void nextchar   ( void );
void identifier ( int  );
void comment    ( void );
void literal    ( void );

void todecimal(Lstr *A, long n);  /* from RXCONV.C */

/* -------------------------------------------------------------- */
/*           P A R S E   next  B A S I C   S Y M B O L            */
/* Return the next basic symbol and advance the input stream      */
/* -------------------------------------------------------------- */
void nextsymbol(void)
{
   char  *_RxPtr;
   int    _lineno;

   Pptr   = RxPtr;
   Pblank = Nblank;
   Nblank = FALSE;
_NEXTSYMBOL:

   while (*RxPtr==' ' || *RxPtr=='\t') nextchar();

   switch (l2u[*RxPtr]) {
      case '0':    case '1':    case '2':
      case '3':    case '4':    case '5':
      case '6':    case '7':    case '8':
      case '9':    symbol = literal_sy;
                   identifier(TRUE);
                   break;

      case 'A':    case 'B':    case 'C':
      case 'D':    case 'E':    case 'F':
      case 'G':    case 'H':    case 'I':
      case 'J':    case 'K':    case 'L':
      case 'M':    case 'N':    case 'O':
      case 'P':    case 'Q':    case 'R':
      case 'S':    case 'T':    case 'U':
      case 'V':    case 'W':    case 'X':
      case 'Y':    case 'Z':

      case '@':
      case '#':
      case '$':
      case '_':
      case '?':
      case '!':    symbol = ident_sy;
                   identifier(FALSE);
                   break;

      case '/':    nextchar();
                   if (*RxPtr=='*')  {
                      comment();
                      goto _NEXTSYMBOL;
                   }
                   if (*RxPtr=='/')  {
                      symbol = mod_sy;
                      nextchar();
                   }  else
                   if (*RxPtr=='=')  {
                      nextchar();
                      if (*RxPtr=='=')  {
                         nextchar();
                         symbol = nde_sy;
                      }  else symbol = ne_sy;
                   }  else symbol = div_sy;
                   break;

      case '-':    nextchar();
                   symbol = minus_sy;
                   break;

      case '*':    nextchar();
                   if (*RxPtr=='/')  error(ERR_INVALID_EXPRESSION);
                   if (*RxPtr=='*')  {
                       symbol = power_sy;
                       nextchar();
                   }
                   else symbol = times_sy;
                   break;

      case '\\':
      case '~':
      case '^':    nextchar();
                   switch (*RxPtr)  {
                     case '=' :  nextchar();
                                 if (*RxPtr=='=') {
                                    symbol = nde_sy;
                                    nextchar();
                                 }  else symbol = ne_sy;
                                 break;

                     case '>' :  nextchar();
                                 symbol = le_sy;
                                 break;

                     case '<' :  nextchar();
                                 symbol = ge_sy;
                                 break;
                     default:
                                 symbol = not_sy;
                   }
                   break;

      case '%':    nextchar();
                   symbol = intdiv_sy;
                   break;

      case '(':    nextchar();
                   symbol = le_parent;
                   break;

      case ')':    nextchar();
                   symbol = ri_parent;
                   break;

      case '\n':   symbol = semicolon_sy;
                   nextchar();
                   break;

      case ';':    nextchar();
                   symbol = semicolon_sy;
                   break;

      case ',':    nextchar();
                  _lineno = lineno;
                  _RxPtr = RxPtr; nextsymbol();
                   Nblank = FALSE;
                   Pblank = TRUE;
                   if (symbol==semicolon_sy &&
                  _lineno != lineno)   goto _NEXTSYMBOL;
                   RxPtr = _RxPtr;
                   symbol = comma_sy;
                   break;

      case '.':    if ((*(RxPtr+1)!=' ')   &&
                       (*(RxPtr+1)!=',')   &&
                       (*(RxPtr+1)!='\n')  &&
                       (*(RxPtr+1)!='\t')  &&
                       (*(RxPtr+1)!=';') )  {
                      symbol = literal_sy;
                      identifier(TRUE);
                   }  else  {
                      nextchar();
                      symbol = dot_sy;
                   }
                   break;

      case '|':    nextchar();
                   if (*RxPtr=='|')  {
                      nextchar();
                      symbol = concat_sy;
                   }  else  symbol = or_sy;
                   break;

      case '&':    nextchar();
                   if (*RxPtr=='&')  {
                      nextchar();
                      symbol = xor_sy;
                   }  else  symbol = and_sy;
                   break;

      case '+':    nextchar();
                   symbol = plus_sy;
                   break;

      case '=':    nextchar();
                   switch (*RxPtr)  {
                      case '=' : symbol = deq_sy;
                                 nextchar();
                                 break;

                      case '>' : symbol = ge_sy;
                                 nextchar();
                                 break;

                      case '<' : symbol = le_sy;
                                 nextchar();
                                 break;

                      default:   symbol = eq_sy;
                   }
                   break;

      case '<':    nextchar();
                   switch (*RxPtr)  {
                      case '=' : symbol = le_sy;
                                 nextchar();
                                 break;

                      case '>' : symbol = ne_sy;
                                 nextchar();
                                 break;

                      default:   symbol = lt_sy;
                   }
                   break;

      case '>':    nextchar();
                   switch (*RxPtr)  {
                      case '=' : symbol = ge_sy;
                                 nextchar();
                                 break;

                      case '<' : symbol = ne_sy;
                                 nextchar();
                                 break;

                      default:   symbol = gt_sy;
                   }
                   break;

      case '\'':
      case '\"':   literal();
                   break;

      case '\0':
                   if ((statement == in_do) ||
                       (statement == in_select) ||
                       (statement == in_if) )
                      error (ERR_INCOMPLETE_STRUCT);
                   if (statement == normal_st)
                      error (ERR_SYMBOL_EXPECTED);
                   if (statement == nothing_st) {
                      symbol = exit_sy;
                      return;
                   }
                   longjmp(finito,1); /* /////////////////// */
                   break;

      default:
                   bprintf("%d\n",(unsigned int)*RxPtr);
                   error(ERR_INVALID_CHAR);
   }
   if ((*RxPtr==' ') ||
       (*RxPtr=='\t') ||
       (*RxPtr==',')) Nblank = TRUE;
}  /* nextsymbol */

/* ----------------------------------------------------------- */
/*return the next character and advance the input stream by one*/
/* ----------------------------------------------------------- */
void nextchar(void)
{
   if (*RxPtr=='\n') lineno++;
   if (*RxPtr) RxPtr++;
} /* nextchar */

/* -------------------------------------------------------------- */
/*     find the identifier                                        */
/* -------------------------------------------------------------- */
void identifier(int isnumber)
{
   char   *s;
   short   l;     /* length */     /* -+-  l > maxlen ? */
   int    hasDot=FALSE, hasExp=FALSE;

   symbolisarray = FALSE;
   symbolisstr   = FALSE;
   TYPE(symbolstr) = STR_TY;
   s = STR(symbolstr);
   l = 0;

   for (;;)
      switch (l2u[*RxPtr]) {
          case '0':   case '1':    case '2':
          case '3':   case '4':    case '5':
          case '6':   case '7':    case '8':
          case '9':   *s++ = *RxPtr;  l++;
                      nextchar();
                      break;

          case 'A':   case 'B':    case 'C':
          case 'D':   case 'E':    case 'F':
          case 'G':   case 'H':    case 'I':
          case 'J':   case 'K':    case 'L':
          case 'M':   case 'N':    case 'O':
          case 'P':   case 'Q':    case 'R':
          case 'S':   case 'T':    case 'U':
          case 'V':   case 'W':    case 'X':
          case 'Y':   case 'Z':

          case '@':
          case '#':
          case '$':
          case '_':
          case '?':
          case '!':   *s = l2u[*RxPtr];  l++;
                      if (isnumber) {
                         if (*s=='E') {
                            if (hasExp) isnumber = FALSE;
                            hasExp = TRUE;
                         } else
                            isnumber = FALSE;
                      }
                      s++;
                      nextchar();
                      break;

          case '+':
          case '-':   if (isnumber) {
                         if (*(s-1)=='E') {
                            *s++ = *RxPtr; l++;
                            nextchar();
                         } else {
                            *s='\0';  LEN(symbolstr) = l;
                            goto leave;
                         }
                      } else {
                          *s='\0';  LEN(symbolstr) = l;
                          goto leave;
                      }
                      break;

          case '.':   *s++ = *RxPtr;  l++;
                      symbolisarray = TRUE;
                      nextchar();
                      if (isnumber) {
                         if (hasDot) isnumber = FALSE;
                         hasDot = TRUE;
                      }
                      break;

          case '(':   nextchar();
                      *s = '\0';    LEN(symbolstr) = l;
                      symbol = function_sy;
                      return;

          case ':':   nextchar();
                      *s = '\0';    LEN(symbolstr) = l;
                      symbol = label_sy;
                      if (flag==f_running)
                        if (tracing == labels_trace) {
                           tracing = all_trace;
                           printcurline();
                           tracing = labels_trace;
                        }
                      return;

          case '\t':
          case ' ':   Nblank = TRUE;
                      *s = '\0';    LEN(symbolstr) = l;
                      while (*RxPtr==' '||*RxPtr=='\t') nextchar();
                      if (*RxPtr!=':') goto leave;    /* literal finished and it is not a label */

                      /* literal is label */
                      symbol = label_sy;
                      if (flag==f_running)
                        if (tracing == labels_trace) {
                           tracing = all_trace;
                           printcurline();
                           tracing = labels_trace;
                        }
                      nextchar();
                      return;

          default:    *s = '\0';    LEN(symbolstr) = l;
                      goto leave;
      }  /* end of switch */

leave:
   if (symbol!=ident_sy) return ;

   if (statement == in_do_init)  {
           if (Lcmp(symbolstr,"BY"   )==0)  symbol = by_sy;
      else if (Lcmp(symbolstr,"FOR"  )==0)  symbol = for_sy;
      else if (Lcmp(symbolstr,"TO"   )==0)  symbol = to_sy;
      else if (Lcmp(symbolstr,"UNTIL")==0)  symbol = until_sy;
      else if (Lcmp(symbolstr,"WHILE")==0)  symbol = while_sy;
   } else
   if (statement == in_parse_value) {
           if (Lcmp(symbolstr,"WITH" )==0)  symbol = with_sy;
   } else
   if (statement == in_if) {
           if (Lcmp(symbolstr,"THEN" )==0)  symbol = then_sy;
   }
}   /* identifier */

/* -------------------------------------------------------------- */
/* extract comments from source... checking is done in readfile   */
/* -------------------------------------------------------------- */
void comment(void)
{
   int level=1;
   nextchar();
   for (;;)
   {
      while ((*RxPtr!='*') && (*RxPtr!='/')) {
         nextchar();
         if (*RxPtr == '\0') error(ERR_UNMATCHED_QUOTE);
/*       if (tracing in air_trace then write(*RxPtr);   -+- tracing */
      };
      nextchar();
      if ((*(RxPtr-1)=='/') && (*RxPtr=='*')) {
         level++;  nextchar();
      }  else
      if ((*(RxPtr-1)=='*') && (*RxPtr=='/')) {
         --level;
         nextchar();
/*             if tracing in air_trace then writeln('* /');  -+- tracing */
         if (!level) return;
      }
   }
} /* comment */
/* -------------------------------------------------------------- */
/* extract a literal symbol                                       */
/* -------------------------------------------------------------- */
void literal(void)
{
   char    quote;
   char   *s;
   short   l;  /* length of symbolstr */
   Lstr   *A;          /* NULLED BELOW */

   symbol = literal_sy;
   quote  = *RxPtr;
   TYPE(symbolstr) = STR_TY;
   s = STR(symbolstr);
   l = 0;
   symbolisstr = TRUE;

   for (;;)  {                   /* -+-  l > maxlen ? */
      nextchar();
      if (l>250) error(ERR_TOO_LONG_STRING);
      if (*RxPtr==quote) {
         nextchar();
         switch (*RxPtr) {
           case 'x':  case 'h':
           case 'X':  case 'H':
                *s = '\0';  LEN(symbolstr) = l;
                A = NULL;
                Lx2c(&A,symbolstr);
                if (A==NULL) error(ERR_INVALID_HEX_CONST);
                if (l2u[*RxPtr]=='H') todecimal(A,0);
                Lstrcpy(&symbolstr,A);
                m_free(A);
                nextchar();
                return;
           case '(':
                *s = '\0';  LEN(symbolstr) = l;
                symbol = function_sy;
                nextchar();
                return;
           default:
                if (*RxPtr==quote)  {
                   *s++ = *RxPtr; l++;
                   nextchar();
                }  else  {
                   *s = '\0';  LEN(symbolstr)=l;
                   return;
                }
         }
      }
      if (*RxPtr == '\n') *s++ = ' ';
      else
      if (*RxPtr == '\0') error(ERR_UNMATCHED_QUOTE);
      else              *s++ = *RxPtr;

      l++;
   }
}  /* literal */
