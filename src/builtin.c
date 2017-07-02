/*
 * $Header: c:/usr/rexx/builtin.c!v 1.4 1997/09/13 15:19:28 bill Exp bill $
 * $Log: builtin.c!v $
 * Revision 1.4  1997/09/13  15:19:28  bill
 * Added: Makebuf routine
 *
 * Revision 1.3  1997/07/04  20:32:20  bill
 * *** empty log message ***
 *
 * Revision 1.2  1996/12/13 01:03:49  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11  03:33:26  bill
 * Initial revision
 *
 */


#include <time.h>
#include <stdlib.h>

#ifdef __MSDOS__
#include <dos.h>
#include <alloc.h>
#else
#	ifndef GCC
  struct timeval_st {
    unsigned long tv_sec;
    long tv_usec;
  };
#	endif
#endif

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"
#include "instruct.h"

#include "rxdefs.h"

int  is_symbol( Lstr  *S );

long      elapsed;   /* initialized in main routine */

char *WeekDays[] = { "Sunday", "Monday", "Tuesday", "Wednesday",
   "Thursday", "Friday", "Saturday" } ;

char *months[] = {
   "January", "February", "March", "April", "May", "June",
   "July", "August", "September", "October", "November", "December" } ;

/* -------------------------------------------------------------- */
/*   ADDRESS()                                                    */
/*       return the current enviroment for commands.              */
/* -------------------------------------------------------------- */
/*  DESBUF()                                                      */
/*       destroys the current buffer, and returns the number of   */
/*       queued lines.                                            */
/* -------------------------------------------------------------- */
/*  DIGITS()                                                      */
/*       returns the current setting of NUMERIC DIGITS.           */
/* -------------------------------------------------------------- */
/*  FORM()                                                        */
/*       returns the current setting of NUMERIC FORM.             */
/* -------------------------------------------------------------- */
/*  FUZZ()                                                        */
/*       returns the current setting of NUMERIC FUZZ.             */
/* -------------------------------------------------------------- */
/*   QUEUED()                                                     */
/*       return the number of lines in the program stack (system  */
/*       data queue)                                              */
/* -------------------------------------------------------------- */
void R_O ( args *arg, int func )
{
   int  q;
   Lstrlist *l;
   Lstr *A;           /* POPED FROM STACK */
   stacklist *s;

   if (ARGN) error(ERR_INCORRECT_CALL);
   switch (func) {
     case f_address:  Lstrcpy(&ARGR,environment);
                      break;
     case f_desbuf:   q = 0;
                      while (stack->head != NULL) {
                        A = pop_Lstr(&(stack->head),&(stack->tail));
                        q++; m_free(A);
                      }
                      if (stack->prev) {
                         s = stack;
                         stack = stack->prev;
                         m_free(s);
                      }
                      Licpy(&ARGR,q);
                      break;
     case f_digits:   Licpy(&ARGR,digits);
                      break;
     case f_form:
             if (form==SCIENTIFIC) Lscpy(&ARGR,"SCIENTIFIC");
             else                  Lscpy(&ARGR,"ENGINEERING");
             break;
     case f_fuzz:     Licpy(&ARGR,fuzz);
                      break;
     case f_queued:   q = 0; l = stack->head;
                      while (l!=NULL) {
                         l = l->next;
                         q++;
                      }
                      Licpy(&ARGR,q);
                      break;
     case f_makebuf:  
                      s = (stacklist *)m_malloc(sizeof(stacklist),"STCK");
                      s->prev = stack;
                      s->head = s->tail = NULL;
                      stack = s;
                      /* count stacks */
                      for (q=0; s; q++) s = s->prev;
                      Licpy(&ARGR,q);
                      break;

     default:         error( ERR_INTERPRETER_FAILURE );
   }
} /* R_O */
/* -------------------------------------------------------------- */
/*  DATE((option))                                                */
/* -------------------------------------------------------------- */
/*  TIME((option))                                                */
/* -------------------------------------------------------------- */
/*  TRACE((option))                                               */
/* -------------------------------------------------------------- */
void R_C ( args *arg, int func )
{
   char   option='N';
   int    i;

   static char *fmt = "%02d/%02d/%02d" ;
   static char *iso = "%4d%02d%02d" ;
   long   length;
   char   *chptr;

   time_t now ;
   long   unow;
   char   *ampm;
   struct tm *tmdata ;
   int    hour;
#ifdef __MSDOS__
   struct time t;
#elif defined(GCC)
   struct timeval tp;
   struct timezone tzp;
#elif defined(__MPW__)
#else
   struct timeval_st tp,tzp;
#endif

   if (ARGN>1) error(ERR_INCORRECT_CALL);
   if (exist(1)) { L2str(&ARG1); option = l2u[STR(ARG1)[0]]; }

   Lfx(&ARGR,30); TYPE(ARGR) = STR_TY;
   switch (func) {

     case f_date:
                 now = time(NULL);
                 tmdata = localtime(&now) ;
                 switch (option) {
                   case 'C':
                     length = tmdata->tm_yday + 1 +
                            (long)(((float)tmdata->tm_year-1)*365.25) + 365 ;
                     sprintf(STR(ARGR),"%ld",length) ;
                     break ;

                   case 'D':
                     sprintf(STR(ARGR), "%d", tmdata->tm_yday+1) ;
                     break ;

                   case 'E':
                     sprintf(STR(ARGR), fmt, tmdata->tm_mday, tmdata->tm_mon+1,
                              tmdata->tm_year) ;
                     break ;

                   case 'M':
                     sprintf(STR(ARGR),"%s",months[tmdata->tm_mon]) ;
                     break ;

                   case 'N':
                     chptr = months[tmdata->tm_mon] ;
                     sprintf(STR(ARGR),"%d %c%c%c %4d",
                        tmdata->tm_mday, chptr[0], chptr[1],
                           chptr[2], tmdata->tm_year+1900) ;
                     break ;

                   case 'O':
                     sprintf(STR(ARGR), fmt, tmdata->tm_year,
                        tmdata->tm_mon+1, tmdata->tm_mday);
                     break ;

                   case 'S':
                     sprintf(STR(ARGR), iso, tmdata->tm_year+1900,
                         tmdata->tm_mon+1, tmdata->tm_mday) ;
                     break ;

                   case 'U':
                     sprintf(STR(ARGR), fmt, tmdata->tm_mon+1,
                        tmdata->tm_mday, tmdata->tm_year ) ;
                     break ;

                   case 'W':
                     chptr = WeekDays[tmdata->tm_wday] ;
                     sprintf(STR(ARGR),"%s",chptr);
                     break ;

                   default: error(ERR_INCORRECT_CALL) ;
                 }
                 break;
/* -------------------* f_time *----------------------------- */
     case f_time:
                 now = time(NULL);
                 tmdata = localtime(&now) ;
                 switch (option) {
                   case 'C':
                      hour = tmdata->tm_hour ;
                      ampm = (hour>11) ? "pm" : "am" ;
                      if ((hour=hour%12)==0)  hour = 12 ;
                      sprintf(STR(ARGR),
                              "%d:%02d%s", hour, tmdata->tm_min, ampm) ;
                      break ;
                   case 'E':
#ifdef __MSDOS__
                      gettime(&t);
                      unow = (long)t.ti_hour*360000L + (long)t.ti_min*6000L +
                              (long)t.ti_sec*100L + (long)t.ti_hund;
                      sprintf(STR(ARGR),"%1.2f", (double)(unow-elapsed)/100.0);
#elif defined(hpux) || defined(__hpux)
                      gettimeofday(&tp,&tzp);
                      unow = tp.tv_sec * 100 + tp.tv_usec / 10000;
                      sprintf(STR(ARGR),"%1.2f", (double)(unow-elapsed)/100.0);
#else
                      unow = (long)tmdata->tm_hour * 3600 +
                             (long)tmdata->tm_min  * 60 +
                             (long)tmdata->tm_sec;
                      sprintf(STR(ARGR),"%ld.00", unow-elapsed);
#endif
                      break ;

                   case 'H':
                      sprintf(STR(ARGR), "%d", tmdata->tm_hour) ;
                      break ;

                   case 'L':
#ifdef __MSDOS__
                      gettime(&t);
                      sprintf(STR(ARGR), "%02d:%02d:%02d.%02d",
                          t.ti_hour, t.ti_min, t.ti_sec, t.ti_hund);
#elif defined(hpux) || defined(__hpux)
                      gettimeofday(&tp,&tzp);
                      unow = tp.tv_sec * 100 + tp.tv_usec / 10000;
                      sprintf(STR(ARGR), "%02d:%02d:%02d.%06d",
                          tmdata->tm_hour, tmdata->tm_min,
                          tmdata->tm_sec, tp.tv_usec );
#else
                      sprintf(STR(ARGR), "%02d:%02d:%02d",
                          tmdata->tm_hour, tmdata->tm_min,
                          tmdata->tm_sec) ;
#endif
                      break ;

                   case 'M':
                      sprintf(STR(ARGR), "%d",
                          tmdata->tm_hour*60 + tmdata->tm_min) ;
                      break ;

                   case 'N':
                      sprintf(STR(ARGR), "%02d:%02d:%02d", tmdata->tm_hour,
                      tmdata->tm_min, tmdata->tm_sec ) ;
                      break ;

                   case 'R':
#ifdef __MSDOS__
                      gettime(&t);
                      unow = (long)t.ti_hour*360000L + (long)t.ti_min*6000L +
                              (long)t.ti_sec*100L + (long)t.ti_hund;
                      sprintf(STR(ARGR),"%1.2f", (double)(unow-elapsed)/100.0);
                      elapsed = unow;
#elif defined(hpux) || defined(__hpux)
                      gettimeofday(&tp,&tzp);
                      unow = tp.tv_sec * 100 + tp.tv_usec / 10000;
                      sprintf(STR(ARGR),"%1.2f", (double)(unow-elapsed)/100.0);
                      elapsed = unow;
#else
                      unow = (long)tmdata->tm_hour * 3600 +
                             (long)tmdata->tm_min  * 60 +
                             (long)tmdata->tm_sec;
                      sprintf(STR(ARGR),"%ld.00", unow-elapsed);
                      elapsed = unow;
#endif
                      break ;

                   case 'S':
                      sprintf(STR(ARGR), "%ld",
                          (long)((long)(tmdata->tm_hour*60L)+tmdata->tm_min)
                          *60L + (long)tmdata->tm_sec) ;
                      break ;

                   default: error(ERR_INCORRECT_CALL);
                 }
                 break;

     case f_trace:
                 i = 0;
                 if (interactive_debug) STR(ARGR)[i++] = '?';
                 if (command_inhibition) STR(ARGR)[i++] = '!';
                 switch (tracing) {
                   case all_trace:       STR(ARGR)[i] = 'A'; break;
                   case commands_trace:  STR(ARGR)[i] = 'C'; break;
                   case error_trace:     STR(ARGR)[i] = 'E'; break;
                   case intermediates_trace: STR(ARGR)[i] = 'I'; break;
                   case labels_trace:    STR(ARGR)[i] = 'L'; break;
                   case normal_trace:    STR(ARGR)[i] = 'N'; break;
                   case off_trace:       STR(ARGR)[i] = 'O'; break;
                   case results_trace:   STR(ARGR)[i] = 'R'; break;
                   case scan_trace:      STR(ARGR)[i] = 'S'; break;
                 }
                 STR(ARGR)[i+1] = 0;

                 if (exist(1)) {
                   ASCIIZ(ARG1);
                   set_trace(STR(ARG1));
                 }
                 break;
     default:    error( ERR_INTERPRETER_FAILURE );
   }  /* switch */
   LEN(ARGR) = strlen(STR(ARGR));
}  /* R_C */
/* -------------------------------------------------------------- */
/*  ARG([n[,option]])                                             */
/*      return the number of arguments, the nth argument, or      */
/*      test if the nth argument Exists or is Omitted.            */
/* -------------------------------------------------------------- */
void R_arg ( args *arg )
{
   switch (ARGN) {
     case  0:   Licpy(&ARGR,oarg->n);
                break;

     case  1:   L2int(&ARG1);
                if (!IN_RANGE(1,INT(ARG1),maxargs)) error(ERR_INCORRECT_CALL);
                Lstrcpy(&ARGR,oarg->a[(int)INT(ARG1)-1]);
                break;

     case  2:   L2int(&ARG1);
                if (!IN_RANGE(1,INT(ARG1),maxargs)) error(ERR_INCORRECT_CALL);
                L2str(&ARG2);
                if (l2u[STR(ARG2)[0]] == 'E')
                   Licpy(&ARGR, (oarg->a[(int)INT(ARG1)-1] != NULL));
                else
                if (l2u[STR(ARG2)[0]] == 'O')
                   Licpy(&ARGR, (oarg->a[(int)INT(ARG1)-1] == NULL));
                else
                    error(ERR_INCORRECT_CALL);
                break;
     default:   error(ERR_INCORRECT_CALL);
   }
} /* R_arg */
/* -------------------------------------------------------------- */
/* DATATYPE(string(,type))                                        */
/*      return "NUM" if STRING is a valid number; otherwise,      */
/*      return "CHAR". TYPE may be Alphanumeric, Bits,            */
/*      Lowercase, Mixed case, Number, Symbol-characters,         */
/*      Uppercase, Whole-number or X (heXadecimal) to test        */
/*      for specific data types.                                  */
/* -------------------------------------------------------------- */
void R_datatype( args *arg )
{
   char type=' ';
   Lstr *ref=NULL;

   if (!IN_RANGE(1,ARGN,2)) error(ERR_INCORRECT_CALL);
   must_exist(1);
   if (exist(2)) {
     L2str(&ARG2); if (!LEN(ARG2)) error(ERR_INCORRECT_CALL);
     type = l2u[STR(ARG2)[0]];
   }  else {
      Lscpy( &ARGR, ((datatype(ARG1)=='C')? "CHAR": "NUM"));
      return;
   }

   if (type=='T') {   /* mine special type /////////// */
      switch (TYPE(ARG1)) {
        case INT_TY:  Lscpy(&ARGR,"INTEGER"); break;
        case REAL_TY: Lscpy(&ARGR,"REAL"); break;
        case STR_TY:  Lscpy(&ARGR,"STRING"); break;
        default:      error(ERR_INTERPRETER_FAILURE);
      }
      return;
   }
   if (type=='N') {
     Licpy(&ARGR,(datatype(ARG1)!='C'));
     return;
   }

   L2str(&ARG1);
   switch (type) {
      case 'A': Lscpy(&ref,clower);
                Lcat(&ref,cUPPER);
                Lcat(&ref,cdigits);
                break;
      case 'B': Lscpy(&ref,"01");
                break;
      case 'L': Lscpy(&ref,clower);
                break;
      case 'M': Lscpy(&ref,clower);
                Lcat(&ref,cUPPER);
                break;
      case 'S': Lscpy(&ref,clower);
                Lcat(&ref,cUPPER);
                Lcat(&ref,cdigits);
                Lcat(&ref,crxsymb);
                break;
      case 'U': Lscpy(&ref,cUPPER);
                break;
      case 'W': Lscpy(&ref,cdigits);
                break;
      case 'X':
                if (!LEN(ARG1)) {
                   Licpy(&ARGR,TRUE);
                   return;
                }
                Lscpy(&ref,chex);
                Lcat(&ref," ");
                break;
      default:  error(ERR_INCORRECT_CALL);
   }
   if (!LEN(ARG1))  Licpy(&ARGR,FALSE);
   else             Licpy(&ARGR,(Lverify(ARG1,ref,FALSE,0)+1)==0);
   m_free(ref);
} /* R_datatype */
/* -------------------------------------------------------------- */
/*  ERRORTEXT(n)                                                  */
/*       returns the error message for error number n.            */
/* -------------------------------------------------------------- */
extern char *errormsg[];
void R_errortext( args *arg )
{
   if (ARGN!=1) error(ERR_INCORRECT_CALL);
   L2int(&ARG1);
   if (IN_RANGE(0,INT(ARG1),49))
          Lscpy(&ARGR, errormsg[(int)INT(ARG1)]);
   else   Lscpy(&ARGR,"");
} /* R_errortext */
/* -------------------------------------------------------------- */
/*  INT( num, reg-string )                                        */
/*      executes a 80x86 soft-interrupt.                          */
/*      num = interrupt number, and reg-string is a string        */
/*      in the format "ax=hex-num bx=hex-num ...."                */
/*      returns in the same format the registers and flags        */
/* -------------------------------------------------------------- */
#ifdef __MSDOS__
char  *s_reg[] = {"AX=","BX=","CX=","DX=",
                  "BP=","SI=","DI=",
                  "DS=","ES=","FLAGS="};
char  flags_str[]="C-P-A-ZSTIDO";
void R_intr( args *arg )
{
   int      i,intno;
   union  {
        struct   REGPACK regpack;
        unsigned regarray[9];
   } reg;
   char  *s;

   if (ARGN != 2) error(ERR_INCORRECT_CALL);
   must_exist(1); L2int(&ARG1); intno = (int)INT(ARG1);
   if (!IN_RANGE(0,intno,0xFF)) error(ERR_INCORRECT_CALL);
   must_exist(2); L2str(&ARG2);

   L2upper(ARG2); Lspace(&ARG3,ARG2,1,' ');
   ASCIIZ(ARG3);

   for (i=0; i<10; i++) {
      s=strstr(STR(ARG3),s_reg[i]);
      if (s!=NULL) {
         s+=3; sscanf(s,"%X",&reg.regarray[i]);
      }
   }

   intr(intno,&reg.regpack);

   Lfx(&ARGR,100); TYPE(ARGR) = STR_TY; s=STR(ARGR); *s='\0';
   for (i=0; i<9; i++) {
      sprintf(s,"%s%04X ",s_reg[i],reg.regarray[i]);
      s += strlen(s);
   }
   strcat(s,s_reg[9]); s += strlen(s);
   for (i=0; i<12; i++, reg.regpack.r_flags >>= 1)
     if (reg.regpack.r_flags & 0x1) *s++ = flags_str[i];
   *s = '\0';

   LEN(ARGR) = strlen(STR(ARGR))-1;
} /* R_int */

/* -------------------------------------------------------------- */
/*  PORT(port(,value))                                            */
/*      if value is not specified then reads one byte from port   */
/*      and returns it in integer format (IN)                     */
/*      if value exists then OUTs that value to the port.         */
/* -------------------------------------------------------------- */
void R_port( args *arg )
{
   long port;
   int  value;

   if (!IN_RANGE(1,ARGN,2)) error(ERR_INCORRECT_CALL);
   get_i(1,port);
   if (exist(2)) {
      L2int(&ARG2);
      value = (int)INT(ARG2);
      outp((int)port,value);
   } else {
      value = inp((int)port);
      Licpy(&ARGR, value);
   }
} /* R_port */

#endif
/* -------------------------------------------------------------- */
/*  VARTREE((symbol),'Depth')                                     */
/*      returns the binary tree in the format                     */
/*      var = "value"  <cr>                                       */
/*      if depth is specified (only the fist char is significant) */
/*      returns the result in the format.                         */
/*      depth var = "value"  <cr>                                 */
/* -------------------------------------------------------------- */
void R_oS( args *arg )
{
   Idrec  *Aid;
   int    option = FALSE;

   if (ARGN>2) error(ERR_INCORRECT_CALL);

   if (ARGN==2) {
      L2str(&ARG2); option = (l2u[STR(ARG2)[0]]=='D');
   }

   if (ARG1==NULL)  getbintree(&ARGR,0,option,scope);
   else  {
      if (!is_symbol(ARG1)) {
         Lscpy(&ARGR,"");
         return;
      }
      Aid = C_search(ARG1,scope);
      if (Aid != NULL) {
         if (stem) getbintree(&ARGR,0,option,Aid->ara);
         else      getbintree(&ARGR,0,option,Aid);
      }
   }
} /* R_oS */
/* -------------------------------------------------------------- */
/*   MAX(number[,number]..])                                      */
/*       returns the largest of given numbers.                    */
/* -------------------------------------------------------------- */
void R_max( args *arg )
{
    int  i;
    double r;

    if (!ARGN) error(ERR_INCORRECT_CALL);
    i = 0; while ((i<ARGN) && (arg->a[i]==NULL)) i++;
    if (i==maxargs) error(ERR_INCORRECT_CALL);
    L2real(&(arg->a[i]));
    r = REAL(arg->a[i]);
    for (i++; i<ARGN; i++)
      if (arg->a[i] != NULL)  {
        L2real(&(arg->a[i]));
        r = MAX(r,REAL(arg->a[i]));
      }
    Lrcpy(&ARGR,r);
} /* R_max */
/* -------------------------------------------------------------- */
/*   MIN(number[,number]..])                                      */
/*       returns the smallest of given numbers.                   */
/* -------------------------------------------------------------- */
void R_min( args *arg )
{
    int  i;
    double r;

    if (!ARGN) error(ERR_INCORRECT_CALL);
    i = 0; while ((i<ARGN) && (arg->a[i]==NULL)) i++;
    if (i==ARGN) error(ERR_INCORRECT_CALL);
    L2real(&(arg->a[i]));
    r = REAL(arg->a[i]);
    for (i++; i<ARGN; i++)
      if (arg->a[i] != NULL)  {
        L2real(&(arg->a[i]));
        r = MIN(r,REAL(arg->a[i]));
      }
    Lrcpy(&ARGR,r);
} /* R_min */
/* -------------------------------------------------------------- */
/* RANDOM((min)(,(max)(,seed)))                                   */
/*      returns a pseudorandom nonnegative whole number in the    */
/*      range min to max inclusive.                               */
/* -------------------------------------------------------------- */
void R_random( args *arg )
{
   long   min, max;
   static long seed;
   static int  sewed=0 ;

   if (!IN_RANGE(0,ARGN,3)) error(ERR_INCORRECT_CALL);

   if (exist(1)) {
      L2int(&ARG1); min = INT(ARG1);
      if (min<0) error(ERR_INCORRECT_CALL);
   } else min = 0;

   if (exist(2)) {
      L2int(&ARG2); max = (int)INT(ARG2);
      if (max<0) error(ERR_INCORRECT_CALL);
   } else max = 999;

   if (min>max) error( ERR_INCORRECT_CALL ) ;

   if (exist(3)) {
      L2int(&ARG3); seed = INT(ARG3);
      if (seed<0) error(ERR_INCORRECT_CALL);
      srand((unsigned)seed);
      sewed = 1;
   } else
   if (sewed==0) {
      sewed = 1 ;
      seed=(time((time_t *)0)%(3600*24));
      srand((unsigned)seed);
   }

   Licpy(&ARGR, (long)rand() % (max-min+1) + min);
} /* R_random */
/* -------------------------------------------------------------- */
/*  STORAGE((address(,(length)(,data))))                          */
/*      returns the current virtual machine size expressed as a   */
/*      DECIMAL string if no arguments are specified.             */
/*      Otherwise, returns length bytes from the user's memory    */
/*      starting at address.  The length is in decimal;           */
/*      the default is 1 byte.  The address is a DECIMAL number.  */
/*      If data is specified, after the "old" value has been      */
/*      retrieved, storage starting at address is overwritten     */
/*      with data (the length argument has no effect on this).    */
/* -------------------------------------------------------------- */
void R_storage( args *arg )
{
   void *ptr;
#ifdef __MSDOS__
   unsigned seg,ofs;
#endif
   size_t length = 1;

   if (ARGN>3) error(ERR_INCORRECT_CALL);
   if (ARGN==0) {
#ifdef __MSDOS__
      Licpy(&ARGR,farcoreleft()); /*//// WRONG!!!!!!!!!*/
#else
      Licpy(&ARGR,0);             /*////////////////// */
#endif
      return;
   }
   if (exist(1)) {      /*// Argument is decimal and not hex */
      L2int(&ARG1);
      if (INT(ARG1) < 0) error(ERR_INCORRECT_CALL);
#ifdef __MSDOS__
      seg = (unsigned)((INT(ARG1) >> 4) & 0xFFFF);
      ofs = (unsigned)(INT(ARG1) & 0x000F);
      ptr = MK_FP(seg,ofs);
#else
      ptr = (void *)INT(ARG1);
#endif
   } else error(ERR_INCORRECT_CALL);

   if (exist(2)) {
     L2int(&ARG2);
     if (INT(ARG2) <= 0) error(ERR_INCORRECT_CALL);
     length = (size_t)INT(ARG2);
   }

   Lfx(&ARGR,length);  TYPE(ARGR) = STR_TY; LEN(ARGR) = length;
   memcpy(STR(ARGR),ptr,length);

   if (exist(3)) {
      L2str(&ARG3);     /*// !!!! Always is converted to string !!!! */
      memcpy(ptr, STR(ARG3), LEN(ARG3));
   }
} /* R_storage */
/* -------------------------------------------------------------- */
/*  SOURCELINE([n])                                               */
/*      return the number of lines in the program, or the nth     */
/*      line.                                                     */
/* -------------------------------------------------------------- */
void R_sourceline( args *arg )
{
   long  l;
   char *c,*co;

   if (ARGN>1) error(ERR_INCORRECT_CALL);
   get_oi(1,l);

   if (!l) {   /* then count lines */
      l++;  c = orig_exec;
      while (*c) if (*c++=='\n') l++;
      Licpy(&ARGR,l-2);   /* -2, because of 2 \n in rexx.c while loading file */
   } else {     /* return line */
      c = orig_exec;
      l--;
      while (*c && l) if (*c++=='\n') l--;
      if (!*c) Lscpy(&ARGR,"");
      else {
         co = c;
         while (*c && *c!='\n') { c++; l++; }
         Lfx(&ARGR,(size_t)l);
         memcpy(STR(ARGR),co,(size_t)l);
         LEN(ARGR) = (size_t)l;
      }
   }
} /* R_sourceline */

#ifdef DEBUGING
/* ---------------------* print memory *--------------------- */
void R_printmem( args *arg )
{
    if (!ARGN) printmemlist();
} /* R_printtree */
#endif
