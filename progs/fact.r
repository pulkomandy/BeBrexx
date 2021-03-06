/* recursion test of factorial */
/* note: this implementation of rexx uses two kinds of
         of numbers integers (C long numbers -2E9 till 2E9),
         and real (C double precision numbers) it is better
         to use realnumber ie 45.0 instead of 45 for large
         factorials since the later will result to a zero number
         (15 is the largest integer factorial that can be calculated
          with out any error )
         */
if arg() ^= 1 then do
   say 'Enter a number'
   pull num
end; else do
   num = arg(1)
end

if datatype(num) ^= 'NUM' | num < 0 then do
   say 'Invalid number "'num'"'
   exit 2
end

/* you can even translate the number to real with the following instr */
/* num = num + 0.0 /* so from now on num will be treated as real */ */

say num'! = 'fact(num)
exit

fact: procedure
if arg(1)<=0 then return 1
return  fact(arg(1)-1)*arg(1)
