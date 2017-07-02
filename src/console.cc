/** console.cpp
*
*   Source for a simple console with command history and editing.
*
*   W.G.J. Langeveld, 1992, 1993, 1998
*
**/
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "console.hh"
/*
*   A few defines for the escape parser
*/
#define ESC 0x1B
#define CAN 0x18
#define CSI 0x9B
#define SS3 0x8F

/**
*
*   The console itself.
*
**/
Console::~Console()
{
   delete [] undobuf;
   delete [] cmdline;
   delete prompt;
   return;
}

Console::Console()
{
   undobuf = NULL;
   cmdline = NULL;
   flags   = CONSOLE_CONVRV | CONSOLE_CONVLF | CONSOLE_CONVFF;
   cursor  = 0;
   hold    = havecr = havelf = havelines = 0;
   cmdlinesize = 0;
   cmdlen  = 0;
   CurParse = &ParseChar;
   prompt  = NULL;
   return;
}


char *Console::getline(char *buffer, long bufsize)
{
   long c, keepgoing = 1, i;
   char *result = buffer, *ptr;

   setraw(1);

   while (keepgoing) {
      keepgoing = 0;

      if (prompt) {
         WriteE(0, 0, prompt, strlen(prompt));
      }

      while (!havelines) {
         c = get_char();
         if (c == -1) return(NULL);
         (this->*CurParse)(c);
      }

      history.add(cmdline, cmdlen);
      history.setpastend();
/*
*   Allow inline system commands
*/
      if (cmdlen > 0) {
         ptr = cmdline;
         for (i = 0; i < cmdlen; i++) {
            if ((*ptr != ' ') && (*ptr != '\t')) {
               if (*ptr == '!') {    /* System command! */
                  cmdline[cmdlen] = '\0';

                  setcooked();
                  system(ptr + 1);
                  setraw(1);

                  havelines = 0;
                  cursor    = 0;
                  cmdlen    = 0;
                  keepgoing = 1;
               }
            }        
            ptr++;
         }
      }
   }

   *buffer = '\0';
   if (cmdlen + 1 > bufsize) result = NULL;
   if (cmdlen == -1)         result = NULL;

   if (result) {
      memmove(buffer, cmdline, cmdlen);
      buffer[cmdlen] = '\0';
   }

   havelines = 0;
   cursor    = 0;
   cmdlen    = 0;

   setcooked();

   return(result);
}

long Console::putline(const char *buffer, long bufsize)
{
   return(WriteE(0, 0, buffer, bufsize));
}

void Console::setprompt(const char *p)
{
   if (prompt) delete [] prompt;
   prompt = new char[strlen(p) + 1];
   strcpy(prompt, p);
   return;
}

void Console::ResizeBuffers(long n)
{
   long newsize;

   if (undobuf == NULL) {
      newsize = 256;
      while (newsize < n) newsize *= 2;
      cmdlinesize = newsize;
      undobuf = new char [cmdlinesize];
      cmdline = new char [cmdlinesize];
      undobuf[0] = '\0';
      cmdline[0] = '\0';
   }
   else {
      newsize = cmdlinesize;
   }

   while  (n + 1 > cmdlinesize) newsize *= 2;

   if (newsize != cmdlinesize) {
      //printf("Resizing\r\n");
      cmdlinesize = newsize;
      char *ub = new char [cmdlinesize];
      char *cl = new char [cmdlinesize];
      memmove(ub, undobuf, strlen(undobuf) + 1);
      memmove(cl, cmdline, strlen(cmdline) + 1);
      delete [] undobuf;
      delete [] cmdline;
      undobuf = ub;
      cmdline = cl;
   }
   return;
} 

/**
*
*   Handle arrow key. These only get called in "cooked" mode.
*
**/
void Console::DoKeyUp(void)
{
   long oldlen;
   long status = history.status();

   char *histline = history.prev();
   if (histline == NULL) return;
/*
*   Copy current command line to undo buffer
*/
   if (cmdlen && (status == CONHIST_NOTINUSE)) {
      ResizeBuffers(cmdlen);
      memmove(undobuf, cmdline, cmdlen);
      undobuf[cmdlen] = '\0';
   }
   oldlen = cmdlen;

   cmdlen = strlen(histline);
   memmove(cmdline, histline, cmdlen);

   if (oldlen > cmdlen) {
      memset(cmdline + cmdlen, ' ', oldlen - cmdlen);
   }
   else {
      oldlen = cmdlen;
   }      

   WriteE(-cursor, cmdlen - oldlen, cmdline, oldlen);

   cursor = cmdlen;
   return;
}

void Console::DoKeyDown(void)
{
   long oldlen;
   long status = history.status();
   char *histline = history.next();
/*
*   Copy current command line to undo buffer
*/
   if (cmdlen && (status == CONHIST_NOTINUSE)) {
      ResizeBuffers(cmdlen);
      memmove(undobuf, cmdline, cmdlen);
      undobuf[cmdlen] = '\0';
   }
   oldlen = cmdlen;
/*
*   Find next command
*/
   if (histline) {
      cmdlen  = strlen(histline);
      memmove(cmdline, histline, cmdlen);
   }
   else {
      cmdlen = 0;
      history.setpastend();
   }

   if (oldlen > cmdlen) {
      memset(cmdline + cmdlen, ' ', oldlen - cmdlen);
   }
   else {
      oldlen = cmdlen;
   }      

   WriteE(-cursor, cmdlen - oldlen, cmdline, oldlen);

   cursor = cmdlen;
   return;
}

void Console::DoKeyLeft(void)
{
   if (cursor > 0) {
      cursor--;
      WriteE(0, -1, NULL, 0);
   }
   return;
}

void Console::DoKeyRight(void)
{
   if (cursor < cmdlen) {
      cursor++;
      WriteE(0, 1, NULL, 0);
   }
   return;
}

/**
*
*   Tell user a bad escape happened. "Cooked" only.
*
**/
void Console::InvalidEsc(void)
{
   VTBell();
   return;
}


/**
*
*   Handles characters
*
**/
void Console::ParseChar(long c)
{
   char ec;
   long size, premove;
   //printf("ParseChar\r\n");
/*
*   In raw mode just add everything to the ring to the shell
*/
   if (flags & CONSOLE_RAW) {
      if ((flags & CONSOLE_BSDEL) && ((flags & CONSOLE_BINARY) == 0)) {
         if      (c == 127)  c = '\b';
         else if (c == '\b') c = 127;
      }

      ResizeBuffers(cmdlen + 1);
      cmdline[cmdlen] = c & 0xFF;
      cmdlen++;
      havelines++;
      return;
   }
   else {
/*
*   Convert cr/lf and lf/cr combinations to simple lf's. If there are
*   *only* lfs they are left alone completely, if there are *only*
*   crs, the crs will be turned into lfs.
*/
      if (c == 13) {
         if (havelf) {
            havelf = 0;
            return;
         }
         else {
            c = 10;
            havecr = 1;
         }
      }
      else if (c == 10) {
         if (havecr) {
            havecr = 0;
            return;
         }
         else {
            havelf = 1;
         }
      }
      else {
         havelf = havecr = 0;
      }

      if (flags & CONSOLE_BSDEL) {
         if      (c == 127)  c = '\b';
         else if (c == '\b') c = 127;
      }

      switch (c) {
         case 0    : /* null - ignore */
            break;
         case 1    : /* ctrl-A: toggle insert/overstrike */
            flags ^= CONSOLE_OVERSTRIKE;
            break;
         case 2    : /* ctrl-B: emacs "back" */
            DoKeyLeft();
            break;
         case 3    : /* ctrl-C: quit */
/*
*   This will force signify an "exit requested"
*/
            cmdlen    = -1;
            cursor    = 0;
            havelines = 1;
            WriteE(0, 0, "*** BREAK ***\n", -1);
            break;
         case 4    : /* ctrl-D: emacs "delete", erase history */
            history.clear();
            break;
         case 6    : /* ctrl-F: emacs "forward" */
            DoKeyRight();
            break;
         case 7    : /* ctrl-G: echo the bell */
            VTBell();
            break;
         case '\b' : /* ctrl-H: backspace */
            if ((cursor > 0) && (cursor <= cmdlen)) {
               size = cmdlen - cursor + 1;
               Delete(cursor - 1, 1);
               cursor--;

               WriteE(-1, -size, cmdline + cursor, size);
            }
            break;
         case '\t' : { /* ctrl-I, tab: match history */
            char *ptr = history.find(cmdline, cmdlen);
            if (ptr) {
               long oldlen;
               long status = history.status();
/*
*   Copy current command line to undo buffer
*/
               if (cmdlen && (status == CONHIST_NOTINUSE)) {
                  ResizeBuffers(cmdlen);
                  memmove(undobuf, cmdline, cmdlen);
                  undobuf[cmdlen] = '\0';
               }
               oldlen = cmdlen;

               cmdlen = strlen(ptr);
               memmove(cmdline, ptr, cmdlen);

               if (oldlen > cmdlen) {
                  memset(cmdline + cmdlen, ' ', oldlen - cmdlen);
               }
               else {
                  oldlen = cmdlen;
               }      

               WriteE(-cursor, cmdlen - oldlen, cmdline, oldlen);

               cursor = cmdlen;
            }
            else {
               VTBell();
            }
            break;
         }
         case '\n' : /* ctrl-J, lf */
         case '\r' : /* ctrl-M, cr */
            ResizeBuffers(cmdlen + 1);
            havelines++;
            WriteE(0, 0, "\n", 1);
            break;
         case 14 : /* ctrl-N: emacs next line */
            DoKeyDown();
            break;
         case 16 : /* ctrl-P: emacs previous line */
            DoKeyUp();
            break;
//         case 17 : /* ctrl-Q */
//            hold = 0;
//            break;
         case 18 : /* ctrl-R: review console commands */
            ShowUsage();
            size = cmdlen;
            WriteE(0, 0, "\n", 1);
            WriteE(0, 0, prompt, -1);
            WriteE(0, cursor - cmdlen, cmdline, size);
            break;
//         case 19 : /* ctrl-S */
//            hold = 1;
//            break;
         case 20 : /* ctrl-T */
            if (cursor > 1) {
               ec = cmdline[cursor - 1];
               cmdline[cursor - 1] = cmdline[cursor - 2];
               cmdline[cursor - 2] = ec;
               WriteE(-2, 0, &cmdline[cursor - 2], 2);
            }
            break;
         case 21 : /* ctrl-U: delete to beginning */
            if (cursor > 0) {
               size = cmdlen;
               Delete(0, cursor);

               WriteE(-cursor, -size, cmdline, size);

               cursor = 0;
            }
            break;
         case 22 : /* ctrl-V: delete to end */
            size = cmdlen - cursor;
            if (size) {
               memset(cmdline + cursor, ' ', size);

               WriteE(0, -size, cmdline + cursor, size);

               cmdlen = cursor;
            }
            break;
         case 23 : /* ctrl-W: redisplay line */
            size = cmdlen;
            WriteE(0, 0, "\n", 1);
            WriteE(0, cursor - cmdlen, cmdline, size);
            break;
         case 24 : /* ctrl-X: delete line */
            if (cursor > 0) premove = -cursor;
            else            premove = 0;

            size = cmdlen;

            if (size) {
               memset(cmdline, ' ', size);

               WriteE(premove, -size, cmdline, size);

               cursor = cmdlen = 0;
            }
            break;
         case ESC :
            CurParse = &ParseESC;
            pars[0]  = 0;
            parcount = 0;
            break;
         case 29  : /* ctrl-]: toggle EOL    */
            if (cmdlen != 0) {
               if (cursor == cmdlen) {
                  WriteE(0, -cursor, NULL, 0);
                  cursor = 0;
               }
               else {
                  WriteE(0, cmdlen - cursor, NULL, 0);
                  cursor = cmdlen;
               }
            }
            break;
         case 30  : /* ctrl-^: force insert mode */
            flags &= ~CONSOLE_OVERSTRIKE;
            break;
         case 31   : /* ctrl--: recall undobuf */
            size = cmdlen;
/*
*   Find previous command
*/
            cmdlen  = strlen(undobuf);
            if (cmdlen) {
               memmove(cmdline, undobuf, cmdlen);
            }

            if (size > cmdlen) {
               memset(cmdline + cmdlen, ' ', size - cmdlen);
            }
            else {
               size = cmdlen;
            }      

            WriteE(-cursor, cmdlen - size, cmdline, size);

            cursor = cmdlen;
            history.setpastend();
            break;
         case 127  :  /* DEL: delete under cursor */
            if (cursor < cmdlen) {
               size = cmdlen - cursor;
               Delete(cursor, 1);

               WriteE(0, -size, cmdline + cursor, size);
            }
            break;
         case CSI :
            CurParse = &ParseCSI;
            pars[0]  = 0;
            parcount = 0;
            break;
         case SS3 :
            CurParse = &ParseKeypad;
            break;
         default   :
            size = cmdlen - cursor;
            if ((flags & CONSOLE_OVERSTRIKE) || (size == 0)) {
               ResizeBuffers(cursor + 1);

               cmdline[cursor] = c;
               cursor++;
               if (cursor > cmdlen) cmdlen = cursor;

               ec = (char) c;
               WriteE(0, 0, &ec, 1);
            }
            else {
               ResizeBuffers(cmdlen + 1);

               Insert(cursor, c);

               cursor++;

               WriteE(0, -size, cmdline + cursor - 1, size + 1);
            }
            break;
      }
   }

   return;
}

/**
*
*   Parse an escape sequence.
*
**/
void Console::ParseESC(long c)
{
   //printf("ParseESC\r\n");
   switch (c) {
      case ESC :
         CurParse   = &ParseESC;
         pars[0]    = 0;
         parcount   = 0;
         break;
      case CAN :
         CurParse   = &ParseChar;
         break;
      case '[' :
         CurParse   = &ParseCSI;
         break;
      case 'O' :
         CurParse   = &ParseKeypad;
         break;
      default:
         ParseFinalESC(c);
         break;
   }
   return;
}


/**
*
*   Parse a control sequence.
*
**/
void Console::ParseCSI(long c)
{
   //printf("ParseCSI\r\n");
   switch (c) {
      case ';' :
         break;
      case CAN :
         CurParse = &ParseChar;
         break;
      case ESC :
         CurParse = &ParseESC;
         pars[0]  = 0;
         parcount = 0;
         break;
      default:
         if ((c <= '9') && (c >= '0')) {
            CurParse = &ParseLong;
            ParseLong(c);
         }
         else {
            ParseFinalCSI(c);
         }
         break;
   }
   return;
}


/**
*
*   Parse a numeric field
*
**/
void Console::ParseLong(long c)
{
   //printf("ParseLong\r\n");
   if (c == ESC) {
      CurParse = &ParseESC;
      pars[0] = 0;
      parcount = 0;
   }
   else if (c == CAN) {
      CurParse = &ParseChar;
   }
   else if (c < '0' || c > '9') {
      parcount++;
      pars[parcount] = 0;
      CurParse = &ParseCSI;
      ParseCSI(c);
   }
   else {
      pars[parcount] *= 10;
      pars[parcount] += c - '0';
   }
   return;
}


/**
*
*   After final character, interpret the escape sequence
*
**/
void Console::ParseFinalESC(long c)
{
   //printf("ParseFinalESC\r\n");
   switch (c) {
      case 'A' :
         DoKeyUp();
         break;
      case 'B' :
         DoKeyDown();
         break;
      case 'C' :
         DoKeyRight();
         break;
      case 'D' :
         DoKeyLeft();
         break;
      default :
         InvalidEsc();
         break;
   }

   CurParse = &ParseChar;
   return;
}


/**
*
*   After final character, interpret the control sequence
*
**/
void Console::ParseFinalCSI(long c)
{
   //printf("ParseFinalCSI\r\n");
   switch (c) {
      case 'A' :
         DoKeyUp();
         break;
      case 'B' :
         DoKeyDown();
         break;
      case 'C' :
         DoKeyRight();
         break;
      case 'D' :
         DoKeyLeft();
         break;
      default :
         InvalidEsc();
         break;
   }

   CurParse = &ParseChar;
   return;
}


/**
*
*   After final character, interpret the escape sequence
*
**/
void Console::ParseKeypad(long c)
{
   //printf("ParseKeypad\r\n");
   switch (c) {
      case 'A' :
         DoKeyUp();
         break;
      case 'B' :
         DoKeyDown();
         break;
      case 'C' :
         DoKeyRight();
         break;
      case 'D' :
         DoKeyLeft();
         break;
      default :
         InvalidEsc();
         break;
   }

   CurParse   = &ParseChar;
   return;
}

/**
*
*   Write a bell character to the terminal
*
**/
void Console::VTBell(void)
{
   WriteE(0, 0, "\007", 1);
   return;
}


/**
*
*   Erase the VT100 screen
*
**/
void Console::VTEraseScreen(void)
{
   WriteE(0, 0, "\033[2J", 4);
   return;
}

/**
*
*   Delete n characters in cmdline at position pos.
*
**/
long Console::Delete(long pos, long n)
{
   long size, oldn;

   oldn = n;
   if (pos < cmdlen) {
      size = cmdlen - pos - n;
      if (n > cmdlen - pos) {
         n = cmdlen - pos;
         size = 0;
      }
      if (size) memmove(cmdline + pos, cmdline + pos + n, size);
      cmdlen -= n;
      memset(cmdline + cmdlen, ' ', oldn);
   }

   return(cmdlen);
}
   
long Console::Insert(long pos, char c)
{
   long size;
/*
*   pad with spaces
*/
   if (pos > cmdlen) {
      //printf("Insert: padding\r\n");
      memset(cmdline + cmdlen, ' ', cmdlen - pos);
      cmdlen = pos;
   }

   if (pos != cmdlen) {
      //printf("Insert: moving\r\n");
      memmove(cmdline + pos + 1, cmdline + pos, cmdlen - pos);
   }

   //printf("Insert: inserting\r\n");
   *(cmdline + pos) = c;

   cmdlen++;

   //printf("Insert: done\r\n");
   return(cmdlen);
}

void Console::ShowUsage(void)
{
   static char *usage[] = {"\n",
      "ctrl-A: toggle insert/overstrike        ctrl-N: next history line (same as down arrow)\n",
      "ctrl-B: back (same as left arrow)       ctrl-O:\n",
      "ctrl-C: quit                            ctrl-P: previous history line (same as up arrow)\n",
      "ctrl-D: clear history buffer            ctrl-Q:\n",
      "ctrl-E:                                 ctrl-R: show this list\n",
      "ctrl-F: forward (same as right arrow)   ctrl-S:\n",
      "ctrl-G: terminal bell                   ctrl-T: switch previous two characters\n",
      "ctrl-H: delete char before cursor       ctrl-U: delete to beginning of line\n",
      "ctrl-I:                                 ctrl-V: delete to end of line\n",
      "ctrl-J: newline (end of command)        ctrl-W: redisplay line\n",
      "ctrl-K:                                 ctrl-X: delete line\n",
      "ctrl-L:                                 ctrl-Y: (Unix suspend)\n",
      "ctrl-M: return (end of command)         ctrl-Z:\n",
      "\n",
      "ctrl-]: toggle beginning/end of line\n",
      "ctrl-^: force insert mode\n",
      "ctrl--: recall \"undobuffer\"\n",
      "<DEL> : delete char under cursor\n",
      "\n",
      "<enter>: end of command\n",
      "\n",
      "<up-arrow>    : retrieve previous history line\n",
      "<down-arrow>  : retrieve next history line\n",
      "<left-arrow>  : move left\n",
      "<right-arrow> : move right\n",
      "\n",
      "Prefixing a line with a ! character will send it to the system\n",
      NULL
   };

   long i = 0;
   while (usage[i]) WriteE(0, 0, usage[i++], -1);

   return;
}
