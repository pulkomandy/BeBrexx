/* encode and decode a file */
arg seed filei fileo .
if filei='' | fileo='' | datatype(seed) ^= "NUM" then do
   say "syntax: code password filein fileout"
   say "password must be a integer number"
   exit 1
end
r = random(,,seed)
filein = open(filei,"rb")
fileout = open(fileo,"wb")
if filein=-1 | fileout=-1 then do
   say "Error opening file '"file"'"
   exit 2
end
do forever
   b = read(filein,1)
   if length(b) = 0 then leave
   call write fileout,bitxor(b,d2c(random(0,255)))
end
call close filein
call close fileout
