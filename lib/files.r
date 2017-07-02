/*
        A small library of files routines.
        Bill N. Vlachoudis   V.Vlachoudis@cern.ch
*/

say "This is a library file, and cannot run alone..."
exit 1

/* ----------- return file size, or -1 if file not found --------------- */
filesize: procedure
trace o
file = open(arg(1),"rb")
if file = -1 then return -1
size = seek(file,0,"eof")
call close file
return size

/* ----------------- return 0 or 1 if file exists -------------- */
state: procedure
trace o
file = open(arg(1),"rb")
if file = -1 then return 0
call close file
return 1

/* the same as state */
exist: return state(arg(1))

/*-------------------  count the lines of a file ----------------*/
lines: procedure
trace o
file = open(arg(1),"r")
if file = -1 then return -1
do lines=1 until eof(file)
	a = read(file)
end
call close file
return lines


/*
 * I'm not sure for the following routines but I assume
 *  that is something like that
 */

/* -----------------------  charin ------------------- */
charin: procedure
trace o
parse arg fn,c
if c = "" then c = 1
if c>1 then call read fn,c-1
return read(arg(1),1)

/* ----------------------- linein -------------------- */
linein: procedure
return read(arg(1))

/* ----------------------- charout -------------------- */
charout: procedure
call write arg(1),arg(2)
return

/* ----------------------- lineout -------------------- */
lineout: procedure
call write arg(1),arg(2),nl
return
