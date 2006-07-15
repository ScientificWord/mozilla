/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

/*
 * Header file to be included by module -
 * warning: defines a whole bunch of static functions
 */

#ifndef nsCharDetConstructors_h__
#define nsCharDetConstructors_h__

// chardet
#include "nsISupports.h"
#include "nsMetaCharsetCID.h"
#include "nsICharsetDetector.h"
#include "nsICharsetAlias.h"
#include "nsMetaCharsetObserver.h"
#include "nsDocumentCharsetInfo.h"
#include "nsXMLEncodingObserver.h"
#include "nsICharsetDetectionAdaptor.h"
#include "nsICharsetDetectionObserver.h"
#include "nsDetectionAdaptor.h"
#include "nsIStringCharsetDetector.h"
#include "nsPSMDetectors.h"
#include "nsCyrillicDetector.h"
#include "nsDocumentCharsetInfoCID.h"
#include "nsXMLEncodingCID.h"
#include "nsCharsetDetectionAdaptorCID.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsMetaCharsetObserver)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDocumentCharsetInfo)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsXMLEncodingObserver)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDetectionAdaptor)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsJAPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsJAStringPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsKOPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsKOStringPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsZHTWPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsZHTWStringPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsZHCNPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsZHCNStringPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsZHPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsZHStringPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsCJKPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsCJKStringPSMDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRUProbDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsUKProbDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRUStringProbDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsUKStringProbDetector)

#ifdef INCLUDE_DBGDETECTOR
NS_GENERIC_FACTORY_CONSTRUCTOR(ns1stBlkDbgDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(ns2ndBlkDbgDetector)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsLastBlkDbgDetector)
#endif /* INCLUDE_DBGDETECTOR */


#endif
