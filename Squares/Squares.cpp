/** Squares.cpp
*
*   This program implements a simple application with a Window and a View.
*   The View registers its Handler as a "Port" with the PortManager (see
*   the docs on PortManager), and its MessageReceived method responds to
*   simple scripting messages, allowing a scripting language such as
*   BREXX to draw colorful squares.
*
*   This code is in the public domain.
*
*   W.G.J. Langeveld, 3 March 1999
*
**/
#include <Application.h>
#include <Window.h>
#include <View.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "PortManager.h"

class SquaresApp: public BApplication {
public:
   SquaresApp();
   virtual ~SquaresApp();
};

class SquaresWindow: public BWindow {
public:
   SquaresWindow();
   virtual ~SquaresWindow();
   virtual bool QuitRequested();
};

class SquaresView: public BView {
public:
   SquaresView( BRect frame );
   virtual ~SquaresView();		
   virtual void Draw( BRect updateRect );
   virtual void MessageReceived(BMessage *);
   virtual status_t GetSupportedSuites(BMessage *);
   virtual BHandler *ResolveSpecifier(BMessage *, int32,
                                      BMessage *, int32, const char *);
   virtual void AttachedToWindow(void);

private:
   PortManager *pman;
};

int main(int argc, char **argv)
{
   BApplication *myApp = new SquaresApp;
   myApp->Run();
   delete myApp;
   return(0);
} 

SquaresApp::SquaresApp() : BApplication("application/x-vnd.VL-Squares")
{
   BWindow *myWindow = new SquaresWindow;
   myWindow->Show();
   return;
}

SquaresApp::~SquaresApp()
{
   return;
}

SquaresWindow::SquaresWindow() : BWindow(BRect(100, 100, 300, 300),
                                        "Squares", B_DOCUMENT_WINDOW, 0)
{
   BView *myView = new SquaresView(Bounds());
   AddChild(myView);
   return;
}

SquaresWindow::~SquaresWindow()
{
   return;
}

bool SquaresWindow::QuitRequested()
{
   be_app->PostMessage(B_QUIT_REQUESTED);
   return true;
}

SquaresView::SquaresView(BRect frame) : BView(frame, "SquaresView",
                                              B_FOLLOW_ALL, B_WILL_DRAW),
                                        pman(0)
{
   return;
}

void SquaresView::AttachedToWindow(void)
{
/*
*   And here's where we register our port, here called "Squares1"
*/
   pman = new PortManager("Squares1", BMessenger(this));
   return;
}

SquaresView::~SquaresView()
{
/*
*   This also unregisters the port
*/
   delete pman;
   return;
}

void SquaresView::Draw(BRect updateRect)
{
/*
*   In principle this should be implemented for drawing our stuff.
*   For simplicity, we're just drawing everything right at the time
*   the messages are received.
*/
   return;
}

void SquaresView::MessageReceived(BMessage *message)
{
   char buffer[100], *replystring;
   int32 err = 0;
   *buffer = '\0';
   replystring = "Normal completion";

   const char *cmdline;
/*
*   Look for the command sent by the script
*/
   if (pman && (cmdline = PortManager::GetCommand(message))) {
/*
*   Parse it. Here we just use sscanf, but you'll probably want to use your
*   own favorite parsing technique...
*/
      char cmd[20], d1[20], d2[20], d3[20], d4[20];
      sscanf(cmdline, "%s %s %s %s %s", cmd, d1, d2, d3, d4);
/*
*   Uppercase the command section
*/
      for (unsigned int i = 0; i < strlen(cmd); i++) cmd[i] = toupper(cmd[i]);
/*
*   Handle the commands.
*   RECTangle <top> <left> <bottom> <right>
*/
      if (strncmp(cmd, "RECT", 4) == 0) {
         BRect frame;
         frame.top    = atof(d1);
         frame.left   = atof(d2);
         frame.bottom = atof(d3);
         frame.right  = atof(d4);
         FillRect(frame);
      }
/*
*   RGB <red> <green> <blue>
*/
      else if (strncmp(cmd, "RGB", 3) == 0) {
         rgb_color rgb;
         rgb.red      = atol(d1);
         rgb.green    = atol(d2);
         rgb.blue     = atol(d3);
         SetHighColor(rgb);
      }
/*
*   GETSize
*/
      else if (strncmp(cmd, "GETS", 4) == 0) {
         BRect size = Bounds();
         sprintf(buffer, "%f %f %f %f", size.top, size.left, size.bottom, size.right);
      }
      else if (strncmp(cmd, "QUIT", 4) == 0) {
         be_app->PostMessage(B_QUIT_REQUESTED);
      }
      else {
         err = -1;
         replystring = "Unrecognized command";
      }
/*
*   Send reply
*/
      PortManager::SendReply(message, buffer, err, replystring);
   }
   else {
      BView::MessageReceived(message);
   }
   return;
}

/**
*
*   Below are the functions that implement standard BeOS scripting
*   for this view. They're not really needed for this example.
*
**/
BHandler *SquaresView::ResolveSpecifier(BMessage *message, int32 index, 
                                        BMessage *specifier, int32 what,
                                        const char *property)
{
   if (PortManager::IsForStandardSuite(property)) return(this);
   return(BView::ResolveSpecifier(message, index, specifier, what, property));
}

status_t SquaresView::GetSupportedSuites(BMessage *message) 
{ 
   PortManager::AddStandardSuite(message, "suite/vnd.VL-Squares");
   return(BView::GetSupportedSuites(message)); 
}
