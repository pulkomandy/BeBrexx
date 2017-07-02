.AUTODEPEND

#               *Translator Definitions*
CC = bcc +REXX.CFG
ASM = tasm
LINK = tlink


.path.obj = OBJ\

#               *Implicit Rules*
.c.obj:
  $(CC) -c $<

.a.obj:
  $(TASM) /ml $<

#               *List Macros*


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
  error.obj \
  variable.obj \
  utils.obj \
  memory.obj \
  lstring.obj \
  convert.obj \
  int2e.obj

#               *Explicit Rules*
rx.exe: rexx.cfg $(EXE_dependencies)
  $(LINK) /x /c /P- /yx /L$(LIBPATH) @&&|
c0l.obj+
OBJ\rexx.obj+
OBJ\nextsymb.obj+
OBJ\instruct.obj+
OBJ\i_cmd.obj+
OBJ\i_do.obj+
OBJ\template.obj+
OBJ\expr.obj+
OBJ\rexxfunc.obj+
OBJ\builtin.obj+
OBJ\rxstr.obj+
OBJ\rxword.obj+
OBJ\rxmath.obj+
OBJ\rxconv.obj+
OBJ\rxfiles.obj+
OBJ\error.obj+
OBJ\variable.obj+
OBJ\utils.obj+
OBJ\memory.obj+
OBJ\lstring.obj+
OBJ\convert.obj+
OBJ\int2e.obj
rx
                # no map file
emu.lib+
mathl.lib+
cl.lib
|


#               *Individual File Dependencies*
rexx.obj: rexx.c

nextsymb.obj: nextsymb.c

instruct.obj: instruct.c

i_cmd.obj: i_cmd.c

i_do.obj: i_do.c

template.obj: template.c

expr.obj: expr.c

rexxfunc.obj: rexxfunc.c

builtin.obj: builtin.c

rxstr.obj: rxstr.c

rxword.obj: rxword.c

rxmath.obj: rxmath.c

rxconv.obj: rxconv.c

rxfiles.obj: rxfiles.c

error.obj: error.c

variable.obj: variable.c

utils.obj: utils.c

memory.obj: memory.c

lstring.obj: lstring.c

convert.obj: convert.c

int2e.obj: int2e.a
	$(ASM) /ml int2e.a
	move int2e.obj obj\ 

#               *Compiler Configuration File*
rexx.cfg: makefile
  copy &&|
-3
-G-
-ml
-d
-O
-Z
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
-I$(INCPATH)
-L$(LIBPATH)
-nOBJ
-P-.C
| rexx.cfg
