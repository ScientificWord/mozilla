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

MOZ_WIDGET_TOOLKIT = os2

# XP_PC is for Window and OS2 on Intel X86
# XP_OS2 is strictly for OS2 only
XP_DEFINE  += -DXP_PC=1  -DXP_OS2=1

# Override prefix
LIB_PREFIX  = $(NULL)

# Override suffix in suffix.mk
LIB_SUFFIX  = lib
# the DLL_SUFFIX must be uppercase for FIPS mode to work. bugzilla 240784
DLL_SUFFIX  = DLL
PROG_SUFFIX = .exe


CCC			= gcc
LINK			= gcc
AR                      = emxomfar r $@
# Keep AR_FLAGS blank so that we do not have to change rules.mk
AR_FLAGS                = 
RANLIB 			= @echo OS2 RANLIB
BSDECHO 		= @echo OS2 BSDECHO
IMPLIB			= emximp -o
FILTER			= emxexp -o

# GCC for OS/2 currently predefines these, but we don't want them
DEFINES 		+= -Uunix -U__unix -U__unix__

DEFINES			+= -DTCPV40HDRS

ifeq ($(MOZ_OS2_HIGH_MEMORY),1)
HIGHMEM_LDFLAG          = -Zhigh-mem
endif

ifndef NO_SHARED_LIB
WRAP_MALLOC_LIB         = 
WRAP_MALLOC_CFLAGS      = 
DSO_CFLAGS              = 
DSO_PIC_CFLAGS          = 
MKSHLIB                 = $(CXX) $(CXXFLAGS) $(DSO_LDOPTS) -o $@
MKCSHLIB                = $(CC) $(CFLAGS) $(DSO_LDOPTS) -o $@
MKSHLIB_FORCE_ALL       = 
MKSHLIB_UNFORCE_ALL     = 
DSO_LDOPTS              = -Zomf -Zdll -Zmap $(HIGHMEM_LDFLAG)
SHLIB_LDSTARTFILE	= 
SHLIB_LDENDFILE		= 
ifdef MAPFILE
MKSHLIB += $(MAPFILE)
endif
PROCESS_MAP_FILE = \
	echo LIBRARY $(LIBRARY_NAME)$(LIBRARY_VERSION) INITINSTANCE TERMINSTANCE > $@; \
	echo PROTMODE >> $@; \
	echo CODE    LOADONCALL MOVEABLE DISCARDABLE >> $@; \
	echo DATA    PRELOAD MOVEABLE MULTIPLE NONSHARED >> $@; \
	echo EXPORTS >> $@; \
	grep -v ';+' $< | grep -v ';-' | \
	sed -e 's; DATA ;;' -e 's,;;,,' -e 's,;.*,,' -e 's,\([\t ]*\),\1_,' | \
	awk 'BEGIN {ord=1;} { print($$0 " @" ord " RESIDENTNAME"); ord++;}' >> $@

endif   #NO_SHARED_LIB

OS_CFLAGS          = -Wall -W -Wno-unused -Wpointer-arith -Wcast-align -Wno-switch -Zomf -DDEBUG -DTRACING -g

ifdef BUILD_OPT
OPTIMIZER		= -O2 -s
DEFINES 		+= -UDEBUG -U_DEBUG -DNDEBUG
DLLFLAGS		= -DLL -OUT:$@ -MAP:$(@:.dll=.map) $(HIGHMEM_LDFLAG)
EXEFLAGS    		= -PMTYPE:VIO -OUT:$@ -MAP:$(@:.exe=.map) -nologo -NOE $(HIGHMEM_LDFLAG)
OBJDIR_TAG 		= _OPT
else
#OPTIMIZER		= -O+ -Oi
DEFINES 		+= -DDEBUG -D_DEBUG -DDEBUGPRINTS     #HCT Need += to avoid overidding manifest.mn 
DLLFLAGS		= -DEBUG -DLL -OUT:$@ -MAP:$(@:.dll=.map) $(HIGHMEM_LDFLAG)
EXEFLAGS    		= -DEBUG -PMTYPE:VIO -OUT:$@ -MAP:$(@:.exe=.map) -nologo -NOE $(HIGHMEM_LDFLAG)
OBJDIR_TAG 		= _DBG
LDFLAGS 		= -DEBUG $(HIGHMEM_LDFLAG)
endif   # BUILD_OPT

# OS/2 use nsinstall that is included in the toolkit.
# since we do not wish to support and maintain 3 version of nsinstall in mozilla, nspr and nss

ifdef BUILD_TREE
NSINSTALL_DIR  = $(BUILD_TREE)/nss
else
NSINSTALL_DIR  = $(CORE_DEPTH)/coreconf/nsinstall
endif
# NSINSTALL      = $(NSINSTALL_DIR)/$(OBJDIR_NAME)/nsinstall
NSINSTALL 	= nsinstall             # HCT4OS2
INSTALL		= $(NSINSTALL)

MKDEPEND_DIR    = $(CORE_DEPTH)/coreconf/mkdepend
MKDEPEND        = $(MKDEPEND_DIR)/$(OBJDIR_NAME)/mkdepend
MKDEPENDENCIES  = $(OBJDIR_NAME)/depend.mk

####################################################################
#
# One can define the makefile variable NSDISTMODE to control
# how files are published to the 'dist' directory.  If not
# defined, the default is "install using relative symbolic
# links".  The two possible values are "copy", which copies files
# but preserves source mtime, and "absolute_symlink", which
# installs using absolute symbolic links.
#   - THIS IS NOT PART OF THE NEW BINARY RELEASE PLAN for 9/30/97
#   - WE'RE KEEPING IT ONLY FOR BACKWARDS COMPATIBILITY
####################################################################

ifeq ($(NSDISTMODE),copy)
	# copy files, but preserve source mtime
	INSTALL  = $(NSINSTALL)
	INSTALL += -t
else
	ifeq ($(NSDISTMODE),absolute_symlink)
		# install using absolute symbolic links
		INSTALL  = $(NSINSTALL)
		INSTALL += -L `pwd`
	else
		# install using relative symbolic links
		INSTALL  = $(NSINSTALL)
		INSTALL += -R
	endif
endif

define MAKE_OBJDIR
if test ! -d $(@D); then rm -rf $(@D); $(NSINSTALL) -D $(@D); fi
endef

#
# override the definition of DLL_PREFIX in prefix.mk
#

ifndef DLL_PREFIX
    DLL_PREFIX = $(NULL)
endif

#
# override the TARGETS defined in ruleset.mk, adding IMPORT_LIBRARY
#
ifndef TARGETS
    TARGETS = $(LIBRARY) $(SHARED_LIBRARY) $(IMPORT_LIBRARY) $(PROGRAM)
endif


ifdef LIBRARY_NAME
    IMPORT_LIBRARY = $(OBJDIR)/$(LIBRARY_NAME)$(LIBRARY_VERSION)$(JDK_DEBUG_SUFFIX).lib
endif

