.AUTODEPEND

.PATH.obj = OBJ

#		*Translator Definitions*
CC = bcc +REXX.CFG
TASM = TASM
TLINK = tlink

#		*Implicit Rules*
.c.obj:
  $(CC) -c $<

.a.obj:
  $(TASM) /ml $<

#		*List Macros*

PXLIB = f:\pxengine\c\lib

EXE_dependencies =  \
 rexx.obj \
 nextsymb.obj \
 instruct.obj \
 i_cmd.obj \
 i_do.obj \
 template.obj \
 expr.obj \
 rexxfunc.obj \
 builtin.obj \
 rxstr.obj \
 rxword.obj \
 rxmath.obj \
 rxconv.obj \
 rxfiles.obj \
 pxdb.obj \
 error.obj \
 variable.obj \
 utils.obj \
 memory.obj \
 lstring.obj \
 convert.obj \
 pxmsg.obj \
 obj\int2e.obj

#		*Explicit Rules*
obj\rx.exe: rexx.cfg $(EXE_dependencies)
  $(TLINK) /x /c /P- /yx /L$(LIBPATH) /o @&&|
/o- c0l.obj+
obj\rexx.obj+
obj\nextsymb.obj+
obj\instruct.obj+
obj\i_cmd.obj+
obj\i_do.obj+
obj\template.obj+
obj\expr.obj+
obj\rexxfunc.obj+
obj\builtin.obj+
obj\rxstr.obj+
obj\rxword.obj+
obj\rxmath.obj+
obj\rxconv.obj+
obj\rxfiles.obj+
/o+ obj\pxdb.obj /o- +
obj\error.obj+
obj\variable.obj+
obj\utils.obj+
obj\memory.obj+
obj\lstring.obj+
obj\convert.obj+
obj\int2e.obj+
/o+ obj\pxmsg.obj
obj\rx
		# no map file
/o+$(PXLIB)\pxengtcl.lib+
/o- overlay.lib+
emu.lib+
mathl.lib+
cl.lib
|


#		*Individual File Dependencies*
rexx.obj:     rexx.c

nextsymb.obj: nextsymb.c

instruct.obj: instruct.c

i_cmd.obj:    i_cmd.c

i_do.obj:     i_do.c

template.obj: template.c

expr.obj:     expr.c

rexxfunc.obj: rexxfunc.c

builtin.obj:  builtin.c

rxstr.obj:    rxstr.c

rxword.obj:   rxword.c

rxmath.obj:   rxmath.c

rxconv.obj:   rxconv.c

rxfiles.obj:  rxfiles.c

pxdb.obj:     pxdb.c

error.obj:    error.c

variable.obj: variable.c

utils.obj:    utils.c

memory.obj:   memory.c

lstring.obj:  lstring.c

convert.obj:  convert.c

pxmsg.obj:    pxmsg.c

#		*Compiler Configuration File*
rexx.cfg: rexx.mak
  copy &&|
-G-
-O
-d
-ml
-1
-Y
-h
-H=REXX.SYM
-wpin
-wamb
-wamp
-wasm
-wpro
-wcln
-wdef
-wsig
-wnod
-wstv
-wucp
-wuse
-nOBJ
-I$(INCPATH)
-L$(LIBPATH)
-P-.C
| rexx.cfg


