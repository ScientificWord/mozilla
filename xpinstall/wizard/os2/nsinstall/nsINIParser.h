/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Samir Gehani <sgehani@netscape.com>
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

#ifndef _NS_INIPARSER_H_
#define _NS_INIPARSER_H_

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <locale.h>

#ifdef __cplusplus

class nsINIParser
{
public:

    /**
     * nsINIParser
     *
     * Construct a new INI parser for the file specified.
     *
     * @param aFilename     path to INI file
     */
    nsINIParser(char *aFilename);
    ~nsINIParser();

    /**
     * GetString
     * 
     * Gets the value of the specified key in the specified section
     * of the INI file represented by this instance. The value is stored
     * in the supplied buffer. The buffer size is provided as input and
     * the actual bytes used by the value is set in the in/out size param.
     *
     * @param aSection      section name
     * @param aKey          key name
     * @param aValBuf       user supplied buffer
     * @param aIOValBufSize buf size on input; actual buf used on output
     *
     * @return mError       operation success code
     */
    int GetString(      char *aSection, char *aKey, 
                        char *aValBuf, int *aIOValBufSize );

    /**
     * GetStringAlloc
     *
     * Same as GetString() except the buffer is allocated to the exact
     * size of the value. Useful when the buffer is allocated everytime 
     * rather than reusing the same buffer when calling this function 
     * multiple times.
     *
     * @param aSection      section name
     * @param aKey          key name
     * @param aOutBuf       buffer to be allocated
     * @param aOutBufSize   size of newly allocated buffer
     *
     * @return mError       operation success code
     */
    int GetStringAlloc( char *aSection, char *aKey, 
                        char **aOutBuf, int *aOutBufSize );
    
    /**
     * GetError
     *
     * Exposes the last error on this instance. Useful for checking
     * the state of the object after construction since the INI file 
     * is parsed once at object-allocation time.
     *
     * @return mError       last error on ops on this object
     */
    int GetError();

    /**
     * ResolveName
     *
     * Given a "root" name we append the runtime locale of the
     * current system to the <root>.ini. If such a file exists we 
     * return this as the name else simply return <root>.ini.
     *
     * NOTE: Returned string is allocated and caller is responsible
     * ----  for its deallocation.
     *
     * @param   aINIRoot    the "root" of the INI file name
     * @return  resolved    the resolved INI file name
     *                      (NULL if neither exist)
     */
    static char     *ResolveName(char *aINIRoot);

    /**
     * WriteString
     * 
     * Gets the value of the specified key in the specified section
     * of the INI file represented by this instance. The value is stored
     * in the supplied buffer. The buffer size is provided as input and
     * the actual bytes used by the value is set in the in/out size param.
     *
     * @param aSection      section name
     * @param aKey          key name
     * @param aValBuf       user supplied buffer
     * @param aIOValBufSize buf size on input; actual buf used on output
     *
     * @return mError       operation success code
     */
    int WriteString(      char *aSection, char *aKey, char *aValBuf );

/*--------------------------------------------------------------------*
 *   Errors
 *--------------------------------------------------------------------*/
    enum 
    {
        OK                  = 0,
        E_READ              = -701,
        E_MEM               = -702,
        E_PARAM             = -703,
        E_NO_SEC            = -704,
        E_NO_KEY            = -705,
        E_SEC_CORRUPT       = -706,
        E_SMALL_BUF         = -707
    };

private:
    int FindSection(char *aSection, char **aOutSecPtr);
    int FindKey(char *aSecPtr, char *aKey, char **aOutSecPtr);
    int GetValue(char *aSecPtr, char *aKey, char *aVal, int *aIOValSize);

    char    *mFileBuf;
    char    *mFilename;
    int     mFileBufSize;
    int     mError;
    int     mfWrite;
};

#define NL '\n'
#define NLSTRING "\n"
#define MAX_VAL_SIZE 512

#if defined(DUMP)
#undef DUMP
#endif
#if defined(DEBUG_sgehani) || defined(DEBUG_druidd) || defined(DEBUG_root)
#define DUMP(_msg) printf("%s %d: %s \n", __FILE__, __LINE__, _msg);
#else
#define DUMP(_msg) 
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
unsigned long GetPrivateProfileString(const char* szAppName,
                                      const char* szKeyName,
                                      const char* szDefault,
                                      char* szReturnedString,
                                      int nSize,
                                      const char* szFileName);

unsigned long WritePrivateProfileString(const char* szAppName,
                                        const char* szKeyName,
                                        const char* szValue,
                                        const char* szFileName);
#ifdef __cplusplus
}
#endif

#endif /*_NS_INIPARSER_H_ */
