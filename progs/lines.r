/** count lines **/
call load "files.r"
lines = 0
size = 0
file = "$$$fff$$$"
"ls" arg(1) ">" file
do until eof(file)
   f = read(file)
   call write ,"file" f
   l = lines(f)
   s = filesize(f)
   say format(l,5) format(s,6)
   lines = lines + l
   size = size + s
end
say "total lines =" lines
say "total size  =" size
"del" file
exit 0
