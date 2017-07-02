/** PortManager.cpp
*
*   Implementation of Registration and Unregistration methods of the
*   PortManager class
*
*   W.G.J. Langeveld, 4 March 1999
*
**/
#include <Application.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MWERKS__
#pragma export on
#endif

#include "PortManager.h"

#include <PropertyInfo.h>
#include <Roster.h>

#define PORTMANAGER_MIME "application/x-PortManager"

PortManager::PortManager() :
      BMessenger(PORTMANAGER_MIME),
      _portname(0)
{
   if (!IsValid()) Launch();
   return;
};

PortManager::PortManager(const char *portname, const BMessenger &messenger) :
      BMessenger(PORTMANAGER_MIME),
      _portname(0)
{
   Register(portname, messenger);
   return;
};

PortManager::~PortManager()
{
   UnRegister();
   return;
};

void PortManager::Launch(void)
{
   long launched = 0;
   while (1) {
      if (be_roster->IsRunning(PORTMANAGER_MIME)) {
         *((BMessenger *) this) = BMessenger(PORTMANAGER_MIME);
         break;
      }
      else {         
         if (launched == 0) {
            be_roster->Launch(PORTMANAGER_MIME);
            launched = 1;
         }
         snooze(100000);
      }
   }
   return;
}

status_t PortManager::Register(const char *portname, const BMessenger &messenger)
{
   if (_portname) return(B_ERROR);
/*
*   Here's where we register to the PortManager. Have to do this
*   here rather than in the contructor, because our own Messenger
*   will not be active until our application is running.
*/
   if (!IsValid()) Launch();
   if (!IsValid()) return(B_ERROR);
/*
*   Create a specifier. Can't use AddSpecifier, because this
*   one is special: it has a BMessenger to the View's loop.
*/
   BMessage spec(B_NAME_SPECIFIER);
   spec.AddString(   "property",  "Port");
   spec.AddString(   "name",      portname);
   spec.AddMessenger("messenger", messenger);
/*
*   Create a message and add the above specifier
*/
   BMessage msg(B_CREATE_PROPERTY), reply;
   msg.AddSpecifier(&spec);
/*
*   Send to the PortManager
*/
   status_t result = SendMessage(&msg, &reply, 1000000, 1000000);
   if (result == B_OK) {
      _portname = new char[strlen(portname) + 1];
      strcpy(_portname, portname);
   }       
   return(result);
}

void PortManager::UnRegister(void)
{
   if (_portname == 0) return;
/*
*   Unregister the port.
*/
   if (IsValid()) {
       BMessage msg(B_DELETE_PROPERTY), reply;
       msg.AddSpecifier("Port", _portname);
       SendMessage(&msg, &reply, 1000000, 1000000);
   }
   return;
}

const char *PortManager::GetCommand(const BMessage *message)
{
   const char *cmd;
   status_t err = message->FindString("command", &cmd);
   if (err == B_OK) return(cmd);
   return(0);
}

status_t PortManager::SendReply(BMessage *message, const char *result,
                                int32 err, const char *replystring)
{
   BMessage reply(err ? B_ERROR : B_REPLY);
/*
*   The order is of some importance here. First the results, then the
*   error code, and then the message.
*/
   reply.AddString("results", result);
   reply.AddInt32( "error",   err);
   reply.AddString("message", replystring);

   return(message->SendReply(&reply));
}

status_t PortManager::SendReply(BMessage *message, const BMessage *result,
                                int32 err, const char *replystring)
{
   BMessage reply(err ? B_ERROR : B_REPLY);
/*
*   The order is of some importance here. First the results, then the
*   error code, and then the message.
*/
   reply.AddMessage("results", result);
   reply.AddInt32( "error",   err);
   reply.AddString("message", replystring);

   return(message->SendReply(&reply));
}

status_t PortManager::FindPort(const char *portname, BMessenger *app)
{
   if (!IsValid()) return(0);

   BMessage getport(B_GET_PROPERTY), reply;
   getport.AddSpecifier("Port", portname);
   status_t result = SendMessage(&getport, &reply, 1000000, 1000000);
   if (result == B_OK) {
      int32 err;
      if ((result = reply.FindInt32("error", &err)) == B_OK) {
         if (err == 0) {
            BMessage msg;
            if ((result = reply.FindMessage("results", &msg)) == B_OK) {
               if ((result = msg.FindMessenger("messenger", app)) == B_OK) {
                  if (app->IsValid()) result = B_OK;
                  else                result = B_ERROR;
               }
            }
         }
         else {
            result = B_ERROR;
         }
      }
   }
   return(result);
}

status_t PortManager::Execute(const BMessenger *app, const char *cmd, BMessage *reply)
{
   BMessage msg(B_EXECUTE_PROPERTY);
   msg.AddSpecifier("Command");
   msg.AddString("command", cmd);
   return(app->SendMessage(&msg, reply, 1000000, 1000000));
}

status_t PortManager::GetPortList(BList *list)
{
   status_t result;
   if (IsValid()) {
      BMessage getport(B_GET_PROPERTY), reply;
      getport.AddSpecifier("PortList");
      result = SendMessage(&getport, &reply, 1000000, 1000000);
      if (result == B_OK) {
         BMessage mlist;
         if ((result = reply.FindMessage("results", &mlist)) == B_OK) {
            char *name;
            uint32 type;
            for (int32 i = 0; mlist.GetInfo(B_ANY_TYPE, i, &name, &type) == B_OK; i++) {
               if (type == B_MESSAGE_TYPE) list->AddItem(name);
            }
         } 
      }
   }
   else {
      result = B_ERROR;
   }
   return(result);
}

int32 PortManager::IsForStandardSuite(const char *property)
{
   if (strcmp(property, "Command") == 0) return(1);
   return(0);
}

void PortManager::AddStandardSuite(BMessage *message, const char *name)
{
   static property_info prop_list[] = { 
      { 
         "Command",
         {B_EXECUTE_PROPERTY, 0,}, 
         {B_DIRECT_SPECIFIER, 0,},
         "Execute a string command",
      }, 
      {0, 0, 0, 0,}
   };

   message->AddString("suites", name);
   BPropertyInfo prop_info(prop_list); 
   message->AddFlat("messages", &prop_info);
   return;
}
   
