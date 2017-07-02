// portmanager.cpp
//
// A simple program that accepts scripting messages to either
// register, unresgister or return named messengers.
//
// Willy Langeveld, february 1999
//
#include <Application.h>
#include <PropertyInfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PortManager.h"
#include "unistd.h"

class PortApp: public BApplication {
public:
   PortApp();
   virtual ~PortApp();
   virtual void MessageReceived(BMessage *);
   virtual status_t GetSupportedSuites(BMessage *);
   virtual BHandler *ResolveSpecifier(BMessage *, int32,
		                      BMessage *, int32, const char *);
   virtual bool QuitRequested(void);

private:
   BMessage  *MessengerList;
};

int main( int argc, char **argv )
{
   setpgid(getpid(), 0);
   BApplication *myApp = new PortApp;
   myApp->Run();
   delete myApp;
   return 0;
} 

PortApp::PortApp() : BApplication("application/x-PortManager" )
{
   MessengerList = new BMessage(B_REPLY);
   return;
}

PortApp::~PortApp()
{
   delete MessengerList;
   return;
}

void PortApp::MessageReceived(BMessage *message)
{
   const char *property;
   int32 index = 0;
   int32 what;
   BMessage specifier;

   if (message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
      if (strcmp(property, "Port") == 0) {
         BMessage msg;
         const char *portname, *replystring;
         int32 err;

         specifier.FindString("name", &portname);

         switch (message->what) {
            case B_CREATE_PROPERTY:
               if (MessengerList->HasMessage(portname)) {
                  err = 1;
                  replystring = "Messenger already exists";
               }
               else {
                  MessengerList->AddMessage(portname, &specifier);
                  err = 0;
                  replystring = "Normal successful completion";
               }
               PortManager::SendReply(message, "", err, replystring);
               break;
            case B_DELETE_PROPERTY:
               if (!MessengerList->HasMessage(portname)) {
                  err = 2;
                  replystring = "Messenger does not exist";
               }
               else {
                  MessengerList->RemoveName(portname);
                  err = 0;
                  replystring = "Normal successful completion";
               }
               PortManager::SendReply(message, "", err, replystring);
               break;
            case B_GET_PROPERTY:
               if (MessengerList->FindMessage(portname, &msg) != B_OK) {
                  err = 2;
                  replystring = "Messenger does not exist";
                  PortManager::SendReply(message, "", err, replystring);
               }
               else {
                  err = 0;
                  replystring = "Normal successful completion";
                  PortManager::SendReply(message, &msg, err, replystring);
               }
               break;
            default:
               PortManager::SendReply(message, "", 3, "Unknown operation");
               break;
         }
         return;
      }
      else if (strcmp(property, "PortList") == 0) {
         if (message->what == B_GET_PROPERTY) {
            PortManager::SendReply(message, MessengerList, 0, "Normal successful completion");
         }
         else {
            PortManager::SendReply(message, "", 3, "Unknown operation");
         }
         return;
      }
   }
   
   BApplication::MessageReceived(message);
   return;
}

BHandler *PortApp::ResolveSpecifier(BMessage *message, int32 index, 
                                    BMessage *specifier, int32 what,
                                    const char *property)
{
   if (strncmp(property, "Port", 4) == 0) return(this);
   return(BApplication::ResolveSpecifier(message, index, specifier, what, property));
}

static property_info prop_list[] = { 
   {  "Port",     {B_CREATE_PROPERTY, 0,}, {B_NAME_SPECIFIER,   0,}, "", },
   {  "Port",     {B_DELETE_PROPERTY, 0,}, {B_NAME_SPECIFIER,   0,}, "", },
   {  "Port",     {B_GET_PROPERTY,    0,}, {B_NAME_SPECIFIER,   0,}, "", },
   {  "PortList", {B_GET_PROPERTY,    0,}, {B_DIRECT_SPECIFIER, 0,}, "", },
   {  0,          0,                       0,                        0,  }
};
   
status_t PortApp::GetSupportedSuites(BMessage *message) 
{ 
   message->AddString("suites", "suite/vnd.PortToy_handler"); 
   BPropertyInfo prop_info(prop_list); 
   message->AddFlat("messages", &prop_info); 
   return BApplication::GetSupportedSuites(message); 
}

bool PortApp::QuitRequested(void)
{
   return(false);
}
