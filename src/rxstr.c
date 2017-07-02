/*
 * $Header: c:/usr/rexx/RXSTR.C!v 1.2 1997/07/04 20:32:24 bill Exp bill $
 * $Log: RXSTR.C!v $
 * Revision 1.2  1997/07/04  20:32:24  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11 03:31:59  bill
 * Initial revision
 *
 */
#include <stdlib.h>

#ifdef __MSDOS__
#include <dos.h>
#endif
#include <string.h>

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"

#include "rxdefs.h"

#include <ctype.h>
/* ------------------------* is_symbol *------------------- */
int  is_symbol( Lstr  *S )
{
   Lstr *A=NULL;
   long  v;

   L2upper(S);
   Lcat(&A,cUPPER);  Lcat(&A,cdigits);
   Lcat(&A,crxsymb);
   v = (int)(Lverify(S,A,FALSE,0)+1);
   m_free(A);
   if  ((v!=0) || (LEN(S)==0)) return FALSE;
   return TRUE;
} /* is_symbol */

/* ------------------------* Soundex *--------------------------- */
#define MAX_LENGTH     20  /* max # of chars to check */
#define SOUNDEX_LENGTH  4  /* length of Soundex code  */

			   /* ALPHA_OFFSET is decimal
		  value of 'A' in ASCII character set */
#define ALPHA_OFFSET    65
char *Soundex( char *name, int len )
{
   /* ctable contains the Soundex value of each
      letter in alphabetical order. 0 represents
      letters which are to be ignored. */
   static char code[SOUNDEX_LENGTH+1];
   static char ctable[] =  { "01230120022455012623010202" };
   char prior = ' ', c;
   short i, y = 0;
		       /* convert name to all upper case */
   if (len >= MAX_LENGTH) name[MAX_LENGTH] = '\0';
		      /* generate 1st character of Soundex code */
   code[0] = name[0];
   y=1;

   if (name[0]=='K')                 /* modifications */
      code[0] = 'C';
   else if (name[0]=='P' && name[1]=='H')
      code[0] = 'F';
	       /* loop through the rest of name, until code complete */
   for (i=1; name[i]; i++) {
      if (!isalpha(name[i]))  continue; /* skip non alpha */
			      /* skip succesive occurance */
      if (name[i]==prior)  continue;
      prior = name[i];
			      /* lookup letter in table   */
      c = name[i] - ALPHA_OFFSET;
      if (ctable[c]=='0')  continue; /* ignore this letter */

      code[y++] = ctable[c];        /* add to code */

      if (y >= SOUNDEX_LENGTH) break;  /* code is complete */
   }
   while (y < SOUNDEX_LENGTH) code[y++] = '0';   /* pad code with zeros */
   code[y] = '\0';
   return (code);
} /* soundex */

/* -------------------------------------------------------------- */
/*  ABBREV(information,info[,length])                             */
/*       tests whether info is a true abbreviation of             */
/*       information, with minimum length.                        */
/* -------------------------------------------------------------- */
/*  INDEX(haystack,needle[,start])                                */
/*       return the position of needle in haystack,               */
/*       beginning at start.                                      */
/* -------------------------------------------------------------- */
/*  LASTPOS(needle,haystack[,start])                              */
/*       return the position of the last occurence of needle in   */
/*       haystack, beginning at start.                            */
/* -------------------------------------------------------------- */
/*  POS(needle,haystack[,start])                                  */
/*       return the position of needle in haystack,               */
/*       beginning at start.                                      */
/* -------------------------------------------------------------- */
/*  WORDPOS(phrase,string[,start])                                */
/*       returns the word number of the first word of phrase      */
/*       in string. Returns "0" if phrase is not found            */
/* -------------------------------------------------------------- */
void R_SSoI( args *arg, int func )
{
   long     l;
   size_t   i;
   boolean  more;

   if (!IN_RANGE(2,ARGN,3)) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   must_exist(2);  L2str(&ARG2);
   get_oi0(3,l);

   switch (func) {
      case f_abbrev:
	      if (!exist(3)) l = LEN(ARG2);
              if (LEN(ARG2)==0) {
                    ARGR = ARG1; ARG1 = NULL;
                    Licpy(&ARGR,TRUE);
                    return;
              }
              if (( LEN(ARG1) < LEN(ARG2) ) ||
                 ( LEN(ARG2) < l )) {
                 ARGR = ARG1; ARG1 = NULL;
                 Licpy(&ARGR,FALSE);
                 return;
              }
              i = 0;  more = TRUE;
              while ( (i<l && more) )
              if (STR(ARG1)[i] == STR(ARG2)[i]) i++;
                                  else  more = FALSE;
              ARGR = ARG1; ARG1 = NULL;
              Licpy(&ARGR,more);
              break;

      case f_index:
              Licpy(&ARGR, Lindex(ARG1, ARG2, (size_t)l));
              break;

      case f_lastpos:
              if (!LEN(ARG1)) {
                 Licpy(&ARGR,LEN(ARG2));
                 return;  }
              Lreverse(ARG1);
              Lreverse(ARG2);
              if (l>LEN(ARG2)) l = LEN(ARG2);
              if (l) l = LEN(ARG2) - (size_t)l + 1;
              i = (size_t)Lindex(ARG2, ARG1, (size_t)l);
              if (i)  i = LEN(ARG2) - (i + LEN(ARG1) - 1) + 1 ;
              Licpy(&ARGR, i);
              break;

      case f_pos:
              Licpy(&ARGR, Lindex(ARG2, ARG1, (size_t)l));
              break;

      case f_wordpos:
              Licpy(&ARGR, Lwordpos(ARG1,ARG2, (size_t)l));
              break;
      default:   error(ERR_INTERPRETER_FAILURE);
   } /* switch */
} /* R_SSoI */
/* -------------------------------------------------------------- */
/*  CENTRE(string,length[,pad])                                   */
/*  CENTER(string,length[,pad])                                   */
/*       return string centered (length LENGTH)                   */
/* -------------------------------------------------------------- */
/*  JUSTIFY(string,length[,pad])                                  */
/*       justify string to both margins (the width of margins     */
/*       equals length), by adding pads between words.            */
/* -------------------------------------------------------------- */
/*  LEFT(string,length[,pad])                                     */
/*       return a string of length LENGTH with STRING left        */
/*       justified in it.                                         */
/* -------------------------------------------------------------- */
/*  RIGHT(string,length[,pad])                                    */
/*       return a string of length LENGTH with STRING right       */
/*       justified in it.                                         */
/* -------------------------------------------------------------- */
void R_SIoC ( args *arg, int func )
{
   long    i,a,l,s,sf;
   size_t  p,lp;
   char    pad,padstr[2];
   Lstr   *A=NULL;
   Lstr   *space=NULL;

   if (!IN_RANGE(2,ARGN,3)) error(ERR_INCORRECT_CALL);

   must_exist(1);  L2str(&ARG1);
   get_i0(2,l);
   get_pad(3);

   switch (func) {
      case f_center:
                       i = l - LEN(ARG1);
                       if (!i) {
                          ARGR = ARG1; ARG1 = NULL;
                          return;
                       }
                       if (i < 0) {
                          i = -i;    a = i / 2;
                          Lstrsub(&ARGR, ARG1, (size_t)a, (size_t)l);
                       } else  {
                          a = i / 2;
                          Lstrset(&ARGR,(size_t)a,pad);
                          Lstrcat(&ARGR,ARG1);
                          Lstrset(&ARG1,(size_t)(a+(i&1)),pad);
                          Lstrcat(&ARGR,ARG1);
                       }
                       break;

      case f_justify:
                        Lscpy(&ARGR,"");
                        if (!l) return;
                        Lspace(&ARGR,ARG1,1,' ');
                        if (LEN(ARGR)==l) return;
                        if (LEN(ARGR) > l) {
                           LEN(ARGR) = (size_t)l;
                           for (p=0; p<l; p++)
                              if (STR(ARGR)[p]==' ') STR(ARGR)[p] = pad;
                           return;
                        }
                        a = 0;    /* count spaces */
                        for (p=0; p<=LEN(ARGR); p++)
                           if (STR(ARGR)[p]==' ') a++;
                        s = l - (LEN(ARGR)-a);
                        sf = s % a;
                        if (sf) {
                           padstr[0] = pad;
                           padstr[1] = 0;
                        }
                        s = s / a;
                        ARG4=ARGR; ARGR = NULL;

                        Lstrset(&space,(size_t)s,pad);
                        for (p=0;;) {
                            lp = p;
                            p = skipword(ARG4,p);
                            Lstrsub(&A,ARG4,(size_t)lp,(size_t)(p-lp));
                            Lstrcat(&ARGR, A);
                            if (p>=LEN(ARG4)) {
                               m_free(space);
                               m_free(A);
                               return;
                            }
                            Lstrcat(&ARGR, space);
                            if (sf) {
                               Lcat(&ARGR, padstr);
                               sf--;
                            }

                            p = skipblanks(ARG4,p);
                        }

      case f_left:
                       if (!l) Lscpy(&ARGR,"");
                       else
                       if (l > LEN(ARG1)) {
                          ARGR = ARG1; ARG1 = NULL;
                          Lstrset(&A, (size_t)(l-LEN(ARGR)), pad);
                          Lstrcat(&ARGR,A);
                          m_free(A);
                       } else
                          Lstrsub(&ARGR,ARG1,0,(size_t)l);
                       break;

      case f_right:
                       if (!l) Lscpy(&ARGR,"");
                       else
                       if (l > LEN(ARG1)) {
                          Lstrset(&ARGR, (size_t)(l-LEN(ARG1)), pad);
                          Lstrcat(&ARGR,ARG1);
                       } else
                         Lstrsub(&ARGR,ARG1, (size_t)(LEN(ARG1)-l), (size_t)l);
                       break;

      default:   error(ERR_INTERPRETER_FAILURE);
   }  /* of switch */
} /* R_SIoC */

/* -------------------------------------------------------------- */
/*  ADDR(symbol)                                                  */
/*       returns the normalized (for msdos) address of symbol     */
/*       contents                                                 */
/* -------------------------------------------------------------- */
/*  C2X(string)                                                   */
/*       Character to Hexadecimal                                 */
/* -------------------------------------------------------------- */
/*  LENGTH(string)                                                */
/*       return the length of string                              */
/* -------------------------------------------------------------- */
/*  WORDS(string)                                                 */
/*       return the number of words in string                     */
/* -------------------------------------------------------------- */
/*  REVERSE(string)                                               */
/*       swap STRING, end-to-end                                  */
/* -------------------------------------------------------------- */
/*  SYMBOL(name)                                                  */
/*      return "BAD" if name is not a valid REXX variable         */
/*      name,  "VAR" if name has been used as a variable, or      */
/*      "LIT" if it has not.                                      */
/* -------------------------------------------------------------- */
/*  VALUE(name)                                                   */
/*      returns the value of the variable name.                   */
/* -------------------------------------------------------------- */
/*  X2C(string)                                                   */
/*      Hexadecimal to Character                                  */
/* -------------------------------------------------------------- */
void R_S ( args *arg, int func )
{
   Idrec *Aid;
   int   isarray;
   long  addr;
   size_t i,r=0;
   char   *re,*ar;
   extern stem;

   if (ARGN!=1) error(ERR_INCORRECT_CALL);
   L2str(&ARG1);
   switch (func) {
      case f_addr:
                  addr = -1;
                  if (is_symbol(ARG1)) {
                    isarray = (memchr(STR(ARG1),'.',LEN(ARG1))!=NULL);
                    Aid = V_search(isarray,ARG1,scope);
                    if (Aid!=NULL) {
#ifdef __MSDOS__
                      addr = (((long)FP_SEG(STR(Aid->var)))<<4) +
                             (long)FP_OFF(STR(Aid->var));
#else
                      addr = (long)(&STR(Aid->var));
#endif
                    }
                  }
                  Licpy(&ARGR,addr);
                  break;

      case f_c2x:
                  Lfx(&ARGR,2*LEN(ARG1));
                  re = STR(ARGR); ar = STR(ARG1);
                  for (i=0; i<LEN(ARG1); i++) {
                     re[r++] = chex[(ar[i] >> 4) & 0x0F];
                     re[r++] = chex[ar[i] & 0x0F];
                  }
                  TYPE(ARGR) = STR_TY;    LEN(ARGR) = r;
                  break;

      case f_getenv:
                  ASCIIZ(ARG1);
                  Lscpy( &ARGR, getenv(STR(ARG1)));
                  break;

      case f_length:
                  Licpy(&ARGR, LEN(ARG1));
                  break;

      case f_words:
                  i = 0;
                  for (;;) {
                     i = skipblanks(ARG1,i);
                     if (i>=LEN(ARG1)) {
                        Licpy(&ARGR,r);
                        return;
                     }
                     r++;
                     i = skipword(ARG1,i);
                  }

      case f_putenv:
                  ASCIIZ(ARG1);
#ifndef __MPW__
                  putenv(STR(ARG1));
#endif
                  ARGR = ARG1; ARG1 = NULL;
                  break;

      case f_reverse:
                  Lreverse(ARG1);
                  ARGR = ARG1; ARG1 = NULL;
                  break;

      case f_soundex:
		  L2upper(ARG1); ASCIIZ(ARG1);
		  Lscpy(&ARGR, Soundex(STR(ARG1),LEN(ARG1)));
		  break;

      case f_symbol:
		  if (!is_symbol(ARG1)) {
		     Lscpy(&ARGR,"BAD");
		     return;
		  }
          isarray = (memchr(STR(ARG1),'.',LEN(ARG1))!=NULL);
		  Aid = V_search(isarray,ARG1,scope);
		  if (Aid!=NULL)  Lscpy(&ARGR,"VAR");
		  else            Lscpy(&ARGR,"LIT");
		  break;

      case f_value:
		  if (!is_symbol(ARG1)) {
                     Lstrcpy(&ARGR,ARG1);
                     return;
                  }
                  isarray = (memchr(STR(ARG1),'.',LEN(ARG1))!=NULL);
                  Aid = V_search(isarray,ARG1,scope);
                  if (Aid!=NULL)  Lstrcpy(&ARGR, Aid->var);
                  else            Lstrcpy(&ARGR, ARG1);
                  break;

      case f_x2c:
                  Lx2c(&ARGR,ARG1);
                  if (ARGR==NULL) error(ERR_INVALID_HEX_CONST);
                  break;

      default:    error(ERR_INTERPRETER_FAILURE);
   }
} /* R_S */
/* -------------------------------------------------------------- */
/*  DELSTR(string,n[,length])                                     */
/*       delete substring of STRING starting at the nth           */
/*       character and of length LENGTH                           */
/* -------------------------------------------------------------- */
/*  DELWORD(string,n[,length])                                    */
/*       delete substring of STRING starting at the nth           */
/*       word and of length LENGTH words.                         */
/* -------------------------------------------------------------- */
/*  SUBWORD(string,n[,length])                                    */
/*       return the substring of string that begins at the nth    */
/*       word and is of length LENGTH words.                      */
/* -------------------------------------------------------------- */
void R_SIoI ( args *arg, int func )
{
   long n,l,i;

   if (!IN_RANGE(2,ARGN,3)) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   get_i(2,n);
   get_oi0(3,l);

   switch (func) {
      case f_delstr:
                n--;
                if (n>=LEN(ARG1)) {
                   ARGR = ARG1; ARG1=NULL;
                   return;
                }

                if (n) Lstrsub(&ARGR,ARG1,0,(size_t)n);
                else   Lscpy(&ARGR,"");
                if (l)
                   if (l+n < LEN(ARG1)) {
                      Lstrsub(&ARG2, ARG1, (size_t)(n+l), (size_t)(LEN(ARG1)-n-l));
                      Lstrcat(&ARGR, ARG2);
                   }
                break;

      case f_delword:
                i = Lwordindex(ARG1,(size_t)n);
                if (i==-1) {
                   ARGR = ARG1; ARG1 = NULL;
                   return;
                }
                if (!i)  Lscpy(&ARGR,"");
                else     Lstrsub(&ARGR,ARG1,0,(size_t)i);

                if (!exist(3)) return;
                while (l) {
                   l--;
                   i = skipword(ARG1,(size_t)i);
                   if (i>=LEN(ARG1)) return;
                   i = skipblanks(ARG1,(size_t)i);
                   if (i>=LEN(ARG1)) return;
                };
                Lstrsub(&ARG2, ARG1, (size_t)i, (size_t)(LEN(ARG1)-i));
                Lstrcat(&ARGR, ARG2);
                break;

      case f_subword:
               i = Lwordindex(ARG1,(size_t)n);
               if (i==-1) {
                  Lscpy(&ARGR,""); return;
               }
               if (!l) {
                  Lstrsub(&ARGR,ARG1,(size_t)i, (size_t)(LEN(ARG1)-i));
                  Lstrip(&ARGR);
                  return;
               }
               n = i;
               for (;;) {
                  l--;
                  i = skipword(ARG1,(size_t)i);
                  if ((i>=LEN(ARG1)) || !l) goto LEAVE;
                  i = skipblanks(ARG1,(size_t)i);
                  if (i>=LEN(ARG1)) goto LEAVE;
                }
        LEAVE:  Lstrsub(&ARGR,ARG1,(size_t)n,(size_t)(i-n));
                Lstrip(&ARGR);
                break;

      default:  error(ERR_INTERPRETER_FAILURE);
   } /* switch */
} /* R_SIoI */
/* -------------------------------------------------------------- */
/* INSERT(new,target[,[n][,[length][,pad]]])                      */
/*       insert the strin NEW of length LENGTH into the string    */
/*       TARGET, after the nth character (n can be 0)             */
/* -------------------------------------------------------------- */
/* OVERLAY(new,target[,[n][,[length][,pad]]])                     */
/*       overlay the string NEW of length LENGTH onto string      */
/*       TARGET, beginning at the nth character.                  */
/* -------------------------------------------------------------- */
void R_SSoIoIoC ( args *arg, int func )
{
   long n,l;
   char pad;

   if (!IN_RANGE(2,ARGN,5)) error(ERR_INCORRECT_CALL);
   must_exist(1); L2str(&ARG1);
   must_exist(2); L2str(&ARG2);
   get_oi0(3,n);
   get_oi0(4,l);
   get_pad(5);

   switch (func) {
      case f_insert:
                      if (n==0) Lscpy(&ARGR,"");
                      else {
                          Lstrset(&ARGR,(size_t)(n+l),pad);
                          if (n > LEN(ARG2))  Lstrcpy(&ARGR,ARG2);
                          else                Lstrsub(&ARGR,ARG2,0,(size_t)n);
                      }
                      LEN(ARGR) = (size_t)(n+l);
                      if (!exist(4)) Lstrcat(&ARGR,ARG1);
                      else
                      if (l > LEN(ARG1))
                         memcpy(STR(ARGR)+(size_t)n, STR(ARG1), LEN(ARG1));
                      else
                          memcpy(STR(ARGR)+(size_t)n, STR(ARG1), (size_t)l);

                      if (n==0) Lstrcat(&ARGR, ARG2);
                      else
                      if (n < LEN(ARG2)) {
                         Lstrsub(&ARG10, ARG2, (size_t)n, (size_t)(LEN(ARG2)-n));
                         Lstrcat(&ARGR, ARG10);
                      }
                      break;

      case f_overlay:
                      if (n==0) {
                         if (exist(3)) error(ERR_INCORRECT_CALL);
                         n = 1;
                      }
                      if (l==0) l = LEN(ARG1);

                      Lstrset(&ARGR,(size_t)(n+l-1),pad);
                      Lstrcpy(&ARGR,ARG2);
                      if ((size_t)(n+l-1) > LEN(ARG2)) LEN(ARGR) = (size_t)(n+l-1);

                      Lstrset(&ARG10,(size_t)l,pad);
                      Lstrcpy(&ARG10,ARG1);

                      memcpy( STR(ARGR)+(size_t)(n-1), STR(ARG10), (size_t)l);
                      break;

      default:    error(ERR_INTERPRETER_FAILURE);
   } /* switch */
} /* R_SSoIoIoC */
/* -------------------------------------------------------------- */
/* COMPARE(string1,string2[,pad])                                 */
/*       compare strings, and return index of first nonmatching   */
/*       character.                                               */
/* -------------------------------------------------------------- */
size_t  lcompare( Lstr *A, Lstr *B, char pad);
void R_compare( args *arg )
{
   char pad;

   if (!IN_RANGE(2,ARGN,3)) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   must_exist(2);  L2str(&ARG2);
   get_pad(3);

  if (LEN(ARG1) <= LEN(ARG2)) Licpy(&ARGR, lcompare(ARG1,ARG2,pad));
  else                        Licpy(&ARGR, lcompare(ARG2,ARG1,pad));
}  /* R_compare */

size_t  lcompare( Lstr *A, Lstr *B, char pad)
{
   size_t c;

   for (c=0; c<LEN(A); c++)
      if (STR(A)[c] != STR(B)[c]) return (c+1);
   for (; c<LEN(B); c++)
      if (pad != STR(B)[c])  return (c+1);
   return 0;
} /* lcompare */
/* -------------------------------------------------------------- */
/*  COPIES(string,n)                                              */
/*       returns n concatenated copies of string.                 */
/* -------------------------------------------------------------- */
void R_copies(args *arg)
{
   long  n;

   if (ARGN != 2) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   must_exist(2); L2int(&ARG2); n = INT(ARG2);
   if (n<0) error(ERR_INCORRECT_CALL);

   Lscpy(&ARGR,"");
   while (n != 0) {
     if (n&1) Lstrcat(&ARGR, ARG1);
     Lstrcpy(&ARG2,ARG1);
     Lstrcat(&ARG1,ARG2);
     n >>= 1;
   }
} /* R_copies */
/* -------------------------------------------------------------- */
/*  SUBSTR(string,n[,[length][,pad]])                             */
/*       return the substring of string that begins at the nth    */
/*       character and is of length LENGTH                        */
/* -------------------------------------------------------------- */
void R_substr( args *arg )
{
   long  n,l;
   char  pad;
   Lstr  *A=NULL;

   if (!IN_RANGE(2,ARGN,4)) error(ERR_INCORRECT_CALL);

   must_exist(1);  L2str(&ARG1);
   get_i(2,n);
   get_oi(3,l);
   get_pad(4);

   if (l<=0) {
      if (n <= LEN(ARG1))
         Lstrsub(&ARGR, ARG1, (size_t)(n-1), (size_t)(LEN(ARG1)-n+1));
      else
         Lscpy(&ARGR,"");
      return;
   }

   if (n+l-1 > LEN(ARG1)) {
      if (n > LEN(ARG1)) Lscpy(&ARGR,"");
      else Lstrsub(&ARGR,ARG1,(size_t)(n-1), (size_t)(LEN(ARG1)-n+1));
      Lstrset(&A, (size_t)(l-LEN(ARGR)), pad);
      Lstrcat(&ARGR,A);
      m_free(A);
      return;
   } else
      Lstrsub(&ARGR, ARG1, (size_t)(n-1),(size_t)l);
} /* R_substr */
/* -------------------------------------------------------------- */
/*  STRIP(string[,[<"L"|"T"|"B">][,char]])                        */
/*       return string less Leading, Trailing, or Both sets of    */
/*       blanks or other CHARs. Default is "B".                   */
/* -------------------------------------------------------------- */
void R_strip( args *arg )
{
   char    action='B';
   char    pad;
   char   *cf,*cl;
   size_t  l;

   if (!IN_RANGE(1,ARGN,3)) error(ERR_INCORRECT_CALL);

   must_exist(1);  L2str(&ARG1);
   if (exist(2)) { L2str(&ARG2); action = l2u[STR(ARG2)[0]]; }
   get_pad(3);

   l = LEN(ARG1);
   cf = STR(ARG1);
   if (action=='B' || action=='L') {
     while (l && (*cf==pad)) {cf++;l--;} /* Search for first non blank char */
     if (!l) {     /* Only blanks */
        Lscpy(&ARGR,"");
        return;
     }
   }
   l = LEN(ARG1);
   cl = STR(ARG1) + LEN(ARG1) - 1;
   if (action=='B' || action=='T')
      while (l && (*cl==pad)) {cl--;l--;}

   l = (size_t)((char HUGE *)cl - (char HUGE *)cf + 1);

   ARGR = ARG1; ARG1 = NULL;
   if (l<LEN(ARGR)) {
      memcpy(STR(ARGR),cf,l);
      LEN(ARGR) = l;
   }
} /* R_strip */
/**
*
*   UPPER(string)
*      Return upper case version of string.
*
**/
void R_upper(args *arg)
{
   char     table[256];
   char     pad;
   int      i;

   if (!IN_RANGE(1,ARGN,1)) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
 
   ARGR = ARG1; ARG1 = NULL;

   L2upper(ARGR);
   return;
} /* R_lower */
/* -------------------------------------------------------------- */
/* TRANSLATE(string(,(tableo)(,(tablei)(,pad))))                  */
/*       translate characters in TABLEI to associated characters  */
/*       in TABLEO. If neither table is specified, convert to     */
/*       uppercase.                                               */
/* -------------------------------------------------------------- */
void R_translate( args *arg )
{
   char     table[256];
   char     pad;
   int      i;

   if (!IN_RANGE(1,ARGN,4)) error(ERR_INCORRECT_CALL);
   must_exist(1);  L2str(&ARG1);
   if (exist(2))   L2str(&ARG2);
   if (exist(3))   L2str(&ARG3);
   get_pad(4);

   ARGR = ARG1; ARG1 = NULL;

   if (ARGN==1) {
      L2upper(ARGR);
      return;
   }

   for (i=0; i<256; i++) table[i] = i;
   if (exist(3)) {
     for (i=LEN(ARG3)-1;  i>=0; i--)
        if (exist(2)) {
          if (i >= LEN(ARG2)) table[ (unsigned char)STR(ARG3)[i] ] = pad;
          else                table[ (unsigned char)STR(ARG3)[i] ] = STR(ARG2)[i];
        } else    table[ (unsigned char)STR(ARG3)[i] ] = i;
   } else {
     for (i=0;  i<256; i++)
        if (exist(2))
          if (i >= LEN(ARG2)) table[ i ] = pad;
          else                table[ i ] = STR(ARG2)[i];
   }

   for (i=0; i< LEN(ARGR); i++)
      STR(ARGR)[i] = table[ (unsigned char) STR(ARGR)[i] ];
} /* R_translate */
/* -------------------------------------------------------------- */
/* VERIFY(string,reference[,[option][,start]])                    */
/*       return the index of the first character in string that   */
/*       is not also in REF. if "Match" is given, then return     */
/*       the result index of the first character in STRING that   */
/*       is in REF.                                               */
/* -------------------------------------------------------------- */
void R_verify( args *arg )
{
   char    match='N';
   long    start;

   if (!IN_RANGE(2,ARGN,4)) error(ERR_INCORRECT_CALL);

   must_exist(1);  L2str(&ARG1);
   must_exist(2);  L2str(&ARG2);
   if (exist(3)) { L2str(&ARG3); match = l2u[STR(ARG3)[0]]; }
   get_oi(4,start);

   match = (match=='M');
   if (start) start--;

   Licpy(&ARGR, 1+Lverify(ARG1,ARG2,match,(size_t)start));
} /* R_verify */
