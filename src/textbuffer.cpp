/** textbuffer.cpp
*
*   Simple text accumulator
*
**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "textbuffer.h"

#define TBSTARTSIZE 512
#define TBBUFSIZE 16384

TextBuffer::TextBuffer(const char *s)
{
   _maxsize = TBSTARTSIZE;

   _text    = new char[_maxsize];
   _text[0] = '\0';

   _size    = 0;

   _name    = 0;
   name(s);

   _buf     = new char[TBBUFSIZE];
   _buf[0]  = '\0';

   return;
}

TextBuffer::TextBuffer(const TextBuffer &tb)
{
   _maxsize = tb._maxsize;

   _text    = new char[tb._maxsize];
   strcpy(_text, tb._text);

   _size    = tb._size;

   _name    = 0;
   name(tb._name);

   _buf     = new char[TBBUFSIZE];
   _buf[0]  = '\0';

   return;
}

TextBuffer &TextBuffer::operator=(const TextBuffer &tb)
{
   if (&tb == this) return(*this);

   _maxsize = tb._maxsize;

   if (_text) delete [] _text;
   _text    = new char[tb._maxsize];
   strcpy(_text, tb._text);

   _size    = tb._size;

   name(tb._name);

   _buf[0] = '\0';

   return(*this);
}

TextBuffer::~TextBuffer(void)
{
   delete [] _text;
   delete [] _name;
   delete [] _buf;
   return;
}

void TextBuffer::add(char *s)
{
   if (s == 0) return;

   double omax = _maxsize;

   long l = strlen(s);
   long tot = l + _size;

   while (tot >= _maxsize) _maxsize += _maxsize;

   if (_maxsize != omax) {
      char *t = new char[_maxsize];
      strcpy(t, _text);
      delete [] _text;
      _text = t;
   }

   strcat(_text, s);
   _size += l;

   return;
}

void TextBuffer::add(char c)
{
   double omax = _maxsize;

   long tot = 1 + _size;

   while (tot >= _maxsize) _maxsize += _maxsize;

   if (_maxsize != omax) {
      char *t = new char[_maxsize];
      strcpy(t, _text);
      delete [] _text;
      _text = t;
   }

   _text[_size] = c;
   _size++;
   _text[_size] = '\0';

   return;
}

void TextBuffer::name(const char *s)
{
   if (_name) {
      delete [] _name;
      _name = 0;
   }
   if (s) {
      _name = new char[strlen(s) + 1];
      strcpy(_name, s);
   }
   return;
}

char *TextBuffer::name(void)
{
   return(_name);
}

char *TextBuffer::text(void)
{
   if (_text) return(_text);
   else       return("");
}

long TextBuffer::size(void)
{
   return(_size);
}

#ifdef AMIGA
struct mydata {char *ptr; int cnt;};

static void __asm __saveds wlputc(register __d0 char c, register __a3 struct mydata *md)
{
   md->ptr[md->cnt++] = c;
   return;
}
#endif

long TextBuffer::file(char *fmt, ...)
{
   va_list args;

   if (_size == 0) return(0);

   if (fmt   == 0) {
      if (_name == 0) return(0);
      strcpy(_buf, _name);
   }
   else {
#ifdef AMIGA
      struct mydata md;
      md.ptr = _buf;
      md.cnt = 0;
#endif
      va_start(args, fmt);
#ifdef AMIGA
      RawDoFmt(fmt, args, wlputc, &md);
#else
      vsprintf(_buf, fmt, args);
#endif
      va_end(args);
   }

   FILE *fp = fopen(_buf, "w");
   if (fp) {
      fwrite(_text, 1, _size, fp);
      fclose(fp);
      return(_size);
   }
   return(0);
}

/**
*
*   Function to add formatted output
*
**/
void TextBuffer::printf(char *fmt, ...)
{
   if (fmt == 0) return;

   va_list args;
#ifdef AMIGA
   struct mydata md;
   md.ptr = _buf;
   md.cnt = 0;
#endif
   va_start(args, fmt);
#ifdef AMIGA
   RawDoFmt(fmt, args, wlputc, &md);
#else
   vsprintf(_buf, fmt, args);
#endif
   va_end(args);

   add(_buf);

   return;
}

