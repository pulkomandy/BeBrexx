/** rawtty.cpp
*
*   RAW terminal I/O, i.e. no echo, non-blocking. UNIX version.
*
*   W.G.J. Langeveld, May 1997.
*
**/
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "consoletty.hh"

#ifdef TESTRAWTTY
void main(int argc, char **argv)
{
   int c;
   ConsoleTTY *tty;

   tty = new ConsoleTTY();
   if (tty == NULL) {
      printf("Couldn't get a raw tty\r\n");
      exit(0);
   }

   tty->setraw(argc - 1);

   while (1) {
      c = tty->get();
      if (c == -1) {
         printf("Got nothing\r\n");
         sleep(1);
      }
      else {
         if (((c & 0x7F) < 32) || ((c & 0x7F) == 127)) printf("-> 0x%02X\r\n", c);
         else                                          printf("-> %c\r\n", c);
         if (c == 'q' || c == 'Q') goto cleanup;
      }
   }

cleanup:
   if (tty) delete tty;
   exit(0);
   return;
}      
#else

ConsoleTTY::ConsoleTTY()
{
   cooked = 1;
   return;
}

void ConsoleTTY::setraw(long wait)
{
   if (cooked == 0) return;

#ifdef __HAIKU__
   waitflag = wait;
   struct termios termios;

   if (waitflag == 0) {
      char *ptr = getenv("TTY");
      if (ptr) {
         strcpy(ttystring, ptr);
         close(0);
         fd = open(ttystring, O_RDONLY | O_NONBLOCK);
      }
      else {
         fd = -1;
      }

      if (fd < 0) {
         printf("Couldn't get tty\r\n");
         return;
      }
   }
   else {
      fd = 0;
   }

   tcgetattr(fd, &termios);

   savedsettings = termios;

   termios.c_iflag &= ~(IXON | IXOFF | ICRNL | INLCR | ISTRIP | IEXTEN);
   termios.c_iflag |= IGNBRK;

   termios.c_oflag &= ~OPOST;

   termios.c_lflag &= ~(ICANON | ECHO);
   termios.c_lflag |= ISIG;

   termios.c_cflag &= ~PARENB;
   termios.c_cflag |= CS8;

   termios.c_cc[VTIME] = 0;
   termios.c_cc[VMIN]  = 1;

   termios.c_cc[VSUSP] = _POSIX_VDISABLE;
   termios.c_cc[VQUIT] = _POSIX_VDISABLE;
   termios.c_cc[VKILL] = _POSIX_VDISABLE;
   termios.c_cc[VINTR] = _POSIX_VDISABLE;

   termios.c_cc[VQUIT] = termios.c_cc[VINTR] = '\\'-'@';

   tcsetattr(fd, TCSADRAIN, &termios);
#else
   int flag;
   struct sgttyb s;

   fd = open("/dev/tty", O_RDONLY);
   if (fd < 0) return;

   ioctl(fd, TIOCGETP, &s);

   savedsettings = s;

   s.sg_flags |= CBREAK;
   s.sg_flags &= ~(ECHO|XTABS);

   ioctl(fd, TIOCSETP, &s);

   waitflag = wait;
   if (waitflag == 0) {
      flag = 1;
      ioctl(fd, FIONBIO, &flag);
   }
#endif
   cooked = 0;
   return;
}

ConsoleTTY::~ConsoleTTY()
{
   setcooked();
   return;
}

void ConsoleTTY::setcooked(void)
{
   if (cooked) return;
#ifdef __HAIKU__
   if (fd >= 0) {
      tcsetattr(fd, TCSANOW, &savedsettings);
      if (waitflag == 0) {
         close(fd);
         open(ttystring, O_RDONLY);
      }
   }
#else
   int flag;
   if (fd >= 0) {
      if (waitflag == 0) {
         flag = 0;
         ioctl(fd, FIONBIO, &flag);
      }
      ioctl(fd, TIOCSETN, &savedsettings);
      close(fd);   
   }
#endif
   cooked = 1;
   return;
}

int ConsoleTTY::get_char(void)
{
   char c;
   int n, nbytes;

   if (cooked) return((long) getchar());

#ifndef __BEOS__
   if (waitflag == 0) {
      ioctl(fd, FIONREAD, &nbytes);
      if (nbytes <= 0) return(-1);
   }
#endif

   if ((n = read(fd, &c, sizeof(char))) <= 0) return(-1);

   return((int) c);
}

/**
*
*   Write to screen in ECHO mode (typically while writing a command line)
*   This function will turn of "raw" mode during each write - only "formatted"
*   stuff should use this function.
*
**/
#define HSCMDSIZ 512
long ConsoleTTY::WriteE(long b, long a, const char *str, long n)
{
   long l;
   char buff[HSCMDSIZ + 20], *ptr;

   ptr = buff;

   if ((n == -1) && str) n = strlen(str);

   if (b) {
      //printf("WriteE: before\r\n");
      l = b;
      if (b < 0) l = -b;

      if ((l < 3) && (b < 0)) {
	 while (l--) *ptr++ = '\b';
      }
      else {
	 if (l == 1) sprintf(ptr, "\033[%c", (b > 0) ? 'C' : 'D');
	 else	     sprintf(ptr, "\033[%d%c", l, (b > 0) ? 'C' : 'D');
	 ptr += strlen(ptr);
      }
   }

   //printf("%x %d\r\n", str, n);
   if (str && n) {
      //printf("WriteE: string\r\n");
      while (n--) {
	 *ptr++ = *str++;
	 if (ptr == (buff + HSCMDSIZ + 10)) {
            //printf("WriteE: WriteT string\r\n");
	    WriteT(buff, HSCMDSIZ + 10, flags & ~(CONSOLE_CONVFF | CONSOLE_RAW));
	    ptr = buff;
	 }
      }
   }

   if (a) {
      //printf("WriteE: after (%d) ptr %08X buff %8X\r\n", a, ptr, buff);
      l = a;
      if (a < 0) l = -a;

      if ((l < 3) && (a < 0)) {
	 while (l--) *ptr++ = '\b';
      }
      else {
         //printf("WriteE: sprintf\r\n");
	 if (l == 1) sprintf(ptr, "\033[%c", (a > 0) ? 'C' : 'D');
	 else	     sprintf(ptr, "\033[%d%c", l, (a > 0) ? 'C' : 'D');
         //printf("WriteE: pointer now %08X\r\n", ptr);
         //printf("WriteE: update ptr (%x %x %x %x %x)\r\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
	 ptr += strlen(ptr);
         //printf("WriteE: ptr now %08x\r\n", ptr);
      }
   }

   if (ptr != buff) {
      WriteT(buff, ptr - buff, flags & ~(CONSOLE_CONVFF | CONSOLE_RAW));
   }

   return(n);
}

/**
*
*   Write a string to a socket and do various conversions...
*
**/
long ConsoleTTY::WriteT(const char *str, long n, long newflags)
{
   char temp[540];
   register char *ptr;
   register long c;
/*
*   Check for linefeeds and other stuff, convert to VT100 sequence
*/
   //printf("WriteT: start\r\n");
   ptr = temp;
   while (n > 0) {
      c = *str++;

      if ((newflags & CONSOLE_BINARY) && (newflags & CONSOLE_RAW)) {
	 *ptr++ = c;
      }
      else {
	 if (c < 32) {
	    switch (c) {
	       case 0	 :
		  break;
	       case  7	 :
	       case '\b' :
	       case 27	 :
		  *ptr++ = c;
		  break;
	       case '\n' :
		  if (newflags & CONSOLE_CONVLF) *ptr++ = '\r';
		  *ptr++ = c;
		  break;
	       case '\f' :
		  if (newflags & CONSOLE_CONVFF) {
		     *ptr++ = '\033';
		     *ptr++ = '[';
		     *ptr++ = '2';
		     *ptr++ = 'J';
		     break;
		  }
	       /* Not converted, fall through */
	       default :
		  if (newflags & CONSOLE_CONVRV) {
		     *ptr++ = '\033';
		     *ptr++ = '[';
		     *ptr++ = '7';
		     *ptr++ = 'm';
		     *ptr++ = c + 64;
		     *ptr++ =  27;
		     *ptr++ = '[';
		     *ptr++ = '0';
		     *ptr++ = 'm';
		  }
		  else {
		     *ptr++ = c;
		  }
		  break;
	    }
	 }
	 else {
	    switch (c) {
	       case 0x9b :  /* CSI */
		  if (newflags & CONSOLE_CONVCSI) {
		     *ptr++ = 0x1b;
		     *ptr++ = '[';
		  }
		  else {
		     *ptr++ = c;
		  }
		  break;
	       default :
		  *ptr++ = c;
		  break;
	    }
	 }
      }

      if (ptr >= (temp + 512)) {
	 Write(temp, ptr - temp);
	 ptr = temp;
      }
      n--;
   }

   long l = 0;
   if (ptr - temp)  l = Write(temp, ptr - temp);
   //printf("WriteT: done\r\n");

   return(0);
}

/**
*
*   Write a string to a socket
*
**/
long ConsoleTTY::Write(const char *str, long n)
{
   long len = fwrite(str, 1, n, stdout);
   fflush(stdout);
   return(len);
}


#endif
