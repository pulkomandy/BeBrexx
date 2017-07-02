/**/
i = 5
b = Hello there
c = "Mia marmita"
a = "Bingo"
l = "Hello"
m = "Pantente"

a = vartree(,'d')
do while a ^= ""
   parse var a d v "0A"x a
   say copies("--",d)"->" v
end
