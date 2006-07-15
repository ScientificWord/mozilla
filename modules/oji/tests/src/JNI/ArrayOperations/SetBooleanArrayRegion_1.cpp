/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
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
#include "JNIEnvTests.h"
#include "ArrayOperations.h"

JNI_OJIAPITest(JNIEnv_SetBooleanArrayRegion_1)
{
  GET_JNI_FOR_TEST

  jbooleanArray arr = env->NewBooleanArray(3);
  jsize start = 0;
  jsize leng = 3;
  jboolean buf[3];
  buf[0] = JNI_TRUE;
  buf[1] = JNI_FALSE;
  buf[2] = 0;
  env->SetBooleanArrayRegion(arr, start, leng, buf);

  jboolean isCopy = JNI_TRUE;
  jboolean *val = env->GetBooleanArrayElements(arr, NULL);
  jboolean val0 = val[0];
  jboolean val1 = val[1];
  jboolean val2 = val[2];

  if((val0==JNI_TRUE)&&(val1==JNI_FALSE)&&(val2==0)){
     return TestResult::PASS("SetBooleanArrayRegion(all correct) returns correct value");
  }else{
     return TestResult::FAIL("SetBooleanArrayRegion(all correct) returns incorrect value");
  }

}
