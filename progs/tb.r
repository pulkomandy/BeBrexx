/*===================================================================*/
/*     Bill N. Vlachoudis                                     o o    */
/*     Computer adDREss.......CACZ0200 AT Grtheun1.EARN     ____oo_  */
/*     HomE AdDreSs...........Parodos Filippoy 51          /||    |\ */
/*   !                        TK 555-35 Pylaia              ||    |  */
/* !  tHe bESt Is             ThesSALONIKI, GREECE          '.___.'  */
/*  Y e t  to   Co mE !                                     MARMITA  */
/*===================================================================*/
call load 'conio.r'
/* time banner by Bill Vlachoudis.*/
A.0.1 = ' 00000000 ';A.0.2 = '00      00';A.0.3 = '00      00';
A.0.4 = '00      00';A.0.5 = '00      00';A.0.6 = ' 00000000 ';
A.1.1 = '    11    ';A.1.2 = '  1111    ';A.1.3 = '    11    ';
A.1.4 = '    11    ';A.1.5 = '    11    ';A.1.6 = '  111111  ';
A.2.1 = '  2222222 ';A.2.2 = '22      22';A.2.3 = '      22  ';
A.2.4 = '    22    ';A.2.5 = '  22    22';A.2.6 = '2222222222';
A.3.1 = '3333333333';A.3.2 = '        33';A.3.3 = '      333 ';
A.3.4 = '        33';A.3.5 = '33      33';A.3.6 = '  333333  ';
A.4.1 = '      44  ';A.4.2 = '    4444  ';A.4.3 = '  44  44  ';
A.4.4 = '44    44  ';A.4.5 = '4444444444';A.4.6 = '      44  ';
A.5.1 = '5555555555';A.5.2 = '55        ';A.5.3 = '55555555  ';
A.5.4 = '        55';A.5.5 = '55      55';A.5.6 = '  555555  ';
A.6.1 = '  666666  ';A.6.2 = '66        ';A.6.3 = '66666666  ';
A.6.4 = '66      66';A.6.5 = '66      66';A.6.6 = '  666666  ';
A.7.1 = '7777777777';A.7.2 = '77      77';A.7.3 = '        77';
A.7.4 = '      77  ';A.7.5 = '    77    ';A.7.6 = '    77    ';
A.8.1 = '  888888  ';A.8.2 = '88      88';A.8.3 = '  888888  ';
A.8.4 = '88      88';A.8.5 = '88      88';A.8.6 = '  888888  ';
A.9.1 = '  999999  ';A.9.2 = '99      99';A.9.3 = '99      99';
A.9.4 = '  99999999';A.9.5 = '        99';A.9.6 = '  999999  ';
d.1 = '    ';d.2 = ' II ';d.3 = '    ';d.4 = ' II ';d.5 = ' II ';d.6 = '    '
'cls'
if arg() = 1 then times = arg(1); else times = 10
t = time()
DO times
    do while t = told
      t = time()
    end
    told = t
    t1 = substr(t,1,1);t2 = substr(t,2,1);t3 = substr(t,4,1)
    t4 = substr(t,5,1);t5 = substr(t,7,1);t6 = substr(t,8,1)
c = random(1,4)
/*if c = 1 then col = 'red'
if c = 2 then col = 'gre'
if c = 3 then col = 'yel'
if c = 4 then col = 'tur'*/
call gotoxy 0,0
/*'EXECIO 0 CP (STRING SCREEN VMO 'COL' NON' */
say;say;say;say;say;say;say
say '  'a.t1.1 a.t2.1 d.1 a.t3.1 a.t4.1 d.1 a.t5.1 a.t6.1
say '  'a.t1.2 a.t2.2 d.2 a.t3.2 a.t4.2 d.2 a.t5.2 a.t6.2
say '  'a.t1.3 a.t2.3 d.3 a.t3.3 a.t4.3 d.3 a.t5.3 a.t6.3
say '  'a.t1.4 a.t2.4 d.4 a.t3.4 a.t4.4 d.4 a.t5.4 a.t6.4
say '  'a.t1.5 a.t2.5 d.5 a.t3.5 a.t4.5 d.5 a.t5.5 a.t6.5
say '  'a.t1.6 a.t2.6 d.6 a.t3.6 a.t4.6 d.6 a.t5.6 a.t6.6
end
/*'cp screen vmo whi non' */
