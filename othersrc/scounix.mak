.c.o:
        cc -c -O $<

#for debuging cc -g -c $<


EXE_dependencies = \
rexx.o \
builtin.o \
convert.o \
error.o \
expr.o \
instruct.o \
i_cmd.o \
i_do.o \
lstring.o \
memory.o \
nextsymb.o \
rexxfunc.o \
rxconv.o \
rxfiles.o \
rxmath.o \
rxstr.o \
rxword.o \
template.o \
utils.o \
variable.o

rx: $(EXE_dependencies)
        cc -orx $(EXE_dependencies) -lm -lc
