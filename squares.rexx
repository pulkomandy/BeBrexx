#! /boot/home/config/bin/rx
/** Squares.rexx
*
*   Draws random squares with Squares the program
*
*   Optionally can be invoked with the arguments:
*   x1 y1 x2 y2 to set the Squares window size
*
**/
call OpenConsole()
if showlist('R', 'Squares1') = 0 then do
   say "Squares is not yet running, I will start it"
   "/boot/home/config/bin/Squares &"
   "sleep 2"

   if showlist('R', 'Squares1') = 0 then do
      say "Still can't find Squares, please start it yourself, and run me again"
      exit 0
   end
end

/*
*   First do some "native BeOS scripting"
*/
address 'Squares'

"get Title of Window 0"
say "I found Squares running. Its window name is" RESULT

say "I will now change it to S q u a r e s"
'set Title of Window 0 to "S q u a r e s"'

parse arg xx1 xx2 yy1 yy2

if strip(yy2) == "" then do
   say "No arguments given, will use default window size"
end
else do
   say "Will set new window size"
   "set Frame of Window 0 to BRect(" || xx1 || "," || xx2 || "," || yy1 || "," || yy2 || ")"
end

/*
*   Now try some user-level scripting
*/
address "Squares1"

"getsize"
parse var RESULT x1 y1 x2 y2

width  = (x2 - x1) % 1
height = (y2 - y1) % 1

starttime = time()
do i = 1 to 10000
   red   = random(0, 255)
   blue  = random(0, 255)
   green = random(0, 255)

   x     = random(0, width)
   y     = random(0, height)

   w     = random(0, width - x)
   h     = random(0, height - y)

   "rgb" red blue green
   "rect" x y x + w y + h
end
totaltime = time('e')

address "Squares1" "quit"

say "Displayed" 10000/totaltime "squares per second"

exit 0
