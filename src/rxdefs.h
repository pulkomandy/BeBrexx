/*
 * $Header: rxdefs.h!v 1.2 1996/12/13 01:05:25 bill Exp bill $
 * $Log: rxdefs.h!v $
 * Revision 1.2  1996/12/13 01:05:25  bill
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/11  03:37:13  bill
 * Initial revision
 *
 */

/* Some general defines for rexx routines */

size_t skipblanks( Lstr *S, size_t p );
size_t skipword( Lstr *S, size_t p );
long   Lwordindex( Lstr *s, size_t n );
long   Lwordpos( Lstr *phrase, Lstr *s, size_t n );
void   Lspace(Lstr **R, Lstr *A, long n, char pad );

#define ARGN   (arg->n)
#define ARGR   (arg->r)
#define ARG1   (arg->a[0])
#define ARG2   (arg->a[1])
#define ARG3   (arg->a[2])
#define ARG4   (arg->a[3])
#define ARG5   (arg->a[4])
#define ARG6   (arg->a[5])
#define ARG7   (arg->a[6])
#define ARG8   (arg->a[7])
#define ARG9   (arg->a[8])
#define ARG10  (arg->a[9])

#define must_exist(I) if (ARG##I == NULL) error(ERR_INCORRECT_CALL)
#define exist(I)  (ARG##I != NULL)

#define get_s(I)   { must_exist(I); L2str(&ARG##I); }
#define get_i(I,N) { must_exist(I); L2int(&ARG##I); N = INT(ARG##I); \
		 if (N<=0) error(ERR_INCORRECT_CALL); }

#define get_oi(I,N) { if (exist(I)) \
   {  L2int(&ARG##I); N = INT(ARG##I); \
      if (N<=0) error(ERR_INCORRECT_CALL); \
   } else N = 0; }

#define get_i0(I,N) { must_exist(I); L2int(&ARG##I); N = INT(ARG##I); \
		 if (N<0) error(ERR_INCORRECT_CALL); }

#define get_oi0(I,N) { if (exist(I)) \
   {  L2int(&ARG##I); N = INT(ARG##I); \
      if (N<0) error(ERR_INCORRECT_CALL); \
   } else N = 0; }

#define get_pad(I) { if (exist(I)) \
   {  L2str(&ARG##I); \
      if (LEN(ARG##I)!=1) error(ERR_INCORRECT_CALL); \
      pad = STR(ARG##I)[0];  \
   } else pad = ' '; }

enum functions {
 f_abbrev,        f_addr,          f_address,       f_arg,
 f_bitand,        f_bitor,         f_bitxor,        f_compare,
 f_copies,        f_center,        f_close,         f_c2d,
 f_c2x,           f_date,          f_datatype,      f_delstr,
 f_delword,       f_d2c,           f_d2x,           f_digits,
 f_errortext,     f_eof,           f_getenv,        f_find,
 f_flush,         f_form,          f_format,        f_fuzz,
 f_justify,       f_index,         f_insert,        f_lastpos,
 f_left,          f_length,        f_load,          f_max,   f_makebuf,
 f_min,           f_open,          f_overlay,       f_value,
 f_pos,           f_putenv,        f_queued,        f_random,
 f_read,          f_reverse,       f_right,         f_time,
 f_trace,         f_translate,     f_trunc,         f_seek,
 f_sourceline,    f_space,         f_storage,       f_strip,
 f_subword,       f_substr,        f_symbol,        f_vartree,
 f_verify,        f_word,          f_wordindex,     f_wordlength,
 f_wordpos,       f_words,         f_write,         f_x2c,
 f_x2d,           f_xrange,        f_desbuf,        f_soundex,
 f_upper,         f_exists,        f_statef,        f_showlist,
 f_setprompt,     f_chdir,         f_closeconsole,  f_openconsole,
 f_getcwd,
#ifdef __MSDOS__
 f_intr, f_port,
#endif
#ifdef DEBUGING
 f_printmem,
#endif

/* Math routines */
 f_abs  ,    f_acos ,    f_asin ,    f_atan ,
 f_atan2,    f_cos  ,    f_cosh ,    f_exp  ,
 f_log  ,    f_log10,    f_pow  ,    f_pow10,
 f_sin  ,    f_sinh ,    f_sign ,    f_sqrt ,
 f_tan  ,    f_tanh ,    f_ceil ,    f_floor,
 f_acosh,    f_asinh,    f_atanh

#ifdef PARADOX
 ,px_init,
  px_tblopen,    px_tblclose,    px_tblempty,
  px_tbldelete,  px_tblupgrade,  px_tblcopy,
  px_tblrename,  px_tbladd,      px_tblname,
  px_tblexist,   px_tblnrecs,    px_tblnkeys,
  px_tblnflds,   px_tblcreate,
  px_blobdrop,
  px_recappend,  px_recinsert,   px_recupdate,
  px_recdelete,  px_recget,      px_reccopy,
  px_recopen,    px_recclose,    px_recempty,
  px_fldput,     px_fldget,
  px_fldhandle,  px_fldtype,     px_fldname,
  px_srchfld,    px_srchkey,     px_errormsg,
  px_keyadd,     px_keydrop,     px_keymap,
  px_keyquery,
  px_tblprotected, px_pswadd,    px_pswdel,
  px_tblencrypt, px_tbldecrypt,
  px_seek
#endif

};
