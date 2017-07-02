/*
 * $Header: pxdb.c!v 1.1 1995/09/11 03:34:09 bill Exp bill $
 * $Log: pxdb.c!v $
 * Revision 1.1  1995/09/11 03:34:09  bill
 * Initial revision
 *
 */
//
//  Check all PX errors!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
#ifdef __MSDOS__
#include <mem.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <pxengine.h>

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"

#include "rxdefs.h"

#ifdef PARADOX

char  YearPrefix[2] = {'1','9'};

extern int   paradoxinit;
int   px_open_tables = 0;
int   pxerror = 0;

struct tables_st {
       Lstr *name;      /* IN STRUCTURE */
       TABLEHANDLE handle;
} *table = NULL;

/* -----------------------* px *------------------------------- */
int px( int pxerr )
{
   pxerror = pxerr;
   if ( !pxerr ||
      (pxerr==PXERR_RECNOTFOUND) ||
      (pxerr==PXERR_ENDOFTABLE) ||
      (pxerr==PXERR_STARTOFTABLE)) {
	  rc = pxerr;
	  return pxerr;
       }
   if (pxerr) {
     rc = pxerr;
     error(ERR_PARADOX_ERROR);
   }
   return pxerr;  /* Just to keep compiler happy */
} /* px */

/* ---------------------* parardox_done *---------------------- */
void paradox_done( void )
{
  /* è®‡´ò üò ß®úßú† §ò °¢ú†§¶§´ò† ¶¢ò ´ò tables ß¶¨ ú†§ò† ò§¶†Æ´ò */
  if (paradoxinit)
    if (PXExit()) puts("Error closing Paradox Engine");
} /* paradox_done */

/* ---------------------* close_tables *----------------------- */
void close_tables(void)
{
   int i;
   for (i=0;i<px_open_tables;i++)
     if (table[i].name != NULL) px(PXTblClose(table[i].handle));
} /* close_tables */
/* ----------------------* find_file *------------------------ */
int  find_table( Lstr *f )
{
   int i,j=-1;

   if (datatype(f)=='I') L2int(&f);
   if (TYPE(f) == INT_TY) j = (int)INT(f);

   if (IN_RANGE(0,j,px_open_tables))
      if (table[j].name != NULL) return j;

   L2str(&f);
#ifdef __MSDOS__
   L2upper(f);
#endif

   for (i=0; i<px_open_tables; i++)
     if (table[i].name != NULL)
	if (!Lstrcmp(f, table[i].name)) return i;

   return -1;
} /* find_table */

/* ----------------------* find_empty_table *---------------------- */
int find_empty_table( void )
{
   int i;
   for (i=0; i<px_open_tables; i++)
      if (table[i].name==NULL) return i;

   i = px_open_tables++;
   /* then allocate some more space */
   table = (struct tables_st *)
	     m_realloc( table, px_open_tables * sizeof(struct tables_st),"PXTB");

   table[i].name = NULL;   table[i].handle = -1;
   return i;
} /* find_empty_table */

/* ----------------------* open_table *----------------------- */
int open_table( Lstr *fn)
{
   int i;

   i = find_empty_table();

#ifdef __MSDOS__
   L2upper(fn);
#endif

   ASCIIZ(fn);

   if ( px(PXTblOpen( STR(fn), &table[i].handle, 0, TRUE ))) return -1;
   Lstrcpy(&table[i].name,fn);
   return i;
} /* open_table */

/* ----------------------* dateencode *--------------------------- */
TDATE dateencode( Lstr *dstr )
{
   TDATE date;
   int   i,j;
   int   day, month, year;
   char  ds[3]={0,0,0}, ms[3]={0,0,0}, ys[5]={0,0,0,0,0};

   Lstrip(&dstr);

   j = i = 0;
   if (STR(dstr)[j] != '/')  ms[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] != '/')  ms[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] == '/')  j++;
   i = 0;
   if (STR(dstr)[j] != '/')  ds[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] != '/')  ds[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] == '/')  j++;
   i = 0;
   if (STR(dstr)[j])  ys[i++] = STR(dstr)[j++];
   if (STR(dstr)[j])  ys[i++] = STR(dstr)[j++];
   if (STR(dstr)[j])  ys[i++] = STR(dstr)[j++];
   if (STR(dstr)[j])  ys[i++] = STR(dstr)[j++];

   if (strlen(ys)<4) {
     ys[2] = ys[0];
     ys[3] = ys[1];
     ys[0] = YearPrefix[0];
     ys[1] = YearPrefix[1];
   }
   day   = atoi(ds);
   month = atoi(ms);
   year  = atoi(ys);
   px( PXDateEncode( month, day, year, &date ));
   return date;
} /* dateencode */

/* ---------------------* datedecode *---------------------------- */
char *datedecode( TDATE date )
{
   int  day, month, year;
   static char dstr[20]="";
   px(PXDateDecode( date, &month, &day, &year ));
   sprintf(dstr, "%d/%02d/%04d", month, day, year );
   return dstr;
} /* dateencode */

/* ----------------------* timeencode *--------------------------- */
long timeencode( Lstr *dstr )
{
   int   i,j;
   int   hour,min,sec,s100;
   long  l;
   char  hd[3]={0,0,0},
	 ss[3]={0,0,0},
	 ms[3]={0,0,0},
	 hs[5]={0,0,0,0,0};

   Lstrip(&dstr);

   j = i = 0;
   if (STR(dstr)[j] != ':')  hs[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] != ':')  hs[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] == ':')  j++;
   i = 0;
   if (STR(dstr)[j] != ':')  ms[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] != ':')  ms[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] == ':')  j++;
   i = 0;
   if (STR(dstr)[j] != '.')  ss[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] != '.')  ss[i++] = STR(dstr)[j++];
   if (STR(dstr)[j] == '.')  j++;
   i = 0;
   if (STR(dstr)[j])  hd[i++] = STR(dstr)[j++];
   if (STR(dstr)[j])  hd[i++] = STR(dstr)[j++];

   hour  = atoi(hs);
   min   = atoi(ms);
   sec   = atoi(ss);
   s100  = atoi(hd);
   l = ((hour&0xFF)<<24) | ((min&0xFF)<<16) | ((sec&0xFF)<<8) | (s100&0xFF);
   return l;
} /* timeencode */
/* --------------------* timedecode *----------------------- */
char *timedecode( long time )
{
   static char tstr[20]="";
   int    hour,min,sec,s100;
   hour = (int)((time >> 24) & 0xFF);
   min  = (int)((time >> 16) & 0xFF);
   sec  = (int)((time >>  8) & 0xFF);
   s100 = (int)(time & 0xFF);
   sprintf(tstr, "%d:%02d:%02d.%02d", hour, min, sec, s100);
   return tstr;
} /* timedecode */

/* --------------------------------------------------------------- */
/*  PXINIT( )                                                      */
/*      Initialize paradox engine                                  */
/* --------------------------------------------------------------- */
void PX_Init( args *arg, int func )
{
   if (ARGN != 0) error( ERR_INCORRECT_CALL );
   if (!paradoxinit)
      if (!px(PXInit())) paradoxinit = TRUE;
   Licpy(&ARGR,paradoxinit);
   func++;   /* just to keep compiler happy */
} /* PX_Init */

/* --------------------------------------------------------------- */
/*  PXTBLOPEN( tablename )                                         */
/*      and returns a number for that tablehandle                  */
/*      -1 if file is not found!                                   */
/* --------------------------------------------------------------- */
/*  PXTBLCLOSE( table )                                            */
/*      closes an opened table.                                    */
/*      file may be string or filenumber                           */
/* --------------------------------------------------------------- */
/*  PXTBLEMPTY( tablename )                                        */
/*  PXTBLUPGRADE( table )                                          */
/*  PXTBLDELETE( tablename )                                       */
/* --------------------------------------------------------------- */
/*  PXTBLEXIST( tablename )                                        */
/*   returns 0 or 1                                                */
/* --------------------------------------------------------------- */
/*  PXTBLNAME( table )                                             */
/*   returns the name corresponding to a table handle              */
/* --------------------------------------------------------------- */
/*  PXRECORDS( table )                                             */
/*  PXFIELDS( table )                                              */
/*  PXKEYS( table )                                                */
/*  returns the number of records, fields, keys in the table       */
/* --------------------------------------------------------------- */
/*  PXRECOPEN( tablename )                                         */
/*      opens a new record handle for this table                   */
/* --------------------------------------------------------------- */
/*  PXRECDELETE( tablename )                                       */
/*     deletes current record from table                           */
/* --------------------------------------------------------------- */
/*  PXPROTECTED( tablename )                                       */
/*     returns TRUE or FALSE                                       */
/* --------------------------------------------------------------- */
/*  PXPSWADD( password )                                           */
/*  PXPSWDEL( password )                                           */
/* --------------------------------------------------------------- */
/*  PXDECRYPT( tablename )                                         */
/* --------------------------------------------------------------- */
void PX_T( args *arg, int func )
{
   int i;
   int exist;
   long l;
   RECORDHANDLE r;

   if (ARGN != 1) error( ERR_INCORRECT_CALL );
   i=find_table(ARG1);
   L2str(&ARG1); ASCIIZ(ARG1);
   switch (func) {
     case px_tblopen:   Licpy(&ARGR, open_table(ARG1));
			break;
     case px_tblclose:  if (i==-1)
			  ;//px( PXERR_FILE_NOT_OPENED );     //!!!!!!!!!!!!!!!!!!!!!!!!!
			px(PXTblClose( table[i].handle ));
			table[i].handle = -1;
			m_free(table[i].name);
			break;
     case px_recopen:   px( PXRecBufOpen( table[i].handle, &r));
			Licpy(&ARGR, r);
			break;
     case px_recdelete: px( PXRecDelete( table[i].handle ));
			break;
     case px_tblempty:  px( PXTblEmpty( STR(ARG1) ));
			break;
     case px_tblupgrade:px( PXTblUpgrade( table[i].handle ));
			break;
     case px_tbldelete: px( PXTblDelete( STR(ARG1) ));
			break;
     case px_tblexist:  px( PXTblExist( STR(ARG1), &exist ));
			Licpy(&ARGR, exist);
			break;
     case px_tblname:   Lstrcpy(&ARGR, table[i].name);
			break;
     case px_tblnrecs:  px( PXTblNRecs( table[i].handle, &l ));
			Licpy(&ARGR, l);
			break;
     case px_tblnflds:  px( PXRecNFlds( table[i].handle, &i ));
			Licpy(&ARGR, i);
			break;
     case px_tblnkeys:  px( PXKeyNFlds( table[i].handle, &i ));
			Licpy(&ARGR, i);
			break;
     case px_tblprotected: px( PXTblProtected( STR(ARG1), &i ));
			Licpy(&ARGR, i);
			break;
     case px_pswadd:    px( PXPswAdd( STR(ARG1) ));
			break;
     case px_pswdel:    px( PXPswDel( STR(ARG1) ));
			break;
     case px_tbldecrypt:px( PXTblDecrypt( STR(ARG1) ));
			break;
   }
} /* PX_T */

/* --------------------------------------------------------------- */
/*   PXTBLCREATE( tablename, nfields, fields, types )              */
/*   Creates an empty table.                                       */
/*   'fields' and 'types' must be strings with the name of rexx    */
/*   arrays containing the fields and types                        */
/* --------------------------------------------------------------- */
void PX_Create( args *arg )
{
   int   i;
   long  nfields;
   char s[10]; /* for the number */
   char **fields;
   char **types;
   Lstr  *L=NULL,*V=NULL;

   if (ARGN != 4) error( ERR_INCORRECT_CALL );
   get_s(1); ASCIIZ(ARG1);
   get_i(2,nfields);
   get_s(3);
     L2upper(ARG3);
     if (STR(ARG3)[ LEN(ARG3)-1 ] != '.') Lcat(&ARG3,".");
   get_s(4);
     L2upper(ARG4);
     if (STR(ARG4)[ LEN(ARG4)-1 ] != '.') Lcat(&ARG4,".");
   if (nfields>255) error( ERR_INCORRECT_CALL );

   fields = (char **)m_malloc((int)nfields * sizeof(char *), "PXFL");
   types  = (char **)m_malloc((int)nfields * sizeof(char *), "PXTY");

   for (i=0; i<(int)nfields; i++) {
      itoa(i+1,s,10);
      Lstrcpy(&L, ARG3); Lcat(&L,s);
      V_eval(TRUE, &V, L, scope );  L2str(&V);
      fields[i] = (char *)m_malloc( LEN(V)+4, "PXF2" );
      ASCIIZ(V);  strcpy(fields[i], STR(V));

      Lstrcpy(&L, ARG4); Lcat(&L,s);
      V_eval(TRUE, &V, L, scope );
      types[i] = (char *)m_malloc( LEN(V)+4, "PXT2" );
      ASCIIZ(V);  strcpy(types[i], STR(V));
   }
   m_free(L);
   m_free(V);

   px( PXTblCreate( STR(ARG1), (int)nfields, fields, types ));

   for (i=0; i<(int)nfields; i++) {
      m_free(fields[i]);
      m_free(types[i]);
   }
   m_free(fields);
   m_free(types);
   Licpy(&ARGR,pxerror);
} /* PX_Create */

/* --------------------------------------------------------------- */
/*  PXTBLCOPY( tablename1, tablename2 )                            */
/*  PXTBLRENAME( tablename1, tablename2 )                          */
/*  PXTBLADD( tablename1, tablename2 )                             */
/* --------------------------------------------------------------- */
/*  PXFLDHANDLE( tablename, fieldname )                            */
/*  returns the field handle associate with the fieldname          */
/* --------------------------------------------------------------- */
/*  PXENCRYPT( tablename, password )                               */
/* --------------------------------------------------------------- */
void PX_TT( args *arg, int func )
{
   int t;
   FIELDHANDLE h;

   if (ARGN != 2) error( ERR_INCORRECT_CALL );
   get_s(1); ASCIIZ(ARG1);
   get_s(2); ASCIIZ(ARG2);
   switch (func) {
     case px_tblcopy:  px( PXTblCopy( STR(ARG1), STR(ARG2) ));
                       break;
     case px_tblrename:px( PXTblRename( STR(ARG1), STR(ARG2) ));
                       break;
     case px_tbladd:   px( PXTblRename( STR(ARG1), STR(ARG2) ));
                       break;
     case px_fldhandle:t=find_table(ARG1);
                       px( PXFldHandle( table[t].handle, STR(ARG2), &h ));
                       Licpy(&ARGR,h);
                       break;
     case px_tblencrypt: px( PXTblEncrypt( STR(ARG1), STR(ARG2) ));
                       break;
   } /* switch */
} /* PX_TT */
/* --------------------------------------------------------------- */
/*  PXRECPUT( table, record )                                      */
/*      writes the record to table                                 */
/* --------------------------------------------------------------- */
/*  PXRECAPPEND( table, record )                                   */
/*       PXRECINSERT, PXRECUPDATE                                  */
/*      reads one record from table                                */
/* --------------------------------------------------------------- */
/*  PXFLDTYPE( table, fieldhandle )                                */
/*  PXFLDNAME( table, fieldhandle )                                */
/*  returns the field name or field type of 'fieldhandle'          */
/* --------------------------------------------------------------- */
/*  PXBLOBDROP( table, fieldhandle )                               */
/*  PXKEYDROP( tablename, fieldhandle )                            */
/* --------------------------------------------------------------- */
void PX_TR( args *arg, int func )
{
    int   i;
    char  s[30];

    if (ARGN != 2) error( ERR_INCORRECT_CALL );
    must_exist(1);
    must_exist(2); L2int(&ARG2);
    i = find_table(ARG1);
    if (i==-1) px(PXERR_TABLENOTFOUND); //!!!!!!!!!!!!!!!!!!!!!!!!!
    switch (func) {
      case px_recappend:px( PXRecAppend( table[i].handle, (int)INT(ARG2) ));
                        break;
      case px_recinsert:px( PXRecInsert( table[i].handle, (int)INT(ARG2) ));
                        break;
      case px_recupdate:px( PXRecUpdate( table[i].handle, (int)INT(ARG2) ));
                        break;
      case px_recget:   px( PXRecGet( table[i].handle, (int)INT(ARG2) ));
                        break;
      case px_fldtype:  px( PXFldType( table[i].handle, (int)INT(ARG2),
                                       sizeof(s), s));
                        Lscpy(&ARGR,s);
                        break;
      case px_fldname:  px( PXFldName( table[i].handle, (int)INT(ARG2),
                                       sizeof(s), s));
                        Lscpy(&ARGR,s);
                        break;
      case px_blobdrop: px( PXBlobDrop( table[i].handle, (int)INT(ARG2) ));
                        break;
      case px_keydrop:  L2str(&ARG1); ASCIIZ(ARG1);
                        px( PXKeyDrop( STR(ARG1), (int)INT(ARG2) ));
                        break;
    }
}  /* PX_TR */
/* --------------------------------------------------------------- */
/*  PXRECCLOSE( record )                                           */
/*  PXRECEMPTY( record )                                           */
/* --------------------------------------------------------------- */
/*  PXERRORMSG( num )                                              */
/*  returns the error message 'num'                                */
/* --------------------------------------------------------------- */
void PX_R( args *arg, int func )
{
    if (ARGN != 1) error( ERR_INCORRECT_CALL );
    L2int(&ARG1);
    switch (func) {
      case px_recclose: px( PXRecBufClose( (int)INT(ARG1) ));
                        break;
      case px_recempty: px( PXRecBufEmpty( (int)INT(ARG1) ));
                        break;
      case px_errormsg: Lscpy(&ARGR,PXErrMsg((int)INT(ARG1)));
                        break;
    }
} /* PX_R */

/* --------------------------------------------------------------- */
/*  PXRECCOPY( record1, record2 )                                  */
/* --------------------------------------------------------------- */
void PX_RR( args *arg )
{
    if (ARGN != 2) error( ERR_INCORRECT_CALL );
    must_exist(1); L2int(&ARG1);
    must_exist(2); L2int(&ARG2);
    px( PXRecBufCopy( (int)INT(ARG1), (int)INT(ARG2) ));
    Licpy(&ARGR,pxerror);
} /* PX_RR */

/* --------------------------------------------------------------- */
/*  PXFLDPUT( record, field, type, value )                         */
/*    Put a field into a record.                                   */
/*    Type can be  'Alpha'  for string                             */
/*                 'Blob'   for blob  fields                       */
/*                 'Date'   for date                               */
/*                 'Short'  for short integer                      */
/*                 'Long'   for long  integer                      */
/*                 'Time'   for Time                               */
/*                 'Number' for real numbers                       */
/*                 nothing  if you want to skip that field         */
/* --------------------------------------------------------------- */
void PX_RFTV( args *arg )
{
   TDATE  date;
   long   f;
   unsigned long size;
   BLOBHANDLE blb;
   char   type;

   if (ARGN!=4) error( ERR_INCORRECT_CALL );
   must_exist(1); L2int(&ARG1);
   get_i(2,f);
   if (exist(3)) { L2str(&ARG3); type = l2u[STR(ARG3)[0]]; }
   else return;
   switch (type) {
      case 'A':  L2str(&ARG4); ASCIIZ(ARG4);
		 px( PXPutAlpha( (int)INT(ARG1), (int)f, STR(ARG4) ));
		 break;
      case 'M':
      case 'B':  L2str(&ARG4);
		 size = LEN(ARG4);
		 px( PXBlobOpenWrite( (int)INT(ARG1), (int)f, &blb,
				      size, PXBLOBACCEPT));
		 px( PXBlobPut( blb, (unsigned)size, 0L, STR(ARG4) ));
		 px( PXBlobClose( blb, PXBLOBACCEPT ));
		 break;
      case 'D':  L2str(&ARG4); ASCIIZ(ARG4);
		 date = dateencode(ARG4);
		 px( PXPutDate( (int)INT(ARG1), (int)f, date ));
		 break;
      case 'S':  L2int(&ARG4);
		 px( PXPutShort( (int)INT(ARG1), (int)f, (short)INT(ARG4) ));
		 break;
      case 'L':  L2int(&ARG4);
		 px( PXPutLong( (int)INT(ARG1), (int)f, (long)INT(ARG4) ));
		 break;
      case 'T':  L2str(&ARG4); ASCIIZ(ARG4);
		 px( PXPutLong( (int)INT(ARG1), (int)f, timeencode(ARG4)));
		 break;
      case '$':
      case 'N':  L2real(&ARG4);
		 px( PXPutDoub( (int)INT(ARG1), (int)f, (double)REAL(ARG4) ));
		 break;
      default:   error( ERR_INCORRECT_CALL );
   } /* switch */
   Licpy(&ARGR,pxerror);
} /* PX_RFTV */

/* --------------------------------------------------------------- */
/*  PXFLDGET( record, field, type )                                */
/*    Returns a field from a record.                               */
/*    Type can be  'Alpha'  for string                             */
/*                 'Blob'   for blob  fields                       */
/*                 'Date'   for date                               */
/*                 'Short'  for short integer                      */
/*                 'Time'   for time                               */
/*                 'Long'   for long  integer                      */
/*                 'Number' for real numbers                       */
/* --------------------------------------------------------------- */
void PX_RFT( args *arg )
{
   TDATE  date;
   long   f;
   char   type;
   char   s[256];
   short  i;
   long   l;
   unsigned long size;
   double d;
   BLOBHANDLE blb;

   if (ARGN!=3) error( ERR_INCORRECT_CALL );
   must_exist(1); L2int(&ARG1);
   get_i(2,f);
   L2str(&ARG3); type = l2u[STR(ARG3)[0]];
   switch (type) {
      case 'A':  px( PXGetAlpha( (int)INT(ARG1), (int)f,
		     sizeof(s), s));
		 Lscpy(&ARGR,s);
		 break;
      case 'M':
      case 'B':  px( PXBlobOpenRead( (int)INT(ARG1), (int)f, &blb));
		 px( PXBlobGetSize( blb, &size ));
		 Lfx( &ARGR, (size_t)size );
		 px( PXBlobGet( blb, (unsigned)size, 0L, STR(ARGR) ));
		 LEN(ARGR) = (size_t)size;
		 px( PXBlobClose( blb, PXBLOBACCEPT ));
		 break;
      case 'D':  px( PXGetDate( (int)INT(ARG1), (int)f, &date ));
		 Lscpy(&ARGR, datedecode(date));
		 break;
      case 'S':  px( PXGetShort( (int)INT(ARG1), (int)f, &i ));
		 Licpy(&ARGR,i);
		 break;
      case 'T':  px( PXGetLong( (int)INT(ARG1), (int)f, &l ));
		 Lscpy(&ARGR, timedecode(l));
		 break;
      case 'L':  px( PXGetLong( (int)INT(ARG1), (int)f, &l ));
		 Licpy(&ARGR,l);
		 break;
      case '$':
      case 'N':  px( PXGetDoub( (int)INT(ARG1), (int)f, &d ));
		 Lrcpy(&ARGR,d);
		 break;
      default:   error( ERR_INCORRECT_CALL );
   } /* switch */
} /* PX_RFTV */

/* --------------------------------------------------------------- */
/*  PXSEEK( table (,"FIRST","PREV","CUR","NEXT","LAST" (,offset))) */
/*       move table cursor to spesified record                     */
/*       returns the current record number                         */
/* --------------------------------------------------------------- */
void PX_Seek( args *arg )
{
    int    i;
    long   pos;

    if (!IN_RANGE(1,ARGN,3)) error( ERR_INCORRECT_CALL );
    must_exist(1); i = find_table(ARG1);
    if (i==-1) px(PXERR_TABLENOTFOUND);  //!!!!!!!!!!!!!!!!!!!!!!

    if (exist(2)) {
       L2str(&ARG2); L2upper(ARG2);
       if (!Lcmp(ARG2,"FIRST")) px( PXRecFirst( table[i].handle ));
       else
       if (!Lcmp(ARG2,"PREV"))  px( PXRecPrev( table[i].handle ));
       else
       if (!Lcmp(ARG2,"CUR")) /* nothing */;
       else
       if (!Lcmp(ARG2,"NEXT"))  px( PXRecNext( table[i].handle ));
       else
       if (!Lcmp(ARG2,"LAST"))  px( PXRecLast( table[i].handle ));
       else error( ERR_INCORRECT_CALL );

       if (rc) setvar("RC",rc);
       if (exist(3)) {
          L2int(&ARG3);
          px( PXRecNum( table[i].handle, &pos ));
          px( PXRecGoto( table[i].handle, pos+INT(ARG3)));
       }
       if (rc) setvar("RC",rc);
    }
    px( PXRecNum( table[i].handle, &pos ));
    if (rc) setvar("RC",rc);
    Licpy(&ARGR, pos);
} /* PX_Seek */

/* --------------------------------------------------------------- */
/*   PXSRCHFLD( table, record, Nfields(, mode))                    */
/*   PXSRCHKEY( table, record, fieldhandle, mode )                 */
/*   where mode can be  "FIRST", "NEXT", "CLOSEST"                 */
/*   default value for mode is "FIRST"                             */
/* --------------------------------------------------------------- */
void PX_TRNM( args *arg, int func )
{
   int t;
   int mode=SEARCHFIRST;

   if (!IN_RANGE(3,ARGN,4)) error( ERR_INCORRECT_CALL );
   must_exist(1); t = find_table(ARG1);
   must_exist(2); L2int(&ARG2);
   must_exist(3); L2int(&ARG3);
   if exist(4) {
      L2str(&ARG4);  L2upper(ARG4);
      if (!Lcmp(ARG4,"FIRST")) mode = SEARCHFIRST;
      else
      if (!Lcmp(ARG4,"NEXT")) mode = SEARCHNEXT;
      else
      if (!Lcmp(ARG4,"CLOSEST")) mode = CLOSESTRECORD;
      else error( ERR_INCORRECT_CALL );
   }
   switch (func) {
     case px_srchfld: px( PXSrchFld( table[t].handle,
                                     (int)INT(ARG2),
                                     (int)INT(ARG3),
                                     mode ));
                      break;
     case px_srchkey: px( PXSrchKey( table[t].handle,
                                     (int)INT(ARG2),
                                     (int)INT(ARG3),
                                     mode ));
                      break;
   }
   Licpy( &ARGR, pxerror );
} /* PX_TRNM */

/* --------------------------------------------------------------- */
/*   PXKEYADD( tablename, fieldhandles(, mode )                    */
/*   add a primary or secondary (maintained/nonmaintained) index   */
/*   'fieldhandles' is a string with words the field numbers       */
/*   mode can be 'Primary', 'Secondary', 'Incsecondary'            */
/*   default is 'Primary'                                          */
/* --------------------------------------------------------------- */
void PX_KeyAdd( args *arg )
{
   int i,n,m;
   int mode = PRIMARY;
   FIELDHANDLE fieldhandles[20];

   if (!IN_RANGE(2,ARGN,3)) error( ERR_INCORRECT_CALL );
   get_s(1); ASCIIZ(ARG1);
   get_s(2);
   if (ARGN==3) {
      get_s(3);
      switch (l2u[STR(ARG3)[0]]) {
        case 'P': mode = PRIMARY; break;
        case 'S': mode = SECONDARY; break;
        case 'I': mode = INCSECONDARY; break;
        default: error( ERR_INCORRECT_CALL );
      }
   }

   for (n=0; 1; n++) {
      i = (size_t)Lwordindex(ARG2,n+1);
      if (i==-1) break;
      m = i;
      i = skipword(ARG2,i);
      Lstrsub(&ARGR,ARG2,m,i-m);  ASCIIZ(ARGR);
      fieldhandles[n] = atoi(STR(ARGR));
   }
   px( PXKeyAdd( STR(ARG1), n, fieldhandles, mode ));
   Licpy(&ARGR,pxerror);
} /* PX_KeyAdd */

#endif
