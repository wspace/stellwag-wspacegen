##########################################################
# GNUmakefile/Makefile
#
# Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Publice License,
# version 2 or any later. The license is contained in the COPYING
# file that comes with the wspacegen distribution.
#
# Makefile for wspacegen project
#/

WSDBG_OBJS=fileio.o storage.o wsdbg.o interprt.o debug.o
WSGEN_OBJS=fileio.o storage.o wsgen_ui.o interprt.o debug.o
WSI_OBJS=fileio.o storage.o wsi.o interprt.o
TARGETS=wsdbg wspacegen wsi
OBJS=$(WSDBG_OBJS) $(WSGEN_OBJS) $(WSI_OBJS)

# Be careful when it comes to compiling with -ansi flag on POSIX-like systems!
#
# This might sound strange, but if you compile with '-ansi' you won't get the
# POSIX-dependant features of the whitespace interpreter, which aren't a
# must-have but at least a want-to-have! To make it more clear: wsi tries to
# set the terminal into non-canonical (aka read character by character) mode, so
# that the whitespace scripts gets the chars even before you hit return.

CFLAGS+=-Wall -W -ggdb #-ansi -pedantic-errors
LDFLAGS+=${CFLAGS} #-lefence

all: $(TARGETS) Makefile

wsdbg: ${WSDBG_OBJS}
	$(CC) ${LDFLAGS} -o $@ ${WSDBG_OBJS}

wspacegen: ${WSGEN_OBJS}
	$(CC) ${LDFLAGS} -o $@ ${WSGEN_OBJS}

wsi: ${WSI_OBJS}
	$(CC) $(LDFLAGS) -o $@ $(WSI_OBJS)

.cxx.o:
	$(CC) -I. $(CFLAGS) -c $<

clean:
	rm -f *.[oad] core tags $(TARGETS)


# START OF GNUmakefile ...
#
# GNUmakefile, this Makefile is able to generate dependencies
# automatically. However this requires GNU-style make and 
# a gcc, supporting -M flag.
#
# To generate the common Makefile everything from here on is chopped
# off and replaced by the contents of the *.d files.
#
%.d: %.c
	$(CC) -MM -I. $(CFLAGS) $< >.$@.dep
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < .$@.dep > $@
	rm -f .$@.dep

Makefile: GNUmakefile $(OBJS:.o=.d)
	grep -e "^# START OF GNUmakefile" -n $< > .$@.tmp
	sed -e "`sed -e "s,:.*,," < .$@.tmp`,9999d" < $< > $@
	cat $(OBJS:.o=.d) >> $@
	rm -f .$@.tmp

-include $(OBJS:.o=.d)
