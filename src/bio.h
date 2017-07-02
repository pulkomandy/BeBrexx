/** bio.h
*
*   IO routines that allow BRexx to be run from different places
*
**/
int bprintf(char *, ...);
int beprintf(char *, ...);
int bputs(char *);
int bputchar(char);
char *bgetline(char *, long);