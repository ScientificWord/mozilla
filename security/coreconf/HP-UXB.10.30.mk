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

# On HP-UX 10.30 and 11.00, the default implementation strategy is
# pthreads.  Classic nspr and pthreads-user are also available.

ifeq ($(OS_RELEASE),B.10.30)
	DEFAULT_IMPL_STRATEGY	 = _PTH
endif

#
# Config stuff for HP-UXB.10.30.
#
include $(CORE_DEPTH)/coreconf/HP-UXB.10.mk

OS_CFLAGS		+= -DHPUX10_30

#
# To use the true pthread (kernel thread) library on 10.30 and
# 11.00, we should define _POSIX_C_SOURCE to be 199506L.
# The _REENTRANT macro is deprecated.
#

ifdef USE_PTHREADS
	OS_CFLAGS	+= -D_POSIX_C_SOURCE=199506L
endif
