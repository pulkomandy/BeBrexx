/** consolebe.cc
*
*   Terminal I/O from a Be view.
*
*   W.G.J. Langeveld, March 1999.
*
**/
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __HAIKU__

#include <AppKit.h>
#include <InterfaceKit.h>
#include "consolebe.hh"

ConsoleBe::ConsoleBe(BRect textframe, const char *name, BRect textrect, int32 followflags,
                                                                        int32 otherflags)
         : BTextView(textframe, name, textrect, followflags, otherflags),
         offset1(0), offset2(0), pasting(0)
{
   SetDoesUndo(false);
   MakeFocus(true);
   cooked = 1;
   list = new LBList(20);
   return;
}

void ConsoleBe::setraw(long wait)
{
   cooked = 0;
   return;
}

ConsoleBe::~ConsoleBe()
{
/*
*   This list only contains longs, not pointers, so we don't need to delete them
*/
   if (list) delete list;
   setcooked();
   return;
}

void ConsoleBe::setcooked(void)
{
   cooked = 1;
   return;
}

extern "C" {
   extern int be_was_interrupted(void);
}

int ConsoleBe::get_char(void)
{
   addr_t c = -1;
   while (c == -1) {
      if (list->Lock()) {
         if (!list->IsEmpty()) {
            c = (addr_t) list->ItemAt(0);
            list->RemoveItem((int32) 0);
            list->Unlock();
            break;
         }
         list->Unlock();
      }
      snooze(10000);
      if (be_was_interrupted()) {
         c = -1;
         break;
      }
   }

   return(c);
}

/**
*
*   Write to screen in ECHO mode (typically while writing a command line)
*   This function will turn of "raw" mode during each write - only "formatted"
*   stuff should use this function.
*
**/
#define HSCMDSIZ 512
long ConsoleBe::WriteE(long b, long a, const char *str, long n)
{
   long l;
   char buff[HSCMDSIZ + 20], *ptr;

   ptr = buff;

   if ((n == -1) && str) n = strlen(str);

   if (b) {
      if (Window()->Lock()) {
         offset1 += b;
         offset2 = offset1;
         Select(offset1, offset1);
         Window()->Unlock();
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

   if (ptr != buff) {
      WriteT(buff, ptr - buff, flags & ~(CONSOLE_CONVFF | CONSOLE_RAW));
   }


   if (a) {
      if (Window()->Lock()) {
         offset1 += a;
         offset2 = offset1;
         Select(offset1, offset1);
         Window()->Unlock();
      }
   }

   return(n);
}

/**
*
*   Write a string to a socket and do various conversions...
*
**/
long ConsoleBe::WriteT(const char *str, long n, long newflags)
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
	       case  7	 :
                  beep();
                  break;
	       case '\n' :
		  *ptr++ = c;
		  break;
	       case '\f' :
                  Clear();
                  break;
               default :
                  break;
	    }
	 }
	 else {
            *ptr++ = c;
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
long ConsoleBe::Write(const char *str, long n)
{
//   printf("Inserting |%s|\n", str);
   if (Window()->Lock()) {
      offset2 = offset1 + n;
      BTextView::Delete(offset1, offset2);
      Select(offset1, offset1);
      BTextView::Insert(str, n);
      ScrollToSelection();
      GetSelection(&offset1, &offset2);
      offset2 = offset1;
      Window()->Unlock();
   }
   return(n);
}

void ConsoleBe::KeyDown(const char *bytes, int32 numBytes)
{
   int c = *bytes;

   if (c == 0x1a || c == 0x1c || c == 0x1d || c == 0x1e || c == 0x1f ||
       c == 0x05 || c == 0x01 || c == 0x04 || c == 0x0b || c == 0x0c ||
       c == 0x10) {
      BMessage *msg = Window()->CurrentMessage();
      if (msg) {
         int32 rawchar;
         if (msg->FindInt32("raw_char", &rawchar) == B_OK) {
            if (list->Lock()) {
               if (rawchar < 32) {
                  switch (c) {
                     case B_UP_ARROW :
                        list->AddItem((void *) 0x9b);
                        list->AddItem((void *) 'A');     // UP
                        break;
                     case B_DOWN_ARROW :
                        list->AddItem((void *) 0x9b);
                        list->AddItem((void *) 'B');     // DOWN
                        break;
                     case B_LEFT_ARROW :
                        list->AddItem((void *) 0x9b);
                        list->AddItem((void *) 'D');     // LEFT
                        break;
                     case B_RIGHT_ARROW :
                        list->AddItem((void *) 0x9b);
                        list->AddItem((void *) 'C');     // RIGHT
                        break;
                     case B_INSERT :
                        list->AddItem((void *) 0x1);     // Flip Insert/Overstrike
                        break;
                  }
               }
               else {
                  list->AddItem((void *) c);
               }
               list->Unlock();
            }
         }
      }
   }
   else {
      if (list->Lock()) {
         list->AddItem((void *) c);
         list->Unlock();
      }
   }
   return;
}

void ConsoleBe::Cut(BClipboard *b)
{
   int32 posfromend = TextLength() - offset1;
   BTextView::Cut(b);
   offset2 = offset1 = TextLength() - posfromend;
   return;
}

void ConsoleBe::Paste(BClipboard *b)
{
   pasting = 1;
   BTextView::Paste(b);
   pasting = 0;
   return;
}

void ConsoleBe::InsertText(const char *text, int32 length, int32 offset, const text_run_array *runs)
{
   if (pasting) {
      if (list->Lock()) {
         for (int32 i = 0; i < length; i++) {
            if (text[i] >= 32) list->AddItem((void *) text[i]);
         }
         list->Unlock();
      }
   }
   else {
      BTextView::InsertText(text, length, offset, runs);
   }
   return;
}

#endif
