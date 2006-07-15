#! /bin/sh  
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

########################################################################
# mozilla/security/nss/tests/fips/fips.sh
#
# Script to test basic functionallity of NSS in FIPS-compliant mode
#
# needs to work on all Unix and Windows platforms
#
# tests implemented:
#
# special strings
# ---------------
#
########################################################################

############################## fips_init ##############################
# local shell function to initialize this script 
########################################################################
fips_init()
{
  SCRIPTNAME=fips.sh      # sourced - $0 would point to all.sh

  if [ -z "${CLEANUP}" ] ; then     # if nobody else is responsible for
      CLEANUP="${SCRIPTNAME}"       # cleaning this script will do it
  fi

  if [ -z "${INIT_SOURCED}" -o "${INIT_SOURCED}" != "TRUE" ]; then
      cd ../common
      . ./init.sh
  fi
  if [ ! -r $CERT_LOG_FILE ]; then  # we need certificates here
      cd ../cert
      . ./cert.sh
  fi
  SCRIPTNAME=fips.sh
  html_head "FIPS 140 Compliance Tests"

  grep "SUCCESS: FIPS passed" $CERT_LOG_FILE >/dev/null || {
      Exit 15 "Fatal - FIPS of cert.sh needs to pass first"
  }

  COPYDIR=${FIPSDIR}/copydir

  R_FIPSDIR=../fips
  P_R_FIPSDIR=../fips
  R_COPYDIR=../fips/copydir

  if [ -n "${MULTIACCESS_DBM}" ]; then
     P_R_FIPSDIR="multiaccess:${D_FIPS}"
  fi

  mkdir -p ${FIPSDIR}
  mkdir -p ${COPYDIR}

  cd ${FIPSDIR}
}

############################## fips_140 ##############################
# local shell function to test basic functionality of NSS while in
# FIPS 140 compliant mode
########################################################################
fips_140()
{
  echo "$SCRIPTNAME: Verify this module is in FIPS mode  -----------------"
  echo "modutil -dbdir ${P_R_FIPSDIR} -list"
  modutil -dbdir ${P_R_FIPSDIR} -list 2>&1
  modutil -dbdir ${P_R_FIPSDIR} -chkfips true 2>&1
  html_msg $? 0 "Verify this module is in FIPS mode (modutil -chkfips true)"

  echo "$SCRIPTNAME: List the FIPS module certificates -----------------"
  echo "certutil -d ${P_R_FIPSDIR} -L"
  certutil -d ${P_R_FIPSDIR} -L 2>&1
  html_msg $? 0 "List the FIPS module certificates (certutil -L)"

  echo "$SCRIPTNAME: List the FIPS module keys -------------------------"
  echo "certutil -d ${P_R_FIPSDIR} -K -f ${R_FIPSPWFILE}"
  certutil -d ${P_R_FIPSDIR} -K -f ${R_FIPSPWFILE} 2>&1
  html_msg $? 0 "List the FIPS module keys (certutil -K)"

  echo "$SCRIPTNAME: Attempt to list FIPS module keys with incorrect password"
  echo "certutil -d ${P_R_FIPSDIR} -K -f ${FIPSBADPWFILE}"
  certutil -d ${P_R_FIPSDIR} -K -f ${FIPSBADPWFILE} 2>&1
  RET=$?
  html_msg $RET 255 "Attempt to list FIPS module keys with incorrect password (certutil -K)"
  echo "certutil -K returned $RET"

  echo "$SCRIPTNAME: Validate the certificate --------------------------"
  echo "certutil -d ${P_R_FIPSDIR} -V -n ${FIPSCERTNICK} -u SR -e -f ${R_FIPSPWFILE}"
  certutil -d ${P_R_FIPSDIR} -V -n ${FIPSCERTNICK} -u SR -e -f ${R_FIPSPWFILE}
  html_msg $? 0 "Validate the certificate (certutil -V -e)"

  echo "$SCRIPTNAME: Export the certificate and key as a PKCS#12 file --"
  echo "pk12util -d ${P_R_FIPSDIR} -o fips140.p12 -n ${FIPSCERTNICK} -w ${R_FIPSP12PWFILE} -k ${R_FIPSPWFILE}"
  pk12util -d ${P_R_FIPSDIR} -o fips140.p12 -n ${FIPSCERTNICK} -w ${R_FIPSP12PWFILE} -k ${R_FIPSPWFILE} 2>&1
  html_msg $? 0 "Export the certificate and key as a PKCS#12 file (pk12util -o)"

  echo "$SCRIPTNAME: Export the certificate as a DER-encoded file ------"
  echo "certutil -d ${P_R_FIPSDIR} -L -n ${FIPSCERTNICK} -r -o fips140.crt"
  certutil -d ${P_R_FIPSDIR} -L -n ${FIPSCERTNICK} -r -o fips140.crt 2>&1
  html_msg $? 0 "Export the certificate as a DER (certutil -L -r)"

  echo "$SCRIPTNAME: List the FIPS module certificates -----------------"
  echo "certutil -d ${P_R_FIPSDIR} -L"
  certutil -d ${P_R_FIPSDIR} -L 2>&1
  html_msg $? 0 "List the FIPS module certificates (certutil -L)"

  echo "$SCRIPTNAME: Delete the certificate and key from the FIPS module"
  echo "certutil -d ${P_R_FIPSDIR} -F -n ${FIPSCERTNICK} -f ${R_FIPSPWFILE}"
  certutil -d ${P_R_FIPSDIR} -F -n ${FIPSCERTNICK} -f ${R_FIPSPWFILE} 2>&1
  html_msg $? 0 "Delete the certificate and key from the FIPS module (certutil -D)"


  echo "$SCRIPTNAME: List the FIPS module certificates -----------------"
  echo "certutil -d ${P_R_FIPSDIR} -L"
  certutil -d ${P_R_FIPSDIR} -L 2>&1
  html_msg $? 0 "List the FIPS module certificates (certutil -L)"

  echo "$SCRIPTNAME: List the FIPS module keys."
  echo "certutil -d ${P_R_FIPSDIR} -K -f ${R_FIPSPWFILE}"
  certutil -d ${P_R_FIPSDIR} -K -f ${R_FIPSPWFILE} 2>&1
  # certutil -K now returns a failure if no keys are found. This verifies that
  # our delete succeded.
  html_msg $? 255 "List the FIPS module keys (certutil -K)"

  echo "$SCRIPTNAME: Import the certificate and key from the PKCS#12 file"
  echo "pk12util -d ${P_R_FIPSDIR} -i fips140.p12 -w ${R_FIPSP12PWFILE} -k ${R_FIPSPWFILE}"
  pk12util -d ${P_R_FIPSDIR} -i fips140.p12 -w ${R_FIPSP12PWFILE} -k ${R_FIPSPWFILE} 2>&1
  html_msg $? 0 "Import the certificate and key from the PKCS#12 file (pk12util -i)"

  echo "$SCRIPTNAME: List the FIPS module certificates -----------------"
  echo "certutil -d ${P_R_FIPSDIR} -L"
  certutil -d ${P_R_FIPSDIR} -L 2>&1
  html_msg $? 0 "List the FIPS module certificates (certutil -L)"

  echo "$SCRIPTNAME: List the FIPS module keys --------------------------"
  echo "certutil -d ${P_R_FIPSDIR} -K -f ${R_FIPSPWFILE}"
  certutil -d ${P_R_FIPSDIR} -K -f ${R_FIPSPWFILE} 2>&1
  html_msg $? 0 "List the FIPS module keys (certutil -K)"

#
# This test was interfering with QA running on more than one machine pointing
# to the same binary file. Turn it off for now.
#
#  echo "$SCRIPTNAME: Detect mangled database --------------------------"
#  SOFTOKEN=${DIST}/${OBJDIR}/lib/${DLL_PREFIX}softokn3.${DLL_SUFFIX}
#  echo "cp ${SOFTOKEN} ${TMP}/softokn3.sav"
#  cp ${SOFTOKEN} ${TMP}/softokn3.sav
#  echo "mangling ${SOFTOKEN}"
#  echo "mangle -i ${SOFTOKEN} -o 60000 -b 5"
#  mangle -i ${SOFTOKEN} -o 60000 -b 5 2>&1
#  if [ $? -eq 0 ]; then
#    echo "dbtest -r  -d ${P_R_FIPSDIR} "
# suppress the expected failure message
#    dbtest -r  -d ${P_R_FIPSDIR}  > ${TMP}/dbtestoutput.txt 2>&1
#    html_msg $? 46 "Init NSS with a corrupted library (dbtest -r)"
#    echo "cp ${TMP}/softokn3.sav ${SOFTOKEN}"
#    cp ${TMP}/softokn3.sav ${SOFTOKEN}
#  else
#    html_msg 0 0 "Skipping corruption test, can't open ${DLL_PREFIX}softokn3.${DLL_SUFFIX}"
#  fi
#  echo "rm ${TMP}/softokn3.sav"
#  rm ${TMP}/softokn3.sav
}

############################## fips_cleanup ############################
# local shell function to finish this script (no exit since it might be 
# sourced)
########################################################################
fips_cleanup()
{
  html "</TABLE><BR>"
  cd ${QADIR}
  . common/cleanup.sh
}

################## main #################################################

fips_init

fips_140
fips_cleanup

