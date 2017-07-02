/** textbuffer.h
*
*   Simple way to accumulate a bunch of text
*
**/
#ifndef TEXTBUFFER__H
#define TEXTBUFFER__H

class TextBuffer {
public:
   TextBuffer(const char * = 0);
   virtual ~TextBuffer();
   TextBuffer(const TextBuffer &);
   TextBuffer &operator=(const TextBuffer &);

   void   add(char *);
   void   add(char);
   void   printf(char *, ...);

   char  *text(void);
   long   size(void);

   long   file(char * = 0, ...);

   char  *name(void);
   void   name(const char *);

private:
   char *_buf;
   char *_name;
   char *_text;
   long  _size;
   long  _maxsize;
};

#endif
