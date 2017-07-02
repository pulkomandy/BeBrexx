/**
*
*   Commands and functions that apply to BeOS
*
**/
#ifdef __BEOS__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#include <OS.h>
#include <Application.h>
#include <Message.h>
#include <Messenger.h>
#include <Roster.h>
#include <Path.h>
#include "PropDump.h"
#include <stdarg.h>
#include "consolebe.hh"
#include "textwindow.h"
#include "consoletty.hh"

#ifdef __cplusplus
extern "C" {
#endif

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"
#include "instruct.h"
#include "rxdefs.h"
#include "bio.h"
#include "be_main.h"

#ifdef __cplusplus
}
#endif

#include "be_cmd.h"


class BRexxApplication : public BApplication {
public:
   BRexxApplication() :
      BApplication("application/x-vnd.VL-brexx"),
      havearg(0),
      win(0),
      fromdesktop(0),
      interrupted(0),
      title(0),
      contty(0),
      conbe(0)
   {
      return;
   };

   ~BRexxApplication()
   {
      if (title) delete [] title;
      if (contty) delete contty;
      // Don't delete conbe: it's deleted by ~TextWindow, which is deleted by ~Bapp.
      return;
   };

   virtual void RefsReceived(BMessage *message);
   virtual void ArgvReceived(int32 argc, char **argv);
   virtual void ReadyToRun(void);
   int          fwrite(char *, int, int, FILE *);
   char        *getline(char *buff, long buffsize);
   void         setprompt(const char *s);
   void         openconsole(const char *, BRect, int32);
   void         closeconsole(void);

   int32        was_interrupted(void)
   {
      return(interrupted);
   };

   void         interrupt(void);

private:
   int32       havearg;
   int32       fromdesktop;
   TextWindow *win;
   ConsoleBe  *conbe;
   int32       interrupted;
   char       *title;
   Console    *contty;
};

int main(int argc, char **argv)
{
   BRexxApplication *my_app = new BRexxApplication();
   my_app->Run();
   delete my_app;
   return(0);
}

void BRexxApplication::setprompt(const char *s)
{
   if (fromdesktop) {
      if (win == NULL) {
         win   = new TextWindow(BRect(30, 30, 300, 200), title);
         conbe = win->GetTextView();
      }
      if (conbe)  conbe->setprompt(s);
   }
   else {
      if (contty == 0) contty = new ConsoleTTY;
      if (contty) contty->setprompt(s);
   }
   return;
};

void BRexxApplication::openconsole(const char *s, BRect frame, int32 useframe)
{
   if (fromdesktop == 0) {
      if (contty) delete contty;
      contty = 0;
      fromdesktop = 1;
   }

   if (s) {
      if (title) delete [] title;
      title = new char[strlen(s) + 1];
      strcpy(title, s);
   }

   if (title == 0) {
      title = new char[6];
      strcpy(title, "BRexx");
   }

   if (win == NULL) {
      win   = new TextWindow(frame, title, useframe);
      conbe = win->GetTextView();
   }
   else {
      if (win->Lock()) {
         win->SetTitle(s);
         if (useframe) {
            win->MoveTo(frame.left, frame.top);
            win->ResizeTo(frame.right - frame.left, frame.bottom - frame.top);
         }
         win->Unlock();
      }
   }
   return;
};

void BRexxApplication::closeconsole(void)
{
   if (fromdesktop) {
      BView *v = 0;
      if (win && win->Lock()) {
         v = win->FindView("scroll_view");
         win->RemoveChild(v);
         delete v;
         win->Quit();
         win = 0;
         conbe = 0;
         fromdesktop = 0;
      }
   }
   return;
};

void BRexxApplication::ArgvReceived(int32 argc, char **argv)
{
/*
*   Protect against multiple RefsReceived for the same instance of this
*   application. Shouldn't happen, since it is multiple launch
*/
   if (havearg) return;
   havearg = 1;

   BRexxMain(argc, argv);
   if ((fromdesktop == 0) || (win == 0)) PostMessage(B_QUIT_REQUESTED);
   return;
}

void BRexxApplication::RefsReceived(BMessage *message)
{
   uint32 type; 
   int32 count; 
   entry_ref ref; 
/*
*   Protect against multiple RefsReceived for the same instance of this
*   application. Shouldn't happen, since it is multiple launch
*/
   if (havearg) return;
   havearg = 1;

   fromdesktop = 1;

   message->GetInfo("refs", &type, &count); 
   if (type != B_REF_TYPE) return;
 
   for (long i = --count; i >= 0; i--) { 
      if (message->FindRef("refs", i, &ref) == B_OK) { 
         BEntry entry(&ref); 
         if (entry.IsFile()) {
            char *argv[3];

            app_info ai;
            GetAppInfo(&ai);
            BEntry aentry(&ai.ref);
            BPath apath;
            aentry.GetPath(&apath);
            const char *ap = apath.Path();
            char *acp = new char[strlen(ap) + 1];
            strcpy(acp, ap);
            argv[0] = acp;

            BPath path;
            entry.GetPath(&path);
            const char *p = path.Path();
            char *cp = new char[strlen(p) + 1];
            strcpy(cp, p);
            argv[1] = cp;

            title = new char[strlen(cp) + 20];
            strcpy(title, "BRexx: ");
            char *ptr = cp + strlen(cp);
            while ((ptr >= cp) && (*ptr != '/')) ptr--;
            strcat(title, ptr == cp ? ptr : ++ptr);

            argv[2] = 0;
            BRexxMain(2, argv);
            delete [] cp;
            delete [] acp;
         } 
      } 
   }
   if ((fromdesktop == 0) || (win == 0)) PostMessage(B_QUIT_REQUESTED);
   return;
}

void BRexxApplication::ReadyToRun(void)
{
/*
*   If we didn't get an Argv or Ref on startup, print usage
*   and quit
*/
   if (havearg == 0) {
      havearg = 1;

      char *argv[2];

      if (getenv("TTY") == NULL) {
         fromdesktop = 1;
         title = new char[20];
         strcpy(title, "BRexx");
      }

      app_info ai;
      GetAppInfo(&ai);
      BEntry aentry(&ai.ref);
      BPath apath;
      aentry.GetPath(&apath);
      const char *ap = apath.Path();
      char *acp = new char[strlen(ap) + 1];
      strcpy(acp, ap);
      argv[0] = acp;

      argv[1] = NULL;

      BRexxMain(1, argv);
      delete [] acp;
      if ((fromdesktop == 0) || (win == 0)) PostMessage(B_QUIT_REQUESTED);
   }
}

char *BRexxApplication::getline(char *buff, long buffsize)
{
   char *c;
   if (fromdesktop) {
      if (win == NULL) {
         win   = new TextWindow(BRect(30, 30, 300, 200), title);
         conbe = win->GetTextView();
      }
      if (conbe) c = conbe->getline(buff, buffsize);
   }
   else {
      if (contty == 0) contty = new ConsoleTTY;
      if (contty) c = contty->getline(buff, buffsize);
      else     c = ::fgets(buff, buffsize, stdin);
   }
   return(c);
}

int BRexxApplication::fwrite(char *s, int size, int len, FILE *fp)
{
   if (fromdesktop) {
      if (win == NULL) {
         win   = new TextWindow(BRect(30, 30, 300, 200), title);
         conbe = win->GetTextView();
      }
      if (win && win->Lock()) {
         len = conbe->putline(s, len);
         win->Unlock();
      }
      else {
         len = 0;
      }
   }
   else {
      if (contty == 0) contty = new ConsoleTTY;
      if (contty) len = contty->putline(s, (long) len);
      else        len = ::fwrite(s, size, len, fp);
   }
   return(len);
}

void BRexxApplication::interrupt(void)
{
   interrupted = 1;
   return;
}

void be_interrupt(void)
{
   ((BRexxApplication *) be_app)->interrupt();
   return;
}

int be_was_interrupted(void)
{
   return(((BRexxApplication *) be_app)->was_interrupted());
}

char *bgetline(char *buff, long buffsize)
{
   return(((BRexxApplication *) be_app)->getline(buff, buffsize));
}

#ifdef AMIGA
struct mydata {char *ptr; int cnt;};

static void __asm __saveds wlputc(register __d0 char c, register __a3 struct mydata *md)
{
   md->ptr[md->cnt++] = c;
   return;
}
#endif

int bprintf(char *fmt, ...)
{
   char buf[16384];
   va_list args;

#ifdef AMIGA
   struct mydata md;
   md.ptr = buf;
   md.cnt = 0;
#endif
   va_start(args, fmt);
#ifdef AMIGA
   RawDoFmt(fmt, args, wlputc, &md);
#else
   vsprintf(buf, fmt, args);
#endif
   va_end(args);

   return(((BRexxApplication *) be_app)->fwrite(buf, 1, strlen(buf), stdout));
}

int beprintf(char *fmt, ...)
{
   char buf[16384];
   va_list args;

#ifdef AMIGA
   struct mydata md;
   md.ptr = buf;
   md.cnt = 0;
#endif
   va_start(args, fmt);
#ifdef AMIGA
   RawDoFmt(fmt, args, wlputc, &md);
#else
   vsprintf(buf, fmt, args);
#endif
   va_end(args);

   return(((BRexxApplication *) be_app)->fwrite(buf, 1, strlen(buf), stderr));
}

int bputs(char *s)
{
   int len = ((BRexxApplication *) be_app)->fwrite(s, 1, strlen(s), stdout);
   return(len + bputchar('\n'));
}

int bputchar(char c)
{
   return(((BRexxApplication *) be_app)->fwrite(&c, 1, 1, stdout));
}


#ifdef __cplusplus
extern "C" {
#endif
void R_setprompt(args *arg);
#ifdef __cplusplus
}
#endif

void R_setprompt(args *arg)
{
   char *ptr = NULL;

   if (!IN_RANGE(0,ARGN,1)) error(ERR_INCORRECT_CALL);

   if (exist(1)) {
      L2str(&ARG1);
      ASCIIZ(ARG1);
      ptr = STR(ARG1);
   }
   else {
      ptr = "";
   }

   ((BRexxApplication *) be_app)->setprompt(ptr);
   
   return;
}

#ifdef __cplusplus
extern "C" {
#endif
void R_openconsole(args *arg);
void R_closeconsole(args *arg);
#ifdef __cplusplus
}
#endif

void R_openconsole(args *arg)
{
   char *ptr = NULL;
   BRect b(50, 50, 300, 200);
   int32 useframe = 0;

   if (!IN_RANGE(0,ARGN,5)) error(ERR_INCORRECT_CALL);

   if (exist(1)) {
      L2str(&ARG1);
      ASCIIZ(ARG1);
      ptr = STR(ARG1);
   }
   if (exist(2)) {
      must_exist(3);
      must_exist(4);
      must_exist(5);
      L2str(&ARG2);
      L2str(&ARG3);
      L2str(&ARG4);
      L2str(&ARG5);
      ASCIIZ(ARG2);
      ASCIIZ(ARG3);
      ASCIIZ(ARG4);
      ASCIIZ(ARG5);
      b.left   = atof(STR(ARG2));
      b.top    = atof(STR(ARG3));
      b.right  = b.left + atof(STR(ARG4));
      b.bottom = b.top + atof(STR(ARG5));
      useframe = 1;
   }

   ((BRexxApplication *) be_app)->openconsole(ptr, b, useframe);
   
   return;
}

void R_closeconsole(args *arg)
{
   ((BRexxApplication *) be_app)->closeconsole();
   
   return;
}

#endif
