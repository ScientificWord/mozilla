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
# The Original Code is mozilla.org code.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1998
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Stephen Lamm
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

# Build the Mozilla client.
#
# This needs CVSROOT set to work, e.g.,
#   setenv CVSROOT :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot
# or
#   setenv CVSROOT :pserver:username%somedomain.org@cvs.mozilla.org:/cvsroot
# 
# To checkout and build a tree,
#    1. cvs co mozilla/minimo.mk
#    2. cd mozilla
#    3. gmake -f minimo.mk
#
# Other targets (gmake -f minimo.mk [targets...]),
#    checkout
#    build
#    clean (realclean is now the same as clean)
#    distclean
#
# See http://www.mozilla.org/build/unix.html for more information.
#
# Options:
#   MOZ_OBJDIR           - Destination object directory
#   MOZ_CO_DATE          - Date tag to use for checkout (default: none)
#   MOZ_CO_MODULE        - Module to checkout (default: SeaMonkeyCore)
#   MOZ_CVS_FLAGS        - Flags to pass cvs (default: -q -z3)
#   MOZ_CO_FLAGS         - Flags to pass after 'cvs co' (default: -P)
#   MOZ_MAKE_FLAGS       - Flags to pass to $(MAKE)
#   MOZ_CO_BRANCH        - Branch tag (Deprecated. Use MOZ_CO_TAG below.)
#

#######################################################################
# Checkout Tags
#
# For branches, uncomment the MOZ_CO_TAG line with the proper tag,
# and commit this file on that tag.
#MOZ_CO_TAG = <tag>
NSPR_CO_TAG = NSPRPUB_PRE_4_2_CLIENT_BRANCH
PSM_CO_TAG = #We will now build PSM from the tip instead of a branch.
NSS_CO_TAG = NSS_CLIENT_TAG
IMGLIB2_CO_TAG = 
BUILD_MODULES = all

#######################################################################
# Defines
#
CVS = cvs

CWD := $(shell pwd)

ifeq "$(CWD)" "/"
CWD   := /.
endif

ifneq (, $(wildcard minimo.mk))
# Ran from mozilla directory
ROOTDIR   := $(shell dirname $(CWD))
TOPSRCDIR := $(CWD)
else
# Ran from mozilla/.. directory (?)
ROOTDIR   := $(CWD)
TOPSRCDIR := $(CWD)/mozilla
endif

# on os2, TOPSRCDIR may have two forward slashes in a row, which doesn't
#  work;  replace first instance with one forward slash
TOPSRCDIR := $(shell echo "$(TOPSRCDIR)" | sed -e 's%//%/%')

ifndef TOPSRCDIR_MOZ
TOPSRCDIR_MOZ=$(TOPSRCDIR)
endif

# if ROOTDIR equals only drive letter (i.e. "C:"), set to "/"
DIRNAME := $(shell echo "$(ROOTDIR)" | sed -e 's/^.://')
ifeq ($(DIRNAME),)
ROOTDIR := /.
endif

AUTOCONF := autoconf
MKDIR := mkdir
SH := /bin/sh
ifndef MAKE
MAKE := gmake
endif

CONFIG_GUESS_SCRIPT := $(wildcard $(TOPSRCDIR)/build/autoconf/config.guess)
ifdef CONFIG_GUESS_SCRIPT
  CONFIG_GUESS = $(shell $(CONFIG_GUESS_SCRIPT))
else
  _IS_FIRST_CHECKOUT := 1
endif

####################################
# CVS

# Add the CVS root to CVS_FLAGS if needed
CVS_ROOT_IN_TREE := $(shell cat $(TOPSRCDIR)/CVS/Root 2>/dev/null)
ifneq ($(CVS_ROOT_IN_TREE),)
ifneq ($(CVS_ROOT_IN_TREE),$(CVSROOT))
  CVS_FLAGS := -d $(CVS_ROOT_IN_TREE)
endif
endif

CVSCO = $(strip $(CVS) $(CVS_FLAGS) co $(CVS_CO_FLAGS))
CVSCO_LOGFILE := $(ROOTDIR)/cvsco.log
CVSCO_LOGFILE := $(shell echo $(CVSCO_LOGFILE) | sed s%//%/%)

ifdef MOZ_CO_TAG
  CVS_CO_FLAGS := -r $(MOZ_CO_TAG)
endif

####################################
# Load mozconfig Options

# See build pages, http://www.mozilla.org/build/unix.html, 
# for how to set up mozconfig.
MOZCONFIG_LOADER := mozilla/build/autoconf/mozconfig2client-mk
MOZCONFIG_FINDER := mozilla/build/autoconf/mozconfig-find 
MOZCONFIG_MODULES := mozilla/build/unix/modules.mk mozilla/build/unix/uniq.pl
run_for_side_effects := \
  $(shell cd $(ROOTDIR) && \
     if test "$(_IS_FIRST_CHECKOUT)"; then \
        $(CVSCO) $(MOZCONFIG_FINDER) $(MOZCONFIG_LOADER) $(MOZCONFIG_MODULES); \
     else true; \
     fi; \
     $(MOZCONFIG_LOADER) $(TOPSRCDIR) mozilla/.mozconfig.mk > mozilla/.mozconfig.out)
include $(TOPSRCDIR)/.mozconfig.mk
include $(TOPSRCDIR)/build/unix/modules.mk

####################################
# Options that may come from mozconfig

# Change CVS flags if anonymous root is requested
ifdef MOZ_CO_USE_MIRROR
  CVS_FLAGS := -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot
endif

# MOZ_CVS_FLAGS - Basic CVS flags
ifeq "$(origin MOZ_CVS_FLAGS)" "undefined"
  CVS_FLAGS := $(CVS_FLAGS) -q -z 3 
else
  CVS_FLAGS := $(MOZ_CVS_FLAGS)
endif

# This option is deprecated. The best way to have minimo.mk pull a tag
# is to set MOZ_CO_TAG (see above) and commit that change on the tag.
ifdef MOZ_CO_BRANCH
  $(warning Use MOZ_CO_TAG instead of MOZ_CO_BRANCH)
  CVS_CO_FLAGS := -r $(MOZ_CO_BRANCH)
endif

# MOZ_CO_FLAGS - Anything that we should use on all checkouts
ifeq "$(origin MOZ_CO_FLAGS)" "undefined"
  CVS_CO_FLAGS := $(CVS_CO_FLAGS) -P
else
  CVS_CO_FLAGS := $(CVS_CO_FLAGS) $(MOZ_CO_FLAGS)
endif

ifdef MOZ_CO_DATE
  CVS_CO_DATE_FLAGS := -D "$(MOZ_CO_DATE)"
endif

ifdef MOZ_OBJDIR
  OBJDIR := $(MOZ_OBJDIR)
  MOZ_MAKE := $(MAKE) $(MOZ_MAKE_FLAGS) -C $(OBJDIR)
else
  OBJDIR := $(TOPSRCDIR)
  MOZ_MAKE := $(MAKE) $(MOZ_MAKE_FLAGS)
endif

####################################
# CVS defines for PSM
#
PSM_CO_MODULE= mozilla/security/manager
PSM_CO_FLAGS := -P -A
ifdef MOZ_CO_FLAGS
  PSM_CO_FLAGS := $(MOZ_CO_FLAGS)
endif
ifdef PSM_CO_TAG
  PSM_CO_FLAGS := $(PSM_CO_FLAGS) -r $(PSM_CO_TAG)
endif
CVSCO_PSM = $(CVS) $(CVS_FLAGS) co $(PSM_CO_FLAGS) $(CVS_CO_DATE_FLAGS) $(PSM_CO_MODULE)

####################################
# CVS defines for NSS
#
NSS_CO_MODULE = mozilla/security/nss \
		mozilla/security/coreconf \
		$(NULL)

NSS_CO_FLAGS := -P
ifdef MOZ_CO_FLAGS
  NSS_CO_FLAGS := $(MOZ_CO_FLAGS)
endif
ifdef NSS_CO_TAG
   NSS_CO_FLAGS := $(NSS_CO_FLAGS) -r $(NSS_CO_TAG)
endif
# Cannot pull static tags by date
ifeq ($(NSS_CO_TAG),NSS_CLIENT_TAG)
CVSCO_NSS = $(CVS) $(CVS_FLAGS) co $(NSS_CO_FLAGS) $(NSS_CO_MODULE)
else
CVSCO_NSS = $(CVS) $(CVS_FLAGS) co $(NSS_CO_FLAGS) $(CVS_CO_DATE_FLAGS) $(NSS_CO_MODULE)
endif

####################################
# CVS defines for NSPR
#
NSPR_CO_MODULE = mozilla/nsprpub
NSPR_CO_FLAGS := -P
ifdef MOZ_CO_FLAGS
  NSPR_CO_FLAGS := $(MOZ_CO_FLAGS)
endif
ifdef NSPR_CO_TAG
  NSPR_CO_FLAGS := $(NSPR_CO_FLAGS) -r $(NSPR_CO_TAG)
endif
# Cannot pull static tags by date
ifeq ($(NSPR_CO_TAG),NSPRPUB_CLIENT_TAG)
CVSCO_NSPR = $(CVS) $(CVS_FLAGS) co $(NSPR_CO_FLAGS) $(NSPR_CO_MODULE)
else
CVSCO_NSPR = $(CVS) $(CVS_FLAGS) co $(NSPR_CO_FLAGS) $(CVS_CO_DATE_FLAGS) $(NSPR_CO_MODULE)
endif


####################################
# CVS defines for new image library
#
IMGLIB2_CO_MODULE = mozilla/modules/libpr0n
IMGLIB2_CO_FLAGS := -P
ifdef MOZ_CO_FLAGS
  IMGLIB2_CO_FLAGS := $(MOZ_CO_FLAGS)
endif
ifdef IMGLIB2_CO_TAG
  IMGLIB2_CO_FLAGS := $(IMGLIB2_CO_FLAGS) -r $(IMGLIB2_CO_TAG)
endif
CVSCO_IMGLIB2 = $(CVS) $(CVS_FLAGS) co $(IMGLIB2_CO_FLAGS) $(CVS_CO_DATE_FLAGS) $(IMGLIB2_CO_MODULE)

####################################
# CVS defines for SeaMonkey
#
ifeq ($(MOZ_CO_MODULE),)
  MOZ_CO_MODULE := SeaMonkeyXPToolKit mozilla/editor mozilla/extensions/Makefile.in mozilla/extensions/cookie mozilla/l10n/Makefile.in mozilla/l10n/langpacks/Makefile.in
endif
CVSCO_SEAMONKEY := $(CVSCO) $(CVS_CO_DATE_FLAGS) $(MOZ_CO_MODULE)

####################################
# CVS defines for standalone modules
#
ifeq ($(BUILD_MODULES),all)
  CHECKOUT_STANDALONE_NOSUBDIRS := true
else
  STANDALONE_CO_MODULE := $(filter-out $(NSPRPUB_DIR) security directory/c-sdk, $(BUILD_MODULE_CVS))

  NOSUBDIRS_MODULE := $(addprefix mozilla/, $(BUILD_MODULE_CVS_NS))
ifneq ($(NOSUBDIRS_MODULE),)
  CHECKOUT_STANDALONE_NOSUBDIRS := cvs_co $(CVSCO) -l $(CVS_CO_DATE_FLAGS) $(NOSUBDIRS_MODULE)
else
  CHECKOUT_STANDALONE_NOSUBDIRS := true
endif

CVSCO_SEAMONKEY :=
ifeq (,$(filter $(NSPRPUB_DIR), $(BUILD_MODULE_CVS)))
  CVSCO_NSPR :=
endif
ifeq (,$(filter security security/manager, $(BUILD_MODULE_CVS)))
  CVSCO_PSM :=
  CVSCO_NSS :=
endif
ifeq (,$(filter modules/libpr0n, $(BUILD_MODULE_CVS)))
  CVSCO_IMGLIB2 :=
endif
endif

# Pull toplevel cvs modules separately to avoid cvs quirks
STANDALONE_CO_MODULE += allmakefiles.sh minimo.mk aclocal.m4 .cvsignore
STANDALONE_CO_MODULE += Makefile.in configure configure.in
STANDALONE_CO_MODULE := $(addprefix mozilla/, $(STANDALONE_CO_MODULE))
CHECKOUT_STANDALONE := cvs_co $(CVSCO) $(CVS_CO_DATE_FLAGS) $(STANDALONE_CO_MODULE)


####################################
# CVS defined for libart (pulled and built if MOZ_INTERNAL_LIBART_LGPL is set)
#
CVSCO_LIBART := $(CVSCO) $(CVS_CO_DATE_FLAGS) mozilla/other-licenses/libart_lgpl

ifdef MOZ_INTERNAL_LIBART_LGPL
FASTUPDATE_LIBART := fast_update $(CVSCO_LIBART)
CHECKOUT_LIBART := cvs_co $(CVSCO_LIBART)
else
CHECKOUT_LIBART := true
FASTUPDATE_LIBART := true
endif

####################################
# CVS defines for codesighs (pulled and built if MOZ_MAPINFO is set)
#
CVSCO_CODESIGHS := $(CVSCO) $(CVS_CO_DATE_FLAGS) mozilla/tools/codesighs

ifdef MOZ_MAPINFO
FASTUPDATE_CODESIGHS := fast_update $(CVSCO_CODESIGHS)
CHECKOUT_CODESIGHS := cvs_co $(CVSCO_CODESIGHS)
else
CHECKOUT_CODESIGHS := true
FASTUPDATE_CODESIGHS := true
endif

#######################################################################
# Rules
# 

# Print out any options loaded from mozconfig.
all build checkout clean depend distclean export libs install realclean::
	@if test -f .mozconfig.out; then \
	  cat .mozconfig.out; \
	  rm -f .mozconfig.out; \
	else true; \
	fi

ifdef _IS_FIRST_CHECKOUT
all:: checkout build
else
all:: checkout alldep
endif

# Windows equivalents
pull_all: checkout
build_all: build
build_all_dep: alldep
build_all_depend: alldep
clobber clobber_all: clean
pull_and_build_all: checkout alldep

# Do everything from scratch
everything: checkout clean build

####################################
# CVS checkout
#
checkout::
#	@: Backup the last checkout log.
	@if test -f $(CVSCO_LOGFILE) ; then \
	  mv $(CVSCO_LOGFILE) $(CVSCO_LOGFILE).old; \
	else true; \
	fi
ifdef RUN_AUTOCONF_LOCALLY
	@echo "Removing local configures" ; \
	cd $(ROOTDIR) && \
	$(RM) -f mozilla/configure mozilla/nsprpub/configure
endif
	@echo "checkout start: "`date` | tee $(CVSCO_LOGFILE)
	@echo '$(CVSCO) $(CVS_CO_DATE_FLAGS) mozilla/minimo.mk $(MOZCONFIG_MODULES)'; \
        cd $(ROOTDIR) && \
	$(CVSCO) $(CVS_CO_DATE_FLAGS) mozilla/minimo.mk $(MOZCONFIG_MODULES)
	@cd $(ROOTDIR) && $(MAKE) -f mozilla/minimo.mk real_checkout

real_checkout:
#	@: Start the checkout. Split the output to the tty and a log file. \
#	 : If it fails, touch an error file because "tee" hides the error.
	@failed=.cvs-failed.tmp; rm -f $$failed*; \
	cvs_co() { echo "$$@" ; \
	  ("$$@" || touch $$failed) 2>&1 | tee -a $(CVSCO_LOGFILE) && \
	  if test -f $$failed; then false; else true; fi; }; \
	$(CHECKOUT_STANDALONE) && \
	$(CHECKOUT_STANDALONE_NOSUBDIRS) && \
	cvs_co $(CVSCO_NSPR) && \
	cvs_co $(CVSCO_NSS) && \
	cvs_co $(CVSCO_PSM) && \
        cvs_co $(CVSCO_IMGLIB2) && \
	$(CHECKOUT_LIBART) && \
	$(CHECKOUT_CODESIGHS) && \
	cvs_co $(CVSCO_SEAMONKEY)
	@echo "checkout finish: "`date` | tee -a $(CVSCO_LOGFILE)
#	@: Check the log for conflicts. ;
	@conflicts=`egrep "^C " $(CVSCO_LOGFILE)` ;\
	if test "$$conflicts" ; then \
	  echo "$(MAKE): *** Conflicts during checkout." ;\
	  echo "$$conflicts" ;\
	  echo "$(MAKE): Refer to $(CVSCO_LOGFILE) for full log." ;\
	  false; \
	else true; \
	fi
ifdef RUN_AUTOCONF_LOCALLY
	@echo Generating configures using $(AUTOCONF) ; \
	cd $(TOPSRCDIR) && $(AUTOCONF) && \
	cd $(TOPSRCDIR)/nsprpub && $(AUTOCONF)
endif

fast-update:
#	@: Backup the last checkout log.
	@if test -f $(CVSCO_LOGFILE) ; then \
	  mv $(CVSCO_LOGFILE) $(CVSCO_LOGFILE).old; \
	else true; \
	fi
ifdef RUN_AUTOCONF_LOCALLY
	@echo "Removing local configures" ; \
	cd $(ROOTDIR) && \
	$(RM) -f mozilla/configure mozilla/nsprpub/configure
endif
	@echo "checkout start: "`date` | tee $(CVSCO_LOGFILE)
	@echo '$(CVSCO) mozilla/minimo.mk $(MOZCONFIG_MODULES)'; \
        cd $(ROOTDIR) && \
	$(CVSCO) mozilla/minimo.mk $(MOZCONFIG_MODULES)
	@cd $(TOPSRCDIR) && \
	$(MAKE) -f minimo.mk real_fast-update

real_fast-update:
#	@: Start the update. Split the output to the tty and a log file. \
#	 : If it fails, touch an error file because "tee" hides the error.
	@failed=.fast_update-failed.tmp; rm -f $$failed*; \
	fast_update() { (config/cvsco-fast-update.pl $$@ || touch $$failed) 2>&1 | tee -a $(CVSCO_LOGFILE) && \
	  if test -f $$failed; then false; else true; fi; }; \
	cvs_co() { echo "$$@" ; \
	  ("$$@" || touch $$failed) 2>&1 | tee -a $(CVSCO_LOGFILE) && \
	  if test -f $$failed; then false; else true; fi; }; \
	fast_update $(CVSCO_NSPR) && \
	cd $(ROOTDIR) && \
	failed=mozilla/.fast_update-failed.tmp && \
	cvs_co $(CVSCO_NSS) && \
	failed=.fast_update-failed.tmp && \
	cd mozilla && \
	fast_update $(CVSCO_PSM) && \
	fast_update $(CVSCO_IMGLIB2) && \
	$(FASTUPDATE_LIBART) && \
	$(FASTUPDATE_CODESIGHS) && \
	fast_update $(CVSCO_SEAMONKEY)
	@echo "fast_update finish: "`date` | tee -a $(CVSCO_LOGFILE)
#	@: Check the log for conflicts. ;
	@conflicts=`egrep "^C " $(CVSCO_LOGFILE)` ;\
	if test "$$conflicts" ; then \
	  echo "$(MAKE): *** Conflicts during fast-update." ;\
	  echo "$$conflicts" ;\
	  echo "$(MAKE): Refer to $(CVSCO_LOGFILE) for full log." ;\
	  false; \
	else true; \
	fi
ifdef RUN_AUTOCONF_LOCALLY
	@echo Generating configures using $(AUTOCONF) ; \
	cd $(TOPSRCDIR) && $(AUTOCONF) && \
	cd $(TOPSRCDIR)/nsprpub && $(AUTOCONF) && \
	cd $(TOPSRCDIR)/directory/c-sdk && $(AUTOCONF)
endif

####################################
# Web configure

WEBCONFIG_FILE  := $(HOME)/.mozconfig

MOZCONFIG2CONFIGURATOR := build/autoconf/mozconfig2configurator
webconfig:
	@cd $(TOPSRCDIR); \
	url=`$(MOZCONFIG2CONFIGURATOR) $(TOPSRCDIR)`; \
	echo Running mozilla with the following url: ;\
	echo ;\
	echo $$url ;\
	mozilla -remote "openURL($$url)" || \
	netscape -remote "openURL($$url)" || \
	mozilla $$url || \
	netscape $$url ;\
	echo ;\
	echo   1. Fill out the form on the browser. ;\
	echo   2. Save the results to $(WEBCONFIG_FILE)

#####################################################
# First Checkout

ifdef _IS_FIRST_CHECKOUT
# First time, do build target in a new process to pick up new files.
build::
	$(MAKE) -f $(TOPSRCDIR)/minimo.mk build
else

#####################################################
# After First Checkout


####################################
# Configure

CONFIG_STATUS := $(wildcard $(OBJDIR)/config.status)
CONFIG_CACHE  := $(wildcard $(OBJDIR)/config.cache)

ifdef RUN_AUTOCONF_LOCALLY
EXTRA_CONFIG_DEPS := \
	$(TOPSRCDIR)/aclocal.m4 \
	$(wildcard $(TOPSRCDIR)/build/autoconf/*.m4) \
	$(NULL)

$(TOPSRCDIR)/configure: $(TOPSRCDIR)/configure.in $(EXTRA_CONFIG_DEPS)
	@echo Generating $@ using autoconf
	cd $(TOPSRCDIR); $(AUTOCONF)
endif

CONFIG_STATUS_DEPS_L10N := $(wildcard $(TOPSRCDIR)/l10n/makefiles.all)

CONFIG_STATUS_DEPS := \
	$(TOPSRCDIR)/configure \
	$(TOPSRCDIR)/allmakefiles.sh \
	$(TOPSRCDIR)/.mozconfig.mk \
	$(wildcard $(TOPSRCDIR)/nsprpub/configure) \
	$(wildcard $(TOPSRCDIR)/directory/c-sdk/configure) \
	$(wildcard $(TOPSRCDIR)/mailnews/makefiles) \
	$(CONFIG_STATUS_DEPS_L10N) \
	$(wildcard $(TOPSRCDIR)/themes/makefiles) \
	$(NULL)

# configure uses the program name to determine @srcdir@. Calling it without
#   $(TOPSRCDIR) will set @srcdir@ to "."; otherwise, it is set to the full
#   path of $(TOPSRCDIR).
ifeq ($(TOPSRCDIR),$(OBJDIR))
  CONFIGURE := ./configure
else
  CONFIGURE := $(TOPSRCDIR)/configure
endif

ifdef MOZ_TOOLS
  CONFIGURE := $(TOPSRCDIR)/configure
endif

$(OBJDIR)/Makefile $(OBJDIR)/config.status: $(CONFIG_STATUS_DEPS)
	@if test ! -d $(OBJDIR); then $(MKDIR) $(OBJDIR); else true; fi
	@echo cd $(OBJDIR);
	@echo $(CONFIGURE) $(CONFIGURE_ARGS)
	@cd $(OBJDIR) && $(CONFIGURE_ENV_ARGS) $(CONFIGURE) $(CONFIGURE_ARGS) \
	  || ( echo "*** Fix above errors and then restart with\
               \"$(MAKE) -f minimo.mk build\"" && exit 1 )
	@touch $(OBJDIR)/Makefile

ifdef CONFIG_STATUS
$(OBJDIR)/config/autoconf.mk: $(TOPSRCDIR)/config/autoconf.mk.in
	cd $(OBJDIR); \
	  CONFIG_FILES=config/autoconf.mk ./config.status
endif


####################################
# Depend

depend:: $(OBJDIR)/Makefile $(OBJDIR)/config.status
	$(MOZ_MAKE) export && $(MOZ_MAKE) depend

####################################
# Build it

build::  $(OBJDIR)/Makefile $(OBJDIR)/config.status
	$(MOZ_MAKE)

####################################
# Other targets

# Pass these target onto the real build system
install export libs clean realclean distclean alldep:: $(OBJDIR)/Makefile $(OBJDIR)/config.status
	$(MOZ_MAKE) $@

cleansrcdir:
	@cd $(TOPSRCDIR); \
        if [ -f webshell/embed/gtk/Makefile ]; then \
          $(MAKE) -C webshell/embed/gtk distclean; \
        fi; \
	if [ -f Makefile ]; then \
	  $(MAKE) distclean ; \
	else \
	  echo "Removing object files from srcdir..."; \
	  rm -fr `find . -type d \( -name .deps -print -o -name CVS \
	          -o -exec test ! -d {}/CVS \; \) -prune \
	          -o \( -name '*.[ao]' -o -name '*.so' \) -type f -print`; \
	   build/autoconf/clean-config.sh; \
	fi;

# (! IS_FIRST_CHECKOUT)
endif

.PHONY: checkout real_checkout depend build export libs alldep install clean realclean distclean cleansrcdir pull_all build_all clobber clobber_all pull_and_build_all everything
