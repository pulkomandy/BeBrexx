/* ============================================================== */
/*   Bill N. Vlachoudis                                    o o    */
/*   Computer adDREss.......bnv@nisyros.physics.auth.gr  ____oo_  */
/*   HomE AdDreSs...........Eirinis 6                   /||    |\ */
/* !                        TK 555-35 Pylaia             ||    |  */
/*  The bEsT is   !         ThesSALONIKI, GREECE         `.___.'  */
/* .. yEt tO   cOme   *                                  MARMITA  */
/* ============================================================== */
/* -------------------------------------------------------------- */
/*             RRRRRR    EEEEEE   XX  XX   XX  XX                 */
/*             RR   RR   EE        XXXX     XXXX                  */
/*             RRRRRR    EEEE       XX       XX                   */
/*             RR RR     EE        XXXX     XXXX                  */
/*             RR  RR    EEEEEE   XX  XX   XX  XX                 */
/*          Restructured    EXtended      eXecutor                */
/* -------------------------------------------------------------- */
/*                                                                */
/*     The Restructured Extended Executor  (REXX) language is a   */
/*  command  programming language  that allows  you to  combine   */
/*  useful sequences  of commands  to create new  commands. The   */
/*  REXX  is especially  suitable for  writing EXECs  or editor   */
/*  macros,  but  is also  a  very  useful tool  for  algorithm   */
/*  development.                                                  */
/*                                                                */
/*                                                                */
/*  Syntax Notation                                               */
/*  ~~~~~~~~~~~~~~~                                               */
/*                                                                */
/*  UPPER CASE        indicates language keywords                 */
/*                                                                */
/*  lower case        indicate defined syntactic units            */
/*                                                                */
/*  [ ]               brackets indicate an optional item          */
/*                                                                */
/*  ...               ellipses mean multiple items are allowed    */
/*                                                                */
/*  < >               angle brackets specify list of alternatives */
/*                    (choose one)                                */
/*                                                                */
/*   |                seperates alternatives in a list            */
/*                                                                */
/* -------------------------------------------------------------- */
/* 
 * $Header: c:/usr/rexx/rexx.c!v 1.6 1997/09/13 15:17:38 bill Exp bill $
 * $Log: rexx.c!v $
 * Revision 1.6  1997/09/13  15:17:38  bill
 * Changed: Multiple stacks support
 *
 * Revision 1.5  1997/07/04  20:32:44  bill
 * *** empty log message ***
 *
 * Revision 1.4  1997/04/15 13:47:13  bill
 * *** empty log message ***
 *
 * Revision 1.3  1996/12/13  01:03:55  bill
 * Release 1.3
 *
 * Revision 1.2  1995/12/13  01:10:38  bill
 * IP address changed
 *
 * Revision 1.1  1995/09/11  03:33:37  bill
 * Initial revision
 *
 */

#define  __REXX_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>


#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "instruct.h"

#ifdef __HAIKU__
#include "be_main.h"
#endif

#ifdef __MSDOS__
#  include <dos.h>
struct time t;
#  ifdef PARADOX
     int paradoxinit=FALSE;
     void paradox_done( void );
#  endif
#else
# include <time.h>
 time_t now;
 struct tm *tm_data;
#	ifndef GCC
 struct timeval_st {
      unsigned long tv_sec;
      long tv_usec;
 } tp, tzp;
#	else
	struct timeval tp;
	struct timezone tzp;
#	endif
#endif

void init_files(void);
void close_files(void);

int   LoadFile( char *, char **);
void  SETUP(void);
void  release_everything( void );

extern long elapsed;  /* from builtin.c */

/* ----------------------- LoadFile ---------------------- */
/* Loads a file into a dynamically created array 'text'    */
/* Errors:                                                 */
/*         0  -  Ok.                                       */
/*         1  -  Error opening while file.                 */
/*         2  -  No read permission for file.              */
/*         3  -  Not enough memory to load file.           */
/* ------------------------------------------------------- */
int LoadFile( char *filename, char **array )
{
    FILE *fin;
    char  *c;
    long   size;
    char buffer[300];

    if (*filename=='\0') fin = stdin;
    else
    if ((fin=fopen(filename,"r")) == NULL) {
       strncpy(buffer, filename, 256);
       strcat(buffer, ".r");
       if ((fin=fopen(buffer, "r")) == NULL) {
          strcat(buffer, "exx");
          if ((fin=fopen(buffer, "r")) == NULL) {
             return 1;
          }
       }
    }
/*    stat(filename,&info);
    if (!(info.st_mode & S_IREAD))  return 2; */
    fseek(fin,0,SEEK_END);
    size = ftell(fin);
    if (!size) size = 2000; /* enough size to hold a simple prog */
                         /* entered from cmdline */
    fseek(fin,0,SEEK_SET);

    if ((*array=m_malloc((int)(size+3),"FILE")) == NULL)  {
       fclose(fin);
       return 3;
    }

    c = *array;
    while (!feof(fin)) *c++ = fgetc(fin);
    *c = *(c-1) = '\n';   /* !!!! if you change this line,
                           Change also the SOURCELINE function !!!*/
    *(c+1) = '\0';

    fclose(fin);
    return 0;
}  /* LoadFile */

/* -------------------* SETUP *--------------- */
void SETUP( void )
{
    stack = (stacklist *)m_malloc(sizeof(stacklist),"STCK");
    stack->head = stack->tail = NULL;
    stack->prev = NULL;
/*    stack_head = stack_tail = NULL; */
    scope = NULL;
    lastlabelptr = NULL;
    label_scope = NULL;

    exectext = NULL;
    symbol = semicolon_sy;
    symbolstr = NULL;
    Lfx(&symbolstr,250);
    flag = f_running;
    rc = 0;
    Pblank = Nblank = FALSE;
    depth = 1;
    TPrevLineno = -1;
    statement = nothing_st;
    file_type = COMMAND;

    command_inhibition = FALSE;
    n_trace = 0;

    environment = NULL;
    Lscpy(&environment,"SYSTEM");

    Error   = OFF;      /*///////// This signal values are not working!!!! */
    Halt    = OFF;
    Novalue = OFF;
    Syntax  = OFF;

    digits  = 9;
    form    = SCIENTIFIC;
    fuzz    = 0;

/* This are from INSTRUCT.C ///////////////////  */
    IN_DO = FALSE;
    IN_FUNCTION = FALSE;
    DO_BPTR=NULL;
    ControlVar=NULL;

    IN_SIGNAL = FALSE;
    INTERPRET_LEVEL = 0;
    EXIT_FROMPRG = 0;

} /* SETUP */

void release_everything( void )
{
    m_free( orig_exec );
    m_free( symbolstr );
    m_free( environment );
} /* release_everything */

/* -------------------------------------------------------------- */
#ifdef __HAIKU__
int BRexxMain(int argc, char **argv)
#else
int main(     int argc, char **argv)
#endif
{
   void *markp;
   int   i,l;

   if (argc == 1) {
      bputs("\nsyntax: rx [-trace] <filename> <args>...");
      bputs("        rx -s   (to use stdin)\n");
      bputs(rx_version);
      bprintf("Author: %s\n", author);
      bputs("Please report any bugs, fatal errors and comments to Willy Langeveld");
      return(24);
   }

   init_ul();       /* initialize u2l and l2u arrays */
   init_files();

   rc = setjmp(finito);
   if (rc!=0)  {
       close_files();
       switch (rc) {
         case 20000+ERR_INTERPRETER_FAILURE:
            return(rc-20000);
         default:
            goto end_of_rexx;
       }
   }

   tracing = normal_trace;             /* Setup tracing */
   interactive_debug = FALSE;
   i = 1;
   if ((argv[1][0] == '-') ||
       (argv[1][0] == '?') ||
       (argv[1][0] == '!')) {
      if (argv[1][0]!='-') i=0;
      set_trace(argv[1]+i);
      i = 2;
   }
   execfile = NULL;
   Lscpy(&execfile,(i>=argc)?"":argv[i]);
   ASCIIZ(execfile);
   markp = m_mark();
   SETUP();
   if ((argc>2) && (argv[1][0]=='-') && (!argv[1][1])) {
      /* intepret from args */
      for (l=0,i=2; i<argc; i++) l+=strlen(argv[i])+1;
      exectext=m_malloc(l+4,"FILE");
      strcpy(exectext,argv[2]);
      for (i=3; i<argc; i++) {
        strcat(exectext," ");
        strcat(exectext,argv[i]);
      }
      l = strlen(exectext);
      exectext[l+1] = exectext[l] = '\n';
      exectext[l+2] = '\0';
      rc = 0;
   } else
      rc = LoadFile(STR(execfile),&exectext );
   switch (rc) {
      case 1:  bprintf("ERROR IN EXEC FILE %s LINE 0 - FILE NOT FOUND\n",
                     STR(execfile));
              rc = 801; goto end_of_rexx;
      case 2:  bprintf("ERROR NO READ PERMISSION TO EXEC FILE\n");
              goto end_of_rexx;
      case 3:  bprintf("ERROR NOT ENOUGH MEMORY TO LOAD FILE\n");
              goto end_of_rexx;
   }


   /* fix program arguments */
   arg = mallocargs();
   if (++i<argc) {
      arg->n = 1;
      Lscpy(&(arg->a[0]),"");
      for (; i<argc; i++) {
        Lcat(&(arg->a[0]),argv[i]);
        Lcat(&(arg->a[0])," ");
      }
      if (LEN((arg->a[0])) > 1) LEN((arg->a[0]))--; /* cut last space */
   }

   orig_exec = exectext;
   RxPtr = exectext;
   lineno = 1;
#ifndef __MSDOS__
   if (*RxPtr=='#') {  /* Skip first line */
      while (*RxPtr != '\n') RxPtr++;
      RxPtr++; lineno++;
   }
#endif

   symbol = semicolon_sy;
#if __MSDOS__
   gettime(&t);
   elapsed = (long)t.ti_hour*360000L + (long)t.ti_min*6000L +
            (long)t.ti_sec*100L + (long)t.ti_hund;
#elif defined(HPUX)
   gettimeofday(&tp,&tzp);
   elapsed = tp.tv_sec * 100 + tp.tv_usec / 10000;
#else
   now = time(NULL);
   tm_data = localtime(&now);
   elapsed = (long)tm_data->tm_hour * 3600 +
            (long)tm_data->tm_min * 60 +
            (long)tm_data->tm_sec;
#endif

/* -------------------- EXECUTE THE PROGRAM ------------------- */
   Main_block();

end_of_rexx:
   m_free(execfile);
   freeargs(arg);
   release_everything();

#ifdef PARADOX
   if (paradoxinit) paradox_done();
#endif

   m_release(markp);

   return rc;
} /* main */
