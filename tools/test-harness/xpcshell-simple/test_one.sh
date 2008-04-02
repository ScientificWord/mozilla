#!/bin/sh
#
# vim:set ts=2 sw=2 sts=2 et:
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
# The Original Code is mozilla.org code.
#
# The Initial Developer of the Original Code is Google Inc.
# Portions created by the Initial Developer are Copyright (C) 2005
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#  Darin Fisher <darin@meer.net>
#  Dave Liebreich <davel@mozilla.com>
#  Jeff Walden <jwalden+code@mit.edu>
#  Alexander J. Vincent <ajvincent@gmail.com> (single-test mode)
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
# ***** END LICENSE BLOCK ***** */

# allow core dumps
ulimit -c 20480 2> /dev/null

# Make assertions fatal
XPCOM_DEBUG_BREAK=stack-and-abort; export XPCOM_DEBUG_BREAK

exit_status=0

# Launch single tests by calling:
# make SOLO_FILE=(filename) -C (test directory) check-interactive
# js>_execute_test();
# js>quit();

##########################
# COMMAND-LINE ARGUMENTS #
##########################

# This provides the path to xpcshell.
xpcshell="$1"

# The directory containing Mozilla source code is specified as the first
# argument to this script, in a format usable by load() in xpcshell.  This
# enables do_import_script to work for any JS file in the source tree.
topsrcdir="$2"

# Tests which depend on files in the source directory will need a native path
# to actually access those files, or otherwise they must rely on hacks such as
# getting the current working/process directory and committing other atrocities.
# This argument is a native path to the top-level source directory, useful for
# tests which require access to files so that they can access them in a vaguely
# clean manner.
native_topsrcdir="$3"

# The sample Makefile for the xpcshell-simple harness adds the directory where
# the test_*.js files reside as an arg.  If no arg is specified, assume the
# current directory is where the *.js files live.
testdir="$4"
if [ "x$testdir" = "x" ]; then
    testdir=.
fi

# The actual JS test to run.
target_js="$5"

# Says if the test should be launch in interactive mode or not
interactive_mode="$6"

###############################
# SETUP FOR RUNNING THE TESTS #
###############################

# files matching the pattern head_*.js are treated like test setup files
# - they are run after head.js but before the test file
headfiles="-f $topsrcdir/tools/test-harness/xpcshell-simple/head.js"
for h in $testdir/unit/head_*.js
do
    if [ -f $h ]; then
        headfiles="$headfiles -f $h"
    fi
done

# files matching the pattern tail_*.js are treated like teardown files
# - they are run after tail.js
tailfiles="-f $topsrcdir/tools/test-harness/xpcshell-simple/tail.js"

if [ ! "$interactive_mode" = "1" ]; then
    tailfiles="$tailfiles -f $topsrcdir/tools/test-harness/xpcshell-simple/execute_test.js"
fi

for t in $testdir/unit/tail_*.js
do
    if [ -f $t ]; then
        tailfiles="$tailfiles -f $t"
    fi
done


############
# RUN TEST #
############

echo "NATIVE_TOPSRCDIR='$native_topsrcdir' TOPSRCDIR='$topsrcdir' $xpcshell -s $headfiles -f $testdir/unit/$target_js $tailfiles 2>&1"
echo -n "$target_js: "
if [ ! "$interactive_mode" = "1" ]; then
    NATIVE_TOPSRCDIR="$native_topsrcdir" TOPSRCDIR="$topsrcdir" $xpcshell -s $headfiles -f $testdir/unit/$target_js $tailfiles  2> $testdir/unit/$target_js.log 1>&2
else
    NATIVE_TOPSRCDIR="$native_topsrcdir" TOPSRCDIR="$topsrcdir" $xpcshell -s $headfiles -f $testdir/unit/$target_js $tailfiles -i 2>&1
fi
rv="$?"
if [ ! "$rv" = "0"  -o \
     `grep -c '\*\*\* PASS' $testdir/unit/$target_js.log` = 0 ]
then
    echo "FAIL"
    echo "$target_js.log:"
    echo ">>>>>>>"
    cat $testdir/unit/$target_js.log
    echo ""
    echo "<<<<<<<"
    exit_status=1
else
    echo "PASS"
fi

exit $exit_status
