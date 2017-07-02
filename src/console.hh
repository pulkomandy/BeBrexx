/** console.h
*
*   A Console Handler with command line history and editing
*
*   Willy Langeveld, September 1998
*
**/
#ifndef CONSOLE__H
#define CONSOLE__H

#define CONSOLE_MAXPARS    20

#include "consolehistory.hh"

class Console {
public:
/*
*   Constructors, destructor
*/
   Console();
   virtual ~Console();
/*
*   set & get flags
*/
   void setflags(long f) {flags = f; return;} 
   long getflags(void)   {return(flags);}
/*
*   Get a line (or whatever characters are available in "raw" mode)
*/
   virtual char *getline(char *buffer, long bufsize);
/*
*   Get a line (or whatever characters are available in "raw" mode)
*/
   virtual long  putline(const char *buffer, long bufsize);
/*
*   Set a prompt
*/
   void setprompt(const char *prompt);

protected:
/*
*   Override this function if you want to do your own character input
*/
   virtual int  get_char(void)                             = 0;
   virtual void setraw(long waitflag)                      = 0;
   virtual void setcooked(void)                            = 0;
/*
*   ...Output routines
*/
   virtual long  WriteE(long, long, const char *, long)    = 0;
   virtual long  WriteT(const char *, long, long)          = 0;
   virtual long  Write(const char *, long)                 = 0;
 
   long         waitflag;
   long         cooked;
   long         flags;           /* various flags - see below       */

private:
   typedef void       (*ConsoleParser)(long);
/*
*   ...Stuff for the VT100 protocol converter. Screen stuff:
*/
   void                VTMove(long);
   void                VTBell(void);
   void                VTEraseScreen(void);
/*
*   ...Key-stroke parsing routines
*/
   void                ResizeBuffers(long);
   void                InvalidEsc(void);

   virtual void        ParseChar(long);
   virtual void        ParseESC(long);
   virtual void        ParseCSI(long);
   virtual void        ParseLong(long);
   virtual void        ParseKeypad(long);
   virtual void        ParseFinalCSI(long);
   virtual void        ParseFinalESC(long);
/*
*   ...Cursor key handlers
*/
   void                DoKeyUp(void);
   void                DoKeyDown(void);
   void                DoKeyLeft(void);
   void                DoKeyRight(void);

   void                ShowUsage(void);
/*
*   ...utitlities for adding and deleting characters in the command line
*/
   long                Delete(long pos, long n);
   long                Insert(long pos, char);


   long                havelines;       /* counts lines in input buffer    */
   long                hold;            /* xon/xoff (^S/^Q) flow control   */

   long                havecr;          /* last char was cr                */
   long                havelf;          /* last char was lf                */

   char               *prompt;          /* an optional prompt              */

   char               *cmdline;         /* current command line            */
   long                cursor;          /* cursor pos                      */
   long                cmdlen;          /* current command length          */
   long                cmdlinesize;     /* maximum (allocated) cmd length  */
   char               *undobuf;         /* undo buffer for cmd line        */

   ConsoleHistory      history;         /* command history                 */

   long                hsize;           /* horizontal size of screen       */
   long                vsize;           /* vertical size of screen         */
   long                xpos;            /* current cursor x position       */
   long                ypos;            /* current cursor y position       */
/*
*   The escape sequence parser for keys from the net
*/
   void               (Console::*CurParse)(long);  /* function*/
   long                pars[CONSOLE_MAXPARS];   /* parameter array for <csi>'s     */
   long                parcount;        /* current parameter               */
   long                prevescape;      /* previous escape start           */
/*
*   No copying
*/
   Console(const Console &);
   const Console &operator=(const Console &);
};

/*
*   "flags". BINARY means "telnet" binary, whereas RAW means "console" raw.
*/
#define CONSOLE_BINARY     0x00000001
#define CONSOLE_RPEND      0x00000002
#define CONSOLE_WAIT_FOR   0x00000004
#define CONSOLE_RAW        0x00000008
#define CONSOLE_OVERSTRIKE 0x00000010
#define CONSOLE_BSDEL      0x00000020
/*
*   Conversion of characters to VT100 equivs.
*/
#define CONSOLE_CONVLF    0x000020000
#define CONSOLE_CONVRV    0x000040000
#define CONSOLE_CONVFF    0x000080000
#define CONSOLE_CONVCSI   0x000100000

#endif
