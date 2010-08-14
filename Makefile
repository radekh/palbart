##*********************************************************************
#
# Makefile for the PDP/8 palbart assembler.
#
# System Name:      PAL Assembler
#
# Initial Author:   Gary Messenbrink
#
# Creation Date:    19Apr96
#
# Amendments Record:
#    Version  Date    by   Comments
#    ------- -------  ---  --------------------------------------------
#     v1.1  27Mar02  GAM  Added macro8x as a target.
#     v1.0  24Aug01  GAM  Updated to build palbart-2.4.c
#     v0.0  12Apr96  GAM  Original
#
#
# Synopsis:  Builds PAL cross-assember for generating PDP/8 executable
#            code.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
#**********************************************************************

PROG1  = palbart
SRC1  = palbart-2.5.c
OBJ1  = palbart-2.5.o

PROG2  = macro8x
SRC2  = macro8x.c
OBJ2  = macro8x.o

PROGS  = $(PROG1) $(PROG2)
SRCS  = $(SRC1) $(SRC2)
OBJS  = $(OBJ1) $(OBJ2)

INCLDIRS = -I./
LDLIBS   = -lm

#----------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .o .c .h .s .S
.S.s:
	$(CPP) -traditional $< -o $*.s
.c.s:
	$(CC) $(CFLAGS) -S $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -ansi -c $<

#----------------------------------------------------------------------
#
# Flags
#
#CFLAGS  = -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer $(INCLDIRS)
CFLAGS  = -g -Wall -Wstrict-prototypes $(INCLDIRS)
DEPENDFLAGS = -MM
INSTALLFLAGS = -s
LDFLAGS =

#----------------------------------------------------------------------
#
# Programs
#
AR        = ar
AS        = as
CC        = gcc
CCDEPEND  = $(CC)
CCLINK    = $(CC)
CXX       = g++
CXXLINK   = $(CXX)
CXXDEPEND = $(CXX)
CTAGS     = ctags -w
ECHO      = echo
INSTALL   = install
LD        = ld
MAKE      = make
MKBPMLIST = mkbpmlist
MKDIRHIER = mkdir -p
MV	  = mv -f
NM        = nm
RM        = rm -f
RMDIR     = rm -f -r
# pmbuild must be used with RedHat Linux >= 7.2
RPMBUILD  = rpmbuild
#RPMBUILD  = rpm
SHELL     = /bin/sh
STRIP     = strip

#----------------------------------------------------------------------
#
# Directories
#
TOP        = $(shell pwd)
PREFIX     = /usr/local
LIBDIR     = $(EXEC_PREFIX)/lib
BINDIR     = $(PREFIX)/bin
INCLUDEDIR = $(PREFIX)/include
INFODIR    = $(PREFIX)/info
MANDIR     = $(PREFIX)/man/man$(MANEXT)
MANEXT     = 1

#----------------------------------------------------------------------
#
# RPM directories
#
rpmtop        = $(TOP)/rpm
rpm_build     = $(rpmtop)/BUILD
rpm_rpms      = $(rpmtop)/RPMS/i386
rpm_sources   = $(rpmtop)/SOURCES
rpm_specs     = $(rpmtop)/SPECS
rpm_srpms     = $(rpmtop)/SRPMS
rpm_buildroot = $(rpmtop)/buildroot

RPMFLAGS = -v -bb --define "_topdir $(rpmtop)" --buildroot "$(rpm_buildroot)"

#----------------------------------------------------------------------

all:	$(PROGS)

$(PROG1):
	$(MAKE) -f $(TOP)/Makefile objects=$(OBJ1) program=$(PROG1) build1

$(PROG2):
	$(MAKE) -f $(TOP)/Makefile objects=$(OBJ2) program=$(PROG2) build1

build1: $(objects)
	$(RM) $(program)
	$(CCLINK) -o $(program) $(LDOPTIONS) $(objects) $(LDLIBS) $(EXTRA_LOAD_FLAGS)


install: install.$(PROG1) install.$(PROG2)

install.$(PROG1):
	$(MAKE) -f $(TOP)/Makefile program=$(PROG1) install_prog

install.$(PROG2):
	$(MAKE) -f $(TOP)/Makefile program=$(PROG2) install_prog

install_prog: $(program)
	@if [ -d $(BINDIR) ]; then set +x; \
	else (set -x; $(MKDIRHIER) $(BINDIR)); fi
	$(INSTALL) $(INSTALLFLAGS) $(program) $(BINDIR)


install.man: installman.$(PROG1) installman.$(PROG2)

installman.$(PROG1):
	$(MAKE) -f $(TOP)/Makefile manpage=$(PROG1).$(MANEXT) install_man

installman.$(PROG2):
	$(MAKE) -f $(TOP)/Makefile manpage=$(PROG2).$(MANEXT) install_man

install_man: $(manpage)
	$(INSTALL) $(MANDIR).$(MANEXT) $(manpage)


emptyrule::

clean::
	$(RM) *.o *.ln *.BAK *.bak *.CKP .emacs_*
	$(RM) core errs tags TAGS make.log MakeOut
	$(RM) ,* *~ "#"*

distclean:: clean
	$(RM) $(PROGS)
	$(RM) *.rpm

realclean:: distclean

tags::
	$(TAGS) -w *.[ch]
	$(TAGS) -xw *.[ch] > TAGS

#----------------------------------------------------------------------

rpms:	rpm.$(PROG1) rpm.$(PROG2)

rpm.$(PROG1):
	$(MAKE) -f $(TOP)/Makefile program=$(PROG1) specfile=$(PROG1).spec rpm_program

rpm.$(PROG2):
	$(MAKE) -f $(TOP)/Makefile program=$(PROG2) specfile=$(PROG2).spec rpm_program

rpm_program: $(specfile) $(program) rpm_dirs rpm_install
	$(RM) $(rpm_rpms)/$(program)*.rpm
	$(RPMBUILD) $(RPMFLAGS) $(specfile)
	$(MV) $(rpm_rpms)/$(program)*.rpm $(TOP)
	$(RMDIR) $(rpmtop)

rpm_dirs:
	@$(MKDIRHIER) $(rpm_build)
	@$(MKDIRHIER) $(rpm_rpms)
	@$(MKDIRHIER) $(rpm_sources)
	@$(MKDIRHIER) $(rpm_specs)
	@$(MKDIRHIER) $(rpm_srpms)
	@$(MKDIRHIER) $(rpm_build)

rpm_install: $(program)
	$(MKDIRHIER) $(rpm_buildroot)$(BINDIR)
	$(INSTALL) $(INSTALLFLAGS) $(program) $(rpm_buildroot)$(BINDIR)

#----------------------------------------------------------------------

depend::
	$(CCDEPEND) $(DEPENDFLAGS) $(INCLDIRS) $(SRCS) > .depend

#----------------------------------------------------------------------
#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
