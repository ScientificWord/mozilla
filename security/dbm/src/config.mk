#! gmake
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
# 
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
# 
# The Original Code is the Netscape security libraries.
# 
# The Initial Developer of the Original Code is Netscape
# Communications Corporation.  Portions created by Netscape are 
# Copyright (C) 1994-2000 Netscape Communications Corporation.  All
# Rights Reserved.
# 
# Contributor(s):
# 
# Alternatively, the contents of this file may be used under the
# terms of the GNU General Public License Version 2 or later (the
# "GPL"), in which case the provisions of the GPL are applicable 
# instead of those above.  If you wish to allow use of your 
# version of this file only under the terms of the GPL and not to
# allow others to use your version of this file under the MPL,
# indicate your decision by deleting the provisions above and
# replace them with the notice and other provisions required by
# the GPL.  If you do not delete the provisions above, a recipient
# may use your version of this file under either the MPL or the
# GPL.
#

DEFINES += -DMEMMOVE -D__DBINTERFACE_PRIVATE $(SECURITY_FLAG)

INCLUDES += -I$(CORE_DEPTH)/../dbm/include

#
#  Currently, override TARGETS variable so that only static libraries
#  are specifed as dependencies within rules.mk.
#

TARGETS        = $(LIBRARY)
SHARED_LIBRARY =
IMPORT_LIBRARY =
PURE_LIBRARY   =
PROGRAM        =

ifdef SHARED_LIBRARY
	ifeq (,$(filter-out WINNT WIN95 WINCE,$(OS_TARGET))) # list omits WIN16
		DLLBASE=/BASE:0x30000000
		RES=$(OBJDIR)/dbm.res
		RESNAME=../include/dbm.rc
	endif
	ifeq ($(DLL_SUFFIX),dll)
		DEFINES += -D_DLL
	endif
endif

ifeq ($(OS_TARGET),AIX)
	OS_LIBS += -lc_r
endif
