#!perl
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
$cvs_id = '@(#) $RCSfile$ $Revision$ $Date$';

$copyright = '/* THIS IS A GENERATED FILE */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
';

$count = -1;
$i = 0;

open(INPUT, "<$ARGV[0]") || die "Can't open $ARGV[0]: $!";

while(<INPUT>) {
  s/^((?:[^"#]+|"[^"]*")*)(\s*#.*$)/$1/;
  next if (/^\s*$/);

#  print;

  /^([\S]+)\s+([^"][\S]*|"[^"]*")/;
  $name = $1;
  $value = $2;

  if( ($name =~ "FUNCTION") && !($name =~ "CK_FUNCTION") ) {
    $count++;
    $x[$count]{name} = $value;
    $i = 0;
  } else {
    if( $count < 0 ) {
      $value =~ s/"//g;
      $g{$name} = $value;
    } else {
      $x[$count]{args}[$i]{type} = $name;
      $x[$count]{args}[$i]{name} = $value;
      $i++;
      $x[$count]{nargs} = $i; # rewritten each time, oh well
    }
  }
}

close INPUT;

# dodump();
doprint();

sub dodump {
  for( $j = 0; $j <= $count; $j++ ) {
    print "CK_RV CK_ENTRY $x[$j]{name}\n";
    for( $i = 0; $i < $x[$j]{nargs}; $i++ ) {
      print "  $x[$j]{args}[$i]{type} $x[$j]{args}[$i]{name}";
      if( $i == ($x[$j]{nargs} - 1) ) {
        print "\n";
      } else {
        print ",\n";
      }
    }
  }
}

sub doprint {
open(PROTOTYPE, ">nssckg.h") || die "Can't open nssckg.h: $!";
open(TYPEDEF, ">nssckft.h") || die "Can't open nssckft.h: $!";
open(EPV, ">nssckepv.h") || die "Can't open nssckepv.h: $!";
open(API, ">nssck.api") || die "Can't open nssck.api: $!";

select PROTOTYPE;

print $copyright;
print <<EOD
#ifndef NSSCKG_H
#define NSSCKG_H

#ifdef DEBUG
static const char NSSCKG_CVS_ID[] = "$g{CVS_ID} ; $cvs_id";
#endif /* DEBUG */

/*
 * nssckg.h
 *
 * This automatically-generated header file prototypes the Cryptoki
 * functions specified by PKCS#11.
 */

#ifndef NSSCKT_H
#include "nssckt.h"
#endif /* NSSCKT_H */

EOD
    ;

for( $j = 0; $j <= $count; $j++ ) {
  print "CK_RV CK_ENTRY $x[$j]{name}\n";
  print "(\n";
  for( $i = 0; $i < $x[$j]{nargs}; $i++ ) {
    print "  $x[$j]{args}[$i]{type} $x[$j]{args}[$i]{name}";
    if( $i == ($x[$j]{nargs} - 1) ) {
      print "\n";
    } else {
      print ",\n";
    }
  }
  print ");\n\n";
}

print <<EOD
#endif /* NSSCKG_H */
EOD
    ;

select TYPEDEF;

print $copyright;
print <<EOD
#ifndef NSSCKFT_H
#define NSSCKFT_H

#ifdef DEBUG
static const char NSSCKFT_CVS_ID[] = "$g{CVS_ID} ; $cvs_id";
#endif /* DEBUG */

/*
 * nssckft.h
 *
 * The automatically-generated header file declares a typedef
 * each of the Cryptoki functions specified by PKCS#11.
 */

#ifndef NSSCKT_H
#include "nssckt.h"
#endif /* NSSCKT_H */

EOD
    ;

for( $j = 0; $j <= $count; $j++ ) {
#  print "typedef CK_RV (CK_ENTRY *CK_$x[$j]{name})(\n";
  print "typedef CK_CALLBACK_FUNCTION(CK_RV, CK_$x[$j]{name})(\n";
  for( $i = 0; $i < $x[$j]{nargs}; $i++ ) {
    print "  $x[$j]{args}[$i]{type} $x[$j]{args}[$i]{name}";
    if( $i == ($x[$j]{nargs} - 1) ) {
      print "\n";
    } else {
      print ",\n";
    }
  }
  print ");\n\n";
}

print <<EOD
#endif /* NSSCKFT_H */
EOD
    ;

select EPV;

print $copyright;
print <<EOD
#ifndef NSSCKEPV_H
#define NSSCKEPV_H

#ifdef DEBUG
static const char NSSCKEPV_CVS_ID[] = "$g{CVS_ID} ; $cvs_id";
#endif /* DEBUG */

/*
 * nssckepv.h
 *
 * This automatically-generated header file defines the type
 * CK_FUNCTION_LIST specified by PKCS#11.
 */

#ifndef NSSCKT_H
#include "nssckt.h"
#endif /* NSSCKT_H */

#ifndef NSSCKFT_H
#include "nssckft.h"
#endif /* NSSCKFT_H */

#include "nssckp.h"

struct CK_FUNCTION_LIST {
  CK_VERSION version;
EOD
    ;

for( $j = 0; $j <= $count; $j++ ) {
  print "  CK_$x[$j]{name} $x[$j]{name};\n";
}

print <<EOD
};

#include "nsscku.h"

#endif /* NSSCKEPV_H */
EOD
    ;

select API;

print $copyright;
print <<EOD

#ifdef DEBUG
static const char NSSCKAPI_CVS_ID[] = "$g{CVS_ID} ; $cvs_id";
#endif /* DEBUG */

/*
 * nssck.api
 *
 * This automatically-generated file is used to generate a set of
 * Cryptoki entry points within the object space of a Module using
 * the NSS Cryptoki Framework.
 *
 * The Module should have a .c file with the following:
 *
 *  #define MODULE_NAME name
 *  #define INSTANCE_NAME instance
 *  #include "nssck.api"
 *
 * where "name" is some module-specific name that can be used to
 * disambiguate various modules.  This included file will then
 * define the actual Cryptoki routines which pass through to the
 * Framework calls.  All routines, except C_GetFunctionList, will
 * be prefixed with the name; C_GetFunctionList will be generated
 * to return an entry-point vector with these routines.  The
 * instance specified should be the basic instance of NSSCKMDInstance.
 *
 * If, prior to including nssck.api, the .c file also specifies
 *
 *  #define DECLARE_STRICT_CRYTPOKI_NAMES
 *
 * Then a set of "stub" routines not prefixed with the name will
 * be included.  This would allow the combined module and framework
 * to be used in applications which are hard-coded to use the
 * PKCS#11 names (instead of going through the EPV).  Please note
 * that such applications should be careful resolving symbols when
 * more than one PKCS#11 module is loaded.
 */

#ifndef MODULE_NAME
#error "Error: MODULE_NAME must be defined."
#endif /* MODULE_NAME */

#ifndef INSTANCE_NAME
#error "Error: INSTANCE_NAME must be defined."
#endif /* INSTANCE_NAME */

#ifndef NSSCKT_H
#include "nssckt.h"
#endif /* NSSCKT_H */

#ifndef NSSCKFWT_H
#include "nssckfwt.h"
#endif /* NSSCKFWT_H */

#ifndef NSSCKFWC_H
#include "nssckfwc.h"
#endif /* NSSCKFWC_H */

#ifndef NSSCKEPV_H
#include "nssckepv.h"
#endif /* NSSCKEPV_H */

#define ADJOIN(x,y) x##y

#define __ADJOIN(x,y) ADJOIN(x,y)

/*
 * The anchor.  This object is used to store an "anchor" pointer in
 * the Module's object space, so the wrapper functions can relate
 * back to this instance.
 */

static NSSCKFWInstance *fwInstance = (NSSCKFWInstance *)0;

static CK_RV CK_ENTRY
__ADJOIN(MODULE_NAME,C_Initialize)
(
  CK_VOID_PTR pInitArgs
)
{
  return NSSCKFWC_Initialize(&fwInstance, INSTANCE_NAME, pInitArgs);
}

#ifdef DECLARE_STRICT_CRYPTOKI_NAMES
CK_RV CK_ENTRY 
C_Initialize
(
  CK_VOID_PTR pInitArgs
)
{
  return __ADJOIN(MODULE_NAME,C_Initialize)(pInitArgs);
}
#endif /* DECLARE_STRICT_CRYPTOKI_NAMES */

static CK_RV CK_ENTRY
__ADJOIN(MODULE_NAME,C_Finalize)
(
  CK_VOID_PTR pReserved
)
{
  return NSSCKFWC_Finalize(&fwInstance);
}

#ifdef DECLARE_STRICT_CRYPTOKI_NAMES
CK_RV CK_ENTRY
C_Finalize
(
  CK_VOID_PTR pReserved
)
{
  return __ADJOIN(MODULE_NAME,C_Finalize)(pReserved);
}
#endif /* DECLARE_STRICT_CRYPTOKI_NAMES */

static CK_RV CK_ENTRY
__ADJOIN(MODULE_NAME,C_GetInfo)
(
  CK_INFO_PTR pInfo
)
{
  return NSSCKFWC_GetInfo(fwInstance, pInfo);
}

#ifdef DECLARE_STRICT_CRYPTOKI_NAMES
CK_RV CK_ENTRY
C_GetInfo
(
  CK_INFO_PTR pInfo
)
{
  return __ADJOIN(MODULE_NAME,C_GetInfo)(pInfo);
}
#endif /* DECLARE_STRICT_CRYPTOKI_NAMES */

/*
 * C_GetFunctionList is defined at the end.
 */

EOD
    ;

for( $j = 4; $j <= $count; $j++ ) {
  print "static CK_RV CK_ENTRY\n";
  print "__ADJOIN(MODULE_NAME,$x[$j]{name})\n";
  print "(\n";
  for( $i = 0; $i < $x[$j]{nargs}; $i++ ) {
    print "  $x[$j]{args}[$i]{type} $x[$j]{args}[$i]{name}";
    if( $i == ($x[$j]{nargs} - 1) ) {
      print "\n";
    } else {
      print ",\n";
    }
  }
  print ")\n";
  print "{\n";
  print "  return NSSCKFW$x[$j]{name}(fwInstance, ";
  for( $i = 0; $i < $x[$j]{nargs}; $i++ ) {
    print "$x[$j]{args}[$i]{name}";
    if( $i == ($x[$j]{nargs} - 1) ) {
      print ");\n";
    } else {
      print ", ";
    }
  }
  print "}\n\n";

  print "#ifdef DECLARE_STRICT_CRYPTOKI_NAMES\n";
  print "CK_RV CK_ENTRY\n";
  print "$x[$j]{name}\n";
  print "(\n";
  for( $i = 0; $i < $x[$j]{nargs}; $i++ ) {
    print "  $x[$j]{args}[$i]{type} $x[$j]{args}[$i]{name}";
    if( $i == ($x[$j]{nargs} - 1) ) {
      print "\n";
    } else {
      print ",\n";
    }
  }
  print ")\n";
  print "{\n";
  print "  return __ADJOIN(MODULE_NAME,$x[$j]{name})(";
  for( $i = 0; $i < $x[$j]{nargs}; $i++ ) {
    print "$x[$j]{args}[$i]{name}";
    if( $i == ($x[$j]{nargs} - 1) ) {
      print ");\n";
    } else {
      print ", ";
    }
  }
  print "}\n";
  print "#endif /* DECLARE_STRICT_CRYPTOKI_NAMES */\n\n";
}

print <<EOD
static CK_RV CK_ENTRY
__ADJOIN(MODULE_NAME,C_GetFunctionList)
(
  CK_FUNCTION_LIST_PTR_PTR ppFunctionList
);

static CK_FUNCTION_LIST FunctionList = {
  { 2, 1 },
EOD
    ;

for( $j = 0; $j <= $count; $j++ ) {
  print "__ADJOIN(MODULE_NAME,$x[$j]{name})";
  if( $j < $count ) {
    print ",\n";
  } else {
    print "\n};\n\n";
  }
}

print <<EOD
static CK_RV CK_ENTRY
__ADJOIN(MODULE_NAME,C_GetFunctionList)
(
  CK_FUNCTION_LIST_PTR_PTR ppFunctionList
)
{
  *ppFunctionList = &FunctionList;
  return CKR_OK;
}

/* This one is always present */
CK_RV CK_ENTRY
C_GetFunctionList
(
  CK_FUNCTION_LIST_PTR_PTR ppFunctionList
)
{
  return __ADJOIN(MODULE_NAME,C_GetFunctionList)(ppFunctionList);
}

#undef __ADJOIN

EOD
    ;

select STDOUT;

}
