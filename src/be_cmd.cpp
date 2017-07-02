/**
*
*   Commands and functions that apply to BeOS
*
**/
#ifdef __HAIKU__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#include <OS.h>
#include <Application.h>
#include <Message.h>
#include <Messenger.h>
#include <Roster.h>
#include "PropDump.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "rexx.h"
#include "error.h"
#include "utils.h"
#include "convert.h"
#include "variable.h"
#include "instruct.h"
#include "rxdefs.h"
#include "bio.h"

#ifdef __cplusplus
}
#endif

#include "be_cmd.h"
#include "textbuffer.h"
#include "hey.h"
#include "PortManager.h"

long  find_app(BMessenger *, char *);

/**
*
*   SHOWLIST(<name>)
*
*   Show a particular system list.
*
*   <name> can be:
*
*   APPS    a list of applications that can receive messages
*
**/
void R_showlist(args *arg)
{
   char *ptr = NULL;
   char pad;
   Lstr *testfor;

   if (!IN_RANGE(1,ARGN,3)) error(ERR_INCORRECT_CALL);

   must_exist(1);  L2str(&ARG1);

   if (exist(2)) {
      L2str(&ARG2);
      testfor = ARG2;
   }
   else {
      testfor = NULL;
   }

   get_pad(3);

   char c = toupper(STR(ARG1)[0]);
   if (c == 'A' || c == 'S' || c == 'R') {
      ptr = be_app_list(testfor, pad, c);
   }

   if (ptr) {
      Lscpy(&ARGR, ptr);
      free(ptr);
   }
   else {
      Lscpy(&ARGR, "");
   }

   return;
}

/**
*
*   Test if an application exists
*
**/
long be_app_exists(Lstr *appname)
{
/*
*   find the application
*/
   ASCIIZ(appname);
   return(find_app(0, STR(appname)));
}

#define IS_PORT 1
#define IS_APP  2

static long oldtype = 0;
static char oldname[256] = "\0";
static BMessenger oldapp;

long find_app(BMessenger *the_application, char *appname)
{
/*
*   Cache application messengers
*/
   if ((strcmp(appname, oldname) == 0) && (oldtype != 0)) {
      if (oldapp.IsValid()) {
         if (the_application) *the_application = oldapp;
         return(oldtype);
      }
   }
   else {
      oldtype = 0;
   }

   BList team_list;
   team_id teamid;
   app_info appinfo;

   BMessenger the_app;
   if (the_application == NULL) the_application = &the_app;
/*
*   find the application. If the application name starts with
*   a colon, then force "standard" scripting, i.e. don't look in
*   the PortManager for a registered port. If an application really
*   does start with a colon, one can use two colons, but no
*   "string"-type scripting can be done, unless the registered
*   port name is different from the application name.
*/
   long force = 0;
   if (*appname == ':') {
      appname++;
      if (strlen(appname) == 0) return(0);
      force = 1;
   }

   if (force != 1) {
/*
*   Check the PortManager
*/
      PortManager pman;
      if (pman.FindPort(appname, the_application) == B_OK) {
         oldtype = IS_PORT;
         oldapp  = *the_application;
         strcpy(oldname, appname);
         return(IS_PORT);
      }
   }
/*
*   Not available in the PortManager. See if it's a regular application.
*/
   be_roster->GetAppList(&team_list);

   for (int32 i = 0; i < team_list.CountItems(); i++) {
      teamid = (team_id) team_list.ItemAt(i);
      be_roster->GetRunningAppInfo(teamid, &appinfo);
      if (strncmp(appinfo.signature, appname, strlen(appname)) == 0) {
         *the_application = BMessenger(appinfo.signature);
         break;
      }
      else if (strncmp(appinfo.ref.name, appname, strlen(appname)) == 0) {
         *the_application = BMessenger(0, teamid);
         break;
      }
   }
	
   if (!the_application->IsValid()) return(0);
/*
*   We have a valid application. Now see if it responds to a simple scripting
*   message, such as getting its messenger.
*/
   status_t result;
   BMessage getmsgr(B_GET_PROPERTY), reply;
   getmsgr.AddSpecifier("Name");
   result = the_application->SendMessage(&getmsgr, &reply, 1000000, 1000000);

   if (result != B_OK) return(0);

   if (reply.what != B_REPLY) return(0);

   oldtype = IS_APP;
   oldapp  = *the_application;
   strcpy(oldname, appname);

   return(IS_APP);
}

long redirect_be_app(Lstr *Lcmd, Lstr *Lenv)
{
   ASCIIZ(Lenv);
   ASCIIZ(Lcmd);

   char *cmd = STR(Lcmd);
   char *env = STR(Lenv);

   BMessenger the_application;
   long type;
   if ((type = find_app(&the_application, env)) == 0) return(1);

   BMessage the_reply;
   status_t err;

   if (type == IS_PORT) {
      err = PortManager::Execute(&the_application, cmd, &the_reply);
   }
   else {
      err = Hey(&the_application, cmd, &the_reply);
   }

   if (err != B_OK) {
      bprintf("Error when sending message to %s!\n", env);
      return(2);
   }
   else if (the_reply.what == (uint32) B_MESSAGE_NOT_UNDERSTOOD || the_reply.what == (uint32) B_ERROR) {
      if (the_reply.HasString("message")) bprintf(">>> %s\n", the_reply.FindString("message"));
      return(the_reply.FindInt32("error"));
   }

//   BString str = DumpMessage(&the_reply, MSG_IS_CMD);
//   bprintf("%s\n", str.String());

   long total = message_to_result(&the_reply, 0);
   char buff[10];
   sprintf(buff, "%d", total);
   be_set_cvar("RESULT", "0", buff);

   return(0);
}

void be_set_var(char *name, char *value)
{
   Lstr   *resultid   = NULL;
   Lstr   *resulttext = NULL;
   Idrec  *Aptr;

   Lscpy(&resultid, name);
   Aptr = createId(resultid, &scope );

   Lscpy(&resulttext, value);

   assign(Aptr, resulttext);

   m_free(resultid);
   m_free(resulttext);

   return;
}

void be_set_cvar(char *name, char *subname, char *value)
{
   char buff[10];
   Lstr   *resultid   = NULL;
   Lstr   *resulttext = NULL;
   Idrec  *Aptr;

   Lscpy(&resultid, name);
   Lcat(&resultid, ".");
   Lcat(&resultid, subname);
   Aptr = C_create(resultid, &scope );

   Lscpy(&resulttext, value);

   assign(Aptr, resulttext);

   m_free(resultid);
   m_free(resulttext);
   return;
}

char *be_app_list(Lstr *appname, char pad, char listtype)
{
   TextBuffer tb;
   long first = 1;
/*
*   If we have an appname, we need to test against it.
*/
   if (appname) return(strdup(be_app_exists(appname) ? "1" : "0"));
/*
*   For listtype R we need to check the PortManager
*/
   if (listtype == 'R') {
      PortManager pman;
      BList list;
      if (pman.GetPortList(&list) == B_OK) {
         for (int32 i = 0; i < list.CountItems(); i++) {
           if (first == 0) tb.add(pad);
           else            first = 0;
           tb.add((char *) list.ItemAt(i));
         }
      }
   }
   else {
/*
*   Not a Portmanager item, check  the BRoster.
*   Get the list of teams
*/
      BList teamlist;
      be_roster->GetAppList(&teamlist);
/*
*   ...and loop over them
*/
      team_id teamid;
      app_info appinfo;
      BMessenger the_application;

      for (int ix = teamlist.CountItems() - 1; ix >= 0; ix--) {
         teamid = (team_id) teamlist.ItemAt(ix);
         be_roster->GetRunningAppInfo(teamid, &appinfo);

         the_application = BMessenger(appinfo.signature);

         if (the_application.IsValid()) {
/*
*   We have a valid application. Now see if it responds to a simple scripting
*   message, such as getting its messenger.
*/
            status_t result;
            BMessenger foo;
            BMessage getmsgr(B_GET_PROPERTY), reply;
            getmsgr.AddSpecifier("Name");
            result = the_application.SendMessage(&getmsgr, &reply, 1000000, 1000000);
            if (result == B_OK) {
               if (reply.what == B_REPLY) {
                  if      (listtype == 'S') {
                     if (first == 0) tb.add(pad);
                     else            first = 0;
                     tb.add(appinfo.signature);
                  }
                  else if (listtype == 'A') {
                     if (first == 0) tb.add(pad);
                     else            first = 0;
                     tb.add(appinfo.ref.name);
                  }
               }
            }
         }
      }
   }

   char *buff = (char *) malloc(tb.size() + 1);
   if (buff && tb.text()) strcpy(buff, tb.text());

   return(buff);
}

#endif
