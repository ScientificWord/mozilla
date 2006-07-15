#
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Netscape security libraries.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1994-2000
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

include $(CORE_DEPTH)/coreconf/UNIX.mk

#
# The default implementation strategy for Linux is now pthreads
#
USE_PTHREADS = 1

ifeq ($(USE_PTHREADS),1)
	IMPL_STRATEGY = _PTH
endif

CC			= gcc
CCC			= g++
RANLIB			= ranlib

DEFAULT_COMPILER = gcc

ifeq ($(OS_TEST),m68k)
	OS_REL_CFLAGS	= -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH	= m68k
else
ifeq ($(OS_TEST),ppc64)
	OS_REL_CFLAGS	= -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH	= ppc
ifeq ($(USE_64),1)
	ARCHFLAG	= -m64
endif
else
ifeq ($(OS_TEST),ppc)
	OS_REL_CFLAGS	= -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH	= ppc
else
ifeq ($(OS_TEST),alpha)
        OS_REL_CFLAGS   = -D_ALPHA_ -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH	= alpha
else
ifeq ($(OS_TEST),ia64)
	OS_REL_CFLAGS	= -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH	= ia64
else
ifeq ($(OS_TEST),x86_64)
ifeq ($(USE_64),1)
	OS_REL_CFLAGS	= -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH	= x86_64
else
	OS_REL_CFLAGS	= -DLINUX1_2 -Di386 -D_XOPEN_SOURCE
	CPU_ARCH	= x86
	ARCHFLAG	= -m32
endif
else
ifeq ($(OS_TEST),sparc)
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = sparc
else
ifeq ($(OS_TEST),sparc64)
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = sparc
else
ifeq (,$(filter-out arm% sa110,$(OS_TEST)))
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = arm
else
ifeq ($(OS_TEST),parisc)
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = hppa
else
ifeq ($(OS_TEST),parisc64)
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = hppa
else
ifeq ($(OS_TEST),s390)
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = s390
else
ifeq ($(OS_TEST),s390x)
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = s390x
else
ifeq ($(OS_TEST),mips)
	OS_REL_CFLAGS   = -DLINUX1_2 -D_XOPEN_SOURCE
	CPU_ARCH        = mips
else
	OS_REL_CFLAGS	= -DLINUX1_2 -Di386 -D_XOPEN_SOURCE
	CPU_ARCH	= x86
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif


LIBC_TAG		= _glibc

ifeq ($(OS_RELEASE),2.0)
	OS_REL_CFLAGS	+= -DLINUX2_0
	MKSHLIB		= $(CC) -shared -Wl,-soname -Wl,$(@:$(OBJDIR)/%.so=%.so)
	ifdef MAPFILE
		MKSHLIB += -Wl,--version-script,$(MAPFILE)
	endif
	PROCESS_MAP_FILE = grep -v ';-' $< | \
         sed -e 's,;+,,' -e 's; DATA ;;' -e 's,;;,,' -e 's,;.*,;,' > $@
endif

ifdef BUILD_OPT
	OPTIMIZER	= -O2
endif

ifeq ($(USE_PTHREADS),1)
OS_PTHREAD = -lpthread 
endif

OS_CFLAGS		= $(DSO_CFLAGS) $(OS_REL_CFLAGS) $(ARCHFLAG) -ansi -Wall -pipe -DLINUX -Dlinux -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR
OS_LIBS			= $(OS_PTHREAD) -ldl -lc

ifdef USE_PTHREADS
	DEFINES		+= -D_REENTRANT
endif

ARCH			= linux

DSO_CFLAGS		= -fPIC
DSO_LDOPTS		= -shared $(ARCHFLAG)
DSO_LDFLAGS		=
LDFLAGS			+= $(ARCHFLAG)

# INCLUDES += -I/usr/include -Y/usr/include/linux
G++INCLUDES		= -I/usr/include/g++

#
# Always set CPU_TAG on Linux, OpenVMS, WINCE.
#
CPU_TAG = _$(CPU_ARCH)
