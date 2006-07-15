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
# The Original Code is the Netscape Portable Runtime (NSPR).
# 
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1998-2000
# the Initial Developer. All Rights Reserved.
# 
# Contributor(s):
# 
# Alternatively, the contents of this file may be used under the terms of
# either of the GNU General Public License Version 2 or later (the "GPL"),
# or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#
# Configuration common to all versions of Windows NT
# and Windows 95.
#

#
# Client build: make sure we use the shmsdos.exe under $(MOZ_TOOLS).
# $(MOZ_TOOLS_FLIPPED) is $(MOZ_TOOLS) with all the backslashes
# flipped, so that gmake won't interpret them as escape characters.
#
ifdef PR_CLIENT_BUILD_WINDOWS
SHELL = $(MOZ_TOOLS_FLIPPED)/bin/shmsdos.exe
endif

CC = cl
CCC = cl
LINK = link
AR = lib -NOLOGO -OUT:"$@"
RANLIB = echo
BSDECHO = echo
NSINSTALL = nsinstall
INSTALL	= $(NSINSTALL)
define MAKE_OBJDIR
if test ! -d $(@D); then rm -rf $(@D); $(NSINSTALL) -D $(@D); fi
endef
RC = rc.exe

GARBAGE = $(OBJDIR)/vc20.pdb $(OBJDIR)/vc40.pdb

XP_DEFINE = -DXP_PC
OBJ_SUFFIX = obj
LIB_SUFFIX = lib
DLL_SUFFIX = dll

OS_CFLAGS = -W3 -nologo -GF -Gy

ifdef BUILD_OPT
OS_CFLAGS += -MD
OPTIMIZER = -O2
DEFINES = -UDEBUG -U_DEBUG -DNDEBUG
DLLFLAGS = -OUT:"$@"
OBJDIR_TAG = _OPT

# Add symbolic information for use by a profiler
ifdef MOZ_PROFILE
OPTIMIZER += -Zi
DLLFLAGS += -DEBUG
LDFLAGS += -DEBUG
endif

else
#
# Define USE_DEBUG_RTL if you want to use the debug runtime library
# (RTL) in the debug build
#
ifdef USE_DEBUG_RTL
OS_CFLAGS += -MDd
else
OS_CFLAGS += -MD
endif
OPTIMIZER = -Od -Zi
#OPTIMIZER = -Zi -Fd$(OBJDIR)/ -Od
DEFINES = -DDEBUG -D_DEBUG -UNDEBUG

DLLFLAGS = -DEBUG -OUT:"$@"
ifdef GLOWCODE
DLLFLAGS = -DEBUG -DEBUGTYPE:both -INCLUDE:_GlowCode -OUT:"$@"
endif

OBJDIR_TAG = _DBG
LDFLAGS = -DEBUG
#
# When PROFILE=1 is defined, set the compile and link options
# to build targets for use by the ms-win32 profiler
#
ifdef PROFILE
LDFLAGS += -PROFILE -MAP
DLLFLAGS += -PROFILE -MAP
endif
endif

DEFINES += -DWIN32 -D_WINDOWS

#
# On Win95, we use the TlsXXX() interface by default because that
# allows us to load the NSPR DLL dynamically at run time.
# If you want to use static thread-local storage (TLS) for better
# performance, build the NSPR library with USE_STATIC_TLS=1.
#
ifeq ($(USE_STATIC_TLS),1)
DEFINES += -D_PR_USE_STATIC_TLS
endif

#
# NSPR uses both fibers and static thread-local storage
# (i.e., __declspec(thread) variables) on NT.  We need the -GT
# flag to turn off certain compiler optimizations so that fibers
# can use static TLS safely.
#
# Also, we optimize for Pentium (-G5) on NT.
#
ifeq ($(OS_TARGET),WINNT)
OS_CFLAGS += -GT
ifeq ($(CPU_ARCH),x86)
OS_CFLAGS += -G5
endif
DEFINES += -DWINNT
else
DEFINES += -DWIN95 -D_PR_GLOBAL_THREADS_ONLY
endif

ifeq ($(CPU_ARCH),x86)
DEFINES += -D_X86_
else
ifeq ($(CPU_ARCH),MIPS)
DEFINES += -D_MIPS_
else
ifeq ($(CPU_ARCH),ALPHA)
DEFINES += -D_ALPHA_=1
else
CPU_ARCH = processor_is_undefined
endif
endif
endif

# Name of the binary code directories

ifeq ($(CPU_ARCH),x86)
CPU_ARCH_TAG =
else
CPU_ARCH_TAG = $(CPU_ARCH)
endif

ifdef USE_DEBUG_RTL
OBJDIR_SUFFIX = OBJD
else
OBJDIR_SUFFIX = OBJ
endif

OBJDIR_NAME = $(OS_CONFIG)$(CPU_ARCH_TAG)$(OBJDIR_TAG).$(OBJDIR_SUFFIX)

OS_DLLFLAGS = -nologo -DLL -SUBSYSTEM:WINDOWS -PDB:NONE
