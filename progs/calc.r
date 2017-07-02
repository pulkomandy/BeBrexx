if arg(1) = "?" then do 
	say "syntax: calc [<expr>]"
	say "desc:   Scientific calculator, either command line or input mode"
	say "author: Bill N. Vlachoudis"
	say "date:   Apr 1996"
	exit
end

if arg() > 0 then do
   interpret "say" arg(1)
   exit
end

call openconsole("Calc");
call setprompt("Calc> ");

trace o
signal on error
signal on syntax

pi = 3.1415927
mp = 1.66054E-27
mn = 1.008664904*mp

mainloop:
do forever
   pull line
   line = space(line,0)
   if line="END" | line="QUIT" then do
      call closeconsole
      exit
   end
   parse var line front "=" rest
   if rest <> "" then do
      if front="" then signal error
      if pos("(",front) = 0 then do
	 interpret line
	 $ = value(front)
	 say front "=" $
      end; else do
	 parse var front name "(" vars ")" .
	 interpret name "= rest"
	 say name"("vars") = "rest
      end
   end; else do
      if line <> "" then interpret "$ =" line
      say ' =' $
   end
end
exit

error:
  say "+++ Error in expression +++"
  signal mainloop

syntax:
  say "+++ Error in expression +++"
  signal mainloop

