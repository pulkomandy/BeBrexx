/* SOUNDEX.R      Created Nov 1994 by Bill Vlachoudis */
/* èò§‡ ©ú £†ò ©°¶ß†ò ©´û§ ß¨¢û ´û™ Ñ/ÉåÑ             */
arg file options .
if file = '' then do 
   say 'syntax: soundex.r [directory\]name options'
   say 'searches directory for a file that the FILENAME sounds like "NAME"'
   say 'options can be "S" for subdirectory'
   say '4DOS is required!"
   exit 
end
backslash = lastpos("\",file)
dir = ''
if backslash<>0 then dir = left(file,backslash)
file = substr(file,backslash+1)
if left(options,1) = 'S' | ,
   left(options,2) = '/S'  then options = '/S'
                           else options = ''
'*dir /B' dir options '(stack'
soundfile = soundex(file)
do queued()
   parse pull file
   bs = lastpos("\",file)
   dir = left(file,bs)
   file = substr(file,bs+1)
   parse var file fn '.' ft
   if soundex(fn) = soundfile then say dir || file
end
