/*
 * $Header: error.h!v 1.1 1995/09/11 03:36:01 bill Exp bill $
 * $Log: error.h!v $
 * Revision 1.1  1995/09/11 03:36:01  bill
 * Initial revision
 *
 */
#ifndef __ERROR_H__
#define __ERROR_H__

void  error (int errornum);
void  tracevar ( char *, Lstr *);
void  printcurline ( void );

#ifdef __ERROR_C__

char     *errormsg[] = {
	 /* Error  0*/  "",
	 /* Error  1*/  "",
	 /* Error  2*/  "",
	 /* Error  3*/  "Program is unreadable",
	 /* Error  4*/  "Program interrupted",
	 /* Error  5*/  "Machine storage exhausted",
	 /* Error  6*/  "Unmatched \"/*\" or quote",
	 /* Error  7*/  "WHEN or OTHERWISE expected",
	 /* Error  8*/  "Unexpected THEN or ELSE",
	 /* Error  9*/  "Unexpected WHEN or OTHERWISE",
	 /* Error 10*/  "Unexpected or unmatched END",
	 /* Error 11*/  "Control stack full",
	 /* Error 12*/  "Clause > 500 characters",
	 /* Error 13*/  "Invalid character in data",
	 /* Error 14*/  "Incomplete DO/SELECT/IF",
	 /* Error 15*/  "Invalid hex constant",
	 /* Error 16*/  "Label not found",
	 /* Error 17*/  "Unexpected PROCEDURE",
	 /* Error 18*/  "THEN expected",
	 /* Error 19*/  "String or symbol expected",
	 /* Error 20*/  "Symbol expected",
	 /* Error 21*/  "Invalid data on end of clause",
	 /* Error 22*/  "Cannot open file",
	 /* Error 23*/  "Paradox Error",
	 /* Error 24*/  "Invalid TRACE request",
	 /* Error 25*/  "Invalid sub-keyword found",
	 /* Error 26*/  "Invalid whole number",
	 /* Error 27*/  "Invalid DO syntax",
	 /* Error 28*/  "Invalid LEAVE or ITERATE",
	 /* Error 29*/  "Environment name too long",
	 /* Error 30*/  "Name or string > 250 characters",
	 /* Error 31*/  "Name starts with numeric or \".\"",
	 /* Error 32*/  "Invalid use of stem",
	 /* Error 33*/  "Invalid expression result",
	 /* Error 34*/  "Logical value not 0 or 1",
	 /* Error 35*/  "Invalid expression",
	 /* Error 36*/  "Unmatched \"(\" in expression",
	 /* Error 37*/  "Unexpected \",\" or \")\"",
	 /* Error 38*/  "Invalid template or pattern",
	 /* Error 39*/  "Evaluation stack overflow",
	 /* Error 40*/  "Incorrect call to routine",
	 /* Error 41*/  "Bad arithmetic conversion",
	 /* Error 42*/  "Arithmetic overflow/underflow",
	 /* Error 43*/  "Routine not found",
	 /* Error 44*/  "Function did not return data",
	 /* Error 45*/  "No data specified on function RETURN",
	 /* Error 46*/  "File not found",
	 /* Error 47*/  "File not opened",
	 /* Error 48*/  "Failure in system service",
	 /* Error 49*/  "Interpreter failure"
       };
#endif



#define ERR_PROG_UNREADABLE	3
#define ERR_PROG_INTERRUPT	4
#define ERR_STORAGE_EXHAUSTED	5
#define ERR_UNMATCHED_QUOTE	6
#define ERR_WHEN_EXCEPTED	7
#define ERR_THEN_UNEXCEPTED	8
#define ERR_WHEN_UNCEPTED	9
#define ERR_UNMATCHED_END	10
#define ERR_FULL_CTRL_STACK	11
#define ERR_TOO_LONG_LINE	12
#define ERR_INVALID_CHAR	13
#define ERR_INCOMPLETE_STRUCT	14
#define ERR_INVALID_HEX_CONST	15
#define ERR_UNEXISTENT_LABEL	16
#define ERR_UNEXPECTED_PROC	17
#define ERR_THEN_EXPECTED	18
#define ERR_STRING_EXPECTED	19
#define ERR_SYMBOL_EXPECTED	20
#define ERR_EXTRAD_DATA		21

#define ERR_INVALID_TRACE	24
#define ERR_INV_SUBKEYWORD	25
#define ERR_INVALID_INTEGER	26
#define ERR_INVALID_DO_SYNTAX	27
#define ERR_INVALID_LEAVE	28
#define ERR_ENVIRON_TOO_LONG	29
#define ERR_TOO_LONG_STRING	30
#define ERR_INVALID_START	31
#define ERR_INVALID_STEM	32
#define ERR_INVALID_RESULT	33
#define ERR_UNLOGICAL_VALUE	34
#define ERR_INVALID_EXPRESSION	35
#define ERR_UNMATCHED_PARAN	36
#define ERR_UNEXPECTED_PARAN	37
#define ERR_INVALID_TEMPLATE	38
#define ERR_STACK_OVERFLOW	39
#define ERR_INCORRECT_CALL	40
#define ERR_BAD_ARITHMETRIC	41
#define ERR_ARITH_OVERFLOW	42
#define ERR_ROUTINE_NOT_FOUND	43
#define ERR_NO_DATA_RETURNED	44
#define ERR_DATA_NOT_SPEC	45

#define ERR_PARADOX_ERROR       23

#define ERR_CANT_OPEN_FILE      22
#define ERR_FILE_NOT_FOUND      46
#define ERR_FILE_NOT_OPENED     47

#define ERR_SYSTEM_FAILURE	48
#define ERR_INTERPRETER_FAILURE	49

#endif
