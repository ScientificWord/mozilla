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
#   Dr Vipul Gupta <vipul.gupta@sun.com>, Sun Microsystems Laboratories
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
#
# mozilla/security/nss/tests/cert/rcert.sh
#
# Certificate generating and handeling for NSS QA, can be included 
# multiple times from all.sh and the individual scripts
#
# needs to work on all Unix and Windows platforms
#
# included from (don't expect this to be up to date)
# --------------------------------------------------
#   all.sh
#   ssl.sh
#   smime.sh
#   tools.sh
#
# special strings
# ---------------
#   FIXME ... known problems, search for this string
#   NOTE .... unexpected behavior
#
# FIXME - Netscape - NSS
########################################################################

############################## cert_init ###############################
# local shell function to initialize this script
########################################################################
cert_init()
{
  SCRIPTNAME="cert.sh"
  if [ -z "${CLEANUP}" ] ; then     # if nobody else is responsible for
      CLEANUP="${SCRIPTNAME}"       # cleaning this script will do it
  fi
  if [ -z "${INIT_SOURCED}" ] ; then
      cd ../common
      . ./init.sh
  fi
  SCRIPTNAME="cert.sh"
  CRL_GRP_DATE=`date "+%Y%m%d%H%M%SZ"`
  if [ -n "$NSS_ENABLE_ECC" ] ; then
      html_head "Certutil and Crlutil Tests with ECC"
  else
      html_head "Certutil and Crlutil Tests"
  fi

  ################## Generate noise for our CA cert. ######################
  # NOTE: these keys are only suitable for testing, as this whole thing 
  # bypasses the entropy gathering. Don't use this method to generate 
  # keys and certs for product use or deployment.
  #
  ps -efl > ${NOISE_FILE} 2>&1
  ps aux >> ${NOISE_FILE} 2>&1
  noise

}

cert_log() ######################    write the cert_status file
{
    echo "$SCRIPTNAME $*"
    echo $* >>${CERT_LOG_FILE}
}

################################ noise ##################################
# Generate noise for our certs
#
# NOTE: these keys are only suitable for testing, as this whole thing bypasses
# the entropy gathering. Don't use this method to generate keys and certs for
# product use or deployment.
#########################################################################
noise()
{
    #netstat >> ${NOISE_FILE} 2>&1
    date >> ${NOISE_FILE} 2>&1
}

################################ certu #################################
# local shell function to call certutil, also: writes action and options to
# stdout, sets variable RET and writes results to the html file results
########################################################################
certu()
{
    echo "$SCRIPTNAME: ${CU_ACTION} --------------------------"

    if [ -n "${CU_SUBJECT}" ]; then
        #the subject of the cert contains blanks, and the shell 
        #will strip the quotes off the string, if called otherwise...
        echo "certutil -s \"${CU_SUBJECT}\" $*"
        certutil -s "${CU_SUBJECT}" $*
        RET=$?
        CU_SUBJECT=""
    else
        echo "certutil $*"
        certutil $*
        RET=$?
    fi
    if [ "$RET" -ne 0 ]; then
        CERTFAILED=$RET
        html_failed "<TR><TD>${CU_ACTION} ($RET) " 
        cert_log "ERROR: ${CU_ACTION} failed $RET"
    else
        html_passed "<TR><TD>${CU_ACTION}"
    fi

    # echo "Contine?"
    # cat > /dev/null
    return $RET
}

################################ crlu #################################
# local shell function to call crlutil, also: writes action and options to
# stdout, sets variable RET and writes results to the html file results
########################################################################
crlu()
{
    echo "$SCRIPTNAME: ${CU_ACTION} --------------------------"
    
    CRLUTIL="crlutil -q"
    echo "$CRLUTIL $*"
    $CRLUTIL $*
    RET=$?
    if [ "$RET" -ne 0 ]; then
        CRLFAILED=$RET
        html_failed "<TR><TD>${CU_ACTION} ($RET) " 
        cert_log "ERROR: ${CU_ACTION} failed $RET"
    else
        html_passed "<TR><TD>${CU_ACTION}"
    fi

    # echo "Contine?"
    # cat > /dev/null
    return $RET
}

############################# cert_init_cert ##########################
# local shell function to initialize creation of client and server certs
########################################################################
cert_init_cert()
{
    CERTDIR="$1"
    CERTNAME="$2"
    CERTSERIAL="$3"
    DOMAIN="$4"

    if [ ! -d "${CERTDIR}" ]; then
        mkdir -p "${CERTDIR}"
    else
        echo "$SCRIPTNAME: WARNING - ${CERTDIR} exists"
    fi
    cd "${CERTDIR}"
    CERTDIR="." 

    PROFILEDIR=${CERTDIR}
    if [ -n "${MULTIACCESS_DBM}" ]; then
	PROFILEDIR="multiaccess:${DOMAIN}"
    fi

    noise
}

############################# hw_acc #################################
# local shell function to add hw accelerator modules to the db
########################################################################
hw_acc()
{
    HW_ACC_RET=0
    HW_ACC_ERR=""
    if [ -n "$O_HWACC" -a "$O_HWACC" = ON -a -z "$USE_64" ] ; then
        echo "creating $CERTNAME s cert with hwaccelerator..."
        #case $ACCELERATOR in
        #rainbow)
   

        echo "modutil -add rainbow -libfile /usr/lib/libcryptoki22.so "
        echo "         -dbdir ${PROFILEDIR} 2>&1 "
        echo | modutil -add rainbow -libfile /usr/lib/libcryptoki22.so \
            -dbdir ${PROFILEDIR} 2>&1 
        if [ "$?" -ne 0 ]; then
            echo "modutil -add rainbow failed in `pwd`"
            HW_ACC_RET=1
            HW_ACC_ERR="modutil -add rainbow"
        fi
    
        echo "modutil -add ncipher "
        echo "         -libfile /opt/nfast/toolkits/pkcs11/libcknfast.so "
        echo "         -dbdir ${PROFILEDIR} 2>&1 "
        echo | modutil -add ncipher \
            -libfile /opt/nfast/toolkits/pkcs11/libcknfast.so \
            -dbdir ${PROFILEDIR} 2>&1 
        if [ "$?" -ne 0 ]; then
            echo "modutil -add ncipher failed in `pwd`"
            HW_ACC_RET=`expr $HW_ACC_RET + 2`
            HW_ACC_ERR="$HW_ACC_ERR,modutil -add ncipher"
        fi
        if [ "$HW_ACC_RET" -ne 0 ]; then
            html_failed "<TR><TD>Adding HW accelerators to certDB for ${CERTNAME} ($HW_ACC_RET) " 
        else
            html_passed "<TR><TD>Adding HW accelerators to certDB for ${CERTNAME}"
        fi

    fi
    return $HW_ACC_RET
}

############################# cert_create_cert #########################
# local shell function to create client certs 
#     initialize DB, import
#     root cert
#     add cert to DB
########################################################################
cert_create_cert()
{
    cert_init_cert "$1" "$2" "$3" "$4"

    CU_ACTION="Initializing ${CERTNAME}'s Cert DB"
    certu -N -d "${PROFILEDIR}" -f "${R_PWFILE}" 2>&1
    if [ "$RET" -ne 0 ]; then
        return $RET
    fi
    hw_acc
    CU_ACTION="Import Root CA for $CERTNAME"
    certu -A -n "TestCA" -t "TC,TC,TC" -f "${R_PWFILE}" -d "${PROFILEDIR}" \
          -i "${R_CADIR}/root.cert" 2>&1
    if [ "$RET" -ne 0 ]; then
        return $RET
    fi
    if [ -n "$NSS_ENABLE_ECC" ] ; then
	CU_ACTION="Import EC Root CA for $CERTNAME"
	certu -A -n "TestCA-ec" -t "TC,TC,TC" -f "${R_PWFILE}" \
	    -d "${PROFILEDIR}" -i "${R_CADIR}/ecroot.cert" 2>&1
	if [ "$RET" -ne 0 ]; then
            return $RET
	fi
    fi
    cert_add_cert "$5"
    return $?
}

############################# cert_add_cert ############################
# local shell function to add client certs to an existing CERT DB
#     generate request
#     sign request
#     import Cert
#
########################################################################
cert_add_cert()
{
    CU_ACTION="Generate Cert Request for $CERTNAME"
    CU_SUBJECT="CN=$CERTNAME, E=${CERTNAME}@bogus.com, O=BOGUS NSS, L=Mountain View, ST=California, C=US"
    certu -R -d "${PROFILEDIR}" -f "${R_PWFILE}" -z "${R_NOISE_FILE}" -o req  2>&1
    if [ "$RET" -ne 0 ]; then
        return $RET
    fi

    CU_ACTION="Sign ${CERTNAME}'s Request"
    certu -C -c "TestCA" -m "$CERTSERIAL" -v 60 -d "${P_R_CADIR}" \
          -i req -o "${CERTNAME}.cert" -f "${R_PWFILE}" "$1" 2>&1
    if [ "$RET" -ne 0 ]; then
        return $RET
    fi

    CU_ACTION="Import $CERTNAME's Cert"
    certu -A -n "$CERTNAME" -t "u,u,u" -d "${PROFILEDIR}" -f "${R_PWFILE}" \
          -i "${CERTNAME}.cert" 2>&1
    if [ "$RET" -ne 0 ]; then
        return $RET
    fi

    cert_log "SUCCESS: $CERTNAME's Cert Created"

#
#   Generate and add EC cert
#
    if [ -n "$NSS_ENABLE_ECC" ] ; then
	CURVE="secp384r1"
	CU_ACTION="Generate EC Cert Request for $CERTNAME"
	CU_SUBJECT="CN=$CERTNAME, E=${CERTNAME}-ec@bogus.com, O=BOGUS NSS, L=Mountain View, ST=California, C=US"
	certu -R -k ec -q "${CURVE}" -d "${PROFILEDIR}" -f "${R_PWFILE}" \
	    -z "${R_NOISE_FILE}" -o req  2>&1
	if [ "$RET" -ne 0 ]; then
            return $RET
	fi

	CU_ACTION="Sign ${CERTNAME}'s EC Request"
	certu -C -c "TestCA-ec" -m "$CERTSERIAL" -v 60 -d "${P_R_CADIR}" \
            -i req -o "${CERTNAME}-ec.cert" -f "${R_PWFILE}" "$1" 2>&1
	if [ "$RET" -ne 0 ]; then
            return $RET
	fi

	CU_ACTION="Import $CERTNAME's EC Cert"
	certu -A -n "${CERTNAME}-ec" -t "u,u,u" -d "${PROFILEDIR}" \
	    -f "${R_PWFILE}" -i "${CERTNAME}-ec.cert" 2>&1
	if [ "$RET" -ne 0 ]; then
            return $RET
	fi
	cert_log "SUCCESS: $CERTNAME's EC Cert Created"
    fi

    return 0
}

################################# cert_all_CA ################################
# local shell function to build the additional Temp. Certificate Authority (CA)
# used for the "real life" ssl test with 2 different CA's in the
# client and in teh server's dir
##########################################################################
cert_all_CA()
{
    echo nss > ${PWFILE}

    ALL_CU_SUBJECT="CN=NSS Test CA, O=BOGUS NSS, L=Mountain View, ST=California, C=US"
    cert_CA $CADIR TestCA -x "CTu,CTu,CTu" ${D_CA} "1"

    ALL_CU_SUBJECT="CN=NSS Server Test CA, O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
    cert_CA $SERVER_CADIR serverCA -x "Cu,Cu,Cu" ${D_SERVER_CA} "2"
    ALL_CU_SUBJECT="CN=NSS Chain1 Server Test CA, O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
    cert_CA $SERVER_CADIR chain-1-serverCA "-c serverCA" "u,u,u" ${D_SERVER_CA} "3"
    ALL_CU_SUBJECT="CN=NSS Chain2 Server Test CA, O=BOGUS NSS, L=Santa Clara, ST=California, C=US" 
    cert_CA $SERVER_CADIR chain-2-serverCA "-c chain-1-serverCA" "u,u,u" ${D_SERVER_CA} "4"



    ALL_CU_SUBJECT="CN=NSS Client Test CA, O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
    cert_CA $CLIENT_CADIR clientCA -x "Tu,Cu,Cu" ${D_CLIENT_CA} "5"
    ALL_CU_SUBJECT="CN=NSS Chain1 Client Test CA, O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
    cert_CA $CLIENT_CADIR chain-1-clientCA "-c clientCA" "u,u,u" ${D_CLIENT_CA} "6"
    ALL_CU_SUBJECT="CN=NSS Chain2 Client Test CA, O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
    cert_CA $CLIENT_CADIR chain-2-clientCA "-c chain-1-clientCA" "u,u,u" ${D_CLIENT_CA} "7"

    rm $CLIENT_CADIR/root.cert $SERVER_CADIR/root.cert

    # root.cert in $CLIENT_CADIR and in $SERVER_CADIR is one of the last 
    # in the chain

    if [ -n "$NSS_ENABLE_ECC" ] ; then
#
#       Create EC version of TestCA
	CA_CURVE="secp521r1"
	ALL_CU_SUBJECT="CN=NSS Test CA (ECC), O=BOGUS NSS, L=Mountain View, ST=California, C=US"
	cert_ec_CA $CADIR TestCA-ec -x "CTu,CTu,CTu" ${D_CA} "1" ${CA_CURVE}
#
#       Create EC versions of the intermediate CA certs
	ALL_CU_SUBJECT="CN=NSS Server Test CA (ECC), O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
	cert_ec_CA $SERVER_CADIR serverCA-ec -x "Cu,Cu,Cu" ${D_SERVER_CA} "2" ${CA_CURVE}
	ALL_CU_SUBJECT="CN=NSS Chain1 Server Test CA (ECC), O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
	cert_ec_CA $SERVER_CADIR chain-1-serverCA-ec "-c serverCA-ec" "u,u,u" ${D_SERVER_CA} "3" ${CA_CURVE}
	ALL_CU_SUBJECT="CN=NSS Chain2 Server Test CA (ECC), O=BOGUS NSS, L=Santa Clara, ST=California, C=US" 
	cert_ec_CA $SERVER_CADIR chain-2-serverCA-ec "-c chain-1-serverCA-ec" "u,u,u" ${D_SERVER_CA} "4" ${CA_CURVE}

	ALL_CU_SUBJECT="CN=NSS Client Test CA (ECC), O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
	cert_ec_CA $CLIENT_CADIR clientCA-ec -x "Tu,Cu,Cu" ${D_CLIENT_CA} "5" ${CA_CURVE}
	ALL_CU_SUBJECT="CN=NSS Chain1 Client Test CA (ECC), O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
	cert_ec_CA $CLIENT_CADIR chain-1-clientCA-ec "-c clientCA-ec" "u,u,u" ${D_CLIENT_CA} "6" ${CA_CURVE}
	ALL_CU_SUBJECT="CN=NSS Chain2 Client Test CA (ECC), O=BOGUS NSS, L=Santa Clara, ST=California, C=US"
	cert_ec_CA $CLIENT_CADIR chain-2-clientCA-ec "-c chain-1-clientCA-ec" "u,u,u" ${D_CLIENT_CA} "7" ${CA_CURVE}

	rm $CLIENT_CADIR/ecroot.cert $SERVER_CADIR/ecroot.cert
#	ecroot.cert in $CLIENT_CADIR and in $SERVER_CADIR is one of the last 
#	in the chain

    fi
}

################################# cert_CA ################################
# local shell function to build the Temp. Certificate Authority (CA)
# used for testing purposes, creating  a CA Certificate and a root cert
##########################################################################
cert_CA()
{
  CUR_CADIR=$1
  NICKNAME=$2
  SIGNER=$3
  TRUSTARG=$4
  DOMAIN=$5
  CERTSERIAL=$6

  echo "$SCRIPTNAME: Creating a CA Certificate $NICKNAME =========================="

  if [ ! -d "${CUR_CADIR}" ]; then
      mkdir -p "${CUR_CADIR}"
  fi
  cd ${CUR_CADIR}
  pwd

  LPROFILE=.
  if [ -n "${MULTIACCESS_DBM}" ]; then
	LPROFILE="multiaccess:${DOMAIN}"
  fi

  if [ "$SIGNER" = "-x" ] ; then # self signed -> create DB
      CU_ACTION="Creating CA Cert DB"
      certu -N -d ${LPROFILE} -f ${R_PWFILE} 2>&1
      if [ "$RET" -ne 0 ]; then
          Exit 5 "Fatal - failed to create CA $NICKNAME "
      fi
      echo "$SCRIPTNAME: Certificate initialized ----------"
  fi


  ################# Creating CA Cert ######################################
  #
  CU_ACTION="Creating CA Cert $NICKNAME "
  CU_SUBJECT=$ALL_CU_SUBJECT
  certu -S -n $NICKNAME -t $TRUSTARG -v 600 $SIGNER -d ${LPROFILE} -1 -2 -5 \
        -f ${R_PWFILE} -z ${R_NOISE_FILE} -m $CERTSERIAL 2>&1 <<CERTSCRIPT
5
6
9
n
y
-1
n
5
6
7
9
n
CERTSCRIPT

  if [ "$RET" -ne 0 ]; then
      echo "return value is $RET"
      Exit 6 "Fatal - failed to create CA cert"
  fi

  ################# Exporting Root Cert ###################################
  #
  CU_ACTION="Exporting Root Cert"
  certu -L -n  $NICKNAME -r -d ${LPROFILE} -o root.cert 
  if [ "$RET" -ne 0 ]; then
      Exit 7 "Fatal - failed to export root cert"
  fi
  cp root.cert ${NICKNAME}.ca.cert
}

################################ cert_ec_CA ##############################
# local shell function to build the Temp. Certificate Authority (CA)
# used for testing purposes, creating  a CA Certificate and a root cert
# This is the ECC version of cert_CA.
##########################################################################
cert_ec_CA()
{
  CUR_CADIR=$1
  NICKNAME=$2
  SIGNER=$3
  TRUSTARG=$4
  DOMAIN=$5
  CERTSERIAL=$6
  CURVE=$7

  echo "$SCRIPTNAME: Creating an EC CA Certificate $NICKNAME =========================="

  if [ ! -d "${CUR_CADIR}" ]; then
      mkdir -p "${CUR_CADIR}"
  fi
  cd ${CUR_CADIR}
  pwd

  LPROFILE=.
  if [ -n "${MULTIACCESS_DBM}" ]; then
	LPROFILE="multiaccess:${DOMAIN}"
  fi

  ################# Creating an EC CA Cert ################################
  #
  CU_ACTION="Creating EC CA Cert $NICKNAME "
  CU_SUBJECT=$ALL_CU_SUBJECT
  certu -S -n $NICKNAME -k ec -q $CURVE -t $TRUSTARG -v 600 $SIGNER \
    -d ${LPROFILE} -1 -2 -5 -f ${R_PWFILE} -z ${R_NOISE_FILE} \
    -m $CERTSERIAL 2>&1 <<CERTSCRIPT
5
6
9
n
y
-1
n
5
6
7
9
n
CERTSCRIPT

  if [ "$RET" -ne 0 ]; then
      echo "return value is $RET"
      Exit 6 "Fatal - failed to create EC CA cert"
  fi

  ################# Exporting EC Root Cert ################################
  #
  CU_ACTION="Exporting EC Root Cert"
  certu -L -n  $NICKNAME -r -d ${LPROFILE} -o ecroot.cert 
  if [ "$RET" -ne 0 ]; then
      Exit 7 "Fatal - failed to export ec root cert"
  fi
  cp ecroot.cert ${NICKNAME}.ca.cert
}

############################## cert_smime_client #############################
# local shell function to create client Certificates for S/MIME tests 
##############################################################################
cert_smime_client()
{
  CERTFAILED=0
  echo "$SCRIPTNAME: Creating Client CA Issued Certificates =============="

  cert_create_cert ${ALICEDIR} "Alice" 30 ${D_ALICE}
  cert_create_cert ${BOBDIR} "Bob" 40  ${D_BOB}

  echo "$SCRIPTNAME: Creating Dave's Certificate -------------------------"
  cert_create_cert "${DAVEDIR}" Dave 50 ${D_DAVE}

## XXX With this new script merging ECC and non-ECC tests, the
## call to cert_create_cert ends up creating two separate certs
## one for Eve and another for Eve-ec but they both end up with
## the same Subject Alt Name Extension, i.e., both the cert for
## Eve@bogus.com and the cert for Eve-ec@bogus.com end up 
## listing eve@bogus.net in the Certificate Subject Alt Name extension. 
## This can cause a problem later when cmsutil attempts to create
## enveloped data and accidently picks up the ECC cert (NSS currently
## does not support ECC for enveloped data creation). This script
## avoids the problem by ensuring that these conflicting certs are
## never added to the same cert database (see comment marked XXXX).
  echo "$SCRIPTNAME: Creating multiEmail's Certificate --------------------"
  cert_create_cert "${EVEDIR}" "Eve" 60 ${D_EVE} "-7 eve@bogus.net,eve@bogus.cc,beve@bogus.com"

  #echo "************* Copying CA files to ${SERVERDIR}"
  #cp ${CADIR}/*.db .
  #hw_acc

  #########################################################################
  #
  #cd ${CERTDIR}
  #CU_ACTION="Creating ${CERTNAME}'s Server Cert"
  #CU_SUBJECT="CN=${CERTNAME}, E=${CERTNAME}@bogus.com, O=BOGUS Netscape, L=Mountain View, ST=California, C=US"
  #certu -S -n "${CERTNAME}" -c "TestCA" -t "u,u,u" -m "$CERTSERIAL" \
  #	-d ${PROFILEDIR} -f "${R_PWFILE}" -z "${R_NOISE_FILE}" -v 60 2>&1

  #CU_ACTION="Export Dave's Cert"
  #cd ${DAVEDIR}
  #certu -L -n "Dave" -r -d ${P_R_DAVE} -o Dave.cert

  ################# Importing Certificates for S/MIME tests ###############
  #
  echo "$SCRIPTNAME: Importing Certificates =============================="
  CU_ACTION="Import Bob's cert into Alice's db"
  certu -E -t "p,p,p" -d ${P_R_ALICEDIR} -f ${R_PWFILE} \
        -i ${R_BOBDIR}/Bob.cert 2>&1

  CU_ACTION="Import Dave's cert into Alice's DB"
  certu -E -t "p,p,p" -d ${P_R_ALICEDIR} -f ${R_PWFILE} \
        -i ${R_DAVEDIR}/Dave.cert 2>&1

  CU_ACTION="Import Dave's cert into Bob's DB"
  certu -E -t "p,p,p" -d ${P_R_BOBDIR} -f ${R_PWFILE} \
        -i ${R_DAVEDIR}/Dave.cert 2>&1

  CU_ACTION="Import Eve's cert into Alice's DB"
  certu -E -t "p,p,p" -d ${P_R_ALICEDIR} -f ${R_PWFILE} \
        -i ${R_EVEDIR}/Eve.cert 2>&1

  CU_ACTION="Import Eve's cert into Bob's DB"
  certu -E -t "p,p,p" -d ${P_R_BOBDIR} -f ${R_PWFILE} \
        -i ${R_EVEDIR}/Eve.cert 2>&1

  if [ -n "$NSS_ENABLE_ECC" ] ; then
      echo "$SCRIPTNAME: Importing EC Certificates =============================="
      CU_ACTION="Import Bob's EC cert into Alice's db"
      certu -E -t "p,p,p" -d ${P_R_ALICEDIR} -f ${R_PWFILE} \
          -i ${R_BOBDIR}/Bob-ec.cert 2>&1

      CU_ACTION="Import Dave's EC cert into Alice's DB"
      certu -E -t "p,p,p" -d ${P_R_ALICEDIR} -f ${R_PWFILE} \
          -i ${R_DAVEDIR}/Dave-ec.cert 2>&1

      CU_ACTION="Import Dave's EC cert into Bob's DB"
      certu -E -t "p,p,p" -d ${P_R_BOBDIR} -f ${R_PWFILE} \
          -i ${R_DAVEDIR}/Dave-ec.cert 2>&1

## XXXX Do not import Eve's EC cert until we can make sure that
## the email addresses listed in the Subject Alt Name Extension 
## inside Eve's ECC and non-ECC certs are different.
#     CU_ACTION="Import Eve's EC cert into Alice's DB"
#     certu -E -t "p,p,p" -d ${P_R_ALICEDIR} -f ${R_PWFILE} \
#         -i ${R_EVEDIR}/Eve-ec.cert 2>&1

#     CU_ACTION="Import Eve's EC cert into Bob's DB"
#     certu -E -t "p,p,p" -d ${P_R_BOBDIR} -f ${R_PWFILE} \
#         -i ${R_EVEDIR}/Eve-ec.cert 2>&1
  fi

  if [ "$CERTFAILED" != 0 ] ; then
      cert_log "ERROR: SMIME failed $RET"
  else
      cert_log "SUCCESS: SMIME passed"
  fi
}

############################## cert_extended_ssl #######################
# local shell function to create client + server certs for extended SSL test
########################################################################
cert_extended_ssl()
{

  ################# Creating Certs for extended SSL test ####################
  #
  CERTFAILED=0
  echo "$SCRIPTNAME: Creating Certificates, issued by the last ==============="
  echo "     of a chain of CA's which are not in the same database============"

  echo "Server Cert"
  cert_init_cert ${EXT_SERVERDIR} "${HOSTADDR}" 1 ${D_EXT_SERVER}

  CU_ACTION="Initializing ${CERTNAME}'s Cert DB (ext.)"
  certu -N -d "${PROFILEDIR}" -f "${R_PWFILE}" 2>&1

  CU_ACTION="Generate Cert Request for $CERTNAME (ext)"
  CU_SUBJECT="CN=$CERTNAME, E=${CERTNAME}@bogus.com, O=BOGUS NSS, L=Mountain View, ST=California, C=US"
  certu -R -d "${PROFILEDIR}" -f "${R_PWFILE}" -z "${R_NOISE_FILE}" -o req 2>&1

  CU_ACTION="Sign ${CERTNAME}'s Request (ext)"
  cp ${CERTDIR}/req ${SERVER_CADIR}
  certu -C -c "chain-2-serverCA" -m 200 -v 60 -d "${P_SERVER_CADIR}" \
        -i req -o "${CERTNAME}.cert" -f "${R_PWFILE}" 2>&1

  CU_ACTION="Import $CERTNAME's Cert  -t u,u,u (ext)"
  certu -A -n "$CERTNAME" -t "u,u,u" -d "${PROFILEDIR}" -f "${R_PWFILE}" \
        -i "${CERTNAME}.cert" 2>&1

  CU_ACTION="Import Client Root CA -t T,, for $CERTNAME (ext.)"
  certu -A -n "clientCA" -t "T,," -f "${R_PWFILE}" -d "${PROFILEDIR}" \
          -i "${CLIENT_CADIR}/clientCA.ca.cert" 2>&1

  if [ -n "$NSS_ENABLE_ECC" ] ; then
#
#     Repeat the above for EC certs
#
      EC_CURVE="secp256r1"
      CU_ACTION="Generate EC Cert Request for $CERTNAME (ext)"
      CU_SUBJECT="CN=$CERTNAME, E=${CERTNAME}-ec@bogus.com, O=BOGUS NSS, L=Mountain View, ST=California, C=US"
      certu -R -d "${PROFILEDIR}" -k ec -q "${EC_CURVE}" -f "${R_PWFILE}" \
	  -z "${R_NOISE_FILE}" -o req 2>&1

      CU_ACTION="Sign ${CERTNAME}'s EC Request (ext)"
      cp ${CERTDIR}/req ${SERVER_CADIR}
      certu -C -c "chain-2-serverCA-ec" -m 200 -v 60 -d "${P_SERVER_CADIR}" \
          -i req -o "${CERTNAME}-ec.cert" -f "${R_PWFILE}" 2>&1

      CU_ACTION="Import $CERTNAME's EC Cert  -t u,u,u (ext)"
      certu -A -n "${CERTNAME}-ec" -t "u,u,u" -d "${PROFILEDIR}" \
	  -f "${R_PWFILE}" -i "${CERTNAME}-ec.cert" 2>&1

      CU_ACTION="Import Client EC Root CA -t T,, for $CERTNAME (ext.)"
      certu -A -n "clientCA-ec" -t "T,," -f "${R_PWFILE}" -d "${PROFILEDIR}" \
          -i "${CLIENT_CADIR}/clientCA-ec.ca.cert" 2>&1
#
#     done with EC certs
#
  fi

  echo "Importing all the server's own CA chain into the servers DB"
  for CA in `find ${SERVER_CADIR} -name "?*.ca.cert"` ;
  do
      N=`basename $CA | sed -e "s/.ca.cert//"`
      if [ $N = "serverCA" -o $N = "serverCA-ec" ] ; then
          T="-t C,C,C"
      else
          T="-t u,u,u"
      fi
      CU_ACTION="Import $N CA $T for $CERTNAME (ext.) "
      certu -A -n $N  $T -f "${R_PWFILE}" -d "${PROFILEDIR}" \
          -i "${CA}" 2>&1
  done
#============
  echo "Client Cert"
  cert_init_cert ${EXT_CLIENTDIR} ExtendedSSLUser 1 ${D_EXT_CLIENT}

  CU_ACTION="Initializing ${CERTNAME}'s Cert DB (ext.)"
  certu -N -d "${PROFILEDIR}" -f "${R_PWFILE}" 2>&1

  CU_ACTION="Generate Cert Request for $CERTNAME (ext)"
  CU_SUBJECT="CN=$CERTNAME, E=${CERTNAME}@bogus.com, O=BOGUS NSS, L=Mountain View, ST=California, C=US"
  certu -R -d "${PROFILEDIR}" -f "${R_PWFILE}" -z "${R_NOISE_FILE}" \
      -o req 2>&1

  CU_ACTION="Sign ${CERTNAME}'s Request (ext)"
  cp ${CERTDIR}/req ${CLIENT_CADIR}
  certu -C -c "chain-2-clientCA" -m 300 -v 60 -d "${P_CLIENT_CADIR}" \
        -i req -o "${CERTNAME}.cert" -f "${R_PWFILE}" 2>&1

  CU_ACTION="Import $CERTNAME's Cert -t u,u,u (ext)"
  certu -A -n "$CERTNAME" -t "u,u,u" -d "${PROFILEDIR}" -f "${R_PWFILE}" \
        -i "${CERTNAME}.cert" 2>&1
  CU_ACTION="Import Server Root CA -t C,C,C for $CERTNAME (ext.)"
  certu -A -n "serverCA" -t "C,C,C" -f "${R_PWFILE}" -d "${PROFILEDIR}" \
          -i "${SERVER_CADIR}/serverCA.ca.cert" 2>&1

  if [ -n "$NSS_ENABLE_ECC" ] ; then
#
#     Repeat the above for EC certs
#
      CU_ACTION="Generate EC Cert Request for $CERTNAME (ext)"
      CU_SUBJECT="CN=$CERTNAME, E=${CERTNAME}-ec@bogus.com, O=BOGUS NSS, L=Mountain View, ST=California, C=US"
      certu -R -d "${PROFILEDIR}" -k ec -q "${EC_CURVE}" -f "${R_PWFILE}" \
	  -z "${R_NOISE_FILE}" -o req 2>&1

      CU_ACTION="Sign ${CERTNAME}'s EC Request (ext)"
      cp ${CERTDIR}/req ${CLIENT_CADIR}
      certu -C -c "chain-2-clientCA-ec" -m 300 -v 60 -d "${P_CLIENT_CADIR}" \
          -i req -o "${CERTNAME}-ec.cert" -f "${R_PWFILE}" 2>&1

      CU_ACTION="Import $CERTNAME's EC Cert -t u,u,u (ext)"
      certu -A -n "${CERTNAME}-ec" -t "u,u,u" -d "${PROFILEDIR}" \
	  -f "${R_PWFILE}" -i "${CERTNAME}-ec.cert" 2>&1

      CU_ACTION="Import Server EC Root CA -t C,C,C for $CERTNAME (ext.)"
      certu -A -n "serverCA-ec" -t "C,C,C" -f "${R_PWFILE}" \
	  -d "${PROFILEDIR}" -i "${SERVER_CADIR}/serverCA-ec.ca.cert" 2>&1
#
# done with EC certs
#
  fi

  echo "Importing all the client's own CA chain into the servers DB"
  for CA in `find ${CLIENT_CADIR} -name "?*.ca.cert"` ;
  do
      N=`basename $CA | sed -e "s/.ca.cert//"`
      if [ $N = "clientCA" -o $N = "clientCA-ec" ] ; then
          T="-t T,C,C"
      else
          T="-t u,u,u"
      fi
      CU_ACTION="Import $N CA $T for $CERTNAME (ext.)"
      certu -A -n $N  $T -f "${R_PWFILE}" -d "${PROFILEDIR}" \
          -i "${CA}" 2>&1
  done
  if [ "$CERTFAILED" != 0 ] ; then
      cert_log "ERROR: EXT failed $RET"
  else
      cert_log "SUCCESS: EXT passed"
  fi
}

############################## cert_ssl ################################
# local shell function to create client + server certs for SSL test
########################################################################
cert_ssl()
{
  ################# Creating Certs for SSL test ###########################
  #
  CERTFAILED=0
  echo "$SCRIPTNAME: Creating Client CA Issued Certificates ==============="
  cert_create_cert ${CLIENTDIR} "TestUser" 70 ${D_CLIENT}

  echo "$SCRIPTNAME: Creating Server CA Issued Certificate for \\"
  echo "             ${HOSTADDR} ------------------------------------"
  cert_create_cert ${SERVERDIR} "${HOSTADDR}" 100 ${D_SERVER}
  CU_ACTION="Modify trust attributes of Root CA -t TC,TC,TC"
  certu -M -n "TestCA" -t "TC,TC,TC" -d ${PROFILEDIR}
  if [ -n "$NSS_ENABLE_ECC" ] ; then
      CU_ACTION="Modify trust attributes of EC Root CA -t TC,TC,TC"
      certu -M -n "TestCA-ec" -t "TC,TC,TC" -d ${PROFILEDIR}
  fi
#  cert_init_cert ${SERVERDIR} "${HOSTADDR}" 1 ${D_SERVER}
#  echo "************* Copying CA files to ${SERVERDIR}"
#  cp ${CADIR}/*.db .
#  hw_acc
#  CU_ACTION="Creating ${CERTNAME}'s Server Cert"
#  CU_SUBJECT="CN=${CERTNAME}, O=BOGUS Netscape, L=Mountain View, ST=California, C=US"
#  certu -S -n "${CERTNAME}" -c "TestCA" -t "Pu,Pu,Pu" -d ${PROFILEDIR} \
#	 -f "${R_PWFILE}" -z "${R_NOISE_FILE}" -v 60 2>&1

  if [ "$CERTFAILED" != 0 ] ; then
      cert_log "ERROR: SSL failed $RET"
  else
      cert_log "SUCCESS: SSL passed"
  fi
}
############################## cert_stresscerts ################################
# local shell function to create client certs for SSL stresstest
########################################################################
cert_stresscerts()
{

  ############### Creating Certs for SSL stress test #######################
  #
  CERTDIR="$CLIENTDIR"
  cd "${CERTDIR}"

  PROFILEDIR=${CERTDIR}
  if [ -n "${MULTIACCESS_DBM}" ]; then
     PROFILEDIR="multiaccess:${D_CLIENT}"
  fi
  CERTFAILED=0
  echo "$SCRIPTNAME: Creating Client CA Issued Certificates ==============="

  CONTINUE=$GLOB_MAX_CERT
  CERTSERIAL=10

  while [ $CONTINUE -ge $GLOB_MIN_CERT ]
  do
      CERTNAME="TestUser$CONTINUE"
#      cert_add_cert ${CLIENTDIR} "TestUser$CONTINUE" $CERTSERIAL
      cert_add_cert 
      CERTSERIAL=`expr $CERTSERIAL + 1 `
      CONTINUE=`expr $CONTINUE - 1 `
  done
  if [ "$CERTFAILED" != 0 ] ; then
      cert_log "ERROR: StressCert failed $RET"
  else
      cert_log "SUCCESS: StressCert passed"
  fi
}

############################## cert_fips #####################################
# local shell function to create certificates for FIPS tests 
##############################################################################
cert_fips()
{
  CERTFAILED=0
  echo "$SCRIPTNAME: Creating FIPS 140 DSA Certificates =============="
  cert_init_cert "${FIPSDIR}" "FIPS PUB 140 Test Certificate" 1000 "${D_FIPS}"

  CU_ACTION="Initializing ${CERTNAME}'s Cert DB"
  certu -N -d "${PROFILEDIR}" -f "${R_FIPSPWFILE}" 2>&1

  echo "$SCRIPTNAME: Enable FIPS mode on database -----------------------"
  CU_ACTION="Enable FIPS mode on database for ${CERTNAME}"
  echo "modutil -dbdir ${PROFILEDIR} -fips true "
  modutil -dbdir ${PROFILEDIR} -fips true 2>&1 <<MODSCRIPT
y
MODSCRIPT
  RET=$?
  if [ "$RET" -ne 0 ]; then
    html_failed "<TR><TD>${CU_ACTION} ($RET) " 
    cert_log "ERROR: ${CU_ACTION} failed $RET"
  else
    html_passed "<TR><TD>${CU_ACTION}"
  fi

  CU_ACTION="Generate Certificate for ${CERTNAME}"
  CU_SUBJECT="CN=${CERTNAME}, E=fips@bogus.com, O=BOGUS NSS, OU=FIPS PUB 140, L=Mountain View, ST=California, C=US"
  certu -S -n ${FIPSCERTNICK} -x -t "Cu,Cu,Cu" -d "${PROFILEDIR}" -f "${R_FIPSPWFILE}" -k dsa -v 600 -m 500 -z "${R_NOISE_FILE}" 2>&1
  if [ "$RET" -eq 0 ]; then
    cert_log "SUCCESS: FIPS passed"
  fi
}

############################## cert_extensions ###############################
# local shell function to test cert extensions generation.
##############################################################################

checkRes()
{
    res=$1
    filterList=$2
    
    [ $res -ne 0 ] && return 1

    for fl in `echo $filterList | tr \| ' '`; do
        fl="`echo $fl | tr _ ' '`"
        expStat=0
        if [ X`echo "$fl" | cut -c 1` = 'X!' ]; then
            expStat=1
            fl=`echo $fl | tr -d '!'`
        fi
        certutil -d ${CERT_EXTENSIONS_DIR} -L -n $CERTNAME | grep "$fl" >/dev/null 2>&1
        [ $? -ne $expStat ] && return 1
    done
    return 0
}


cert_extensions()
{

    CERTNAME=TestExt
    cert_create_cert ${CERT_EXTENSIONS_DIR} $CERTNAME 90 ${D_CERT_EXTENSTIONS}
    TARG_FILE=${CERT_EXTENSIONS_DIR}/test.args

    CU_SUBJECT="CN=$CERTNAME, E=${CERTNAME}@bogus.com, O=BOGUS NSS, L=Mountain View, ST=California, C=US"

    count=0
    while read arg opt filterList; do
        if [ X"`echo $arg | cut -c 1`" = "X#" ]; then
            continue
        fi
        if [ X"`echo $arg | cut -c 1`" = "X!" ]; then
            testName="$filterList"
            continue
        fi
        if [ X"$arg" = "X=" ]; then
            count=`expr $count + 1`
            echo "#################################################"
            CU_ACTION="Testing $testName"
            certutil -d ${CERT_EXTENSIONS_DIR} -D -n $CERTNAME
            echo certutil -d ${CERT_EXTENSIONS_DIR} -S -n $CERTNAME -t "u,u,u" \
                -s "${CU_SUBJECT}" -x -f ${R_PWFILE} -z "${R_NOISE_FILE}" -$opt < $TARG_FILE
            certutil -d ${CERT_EXTENSIONS_DIR} -S -n $CERTNAME -t "u,u,u" -o /tmp/cert \
                -s "${CU_SUBJECT}" -x -f ${R_PWFILE} -z "${R_NOISE_FILE}" -$opt < $TARG_FILE
            ret=$?  
            echo "certutil options:"
            cat $TARG_FILE
            checkRes $ret "$filterList"
            RET=$?
            if [ "$RET" -ne 0 ]; then
                CERTFAILED=$RET
                html_failed "<TR><TD>${CU_ACTION} ($RET) " 
                cert_log "ERROR: ${CU_ACTION} failed $RET"
            else
                html_passed "<TR><TD>${CU_ACTION}"
            fi
            rm -f $TARG_FILE
        else
            echo $arg >> $TARG_FILE
        fi
    done < ${QADIR}/cert/certext.txt
}


############################## cert_crl_ssl ############################
# local shell function to generate certs and crls for SSL tests
########################################################################
cert_crl_ssl()
{
    
  ################# Creating Certs ###################################
  #
  CERTFAILED=0
  CERTSERIAL=${CRL_GRP_1_BEGIN}

  cd $CADIR
  
  PROFILEDIR=${CLIENTDIR}
  CRL_GRPS_END=`expr ${CRL_GRP_1_BEGIN} + ${TOTAL_CRL_RANGE} - 1`
  echo "$SCRIPTNAME: Creating Client CA Issued Certificates Range $CRL_GRP_1_BEGIN - $CRL_GRPS_END ==="
  CU_ACTION="Creating client test certs"

  while [ $CERTSERIAL -le $CRL_GRPS_END ]
  do
      CERTNAME="TestUser$CERTSERIAL"
      cert_add_cert 
      CERTSERIAL=`expr $CERTSERIAL + 1 `
  done

  #################### CRL Creation ##############################
  CRL_GEN_RES=0
  echo "$SCRIPTNAME: Creating CA CRL ====================================="

  CRL_GRP_END=`expr ${CRL_GRP_1_BEGIN} + ${CRL_GRP_1_RANGE} - 1`
  CRL_FILE_GRP_1=${R_SERVERDIR}/root.crl_${CRL_GRP_1_BEGIN}-${CRL_GRP_END}
  CRL_FILE=${CRL_FILE_GRP_1}
  
  CRLUPDATE=`date +%Y%m%d%H%M%SZ`
  CU_ACTION="Generating CRL for range ${CRL_GRP_1_BEGIN}-${CRL_GRP_END} TestCA authority"
  CRL_GRP_END_=`expr ${CRL_GRP_END} - 1`
  crlu -d $CADIR -G -n "TestCA" -f ${R_PWFILE} \
      -o ${CRL_FILE_GRP_1}_or <<EOF_CRLINI
update=$CRLUPDATE
addcert ${CRL_GRP_1_BEGIN}-${CRL_GRP_END_} $CRL_GRP_DATE
addext reasonCode 0 4
addext issuerAltNames 0 "rfc822Name:caemail@ca.com|dnsName:ca.com|directoryName:CN=NSS Test CA,O=BOGUS NSS,L=Mountain View,ST=California,C=US|URI:http://ca.com|ipAddress:192.168.0.1|registerID=reg CA"
EOF_CRLINI
# This extension should be added to the list, but currently nss has bug
#addext authKeyId 0 "CN=NSS Test CA,O=BOGUS NSS,L=Mountain View,ST=California,C=US" 1
  CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
  chmod 600 ${CRL_FILE_GRP_1}_or

  if [ -n "$NSS_ENABLE_ECC" ] ; then
      CU_ACTION="Generating CRL (ECC) for range ${CRL_GRP_1_BEGIN}-${CRL_GRP_END} TestCA-ec authority"

#     Until Bug 292285 is resolved, do not encode x400 Addresses. After
#     the bug is resolved, reintroduce "x400Address:x400Address" within
#     addext issuerAltNames ...
      crlu -q -d $CADIR -G -n "TestCA-ec" -f ${R_PWFILE} \
	  -o ${CRL_FILE_GRP_1}_or-ec <<EOF_CRLINI
update=$CRLUPDATE
addcert ${CRL_GRP_1_BEGIN}-${CRL_GRP_END_} $CRL_GRP_DATE
addext reasonCode 0 4
addext issuerAltNames 0 "rfc822Name:ca-ecemail@ca.com|dnsName:ca-ec.com|directoryName:CN=NSS Test CA (ECC),O=BOGUS NSS,L=Mountain View,ST=California,C=US|URI:http://ca-ec.com|ipAddress:192.168.0.1|registerID=reg CA (ECC)"
EOF_CRLINI
      CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
      chmod 600 ${CRL_FILE_GRP_1}_or-ec
  fi

  echo test > file
  ############################# Modification ##################################

  echo "$SCRIPTNAME: Modifying CA CRL by adding one more cert ============"
  sleep 2
  CRL_GRP_DATE=`date "+%Y%m%d%H%M%SZ"`
  CU_ACTION="Modify CRL by adding one more cert"
  crlu -d $CADIR -M -n "TestCA" -f ${R_PWFILE} -o ${CRL_FILE_GRP_1}_or1 \
      -i ${CRL_FILE_GRP_1}_or <<EOF_CRLINI
addcert ${CRL_GRP_END} $CRL_GRP_DATE
EOF_CRLINI
  CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
  chmod 600 ${CRL_FILE_GRP_1}_or1
  TEMPFILES="$TEMPFILES ${CRL_FILE_GRP_1}_or"
  if [ -n "$NSS_ENABLE_ECC" ] ; then
      CU_ACTION="Modify CRL (ECC) by adding one more cert"
      crlu -d $CADIR -M -n "TestCA-ec" -f ${R_PWFILE} \
	  -o ${CRL_FILE_GRP_1}_or1-ec -i ${CRL_FILE_GRP_1}_or-ec <<EOF_CRLINI
addcert ${CRL_GRP_END} $CRL_GRP_DATE
EOF_CRLINI
      CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
      chmod 600 ${CRL_FILE_GRP_1}_or1-ec
      TEMPFILES="$TEMPFILES ${CRL_FILE_GRP_1}_or-ec"
  fi

  ########### Removing one cert ${UNREVOKED_CERT_GRP_1} #######################
  echo "$SCRIPTNAME: Modifying CA CRL by removing one cert ==============="
  CU_ACTION="Modify CRL by removing one cert"
  crlu -d $CADIR -M -n "TestCA" -f ${R_PWFILE} -o ${CRL_FILE_GRP_1} \
      -i ${CRL_FILE_GRP_1}_or1 <<EOF_CRLINI
rmcert  ${UNREVOKED_CERT_GRP_1}
EOF_CRLINI
  chmod 600 ${CRL_FILE_GRP_1}
  TEMPFILES="$TEMPFILES ${CRL_FILE_GRP_1}_or1"
  if [ -n "$NSS_ENABLE_ECC" ] ; then
      CU_ACTION="Modify CRL (ECC) by removing one cert"
      crlu -d $CADIR -M -n "TestCA-ec" -f ${R_PWFILE} -o ${CRL_FILE_GRP_1}-ec \
	  -i ${CRL_FILE_GRP_1}_or1-ec <<EOF_CRLINI
rmcert  ${UNREVOKED_CERT_GRP_1}
EOF_CRLINI
      chmod 600 ${CRL_FILE_GRP_1}-ec
      TEMPFILES="$TEMPFILES ${CRL_FILE_GRP_1}_or1-ec"
  fi

  ########### Creating second CRL which includes groups 1 and 2 ##############
  CRL_GRP_END=`expr ${CRL_GRP_2_BEGIN} + ${CRL_GRP_2_RANGE} - 1`
  CRL_FILE_GRP_2=${R_SERVERDIR}/root.crl_${CRL_GRP_2_BEGIN}-${CRL_GRP_END}

  echo "$SCRIPTNAME: Creating CA CRL for groups 1 and 2  ==============="
  CRLUPDATE=`date "+%Y%m%d%H%M%SZ"`
  CRL_GRP_DATE=`date "+%Y%m%d%H%M%SZ"`
  CU_ACTION="Creating CRL for groups 1 and 2"
  crlu -d $CADIR -M -n "TestCA" -f ${R_PWFILE} -o ${CRL_FILE_GRP_2} \
          -i ${CRL_FILE_GRP_1} <<EOF_CRLINI
update=$CRLUPDATE
addcert ${CRL_GRP_2_BEGIN}-${CRL_GRP_END} $CRL_GRP_DATE
addext invalidityDate 0 $CRLUPDATE
rmcert  ${UNREVOKED_CERT_GRP_2}
EOF_CRLINI
  CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
  chmod 600 ${CRL_FILE_GRP_2}
  if [ -n "$NSS_ENABLE_ECC" ] ; then
      CU_ACTION="Creating CRL (ECC) for groups 1 and 2"
      crlu -d $CADIR -M -n "TestCA-ec" -f ${R_PWFILE} -o ${CRL_FILE_GRP_2}-ec \
          -i ${CRL_FILE_GRP_1}-ec <<EOF_CRLINI
update=$CRLUPDATE
addcert ${CRL_GRP_2_BEGIN}-${CRL_GRP_END} $CRL_GRP_DATE
addext invalidityDate 0 $CRLUPDATE
rmcert  ${UNREVOKED_CERT_GRP_2}
EOF_CRLINI
      CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
      chmod 600 ${CRL_FILE_GRP_2}-ec
  fi

  ########### Creating second CRL which includes groups 1, 2 and 3 ##############
  CRL_GRP_END=`expr ${CRL_GRP_3_BEGIN} + ${CRL_GRP_3_RANGE} - 1`
  CRL_FILE_GRP_3=${R_SERVERDIR}/root.crl_${CRL_GRP_3_BEGIN}-${CRL_GRP_END}

  echo "$SCRIPTNAME: Creating CA CRL for groups 1, 2 and 3  ==============="
  sleep 2
  CRLUPDATE=`date "+%Y%m%d%H%M%SZ"`
  CRL_GRP_DATE=`date "+%Y%m%d%H%M%SZ"`
  CU_ACTION="Creating CRL for groups 1, 2 and 3"
  crlu -d $CADIR -M -n "TestCA" -f ${R_PWFILE} -o ${CRL_FILE_GRP_3} \
            -i ${CRL_FILE_GRP_2} <<EOF_CRLINI
update=$CRLUPDATE
addcert ${CRL_GRP_3_BEGIN}-${CRL_GRP_END} $CRL_GRP_DATE
rmcert  ${UNREVOKED_CERT_GRP_3}
addext crlNumber 0 2
EOF_CRLINI
  CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
  chmod 600 ${CRL_FILE_GRP_3}
  if [ -n "$NSS_ENABLE_ECC" ] ; then
      CU_ACTION="Creating CRL (ECC) for groups 1, 2 and 3"
      crlu -d $CADIR -M -n "TestCA-ec" -f ${R_PWFILE} -o ${CRL_FILE_GRP_3}-ec \
          -i ${CRL_FILE_GRP_2}-ec <<EOF_CRLINI
update=$CRLUPDATE
addcert ${CRL_GRP_3_BEGIN}-${CRL_GRP_END} $CRL_GRP_DATE
rmcert  ${UNREVOKED_CERT_GRP_3}
addext crlNumber 0 2
EOF_CRLINI
      CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
      chmod 600 ${CRL_FILE_GRP_3}-ec
  fi

  ############ Importing Server CA Issued CRL for certs of first group #######

  echo "$SCRIPTNAME: Importing Server CA Issued CRL for certs ${CRL_GRP_BEGIN} trough ${CRL_GRP_END}"
  CU_ACTION="Importing CRL for groups 1"
  crlu -I -i ${CRL_FILE} -n "TestCA" -f "${R_PWFILE}" -d "${R_SERVERDIR}"
  CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
  if [ -n "$NSS_ENABLE_ECC" ] ; then
      CU_ACTION="Importing CRL (ECC) for groups 1"
      crlu -I -i ${CRL_FILE}-ec -n "TestCA-ec" -f "${R_PWFILE}" \
	  -d "${R_SERVERDIR}"
      CRL_GEN_RES=`expr $? + $CRL_GEN_RES`
  fi

  if [ "$CERTFAILED" != 0 -o "$CRL_GEN_RES" != 0 ] ; then
      cert_log "ERROR: SSL CRL prep failed $CERTFAILED : $CRL_GEN_RES"
  else
      cert_log "SUCCESS: SSL CRL prep passed"
  fi
}

############################## cert_cleanup ############################
# local shell function to finish this script (no exit since it might be
# sourced)
########################################################################
cert_cleanup()
{
  cert_log "$SCRIPTNAME: finished $SCRIPTNAME"
  html "</TABLE><BR>" 
  cd ${QADIR}
  . common/cleanup.sh
}

################## main #################################################

cert_init 
cert_all_CA
cert_extended_ssl 
cert_ssl 
cert_smime_client        
cert_fips
cert_extensions
cert_crl_ssl
if [ -n "$DO_DIST_ST" -a "$DO_DIST_ST" = "TRUE" ] ; then
    cert_stresscerts 
    #following lines to be used when databases are to be reused
    #cp -r /u/sonmi/tmp/stress/kentuckyderby.13/* $HOSTDIR
    #cp -r $HOSTDIR/../${HOST}.2/* $HOSTDIR

fi
cert_cleanup
