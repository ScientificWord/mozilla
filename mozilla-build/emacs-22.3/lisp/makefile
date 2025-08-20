# Start of settings from configure.bat 
COMPILER=gcc
MCPU_FLAG=-mtune=pentium4
# End of settings from configure.bat
 
#  -*- Makefile -*- definition file for building GNU Emacs on Windows NT.
#  Copyright (C) 2000, 2001, 2002, 2003, 2004,
#    2005, 2006, 2007, 2008 Free Software Foundation, Inc.
#
#  GNU Emacs is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3, or (at your option)
#  any later version.
#
#  GNU Emacs is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with GNU Emacs; see the file COPYING.  If not, write to
#  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.

# Ensure 'all' is the default target
all:

# NOTES
#
# I tried to force gmake to use the native shell for simplicity, by
# setting SHELL as below, but this didn't work reliably because of
# various case sensitivity niggles.  Specifically, COMSPEC (which is in
# fact usually spelled ComSpec on NT, to make life difficult) typically
# references "cmd.exe" (on NT) when the file is actually called
# "CMD.EXE" on disk for hysterical raisons.  As a result, GNU make
# thinks it doesn't exist (unless compiled with a switch to ignore
# case), and so doesn't change which shell it will invoke to execute
# commands.
#
# It would be possible, though very tedious using just gmake facilities,
# to convert the COMSPEC value to uppercase to solve this problem, but
# it isn't worth it.  That is partly because, even when using the native
# shell, gmake tends to not be happy with backslashes in command
# strings.  The obvious solution is to use forward slashes as much as
# possible, which can be made to work most of the time (putting
# filenames in quotes often helps), but there are still some internal
# cmd.exe commands like `del' and `copy' that won't work with them.
# Although it is possible to convert slashes to backslashes when
# necessary, gmake requires explicitly calling its subst function, which
# nmake does not understand).  For this reason, it is simplest to
# mandate that rm and cp be available, so we can use Unix-format file
# names everywhere.  (Fortunately both MS and GNU make, and the
# respective compilers, are happy with Unix-format names.)
#
# Since we cannot easily force the choice of a particular shell, we must
# make the effort to cope with whichever shell is being used.
# Fortunately, the only command we need to use that is shell specific is
# the testing of a file's existence for the purpose of working out when
# we are copying files to their original location.  That particular
# requirement is abstracted easily enough.
#
# The only other problem area was the change of directory when running
# temacs to dump emacs.exe (where gmake doesn't support cd foo in any
# useful way), but that has been resolved by modifying the Windows
# unexec function slightly to not require the directory change while
# still allowing objects and binaries to be in subdirectories.

# This doesn't work.
#SHELL:=$(COMSPEC)

# Determine whether make is using sh or cmd/command as shell; cmd.exe
# will output "ECHO is on" when echo is given by itself, while sh will
# not produce any output.
sh_output := $(shell echo)
ifeq "$(findstring ECHO, $(sh_output))" "ECHO"
THE_SHELL = $(COMSPEC)$(ComSpec)
SHELLTYPE=CMD
else
USING_SH = 1
THE_SHELL = $(SHELL)
SHELLTYPE=SH
endif

MAKETYPE=gmake

# The following "ifeq" does not appear to DTRT, and therefore breaks
# the build on mingw32. Also the -m option does not exist in many
# (reasonably recent even) versions of Cygwin. These issues need to be
# remedied before putting this cygpath kludge back in.

# Convert CURDIR to native file name, if in Cygwin format
#ifeq "$(shell cygpath $(CURDIR))" "$(CURDIR)"
#CURDIR	        := $(shell cygpath -m $(CURDIR))
#endif

THISDIR		= .

# Cygwin has changed quoting rules somewhat since b20, in a way that
# affects makefiles using sh as the command processor, so we need to
# detect which rules to use.
ifdef USING_SH
sh_output := $(shell echo [Please ignore a syntax error on the next line - it is intentional] 1>&2)
sh_output := $(shell echo foo")
ifeq "$(sh_output)" ""
NEW_CYGWIN = 1
endif

# By default, newer versions of Cygwin mess with NTFS ACLs in an
# attempt to emulate traditional posix file permissions. This can
# cause bad effects, such as .exe files that are missing the
# FILE_EXECUTE/FILE_GENERIC_EXECUTE permissions when they are created
# with Cygwin commands that don't expect to be creating executable
# files. Then when we later use a non-Cygwin program to create the
# real .exe, the previous Cygwin defined ACL sticks.
CYGWIN=nontsec
export CYGWIN
endif

ALL_DEPS	= $^
EMPTY =
SPACE = $(EMPTY) $(EMPTY)

SUBSYSTEM_WINDOWS=-Wl,-subsystem,windows
SUBSYSTEM_CONSOLE=-Wl,-subsystem,console

# INSTALL_DIR is the directory into which emacs will be installed.
#
ifndef INSTALL_DIR
INSTALL_DIR     = $(CURDIR)/..
endif

export EMACSLOADPATH

# Determine the architecture we're running on.
# Define ARCH for our purposes;
# Define CPU for use by ntwin32.mak;
# Define CONFIG_H to the appropriate config.h for the system;
#
ifdef PROCESSOR_ARCHITECTURE
# We're on Windows NT
CPU		= $(PROCESSOR_ARCHITECTURE)
CONFIG_H	= config.nt
OS_TYPE		= windowsnt
 ifeq "$(PROCESSOR_ARCHITECTURE)" "x86"
ARCH		= i386
CPU		= i386
 else
  ifeq "$(PROCESSOR_ARCHITECTURE)" "MIPS"
ARCH		= mips
  else
   ifeq "$(PROCESSOR_ARCHITECTURE)" "ALPHA"
ARCH		= alpha
   else
    ifeq "$(PROCESSOR_ARCHITECTURE)" "PPC"
ARCH		= ppc
    else
error Unknown architecture type "$(PROCESSOR_ARCHITECTURE)"
    endif
   endif
  endif
 endif
else
# We're on Windows 95
ARCH		= i386
CPU		= i386
CONFIG_H	= config.nt
OS_TYPE		= windows95
endif

AR		= ar -rsc
AR_OUT		=
CC		= gcc
CC_OUT		= -o$(SPACE)
LINK		= gcc
LINK_OUT	= -o$(SPACE)
RC		= windres -O coff
RC_OUT		= -o$(SPACE)
RC_INCLUDE	= --include-dir$(SPACE)

libc		=
baselibs	=
O		= o
A		= a

BASE_LIBS	= $(libc) $(baselibs)

ADVAPI32	= -ladvapi32
COMCTL32	= -lcomctl32
COMDLG32	= -lcomdlg32
GDI32		= -lgdi32
MPR		= -lmpr
SHELL32		= -lshell32
USER32		= -luser32
WSOCK32		= -lwsock32
WINMM		= -lwinmm
WINSPOOL	= -lwinspool
OLE32		= -lole32

ifdef NOOPT
DEBUG_CFLAGS	= -DEMACSDEBUG
else
DEBUG_CFLAGS	=
endif
CFLAGS          = -I. -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0400 $(ARCH_CFLAGS) -D$(ARCH) \
		  -D_CRTAPI1=_cdecl \
		  $(DEBUG_CFLAGS) $(USER_CFLAGS) $(LOCAL_FLAGS)
EMACS_EXTRA_C_FLAGS = -DUSE_CRT_DLL=1

# see comments in allocate_heap in w32heap.c before changing any of the
# -stack, -heap, or -image-base settings.
TEMACS_EXTRA_LINK = -Wl,-stack,0x00800000 -Wl,-heap,0x00100000 -Wl,-image-base,0x01000000 $(SUBSYSTEM_CONSOLE) -Wl,-entry,__start -Wl,-Map,$(BLD)/temacs.map

ifdef NOOPT
OBJDIR          = oo
else
OBJDIR          = oo-spd
endif
$(OBJDIR):;	-mkdir "$(OBJDIR)"
BLD             = $(OBJDIR)/$(ARCH)
stamp_BLD:      $(OBJDIR)
		-mkdir "$(BLD)"
		echo $(BLD) > $@

COMPILER_TEMP_FILES =

CP		= cp -f
CP_DIR		= cp -rf
DEL		= rm
DEL_TREE	= rm -r

ifdef USING_SH

IFNOTSAMEDIR	= if [ ! -s ../same-dir.tst ] ; then
FOREACH		= for f in
FORVAR		= $${f}
FORDO		= ; do
ENDFOR		= ; done
ENDIF		= ; fi
ARGQUOTE	= '
ifdef NEW_CYGWIN
DQUOTE		= "
else
DQUOTE		= ""
endif

else

IFNOTSAMEDIR	= if not exist ../same-dir.tst
FOREACH		= for %%f in (
FORVAR		= %%f
FORDO		= ) do
ENDFOR		=
ENDIF		=
ARGQUOTE	= "
DQUOTE		= \"

endif

ifdef NODEBUG
DEBUG_FLAG =
DEBUG_LINK =
else
DEBUG_FLAG = -gstabs+ -g3
DEBUG_LINK = -gstabs+ -g3
endif

ifdef NOCYGWIN
NOCYGWIN = -mno-cygwin
endif

ifeq "$(ARCH)" "i386"
ifdef NOOPT
ARCH_CFLAGS     = -D_X86_=1 -c $(DEBUG_FLAG) $(NOCYGWIN)
else
ARCH_CFLAGS     = -D_X86_=1 -c $(DEBUG_FLAG) $(NOCYGWIN) $(MCPU_FLAG) -O2 \
		  # -fbuiltin \
		  # -finline-functions \
		  # -fomit-frame-pointer
endif
ARCH_LDFLAGS	= $(SYS_LDFLAGS)
else
ERROR Unknown architecture type "$(ARCH)".
endif

LINK_FLAGS	= $(ARCH_LDFLAGS) $(DEBUG_LINK) $(NOCYGWIN) $(USER_LDFLAGS)

export XMFLAGS

.DEFAULT:

$(BLD)/%.o: %.c
		$(CC) $(CFLAGS) $(CC_OUT)$@ $<

# arch-tag: 35eb9662-8534-4bcf-b891-0730a09d657f
#  -*- Makefile -*- for GNU Emacs on the Microsoft W32 API.
#  Copyright (C) 2000, 2001, 2002, 2003, 2004,
#                2005, 2006, 2007, 2008  Free Software Foundation, Inc.
#
#  This file is part of GNU Emacs.
#
#  GNU Emacs is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3, or (at your option)
#  any later version.
#
#  GNU Emacs is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with GNU Emacs; see the file COPYING.  If not, write to the
#  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.
#

ALL =

all: $(ALL)

SQUOTE='
# '

lisp = $(CURDIR)
srcdir = $(CURDIR)/..

# You can specify a different executable on the make command line,
# e.g. "make EMACS=../src/emacs ...".

EMACS = $(THISDIR)/../bin/emacs.exe

# Command line flags for Emacs.  This must include --multibyte,
# otherwise some files will not compile.

EMACSOPT = -batch --no-init-file --no-site-file --multibyte

# Set EMACSLOADPATH correctly (already defined in environment).
EMACSLOADPATH=$(lisp)

lisptagsfiles1 = $(lisp)/*.el
lisptagsfiles2 = $(lisp)/*/*.el
ETAGS = "../lib-src/$(BLD)/etags"

# Files to compile before others during a bootstrap.  This is done to
# speed up the bootstrap process.  The CC files are compiled first
# because CC mode tweaks the compilation process, and requiring
# cc-mode when it is not compiled doesn't work during the
# bootstrapping.

COMPILE_FIRST = \
	$(lisp)/emacs-lisp/byte-opt.el \
	$(lisp)/emacs-lisp/bytecomp.el \
	$(lisp)/subr.el \
	$(lisp)/progmodes/cc-mode.el \
	$(lisp)/progmodes/cc-vars.el

# The actual Emacs command run in the targets below.
# The quotes around $(EMACS) are here because the user could type
# it with forward slashes and without quotes, which will fail if
# the shell is cmd.exe.

emacs = "$(EMACS)" $(EMACSOPT)

# Have to define the list of subdirs manually when not using sh.
WINS_ALMOST=\
	calc \
	calendar \
	emacs-lisp \
	emulation \
	erc \
	eshell \
	gnus \
	international \
	language \
	mail \
	mh-e \
	net \
	play \
	progmodes \
	term \
	textmodes \
	url

WINS= $(WINS_ALMOST) \
	obsolete

doit:

$(lisp)/cus-load.el:
	touch $@
# WARNING: Do NOT split the part inside $(ARGQUOTE)s into multiple lines as
#          this can break with GNU Make 3.81 and later if sh.exe is used.
custom-deps: $(lisp)/cus-load.el $(lisp)/loaddefs.el doit
	@echo Directories: $(WINS)
	-$(emacs) -l cus-dep --eval $(ARGQUOTE)(setq find-file-hook nil)$(ARGQUOTE) -f custom-make-dependencies $(lisp) $(WINS)

finder-data: $(lisp)/loaddefs.el doit
	@echo Directories: $(WINS)
	$(emacs) -l finder -f finder-compile-keywords-make-dist $(lisp) $(WINS)

$(lisp)/loaddefs.el:
	$(MAKE) $(MFLAGS) loaddefs.el-$(SHELLTYPE)
	cp loaddefs.el-$(SHELLTYPE) $@
	rm loaddefs.el-$(SHELLTYPE)

loaddefs.el-SH:
	echo ";;; loaddefs.el --- automatically extracted autoloads" > $@
	echo ";;" >> $@; echo ";;; Code:" >> $@
	echo "(autoload 'define-minor-mode \"easy-mmode\")" >>$@
	echo "(autoload 'define-ccl-program \"ccl\")" >>$@
	echo "(autoload 'regexp-opt \"regexp-opt\")" >>$@
	echo "(autoload 'string-to-list \"mule-util\")" >>$@
	echo "(autoload 'define-derived-mode \"derived\")" >>$@
	echo "(autoload 'encoded-kbd-mode \"encoded-kb\")" >>$@
	echo "(defvar cvs-global-menu nil)" >>$@
	echo "" >> $@
	echo ";;; Local Variables:" >> $@
	echo ";;; version-control: never" >> $@
	echo ";;; no-byte-compile: t" >> $@
	echo ";;; no-update-autoloads: t" >> $@
	echo ";;; End:" >> $@
	echo ";;; loaddefs.el ends here" >> $@

loaddefs.el-CMD:
	echo ;;; loaddefs.el --- automatically extracted autoloads> $@
	echo ;;; Code:>> $@
	echo (autoload 'define-minor-mode "easy-mmode")>> $@
	echo (autoload 'define-ccl-program "ccl")>> $@
	echo (autoload 'regexp-opt "regexp-opt")>> $@
	echo (autoload 'string-to-list "mule-util")>> $@
	echo (autoload 'define-derived-mode "derived")>> $@
	echo (autoload 'encoded-kbd-mode "encoded-kb")>> $@
	echo (defvar cvs-global-menu nil)>> $@
	echo.>> $@
	echo ;;; Local Variables:>> $@
	echo ;;; version-control: never>> $@
	echo ;;; no-byte-compile: t>> $@
	echo ;;; no-update-autoloads: t>> $@
	echo ;;; End:>> $@
	echo ;;; loaddefs.el ends here>> $@

# Use . instead of $(lisp) because $(lisp) is an absolute file name,
# including a drive letter and any leading directories, so the generated
# loaddefs.el will mention file names that on other machine reference
# possibly non-existent directories.
#
# WARNING: Do NOT split the part inside $(ARGQUOTE)s into multiple lines as
#          this can break with GNU Make 3.81 and later if sh.exe is used.
autoloads: $(lisp)/loaddefs.el doit
	@echo Directories: . $(WINS_ALMOST)
	$(emacs) -l autoload \
		--eval $(ARGQUOTE)(setq find-file-hook nil find-file-suppress-same-file-warnings t)$(ARGQUOTE) \
		-f w32-batch-update-autoloads "$(lisp)/loaddefs.el" . $(WINS_ALMOST)

$(lisp)/subdirs.el:
	$(MAKE) $(MFLAGS) update-subdirs

# Need separate version for sh and native cmd.exe
update-subdirs: update-subdirs-$(SHELLTYPE)

update-subdirs-CMD: doit
	echo ;; -*- no-byte-compile: t -*-> $(lisp)/subdirs.el
	echo ;; In load-path, after this directory should come>> $(lisp)/subdirs.el
	echo ;; certain of its subdirectories.  Here we specify them.>> $(lisp)/subdirs.el
	echo (normal-top-level-add-to-load-path $(SQUOTE)(>> $(lisp)/subdirs.el
	@for %%d in ($(WINS)) do if not (%%d)==(term) echo "%%d">> $(lisp)/subdirs.el
	echo ))>> $(lisp)/subdirs.el

update-subdirs-SH: doit
	$(srcdir)/update-subdirs $(lisp); \
	for file in $(WINS); do \
	   $(srcdir)/update-subdirs $$file; \
	done;

updates: update-subdirs autoloads mh-autoloads finder-data custom-deps

# This is useful after "cvs up".
cvs-update: recompile autoloads finder-data custom-deps

# Update the AUTHORS file.

update-authors:
	$(emacs) -l authors -f batch-update-authors $(srcdir)/AUTHORS $(srcdir)

TAGS: $(lisptagsfiles1) $(lisptagsfiles2)
	$(ETAGS) $(lisptagsfiles1) $(lisptagsfiles2)

TAGS-LISP: $(lisptagsfiles1) $(lisptagsfiles2)
	$(ETAGS) -o TAGS-LISP $(lisptagsfiles1) $(lisptagsfiles2)

.SUFFIXES: .elc .el

.el.elc:
	-$(emacs) -f batch-byte-compile $<

# Compile all Lisp files, but don't recompile those that are up to
# date.  Some files don't actually get compiled because they set the
# local variable no-byte-compile.

# All .elc files are made writable before compilation in case we
# checked out read-only (CVS option -r).  Files MUST be compiled one by
# one.  If we compile several files in a row we can't make sure that
# the compilation environment is clean.  We also set the load-path of
# the Emacs used for compilation to the current directory and its
# subdirectories, to make sure require's and load's in the files being
# compiled find the right files.

# Need separate version for sh and native cmd.exe
compile: $(lisp)/subdirs.el mh-autoloads compile-$(SHELLTYPE) doit

compile-CMD:
#	-for %%f in ($(lisp) $(WINS)) do for %%g in (%%f\*.elc) do @attrib -r %%g
	for %%f in ($(COMPILE_FIRST)) do \
	  $(emacs) -l loaddefs -f batch-byte-compile-if-not-done %%f
	for %%f in (. $(WINS)) do for %%g in (%%f/*.el) do \
	  $(emacs) -l loaddefs -f batch-byte-compile-if-not-done %%f/%%g

compile-SH:
#	for elc in $(lisp)/*.elc $(lisp)/*/*.elc; do attrib -r $$elc; done
	for el in $(COMPILE_FIRST); do \
	  echo Compiling $$el; \
	  $(emacs) -l loaddefs -f batch-byte-compile-if-not-done $$el; \
	done
	for dir in $(lisp) $(WINS); do \
	  for el in $$dir/*.el; do \
	    if test -f $$el; \
	    then \
	      echo Compiling $$el; \
	      $(emacs) -l loaddefs -f batch-byte-compile-if-not-done $$el; \
	    fi \
	  done; \
	done

# Compile all Lisp files.  This is like `compile' but compiles files
# unconditionally.  Some files don't actually get compiled because they
# set the local variable no-byte-compile.

compile-always: $(lisp)/subdirs.el compile-always-$(SHELLTYPE) doit

compile-always-CMD:
#	-for %%f in ($(lisp) $(WINS)) do for %%g in (%%f\*.elc) do @attrib -r %%g
	for %%f in ($(COMPILE_FIRST)) do $(emacs) -f batch-byte-compile %%f
	for %%f in (. $(WINS)) do for %%g in (%%f/*.el) do $(emacs) -f batch-byte-compile %%f/%%g

compile-always-SH:
#	for elc in $(lisp)/*.elc $(lisp)/*/*.elc; do attrib -r $$elc; done
	for el in $(COMPILE_FIRST); do \
	  echo Compiling $$el; \
	  $(emacs) -f batch-byte-compile $$el || exit 1; \
	done
	for dir in $(lisp) $(WINS); do \
	  for el in $$dir/*.el; do \
	    echo Compiling $$el; \
	    $(emacs) -f batch-byte-compile $$el || exit 1; \
	  done; \
	done

compile-calc: compile-calc-$(SHELLTYPE)

compile-calc-CMD:
	for %%f in ($(lisp)/calc/*.el) do $(emacs) -f batch-byte-compile %%f

compile-calc-SH:
	for el in $(lisp)/calc/*.el; do \
	  echo Compiling $$el; \
	  $(emacs) -f batch-byte-compile $$el || exit 1; \
	done

# Backup compiled Lisp files in elc.tar.gz.  If that file already
# exists, make a backup of it.

backup-compiled-files:
	-mv $(lisp)/elc.tar.gz $(lisp)/elc.tar.gz~
	-tar czf $(lisp)/elc.tar.gz $(lisp)/*.elc $(lisp)/*/*.elc

# Compile Lisp files, but save old compiled files first.

compile-after-backup: backup-compiled-files compile-always

# Recompile all Lisp files which are newer than their .elc files.
# Note that this doesn't create .elc files.  It only recompiles if an
# .elc is present.
# WARNING: Do NOT split the part inside $(ARGQUOTE)s into multiple lines as
#          this can break with GNU Make 3.81 and later if sh.exe is used.
recompile: mh-autoloads doit $(lisp)/progmodes/cc-mode.elc
	$(emacs) --eval $(ARGQUOTE)(batch-byte-recompile-directory 0)$(ARGQUOTE) $(lisp)

# Update MH-E internal autoloads. These are not to be confused with
# the autoloads for the MH-E entry points, which are already in
# loaddefs.el.
MH_E_SRC = $(lisp)/mh-e/mh-acros.el $(lisp)/mh-e/mh-alias.el	\
	$(lisp)/mh-e/mh-buffers.el $(lisp)/mh-e/mh-compat.el	\
	$(lisp)/mh-e/mh-comp.el $(lisp)/mh-e/mh-e.el		\
	$(lisp)/mh-e/mh-folder.el $(lisp)/mh-e/mh-funcs.el	\
	$(lisp)/mh-e/mh-gnus.el $(lisp)/mh-e/mh-identity.el	\
	$(lisp)/mh-e/mh-inc.el $(lisp)/mh-e/mh-junk.el		\
	$(lisp)/mh-e/mh-letter.el $(lisp)/mh-e/mh-limit.el	\
	$(lisp)/mh-e/mh-mime.el $(lisp)/mh-e/mh-print.el	\
	$(lisp)/mh-e/mh-scan.el $(lisp)/mh-e/mh-search.el	\
	$(lisp)/mh-e/mh-seq.el $(lisp)/mh-e/mh-show.el		\
	$(lisp)/mh-e/mh-speed.el $(lisp)/mh-e/mh-thread.el	\
	$(lisp)/mh-e/mh-tool-bar.el $(lisp)/mh-e/mh-utils.el	\
	$(lisp)/mh-e/mh-xface.el

# See the commentary for autoloads above for why we use ./mh-e below
# instead of $(lisp)/mh-e.
mh-autoloads: $(lisp)/mh-e/mh-loaddefs.el
$(lisp)/mh-e/mh-loaddefs.el: $(MH_E_SRC)
	$(MAKE) $(MFLAGS) pre-mh-loaddefs.el-$(SHELLTYPE)
	cp pre-mh-loaddefs.el-$(SHELLTYPE) $@
	rm pre-mh-loaddefs.el-$(SHELLTYPE)
	"$(EMACS)" $(EMACSOPT) \
	   -l autoload \
	   --eval "(setq generate-autoload-cookie \";;;###mh-autoload\")" \
	   --eval "(setq find-file-suppress-same-file-warnings t)" \
	   --eval "(setq make-backup-files nil)" \
	   -f w32-batch-update-autoloads \
	   "$(lisp)/mh-e/mh-loaddefs.el" ./mh-e

pre-mh-loaddefs.el-SH:
	echo ";;; mh-loaddefs.el --- automatically extracted autoloads" > $@
	echo ";;" >> $@
	echo ";; Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc." >> $@
	echo ";; Author: Bill Wohler <wohler@newt.com>" >> $@
	echo ";; Keywords: mail" >> $@
	echo ";;; Commentary:" >> $@
	echo ";;; Change Log:" >> $@
	echo ";;; Code:" >> $@
	echo "" >> $@
	echo "(provide 'mh-loaddefs)" >> $@
	echo ";; Local Variables:" >> $@
	echo ";; version-control: never" >> $@
	echo ";; no-byte-compile: t" >> $@
	echo ";; no-update-autoloads: t" >> $@
	echo ";; End:" >> $@
	echo ";;; mh-loaddefs.el ends here" >> $@

pre-mh-loaddefs.el-CMD:
	echo ;;; mh-loaddefs.el --- automatically extracted autoloads> $@
	echo ;;>> $@
	echo ;; Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.>> $@
	echo ;; Author: Bill Wohler (wohler@newt.com)>> $@
	echo ;; Keywords: mail>> $@
	echo ;;; Commentary:>> $@
	echo ;;; Change Log:>> $@
	echo ;;; Code:>> $@
	echo.>> $@
	echo (provide 'mh-loaddefs)>> $@
	echo ;; Local Variables:>> $@
	echo ;; version-control: never>> $@
	echo ;; no-byte-compile: t>> $@
	echo ;; no-update-autoloads: t>> $@
	echo ;; End:>> $@
	echo ;;; mh-loaddefs.el ends here>> $@

# Prepare a bootstrap in the lisp subdirectory.
#
# Build loaddefs.el to make sure it's up-to-date.  If it's not, that
# might lead to errors during the bootstrap because something fails to
# autoload as expected.  If there is no emacs binary, then we can't
# build autoloads yet.  In that case we have to use ldefs-boot.el;
# bootstrap should always work with ldefs-boot.el.  (Because
# loaddefs.el is an automatically generated file, we don't want to
# store it in the source repository).
#
# Remove compiled Lisp files so that bootstrap-emacs will be built from
# sources only.

# Need separate version for sh and native cmd.exe
# Note that bootstrap-clean-$(SHELLTYPE) copies ldefs-boot.el to loaddefs.el,
# and thus the almost-empty loaddefs.el crafted by the $(lisp)/loaddefs.el
# target can NOT be built _after_ bootstrap-clean-$(SHELLTYPE) does its
# thing, or else an empty loaddefs.el will overwrite the full one.
bootstrap-clean: $(lisp)/loaddefs.el
	$(MAKE) $(MFLAGS) bootstrap-clean-$(SHELLTYPE)

bootstrap-clean-CMD:
#	if exist "$(EMACS)" $(MAKE) $(MFLAGS) autoloads
	cp $(lisp)/ldefs-boot.el $(lisp)/loaddefs.el
	-for %%f in (. $(WINS)) do for %%g in (%%f\*.elc) do @$(DEL) %%g

bootstrap-clean-SH:
#	if test -f "$(EMACS)"; then $(MAKE) $(MFLAGS) autoloads; fi
#	-rm -f $(lisp)/*.elc $(lisp)/*/*.elc
	cp $(lisp)/ldefs-boot.el $(lisp)/loaddefs.el
	-for dir in . $(WINS); do rm -f $$dir/*.elc; done

# Generate/update files for the bootstrap process.
# When done, remove bootstrap-emacs from ../bin, so that
# it will not be mistaken for an installed binary.

bootstrap: update-subdirs autoloads mh-autoloads compile finder-data custom-deps
	- $(DEL) "$(EMACS)"

#
# Assuming INSTALL_DIR is defined, copy the elisp files to it
# Windows 95 makes this harder than it should be.
#
install:
		- mkdir "$(INSTALL_DIR)/lisp"
		- $(DEL) ../same-dir.tst
		- $(DEL) "$(INSTALL_DIR)/same-dir.tst"
		echo SameDirTest > "$(INSTALL_DIR)/same-dir.tst"
#ifdef COPY_LISP_SOURCE
		$(IFNOTSAMEDIR) $(MAKE) $(MFLAGS) install-lisp-$(SHELLTYPE) $(ENDIF)
#else
#		$(IFNOTSAMEDIR) $(CP_DIR) *.elc "$(INSTALL_DIR)/lisp" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) cus-load.el "$(INSTALL_DIR)/lisp" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) forms*.el "$(INSTALL_DIR)/lisp" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) patcomp.el "$(INSTALL_DIR)/lisp" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) subdirs.el "$(INSTALL_DIR)/lisp" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) version.el "$(INSTALL_DIR)/lisp" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) mail/blessmail.el "$(INSTALL_DIR)/lisp/mail" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) mail/sc.el "$(INSTALL_DIR)/lisp/mail" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) play/bruce.el "$(INSTALL_DIR)/lisp/play" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) international/latin-*.el "$(INSTALL_DIR)/lisp/international" $(ENDIF)
#		$(IFNOTSAMEDIR) $(CP) international/mule-conf.el "$(INSTALL_DIR)/lisp/international" $(ENDIF)
#endif
		- $(DEL) ../same-dir.tst
		- $(DEL) "$(INSTALL_DIR)/same-dir.tst"

# Need to copy *.el files first, to avoid "source file is newer" annoyance
# since cp does not preserve time stamps
install-lisp-SH:
	cp -f *.el "$(INSTALL_DIR)/lisp"
	for dir in $(WINS); do [ -d "$(INSTALL_DIR)/lisp/$$dir" ] || mkdir "$(INSTALL_DIR)/lisp/$$dir"; done
	for dir in $(WINS); do cp -f $$dir/*.el "$(INSTALL_DIR)/lisp/$$dir"; done
	for dir in . $(WINS); do cp $$dir/*.elc "$(INSTALL_DIR)/lisp/$$dir"; done

install-lisp-CMD:
	cp -f *.el "$(INSTALL_DIR)/lisp"
	for %%f in ($(WINS)) do if not exist "$(INSTALL_DIR)/lisp/%%f" mkdir "$(INSTALL_DIR)/lisp/%%f"
	for %%f in ($(WINS)) do cp -f %%f/*.el "$(INSTALL_DIR)/lisp/%%f"
	for %%f in (. $(WINS)) do cp -f %%f/*.elc "$(INSTALL_DIR)/lisp/%%f"

#
# Maintenance
#
# We used to delete *~ here, but that might inadvertently remove
# precious files if it happens to match their short 8+3 aliases.
clean:
		- $(DEL) *.el~
