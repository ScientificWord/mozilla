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
# The Original Code is mozilla.org.
#
# The Initial Developer of the Original Code is 
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 2002
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

XPCOM_GLUE_SRC_LCSRCS =          \
  pldhash.c                      \
  $(NULL)

XPCOM_GLUE_SRC_CSRCS = $(addprefix $(topsrcdir)/xpcom/glue/, $(XPCOM_GLUE_SRC_LCSRCS))

XPCOM_GLUE_SRC_LCPPSRCS =        \
  nsArrayEnumerator.cpp          \
  nsArrayUtils.cpp               \
  nsCategoryCache.cpp            \
  nsCOMPtr.cpp                   \
  nsCOMArray.cpp                 \
  nsCRTGlue.cpp                  \
  nsComponentManagerUtils.cpp    \
  nsEnumeratorUtils.cpp          \
  nsID.cpp                       \
  nsIInterfaceRequestorUtils.cpp \
  nsINIParser.cpp                \
  nsISupportsImpl.cpp            \
  nsMemory.cpp                   \
  nsWeakReference.cpp            \
  nsGREGlue.cpp                  \
  nsVersionComparator.cpp        \
  nsTHashtable.cpp               \
  nsQuickSort.cpp                \
  nsVoidArray.cpp                \
  nsTArray.cpp                   \
  nsThreadUtils.cpp              \
  nsTObserverArray.cpp           \
  nsCycleCollectionParticipant.cpp \
  nsDeque.cpp \
  $(NULL)

XPCOM_GLUE_SRC_CPPSRCS = $(addprefix $(topsrcdir)/xpcom/glue/, $(XPCOM_GLUE_SRC_LCPPSRCS))

# nsGenericFactory is not really all that helpful in the standalone glue,
# and it has a bad dependency on the NSPR AtomicIncrement function, so we
# only build it for the dependent XPCOM glue and builtin to xpcom-core.

XPCOM_GLUENS_SRC_LCPPSRCS =      \
  nsAutoLock.cpp                 \
  nsGenericFactory.cpp           \
  nsProxyRelease.cpp             \
  nsTextFormatter.cpp            \
  $(NULL)

XPCOM_GLUENS_SRC_CPPSRCS = $(addprefix $(topsrcdir)/xpcom/glue/,$(XPCOM_GLUENS_SRC_LCPPSRCS))
