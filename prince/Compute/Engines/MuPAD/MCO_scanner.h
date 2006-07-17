//
// DATEI:         MCO_scanner.h
// AUTOR:         Torsten Metzner
// ERSTELLT:      26/08/98
// GEAENDERT:     24/11/98
// INHALT:        Scannerklassen für den MCode
//
// Hier wird eine abstrakte Netzwerk-Scanner Klasse deklariert. Darauf
// aufbauend wird eine Scannerklasse MTxdrScanner deklariert, die dazu 
// geeignet ist, auf einen BasicStream (MTstream) typisierte Daten im
// XDR-Format zu schreiben.
//
// Diese Headerdatei kann derzeit unter folgenden Betriebssystemen eingesetzt
// werden (hier wurde es getestet):
//    SPARC/Solaris 2.5 , PC/Linux 2.0
//

#ifndef __MCO_scanner__
#define __MCO_scanner__


#include "MKT_stream.h"     

#if defined MACINTOSH || defined DARWIN
#  include "xdr/mac_xdr.h"
#else
#ifdef WIN32
#  include "xdr/win_xdr.h"
#else
// TRUE and FALSE are redefined in rpc/types.h
#  ifdef TRUE
#    undef TRUE
#  endif
#  ifdef FALSE
#    undef FALSE
#  endif
#  include <rpc/types.h>
#  include <rpc/xdr.h>    
// Alternativ geht auch #include <rpc/rpc.h>
#endif
#endif

/** Class MTscanner
    Basic class which can write data in a "network" format on a stream of
    type MTstream
*/
class MTscanner
{
public:
   /// Scanner status: Ok or some error. Ok is 0
   enum Stat {
      /// Scanner status value Ok (no error)
      Ok    =  0 ,
      /// Scanner status value for unspecified errors. 
      Error = -1 ,
      /// Scanner status value if there was a read error on the stream
      Read  = -5 ,
      /// Scanner status value if there was a write error on the stream
      Write = -6 ,
      /// Scanner status value if it can't flush the tokens
      Flush = -7 
   } ;
   
   virtual Stat read(  char* buff, size_t& num_read )                     = 0 ;
   virtual Stat read(  char* buff, size_t num, size_t& num_read )         = 0 ;
   virtual Stat read(  unsigned char* buff, size_t num, size_t& num_read )= 0 ;
   virtual Stat read(  short* buff, size_t num, size_t& num_read )        = 0 ;
   virtual Stat read(  unsigned short* buff, size_t num, size_t& num_read )=0 ;
   virtual Stat read(  int* buff, size_t num, size_t& num_read )          = 0 ;
   virtual Stat read(  unsigned int* buff, size_t num, size_t& num_read ) = 0 ;
   virtual Stat read(  long* buff, size_t num, size_t& num_read )         = 0 ;
   virtual Stat read(  unsigned long* buff, size_t num, size_t& num_read )= 0 ;
   virtual Stat read(  float* buff, size_t num, size_t& num_read )        = 0 ;
   virtual Stat read(  double* buff, size_t num, size_t& num_read )       = 0 ;

   virtual Stat write(  const char* data, size_t& num_written ) = 0 ;  
   virtual Stat write(  const char* data, size_t num, 
                        size_t& num_written )  = 0 ;  
   virtual Stat write(  const unsigned char* data, size_t num, 
                        size_t& num_written )  = 0 ;
   virtual Stat write(  const short* data, size_t num, 
                        size_t& num_written )  = 0 ;
   virtual Stat write(  const unsigned short* data, size_t num, 
                        size_t& num_written)   = 0 ;
   virtual Stat write(  const int* data, size_t num, 
                        size_t& num_written )  = 0 ;
   virtual Stat write(  const unsigned int* data, size_t num, 
                        size_t& num_written )  = 0 ;
   virtual Stat write(  const long* data, size_t num, 
                        size_t& num_written )  = 0 ;
   virtual Stat write(  const unsigned long* data, size_t num, 
                        size_t& num_written )  = 0 ;
   virtual Stat write(  const float* data, size_t num, 
                        size_t& num_written )  = 0 ;
   virtual Stat write(  const double* data, size_t num, 
                        size_t& num_written )  = 0 ;

   virtual Stat flush() = 0 ;
} ;

//
// Klasse MTxdrScanner -- Klasse die auf jeden Basisstream (MTstream)
//    Daten in einem xdr-Format schreiben und lesen kann.
//
class MTxdrScanner : public MTscanner
{
public:    
   Stat status_ ;
private:
   XDR xdr_ ;        
   long mcode_version_ ;
#if (defined linux || defined SOL25 || defined WIN32 || defined MACINTOSH || defined DARWIN || defined __OpenBSD__)
   struct XDR::xdr_ops xdr_ops_ ;    
#else
   // Für Solaris2.6 muß es wie folgt heißen:
   struct xdr_ops xdr_ops_ ;
#endif
public:
   MTstream *str_ ;
public:
   // Bem.: Weitere Fehlercodes ergeben sich erst aus der Implementation
   MTxdrScanner( long mcode_version=2 ) ;
   MTxdrScanner( MTstream *stre, long mcode_version=2 ) ;
   virtual ~MTxdrScanner() ;

   void stream( MTstream *stre ) ;
   Stat read(  void* buff, size_t buff_len, size_t& num_read ) ;
   Stat read(  char* buff, size_t num, size_t& num_read ) ;    
   Stat read(  char* buff, size_t& num_read ) ;
   Stat read(  unsigned char* buff, size_t num, size_t& num_read ) ;
   Stat read(  short* buff, size_t num, size_t& num_read ) ;
   Stat read(  unsigned short* buff, size_t num, size_t& num_read ) ;
   Stat read(  int* buff, size_t num, size_t& num_read ) ;
   Stat read(  unsigned int* buff, size_t num, size_t& num_read ) ;
   Stat read(  long* buff, size_t num, size_t& num_read ) ;
   Stat read(  unsigned long* buff, size_t num, size_t& num_read ) ;
   Stat read(  float* buff, size_t num, size_t& num_read ) ;
   Stat read(  double* buff, size_t num, size_t& num_read ) ;
   
   Stat write(  const void* data, size_t num_bytes, 
			  size_t& num_written ) ;
   Stat write(  const char* data, size_t& num_written ) ;  
   Stat write(  const char* data, size_t num, size_t& num_written ) ;
   Stat write(  const unsigned char* data, size_t num, 
			  size_t& num_written ) ;
   Stat write(  const short* data, size_t num, size_t& num_written ) ;
   Stat write(  const unsigned short* data, size_t num, 
			  size_t& num_written ) ;
   Stat write(  const int* data, size_t num, size_t& num_written ) ;
   Stat write(  const unsigned int* data, size_t num, 
			  size_t& num_written ) ;
   Stat write(  const long* data, size_t num, size_t& num_written ) ;
   Stat write(  const unsigned long* data, size_t num, 
			  size_t& num_written ) ;
   Stat write(  const float* data, size_t num, size_t& num_written ) ;
   Stat write(  const double* data, size_t num, 
			  size_t& num_written ) ;

   // Und nun noch einige weiteren Funktionen 
   Stat flush() ;
   Stat status() ;
   const char* status_text() ;

protected:
   Stat status( Stat ) ;

} ;

#endif // __MCO_scanner__



