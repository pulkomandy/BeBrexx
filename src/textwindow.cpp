//
// Text World
//
// A sample program that's gradually evolving into
// a real text editor application.
//
// Copyright 1998, Be Incorporated
//
// Written by: Eric Shepherd
// Shamelessly stolen by Willy Langeveld and perverted to my own ends.
//

#include <Application.h>
#include <Messenger.h>
#include <Message.h>
#include <Roster.h>
#include <Window.h>
#include <View.h>
#include <MenuBar.h>
#include <Menu.h>
#include <Entry.h>
#include <Path.h>
#include <MenuItem.h>
#include <TextView.h>
#include <FilePanel.h>
#include <ScrollView.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "textwindow.h"

extern "C" {
#include "be_main.h"
}

//
// TextWindow::TextWindow
//
// Constructs the window we'll be drawing into.
//
TextWindow::TextWindow(BRect frame, const char *title, int32 u)
   : BWindow(frame, title, B_DOCUMENT_WINDOW, 0), useframe(u)
{
   _InitWindow();
   Show();
}

//
// TextWindow::_InitWindow
//
// Initialize the window.
//
void TextWindow::_InitWindow(void)
{
   BRect r;
   BMenu *menu;
   BMenuItem *item;

   SetToSavedFrame();
	
// Initialize variables
	
   savemessage = NULL; // No saved path yet

// Add the menu bar

   r = Bounds();
   menubar = new BMenuBar(r, "menu_bar");

// Add File menu to menu bar
	
   menu = new BMenu("File");
   menu->AddItem(saveitem=new BMenuItem("Save", new BMessage(MENU_FILE_SAVE), 'S'));
   saveitem->SetEnabled(false);
   menu->AddItem(new BMenuItem("Save as" B_UTF8_ELLIPSIS,
                               new BMessage(MENU_FILE_SAVEAS)));
   menu->AddSeparatorItem();
   menu->AddItem(item=new BMenuItem("Page Setup" B_UTF8_ELLIPSIS,
                 new BMessage(MENU_FILE_PAGESETUP)));
   item->SetEnabled(false);
   menu->AddItem(item=new BMenuItem("Print" B_UTF8_ELLIPSIS,
                                    new BMessage(MENU_FILE_PRINT), 'P'));
   item->SetEnabled(false);
   menu->AddSeparatorItem();
   menu->AddItem(new BMenuItem("Quit", new BMessage(MENU_FILE_QUIT), 'Q'));
   menubar->AddItem(menu);
	
// Attach the menu bar to he window
	
   AddChild(menubar);

// Add the text view
	
   BRect textframe = Bounds();
   textframe.top = menubar->Bounds().bottom + 1.0;
   textframe.right -= B_V_SCROLL_BAR_WIDTH;
   BRect textrect = textframe;
   textrect.OffsetTo(B_ORIGIN);
   r.InsetBy(3.0,3.0);
   textview = new ConsoleBe(textframe, "text_view", textrect,
                            B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);
   AddChild(scrollview = new BScrollView("scroll_view", textview,
            B_FOLLOW_ALL_SIDES, 0, false, true, B_NO_BORDER));
   textview->SetDoesUndo(false);
   textview->MakeFocus(true);

// Add the Edit menu to the menu bar

   menu = new BMenu("Edit");
   menu->AddItem(item=new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
   item->SetTarget(textview);
   menu->AddItem(item=new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
   item->SetTarget(textview);
   menu->AddItem(item=new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
   item->SetTarget(textview);
   menu->AddSeparatorItem();
   menu->AddItem(item=new BMenuItem("Select All", new BMessage(B_SELECT_ALL), 'A'));
   item->SetTarget(textview);
   menubar->AddItem(menu);

// Create the save filepanel for this window
	
   savePanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL,
                              B_FILE_NODE, false);

// Tell the application that there's one more window
// and get the number for this untitled window.
}


//
// TextWindow::FrameResized
//
// Adjust the size of the BTextView's text rectangle
// when the window is resized.
//
void TextWindow::FrameResized(float width, float height)
{
   BRect textrect = textview->TextRect();
	
   textrect.right = textrect.left + (width - B_V_SCROLL_BAR_WIDTH - 3.0);
   textview->SetTextRect(textrect);
}


//
// TextWindow::~TextWindow
//
// Destruct the window.  This calls Unregister().
//
TextWindow::~TextWindow()
{
   SaveFrame();
   if (savemessage) {
      delete savemessage;
   }
   delete savePanel;
}


//
// TextWindow::MessageReceived
//
// Called when a message is received by our
// application.
//
void TextWindow::MessageReceived(BMessage *message)
{
   switch(message->what) {
      case MENU_FILE_QUIT:
         be_interrupt();
         be_app->PostMessage(B_QUIT_REQUESTED);
         break;
      case MENU_FILE_SAVEAS:
         savePanel->Show();
         break;
      case MENU_FILE_SAVE:
         Save(NULL);
         break;
      case B_SAVE_REQUESTED:
         Save(message);
         break;
      default:
         BWindow::MessageReceived(message);
         break;
   }
}


//
// TextWindow::QuitRequested
//
// Tell the application to quit.
//
bool TextWindow::QuitRequested()
{
   be_interrupt();
   be_app->PostMessage(B_QUIT_REQUESTED);	
   return true;
}

#define SETTINGSFILE "/boot/home/config/settings/BRexx"
void TextWindow::SetToSavedFrame(void)
{
   if (useframe) return;

   char buff[512];
   double top = 50, left = 50, width = 400, height = 200;
   float d;
   FILE *fp = fopen(SETTINGSFILE, "r");
   if (fp) {
      while (fgets(buff, 512, fp)) {
         if      (sscanf(buff, "top %f",    &d)) top    = d;
         else if (sscanf(buff, "left %f",   &d)) left   = d;
         else if (sscanf(buff, "width %f",  &d)) width  = d;
         else if (sscanf(buff, "height %f", &d)) height = d;
      }
      fclose(fp);
   }
   MoveTo(left, top);
   ResizeTo(width, height);
   return;
}

void TextWindow::SaveFrame(void)
{
   if (useframe) return;

   BRect b = Bounds();
   BRect f = Frame();
   FILE *fp = fopen(SETTINGSFILE, "w");
   if (fp) {
      fprintf(fp, "top %f\n",    f.top);
      fprintf(fp, "left %f\n",   f.left);
      fprintf(fp, "width %f\n",  b.right);
      fprintf(fp, "height %f\n", b.bottom);
      fclose(fp);
   }
   return;
}

//
// TextWindow::Save
//
// Save the contents of the window.  The message specifies
// where to save it (see BFilePanel in the Storage Kit chapter
// of the Be Book).
//
status_t TextWindow::Save(BMessage *message)
{
   entry_ref ref;		// For the directory to save into
   status_t err;		// For the return code
   const char *name;		// For the filename
   BPath path;			// For the pathname
   BEntry entry;		// Used to make the path
   FILE *f;			// Standard Posix file
	
// If a NULL is passed for the message pointer, use
// the value we've cached; this lets us do saves without
// thinking.
	
   if (!message) {
      message = savemessage;
      if (!message) {
         return B_ERROR;
      }
   }
	
// Peel the entry_ref and name of the directory and
// file out of the message.
	
   if ((err=message->FindRef("directory", &ref)) != B_OK) {
      return err;
   }
   if ((err=message->FindString("name", &name)) != B_OK) {
      return err;
   }
	
// Take the directory and create a pathname out of it
	
   if ((err=entry.SetTo(&ref)) != B_OK) {
      return err;
   }
   entry.GetPath(&path);		// Create a pathname for the directory
   path.Append(name);			// Tack on the filename
	
// Now we can save the file.
	
   if (!(f = fopen(path.Path(), "w"))) {
      return B_ERROR;
   }
	
   err = fwrite(textview->Text(), 1, textview->TextLength(), f);
   fclose(f);
   if (err >= 0) {
      saveitem->SetEnabled(true);
      if (savemessage != message) {
         if (savemessage) {
            delete savemessage;
         }
         savemessage = new BMessage(message);
      }
   }
   return err;
}
