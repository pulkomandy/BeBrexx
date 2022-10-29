/* ============================================================== */
/*   Bill N. Vlachoudis                                    o o    */
/*   Computer adDREss.......bnv@nisyros.physics.auth.gr  ____oo_  */
/*   HomE AdDreSs...........Eirinis 6                   /||    |\ */
/* !                        TK 555-35 Pylaia             ||    |  */
/*  The bEsT is   !         ThesSALONIKI, GREECE         `.___.'  */
/* .. yEt tO   cOme   *                                  MARMITA  */
/* ============================================================== */
/* 
 * $Header: c:/usr/rexx/rexx.h!v 1.6 1997/09/13 15:17:21 bill Exp bill $
 * $Log: rexx.h!v $
 * Revision 1.6  1997/09/13  15:17:21  bill
 * Changed: Multiple stacks support
 *
 * Revision 1.5  1997/07/04  20:33:15  bill
 * *** empty log message ***
 *
 * Revision 1.4  1997/04/15 13:47:53  bill
 * *** empty log message ***
 *
 * Revision 1.3  1996/12/13  01:05:06  bill
 * Changed mail address
 *
 * Revision 1.2  1995/12/13  01:10:26  bill
 * IP address changed
 *
 * Revision 1.1  1995/09/11  03:36:52  bill
 * Initial revision
 *
 */

#ifndef __REXX_H__
#define __REXX_H__

#if defined(hpux) || defined(__hpux)
/*# define _HPUX_SOURCE */
# define HPUX
#endif

#ifdef __MSDOS__
# ifndef __LARGE__
#  error  Please compile in Large model!
# endif
# define HUGE huge
# define OS    "MSDOS"
# define SHELL "COMSPEC"
# define FILESEP '\\'
# define PATHSEP ';'
#else
#ifdef __BEOS__
# undef  HUGE
# define HUGE
#ifdef __INTEL__
# define OS    "BeOS x86"
#else
# define OS    "BeOS PPC"
#endif
# define SHELL "SHELL"
# define FILESEP '/'
# define PATHSEP ':'
#else
# undef  HUGE
# define HUGE
# define OS    "UNIX"
# define SHELL "SHELL"
# define FILESEP '/'
# define PATHSEP ':'
#endif
#endif

/* Comment or uncomment the following defines */
#define MATH
#ifdef __MSDOS__
/*#   define PARADOX*/
#endif
/* #define DEBUGING */

#include <setjmp.h>

#include "common.h"
#include "memory.h" /* Warning there is a file in include dir with the same name */
#include "lstring.h"

#ifdef  __REXX_C__
#  define EXTERN
#  ifdef __MSDOS__
      extern unsigned _stklen = 32000U;
#  endif
#else
#  define EXTERN extern
#endif

#define   maxerr         7
#define   maxargs       10
#define   linelen      126
#define   maxfile     1024
#define   maxenvi      250
#if defined(PARADOX)
#  define   rx_version     "BREXX R1.3.2 "##__DATE__##" PARADOX SUPPORT"
#elif defined(__MPW__)
#  define   rx_version     "BREXX R1.3.2_MPW "##__DATE__##" PARADOX SUPPORT"
#else
#  define   rx_version     "BREXX R1.3.2 "
#endif
#define   author      "Bill N. Vlachoudis (bnv@nisyros.physics.auth.gr or V.Vlachoudis@cern.ch)\n" \
                      "This version by W.G.J. Langeveld (langeveld@verifiedlogic.com)"
#define   SCIENTIFIC     0
#define   ENGINEERING    1

#ifdef __REXX_C__
   char
#ifdef __MSDOS__
      *clower="abcdefghijklmnopqrstuvwxyzòôöõúùûü†°¢£§•¶ß®©´¨≠ÆØ‡™·‚„ÂÊÁÈ‰Ë",
      *cUPPER="ABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóëÍÎÏÌÓÔàì",
#else
      *clower="abcdefghijklmnopqrstuvwxyz",
      *cUPPER="ABCDEFGHIJKLMNOPQRSTUVWXYZ",
#endif
      *cdigits = "0123456789",
      *chex    = "0123456789ABCDEFabcdef",
      *crxsymb = "@#$_.?!",
      *interactive_msg = "       +++ Interactive trace.  TRACE OFF to end debug, ENTER to continue. +++";
#else
   extern char
      *clower,
      *cUPPER,
      *cdigits,
      *chex,
      *crxsymb,
      *interactive_msg;
#endif


typedef struct Idrec_st Idrec;      /* Variables binary tree, record */
struct Idrec_st {
       Lstr    *name;
       Lstr    *var;
       Idrec   *ara;
       Idrec   *left;
       Idrec   *right;
};

typedef struct label_st Label;      /* Labels binary tree, record */
struct label_st {
       Lstr   *name;
       char   *ptr;
       int     nnn;
       Label  *left;
       Label  *right;
};

typedef struct arguments_st args;
struct arguments_st {
       int    n;            /* number of arguments */
       Lstr  *r;            /* result              */
       Lstr  *a[maxargs];   /* arguments           */
};

typedef struct Lstrlist_st Lstrlist;   /* double linked Lstring list */
struct Lstrlist_st {                   /* for general purposes, stack etc */
       Lstr     *s;
       Lstrlist *prev;
       Lstrlist *next;
};

typedef struct stacklist_st stacklist; /* double linked Lstring list */
struct stacklist_st {                
       Lstrlist  *head;
       Lstrlist  *tail;
       stacklist *prev;
};

enum tracetype   { all_trace           =0x0001,
                   commands_trace      =0x0002,
                   error_trace         =0x0004,
                   intermediates_trace =0x0008,
                   labels_trace        =0x0010,
                   normal_trace        =0x0020,
                   off_trace           =0x0040,
                   results_trace       =0x0080,
                   scan_trace          =0x0100
                 };
#define AIR_trace  ( all_trace | intermediates_trace | results_trace )

enum flagtype    { f_running
                  ,f_stopped
                 };

enum symboltype   { ident_sy    ,function_sy
                   ,literal_sy
                   ,le_parent                      /*     (    */
                   ,plus_sy                        /*     +    */
                   ,minus_sy                       /*     -    */
                   ,not_sy                         /*     ^    */
                   ,ri_parent                      /*     )    */
                   ,dot_sy                         /*     .    */
                   ,eq_sy       ,deq_sy            /*   =  ==  */
                   ,ne_sy       ,nde_sy            /*  ^= ^==  */
                   ,le_sy       ,ge_sy             /*  <=  =>  */
                   ,lt_sy       ,gt_sy             /*   <  >   */
                   ,times_sy                       /*     *    */
                   ,div_sy                         /*     /    */
                   ,mod_sy                         /*     %    */
                   ,intdiv_sy                      /*    //    */
                   ,power_sy                       /*    **    */
                   ,and_sy                         /*     &    */
                   ,or_sy                          /*     |    */
                   ,xor_sy                         /*    &&    */
                   ,concat_sy                      /*    ||    */
                   ,bconcat_sy                     /*   |b|    */
                   ,comma_sy                       /*     ,    */
                   ,semicolon_sy                   /*     ;    */
                   ,by_sy                          /* in_do    */
                   ,to_sy                          /* in_do    */
                   ,for_sy                         /* in_do    */
                   ,until_sy                       /* in_do    */
                   ,while_sy                       /* in_do    */
                   ,then_sy                        /* in_if_...*/
                   ,with_sy                        /* in_parse */
                   ,label_sy
                   ,exit_sy
                };

enum  stat_type {   nothing_st
                   ,normal_st
                   ,in_do
                   ,in_do_init
                   ,in_if
                   ,in_select
                   ,in_parse_value
                };

enum  prg_type {   COMMAND,
                   FUNCTION,
                   SUBROUTINE };

/* ----------------------------------------------------- (c)BNV 1991 - */
/*  Variable definitions follows                                       */
/* ------------------------------------------------------------------- */

EXTERN int       rc;                 /* Return Code                   */
EXTERN stacklist *stack;             /* List of stacks                */
/*EXTERN Lstrlist *stack_head;*/         /* Head and Tail of              */
/*EXTERN Lstrlist *stack_tail;*/         /* system stack                  */
EXTERN int       flag;               /* General Purpose flag          */

EXTERN Idrec    *ArrayHead;          /* Idrecord to head of array     */
EXTERN int       stem;               /* If current array is stem      */

EXTERN jmp_buf   finito;             /* end of program                */
EXTERN jmp_buf   mainblock;          /* main block of program...      */

EXTERN Idrec    *scope;              /* Head of the binary tree       */
EXTERN char     *lastlabelptr;       /* Pointer to Last label found   */
EXTERN int       lastlineno;         /* Last line number              */
EXTERN Label    *label_scope;        /* Head of labels bin-tree       */

EXTERN Lstr     *execfile;           /* file name of the exec         */
EXTERN char     *exectext;           /* actuall exec file             */
EXTERN char     *orig_exec;          /* the originall exec file pointer*/
EXTERN int       file_type;          /* for parse source              */
EXTERN int       call_type;          /* CALL or FUNCTION call         */

EXTERN args     *arg;                /* Pointer to program arguments  */
EXTERN args     *oarg;               /* Old arguments used only in arg() */

struct rexx_vars_st {
       char     *PTR;                /* pointer to current file       */
       char     *PPTR;               /* pointer before symbol         */
       char     *BPTR;               /* pointer to beg of cur cmd     */
       int       TPREVLINENO;        /* prev line no, used 4 trace    */
       int       LINENO;             /* number of line                */
       int       BLINENO;            /* line number at BPTR           */
       int       PBLANK;             /* if prevcharacter was blank    */
       int       NBLANK;             /* if nextcharacter is blank     */
       int       DEPTH;              /* current nesting level         */
       enum symboltype SYMBOL;       /* contains the symbol           */
       Lstr     *SYMBOLSTR;          /* symbol identifier             */
       int       SYMBOLISARRAY;      /* dot exists in symbolstr       */
       int       SYMBOLISSTR;        /*                               */
       enum stat_type  STATEMENT;    /* statement type                */

       int       TRACING;            /* Kind of active tracing        */
       int       INT_DEBUG;          /* Interactive debug             */
       int       COM_INHIB;          /* command inhibition            */
       int       N_TRACE;            /* number of commands to trace   */
       Lstr     *ENVIRONMENT;        /* string pointing current env   */

       boolean   ERROR;              /* Signal on ERROR?              */
       boolean   HALT;               /* Signal on HALT?               */
       boolean   NOVALUE;            /* Signal on NOVALUE?            */
       boolean   SYNTAX;             /* Signal on SYNTAX?             */

       int       DIGITS;             /* Setting of numeric digits     */
       boolean   FORM;               /* Numeric format                */
       int       FUZZ;               /* Numeric fuzz                  */
};

EXTERN struct rexx_vars_st V;

#define  RxPtr           (V.PTR           )
#define  Pptr            (V.PPTR          )
#define  Bptr            (V.BPTR          )
#define  TPrevLineno     (V.TPREVLINENO   )
#define  lineno          (V.LINENO        )
#define  Blineno         (V.BLINENO       )
#define  Pblank          (V.PBLANK        )
#define  Nblank          (V.NBLANK        )
#define  depth           (V.DEPTH         )
#define  symbol          (V.SYMBOL        )
#define  symbolstr       (V.SYMBOLSTR     )
#define  symbolisarray   (V.SYMBOLISARRAY )
#define  symbolisstr     (V.SYMBOLISSTR   )
#define  statement       (V.STATEMENT     )

#define  tracing         (V.TRACING       )
#define  interactive_debug (V.INT_DEBUG   )
#define  command_inhibition (V.COM_INHIB  )
#define  n_trace         (V.N_TRACE       )

#define  environment     (V.ENVIRONMENT   )

#define  Error           (V.ERROR         )
#define  Halt            (V.HALT          )
#define  Novalue         (V.NOVALUE       )
#define  Syntax          (V.SYNTAX        )

#define  digits          (V.DIGITS        )
#define  form            (V.FORM          )
#define  fuzz            (V.FUZZ          )

#define  ISSPACE(c)      ((c==0x09) || (c==0x0D) || (c==0x20))

/* -------------------- function prototypes ---------------------- */
void  nextsymbol(void);

void    Signal( Label *Lab );
void  I_procedure ( void );
void  I_template ( Lstr * );
void  I_interpret_string ( Lstr **A );

void  Rexx_Function ( Lstr * );
void  System_Function ( Lstr ** );

int   Main_block ( void );

#undef EXTERN
#endif
