address command
call load "conio.r"
arg fn .
if fn = '?' then do
   say 'syntax: pxview.r <paradox file>'
   exit
end
if fn = '' then fn = getfromdir()
call pxinit
tbl = pxopen(fn)
flds = pxfields(fn)
nrecs = pxrecords(fn)
keys = pxkeys(fn)
if nrecs = 0 then do
   say 'Table' fn 'is empty!'
   call pxclose tbl
   exit
end
m = 0
do i = 1 to flds
   type.i = pxfldtype(tbl,i)
   name.i = pxfldname(tbl,i)
   m = max(m,length(name.i))
end
do forever
   call cls
   call printrecord
   cmd = getch();
   upper cmd
   select
      when cmd = 'Q' | c2d(cmd) = 27 then leave
      when cmd = 4D then call pxseek tbl,"NEXT"
      when cmd = 4B then call pxseek tbl,"PREV"
      when cmd = 47 then call pxseek tbl,"FIRST"
      when cmd = 4F then call pxseek tbl,"LAST"
      when cmd = 'G' then do
         call write ,'Enter record number to go: '
         pull nnn .
         if datatype(nnn,'NUM') then call pxseek tbl,"FIRST",nnn-1
                                else do
                                   say "Invalid number."
                                   "delay 1"
                                end
      end;
      when cmd = 'C' then call changefield
   end
end
call pxclose tbl
exit

printrecord:
  rec = pxrecopen(tbl)
  call pxrecget tbl,rec
  say 'Table = 'fn ' Fields =' flds ' Keys= 'keys 'Records =' nrecs ' Current record='pxseek(tbl)
  say copies('Ä',79)
  do i = 1 to flds
     call write ,right(i,2) left(name.i,m) left(type.i,4) ': "'
     line = pxfldget(rec,i,type.i) || '"'
     l = 3 + m + 5 + 4
     say left(line,79-l)
     line = substr(line,80-l)
     do while line ^= ""
        say copies(' ',l) || left(line,79-l)
        line = substr(line,80-l)
     end
  end
  call pxrecclose rec
return

changefield:
   call write ,'Enter number of field to change type: '
   pull nnn fieldtype .
   if ^datatype(nnn,'NUM') | nnn < 1 | nnn > nflds then do
      say "Invalid number."
      "delay 1"
   end; else do
      if fieldtype = '' then do 
         call write ,'Enter new field type: '
         pull fieldtype .
      end
      type.nnn = fieldtype
   end
return

getfromdir: procedure
'*dir /b *.db (stack'
files = queued()
if files = 0 then do
   say 'No Paradox files found in current directory.'
   exit
end
do i = 1 to files
   pull fn.i '.' .
end
chooseagain:
call cls
say 'Select one of the followings:'
say copies('Ä',79)
do i = 1 to files
   call write ,right(i,4) left(fn.i,15)
end
say
say copies('Ä',79)
call write ,'Input name or number> '
pull selected .
if selected = '' then exit
if datatype(selected,'NUM') then return fn.selected
do i = 1 to files
   if fn.i = selected then return selected
end
