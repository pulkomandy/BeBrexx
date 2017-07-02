/** consolehistory.hh
*
*   Simple console history
*
*   W.G.J. Langeveld, March 1999
*
**/
#ifndef CONSOLEHISTORY__HH
#define CONSOLEHISTORY__HH

#define CONHIST_NOTINUSE   1

class ConsoleHistory {
public:
/*
*   Constructors, destructor
*/
   ConsoleHistory();
   virtual ~ConsoleHistory();
/*
*   Add a string
*/
   void add(const char *s, long len);
/*
*   Retrieve the first and next string
*/
   char *first(void) const;
   char *last(void) const;
   char *next(void) const;
   char *prev(void) const;
   void  setpastend(void) const;

   void  clear(void);
/*
*   Retrieve a node with a certain ID.
*/
   char *find(long) const;
   char *find(const char *, long) const;
/*
*   Retrieve the size of the history array
*/
   long  size(void) const;
   long  status(void) const;

private:
/*
*   A list of strings
*/
   char  **list;
   long    num;
   long    index;
   long    allocsize;
/*
*   Prevent copy, assignment, sequence
*/
   ConsoleHistory(const ConsoleHistory &);
   ConsoleHistory &operator=(const ConsoleHistory &);
   ConsoleHistory &operator,(const ConsoleHistory &);
};

#endif
