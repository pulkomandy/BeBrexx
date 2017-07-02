/* ----------------------------------------- (c)BNV1993 -- *
 * Paradox extended functions for REXX                     *
 * ------------------------------------------------------- */
say 'This is a REXX library for paradox, cannot run alone'
exit 3;

/* Opens a Paradox .db file and creates a tbl array of fieldtypes */
/* opendb(pdox_file[,tbl_name]) */
opendb:
   if arg()=0 | arg()>2 then return 0
   if ^pxexist(arg(1))  then  return 0
   push tbl
   push i
   push n
   if arg()=2 then tbl = arg(2);
              else tbl = arg(1);
   interpret tbl".0.0 = pxopen(arg(1))"
   n = pxfields(value(tbl".0.0"))
   interpret tbl".0 = n"
   do i = 1 to n
      interpret tbl".i.0 = pxfldtype(tbl,i)"
   end
   do i = 1 to n
      interpret tbl".i.1 = pxfldname(tbl,i)"
   end
   pull n
   pull i
   pull tbl
return 1

/* closes a .db file using the name of the tbl and drops the tbl array */
closedb:
   call pxclose value(arg(1)".0.0")
   interpret "drop" arg(1)"."
return

/* read a complete record using the tbl array */
getrecord:
   push rec
   push i
   push n
   rec = pxrecopen(value(arg(1)".0.0"))
   call pxrecget value(arg(1)".0.0"),rec
   n = value(arg(1)".0")
   do i = 1 to n
      interpret arg(1)".i = pxfldget(rec,i,"arg(1)".i.0)"
   end
   call pxrecclose rec
   pull  n
   pull  i
   pull  rec
return

/* inserts a complete record */
insrecord:
   push rec
   push i
   push n
   call makerecord arg(1)
   call pxrecinsert value(arg(1)".0.0"),rec
   call pxrecclose rec
   pull  n
   pull  i
   pull  rec
return

updrecord:
   push rec
   push i
   push n
   call makerecord arg(1)
   call pxrecupdate value(arg(1)".0.0"),rec
   call pxrecclose rec
   pull  n
   pull  i
   pull  rec
return

apprecord:
   push rec
   push i
   push n
   call makerecord arg(1)
   call pxrecappend value(arg(1)".0.0"),rec
   call pxrecclose rec
   pull  n
   pull  i
   pull  rec
return

/* used from insrecord, updrecord, apprecord */
makerecord:
   rec = pxrecopen(value(arg(1)".0.0"))
   n = value(arg(1)".0")
   do i = 1 to n
      interpret "call pxfldput rec,i,"arg(1)".i.0,"arg(1)".i"
   end
return
