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

# Configuration information for building in the "Core Components" source module

#######################################################################
# [1.0] Master "Core Components" source and release <architecture>    #
#       tags                                                          #
#######################################################################
ifndef MK_ARCH
include $(CORE_DEPTH)/coreconf/arch.mk
endif

#######################################################################
# [2.0] Master "Core Components" default command macros               #
#       (NOTE: may be overridden in $(OS_TARGET)$(OS_RELEASE).mk)     #
#######################################################################
ifndef MK_COMMAND
include $(CORE_DEPTH)/coreconf/command.mk
endif

#######################################################################
# [3.0] Master "Core Components" <architecture>-specific macros       #
#       (dependent upon <architecture> tags)                          #
#                                                                     #
#       We are moving towards just having a $(OS_TARGET).mk file      #
#       as opposed to multiple $(OS_TARGET)$(OS_RELEASE).mk files,    #
#       one for each OS release.                                      #
#######################################################################

TARGET_OSES = FreeBSD BSD_OS NetBSD OpenUNIX OS2 QNX Darwin BeOS OpenBSD \
              OpenVMS AIX

ifeq (,$(filter-out $(TARGET_OSES),$(OS_TARGET)))
include $(CORE_DEPTH)/coreconf/$(OS_TARGET).mk
else
include $(CORE_DEPTH)/coreconf/$(OS_TARGET)$(OS_RELEASE).mk
endif

#######################################################################
# [4.0] Master "Core Components" source and release <platform> tags   #
#       (dependent upon <architecture> tags)                          #
#######################################################################
PLATFORM = $(OBJDIR_NAME)

#######################################################################
# [5.0] Master "Core Components" release <tree> tags                  #
#       (dependent upon <architecture> tags)                          #
#######################################################################
ifndef MK_TREE
include $(CORE_DEPTH)/coreconf/tree.mk
endif

#######################################################################
# [6.0] Master "Core Components" source and release <component> tags  #
#       NOTE:  A component is also called a module or a subsystem.    #
#       (dependent upon $(MODULE) being defined on the                #
#        command line, as an environment variable, or in individual   #
#        makefiles, or more appropriately, manifest.mn)               #
#######################################################################
ifndef MK_MODULE
include $(CORE_DEPTH)/coreconf/module.mk
endif

#######################################################################
# [7.0] Master "Core Components" release <version> tags               #
#       (dependent upon $(MODULE) being defined on the                #
#        command line, as an environment variable, or in individual   #
#        makefiles, or more appropriately, manifest.mn)               #
#######################################################################
ifndef MK_VERSION
include $(CORE_DEPTH)/coreconf/version.mk
endif

#######################################################################
# [8.0] Master "Core Components" macros to figure out                 #
#       binary code location                                          #
#       (dependent upon <platform> tags)                              #
#######################################################################
ifndef MK_LOCATION
include $(CORE_DEPTH)/coreconf/location.mk
endif

#######################################################################
# [9.0] Master "Core Components" <component>-specific source path     #
#       (dependent upon <user_source_tree>, <source_component>,       #
#        <version>, and <platform> tags)                              #
#######################################################################
ifndef MK_SOURCE
include $(CORE_DEPTH)/coreconf/source.mk
endif

#######################################################################
# [10.0] Master "Core Components" include switch for support header   #
#        files                                                        #
#        (dependent upon <tree>, <component>, <version>,              #
#         and <platform> tags)                                        #
#######################################################################
ifndef MK_HEADERS
include $(CORE_DEPTH)/coreconf/headers.mk
endif

#######################################################################
# [11.0] Master "Core Components" for computing program prefixes      #
#######################################################################
ifndef MK_PREFIX
include $(CORE_DEPTH)/coreconf/prefix.mk
endif

#######################################################################
# [12.0] Master "Core Components" for computing program suffixes      #
#        (dependent upon <architecture> tags)                         #
#######################################################################
ifndef MK_SUFFIX
include $(CORE_DEPTH)/coreconf/suffix.mk
endif

#######################################################################
# [13.0] Master "Core Components" for defining JDK                    #
#        (dependent upon <architecture>, <source>, and <suffix>  tags)#
#######################################################################
ifdef NS_USE_JDK
include $(CORE_DEPTH)/coreconf/jdk.mk
endif

#######################################################################
# [14.0] Master "Core Components" rule set                            #
#######################################################################
ifndef MK_RULESET
include $(CORE_DEPTH)/coreconf/ruleset.mk
endif

#######################################################################
# [15.0] Dependencies.
#######################################################################

-include $(MKDEPENDENCIES)

#######################################################################
# [16.0] Global environ ment defines
#######################################################################

ifdef NSS_ENABLE_ECC
DEFINES += -DNSS_ENABLE_ECC
endif

ifdef NSS_ECC_MORE_THAN_SUITE_B
DEFINES += -DNSS_ECC_MORE_THAN_SUITE_B
endif
