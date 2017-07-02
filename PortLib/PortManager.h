/** PortManager.h
*
*   Simple interface to the global port list maintained by the PortManager
*   application.
*
*   Note: Registration can only be successful if the registered BMessenger
*         is Valid at the time of reistration. This means that the host
*         application needs to be running. In practical terms, either
*         use the second constructor, or the Register method, in the
*         AttachedToWindow() method of the View that will receive the
*         messages.
*
*   W.G.J. Langeveld, 4 March 1999
*
**/
#ifndef PORTMANAGER__H
#define PORTMANAGER__H

class PortManager : public BMessenger {
public:
/*
*   Constructors, destructor
*/
   PortManager();
   PortManager(const char *portname, const BMessenger &messenger);
   virtual ~PortManager();
/*
*   Register and unregister a port
*/
   status_t           Register(const char *portname, const BMessenger &messenger);
   void               UnRegister();
/*
*   Find a port and its messenger, or get the entire list of port names
*/
   status_t           FindPort(const char *portname, BMessenger *app);
   status_t           GetPortList(BList *list);
/*
*   Get a command from an incoming message
*/
   static const char *GetCommand(const BMessage *message);
/*
*   Send various types of reply
*/
   static status_t    SendReply(BMessage *message, const char *result, int32 err, const char *replystring);
   static status_t    SendReply(BMessage *message, const BMessage *result, int32 err, const char *replystring);
/*
*   Execute a command given the messenger of an application
*/
   static status_t    Execute(const BMessenger *app, const char *cmd, BMessage *reply);
/*
*   Check if a command is for the standard suite. To be used in ResolveSpecifier.
*/
   static int32       IsForStandardSuite(const char *property);
/*
*   Add a "standard" (i.e. "Command") suite. To be used in GetSupportedSuites.
*/
   static void        AddStandardSuite(BMessage *message, const char *name);

private:
   char *_portname;
   void Launch(void);
};

#endif
