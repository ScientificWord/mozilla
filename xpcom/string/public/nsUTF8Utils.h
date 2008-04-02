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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Peter Annema <jaggernaut@netscape.com> (original author)
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
#ifndef nsUTF8Utils_h_
#define nsUTF8Utils_h_

// This file may be used in two ways: if MOZILLA_INTERNAL_API is defined, this
// file will provide signatures for the Mozilla abstract string types. It will
// use XPCOM assertion/debugging macros, etc.

#include "nscore.h"

#include "nsCharTraits.h"

class UTF8traits
  {
    public:
      static PRBool isASCII(char c) { return (c & 0x80) == 0x00; }
      static PRBool isInSeq(char c) { return (c & 0xC0) == 0x80; }
      static PRBool is2byte(char c) { return (c & 0xE0) == 0xC0; }
      static PRBool is3byte(char c) { return (c & 0xF0) == 0xE0; }
      static PRBool is4byte(char c) { return (c & 0xF8) == 0xF0; }
      static PRBool is5byte(char c) { return (c & 0xFC) == 0xF8; }
      static PRBool is6byte(char c) { return (c & 0xFE) == 0xFC; }
  };

#ifdef __GNUC__
#define NS_ALWAYS_INLINE __attribute__((always_inline))
#else
#define NS_ALWAYS_INLINE
#endif

/**
 * Extract the next UCS-4 character from the buffer and return it.  The
 * pointer passed in is advanced to the start of the next character in the
 * buffer.  If non-null, the parameters err and overlong are filled in to
 * indicate that the character was represented by an overlong sequence, or
 * that an error occurred.
 */

class UTF8CharEnumerator
{
public:
  static PRUint32 NextChar(const char **buffer, const char *end,
                           PRBool *err = nsnull, PRBool* overlong = nsnull)
  {
    NS_ASSERTION(buffer && *buffer, "null buffer!");

    const char *p = *buffer;

    if (p >= end)
      {
        if (err)
          *err = PR_TRUE;

        return 0;
      }

    char c = *p++;

    if ( UTF8traits::isASCII(c) )
      {
        if (err)
          *err = PR_FALSE;
        if (overlong)
          *overlong = PR_FALSE;
        *buffer = p;
        return c;
      }

    PRUint32 ucs4;
    PRUint32 minUcs4;
    PRInt32 state = 0;

    if (!CalcState(c, ucs4, minUcs4, state)) {
        NS_ERROR("Not a UTF-8 string. This code should only be used for converting from known UTF-8 strings.");
        if (err)
          *err = PR_TRUE;
        return 0;
    }

    while ( state-- )
      {
        if (p == end)
          {
            if (err)
              *err = PR_TRUE;

            return 0;
          }

        c = *p++;

        if (!AddByte(c, state, ucs4))
          {
            NS_ERROR("not a UTF8 string");
            if (err)
              *err = PR_TRUE;
            return 0;
          }
      }

    if (err)
      *err = PR_FALSE;
    if (overlong)
      *overlong = ucs4 < minUcs4;
    *buffer = p;
    return ucs4;
  }

#ifdef MOZILLA_INTERNAL_API

  static PRUint32 NextChar(nsACString::const_iterator& iter,
                           const nsACString::const_iterator& end,
                           PRBool *err = nsnull, PRBool *overlong = nsnull)
  {
    if ( iter == end )
      {
        NS_ERROR("No input to work with");
        if (err)
          *err = PR_TRUE;

        return 0;
      }

    char c = *iter++;

    if ( UTF8traits::isASCII(c) )
      {
        if (err)
          *err = PR_FALSE;
        if (overlong)
          *overlong = PR_FALSE;
        return c;
      }

    PRUint32 ucs4;
    PRUint32 minUcs4;
    PRInt32 state = 0;

    if (!CalcState(c, ucs4, minUcs4, state)) {
        NS_ERROR("Not a UTF-8 string. This code should only be used for converting from known UTF-8 strings.");
        if (err)
          *err = PR_TRUE;
        return 0;
    }

    while ( state-- )
      {
        if (iter == end)
          {
            NS_ERROR("Buffer ended in the middle of a multibyte sequence");
            if (err)
              *err = PR_TRUE;

            return 0;
          }

        c = *iter++;

        if (!AddByte(c, state, ucs4))
          {
            NS_ERROR("not a UTF8 string");
            if (err)
              *err = PR_TRUE;
            return 0;
          }
      }

    if (err)
      *err = PR_FALSE;
    if (overlong)
      *overlong = ucs4 < minUcs4;
    return ucs4;
  }
#endif // MOZILLA_INTERNAL_API

private:
  static PRBool CalcState(char c, PRUint32& ucs4, PRUint32& minUcs4,
                          PRInt32& state)
  {
    if ( UTF8traits::is2byte(c) )
      {
        ucs4 = (PRUint32(c) << 6) & 0x000007C0L;
        state = 1;
        minUcs4 = 0x00000080;
      }
    else if ( UTF8traits::is3byte(c) )
      {
        ucs4 = (PRUint32(c) << 12) & 0x0000F000L;
        state = 2;
        minUcs4 = 0x00000800;
      }
    else if ( UTF8traits::is4byte(c) )
      {
        ucs4 = (PRUint32(c) << 18) & 0x001F0000L;
        state = 3;
        minUcs4 = 0x00010000;
      }
    else if ( UTF8traits::is5byte(c) )
      {
        ucs4 = (PRUint32(c) << 24) & 0x03000000L;
        state = 4;
        minUcs4 = 0x00200000;
      }
    else if ( UTF8traits::is6byte(c) )
      {
        ucs4 = (PRUint32(c) << 30) & 0x40000000L;
        state = 5;
        minUcs4 = 0x04000000;
      }
    else
      {
        return PR_FALSE;
      }

    return PR_TRUE;
  }

  static PRBool AddByte(char c, PRInt32 state, PRUint32& ucs4)
  {
    if ( UTF8traits::isInSeq(c) )
      {
        PRInt32 shift = state * 6;
        ucs4 |= (PRUint32(c) & 0x3F) << shift;
        return PR_TRUE;
      }

    return PR_FALSE;
  }
};


/**
 * Extract the next UCS-4 character from the buffer and return it.  The
 * pointer passed in is advanced to the start of the next character in the
 * buffer.  If non-null, the err parameter is filled in if an error occurs.
 */


class UTF16CharEnumerator
{
public:
  static PRUint32 NextChar(const PRUnichar **buffer, const PRUnichar *end,
                           PRBool *err = nsnull)
  {
    NS_ASSERTION(buffer && *buffer, "null buffer!");

    const PRUnichar *p = *buffer;

    if (p >= end)
      {
        NS_ERROR("No input to work with");
        if (err)
          *err = PR_TRUE;

        return 0;
      }

    PRUnichar c = *p++;

    if (!IS_SURROGATE(c)) // U+0000 - U+D7FF,U+E000 - U+FFFF
      {
        if (err)
          *err = PR_FALSE;
        *buffer = p;
        return c;
      }
    else if (NS_IS_HIGH_SURROGATE(c)) // U+D800 - U+DBFF
      {
        if (p == end)
          {
            // Found a high surrogate the end of the buffer. Flag this
            // as an error and return the Unicode replacement
            // character 0xFFFD.

            NS_WARNING("Unexpected end of buffer after high surrogate");

            if (err)
              *err = PR_TRUE;
            *buffer = p;
            return 0xFFFD;
          }

        // D800- DBFF - High Surrogate
        PRUnichar h = c;

        c = *p++;

        if (NS_IS_LOW_SURROGATE(c))
          {
            // DC00- DFFF - Low Surrogate
            // N = (H - D800) *400 + 10000 + (L - DC00)
            PRUint32 ucs4 = SURROGATE_TO_UCS4(h, c);
            if (err)
              *err = PR_FALSE;
            *buffer = p;
            return ucs4;
          }
        else
          {
            // Found a high surrogate followed by something other than
            // a low surrogate. Flag this as an error and return the
            // Unicode replacement character 0xFFFD.

            NS_WARNING("got a High Surrogate but no low surrogate");

            if (err)
              *err = PR_TRUE;
            *buffer = p;
            return 0xFFFD;
          }
      }
    else // U+DC00 - U+DFFF
      {
        // DC00- DFFF - Low Surrogate

        // Found a low surrogate w/o a preceeding high surrogate. Flag
        // this as an error and return the Unicode replacement
        // character 0xFFFD.

        NS_WARNING("got a low Surrogate but no high surrogate");
        if (err)
          *err = PR_TRUE;
        *buffer = p;
        return 0xFFFD;
      }

    if (err)
      *err = PR_TRUE;
    return 0;
  }

#ifdef MOZILLA_INTERNAL_API

  static PRUint32 NextChar(nsAString::const_iterator& iter,
                           const nsAString::const_iterator& end,
                           PRBool *err = nsnull)
  {
    if (iter == end)
      {
        if (err)
          *err = PR_TRUE;

        return 0;
      }

    PRUnichar c = *iter++;

    if (!IS_SURROGATE(c)) // U+0000 - U+D7FF,U+E000 - U+FFFF
      {
        if (err)
          *err = PR_FALSE;
        return c;
      }
    else if (NS_IS_HIGH_SURROGATE(c)) // U+D800 - U+DBFF
      {
        if (iter == end)
          {
            // Found a high surrogate the end of the buffer. Flag this
            // as an error and return the Unicode replacement
            // character 0xFFFD.

            NS_WARNING("Unexpected end of buffer after high surrogate");

            if (err)
              *err = PR_TRUE;
            return 0xFFFD;
          }

        // D800- DBFF - High Surrogate
        PRUnichar h = c;

        c = *iter++;

        if (NS_IS_LOW_SURROGATE(c))
          {
            // DC00- DFFF - Low Surrogate
            // N = (H - D800) *400 + 10000 + ( L - DC00 )
            PRUint32 ucs4 = SURROGATE_TO_UCS4(h, c);
            if (err)
              *err = PR_FALSE;
            return ucs4;
          }
        else
          {
            // Found a high surrogate followed by something other than
            // a low surrogate. Flag this as an error and return the
            // Unicode replacement character 0xFFFD.

            NS_WARNING("got a High Surrogate but no low surrogate");

            if (err)
              *err = PR_TRUE;
            return 0xFFFD;
          }
      }
    else // U+DC00 - U+DFFF
      {
        // DC00- DFFF - Low Surrogate

        // Found a low surrogate w/o a preceeding high surrogate. Flag
        // this as an error and return the Unicode replacement
        // character 0xFFFD.

        NS_WARNING("got a low Surrogate but no high surrogate");

        if (err)
          *err = PR_TRUE;
        return 0xFFFD;
      }

    if (err)
      *err = PR_TRUE;
    return 0;
  }
#endif // MOZILLA_INTERNAL_API
};


/**
 * A character sink (see |copy_string| in nsAlgorithm.h) for converting
 * UTF-8 to UTF-16
 */
class ConvertUTF8toUTF16
  {
    public:
      typedef char      value_type;
      typedef PRUnichar buffer_type;

    ConvertUTF8toUTF16( buffer_type* aBuffer )
        : mStart(aBuffer), mBuffer(aBuffer), mErrorEncountered(PR_FALSE) {}

    size_t Length() const { return mBuffer - mStart; }

    void NS_ALWAYS_INLINE write( const value_type* start, PRUint32 N )
      {
        if ( mErrorEncountered )
          return;

        // algorithm assumes utf8 units won't
        // be spread across fragments
        const value_type* p = start;
        const value_type* end = start + N;
        buffer_type* out = mBuffer;
        for ( ; p != end /* && *p */; )
          {
            PRBool overlong, err;
            PRUint32 ucs4 = UTF8CharEnumerator::NextChar(&p, end, &err,
                                                         &overlong);

            if ( err )
              {
                mErrorEncountered = PR_TRUE;
                mBuffer = out;
                return;
              }

            if ( overlong )
              {
                // Overlong sequence
                *out++ = UCS2_REPLACEMENT_CHAR;
              }
            else if ( ucs4 <= 0xD7FF )
              {
                *out++ = ucs4;
              }
            else if ( /* ucs4 >= 0xD800 && */ ucs4 <= 0xDFFF )
              {
                // Surrogates
                *out++ = UCS2_REPLACEMENT_CHAR;
              }
            else if ( ucs4 == 0xFFFE || ucs4 == 0xFFFF )
              {
                // Prohibited characters
                *out++ = UCS2_REPLACEMENT_CHAR;
              }
            else if ( ucs4 >= PLANE1_BASE )
              {
                if ( ucs4 >= UCS_END )
                  *out++ = UCS2_REPLACEMENT_CHAR;
                else {
                  *out++ = (buffer_type)H_SURROGATE(ucs4);
                  *out++ = (buffer_type)L_SURROGATE(ucs4);
                }
              }
            else
              {
                *out++ = ucs4;
              }
          }
        mBuffer = out;
      }

    void write_terminator()
      {
        *mBuffer = buffer_type(0);
      }

    private:
      buffer_type* const mStart;
      buffer_type* mBuffer;
      PRBool mErrorEncountered;
  };

/**
 * A character sink (see |copy_string| in nsAlgorithm.h) for computing
 * the length of the UTF-16 string equivalent to a UTF-8 string.
 */
class CalculateUTF8Length
  {
    public:
      typedef char value_type;

    CalculateUTF8Length() : mLength(0), mErrorEncountered(PR_FALSE) { }

    size_t Length() const { return mLength; }

    void NS_ALWAYS_INLINE write( const value_type* start, PRUint32 N )
      {
          // ignore any further requests
        if ( mErrorEncountered )
            return;

        // algorithm assumes utf8 units won't
        // be spread across fragments
        const value_type* p = start;
        const value_type* end = start + N;
        for ( ; p < end /* && *p */; ++mLength )
          {
            if ( UTF8traits::isASCII(*p) )
                p += 1;
            else if ( UTF8traits::is2byte(*p) )
                p += 2;
            else if ( UTF8traits::is3byte(*p) )
                p += 3;
            else if ( UTF8traits::is4byte(*p) ) {
                p += 4;
                // Because a UTF-8 sequence of 4 bytes represents a codepoint
                // greater than 0xFFFF, it will become a surrogate pair in the
                // UTF-16 string, so add 1 more to mLength.
                // This doesn't happen with is5byte and is6byte because they
                // are illegal UTF-8 sequences (greater than 0x10FFFF) so get
                // converted to a single replacement character.
                //
                // XXX: if the 4-byte sequence is an illegal non-shortest form,
                //      it also gets converted to a replacement character, so
                //      mLength will be off by one in this case.
                ++mLength;
            }
            else if ( UTF8traits::is5byte(*p) )
                p += 5;
            else if ( UTF8traits::is6byte(*p) )
                p += 6;
            else
              {
                break;
              }
          }
        if ( p != end )
          {
            NS_ERROR("Not a UTF-8 string. This code should only be used for converting from known UTF-8 strings.");
            mErrorEncountered = PR_TRUE;
          }
      }

    private:
      size_t mLength;
      PRBool mErrorEncountered;
  };

/**
 * A character sink (see |copy_string| in nsAlgorithm.h) for
 * converting UTF-16 to UTF-8. Treats invalid UTF-16 data as 0xFFFD
 * (0xEFBFBD in UTF-8).
 */
class ConvertUTF16toUTF8
  {
    public:
      typedef PRUnichar value_type;
      typedef char      buffer_type;

    // The error handling here is more lenient than that in
    // |ConvertUTF8toUTF16|, but it's that way for backwards
    // compatibility.

    ConvertUTF16toUTF8( buffer_type* aBuffer )
        : mStart(aBuffer), mBuffer(aBuffer) {}

    size_t Size() const { return mBuffer - mStart; }

    void NS_ALWAYS_INLINE write( const value_type* start, PRUint32 N )
      {
        buffer_type *out = mBuffer; // gcc isn't smart enough to do this!

        for (const value_type *p = start, *end = start + N; p < end; ++p )
          {
            value_type c = *p;
            if (! (c & 0xFF80)) // U+0000 - U+007F
              {
                *out++ = (char)c;
              }
            else if (! (c & 0xF800)) // U+0100 - U+07FF
              {
                *out++ = 0xC0 | (char)(c >> 6);
                *out++ = 0x80 | (char)(0x003F & c);
              }
            else if (!IS_SURROGATE(c)) // U+0800 - U+D7FF,U+E000 - U+FFFF
              {
                *out++ = 0xE0 | (char)(c >> 12);
                *out++ = 0x80 | (char)(0x003F & (c >> 6));
                *out++ = 0x80 | (char)(0x003F & c );
              }
            else if (NS_IS_HIGH_SURROGATE(c)) // U+D800 - U+DBFF
              {
                // D800- DBFF - High Surrogate
                value_type h = c;

                ++p;
                if (p == end)
                  {
                    // Treat broken characters as the Unicode
                    // replacement character 0xFFFD (0xEFBFBD in
                    // UTF-8)
                    *out++ = 0xEF;
                    *out++ = 0xBF;
                    *out++ = 0xBD;

                    NS_WARNING("String ending in half a surrogate pair!");

                    break;
                  }
                c = *p;

                if (NS_IS_LOW_SURROGATE(c))
                  {
                    // DC00- DFFF - Low Surrogate
                    // N = (H - D800) *400 + 10000 + ( L - DC00 )
                    PRUint32 ucs4 = SURROGATE_TO_UCS4(h, c);

                    // 0001 0000-001F FFFF
                    *out++ = 0xF0 | (char)(ucs4 >> 18);
                    *out++ = 0x80 | (char)(0x003F & (ucs4 >> 12));
                    *out++ = 0x80 | (char)(0x003F & (ucs4 >> 6));
                    *out++ = 0x80 | (char)(0x003F & ucs4);
                  }
                else
                  {
                    // Treat broken characters as the Unicode
                    // replacement character 0xFFFD (0xEFBFBD in
                    // UTF-8)
                    *out++ = 0xEF;
                    *out++ = 0xBF;
                    *out++ = 0xBD;

                    NS_WARNING("got a High Surrogate but no low surrogate");
                  }
              }
            else // U+DC00 - U+DFFF
              {
                // Treat broken characters as the Unicode replacement
                // character 0xFFFD (0xEFBFBD in UTF-8)
                *out++ = 0xEF;
                *out++ = 0xBF;
                *out++ = 0xBD;

                // DC00- DFFF - Low Surrogate
                NS_WARNING("got a low Surrogate but no high surrogate");
              }
          }

        mBuffer = out;
      }

    void write_terminator()
      {
        *mBuffer = buffer_type(0);
      }

    private:
      buffer_type* const mStart;
      buffer_type* mBuffer;
  };

/**
 * A character sink (see |copy_string| in nsAlgorithm.h) for computing
 * the number of bytes a UTF-16 would occupy in UTF-8. Treats invalid
 * UTF-16 data as 0xFFFD (0xEFBFBD in UTF-8).
 */
class CalculateUTF8Size
  {
    public:
      typedef PRUnichar value_type;

    CalculateUTF8Size()
      : mSize(0) { }

    size_t Size() const { return mSize; }

    void NS_ALWAYS_INLINE write( const value_type* start, PRUint32 N )
      {
        // Assume UCS2 surrogate pairs won't be spread across fragments.
        for (const value_type *p = start, *end = start + N; p < end; ++p )
          {
            value_type c = *p;
            if (! (c & 0xFF80)) // U+0000 - U+007F
              mSize += 1;
            else if (! (c & 0xF800)) // U+0100 - U+07FF
              mSize += 2;
            else if (0xD800 != (0xF800 & c)) // U+0800 - U+D7FF,U+E000 - U+FFFF
              mSize += 3;
            else if (0xD800 == (0xFC00 & c)) // U+D800 - U+DBFF
              {
                ++p;
                if (p == end)
                  {
                    // Treat broken characters as the Unicode
                    // replacement character 0xFFFD (0xEFBFBD in
                    // UTF-8)
                    mSize += 3;

                    NS_WARNING("String ending in half a surrogate pair!");

                    break;
                  }
                c = *p;

                if (0xDC00 == (0xFC00 & c))
                  mSize += 4;
                else
                  {
                    // Treat broken characters as the Unicode
                    // replacement character 0xFFFD (0xEFBFBD in
                    // UTF-8)
                    mSize += 3;

                    NS_WARNING("got a high Surrogate but no low surrogate");
                  }
              }
            else // U+DC00 - U+DFFF
              {
                // Treat broken characters as the Unicode replacement
                // character 0xFFFD (0xEFBFBD in UTF-8)
                mSize += 3;

                NS_WARNING("got a low Surrogate but no high surrogate");
              }
          }
      }

    private:
      size_t mSize;
  };

#ifdef MOZILLA_INTERNAL_API
/**
 * A character sink that performs a |reinterpret_cast| style conversion
 * between character types.
 */
template <class FromCharT, class ToCharT>
class LossyConvertEncoding
  {
    public:
      typedef FromCharT value_type;
 
      typedef FromCharT input_type;
      typedef ToCharT   output_type;

      typedef typename nsCharTraits<FromCharT>::unsigned_char_type unsigned_input_type;

    public:
      LossyConvertEncoding( output_type* aDestination ) : mDestination(aDestination) { }

      void
      write( const input_type* aSource, PRUint32 aSourceLength )
        {
          const input_type* done_writing = aSource + aSourceLength;
          while ( aSource < done_writing )
            *mDestination++ = (output_type)(unsigned_input_type)(*aSource++);  // use old-style cast to mimic old |ns[C]String| behavior
        }

      void
      write_terminator()
        {
          *mDestination = output_type(0);
        }

    private:
      output_type* mDestination;
  };
#endif // MOZILLA_INTERNAL_API

#endif /* !defined(nsUTF8Utils_h_) */
