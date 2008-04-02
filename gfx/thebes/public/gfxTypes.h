/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Oracle Corporation code.
 *
 * The Initial Developer of the Original Code is Oracle Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@pavlov.net>
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

#ifndef GFX_TYPES_H
#define GFX_TYPES_H

#include "prtypes.h"

/**
 * Currently needs to be 'double' for Cairo compatibility. Could
 * become 'float', perhaps, in some configurations.
 */
typedef double gfxFloat;

#if defined(MOZ_STATIC_BUILD)
# define THEBES_API
#elif defined(IMPL_THEBES)
# define THEBES_API NS_EXPORT
#else
# define THEBES_API NS_IMPORT
#endif

/**
 * Define refcounting for Thebes.  For now use the stuff from nsISupportsImpl
 * even though it forces the functions to be virtual...
 */
#include "nsISupportsImpl.h"
#include "nsAutoPtr.h"

#define THEBES_INLINE_DECL_REFCOUNTING(_class)                                \
public:                                                                       \
    nsrefcnt AddRef(void) {                                                   \
        NS_PRECONDITION(PRInt32(mRefCnt) >= 0, "illegal refcnt");             \
        ++mRefCnt;                                                            \
        NS_LOG_ADDREF(this, mRefCnt, #_class, sizeof(*this));                 \
        return mRefCnt;                                                       \
    }                                                                         \
    nsrefcnt Release(void) {                                                  \
        NS_PRECONDITION(0 != mRefCnt, "dup release");                         \
        --mRefCnt;                                                            \
        NS_LOG_RELEASE(this, mRefCnt, #_class);                               \
        if (mRefCnt == 0) {                                                   \
            mRefCnt = 1; /* stabilize */                                      \
            NS_DELETEXPCOM(this);                                             \
            return 0;                                                         \
        }                                                                     \
        return mRefCnt;                                                       \
    }                                                                         \
protected:                                                                    \
    nsAutoRefCnt mRefCnt;                                                     \
public:

#endif /* GFX_TYPES_H */
