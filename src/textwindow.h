//
// TextWindow class
//
// This class defines the hello world window.
//

#ifndef __TWINDOW_H__
#define __TWINDOW_H__

#include <FilePanel.h>
#include <Message.h>
#include <Window.h>

#include "consolebe.hh"

class TextWindow : public BWindow {
public:
   TextWindow(BRect frame, const char *title, int32 useframe = 0);
   virtual ~TextWindow();
   virtual bool  QuitRequested();
   virtual void  MessageReceived(BMessage *message);
   virtual void  FrameResized(float width, float height);

   status_t      Save(BMessage *message);

   ConsoleBe    *GetTextView(void) { return textview; }

private:
   void         _InitWindow(void);
   void          SetToSavedFrame(void);
   void          SaveFrame(void);

   int32         useframe;

   BMenuBar     *menubar;
   ConsoleBe    *textview;
   BScrollView  *scrollview;
   BMenuItem    *saveitem;
   BMessage     *savemessage;

   BFilePanel   *savePanel;
};

#endif
