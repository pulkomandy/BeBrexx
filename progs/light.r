/*===================================================================*/
/*     Bill N. Vlachoudis                             1989    o o    */
/*     Computer adDREss.......CACZ0200 AT Grtheun1.EARN     ____oo_  */
/*     HomE AdDreSs...........Parodos Filippoy 51          /||    |\ */
/*   !                        TK 555-35 Pylaia              ||    |  */
/* !  tHe bESt Is             ThesSALONIKI, GREECE          '.___.'  */
/*  Y e t  to   Co mE !                                     MARMITA  */
/*===================================================================*/
/* This is a color design-lightshow, version 2, without control chrs */
address int2e
call load 'conio.r'
n = copies(' ',68);
say n "   o o   "
say n " ____oo_ "
say n "/||    |\"
say n " ||    | "
say n " `.___.' "
say n " MARMITA "
drop n
say '                 LL        IIII    GGGGG    HH    HH  TTTTTTTT (R)'
say '                 LL         II    GG   GG   HH    HH     TT   '
say '                 LL         II    GG        HH____HH     TT   '
say '                 LL         II    GG  GGG   HH    HH     TT   '
say '                 LL     L   II    GG   GG   HH    HH     TT   '
say '                 LLLLLLLL  IIII    GGGGG    HH    HH     TT   '
say;
say '                                  S H O W'
say '               by bill vlachoudis CACZ0200@GRTHEUN1.EARN (c)1988'
say;say;
say '             Warning: you''ll need color terminal to run that prog.'
say '                             also 4DOS is needed.'
say '                       Press <ENTER> to continue...'
pull .
colors = 'BLUE GREEN MAGENTA RED CYAN YELLOW WHITE BLACK'
call cls
Do w = 3 to 50
   Do i = 1 to 19
      Do j = 0 to 19
         k = i + j                /* Design type follows: */
         c = trunc(j * 6 / (i + 7) + I * w / 12)
         if c > 15 then c = c - 15
         xl = i * 2
         xr = 80 - xl
         yt = trunc(k/2 + 1)
         yb = 21 - yt
         if c > 7 then col = 'BRIGHT'; else col = ''
         col = col word(colors,c//7+1) 'ON BLACK'

         call gotoxy xl,yt ; 'COLOR' col; call write ,'лл'
         call gotoxy xr,yt ; 'COLOR' col; call write ,'лл'
         call gotoxy xl,yb ; 'COLOR' col; call write ,'лл'
         call gotoxy xr,yb ; 'COLOR' col; call write ,'лл'
         yt = trunc(i / 2 + 1)
         yb = 21 - yt
         xl = k * 2
         xr = 80 - xl
         call gotoxy xl,yt ; 'COLOR' col; call write ,'лл'
         call gotoxy xr,yt ; 'COLOR' col; call write ,'лл'
         call gotoxy xl,yb ; 'COLOR' col; call write ,'лл'
         call gotoxy xr,yb ; 'COLOR' col; call write ,'лл'
      End
   End
End
Exit 0
 
exit:
'cls'
exit 0
