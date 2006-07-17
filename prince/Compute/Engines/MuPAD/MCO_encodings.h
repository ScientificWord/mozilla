#ifndef __MCO_encodings_h__
#define __MCO_encodings_h__

/** @file MCO_encodings.h
 * Encoding options for MCode streams.
 */

/**@defgroup mco_long_enc Encoding of Long Integers in MCode Streams.
 */
//@{

/** Type of flags which define how long integers are encoded
 * in a MCode stream. */
typedef unsigned char MTlongFormat;

/// Encode long integer as binary PARI data.
const MTlongFormat MCpariLong = 1;

/// Encode long integer as ASCII string.
const MTlongFormat MCstringLong = 2;

//@}

/**@defgroup mco_float_enc Encoding of Machine Doubles in MCode Streams.
 */
//@{

/** Type of flags which define how machine doubles are encoded
 * in a MCode stream. */
typedef unsigned char MTfloatFormat;

/// Use IEEE binary encoding.
const MTlongFormat MCieeeFloat = 1;

/// Use ASCII strings.
const MTlongFormat MCstringFloat = 2;

//@}

/**@defgroup mco_enc_opts Additional Encoding Options for MCode Streams.
 */
//@{

/** Type of flags which define additional options how trees are
 * serialized in a MCode stream. */
typedef unsigned char MTcoding;

/// Write trees flat without taking sharing into acount.
const MTcoding MCflatCoding       = 0x00;

/// Share common subtrees, write references for common subtrees.
const MTcoding MCreferenceCoding  = 0x01;

/// Write properties of identifiers.
const MTcoding MCpropertiesCoding = 0x02;

/// Write names of identifiers only.
const MTcoding MCidentCoding      = 0x04;

//@}

#endif // __MCO_encodings_h__
