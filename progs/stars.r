/* display random stars */
call load "conio.r"
call cls
colors = "white red yellow blue green magenta"
address int2e
do 100
   x = random(0,79)
   y = random(0,24)
   call gotoxy x,y
   call write ,'*'
   "color" word(colors,random(1,words(colors))) " on black"
/*   address int2e 'delay 1' */
   call gotoxy x,y
   call write ,'.'
end
'color green on black'
