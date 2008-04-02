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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
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

/*
 * ssltap.c
 *
 * Version 1.0 : Frederick Roeber : 11 June 1997
 * Version 2.0 : Steve Parkinson  : 13 November 1997
 * Version 3.0 : Nelson Bolyard   : 22 July 1998
 * Version 3.1 : Nelson Bolyard   : 24 May  1999
 *
 * changes in version 2.0:
 *  Uses NSPR20
 *  Shows structure of SSL negotiation, if enabled.
 *
 * This "proxies" a socket connection (like a socks tunnel), but displays the
 * data is it flies by.
 *
 * In the code, the 'client' socket is the one on the client side of the
 * proxy, and the server socket is on the server side.
 *
 */

#include "nspr.h"
#include "plstr.h"
#include "secutil.h"
#include <memory.h>	/* for memcpy, etc. */
#include <string.h>
#include <time.h>

#include "plgetopt.h"
#include "nss.h"
#include "cert.h"
#include "sslproto.h"

#define VERSIONSTRING "$Revision$ ($Date$) $Author$"


struct _DataBufferList;
struct _DataBuffer;

typedef struct _DataBufferList {
  struct _DataBuffer *first,*last;
  int size;
  int isEncrypted;
} DataBufferList;

typedef struct _DataBuffer {
  unsigned char *buffer;
  int length;
  int offset;  /* offset of first good byte */
  struct _DataBuffer *next;
} DataBuffer;


DataBufferList
  clientstream = {NULL, NULL, 0, 0},
  serverstream = {NULL, NULL, 0, 0};

struct sslhandshake {
  PRUint8 type;
  PRUint32 length;
};

typedef struct _SSLRecord {
  PRUint8 type;
  PRUint8 ver_maj,ver_min;

  PRUint8 length[2];
} SSLRecord;

typedef struct _ClientHelloV2 {
  PRUint8 length[2];
  PRUint8 type;
  PRUint8 version[2];
  PRUint8 cslength[2];
  PRUint8 sidlength[2];
  PRUint8 rndlength[2];
  PRUint8 csuites[1];
} ClientHelloV2;

typedef struct _ServerHelloV2 {
  PRUint8 length[2];
  PRUint8 type;
  PRUint8 sidhit;
  PRUint8 certtype;
  PRUint8 version[2];
  PRUint8 certlength[2];
  PRUint8 cslength[2];
  PRUint8 cidlength[2];
} ServerHelloV2;

typedef struct _ClientMasterKeyV2 {
  PRUint8 length[2];
  PRUint8 type;

  PRUint8 cipherkind[3];
  PRUint8 clearkey[2];
  PRUint8 secretkey[2];

} ClientMasterKeyV2;



#define TAPBUFSIZ 16384

#define DEFPORT 1924
#include <ctype.h>

int hexparse=0;
int sslparse=0;
int sslhexparse=0;
int looparound=0;
int fancy=0;
int isV2Session=0;
int currentcipher=0;
int hMACsize=0;

#define PR_FPUTS(x) PR_fprintf(PR_STDOUT, x )

#define GET_SHORT(x) ((PRUint16)(((PRUint16)((PRUint8*)x)[0]) << 8) + ((PRUint16)((PRUint8*)x)[1]))
#define GET_24(x) ((PRUint32)   (  \
				 (((PRUint32)((PRUint8*)x)[0]) << 16) \
				 +                          \
				 (((PRUint32)((PRUint8*)x)[1]) << 8)  \
				 +                          \
				 (((PRUint32)((PRUint8*)x)[2]) << 0)  \
				 ) )

void print_hex(int amt, unsigned char *buf);
void read_stream_bytes(unsigned char *d, DataBufferList *db, int length);

void myhalt(int dblsize,int collectedsize) {

  while(1) ;

}

const char *get_error_text(int error) {
  switch (error) {
  case PR_IO_TIMEOUT_ERROR:
    return "Timeout";
    break;
  case PR_CONNECT_REFUSED_ERROR:
    return "Connection refused";
    break;
  case PR_NETWORK_UNREACHABLE_ERROR:
    return "Network unreachable";
    break;
  case PR_BAD_ADDRESS_ERROR:
    return "Bad address";
    break;
  case PR_CONNECT_RESET_ERROR:
    return "Connection reset";
    break;
  case PR_PIPE_ERROR:
    return "Pipe error";
    break;
  }

  return "";
}





void check_integrity(DataBufferList *dbl) {
  DataBuffer *db;
  int i;

  db = dbl->first;
  i =0;
  while (db) {
    i+= db->length - db->offset;
    db = db->next;
  }
  if (i != dbl->size) {
    myhalt(dbl->size,i);
  }
}

/* Free's the DataBuffer at the head of the list and returns the pointer
 * to the new head of the list.
 */
DataBuffer * 
free_head(DataBufferList *dbl)
{
  DataBuffer *db       = dbl->first;
  PR_ASSERT(db->offset >= db->length);
  if (db->offset >= db->length) {
    dbl->first = db->next;
    if (dbl->first == NULL) {
      dbl->last = NULL;
    }
    PR_Free(db->buffer);
    PR_Free(db);
    db = dbl->first;
  }
  return db;
}

void 
read_stream_bytes(unsigned char *d, DataBufferList *dbl, int length) {
  int         copied 	= 0;
  DataBuffer *db	= dbl->first;

  if (!db) {
    PR_fprintf(PR_STDERR,"assert failed - dbl->first is null\n");
    exit(8);
  }
  while (length) {
    int         toCopy;
    /* find the number of bytes to copy from the head buffer */
    /* if there's too many in this buffer, then only copy 'length' */
    toCopy = PR_MIN(db->length - db->offset, length);

    memcpy(d + copied, db->buffer + db->offset, toCopy);
    copied     += toCopy;
    db->offset += toCopy;
    length     -= toCopy;
    dbl->size  -= toCopy;

    /* if we emptied the head buffer */
    if (db->offset >= db->length) {
      db = free_head(dbl);
    }
  }

  check_integrity(dbl);

}

void
flush_stream(DataBufferList *dbl)
{
    DataBuffer *db = dbl->first;
    check_integrity(dbl);
    while (db) {
	db->offset = db->length;
    	db = free_head(dbl);
    }
    dbl->size = 0;
    check_integrity(dbl);
}


const char * V2CipherString(int cs_int) {
  char *cs_str;
  cs_str = NULL;
  switch (cs_int) {

  case 0x010080:    cs_str = "SSL2/RSA/RC4-128/MD5";       break;
  case 0x020080:    cs_str = "SSL2/RSA/RC4-40/MD5";    break;
  case 0x030080:    cs_str = "SSL2/RSA/RC2CBC128/MD5";     break;
  case 0x040080:    cs_str = "SSL2/RSA/RC2CBC40/MD5";  break;
  case 0x050080:    cs_str = "SSL2/RSA/IDEA128CBC/MD5";    break;
  case 0x060040:    cs_str = "SSL2/RSA/DES56-CBC/MD5";      break;
  case 0x0700C0:    cs_str = "SSL2/RSA/3DES192EDE-CBC/MD5"; break;

  case 0x000001:    cs_str = "SSL3/RSA/NULL/MD5";             break;
  case 0x000002:    cs_str = "SSL3/RSA/NULL/SHA";             break;
  case 0x000003:    cs_str = "SSL3/RSA/RC4-40/MD5";       break;
  case 0x000004:    cs_str = "SSL3/RSA/RC4-128/MD5";          break;
  case 0x000005:    cs_str = "SSL3/RSA/RC4-128/SHA";          break;
  case 0x000006:    cs_str = "SSL3/RSA/RC2CBC40/MD5";     break;
  case 0x000007:    cs_str = "SSL3/RSA/IDEA128CBC/SHA";       break;
  case 0x000008:    cs_str = "SSL3/RSA/DES40-CBC/SHA";         break;
  case 0x000009:    cs_str = "SSL3/RSA/DES56-CBC/SHA";         break;
  case 0x00000A:    cs_str = "SSL3/RSA/3DES192EDE-CBC/SHA";    break;

  case 0x00000B:    cs_str = "SSL3/DH-DSS/DES40-CBC/SHA";      break;
  case 0x00000C:    cs_str = "SSL3/DH-DSS/DES56-CBC/SHA";      break;
  case 0x00000D:    cs_str = "SSL3/DH-DSS/DES192EDE3CBC/SHA"; break;
  case 0x00000E:    cs_str = "SSL3/DH-RSA/DES40-CBC/SHA";      break;
  case 0x00000F:    cs_str = "SSL3/DH-RSA/DES56-CBC/SHA";      break;
  case 0x000010:    cs_str = "SSL3/DH-RSA/3DES192EDE-CBC/SHA"; break;

  case 0x000011:    cs_str = "SSL3/DHE-DSS/DES40-CBC/SHA";      break;
  case 0x000012:    cs_str = "SSL3/DHE-DSS/DES56-CBC/SHA";      break;
  case 0x000013:    cs_str = "SSL3/DHE-DSS/DES192EDE3CBC/SHA"; break;
  case 0x000014:    cs_str = "SSL3/DHE-RSA/DES40-CBC/SHA";      break;
  case 0x000015:    cs_str = "SSL3/DHE-RSA/DES56-CBC/SHA";      break;
  case 0x000016:    cs_str = "SSL3/DHE-RSA/3DES192EDE-CBC/SHA"; break;

  case 0x000017:    cs_str = "SSL3/DH-anon/RC4-40/MD5";     break;
  case 0x000018:    cs_str = "SSL3/DH-anon/RC4-128/MD5";        break;
  case 0x000019:    cs_str = "SSL3/DH-anon/DES40-CBC/SHA";       break;
  case 0x00001A:    cs_str = "SSL3/DH-anon/DES56-CBC/SHA";       break;
  case 0x00001B:    cs_str = "SSL3/DH-anon/3DES192EDE-CBC/SHA"; break;

  case 0x00001C:    cs_str = "SSL3/FORTEZZA-DMS/NULL/SHA";      break;
  case 0x00001D:    cs_str = "SSL3/FORTEZZA-DMS/FORTEZZA-CBC/SHA";  break;
  case 0x00001E:    cs_str = "SSL3/FORTEZZA-DMS/RC4-128/SHA";  break;

  case 0x00002F:    cs_str = "TLS/RSA/AES128-CBC/SHA";  	break;
  case 0x000030:    cs_str = "TLS/DH-DSS/AES128-CBC/SHA";	break;
  case 0x000031:    cs_str = "TLS/DH-RSA/AES128-CBC/SHA";	break;
  case 0x000032:    cs_str = "TLS/DHE-DSS/AES128-CBC/SHA";	break;
  case 0x000033:    cs_str = "TLS/DHE-RSA/AES128-CBC/SHA";	break;
  case 0x000034:    cs_str = "TLS/DH-ANON/AES128-CBC/SHA";	break;

  case 0x000035:    cs_str = "TLS/RSA/AES256-CBC/SHA";  	break;
  case 0x000036:    cs_str = "TLS/DH-DSS/AES256-CBC/SHA";	break;
  case 0x000037:    cs_str = "TLS/DH-RSA/AES256-CBC/SHA";	break;
  case 0x000038:    cs_str = "TLS/DHE-DSS/AES256-CBC/SHA";	break;
  case 0x000039:    cs_str = "TLS/DHE-RSA/AES256-CBC/SHA";	break;
  case 0x00003A:    cs_str = "TLS/DH-ANON/AES256-CBC/SHA";	break;

  case 0x000041:    cs_str = "TLS/RSA/CAMELLIA128-CBC/SHA";	break;
  case 0x000042:    cs_str = "TLS/DH-DSS/CAMELLIA128-CBC/SHA";	break;
  case 0x000043:    cs_str = "TLS/DH-RSA/CAMELLIA128-CBC/SHA";	break;
  case 0x000044:    cs_str = "TLS/DHE-DSS/CAMELLIA128-CBC/SHA";	break;
  case 0x000045:    cs_str = "TLS/DHE-RSA/CAMELLIA128-CBC/SHA";	break;
  case 0x000046:    cs_str = "TLS/DH-ANON/CAMELLIA128-CBC/SHA";	break;

  case 0x000060:    cs_str = "TLS/RSA-EXPORT1024/RC4-56/MD5";	break;
  case 0x000061:    cs_str = "TLS/RSA-EXPORT1024/RC2CBC56/MD5";	break;
  case 0x000062:    cs_str = "TLS/RSA-EXPORT1024/DES56-CBC/SHA";   break;
  case 0x000064:    cs_str = "TLS/RSA-EXPORT1024/RC4-56/SHA";	   break;
  case 0x000063:    cs_str = "TLS/DHE-DSS_EXPORT1024/DES56-CBC/SHA"; break;
  case 0x000065:    cs_str = "TLS/DHE-DSS_EXPORT1024/RC4-56/SHA";  break;
  case 0x000066:    cs_str = "TLS/DHE-DSS/RC4-128/SHA";		   break;

  case 0x000072:    cs_str = "TLS/DHE-DSS/3DESEDE-CBC/RMD160"; break;
  case 0x000073:    cs_str = "TLS/DHE-DSS/AES128-CBC/RMD160";  break;
  case 0x000074:    cs_str = "TLS/DHE-DSS/AES256-CBC/RMD160";  break;

  case 0x000079:    cs_str = "TLS/DHE-RSA/AES256-CBC/RMD160";  break;

  case 0x00007C:    cs_str = "TLS/RSA/3DESEDE-CBC/RMD160"; break;
  case 0x00007D:    cs_str = "TLS/RSA/AES128-CBC/RMD160"; break;
  case 0x00007E:    cs_str = "TLS/RSA/AES256-CBC/RMD160"; break;

  case 0x000080:    cs_str = "TLS/GOST341094/GOST28147-OFB/GOST28147"; break;
  case 0x000081:    cs_str = "TLS/GOST34102001/GOST28147-OFB/GOST28147"; break;
  case 0x000082:    cs_str = "TLS/GOST341094/NULL/GOSTR3411"; break;
  case 0x000083:    cs_str = "TLS/GOST34102001/NULL/GOSTR3411"; break;

  case 0x000084:    cs_str = "TLS/RSA/CAMELLIA256-CBC/SHA";	break;
  case 0x000085:    cs_str = "TLS/DH-DSS/CAMELLIA256-CBC/SHA";	break;
  case 0x000086:    cs_str = "TLS/DH-RSA/CAMELLIA256-CBC/SHA";	break;
  case 0x000087:    cs_str = "TLS/DHE-DSS/CAMELLIA256-CBC/SHA";	break;
  case 0x000088:    cs_str = "TLS/DHE-RSA/CAMELLIA256-CBC/SHA";	break;
  case 0x000089:    cs_str = "TLS/DH-ANON/CAMELLIA256-CBC/SHA";	break;
  case 0x00008A:    cs_str = "TLS/PSK/RC4-128/SHA";		break;
  case 0x00008B:    cs_str = "TLS/PSK/3DES-EDE-CBC/SHA";	break;
  case 0x00008C:    cs_str = "TLS/PSK/AES128-CBC/SHA";		break;      
  case 0x00008D:    cs_str = "TLS/PSK/AES256-CBC/SHA";		break;      
  case 0x00008E:    cs_str = "TLS/DHE-PSK/RC4-128/SHA";		break;      
  case 0x00008F:    cs_str = "TLS/DHE-PSK/3DES-EDE-CBC/SHA";	break; 
  case 0x000090:    cs_str = "TLS/DHE-PSK/AES128-CBC/SHA";	break;  
  case 0x000091:    cs_str = "TLS/DHE-PSK/AES256-CBC/SHA";	break;  
  case 0x000092:    cs_str = "TLS/RSA-PSK/RC4-128/SHA";		break;      
  case 0x000093:    cs_str = "TLS/RSA-PSK/3DES-EDE-CBC/SHA";	break; 
  case 0x000094:    cs_str = "TLS/RSA-PSK/AES128-CBC/SHA";	break;  
  case 0x000095:    cs_str = "TLS/RSA-PSK/AES256-CBC/SHA";	break;  
  case 0x000096:    cs_str = "TLS/RSA/SEED-CBC/SHA";		break;         
  case 0x000097:    cs_str = "TLS/DH-DSS/SEED-CBC/SHA";		break;      
  case 0x000098:    cs_str = "TLS/DH-RSA/SEED-CBC/SHA";		break;      
  case 0x000099:    cs_str = "TLS/DHE-DSS/SEED-CBC/SHA";	break;     
  case 0x00009A:    cs_str = "TLS/DHE-RSA/SEED-CBC/SHA";	break;     
  case 0x00009B:    cs_str = "TLS/DH-ANON/SEED-CBC/SHA";	break;     

  case 0x00C001:    cs_str = "TLS/ECDH-ECDSA/NULL/SHA";         break;
  case 0x00C002:    cs_str = "TLS/ECDH-ECDSA/RC4-128/SHA";      break;
  case 0x00C003:    cs_str = "TLS/ECDH-ECDSA/3DES-EDE-CBC/SHA"; break;
  case 0x00C004:    cs_str = "TLS/ECDH-ECDSA/AES128-CBC/SHA";   break;
  case 0x00C005:    cs_str = "TLS/ECDH-ECDSA/AES256-CBC/SHA";   break;
  case 0x00C006:    cs_str = "TLS/ECDHE-ECDSA/NULL/SHA";        break;
  case 0x00C007:    cs_str = "TLS/ECDHE-ECDSA/RC4-128/SHA";     break;
  case 0x00C008:    cs_str = "TLS/ECDHE-ECDSA/3DES-EDE-CBC/SHA";break;
  case 0x00C009:    cs_str = "TLS/ECDHE-ECDSA/AES128-CBC/SHA";  break;
  case 0x00C00A:    cs_str = "TLS/ECDHE-ECDSA/AES256-CBC/SHA";  break;
  case 0x00C00B:    cs_str = "TLS/ECDH-RSA/NULL/SHA";           break;
  case 0x00C00C:    cs_str = "TLS/ECDH-RSA/RC4-128/SHA";        break;
  case 0x00C00D:    cs_str = "TLS/ECDH-RSA/3DES-EDE-CBC/SHA";   break;
  case 0x00C00E:    cs_str = "TLS/ECDH-RSA/AES128-CBC/SHA";     break;
  case 0x00C00F:    cs_str = "TLS/ECDH-RSA/AES256-CBC/SHA";     break;
  case 0x00C010:    cs_str = "TLS/ECDHE-RSA/NULL/SHA";          break;
  case 0x00C011:    cs_str = "TLS/ECDHE-RSA/RC4-128/SHA";       break;
  case 0x00C012:    cs_str = "TLS/ECDHE-RSA/3DES-EDE-CBC/SHA";  break;
  case 0x00C013:    cs_str = "TLS/ECDHE-RSA/AES128-CBC/SHA";    break;
  case 0x00C014:    cs_str = "TLS/ECDHE-RSA/AES256-CBC/SHA";    break;
  case 0x00C015:    cs_str = "TLS/ECDH-anon/NULL/SHA";          break;
  case 0x00C016:    cs_str = "TLS/ECDH-anon/RC4-128/SHA";       break;
  case 0x00C017:    cs_str = "TLS/ECDH-anon/3DES-EDE-CBC/SHA";  break;
  case 0x00C018:    cs_str = "TLS/ECDH-anon/AES128-CBC/SHA";    break;
  case 0x00C019:    cs_str = "TLS/ECDH-anon/AES256-CBC/SHA";    break;

  case 0x00feff:    cs_str = "SSL3/RSA-FIPS/3DESEDE-CBC/SHA";	break;
  case 0x00fefe:    cs_str = "SSL3/RSA-FIPS/DES-CBC/SHA";	break;
  case 0x00ffe1:    cs_str = "SSL3/RSA-FIPS/DES56-CBC/SHA";     break;
  case 0x00ffe0:    cs_str = "SSL3/RSA-FIPS/3DES192EDE-CBC/SHA";break;

  /* the string literal is broken up to avoid trigraphs */
  default:          cs_str = "????" "/????????" "/?????????" "/???"; break;
  }

  return cs_str;
}

const char * helloExtensionNameString(int ex_num) {
  const char *ex_name = NULL;
  static char buf[10];

  switch (ex_num) {
  case  0: ex_name = "server_name";                    break;
  case  1: ex_name = "max_fragment_length";            break;
  case  2: ex_name = "client_certificate_url";         break;
  case  3: ex_name = "trusted_ca_keys";                break;
  case  4: ex_name = "truncated_hmac";                 break;
  case  5: ex_name = "status_request";                 break;
  case 10: ex_name = "elliptic_curves";                break;
  case 11: ex_name = "ec_point_formats";               break;
  default: sprintf(buf, "%d", ex_num);  ex_name = (const char *)buf; break;
  }

  return ex_name;
}

static int isNULLmac(int cs_int)
{
    return (cs_int == SSL_NULL_WITH_NULL_NULL); 
}

static int isNULLcipher(int cs_int)
{
 return ((cs_int == SSL_RSA_WITH_NULL_MD5) ||
         (cs_int == SSL_RSA_WITH_NULL_SHA) ||
         (cs_int == SSL_FORTEZZA_DMS_WITH_NULL_SHA) ||
         (cs_int == TLS_ECDH_ECDSA_WITH_NULL_SHA) ||
         (cs_int == TLS_ECDHE_ECDSA_WITH_NULL_SHA) ||
         (cs_int == TLS_ECDH_RSA_WITH_NULL_SHA) ||
         (cs_int == TLS_ECDHE_RSA_WITH_NULL_SHA));
} 

void partial_packet(int thispacket, int size, int needed)
{
  PR_fprintf(PR_STDOUT,"(%u bytes", thispacket);
  if (thispacket < needed) {
    PR_fprintf(PR_STDOUT,", making %u", size);
  }
  PR_fprintf(PR_STDOUT," of %u", needed);
  if (size > needed) {
    PR_fprintf(PR_STDOUT,", with %u left over", size - needed);
  }
  PR_fprintf(PR_STDOUT,")\n");
}

char * get_time_string(void)
{
  char      *cp;
  char      *eol;
  time_t     tt;

  time(&tt);
  cp = ctime(&tt);
  eol = strchr(cp, '\n');
  if (eol) 
    *eol = 0;
  return cp;
}

void print_sslv2(DataBufferList *s, unsigned char *tbuf, unsigned int alloclen)
{
  ClientHelloV2 *chv2;
  ServerHelloV2 *shv2;
  unsigned char *pos;
  unsigned int   p;
  unsigned int   q;
  PRUint32       len;

  chv2 = (ClientHelloV2 *)tbuf;
  shv2 = (ServerHelloV2 *)tbuf;
  if (s->isEncrypted) {
    PR_fprintf(PR_STDOUT," [ssl2]  Encrypted {...}\n");
    return;
  }
  PR_fprintf(PR_STDOUT," [%s]", get_time_string() );
  switch(chv2->type) {
  case 1:
    PR_fprintf(PR_STDOUT," [ssl2]  ClientHelloV2 {\n");
    PR_fprintf(PR_STDOUT,"           version = {0x%02x, 0x%02x}\n",
	       (PRUint32)chv2->version[0],(PRUint32)chv2->version[1]);
    PR_fprintf(PR_STDOUT,"           cipher-specs-length = %d (0x%02x)\n",
	       (PRUint32)(GET_SHORT((chv2->cslength))),
	       (PRUint32)(GET_SHORT((chv2->cslength))));
    PR_fprintf(PR_STDOUT,"           sid-length = %d (0x%02x)\n",
	       (PRUint32)(GET_SHORT((chv2->sidlength))),
	       (PRUint32)(GET_SHORT((chv2->sidlength))));
    PR_fprintf(PR_STDOUT,"           challenge-length = %d (0x%02x)\n",
	       (PRUint32)(GET_SHORT((chv2->rndlength))),
	       (PRUint32)(GET_SHORT((chv2->rndlength))));
    PR_fprintf(PR_STDOUT,"           cipher-suites = { \n");
    for (p=0;p<GET_SHORT((chv2->cslength));p+=3) {
      const char *cs_str=NULL;
      PRUint32 cs_int=0;
      cs_int = GET_24((&chv2->csuites[p]));
      cs_str = V2CipherString(cs_int);

      PR_fprintf(PR_STDOUT,"                (0x%06x) %s\n",
		  cs_int, cs_str);
    }
    q = p;
    PR_fprintf(PR_STDOUT,"                }\n");
    if (chv2->sidlength) {
      PR_fprintf(PR_STDOUT,"           session-id = { ");
      for (p=0;p<GET_SHORT((chv2->sidlength));p+=2) {
	PR_fprintf(PR_STDOUT,"0x%04x ",(PRUint32)(GET_SHORT((&chv2->csuites[p+q]))));
      }
    }
    q += p;
    PR_fprintf(PR_STDOUT,"}\n");
    if (chv2->rndlength) {
      PR_fprintf(PR_STDOUT,"           challenge = { ");
      for (p=0;p<GET_SHORT((chv2->rndlength));p+=2) {
	PR_fprintf(PR_STDOUT,"0x%04x ",(PRUint32)(GET_SHORT((&chv2->csuites[p+q]))));
      }
      PR_fprintf(PR_STDOUT,"}\n");
    }
    PR_fprintf(PR_STDOUT,"}\n");
    break;
    /* end of V2 CLientHello Parsing */

  case 2:  /* Client Master Key  */
    {
    const char *cs_str=NULL;
    PRUint32 cs_int=0;
    ClientMasterKeyV2 *cmkv2;
    cmkv2 = (ClientMasterKeyV2 *)chv2;
    isV2Session = 1;

    PR_fprintf(PR_STDOUT," [ssl2]  ClientMasterKeyV2 { \n");

    cs_int = GET_24(&cmkv2->cipherkind[0]);
    cs_str = V2CipherString(cs_int);
    PR_fprintf(PR_STDOUT,"         cipher-spec-chosen = (0x%06x) %s\n",
	       cs_int, cs_str);

    PR_fprintf(PR_STDOUT,"         clear-portion = %d bits\n",
	       8*(PRUint32)(GET_SHORT((cmkv2->clearkey))));

    PR_fprintf(PR_STDOUT,"      }\n");
    clientstream.isEncrypted = 1;
    serverstream.isEncrypted = 1;
    }
    break;


  case 3:
    PR_fprintf(PR_STDOUT," [ssl2]  Client Finished V2 {...}\n");
    isV2Session = 1;
    break;


  case 4:  /* V2 Server Hello */
    isV2Session = 1;

    PR_fprintf(PR_STDOUT," [ssl2]  ServerHelloV2 {\n");
    PR_fprintf(PR_STDOUT,"           sid hit = {0x%02x}\n",
	       (PRUintn)shv2->sidhit);
    PR_fprintf(PR_STDOUT,"           version = {0x%02x, 0x%02x}\n",
	       (PRUint32)shv2->version[0],(PRUint32)shv2->version[1]);
    PR_fprintf(PR_STDOUT,"           cipher-specs-length = %d (0x%02x)\n",
	       (PRUint32)(GET_SHORT((shv2->cslength))),
	       (PRUint32)(GET_SHORT((shv2->cslength))));
    PR_fprintf(PR_STDOUT,"           sid-length = %d (0x%02x)\n",
	       (PRUint32)(GET_SHORT((shv2->cidlength))),
	       (PRUint32)(GET_SHORT((shv2->cidlength))));

    pos = (unsigned char *)shv2;
    pos += 2;   /* skip length header */
    pos += 11;  /* position pointer to Certificate data area */
    q = GET_SHORT(&shv2->certlength);
    if (q >alloclen) {
      goto eosh;
    }
    pos += q; 			/* skip certificate */

    PR_fprintf(PR_STDOUT,"           cipher-suites = { ");
    len = GET_SHORT((shv2->cslength));
    for (p = 0; p < len; p += 3) {
      const char *cs_str=NULL;
      PRUint32 cs_int=0;
      cs_int = GET_24((pos+p));
      cs_str = V2CipherString(cs_int);
      PR_fprintf(PR_STDOUT,"\n              ");
      PR_fprintf(PR_STDOUT,"(0x%06x) %s", cs_int, cs_str);
    }
    pos += len;
    PR_fprintf(PR_STDOUT,"   }\n");	/* End of cipher suites */
    len = (PRUint32)GET_SHORT((shv2->cidlength));
    if (len) {
      PR_fprintf(PR_STDOUT,"           connection-id = { ");
      for (p = 0; p < len; p += 2) {
	PR_fprintf(PR_STDOUT,"0x%04x ", (PRUint32)(GET_SHORT((pos + p))));
      }
      PR_fprintf(PR_STDOUT,"   }\n");	/* End of connection id */
    }
eosh:
    PR_fprintf(PR_STDOUT,"\n              }\n"); /* end of ServerHelloV2 */
    if (shv2->sidhit) {
      clientstream.isEncrypted = 1;
      serverstream.isEncrypted = 1;
    }
    break;

  case 5:
    PR_fprintf(PR_STDOUT," [ssl2]  Server Verify V2 {...}\n");
    isV2Session = 1;
    break;

  case 6:
    PR_fprintf(PR_STDOUT," [ssl2]  Server Finished V2 {...}\n");
    isV2Session = 1;
    break;

  case 7:
    PR_fprintf(PR_STDOUT," [ssl2]  Request Certificate V2 {...}\n");
    isV2Session = 1;
    break;

  case 8:
    PR_fprintf(PR_STDOUT," [ssl2]  Client Certificate V2 {...}\n");
    isV2Session = 1;
    break;

  default:
    PR_fprintf(PR_STDOUT," [ssl2]  UnknownType 0x%02x {...}\n",
	  (PRUint32)chv2->type);
    break;
  }
}



unsigned int print_hello_extension(unsigned char *  hsdata,
				   unsigned int     length,
				   unsigned int     pos)
{
  /* pretty print extensions, if any */
  if (pos < length) {
    int exListLen = GET_SHORT((hsdata+pos)); pos += 2;
    PR_fprintf(PR_STDOUT,
	       "            extensions[%d] = {\n", exListLen);
    while (exListLen > 0 && pos < length) {
      int exLen;
      int exType = GET_SHORT((hsdata+pos)); pos += 2;
      exLen = GET_SHORT((hsdata+pos)); pos += 2;
      /* dump the extension */
      PR_fprintf(PR_STDOUT,
		 "              extension type %s, length [%d]",
		 helloExtensionNameString(exType), exLen);
      if (exLen > 0) {
	  PR_fprintf(PR_STDOUT, " = {\n");
	  print_hex(exLen, hsdata + pos);
	  PR_fprintf(PR_STDOUT, "              }\n");
      } else {
	  PR_fprintf(PR_STDOUT, "\n");
      }
      pos += exLen;
      exListLen -= 2 + exLen;
    }
    PR_fprintf(PR_STDOUT,"            }\n");
  }
  return pos;
}


void print_ssl3_handshake(unsigned char *tbuf, 
                          unsigned int   alloclen,
                          SSLRecord *    sr)
{
  struct sslhandshake sslh; 
  unsigned char *     hsdata;  
  int                 offset=0;

  PR_fprintf(PR_STDOUT,"   handshake {\n");

  while (offset + hMACsize < alloclen) {
    sslh.type = tbuf[offset]; 
    sslh.length = GET_24(tbuf+offset+1);
    hsdata= &tbuf[offset+4];

    if (sslhexparse) print_hex(4,tbuf+offset);

    PR_fprintf(PR_STDOUT,"      type = %d (",sslh.type);
    switch(sslh.type) {
    case 0:  PR_FPUTS("hello_request)\n"               ); break;
    case 1:  PR_FPUTS("client_hello)\n"                ); break;
    case 2:  PR_FPUTS("server_hello)\n"                ); break;
    case 11: PR_FPUTS("certificate)\n"                 ); break;
    case 12: PR_FPUTS("server_key_exchange)\n"         ); break;
    case 13: PR_FPUTS("certificate_request)\n"         ); break;
    case 14: PR_FPUTS("server_hello_done)\n"           ); break;
    case 15: PR_FPUTS("certificate_verify)\n"          ); break;
    case 16: PR_FPUTS("client_key_exchange)\n"         ); break;
    case 20: PR_FPUTS("finished)\n"                    ); break;
    default: PR_FPUTS("unknown)\n"                     ); break;
    }

    PR_fprintf(PR_STDOUT,"      length = %d (0x%06x)\n",sslh.length,sslh.length);
    switch (sslh.type) {

    case 0: /* hello_request */ /* not much to show here. */ break;

    case 1: /* client hello */
      switch (sr->ver_maj)  {
      case 3:  /* ssl version 3 */
	{
	  unsigned int pos;
	  int w;

	  PR_fprintf(PR_STDOUT,"         ClientHelloV3 {\n");
	  PR_fprintf(PR_STDOUT,"            client_version = {%d, %d}\n",
		     (PRUint8)hsdata[0],(PRUint8)hsdata[1]);
	  PR_fprintf(PR_STDOUT,"            random = {...}\n");
	  if (sslhexparse) print_hex(32,&hsdata[2]);

	  /* pretty print Session ID */
	  {
	    int sidlength = (int)hsdata[2+32];
	    PR_fprintf(PR_STDOUT,"            session ID = {\n");
	    PR_fprintf(PR_STDOUT,"                length = %d\n",sidlength);
	    PR_fprintf(PR_STDOUT,"                contents = {..}\n");
	    if (sslhexparse) print_hex(sidlength,&hsdata[2+32+1]);
	    PR_fprintf(PR_STDOUT,"            }\n");
	    pos = 2+32+1+sidlength;
	  }

	  /* pretty print cipher suites */
	  {
	    int csuitelength = GET_SHORT((hsdata+pos));
	    PR_fprintf(PR_STDOUT,"            cipher_suites[%d] = { \n",
		       csuitelength/2);
	    if (csuitelength % 2) {
	      PR_fprintf(PR_STDOUT,
		 "*error in protocol - csuitelength shouldn't be odd*\n");
	    }
	    for (w=0; w<csuitelength; w+=2) {
	      const char *cs_str=NULL;
	      PRUint32 cs_int=0;
	      cs_int = GET_SHORT((hsdata+pos+2+w));
	      cs_str = V2CipherString(cs_int);
	      PR_fprintf(PR_STDOUT,
		"                (0x%04x) %s\n", cs_int, cs_str);
	    }
	    pos += 2 + csuitelength;
	    PR_fprintf(PR_STDOUT,"            }\n");
	  }

	  /* pretty print compression methods */
	  {
	    int complength = hsdata[pos];
	    PR_fprintf(PR_STDOUT,"            compression[%d] = {",
	               complength);
	    for (w=0; w < complength; w++) {
	      PR_fprintf(PR_STDOUT, " %02x", hsdata[pos+1+w]);
	    }
	    pos += 1 + complength;
	    PR_fprintf(PR_STDOUT," }\n");
	  }

	  /* pretty print extensions, if any */
	  pos = print_hello_extension(hsdata, sslh.length, pos);

	  PR_fprintf(PR_STDOUT,"         }\n");
	} /* end of ssl version 3 */
	break;
      default:
	PR_fprintf(PR_STDOUT,"         UNDEFINED VERSION %d.%d {...}\n",
	                     sr->ver_maj, sr->ver_min );
	if (sslhexparse) print_hex(sslh.length, hsdata);
	break;
      } /* end of switch sr->ver_maj */
      break;

    case 2: /* server hello */
      {
	unsigned int sidlength, pos;

	PR_fprintf(PR_STDOUT,"         ServerHello {\n");

	PR_fprintf(PR_STDOUT,"            server_version = {%d, %d}\n",
		   (PRUint8)hsdata[0],(PRUint8)hsdata[1]);
	PR_fprintf(PR_STDOUT,"            random = {...}\n");
	if (sslhexparse) print_hex(32,&hsdata[2]);
	PR_fprintf(PR_STDOUT,"            session ID = {\n");
	sidlength = (int)hsdata[2+32];
	PR_fprintf(PR_STDOUT,"                length = %d\n",sidlength);
	PR_fprintf(PR_STDOUT,"                contents = {..}\n");
	if (sslhexparse) print_hex(sidlength,&hsdata[2+32+1]);
	PR_fprintf(PR_STDOUT,"            }\n");
	pos = 2+32+1+sidlength;

	/* pretty print chosen cipher suite */
	{
	  PRUint32 cs_int    = GET_SHORT((hsdata+pos));
	  const char *cs_str = V2CipherString(cs_int);
	  PR_fprintf(PR_STDOUT,"            cipher_suite = (0x%04x) %s\n",
		     cs_int, cs_str);
	  currentcipher = cs_int;
	  pos += 2;
	}
	PR_fprintf(PR_STDOUT,  "            compression method = %02x\n", 
		   hsdata[pos++]);

	/* pretty print extensions, if any */
	pos = print_hello_extension(hsdata, sslh.length, pos);

	PR_fprintf(PR_STDOUT,"         }\n");
      }
      break;

    case 11: /* certificate */
      {
	PRFileDesc *cfd;
	int         pos;
	int         certslength;
	int         certlength;
	int         certbytesread	= 0;
	static int  certFileNumber;
	char        certFileName[20];

	PR_fprintf(PR_STDOUT,"         CertificateChain {\n");
	certslength = GET_24(hsdata);
	PR_fprintf(PR_STDOUT,"            chainlength = %d (0x%04x)\n",
		certslength,certslength);
	pos = 3;
	while (certbytesread < certslength) {
	  certlength = GET_24((hsdata+pos));
	  pos += 3;
	  PR_fprintf(PR_STDOUT,"            Certificate {\n");
	  PR_fprintf(PR_STDOUT,"               size = %d (0x%04x)\n",
		certlength,certlength);

	  PR_snprintf(certFileName, sizeof certFileName, "cert.%03d",
	              ++certFileNumber);
	  cfd = PR_Open(certFileName, PR_WRONLY|PR_CREATE_FILE|PR_TRUNCATE, 
	                0664);
	  if (!cfd) {
	    PR_fprintf(PR_STDOUT,
	               "               data = { couldn't save file '%s' }\n",
		       certFileName);
	  } else {
	    PR_Write(cfd, (hsdata+pos), certlength);
	    PR_fprintf(PR_STDOUT,
	               "               data = { saved in file '%s' }\n",
		       certFileName);
	    PR_Close(cfd);
	  }

	  PR_fprintf(PR_STDOUT,"            }\n");
	  pos           += certlength;
	  certbytesread += certlength+3;
	}
	PR_fprintf(PR_STDOUT,"         }\n");
      }
      break;

    case 12: /* server_key_exchange */                    
      if (sslhexparse) print_hex(sslh.length, hsdata);
      break;

    case 13: /* certificate request */
      { 
	unsigned int pos = 0;
	int w, reqLength;

	PR_fprintf(PR_STDOUT,"         CertificateRequest {\n");

	/* pretty print requested certificate types */
	reqLength = hsdata[pos];
	PR_fprintf(PR_STDOUT,"            certificate types[%d] = {",
		   reqLength);
	for (w=0; w < reqLength; w++) {
	  PR_fprintf(PR_STDOUT, " %02x", hsdata[pos+1+w]);
	}
	pos += 1 + reqLength;
	PR_fprintf(PR_STDOUT," }\n");

	/* pretty print CA names, if any */
	if (pos < sslh.length) {
	  int exListLen = GET_SHORT((hsdata+pos)); pos += 2;
	  PR_fprintf(PR_STDOUT,
		     "            certificate_authorities[%d] = {\n", 
		     exListLen);
	  while (exListLen > 0 && pos < sslh.length) {
	    char *  ca_name;
	    SECItem it;
	    int     dnLen = GET_SHORT((hsdata+pos)); pos += 2;

	    /* dump the CA name */
	    it.type = siBuffer;
	    it.data = hsdata + pos;
	    it.len  = dnLen;
	    ca_name = CERT_DerNameToAscii(&it);
	    if (ca_name) {
	      PR_fprintf(PR_STDOUT,"   %s\n", ca_name);
	      PORT_Free(ca_name);
	    } else {
	      PR_fprintf(PR_STDOUT,
			 "              distinguished name [%d]", dnLen);
	      if (dnLen > 0 && sslhexparse) {
		  PR_fprintf(PR_STDOUT, " = {\n");
		  print_hex(dnLen, hsdata + pos);
		  PR_fprintf(PR_STDOUT, "              }\n");
	      } else {
		  PR_fprintf(PR_STDOUT, "\n");
	      }
            }
	    pos += dnLen;
	    exListLen -= 2 + dnLen;
	  }
	  PR_fprintf(PR_STDOUT,"            }\n");
	}

	PR_fprintf(PR_STDOUT,"         }\n");
      }
      break;

    case 14: /* server_hello_done */ /* not much to show here. */ break;

    case 15: /* certificate_verify */	           
      if (sslhexparse) print_hex(sslh.length, hsdata);
      break;

    case 16: /* client key exchange */
      {
	PR_fprintf(PR_STDOUT,"         ClientKeyExchange {\n");
	PR_fprintf(PR_STDOUT,"            message = {...}\n");
	PR_fprintf(PR_STDOUT,"         }\n");
      }
      break;

    case 20: /* finished */			 
      PR_fprintf(PR_STDOUT,"         Finished {\n");
      PR_fprintf(PR_STDOUT,"            verify_data = {...}\n");
      if (sslhexparse) print_hex(sslh.length, hsdata);
      PR_fprintf(PR_STDOUT,"         }\n");

      if (!isNULLmac(currentcipher) && !hMACsize) {
          /* To calculate the size of MAC, we subtract the number
           * of known bytes of message from the number of remaining
           * bytes in the record. */
          hMACsize = alloclen - (sslh.length + 4);
      }
      break;

    default:
      {
	PR_fprintf(PR_STDOUT,"         UNKNOWN MESSAGE TYPE %d [%d] {\n",
	                     sslh.type, sslh.length);
	if (sslhexparse) print_hex(sslh.length, hsdata);
	PR_fprintf(PR_STDOUT,"         }\n");

      }
    }  /* end of switch sslh.type */
    offset += sslh.length + 4; /* +4 because of length (3 bytes) and type (1 byte) */
  } /* while */
  if (hMACsize) {
      /* at this point offset should be at the first byte of MAC */
      if (offset + hMACsize > alloclen) {
          PR_fprintf(PR_STDOUT,"BAD RECORD: content + MAC ends beyond "
                     "allocated limit.\n");
      } else {
          PR_fprintf(PR_STDOUT,"      MAC = {...}\n");
          if (sslhexparse) print_hex(hMACsize, hsdata);
      }
  }
  PR_fprintf(PR_STDOUT,"   }\n");
}


void print_ssl(DataBufferList *s, int length, unsigned char *buffer)
{
  /* --------------------------------------------------------  */
  /* first, create a new buffer object for this piece of data. */

  DataBuffer *db;
  int i,l;

  if (s->size == 0 && length > 0 && buffer[0] >= 32 && buffer[0] < 128) {
    /* Not an SSL record, treat entire buffer as plaintext */
    PR_Write(PR_STDOUT,buffer,length);
    return;
  }


  check_integrity(s);

  i = 0;
  l = length;

  db = PR_NEW(struct _DataBuffer);

  db->buffer = (unsigned char*)PR_Malloc(length);
  db->length = length;
  db->offset = 0;
  memcpy(db->buffer, buffer, length);
  db->next = NULL;

  /* now, add it to the stream */

  if (s->last != NULL) s->last->next = db;
  s->last = db;
  s->size += length;
  if (s->first == NULL) s->first = db;

  check_integrity(s);

  /*------------------------------------------------------- */
  /* now we look at the stream to see if we have enough data to
     decode  */

  while (s->size > 0 ) {
    unsigned char *tbuf = NULL;

    SSLRecord sr;
    unsigned alloclen;
    unsigned recordsize;

    check_integrity(s);

    if ( s->first == NULL) {
      PR_fprintf(PR_STDOUT,"ERROR: s->first is null\n");
      exit(9);
    }

    /* in the case of an SSL 2 client-hello (which is all ssltap supports) */
    /* will have the high-bit set, whereas an SSL 3 client-hello will not  */
    /* SSL2 can also send records that begin with the high bit clear.
     * This code will incorrectly handle them. XXX
     */
    if (isV2Session || s->first->buffer[s->first->offset] & 0x80) {
      /* it's an SSL 2 packet */
      unsigned char lenbuf[3];

      /* first, we check if there's enough data for it to be an SSL2-type
       * record.  What a pain.*/
      if (s->size < sizeof lenbuf) {
	partial_packet(length, s->size, sizeof lenbuf);
	return;
      }

      /* read the first two bytes off the stream. */
      read_stream_bytes(lenbuf, s, sizeof(lenbuf));
      alloclen = ((unsigned int)(lenbuf[0] & 0x7f) << 8) + lenbuf[1] + 
                 ((lenbuf[0] & 0x80) ? 2 : 3);
      PR_fprintf(PR_STDOUT, "alloclen = %u bytes\n", alloclen);

      /* put 'em back on the head of the stream. */
      db = PR_NEW(struct _DataBuffer);

      db->length = sizeof lenbuf;
      db->buffer = (unsigned char*) PR_Malloc(db->length);
      db->offset = 0;
      memcpy(db->buffer, lenbuf, sizeof lenbuf);

      db->next = s->first;
      s->first = db;
      if (s->last == NULL) 
	s->last = db;
      s->size += db->length;

      /* if there wasn't enough, go back for more. */
      if (s->size < alloclen) {
	check_integrity(s);
	partial_packet(length, s->size, alloclen);
	return;
      }
      partial_packet(length, s->size, alloclen);

      /* read in the whole record. */
      tbuf = PR_Malloc(alloclen);
      read_stream_bytes(tbuf, s, alloclen);

      print_sslv2(s, tbuf, alloclen);
      PR_FREEIF(tbuf);
      check_integrity(s);

      continue;
    }

    /***********************************************************/
    /* It's SSL v3 */
    /***********************************************************/
    check_integrity(s);

    if (s->size < sizeof(SSLRecord)) {
      partial_packet(length, s->size, sizeof(SSLRecord));
      return;
    }

    read_stream_bytes((unsigned char *)&sr, s, sizeof sr);

    /* we have read the stream bytes. Look at the length of
       the ssl record. If we don't have enough data to satisfy this
       request, then put the bytes we just took back at the head
       of the queue */
    recordsize = GET_SHORT(sr.length);

    if (recordsize > s->size) {
      db = PR_NEW(struct _DataBuffer);

      db->length = sizeof sr;
      db->buffer = (unsigned char*) PR_Malloc(db->length);
      db->offset = 0;
      memcpy(db->buffer, &sr, sizeof sr);
      db->next = s->first;

      /* now, add it back on to the head of the stream */

      s->first = db;
      if (s->last == NULL) 
        s->last = db;
      s->size += db->length;

      check_integrity(s);
      partial_packet(length, s->size, recordsize);
      return;
    }
    partial_packet(length, s->size, recordsize);


    PR_fprintf(PR_STDOUT,"SSLRecord { [%s]\n", get_time_string() );
    if (sslhexparse) {
      print_hex(5,(unsigned char*)&sr);
    }

    check_integrity(s);

    PR_fprintf(PR_STDOUT,"   type    = %d (",sr.type);
    switch(sr.type) {
    case 20 :
      PR_fprintf(PR_STDOUT,"change_cipher_spec)\n");
      break;
    case 21 :
      PR_fprintf(PR_STDOUT,"alert)\n");
      break;
    case 22 :
      PR_fprintf(PR_STDOUT,"handshake)\n");
      break;
    case 23 :
      PR_fprintf(PR_STDOUT,"application_data)\n");
      break;
    default:
      PR_fprintf(PR_STDOUT,"unknown)\n");
      break;
    }
    PR_fprintf(PR_STDOUT,"   version = { %d,%d }\n",
	       (PRUint32)sr.ver_maj,(PRUint32)sr.ver_min);
    PR_fprintf(PR_STDOUT,"   length  = %d (0x%x)\n",
    	(PRUint32)GET_SHORT(sr.length), (PRUint32)GET_SHORT(sr.length));


    alloclen = recordsize;
    PR_ASSERT(s->size >= alloclen);
    if (s->size >= alloclen) {
      tbuf = (unsigned char*) PR_Malloc(alloclen);
      read_stream_bytes(tbuf, s, alloclen);

      if (s->isEncrypted) {
	PR_fprintf(PR_STDOUT,"            < encrypted >\n");
      } else 

      switch(sr.type) {
      case 20 : /* change_cipher_spec */
	if (sslhexparse) print_hex(alloclen,tbuf);
         /* mark to say we can only dump hex form now on
          * if it is not one on a null cipher */
	s->isEncrypted = isNULLcipher(currentcipher) ? 0 : 1; 
	break;

      case 21 : /* alert */
	switch(tbuf[0]) {
	case 1: PR_fprintf(PR_STDOUT, "   warning: "); break;
	case 2: PR_fprintf(PR_STDOUT, "   fatal: "); break;
	default: PR_fprintf(PR_STDOUT, "   unknown level %d: ", tbuf[0]); break;
	}

	switch(tbuf[1]) {
	case 0:   PR_FPUTS("close_notify\n"                    ); break;
	case 10:  PR_FPUTS("unexpected_message\n"              ); break;
	case 20:  PR_FPUTS("bad_record_mac\n"                  ); break;
	case 21:  PR_FPUTS("decryption_failed\n"               ); break;
	case 22:  PR_FPUTS("record_overflow\n"                 ); break;
	case 30:  PR_FPUTS("decompression_failure\n"           ); break;
	case 40:  PR_FPUTS("handshake_failure\n"               ); break;
	case 41:  PR_FPUTS("no_certificate\n"                  ); break;
	case 42:  PR_FPUTS("bad_certificate\n"                 ); break;
	case 43:  PR_FPUTS("unsupported_certificate\n"         ); break;
	case 44:  PR_FPUTS("certificate_revoked\n"             ); break;
	case 45:  PR_FPUTS("certificate_expired\n"             ); break;
	case 46:  PR_FPUTS("certificate_unknown\n"             ); break;
	case 47:  PR_FPUTS("illegal_parameter\n"               ); break;
	case 48:  PR_FPUTS("unknown_ca\n"                      ); break;
	case 49:  PR_FPUTS("access_denied\n"                   ); break;
	case 50:  PR_FPUTS("decode_error\n"                    ); break;
	case 51:  PR_FPUTS("decrypt_error\n"                   ); break;
	case 60:  PR_FPUTS("export_restriction\n"              ); break;
	case 70:  PR_FPUTS("protocol_version\n"                ); break;
	case 71:  PR_FPUTS("insufficient_security\n"           ); break;
	case 80:  PR_FPUTS("internal_error\n"                  ); break;
	case 90:  PR_FPUTS("user_canceled\n"                   ); break;
	case 100: PR_FPUTS("no_renegotiation\n"                ); break;
	case 110: PR_FPUTS("unsupported_extension\n"           ); break;
	case 111: PR_FPUTS("certificate_unobtainable\n"        ); break;
	case 112: PR_FPUTS("unrecognized_name\n"               ); break;
	case 113: PR_FPUTS("bad_certificate_status_response\n" ); break;
	case 114: PR_FPUTS("bad_certificate_hash_value\n"      ); break;

	default:  PR_fprintf(PR_STDOUT, "unknown alert %d\n", tbuf[1]); break;
	}

	if (sslhexparse) print_hex(alloclen,tbuf);
	break;

      case 22 : /* handshake */ 	
        print_ssl3_handshake( tbuf, alloclen, &sr );
	break;

      case 23 : /* application data */
         if (hMACsize) {
             print_hex(alloclen - hMACsize,tbuf);
             PR_fprintf(PR_STDOUT,"      MAC = {...}\n");
             if (sslhexparse) {
                 unsigned char *offset = tbuf + (alloclen - hMACsize);
                 print_hex(hMACsize, offset);
             }
         } else {
             print_hex(alloclen,tbuf);
         }
         break;

      default:
	print_hex(alloclen,tbuf);
	break;
      }
    }
    PR_fprintf(PR_STDOUT,"}\n");
    PR_FREEIF(tbuf);
    check_integrity(s);
  }
}

void print_hex(int amt, unsigned char *buf) {
  int i,j,k;
  char t[20];
  static char string[5000];


  for(i=0;i<amt;i++) {
    t[1] =0;

    if (i%16 ==0) {  /* if we are at the beginning of a line */
      PR_fprintf(PR_STDOUT,"%4x:",i); /* print the line number  */
      strcpy(string,"");
    }

    if (i%4 == 0) {
      PR_fprintf(PR_STDOUT," ");
    }

    j = buf[i];

    t[0] = (j >= 0x20 && j < 0x80) ? j : '.';

    if (fancy)  {
      switch (t[0]) {
      case '<':
        strcpy(t,"&lt;");
        break;
      case '>':
        strcpy(t,"&gt;");
        break;
      case '&':
        strcpy(t,"&amp;");
        break;
      }
    }
    strcat(string,t);

    PR_fprintf(PR_STDOUT,"%02x ",(PRUint8) buf[i]);

    /* if we've reached the end of the line - add the string */
    if (i%16 == 15) PR_fprintf(PR_STDOUT," | %s\n",string);
  }
  /* we reached the end of the buffer,*/
  /* do we have buffer left over? */
  j = i%16;
  if (j > 0) {
    for (k=0;k<(16-j);k++) {
        /* print additional space after every four bytes */
        if ((k + j)%4 == 0) {
            PR_fprintf(PR_STDOUT," ");
        }
        PR_fprintf(PR_STDOUT,"   ");
    }
    PR_fprintf(PR_STDOUT," | %s\n",string);
  }
}

void Usage(void) {
  PR_fprintf(PR_STDERR, "SSLTAP (C) 1997, 1998 Netscape Communications Corporation.\n");
  PR_fprintf(PR_STDERR, "Usage: ssltap [-vhfsxl] [-p port] hostname:port\n");
  PR_fprintf(PR_STDERR, "   -v      [prints version string]\n");
  PR_fprintf(PR_STDERR, "   -h      [outputs hex instead of ASCII]\n");
  PR_fprintf(PR_STDERR, "   -f      [turn on Fancy HTML coloring]\n");
  PR_fprintf(PR_STDERR, "   -s      [turn on SSL decoding]\n");
  PR_fprintf(PR_STDERR, "   -x      [turn on extra SSL hex dumps]\n");
  PR_fprintf(PR_STDERR, "   -p port [specify rendezvous port (default 1924)]\n");
  PR_fprintf(PR_STDERR, "   -l      [loop - continue to wait for more connections]\n");


}

void
showErr(const char * msg) {
  PRErrorCode  err       = PR_GetError();
  const char * errString;

  if (err == PR_UNKNOWN_ERROR)
    err = PR_CONNECT_RESET_ERROR;	/* bug in NSPR. */
  errString = SECU_Strerror(err);

  if (!errString)
    errString = "(no text available)";
  PR_fprintf(PR_STDERR, "Error %d: %s: %s", err, errString, msg);
}

int main(int argc,  char *argv[])
{
  char *hostname=NULL;
  PRUint16 rendport=DEFPORT,port;
  PRHostEnt hp;
  PRStatus r;
  PRNetAddr na_client,na_server,na_rend;
  PRFileDesc *s_server,*s_client,*s_rend; /*rendezvous */
  char netdbbuf[PR_NETDB_BUF_SIZE];
  int c_count=0;
  PLOptState *optstate;
  PLOptStatus status;
  SECStatus   rv;

  optstate = PL_CreateOptState(argc,argv,"fvxhslp:");
    while ((status = PL_GetNextOpt(optstate)) == PL_OPT_OK) {
    switch (optstate->option) {
    case 'f':
      fancy++;
      break;
    case 'h':
      hexparse++;
      break;
    case 'v':
      PR_fprintf(PR_STDOUT,"Version: %s\n",VERSIONSTRING);
      break;
    case 's':
      sslparse++;
      break;
    case 'x':
      sslhexparse++;
      break;
    case 'l':
      looparound++;
      break;
    case 'p':
      rendport = atoi(optstate->value);
      break;
    case '\0':
      hostname = PL_strdup(optstate->value);
    }
  }
  if (status == PL_OPT_BAD)
    Usage();

  if (fancy) {
    if (!hexparse && !sslparse) {
      PR_fprintf(PR_STDERR,
"Note: use of -f without -s or -h not recommended, \n"
"as the output looks a little strange. It may be useful, however\n");
    }
  }

  if(! hostname ) Usage(), exit(2);

  {
    char *colon = (char *)strchr(hostname, ':');
    if (!colon) {
      PR_fprintf(PR_STDERR,
      "You must specify the host AND port you wish to connect to\n");
      Usage(), exit(3);
    }
    port = atoi(&colon[1]);
    *colon = '\0';

    if (port == 0) {
	PR_fprintf(PR_STDERR, "Port must be a nonzero number.\n");
	exit(4);
      }
  }

  /* find the 'server' IP address so we don't have to look it up later */

  if (fancy) {
      PR_fprintf(PR_STDOUT,"<HTML><HEAD><TITLE>SSLTAP output</TITLE></HEAD>\n");
      PR_fprintf(PR_STDOUT,"<BODY><PRE>\n");
    }
  PR_fprintf(PR_STDERR,"Looking up \"%s\"...\n", hostname);
  r = PR_GetHostByName(hostname,netdbbuf,PR_NETDB_BUF_SIZE,&hp);
  if (r) {
    showErr("Host Name lookup failed\n");
    exit(5);
  }

  PR_EnumerateHostEnt(0,&hp,0,&na_server);
  PR_InitializeNetAddr(PR_IpAddrNull,port,&na_server);
  /* set up the port which the client will connect to */

  r = PR_InitializeNetAddr(PR_IpAddrAny,rendport,&na_rend);
  if (r == PR_FAILURE) {
    PR_fprintf(PR_STDERR,
    "PR_InitializeNetAddr(,%d,) failed with error %d\n",PR_GetError());
    exit(0);
  }

  rv = NSS_NoDB_Init("");
  if (rv != SECSuccess) {
    PR_fprintf(PR_STDERR,
    "NSS_NoDB_Init() failed with error %d\n",PR_GetError());
    exit(5);
  }

  s_rend = PR_NewTCPSocket();
  if (!s_rend) {
    showErr("Couldn't create socket\n");
    exit(6);
  }

  if (PR_Bind(s_rend, &na_rend )) {
    PR_fprintf(PR_STDERR,"Couldn't bind to port %d (error %d)\n",rendport, PR_GetError());
    exit(-1);
  }

  if ( PR_Listen(s_rend, 5)) {
    showErr("Couldn't listen\n");
    exit(-1);
  }

  PR_fprintf(PR_STDERR,"Proxy socket ready and listening\n");
  do {	/* accept one connection and process it. */
      PRPollDesc pds[2];

      s_client = PR_Accept(s_rend,&na_client,PR_SecondsToInterval(3600));
      if (s_client == NULL) {
	showErr("accept timed out\n");
	exit(7);
      }

      s_server = PR_NewTCPSocket();
      if (s_server == NULL) {
	showErr("couldn't open new socket to connect to server \n");
	exit(8);
      }

      r = PR_Connect(s_server,&na_server,PR_SecondsToInterval(5));

      if ( r == PR_FAILURE )
        {
	  showErr("Couldn't connect\n");
	  return -1;
        }

      if (looparound) {
	if (fancy)  PR_fprintf(PR_STDOUT,"<p><HR><H2>");
	PR_fprintf(PR_STDOUT,"Connection #%d [%s]\n", c_count+1,
	                     get_time_string());
	if (fancy)  PR_fprintf(PR_STDOUT,"</H2>");
	}


      PR_fprintf(PR_STDOUT,"Connected to %s:%d\n", hostname, port);

#define PD_C 0
#define PD_S 1

      pds[PD_C].fd = s_client;
      pds[PD_S].fd = s_server;
      pds[PD_C].in_flags = PR_POLL_READ;
      pds[PD_S].in_flags = PR_POLL_READ;

      /* make sure the new connections don't start out encrypted. */
      clientstream.isEncrypted = 0;
      serverstream.isEncrypted = 0;
      isV2Session = 0;

      while( (pds[PD_C].in_flags & PR_POLL_READ) != 0 ||
             (pds[PD_S].in_flags & PR_POLL_READ) != 0 )
        {  /* Handle all messages on the connection */
	  PRInt32 amt;
	  PRInt32 wrote;
	  unsigned char buffer[ TAPBUFSIZ ];

	  amt = PR_Poll(pds,2,PR_INTERVAL_NO_TIMEOUT);
	  if (amt <= 0) {
	    if (amt)
		showErr( "PR_Poll failed.\n");
	    else
		showErr( "PR_Poll timed out.\n");
	    break;
	  }

	  if (pds[PD_C].out_flags & PR_POLL_EXCEPT) {
	    showErr( "Exception on client-side socket.\n");
	    break;
	  }

	  if (pds[PD_S].out_flags & PR_POLL_EXCEPT) {
	    showErr( "Exception on server-side socket.\n");
	    break;
	  }


/* read data, copy it to stdout, and write to other socket */

	  if ((pds[PD_C].in_flags  & PR_POLL_READ) != 0 &&
	      (pds[PD_C].out_flags & PR_POLL_READ) != 0 ) {

	    amt = PR_Read(s_client, buffer, sizeof(buffer));

	    if ( amt < 0)  {
	      showErr( "Client socket read failed.\n");
	      break;
	    }

	    if( amt == 0 ) {
	      PR_fprintf(PR_STDOUT, "Read EOF on Client socket. [%s]\n",
	                            get_time_string() );
	      pds[PD_C].in_flags &= ~PR_POLL_READ;
	      PR_Shutdown(s_server, PR_SHUTDOWN_SEND);
	      continue;
	    }

	    PR_fprintf(PR_STDOUT,"--> [\n");
	    if (fancy) PR_fprintf(PR_STDOUT,"<font color=blue>");

	    if (hexparse)  print_hex(amt, buffer);
	    if (sslparse)  print_ssl(&clientstream,amt,buffer);
	    if (!hexparse && !sslparse)  PR_Write(PR_STDOUT,buffer,amt);
	    if (fancy) PR_fprintf(PR_STDOUT,"</font>");
	    PR_fprintf(PR_STDOUT,"]\n");

	    wrote = PR_Write(s_server, buffer, amt);
	    if (wrote != amt )  {
	      if (wrote < 0) {
	        showErr("Write to server socket failed.\n");
		break;
	      } else {
		PR_fprintf(PR_STDERR, "Short write to server socket!\n");
	      }
	    }
	  }  /* end of read from client socket. */

/* read data, copy it to stdout, and write to other socket */
	  if ((pds[PD_S].in_flags  & PR_POLL_READ) != 0 &&
	      (pds[PD_S].out_flags & PR_POLL_READ) != 0 ) {

	    amt = PR_Read(s_server, buffer, sizeof(buffer));

	    if ( amt < 0)  {
	      showErr( "error on server-side socket.\n");
	      break;
	    }

	    if( amt == 0 ) {
	      PR_fprintf(PR_STDOUT, "Read EOF on Server socket. [%s]\n",
	                            get_time_string() );
	      pds[PD_S].in_flags &= ~PR_POLL_READ;
	      PR_Shutdown(s_client, PR_SHUTDOWN_SEND);
	      continue;
	    } 

	    PR_fprintf(PR_STDOUT,"<-- [\n");
	    if (fancy) PR_fprintf(PR_STDOUT,"<font color=red>");
	    if (hexparse)  print_hex(amt, (unsigned char *)buffer);
	    if (sslparse)  print_ssl(&serverstream,amt,(unsigned char *)buffer);
	    if (!hexparse && !sslparse)  PR_Write(PR_STDOUT,buffer,amt);
	    if (fancy) PR_fprintf(PR_STDOUT,"</font>");
	    PR_fprintf(PR_STDOUT,"]\n");


	    wrote = PR_Write(s_client, buffer, amt);
	    if (wrote != amt )  {
	      if (wrote < 0) {
	        showErr("Write to client socket failed.\n");
		break;
	      } else {
		PR_fprintf(PR_STDERR, "Short write to client socket!\n");
	      }
	    }

	  } /* end of read from server socket. */

/* Loop, handle next message. */

        }	/* handle messages during a connection loop */
      PR_Close(s_client);
      PR_Close(s_server);
      flush_stream(&clientstream);
      flush_stream(&serverstream);
      /* Connection is closed, so reset the current cipher */
      currentcipher = 0;
      /* Reset MAC size */
      hMACsize = 0;

      c_count++;
      PR_fprintf(PR_STDERR,"Connection %d Complete [%s]\n", c_count,
                            get_time_string() );
    }  while (looparound); /* accept connection and process it. */
    PR_Close(s_rend);
    NSS_Shutdown();
    return 0;
}
