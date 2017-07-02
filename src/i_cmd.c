/*
 * $Header: c:/usr/rexx/i_cmd.c!v 1.4 1997/09/13 15:19:47 bill Exp bill $
 * $Log: i_cmd.c!v $
 * Revision 1.4  1997/09/13  15:19:47  bill
 * Changed: Multiple stack support
 *
 * Revision 1.3  1997/07/04  20:32:47  bill
 * *** empty log message ***
 *
 * Revision 1.2  1996/12/13 01:04:39  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11  03:33:53  bill
 * Initial revision
 *
 */


#ifdef __MSDOS__
# include <io.h>
# include <dir.h>
# include <process.h>
# include <sys/stat.h>
#elif defined(HPUX) || defined(AIX)
# include <unistd.h>
# define S_IREAD  0    /* //// Den ta briskei!!!! */
# define S_IWRITE 1    /* //// Den ta briskei!!!! */
#elif defined(__MPW__)
#else
# if defined(__UNIX__) || defined(__HAIKU__)
#    include <unistd.h>
# endif
# include <sys/stat.h>
# include <fcntl.h>
#endif

#include <stdlib.h>
#include "instruct.h"

#ifndef S_IREAD
# define S_IREAD 0
# define S_IWRITE 1
#endif

#include <fcntl.h>
#include <string.h>

#define  NOSTACK 0
#define  FIFO    1
#define  LIFO    2
#define  STACK   3

#define  STDIN   0
#define  STDOUT  1

#include <signal.h>

static sig_atomic_t sig_interrupted = 0;
void inthandler(int sig)
{
   signal(sig, &inthandler);
   sig_interrupted = 1;
   return;
}

/* ---------------------- Chk4stack ------------------------- */
void Chk4stack(Lstr *A, int *in, int *out )
{
    Lstr *B=NULL;

    *in = *out = 0;
    if (LEN(A)<7) return;
    /* Search for string "STACK)" in front of command
       or for strings    "(STACK", "(FIFO", "(LIFO" at the end */

    Lstrcpy(&B,A); L2upper(B);
    if (!memcmp(STR(B),"STACK>",6)) *in=FIFO;
    if (!memcmp(STR(B)+LEN(B)-6,"(STACK",6)) *out = STACK;
    if (!memcmp(STR(B)+LEN(B)-5,"(FIFO",5)) *out = FIFO;
    if (!memcmp(STR(B)+LEN(B)-5,"(LIFO",5)) *out = LIFO;
    m_free(B);

    if (*in) {
        memmove(STR(A),STR(A)+6,LEN(A)-6);
        LEN(A) -= 6;
    }
    if (*out) LEN(A) -= (*out==STACK)?6:5;
    if (*out==STACK) *out = FIFO;
} /* Chk4stack */

#ifdef __MPW__
int redirect_cmd(char *cmd, int in, int out, int isfunc ) {
int x;
  /* Lets avoid some Warnings... */
  x= *cmd, x= in, x= out, x= isfunc;
  /* And now, get the hell out of here... */
  return 0;
}
#else
/* ------------------ redirect_cmd ----------------- */
int redirect_cmd(char *cmd, int in, int out, int isfunc )
{
    char  fnin[256], fnout[256];
    int   old_stdin, old_stdout;
    int   filein, fileout;
    FILE *f;
    int   rc,l;
    char *c,ci;
    Lstr *A=NULL;

    signal(SIGTERM, &inthandler);
    signal(SIGINT, &inthandler);

    if (in) {    /* redirect input */
       fnin[0] = '\0'; c = getenv("TEMP");
       if (c) strcpy(fnin,c);
       else   strcpy(fnin, "/tmp/");
       l = strlen(fnin);
       if (l) if (fnin[l-1] != FILESEP) {
          fnin[l] = FILESEP; fnin[l+1] = '\0';
       }
       strcat(fnin,"OXXXXXX");
       mktemp(fnin);
       if ((f=fopen(fnin,"w"))!=NULL) {
          while (stack->head!=NULL) {    /* WARNING MAKEBUF!!!!! */
             A = pop_Lstr(&(stack->head),&(stack->tail));
             if (A!=NULL) {
                L2str(&A);  ASCIIZ(A);   fputs(STR(A),f);
                fputc('\n',f);
                m_free(A);
                A = NULL;
             }
          }
          fclose(f);
          filein = open(fnin,S_IREAD);
          if (filein != -1) {
             old_stdin = dup(STDIN);
             dup2(filein,STDIN);
             close(filein);
          }
          else {
             remove(fnin);
             in = FALSE;
          }
       }
       else {
          in = FALSE;
       }
    }
    if (out) {   /* redirect output */
       fnout[0] = '\0'; c = getenv("TEMP");
       if (c) strcpy(fnout,c);
       else   strcpy(fnout, "/tmp/");
       l = strlen(fnout);
       if (l) if (fnout[l-1] != FILESEP) {
          fnout[l] = FILESEP; fnout[l+1] = '\0';
       }
       strcat(fnout,"OXXXXXX");
       mktemp(fnout);
       fileout = creat(fnout,S_IWRITE);
       if (fileout != -1) {
          old_stdout = dup(STDOUT);
          dup2(fileout,STDOUT);
          close(fileout);
       }
       else {
          out = FALSE;
       }
    }

/* --- Execute the command --- */
    rc = system(cmd);

    if (in) {   /* restore input */
       close(STDIN);
       dup2(old_stdin,STDIN);
       close(old_stdin);
       remove(fnin);
    }
    if (out) {  /* restore output */
       close(STDOUT);
       dup2(old_stdout,STDOUT);  /* restore stdout */
       close(old_stdout);
#ifndef __MSDOS__
       chmod(fnout,0666);
#endif
       if ((f=fopen(fnout,"r"))!=NULL) {
        if (isfunc) {
           Lfx(&A,50); l = 0;
           while (!feof(f)) {
              c = STR(A) + l;
              *c = fgetc(f);  l++;
              if (l >= MAXL(A))  Lfx(&A, l + 50);
              }
           l--;
           if (STR(A)[l-1] == '\n') l--;
           LEN(A) = l;
           arg->r = A;
        } else {
           Lfx(&A,50);
           while (!feof(f)) {
              l = 0;
              while ((ci=fgetc(f))!='\n' && !feof(f)) {
                 c = STR(A) + l;
                 *c = ci;  l++;
                 if (l >= MAXL(A))  Lfx(&A, l + 50);
              }
              if (feof(f)) break;
              LEN(A) = l;
              if (out==FIFO) queue_Lstr(A,&(stack->head), &(stack->tail));
              else            push_Lstr(A,&(stack->head), &(stack->tail));
           }
           m_free(A);
        }
        fclose(f);
       }
       remove(fnout); 
    }

    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    if (sig_interrupted) longjmp(mainblock,2);

    return rc;
} /* redirect_cmd */
#endif

/* ----------------------- I_cmd ---------------------------- */
void I_cmd(Lstr *A, Lstr *env )
{
   int    rc=0;
   int    in,out;
   Lstr  *Labname=NULL;
   Label *Lab;

   L2str(&A);
   rc = 0;

   if (tracing == commands_trace ) printcurline();

   ASCIIZ(A);
   if (!Lcmp(env,"COMMAND") ||
       !Lcmp(env,"DOS")     ||
       !Lcmp(env,"CMS")     ||
       !Lcmp(env,"SYSTEM")) {
      Chk4stack(A,&in,&out);
      ASCIIZ(A);
      rc = redirect_cmd(STR(A),in,out,FALSE);
   }
#ifdef __MSDOS__
   else if (!Lcmp(env,"INT2E")) {
      int2e(STR(A));
   }
#endif
#ifdef __BEOS__
   else if (1) { //be_app_exists(env)) {
      ASCIIZ(A);
      rc = redirect_be_app(A, env);
   } 
#endif
   else {
      rc = -3;
   }
 
   if (!Lcmp(env,"EXEC"))  ; /*execl(...); */
/*///    else    execlp(...); */

   setvar("RC",rc);

   if (rc && (tracing & (error_trace | normal_trace))) {
      printcurline();
      bprintf("       +++  RC(%d)\n",rc);
   }

   if (rc && Error) {
      Lscpy(&Labname,"ERROR");
      Lab = searchlabel(Labname,&label_scope);
      Signal(Lab);
   }
   return;
} /* I_cmd */
