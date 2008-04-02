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

#######################################################################
# Master "Core Components" macros to figure out binary code location  #
#######################################################################

#
# Figure out where the binary code lives.
#

ifdef BUILD_TREE
ifdef LIBRARY_NAME
BUILD         = $(BUILD_TREE)/nss/$(LIBRARY_NAME)
OBJDIR        = $(BUILD_TREE)/nss/$(LIBRARY_NAME)
DEPENDENCIES  = $(BUILD_TREE)/nss/$(LIBRARY_NAME)/.md
else
BUILD         = $(BUILD_TREE)/nss
OBJDIR        = $(BUILD_TREE)/nss
DEPENDENCIES  = $(BUILD_TREE)/nss/.md
endif
else
BUILD         = $(PLATFORM)
OBJDIR        = $(PLATFORM)
DEPENDENCIES  = $(PLATFORM)/.md
endif

DIST          = $(SOURCE_PREFIX)/$(PLATFORM)

ifdef BUILD_DEBUG_GC
    DEFINES += -DDEBUG_GC
endif

GARBAGE += $(DEPENDENCIES) core $(wildcard core.[0-9]*)

ifdef NSPR_INCLUDE_DIR
    INCLUDES += -I$(NSPR_INCLUDE_DIR)
endif

ifndef NSPR_LIB_DIR
    NSPR_LIB_DIR = $(DIST)/lib
endif

ifdef NSS_INCLUDE_DIR
    INCLUDES += -I$(NSS_INCLUDE_DIR)
endif

ifndef NSS_LIB_DIR
    NSS_LIB_DIR = $(DIST)/lib
endif

MK_LOCATION = included
