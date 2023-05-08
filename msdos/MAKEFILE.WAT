# This makes the text adventure game Lugi, using Open Watcom C++ for Windows.
# The source is mostly generic C, compilable by almost anything.
# Invoke from C source directory with:   wmake -h -f msdos\makefile.wat

.ERASE
.EXTENSIONS:
.EXTENSIONS: .obj .c

!ifdef DEBUG
cdebug = -od -d2 -q -dDEBUG
ldebug = d w all op q op symf
!else
cdebug = -s -obhikl+rt -oe=100 -zp8 -q
# -oa helps slightly but might be dangerous.
ldebug = op el op q
!endif

CFLAGS = $(cdebug) -bt=DOS -0 -ml -zt -zq -wx
LFLAGS = $(ldebug) sys DOS
O = msdos\ob\     # comment so backslash won't continue the line

ZOB1 = $(O)basics.obj $(O)diction.obj $(O)embassy.obj $(O)player.obj
ZOB2 = $(O)roomz.obj $(O)take.obj $(O)put.obj $(O)drop.obj $(O)gothere.obj
ZOB = $(ZOB1) $(ZOB2) $(O)fiddle.obj $(O)upchuck.obj $(O)obey.obj $(O)lugi.obj

msdos\LUGI.EXE : $(ZOB)
	wlink $(LFLAGS) name msdos\LUGI.EXE file { $(ZOB) }
	-@echo --
	-@dir msdos\LUGI.EXE

$(ZOB) : lugi.h

# gah, this make won't do this automatically if not in the same directory:
$(O)basics.obj  : basics.c
$(O)diction.obj : diction.c
$(O)embassy.obj : embassy.c
$(O)player.obj  : player.c
$(O)roomz.obj   : roomz.c
$(O)take.obj    : take.c
$(O)put.obj     : put.c
$(O)drop.obj    : drop.c
$(O)gothere.obj : gothere.c
$(O)fiddle.obj  : fiddle.c
$(O)upchuck.obj : upchuck.c
$(O)obey.obj    : obey.c
$(O)lugi.obj    : lugi.c

.c.obj :
	wcc $(CFLAGS) $[@ -fo=$@

# we don't need this as it alreawdy defines MSDOS
#$(O)basics.obj : basics.c
#	wcc -dMSDOS $(CFLAGS) basics.c -fo=$@

clean: .SYMBOLIC
	-del $(O)*.obj msdos\lugi.exe msdos\lugi.sym
