/* ----------------------------------------------------- o o ---- */
/*                                                     ____oo_    */
/*                                          (c)1992   /||    |\   */
/*  Rexx Functions                            BNV      ||    |    */
/*  ~~~~~~~~~~~~~~                                     '.___.'    */
/*                                                     MARMITA    */
/*  Find out if the function is REXX built-in, user defined       */
/*  or an external function and call it.                          */
/* -------------------------------------------------------------- */
/*
 * $Header: rexxfunc.c!v 1.2 1996/12/13 01:04:42 bill Exp bill $
 * $Log: rexxfunc.c!v $
 * Revision 1.2  1996/12/13 01:04:42  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11  03:33:59  bill
 * Initial revision
 *
 */

#include <stdlib.h>
#include <string.h>

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "rxdefs.h"
#include "convert.h"
#include "variable.h"
#include "instruct.h"

void I_procedure ( void );

int  loaded_files_no = 0;

struct loaded_files_st {
       Lstr  *fn;             /* STRUCT */
       char  *text;
       Label *label_scope;
} *loaded_files = NULL;


#define DECL( A )  void R_##A ( args *, int );
#define PXDECL( A )  void PX_##A ( args *, int );

DECL( SSoI     )   DECL( SIoC  )  DECL( S   )   DECL( SIoI )
DECL( SSoIoIoC )   DECL( SoSoC )  DECL( SoI )   DECL( IoI  )
DECL( O        )   DECL( SI    )  DECL( C   )   DECL( oS   )

DECL( arg       )  DECL( compare   )  DECL( copies    )  DECL( close     )
DECL( datatype  )  DECL( eof       )  DECL( errortext )  DECL( find      )
DECL( filesize  )  DECL( format    )  DECL( intr      )  DECL( load      )
DECL( max       )  DECL( min       )  DECL( open      )  DECL( random    )
DECL( read      )  DECL( seek      )  DECL( substr    )  DECL( sourceline)
DECL( strip     )  DECL( storage   )  DECL( space     )  DECL( translate )
DECL( trunc     )  DECL( verify    )  DECL( write     )  DECL( xrange    )

DECL( flush     )  DECL( showlist  )
DECL( port      )

DECL( upper     )  DECL( exists    )  DECL( statef    ) DECL( setprompt  )
DECL( chdir     )  DECL( openconsole) DECL( closeconsole ) DECL( getcwd )

#ifdef DEBUGING
DECL( printmem  )
#endif

/* Math routines */
DECL( abs_sign  )
DECL( math )
DECL( pow )
DECL( atan2 )

#ifdef PARADOX
  PXDECL( Init )
  PXDECL( T )
  PXDECL( TT )
  PXDECL( R )
  PXDECL( TR )
  PXDECL( RR )
  PXDECL( RFTV )
  PXDECL( RFT )
  PXDECL( TRNM )
  PXDECL( Seek )
  PXDECL( Create )
  PXDECL( KeyAdd )
#endif

struct sort_list_st {
          char *name;
          void (*func)(args *, int);
          int  type;
       }
/* //// WARNING THE LIST MUST BE SORTED!!!!!!!!!!!! */
routines_list[] = {
        { "ABBREV",     R_SSoI       ,f_abbrev     },
        { "ABS",        R_abs_sign   ,f_abs        },
#ifdef MATH
        { "ACOS",       R_math       ,f_acos       },
        { "ACOSH",      R_math       ,f_acosh      },
#endif
        { "ADDR",       R_S          ,f_addr       },
        { "ADDRESS",    R_O          ,f_address    },
        { "ARG",        R_arg        ,f_arg        },
#ifdef MATH
	{ "ASIN",       R_math       ,f_asin       },
	{ "ASINH",      R_math       ,f_asinh      },
        { "ATAN",       R_math       ,f_atan       },
        { "ATAN2",      R_atan2      ,f_atan2      },
        { "ATANH",      R_math       ,f_atanh      },
#endif
        { "BITAND",     R_SoSoC      ,f_bitand     },
        { "BITOR",      R_SoSoC      ,f_bitor      },
        { "BITXOR",     R_SoSoC      ,f_bitxor     },
        { "C2D",        R_SoI        ,f_c2d        },
	{ "C2X",        R_S          ,f_c2x        },
        { "CEIL",       R_math       ,f_ceil       },
        { "CENTER",     R_SIoC       ,f_center     },
        { "CENTRE",     R_SIoC       ,f_center     },
        { "CHDIR",      R_chdir      ,f_chdir      },
        { "CLOSE",      R_close      ,f_close      },
        { "CLOSECONSOLE", R_closeconsole, f_closeconsole },
        { "COMPARE",    R_compare    ,f_compare    },
        { "COPIES",     R_copies     ,f_copies     },
#ifdef MATH
        { "COS",        R_math       ,f_cos        },
        { "COSH",       R_math       ,f_cosh       },
#endif
        { "D2C",        R_IoI        ,f_d2c        },
	{ "D2X",        R_IoI        ,f_d2x        },
	{ "DATATYPE",   R_datatype   ,f_datatype   },
        { "DATE",       R_C          ,f_date       },
        { "DELSTR",     R_SIoI       ,f_delstr     },
        { "DELWORD",    R_SIoI       ,f_delword    },
        { "DESBUF",     R_O          ,f_desbuf     },
        { "DIGITS",     R_O          ,f_digits     },
        { "EOF",        R_eof        ,f_eof        },
	{ "ERRORTEXT",  R_errortext  ,f_errortext  },
        { "EXISTS",     R_exists     ,f_exists     },
#ifdef MATH
        { "EXP",        R_math       ,f_exp        },
#endif
        { "FIND",       R_find       ,f_find       },
#ifdef MATH
        { "FLOOR",      R_math       ,f_floor      },
#endif
        { "FLUSH",      R_flush      ,f_flush      },
        { "FORM",       R_O          ,f_form       },
        { "FORMAT",     R_format     ,f_format     },
        { "FUZZ",       R_O          ,f_fuzz       },
        { "GETCWD",     R_getcwd     ,f_getcwd     },
        { "GETENV",     R_S          ,f_getenv     },
        { "INDEX",      R_SSoI       ,f_index      },
        { "INSERT",     R_SSoIoIoC   ,f_insert     },
#ifdef __MSDOS__
	{ "INTR",       R_intr       ,f_intr       },
#endif
        { "JUSTIFY",    R_SIoC       ,f_justify    },
        { "LASTPOS",    R_SSoI       ,f_lastpos    },
        { "LEFT",       R_SIoC       ,f_left       },
        { "LENGTH",     R_S          ,f_length     },
#ifdef MATH
        { "LN",         R_math       ,f_log        },
#endif
        { "LOAD",       R_load       ,f_load       },
#ifdef MATH
        { "LOG",        R_math       ,f_log        },
        { "LOG10",      R_math       ,f_log10      },
#endif
        { "MAKEBUF",    R_O          ,f_makebuf    },
        { "MAX",        R_max        ,f_max        },
        { "MIN",        R_min        ,f_min        },
        { "OPEN",       R_open       ,f_open       },
        { "OPENCONSOLE",R_openconsole,f_openconsole},
        { "OVERLAY",    R_SSoIoIoC   ,f_overlay    },
#ifdef __MSDOS__
        { "PORT",       R_port       ,f_port       },
#endif
	{ "POS",        R_SSoI       ,f_pos        },
#ifdef MATH
        { "POW",        R_pow        ,f_pow        },
        { "POW10",      R_math       ,f_pow10      },
#endif
#ifdef DEBUGING
        { "PRINTMEM",   R_printmem   ,f_printmem   },
#endif
	{ "PUTENV",     R_S          ,f_putenv     },
#ifdef PARADOX
	{ "PXADD",      PX_TT        ,px_tbladd    },
	{ "PXBLOBDROP", PX_TR        ,px_blobdrop  },
	{ "PXCLOSE",    PX_T         ,px_tblclose  },
	{ "PXCOPY",     PX_TT        ,px_tblcopy   },
	{ "PXCREATE",   PX_Create    ,px_tblcreate },
	{ "PXDECRYPT",  PX_T         ,px_tbldecrypt},
	{ "PXDELETE",   PX_T         ,px_tbldelete },
	{ "PXEMPTY",    PX_T         ,px_tblempty  },
	{ "PXENCRYPT",  PX_TT        ,px_tblencrypt},
	{ "PXERRORMSG", PX_R         ,px_errormsg  },
	{ "PXEXIST",    PX_T         ,px_tblexist  },
	{ "PXFIELDS",   PX_T         ,px_tblnflds  },
	{ "PXFLDGET",   PX_RFT       ,px_fldget    },
	{ "PXFLDHANDLE",PX_TT        ,px_fldhandle },
	{ "PXFLDNAME",  PX_TR        ,px_fldname   },
	{ "PXFLDPUT",   PX_RFTV      ,px_fldput    },
	{ "PXFLDTYPE",  PX_TR        ,px_fldtype   },
	{ "PXINIT",     PX_Init      ,px_init      },
	{ "PXKEYADD",   PX_KeyAdd    ,px_keyadd    },
	{ "PXKEYDROP",  PX_TR        ,px_keydrop   },
	{ "PXKEYS",     PX_T         ,px_tblnkeys  },
	{ "PXNAME",     PX_T         ,px_tblname   },
	{ "PXOPEN",     PX_T         ,px_tblopen   },
	{ "PXPROTECTED",PX_T         ,px_tblprotected},
	{ "PXPSWADD",   PX_T         ,px_pswadd    },
	{ "PXPSWDEL",   PX_T         ,px_pswdel    },
	{ "PXRECAPPEND",PX_TR        ,px_recappend },
	{ "PXRECCLOSE", PX_R         ,px_recclose  },
	{ "PXRECCOPY",  PX_RR        ,px_reccopy   },
	{ "PXRECDELETE",PX_T         ,px_recdelete },
	{ "PXRECEMPTY", PX_R         ,px_recempty  },
	{ "PXRECGET",   PX_TR        ,px_recget    },
	{ "PXRECINSERT",PX_TR        ,px_recinsert },
	{ "PXRECOPEN",  PX_T         ,px_recopen   },
	{ "PXRECORDS",  PX_T         ,px_tblnrecs  },
	{ "PXRECUPDATE",PX_TR        ,px_recupdate },
	{ "PXRENAME",   PX_TT        ,px_tblrename },
	{ "PXSEEK",     PX_Seek      ,px_seek      },
	{ "PXSRCHFLD",  PX_TRNM      ,px_srchfld   },
	{ "PXSRCHKEY",  PX_TRNM      ,px_srchkey   },
	{ "PXUPGRADE",  PX_T         ,px_tblupgrade},
#endif
	{ "QUEUED",     R_O          ,f_queued     },
	{ "RANDOM",     R_random     ,f_random     },
	{ "READ",       R_read       ,f_read       },
	{ "REVERSE",    R_S          ,f_reverse    },
	{ "RIGHT",      R_SIoC       ,f_right      },
	{ "SEEK",       R_seek       ,f_seek       },
#ifdef __HAIKU__
	{ "SETPROMPT",  R_setprompt  ,f_setprompt  },
	{ "SHOWLIST",   R_showlist   ,f_showlist   },
#endif
	{ "SIGN",       R_abs_sign   ,f_sign       },
#ifdef MATH
	{ "SIN",        R_math       ,f_sin        },
	{ "SINH",       R_math       ,f_sinh       },
#endif
	{ "SOUNDEX",    R_S          ,f_soundex    },
	{ "SOURCELINE", R_sourceline ,f_sourceline },
        { "SPACE",      R_space      ,f_space      },
#ifdef MATH
        { "SQRT",       R_math       ,f_sqrt       },
#endif
        { "STATEF",     R_statef     ,f_statef     },
        { "STORAGE",    R_storage    ,f_storage    },
        { "STRIP",      R_strip      ,f_strip      },
        { "SUBSTR",     R_substr     ,f_substr     },
        { "SUBWORD",    R_SIoI       ,f_subword    },
        { "SYMBOL",     R_S          ,f_symbol     },
#ifdef MATH
        { "TAN",        R_math       ,f_tan        },
        { "TANH",       R_math       ,f_tanh       },
#endif
        { "TIME",       R_C          ,f_time       },
        { "TRACE",      R_C          ,f_trace      },
        { "TRANSLATE",  R_translate  ,f_translate  },
        { "TRUNC",      R_trunc      ,f_trunc      },
        { "UPPER",      R_upper      ,f_upper      },
        { "VALUE",      R_S          ,f_value      },
        { "VARTREE",    R_oS         ,f_vartree    },
        { "VERIFY",     R_verify     ,f_verify     },
        { "WORD",       R_SI         ,f_word       },
        { "WORDINDEX",  R_SI         ,f_wordindex  },
        { "WORDLENGTH", R_SI         ,f_wordlength },
        { "WORDPOS",    R_SSoI       ,f_wordpos    },
        { "WORDS",      R_S          ,f_words      },
        { "WRITE",      R_write      ,f_write      },
        { "X2C",        R_S          ,f_x2c        },
        { "X2D",        R_SoI        ,f_x2d        },
        { "XRANGE",     R_xrange     ,f_xrange     }
};

#define LISTSIZE(a) sizeof((a)) / sizeof(struct sort_list_st) -1

/* -------------------------------------------------------------- */
/* Rexx_Function( Lstr *function-name )                           */
/* this routine will search first to see if the function specified*/
/* is internal (built-in) rexx function and call it on success,   */
/* otherwise, it will search the entire program for a label       */
/* with the same name,                                            */
/* and if it also doesn't succeed it will search the directory    */
/* path for a program with the same name                          */
/* -------------------------------------------------------------- */
void Rexx_Function ( Lstr *funcname )
{
     Label   *lab;
     int      lno;
     char    *_Bptr;
     int      _file_type;
     char    *_orig_exec;
     char    *_exectext;
     Lstr    *_execfile;           /* REMEMBER OLD VALUE */
     Label   *_label_scope;


     int      first, middle, last, cmp, found;

     first = found = 0;        /* Use binary search to find intruction */
     last  = LISTSIZE(routines_list);
     while (first<=last)   {
       middle = (first+last)/2;
       if ((cmp=Lcmp(funcname,routines_list[middle].name))==0)  {
          found=1;
          break;
       }  else
       if (cmp<0) last  = middle-1;
       else       first = middle+1;
     }
     if (found) {
       (*routines_list[middle].func)(arg, routines_list[middle].type );
       arg->n = 1;   /* Result is returned */
       return;
     }
/* Check if it is user defined function */

     setvar("SIGL",Blineno);

     lab = searchlabel( funcname, &label_scope );

     _file_type = file_type;
     _orig_exec = orig_exec;
     _exectext  = exectext;
     _execfile  = execfile;
     _label_scope = label_scope;

     if (lab == NULL) {
        file_type = call_type;   lab = NULL;
        for (first=0; (first<loaded_files_no) && (lab==NULL); first++)
           lab = searchlabel( funcname, &loaded_files[first].label_scope);

        if (lab!=NULL) {
           first--;
           execfile = loaded_files[first].fn;
           exectext = orig_exec = loaded_files[first].text;
           label_scope = loaded_files[first].label_scope;
        }
     }

     if (lab != NULL) {
       push_program_status();
       lno = lineno;

       _Bptr = Bptr;
       Bptr = RxPtr = lab->ptr;
       lineno = lab->nnn;       /* jump to the new line */

    nextsymbol();            /* get label */
       nextsymbol();
       skip_semicolons;
       if ((symbol==ident_sy) && (!Lcmp(symbolstr,"PROCEDURE")))
             I_procedure();
       else
           if (Main_block()==2) longjmp(mainblock,2);
       pop_program_status();
       lineno = lno;
       Bptr = _Bptr;
     } else {
       /* Search for an outer function */
       error(ERR_ROUTINE_NOT_FOUND);
     }

     file_type = _file_type;
     orig_exec = _orig_exec;
     exectext  = _exectext;
     execfile  = _execfile;
     label_scope = _label_scope;
} /* Rexx_func */

/* -------------------------------------------------------------- */
/*  System_Function( Lstr **funcname )                            */
/*      executes a system command and return the result in ARGR   */
/* -------------------------------------------------------------- */
void System_Function( Lstr **funcname )
{
    int  rc;
    int  i;


    for (i=0;i<ARGN;i++)
      if (arg->a[i] != NULL) {
         Lcat(funcname," ");
         Lstrcat(funcname,arg->a[i]);
      }

    ASCIIZ(*funcname)

    rc = redirect_cmd(STR(*funcname), FALSE, TRUE, TRUE);

    setvar("RC",rc);
    arg->n = 1;
} /* System_Function */

/* ----------------------* * --------------------- */
void NULLvars( Idrec *root )
{
   if (root != NULL) {
      disposetree(root->left);
      disposetree(root->right);
      root->var = NULL;
   }
} /* NULLvars */

/* -------------------------------------------------------------- */
/*  PROCEDURE [EXPOSE name [name]...] ;                           */
/*      start a new generation of variables within an internal    */
/*      routine. Optionally named variables or groups of          */
/*      variables from an earlier generation may be exposed       */
/* -------------------------------------------------------------- */
void I_procedure ( void )
{
   Idrec     *_scope;
   Lstrlist  *head=NULL, *tail=NULL;
   Idrec     *_A,*A;
   Idrec     *new_arrayhead;
   Lstr      *s;             /* POPPED FROM STACK */
   int        isarray, _tracing;

  _scope = scope;     /* Push old Scope */
  scope  = NULL;      /* Create a new set of variables */
  AIR_print;          /* print label */
  Bptr = Pptr; Blineno = lineno;
  nextsymbol();
  AIR_print;

/*
//  Den doyleyei kala otan baloyme dyo i treis fores tin
//  idia metablhth  procedure expose haha haha   .....
//  i gia arrays    procedure expose i test.i test.
//  PROBLHMATIKO DEN DOYLEYEI KALA  ^^^^^^^^^^^^^^^^
//  Meta tin routina sbynontai ola
*/

  _tracing = tracing;
  tracing = off_trace;
  if ((symbol==ident_sy) && (!Lcmp(symbolstr,"EXPOSE"))) {
     nextsymbol();
     while (symbol==ident_sy) {
       A = V_create(symbolisarray,symbolstr, &scope);
       _A = V_create(symbolisarray,symbolstr,&_scope);
       A->var = _A->var;   /* point the old one    */
       if (symbolisarray && stem) {
/* Destroy the tree that might been created by a command like:
   procedure expose test.i test.   */
          NULLvars(A->ara);
          disposetree(A->ara);
          A->ara = _A->ara;
       }
       push_Lstr(symbolstr,&head,&tail);
       nextsymbol();
     }
  }

  tracing = _tracing;
  mustbe_semicolon;
  if (Main_block()==2) longjmp(mainblock,2);

  _tracing = tracing;
  tracing = off_trace;
  while (head != NULL) {
     s = pop_Lstr(&head,&tail);
     isarray = (memchr(STR(s),'.',LEN(s))!=NULL);
     A = V_search(isarray,s, scope);  new_arrayhead = ArrayHead;
    _A = V_search(isarray,s,_scope);

     if (isarray && (new_arrayhead==ArrayHead)) ;
     else
     if (A != NULL) {
       _A->var = A->var;
       _A->ara = A->ara;
       A->var = NULL;       /* so it wont be freed with disposetree(scope) */
       A->ara = NULL;
     } else
     if (_A != NULL) {   /* New is NULL, and old is not, ie DROPPED */
       V_drop(isarray,s,&_scope);
     }
     m_free(s);
  }
  tracing = _tracing;

  if (scope != NULL) disposetree(scope);    /* Destroy local scope */
  scope = _scope;                           /* Pop the old one  */
} /* I_procedure */

/* -------------------------------------------------------------- */
/*  LOAD( filename )                                              */
/*      load a rexx file so it can be used as a library           */
/*      returns a return code from loadfile                       */
/*        "-1" when file is already loaded                        */
/*         "0" on success                                         */
/*         "1" on error opening the file                          */
/* -------------------------------------------------------------- */
int  LoadFile( char *, char **);

void R_load( args *arg, int i )
{
   char  *_lastlabelptr;   /* Pointer to Last label found   */
   int    _lastlineno;     /* Last line number              */
   char  *_orig_exec;      /* the originall exec file pointer*/
   Lstr  *_execfile;          /* REMEMBER OLD VALUE */

   char  *exectext;

   if (ARGN!=1) error(ERR_INCORRECT_CALL);
   L2str(&ARG1);  ASCIIZ(ARG1);
#ifdef __MSDOS__
   L2upper(ARG1);
#endif

   for (i=0; i<loaded_files_no; i++)
      if (!Lstrcmp(ARG1,loaded_files[i].fn)) {
         Licpy(&ARGR,-1); return;
      }
   Licpy(&ARGR, LoadFile( STR(ARG1), &exectext));

   if (INT(ARGR))  /* Retry with getenv("rxlib") in prefix */
     if ((exectext=getenv("RXLIB"))!=NULL) {
         Lscpy(&ARG2,exectext); i = strlen(exectext);
         if (exectext[i-1]!=FILESEP) {
            STR(ARG2)[i] = FILESEP;
            LEN(ARG2)++;
         }
         Lstrcat(&ARG2,ARG1); ASCIIZ(ARG2);
         Licpy(&ARGR, LoadFile( STR(ARG2), &exectext));
     }

   if (!INT(ARGR)) {
      i = loaded_files_no++;
      loaded_files = m_realloc(loaded_files,
                               loaded_files_no *
                                  sizeof(struct loaded_files_st),
                               "LOAD");
      loaded_files[i].fn = NULL;
      Lstrcpy(&loaded_files[i].fn, ARG1);  ASCIIZ(loaded_files[i].fn);
      loaded_files[i].text = exectext;
      loaded_files[i].label_scope = NULL;
      /* Now search all the file for labels  */

      _lastlabelptr = lastlabelptr;
      _lastlineno = lastlineno;
      _orig_exec = orig_exec;
      _execfile = execfile;

      lastlabelptr = NULL;
      orig_exec = exectext;
      execfile = ARG1;

      Lscpy(&ARG2,"0000"); /* an invalid one, so it will search everything */
      searchlabel(ARG2, &loaded_files[i].label_scope);

      lastlabelptr = _lastlabelptr;
      lastlineno = _lastlineno;
      orig_exec = _orig_exec;
      execfile = _execfile;
   }
} /* R_load */
