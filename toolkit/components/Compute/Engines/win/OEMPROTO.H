/* $Source$*/
/* $Revision$ */

#ifndef __MAPLE_OEMPROTO_H__
#define __MAPLE_OEMPROTO_H__

#undef P
#undef PP
#if defined(__STDC__) || defined(__cplusplus) || defined(USE_PROTO)
#define P(x) x
#define PP(x) x
#else
#define P(x) ()
#define PP(x) ()
#endif

#ifdef _MAPLE_WINDOWS_
/* __stdcall calling convention is used for the Maple OEM library
   and kernel instead of __cdecl, because it is faster.  At Maple,
   we use this calling convention macro; these cannot be changed,
   by the OEM customer, only at Maple */
#ifndef OEM_CALL_CONV
#define OEM_CALL_CONV __stdcall
#endif
#define MAPLE_CALL_CONV __stdcall
#else
#define OEM_CALL_CONV
#define MAPLE_CALL_CONV
#endif

#endif /* MAPLE_OEMPROTO_H */
