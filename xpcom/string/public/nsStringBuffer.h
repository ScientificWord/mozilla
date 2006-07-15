/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by IBM Corporation are Copyright (C) 2003
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@meer.net>
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

#ifndef nsStringBuffer_h__
#define nsStringBuffer_h__

/**
 * This structure precedes the string buffers "we" allocate.  It may be the
 * case that nsTAString::mData does not point to one of these special
 * buffers.  The mFlags member variable distinguishes the buffer type.
 *
 * When this header is in use, it enables reference counting, and capacity
 * tracking.  NOTE: A string buffer can be modified only if its reference
 * count is 1.
 */
class nsStringBuffer
  {
    private:

      PRInt32  mRefCount;
      PRUint32 mStorageSize;

    public:
      
      /**
       * Allocates a new string buffer, with given size in bytes and a
       * reference count of one.  When the string buffer is no longer needed,
       * it should be released via Release.
       *
       * It is up to the caller to set the bytes corresponding to the string
       * buffer by calling the Data method to fetch the raw data pointer.  Care
       * must be taken to properly null terminate the character array.  The
       * storage size can be greater than the length of the actual string
       * (i.e., it is not required that the null terminator appear in the last
       * storage unit of the string buffer's data).
       *
       * @return new string buffer or null if out of memory.
       */
      NS_COM static nsStringBuffer* Alloc(size_t storageSize);

      /**
       * Resizes the given string buffer to the specified storage size.  This
       * method must not be called on a readonly string buffer.  Use this API
       * carefully!!
       *
       * This method behaves like the ANSI-C realloc function.  (i.e., If the
       * allocation fails, null will be returned and the given string buffer
       * will remain unmodified.)
       *
       * @see IsReadonly
       */
      NS_COM static nsStringBuffer* Realloc(nsStringBuffer* buf, size_t storageSize);

      /**
       * Increment the reference count on this string buffer.
       */
      NS_COM void NS_FASTCALL AddRef();

      /**
       * Decrement the reference count on this string buffer.  The string
       * buffer will be destroyed when its reference count reaches zero.
       */
      NS_COM void NS_FASTCALL Release();

      /**
       * This method returns the string buffer corresponding to the given data
       * pointer.  The data pointer must have been returned previously by a
       * call to the nsStringBuffer::Data method.
       */
      static nsStringBuffer* FromData(void* data)
        {
          return (nsStringBuffer*) ( ((char*) data) - sizeof(nsStringBuffer) );
        }

      /**
       * This method returns the data pointer for this string buffer.
       */
      void* Data() const
        {
          return (void*) ( ((char*) this) + sizeof(nsStringBuffer) );
        }

      /**
       * This function returns the storage size of a string buffer in bytes.
       * This value is the same value that was originally passed to Alloc (or
       * Realloc).
       */
      PRUint32 StorageSize() const
        {
          return mStorageSize;
        }

      /**
       * If this method returns false, then the caller can be sure that their
       * reference to the string buffer is the only reference to the string
       * buffer, and therefore it has exclusive access to the string buffer and
       * associated data.  However, if this function returns true, then other
       * consumers may rely on the data in this buffer being immutable and
       * other threads may access this buffer simultaneously.
       */
      PRBool IsReadonly() const
        {
          return mRefCount > 1;
        }

      /**
       * The FromString methods return a string buffer for the given string 
       * object or null if the string object does not have a string buffer.
       * The reference count of the string buffer is NOT incremented by these
       * methods.  If the caller wishes to hold onto the returned value, then
       * the returned string buffer must have its reference count incremented
       * via a call to the AddRef method.
       */
      NS_COM static nsStringBuffer* FromString(const nsAString &str);
      NS_COM static nsStringBuffer* FromString(const nsACString &str);

      /**
       * The ToString methods assign this string buffer to a given string
       * object.  If the string object does not support sharable string
       * buffers, then its value will be set to a copy of the given string
       * buffer.  Otherwise, these methods increment the reference count of the
       * given string buffer.  It is important to specify the length (in
       * storage units) of the string contained in the string buffer since the
       * length of the string may be less than its storage size.  The string
       * must have a null terminator at the offset specified by |len|.
       *
       * NOTE: storage size is measured in bytes even for wide strings;
       *       however, string length is always measured in storage units
       *       (2-byte units for wide strings).
       */
      NS_COM void ToString(PRUint32 len, nsAString &str);
      NS_COM void ToString(PRUint32 len, nsACString &str);
  };

#endif /* !defined(nsStringBuffer_h__ */
