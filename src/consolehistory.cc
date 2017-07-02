/** consolehistory.cc
*
*   Simple console history
*
*   W.G.J. Langeveld, march 1999
*
**/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "consolehistory.hh"

#define HISTORY_STARTSIZE 32

/**
*
*   Constructors
*
**/
ConsoleHistory::~ConsoleHistory()
{
   for (long i = 0; i < num; i++) delete [] list[i];
   delete [] list;
   return;
}

ConsoleHistory::ConsoleHistory()
{
   allocsize  = HISTORY_STARTSIZE;
   list       = new char * [allocsize];
   num        = 0;
   index      = 0;
   return;
}

/**
*
*   Public functions
*
**/
char *ConsoleHistory::find(long id) const
{
   if   ((id > num) || (id < 1)) return(0);
   else                          return(list[id - 1]);
}

char *ConsoleHistory::next(void) const
{
   if (index <= num) ((ConsoleHistory *) this)->index++;
   return(find(index));
}

char *ConsoleHistory::prev(void) const
{
   if (index > 1) ((ConsoleHistory *) this)->index--;
   return(find(index));
}

char *ConsoleHistory::first(void) const
{
   ((ConsoleHistory *) this)->index = 1;
   return(find(index));
}

char *ConsoleHistory::last(void) const
{
   ((ConsoleHistory *) this)->index = num;
   return(find(index));
}

void ConsoleHistory::setpastend(void) const
{
   ((ConsoleHistory *) this)->index = num + 1;
   return;
}

char *ConsoleHistory::find(const char *s, long l) const
{
   long i;
   for (i = num - 1; i >= 0; i--) {
      if (strncmp(s, list[i], l) == 0) {
         ((ConsoleHistory *) this)->index = i + 1;
         return(list[i]);
      }
   }
   return(NULL);
}

long ConsoleHistory::size(void) const
{
   return(num);
}

long ConsoleHistory::status(void) const
{
   if ((index > 0) && (index <= num)) return(0);
   return(CONHIST_NOTINUSE);
}

void ConsoleHistory::clear(void)
{
   for (long i = 0; i < num; i++) delete [] list[i];
   num = 1;
   return;
}

/**
*
*   Add a string.
*
**/
void ConsoleHistory::add(const char *s, long length)
{
   if (length <= 0) return;

   if (s) {
/*
*   Only add lines if they're not the same as the last one.
*/
      if (num > 1) {
         if (strncmp(s, list[num - 1], length) == 0) return;
      }

      char *ss = new char [length + 1];
      memmove(ss, s, length);
      ss[length] = '\0';
      list[num]    = ss;

      num++;

      if (num == allocsize) {
         char **v = new char * [allocsize *= 2];
         long i;
         for (i = 0; i < num; i++) v[i] = list[i];
         delete [] list;
         list = v;
      }
   }
   return;
}

