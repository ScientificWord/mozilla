//
// DATEI:         MCO_types.h
// AUTOR:         Torsten Metzner
// ERSTELLT:      14/08/98
// GEAENDERT:     18/08/98
// INHALT:        Festlegung einiger elementarer Datentypen die innerhalb des 
//                MCodes benötigt werden.
//
// Diese Headerdatei kann derzeit unter folgenden Betriebssystemen eingesetzt
// werden (hier wurde es getestet):
//    SPARC/Solaris 2.5 , PC/Linux 2.0
//

#ifndef __MCO_types__
#define __MCO_types__

#include "MCO_compat.h"
#include "MCO_encodings.h"

//     
// Klasse MTinternalHeader -- Basisklasse für MTmcodeHeader und MTdataHeader 
//    Hier werden einige gemeinsame Konstanten definiert                        
// 
class MTinternalHeader
{     
protected:  
   static const unsigned short MC_mcode_bit_ ;
   static const unsigned short MC_nomcode_bit_ ;
} ;


//
// Klasse MTmcodeHeader -- Header eines MCode Objektes
//    Der Header enthält gewisse Informationen über die Kodierung des nächsten 
//    Objektes
//
class MTmcodeHeader : public MTinternalHeader
{
private: 
   unsigned short version_ ; 
                   // Enthält die Versionsnummer des MCodes, m.M. sollte es
                   // die Versionsnummer des MuPAD Kernes seien, der den Code
                   // erzeugt hat (hätte erzeugen können). Also z.B.:
                   // 141 oder 150 oder ....
   MTlongFormat longformat_ ;   // Übertragungsformat der Langzahlen
   MTfloatFormat floatformat_ ; // Übertragungsformat der Floatzahlen
   MTcoding coding_ ;           // Art der Codierung des MCodes
                                // Flach, mit Referenzen, Unique data ...
   unsigned char morebytes_ ;   // Anzahl der weiteren Bytes, die zum
                                // Header gehören. Als Erweiterungsmöglichkeit
                                // des Headers gedacht.
public:
   MTmcodeHeader() ;
   MTmcodeHeader( unsigned short vers, MTlongFormat whichl,  
                  MTfloatFormat whichf, MTcoding coding, unsigned char bytes ) ;
   void version( unsigned short vers ) ;
   void longformat( MTlongFormat which ) ;
   void floatformat( MTfloatFormat which ) ;
   void coding( MTcoding coding ) ;
   void morebytes( unsigned char bytes ) ;
   unsigned short version() const ;
   MTlongFormat longformat() const ;
   MTfloatFormat floatformat() const ;
   MTcoding coding() const ;
   bool isflatCoding() const ;
   bool isreferenceCoding() const ;
   bool isidentCoding() const ;
   bool ispropertiesCoding() const ;
   unsigned char morebytes() const ;
} ;

//
// Klasse MTdataHeader -- Header eines Datums, das als MCode kodiert ist
//    
class MTdataHeader : public MTinternalHeader
{
private:
   friend class MTmcodeParser ;
    
   static const unsigned short MC_cat_bit_ ;
   static const unsigned short MC_nocat_bit_ ;
   unsigned short mmmtyp_ ;        
     // Im Falle von eines MMM_CATEGORY Typen, steht
     // hier der CAT_XXX Typ
     // Ist das 2 oberste Bit gesetzt, so handelt es sich um einen
     // CAT-Typen         
     // Bem.: Das oberste Bit wird schon für die Unterscheidung  
     //       MCode-Header <--> Daten-Header benutzt
   unsigned long size_ ;
   unsigned long sons_ ;
public:
   MTdataHeader() ;
   MTdataHeader( long ty, unsigned long size, unsigned long sons,
                 bool cat ) ;
   long type();
   bool cat() ;              // CAT-Typ ?
   unsigned long size() ;
   unsigned long sons() ;
   void type( long ty ) ;
   void cat( bool what ) ;  // false: MMM_XXX-Typ oder true: CAT_XXX-Typ
   void size( unsigned long size ) ;
   void sons( unsigned long sons ) ;
   void dataheader( long ty, unsigned long size, unsigned long sons,
                    bool cat ) ;
} ;

//---------------------------------------------------------------------------
// Implementation

//
// Klasse MTmcodeHeader -- Header eines MCode Objektes
//    Der Header enthält gewisse Informationen über die Kodierung des nächsten 
//    Objektes
//

//
// Konstruktoren
//
inline MTmcodeHeader::MTmcodeHeader() : version_(150u), longformat_(MCstringLong),
   floatformat_(MCieeeFloat), coding_(MCflatCoding|MCidentCoding), morebytes_(0)
{
}

inline MTmcodeHeader::MTmcodeHeader( unsigned short vers, MTlongFormat whichl, 
                                     MTfloatFormat whichf, MTcoding coding, 
                                     unsigned char bytes ) : version_(vers), 
   longformat_(whichl), floatformat_(whichf), coding_(coding), morebytes_(bytes)
{
}

//
// Setting information
//
inline void MTmcodeHeader::version( unsigned short vers ) { version_ = vers ; }

inline void MTmcodeHeader::longformat( MTlongFormat which ) { longformat_ = which; }

inline void MTmcodeHeader::floatformat( MTfloatFormat which ) {floatformat_=which;} 

inline void MTmcodeHeader::coding( MTcoding coding ) { coding_ = coding ; }

inline void MTmcodeHeader::morebytes( unsigned char bytes ) { morebytes_ = bytes ; }

//
// Getting information
//
inline unsigned short MTmcodeHeader::version() const { return version_ ; }

inline MTlongFormat MTmcodeHeader::longformat() const { return longformat_ ; }

inline MTfloatFormat MTmcodeHeader::floatformat() const { return floatformat_ ; }

inline MTcoding MTmcodeHeader::coding() const { return coding_ ; }

inline bool MTmcodeHeader::isflatCoding() const
{ return(  MCflatCoding == ( coding_ & MCflatCoding ) ) ; } 

inline bool MTmcodeHeader::isreferenceCoding() const
{ return(  MCreferenceCoding == ( coding_ & MCreferenceCoding ) ) ; } 

inline bool MTmcodeHeader::isidentCoding() const
{ return( MCidentCoding == ( coding_ & MCidentCoding ) ) ; } 

inline bool MTmcodeHeader::ispropertiesCoding() const
{ return( MCpropertiesCoding == ( coding_ & MCpropertiesCoding ) ) ; } 

inline unsigned char MTmcodeHeader::morebytes() const { return morebytes_ ; }

 
//
// Klasse MTdataHeader -- Header eines Datums, das als MCode kodiert ist
// 

//
// Konstruktoren
//
inline MTdataHeader::MTdataHeader() : mmmtyp_(0), size_(0), sons_(0) { } 
inline MTdataHeader::MTdataHeader( long ty, unsigned long size, unsigned long sons,
                                   bool cat ) : size_(size), sons_(sons)
{
   if ( cat )
      mmmtyp_ = (unsigned short ) ty | MC_cat_bit_ ;
   else
      mmmtyp_ = (unsigned short ) ty ;
}


//
// Getting information
//
inline long MTdataHeader::type() { return( (long) (mmmtyp_ & MC_nocat_bit_) ) ; } 
inline bool MTdataHeader::cat() { return( (bool) ( mmmtyp_ & MC_cat_bit_ ) ) ; } 
inline unsigned long MTdataHeader::size() { return size_ ; } 
inline unsigned long MTdataHeader::sons() { return sons_ ; } 

//
// Setting information
//
inline void MTdataHeader::type( long ty ) 
{
   if ( cat() )
      mmmtyp_ = (unsigned short ) ty | MC_cat_bit_ ;
   else
      mmmtyp_ = (unsigned short ) ty ;
}

inline void MTdataHeader::cat( bool what ) 
{ 
   if ( what ) 
      mmmtyp_ |= MC_cat_bit_ ;
   else
      mmmtyp_ &= MC_nocat_bit_ ;
}


inline void MTdataHeader::size( unsigned long size ) { size_ = size ; } 

inline void MTdataHeader::sons( unsigned long sons ) { sons_ = sons ; }

inline void MTdataHeader::dataheader( long ty, unsigned long size, unsigned long sons,
                                      bool cat ) 
{
   size_ = size ;
   sons_ = sons ;
   if ( cat )
      mmmtyp_ = (unsigned short ) ty | MC_cat_bit_ ;
   else
      mmmtyp_ = (unsigned short ) ty ;
}

#endif // __MCO_types__
