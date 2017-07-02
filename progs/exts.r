#!/users/vvlachou/bin/rx
/* exts.r, displays all extensions (+info) in a directory */
/* Bill N. Vlachoudis (c) May 1996 */
if arg(1) = "?" then do 
	say "syntax: exts [<dir>]"
	say "desc:   Displays a summary of file extensions in the current directory"
	say "author: Bill N. Vlachoudis"
	say "date:   May 1996"
	exit
end

if arg() > 0 then do
end
'ls -l' arg(1) '(stack'
size. = 0
count. = 0
exts = ""
totsize = 0
files = 0
maxtype = 0
do queued()
	parse pull . . . . size . . . name '.' type .
	if datatype(size,'NUM') then do
		if type="" then type="(none)"
		size.type = size.type + size
		count.type = count.type + 1
		totsize = totsize + size
		files = files + 1
		if count.type = 1 then exts = exts type
		maxtype = max(maxtype,length(type))
	end
end
maxtype = maxtype + 3
line=left("Extensions",maxtype) " Files" right("Size",10)
say line
say copies('-',length(line))
do i = 1 to words(exts)
	type = word(exts,i)
	say left(type,maxtype) format(count.type,6) format(size.type,10)
end
say copies('-',length(line))
say words(exts) "Extensions, " files "Files"
say "Total Size =" totsize"."
