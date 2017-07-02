/** consolebe.hh
*
*   An interface to a BView window, i.e. non-blocking, no-echo.
*
*   W.G.J. Langeveld, March 1999
*
**/
#ifndef CONSOLEBE__H
#define CONSOLEBE__H

#ifdef __BEOS__
#include <SupportKit.h>
#include <TextView.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "console.hh"

class LBList : public BList {
public:
   LBList(int32 size = 20) : BList(size) { lock = new BLocker(); return; }
   virtual ~LBList() { delete lock; return; }

   bool Lock(void)   { return(lock->Lock()); }
   void Unlock(void) { lock->Unlock(); return; }

private:
   LBList(const LBList &);
   BList &operator=(const BList &);
   BLocker *lock;
};

class ConsoleBe : public Console, public BTextView {
public:
   ConsoleBe(BRect textframe, const char *name, BRect textrect, int32 followflags,
                                                                        int32 otherflags);
   virtual ~ConsoleBe();

protected:
   virtual int  get_char(void);
   virtual void setraw(long waitflag);
   virtual void setcooked(void);
/*
*   ...Output routines
*/
   virtual long  WriteE(long, long, const char *, long);
   virtual long  WriteT(const char *, long, long);
   virtual long  Write(const char *, long);

private:
   LBList        *list;
   int32          offset1;    // This is the current insert point
   int32          offset2;
   int32          pasting;

   virtual void KeyDown(const char *bytes, int32 numBytes);
   virtual void Paste(BClipboard *);
   virtual void Cut(BClipboard *);
   virtual void InsertText(const char *, int32, int32, const text_run_array *);

   ConsoleBe(const ConsoleBe &);
   ConsoleBe &operator=(const ConsoleBe &);
};

#endif
#endif
