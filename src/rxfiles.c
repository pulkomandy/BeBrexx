/*
 * $Header: rxfiles.c!v 1.2 1996/12/13 01:03:10 bill Exp bill $
 * $Log: rxfiles.c!v $
 * Revision 1.2  1996/12/13 01:03:10  bill
 * Added: FLUSH(file)
 *
 * Revision 1.1  1995/09/11  03:33:05  bill
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef __sun__
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include <time.h>

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"

#include "rxdefs.h"

#ifdef __MSDOS__
#define OPEN_FILES 5
#else
#define OPEN_FILES 3
#endif
int   open_files = OPEN_FILES;

struct files_st {
       Lstr *name;      /* IN STRUCTURE */
       FILE *f;
}  *file;


/* ----------------------* init_files *-------------------------- */
void init_files(void)
{
   int i;

   file = (struct files_st *)
	     m_malloc( open_files * sizeof(struct files_st),"FILE");
   for (i=0; i<open_files; i++) file[i].name = NULL;
   Lscpy(&file[0].name,"<STDIN>");    file[0].f = stdin;
   Lscpy(&file[1].name,"<STDOUT>");   file[1].f = stdout;
   Lscpy(&file[2].name,"<STDERR>");   file[2].f = stderr;
#ifdef __MSDOS__
   Lscpy(&file[3].name,"<STDAUX>");   file[3].f = stdaux;
   Lscpy(&file[4].name,"<STDPRN>");   file[4].f = stdprn;
#endif
} /* init_files */

/* ---------------------* close_files *----------------------- */
void close_files(void)
{
   int i;

   for (i=OPEN_FILES;i<open_files;i++)
     if (file[i].name != NULL) fclose(file[i].f);
} /* close_files */
/* ----------------------* find_file *------------------------ */
int  find_file( Lstr *f )
{
   int i,j=-1;

   if (datatype(f)=='I') L2int(&f);
   if (TYPE(f) == INT_TY) j = (int)INT(f);

   if (IN_RANGE(0,j,open_files-1))
      if (file[j].name != NULL) return j;

   L2str(&f);
#ifdef __MSDOS__
   L2upper(f);
#endif

   for (i=0; i<open_files; i++)
     if (file[i].name != NULL)
	if (!Lstrcmp(f, file[i].name)) return i;

   return -1;
} /* find_file */

/* ----------------------* find_empty *---------------------- */
int find_empty( void )
{
   int i;
   for (i=0; i<open_files; i++)
      if (file[i].name==NULL) return i;

   i = open_files++;
   /* then allocate some more space */
   file = (struct files_st *)
	     m_realloc( file, open_files * sizeof(struct files_st),"FILE");

   file[i].name = NULL;   file[i].f = NULL;
   return i;
} /* find_empty */

/* ----------------------* open_file *----------------------- */
int open_file( Lstr *fn, char *mode )
{
   int i;
   i = find_empty();

#ifdef __MSDOS__
   L2upper(fn);
#endif

   ASCIIZ(fn);

   if ((file[i].f=fopen(STR(fn),mode))==NULL) return -1;
   Lstrcpy(&file[i].name,fn);
   return i;
} /* open_file */

/* --------------------------------------------------------------- */
/*  OPEN( file, mode )                                             */
/*      opens a file. (mode follows C prototypes                   */
/*      and returns a number for that file                         */
/*      -1 if file is not found!                                   */
/* --------------------------------------------------------------- */
void R_open( args *arg )
{
    if (ARGN != 2) error( ERR_INCORRECT_CALL );
    must_exist(1); L2str(&ARG1);
    must_exist(2); L2str(&ARG2);
    L2lower(ARG2); ASCIIZ(ARG2);
    Licpy(&ARGR, open_file(ARG1,STR(ARG2)));
} /* R_open */
/* --------------------------------------------------------------- */
/*  CLOSE( file )                                                  */
/*      closes an opened file.                                     */
/*      file may be string or filenumber                           */
/* --------------------------------------------------------------- */
void R_close( args *arg )
{
   int i;

   if (ARGN != 1) error( ERR_INCORRECT_CALL );
   i=find_file(ARG1);
   if (i==-1) error( ERR_FILE_NOT_OPENED );
   fclose(file[i].f);      file[i].f = NULL;
   m_free(file[i].name);
   file[i].name = NULL;
} /* R_close */
/* --------------------------------------------------------------- */
/*  EOF( file )                                                    */
/*      returns 1 at eof, -1 when file is not opened               */
/*      and 0 otherwise                                            */
/* --------------------------------------------------------------- */
void R_eof( args *arg )
{
   int i;
   if (ARGN!=1) error( ERR_INCORRECT_CALL );
   i = find_file(ARG1);
   if (i==-1) Licpy(&ARGR,-1);
   else  Licpy(&ARGR,((feof(file[i].f))?1:0));
} /* R_eof */
/* --------------------------------------------------------------- */
/*  FLUSH( file )                                                  */
/*      flushes stream file to disk                                */
/* --------------------------------------------------------------- */
void R_flush( args *arg )
{
   int i;
   if (ARGN!=1) error( ERR_INCORRECT_CALL );
   i = find_file(ARG1);
   if (i==-1) Licpy(&ARGR,-1);
   else  Licpy(&ARGR,(fflush(file[i].f)));
} /* R_flush */
/* --------------------------------------------------------------- */
/*  WRITE( (file)(, string(,)))                                    */
/*      writes the string to file.                                 */
/*      returns the number of bytes written                        */
/*    **if a comma is specified as a third argument then           */
/*      newline is added at the end of the string                  */
/*    **if neither string is specified then results to one         */
/*      newline in the file                                        */
/* --------------------------------------------------------------- */
void R_write( args *arg )
{
    int    i;
    size_t p;
    char  *c;

    if (!IN_RANGE(1,ARGN,3)) error( ERR_INCORRECT_CALL );
    i = 1;
    if (exist(1))
       if (LEN(ARG1))  i = find_file(ARG1);
/*    if (exist(3)) error( ERR_INCORRECT_CALL ); */
    if (i==-1) i = open_file(ARG1,"w");
    if (i==-1) error(ERR_CANT_OPEN_FILE);
    if (exist(2)) {
       L2str(&ARG2);
       c = STR(ARG2); p = LEN(ARG2);
       while (p--) fputc(*c++,file[i].f);
       Licpy(&ARGR, LEN(ARG2));
    }  else {
       fputc('\n',file[i].f);
       Licpy(&ARGR,1);
    }
    if (ARGN==3) {
       fputc('\n',file[i].f);
       INT(ARGR)++;
    }
}  /* R_write */
/* --------------------------------------------------------------- */
/*  READ( (file)(,length) )                                        */
/*      returns reads length bytes from file.                      */
/*      if length is not specified then return one line from file  */
/* --------------------------------------------------------------- */
void R_read( args *arg )
{
    int    i,ci;
    long   li,l;
    char  *c;

    if (!IN_RANGE(0,ARGN,2)) error( ERR_INCORRECT_CALL );
    i = 0;
    if (exist(1))
       if (LEN(ARG1)) i = find_file(ARG1);
    if (i==-1) i = open_file(ARG1,"r");
    if (i==-1) error(ERR_CANT_OPEN_FILE);
    get_oi(2,l);

    if (l) {
       Lfx(&ARGR,(size_t)l); TYPE(ARGR) = STR_TY;
       c = STR(ARGR);
       for (li=0; li<l; li++) {
         ci = getc(file[i].f);
         if (ci==-1) { if (li) li--; break; }
         *c++ = ci;
       }
       LEN(ARGR)=(size_t)li;
    }  else {
       Lfx(&ARGR,50);
       l = 0;
       while ((ci=getc(file[i].f))!='\n') {
          if (ci==-1) break;
          c = STR(ARGR) + (size_t)l;
          *c = ci;  l++;
          if (l > MAXL(ARGR))  Lfx(&ARGR, (size_t)(l + 50));
       }
       LEN(ARGR) = (size_t)l;
    }
} /* R_read */
/* --------------------------------------------------------------- */
/*  SEEK( file (,offset (,"TOF","CUR","EOF")))                     */
/*       move file pointer to offset  relative from TOF  (default) */
/*       and return new file pointer                               */
/* --------------------------------------------------------------- */
void R_seek( args *arg )
{
    int  i;
    int  SEEK=SEEK_SET;

    if (!IN_RANGE(1,ARGN,3)) error( ERR_INCORRECT_CALL );
    must_exist(1); i = find_file(ARG1);
    if (i==-1) error(ERR_FILE_NOT_OPENED);

    if (exist(2)) {
       L2int(&ARG2);
       if (exist(3)) {
          L2str(&ARG3);  L2upper(ARG3);
          if (!Lcmp(ARG3,"TOF")) SEEK = SEEK_SET;
          else
          if (!Lcmp(ARG3,"CUR")) SEEK = SEEK_CUR;
          else
          if (!Lcmp(ARG3,"EOF")) SEEK = SEEK_END;
          else error( ERR_INCORRECT_CALL );
       }
       fseek( file[i].f, INT(ARG2), SEEK );
    }
    Licpy(&ARGR, ftell(file[i].f));
} /* R_seek */

/* --------------------------------------------------------------- */
/*  EXISTS( filename )                                             */
/*      returns 1 if file exists                                   */
/*      and 0 otherwise                                            */
/* --------------------------------------------------------------- */
void R_exists( args *arg )
{
   int i;
   must_exist(1); L2str(&ARG1);

   ASCIIZ(ARG1)

   i = access(STR(ARG1), F_OK) == 0;
   if (i) Licpy(&ARGR, 1);
   else   Licpy(&ARGR, 0);
} /* R_exists */

/* --------------------------------------------------------------- */
/*  CHDIR( pathname )                                              */
/*      returns 1 if successfully changed directory                */
/*      and 0 otherwise                                            */
/* --------------------------------------------------------------- */
void R_chdir( args *arg )
{
   int i;
   must_exist(1); L2str(&ARG1);

   ASCIIZ(ARG1)

   i = chdir(STR(ARG1));

   Licpy(&ARGR, i ? 0 : 1);
} /* R_chdir */

/* --------------------------------------------------------------- */
/*  GETCWD()                                                       */
/*      returns current directory                                  */
/*      and 0 otherwise                                            */
/* --------------------------------------------------------------- */
void R_getcwd( args *arg )
{
   char buff[1024];
   char *p = getcwd(buff, 1024);
   if (p == 0) p = "";
   Lscpy(&ARGR, p);
} /* R_chdir */

static long isdir(char *s, struct stat *stptr)
{
   struct stat st;

   if (stptr == NULL) {
      if (s == NULL) return(0);

      stptr = &st;
      if (stat(s, stptr) == -1) return(0);
   }

   if (S_ISDIR(stptr->st_mode)) return(1);
#ifndef __BEOS__
   if (S_ISLNK(stptr->st_mode)) {
      DIR *r = opendir(s);
      if (r) {
         closedir(r);
         return(1);
      }
   }
#endif
   return(0);
}


/* --------------------------------------------------------------- */
/*  STATEF( filename )                                             */
/*      returns info about file or directory                       */
/*      {DIR | FILE} <length>                                      */
/* --------------------------------------------------------------- */
void R_statef( args *arg )
{
   static char *iso = "%4d%02d%02d" ;
   struct tm *tmdata ;
   struct stat st;
   char buff[256];

   *buff = '\0';

   must_exist(1); L2str(&ARG1);

   ASCIIZ(ARG1);

   if (stat(STR(ARG1), &st) == 0) { 
      tmdata = localtime(&st.st_mtime);
      sprintf(buff, "%s %Ld %Ld %c%c%c%c%c%c%c%c%c %4d/%02d/%02d %02d:%02d",
              isdir(STR(ARG1), &st) ? "DIR" : "FILE",
              st.st_size, st.st_size/512,
              (st.st_mode & S_IRUSR) ? 'R' : '-',
              (st.st_mode & S_IWUSR) ? 'W' : '-',
              (st.st_mode & S_IXUSR) ? 'X' : '-',
              (st.st_mode & S_IRGRP) ? 'R' : '-',
              (st.st_mode & S_IWGRP) ? 'W' : '-',
              (st.st_mode & S_IXGRP) ? 'X' : '-',
              (st.st_mode & S_IROTH) ? 'R' : '-',
              (st.st_mode & S_IWOTH) ? 'W' : '-',
              (st.st_mode & S_IXOTH) ? 'X' : '-',
              tmdata->tm_year+1900, tmdata->tm_mon+1, tmdata->tm_mday,
              tmdata->tm_hour, tmdata->tm_min);
   }
   Lscpy(&ARGR, buff);
} /* R_statef */
