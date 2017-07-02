/** consoletty.h
*
*   An interface to raw TTY, i.e. non-blocking, no-echo.
*
*   W.G.J. Langeveld, May 1997
*
**/
#ifndef CONSOLETTY__H
#define CONSOLETTY__H

#include "console.hh"

#define BSD_COMP
#ifdef __BEOS__
#include <termios.h>
#endif
#include <sys/ioctl.h>

class ConsoleTTY : public Console {
public:
   ConsoleTTY();
   virtual ~ConsoleTTY();

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
   int fd;

#ifndef __BEOS__
   sgttyb savedsettings;
#else
   termios savedsettings;
   char ttystring[256];
#endif
   
   ConsoleTTY(const ConsoleTTY &);
   ConsoleTTY &operator=(const ConsoleTTY &);
};

#endif
