/* dates.r */
arg day month year
days = 'Sunday Monday Tuesday Wednesday Thrusday Friday Saturday'
say word(days,DayOfWeek(day,month,year)+1)
say 'Easter = 'Easter(year)'/04/'year
say 'Days = 'DaysOfMonth(month,year)
say 'Dates Library'
exit

/* ------------------ DayOfWeek -------------------- */
/* return day of week 0=Sun, 1=Mon, ... 6=Sat  */
DayOfWeek: procedure
	if arg() = 3	then parse arg day,month,year
			else parse arg day '/' month '/' year
	if month < 3 then do
		month = month + 10
		year  = year - 1
	end;
	else month = month - 2
	century = year % 100
	Yr = year // 100
	Dw = (((26*Month - 2) % 10) + Day + Yr + (Yr % 4) +,
		(Century % 4) - (2 * Century)) // 7;
	if Dw < 0	then DayOfWeek = Dw + 7
			else DayOfWeek = Dw
return DayOfWeek

/* ------------------ Easter In April ----------------------- */
/* if Easter > 30 then Month = May, Easter = Easter - 30      */
/* this algoritm works only for Greek Orthodox Easter         */
Easter: procedure
arg year
	a = year // 19;
	b = year // 4;
	c = year // 7;
	d = (19*a+16) // 30;
	e = (2*b + 4*c + 6*d) // 7;
return 3 + d + e;

/* -------------------- daysOfMonth ---------------------- */
daysOfMonth: procedure
daysInMonth = '312831303130313130313031'
	if arg() = 2	then parse arg month,year
			else parse arg month '/' year
	Days = substr(DaysInMonth,Month*2-1,2)
	if Month=2 & Year // 400 ^= 0 then
	if Year // 4 = 0 then Days = 29
			else Days = 28
return Days

/* ------------------ addMonths ------------------- */
addMonths: procedure
	if arg() = 2 then parse arg day '/' month '/' year, months
		else parse arg day,month,year,months

	months = '' then months = 1
	month = month + months
	if months > 0 then do
		do while month > 12
			month = month - 12
			year = year + 1
		end
	end
	else do
		do while month < 1
			month = month + 12
			year = year - 1
		end
	end
return day'/'month'/'year

/* ----------------------- addDays ------------------- */
addDays: procedure
	if arg() = 2 then parse arg day '/' month '/' year, days
			else parge arg day,month,year,days
	if days = '' then days = 1
	day = day + days
	if days > 0 then do
		do while day > DaysOfMonth(month,year)
			day = day - DaysOfMonth(month,year)
			parse value addMonths(day'/'month'/'year,1) with day '/' month '/' year
		end
	end
	else do
		do while day < 1
			parse value addMonths(day'/'month'/'year,-1) with day '/' month '/' year
			day = day + DaysOfMonth(month,year)
		end
	end
return day'/'month'/'year

/* -------------------- swapDate ----------------- */
swapDate: procedure
	if arg()=1 then parse arg day '/' month '/' year
		else parse arg day,month,year
return month'/'day'/'year
