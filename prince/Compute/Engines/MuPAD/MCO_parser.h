//
// DATEI:         MCO_parser.h
// AUTOR:         Torsten Metzner
// ERSTELLT:      26/08/98
// GEAENDERT:     24/11/98
// INHALT:        Basisklasse für einen MCode Parser
//
// In dieser Datei wird eine Basisklasse MTmcodeParser deklariert, die
// zum grundlegenden parsen eines MCodeBytestreams, also eines Bytestreams
// auf dem MCode liegt geeignet ist. 
//
// Diese Headerdatei kann derzeit unter folgenden Betriebssystemen eingesetzt
// werden (hier wurde es getestet):
//    SPARC/Solaris 2.5 , PC/Linux 2.0
//

#ifndef __MCO_parser__
#define __MCO_parser__

#include "MCO_scanner.h"
#include "MCO_types.h"
 
//#define MCODEBUG
// Zum Debuggen
#ifdef MCODEBUG
void MCOerror( char *str ) ;
extern int DEBUGCOUNTER ;
#endif

//
// Klasse MTmcodeParser -- Grundlegender MCode Parser
//
class MTmcodeParser 
{
public:
   enum Stat { 
   Ok = 0 , 
   Error            , //  1
   Build            , //  2
   Unknown          , //  3
   UnknownCat       , //  4
   WriteDataHeader  , //  5 
   WriteMcodeHeader , //  6
   ReadDataHeader   , //  7
   ReadMcodeHeader  , //  8
   WriteToken       , //  9
   ReadToken          // 10
   } ;

protected: 
   MTmcodeHeader mcodeheader_ ;
   MTdataHeader  dataheader_ ;
private:
   bool mcodehead_ ;
   static const char *Messages_[11]   ;
   Stat status_ ;
   
   Stat mcodheader_() ;
   Stat datheader_() ;
   Stat header_() ;
protected:
   MTscanner *scan_ ;           // Dieser Scanner soll benutzt werden

public:
   
   //  CAT_REF ist KEINE MuPAD-Kern-Kategorie
   // Es ist ein Objekt, welches anzeigt das beim MCode eine Referenz vorliegt.
   static const unsigned short CAT_REF ; // Wert 201
   //  CAT_MMMNULL ist KEINE MuPAD-Kern-Kategorie
   // Es ist ein Objekt, welches anzeigt das beim MCode ein MMMNULL
   // übertragen wurde.
   static const unsigned short CAT_MMMNULL ; // Wert 202

   MTmcodeParser() ;
   MTmcodeParser( MTscanner *scan ) ;
   // Initialize this parser from another parser using the same scanner
   MTmcodeParser( MTmcodeParser *parser) ;
   virtual ~MTmcodeParser() ;
   
   virtual void scanner( MTscanner *scan ) ; // Verbindet den Parser mit einem
                         // Stream. S.a. MTmcodeParser( MTscanner *scan ) ;
    
   //virtual Stat skip_data() ; 

   virtual const char* status_text() ;
   Stat status() ;
   Stat status( Stat stat) ;
   
   virtual bool test_and_send_reference( const void *object ) ; 
   virtual void remember_read( const void *object ) ;
   virtual void remember_write( const void *object ) ;
   
   virtual Stat parse(void *object) ;

   // Bem.: buildgeneric() ist eine Dummy Implementation, die 
   // nichts weiteres tut als eine Fehlermeldung auszugeben und den
   // Fehlerwert MC_CANTBUILD zurückliefert und das übergebene Objekt
   // auf den NULL-Pointer setzt.

   // Demnachst könnte man sich auch folgendes Verhalten vorstellen.
   //       buildgeneric() ist eine Dummy Implementation, die
   // nichts weiteres tut als die Daten zu verwerfen und
   // den NULL Pointer zur"uckliefert. Dies ist die Voreinstellung
   // jeder weiteren build-Fkt, sofern diese nicht reimplementiert wird.
   
   virtual Stat buildgeneric( void *object ) ;
   virtual Stat buildHeader( const MTmcodeHeader *head ) ;
   
   //
   // Fkt. für die CAT-Typen
   //
   
   virtual Stat buildCatString(   void *object, unsigned long special   ) ; 
   virtual Stat buildCatStatList( void *spointer, unsigned long sons    ) ;
   virtual Stat buildCatIdent(    void *spointer, unsigned long special ) ;
   virtual Stat buildCatInt(      void *spointer, unsigned long special ) ;
   virtual Stat buildCatApm(      void *spointer, unsigned long special ) ;
   virtual Stat buildCatFloat(    void *spointer, unsigned long special ) ;
   virtual Stat buildCatRat(      void *spointer, unsigned long special ) ;
   virtual Stat buildCatComplex(  void *spointer, unsigned long special ) ;
   virtual Stat buildCatInterval( void *spointer, unsigned long special ) ;
#ifndef GRAPHIC
   virtual Stat buildCatFrame(    void *spointer, unsigned long special ) ;
   virtual Stat buildCatExec(     void *spointer, unsigned long sons ) ;
   virtual Stat buildCatProc(     void *spointer, unsigned long sons ) ;
   virtual Stat buildCatDom(      void *spointer, unsigned long special ) ;
   virtual Stat buildCatPoly(     void *spointer, unsigned long special ) ;
   virtual Stat buildCatRPoly(    void *spointer, unsigned long special ) ;
   virtual Stat buildCatExt(      void *spointer, unsigned long sons    ) ;
   virtual Stat buildCatExt2(     void *spointer, unsigned long special ) ;
   virtual Stat buildCatTable(    void *spointer, unsigned long special ) ;
   virtual Stat buildCatSetFinite(void *spointer, unsigned long special ) ;
   virtual Stat buildCatProcEnv(  void *spointer, unsigned long sons ) ;
   virtual Stat buildCatFuncEnv(  void *spointer, unsigned long sons ) ;
   virtual Stat buildCatArray(    void *spointer, unsigned long special ) ;
#endif
   virtual Stat buildCatBool(     void *spointer, unsigned long special ) ;
   virtual Stat buildCatNull(     void *spointer, unsigned long special ) ;
   virtual Stat buildCatNil(      void *spointer, unsigned long special ) ;
   virtual Stat buildCatFailed(   void *spointer, unsigned long special ) ;
   virtual Stat buildCatMmmnull(  void *spointer, unsigned long special ) ;
   virtual Stat buildCatExpr(     void *spointer, unsigned long sons ) ;
   virtual Stat buildCatFuncarg(  void *spointer, unsigned long special ) ;
   virtual Stat buildCatPoint(    void *spointer, unsigned long special ) ;
   virtual Stat buildCatPolygon(  void *spointer, unsigned long special ) ;
   virtual Stat buildCatRef(      void *spointer, unsigned long special ) ;
   virtual Stat buildCatVar(      void *spointer, unsigned long special ) ;
    
   // virtual Stat buildCat( void *object ) ; 

   Stat unparse( const MTmcodeHeader *mcodeheader ) ;
   Stat unparse( const void* object, const MTdataHeader *dataheader ) ;
 
  // Bem.: writegeneric() ist eine Dummy Implementation, die 
   // garnichts tut. Dies ist die Voreinstellung
   // jeder weiteren write-Fkt, sofern diese nicht reimplementiert wird.
   
   virtual Stat writegeneric( const void *object ) ;   //

   //
   // Fkt. für die CAT-Typen
   //
   virtual Stat writeCatString(  const void *object, unsigned long special   ) ;
   virtual Stat writeCatStatList(const void *spointer, unsigned long sons    ) ;
#ifndef GRAPHIC
   virtual Stat writeCatIdent(   const void *spointer, unsigned long special ) ;
   virtual Stat writeCatFrame(   const void *spointer, unsigned long special ) ;
   virtual Stat writeCatInt(     const void *spointer, unsigned long special ) ;
   virtual Stat writeCatApm(     const void *spointer, unsigned long special ) ;
   virtual Stat writeCatFloat(   const void *spointer, unsigned long special ) ;
   virtual Stat writeCatRat(     const void *spointer, unsigned long special ) ;
   virtual Stat writeCatComplex( const void *spointer, unsigned long special ) ;
   virtual Stat writeCatInterval(const void *spointer, unsigned long special ) ;
   virtual Stat writeCatExec(    const void *spointer, unsigned long special ) ;
   virtual Stat writeCatProc(    const void *spointer, unsigned long special ) ;
   virtual Stat writeCatDom(     const void *spointer, unsigned long special ) ;
   virtual Stat writeCatPoly(    const void *spointer, unsigned long special ) ;
   virtual Stat writeCatRPoly(   const void *spointer, unsigned long special ) ;
   virtual Stat writeCatExt(     const void *spointer, unsigned long sons    ) ;
   virtual Stat writeCatExt2(    const void *spointer, unsigned long special ) ;
   virtual Stat writeCatTable(   const void *spointer, unsigned long special ) ;
   virtual Stat writeCatSetFinite(const void *spointer, unsigned long special) ;
   virtual Stat writeCatProcEnv( const void *spointer, unsigned long special ) ;
   virtual Stat writeCatFuncEnv( const void *spointer, unsigned long special ) ;
   virtual Stat writeCatArray(   const void *spointer, unsigned long special ) ;
#endif
   virtual Stat writeCatBool(    const void *spointer, unsigned long special ) ;
   virtual Stat writeCatNull(    const void *spointer, unsigned long special ) ;
   virtual Stat writeCatNil(     const void *spointer, unsigned long special ) ;
   virtual Stat writeCatFailed(  const void *spointer, unsigned long special ) ;
   virtual Stat writeCatMmmnull( const void *spointer, unsigned long special ) ;
   virtual Stat writeCatExpr(    const void *spointer, unsigned long special ) ;
   virtual Stat writeCatFuncarg( const void *spointer, unsigned long special ) ;
   virtual Stat writeCatPoint(   const void *spointer, unsigned long special ) ;
   virtual Stat writeCatPolygon( const void *spointer, unsigned long special ) ;
   virtual Stat writeCatRef(     const void *spointer, unsigned long special ) ;
   virtual Stat writeCatVar(     const void *spointer, unsigned long special ) ;

   ///Fkt. für die MMM-Typen
#ifndef GRAPHIC
   virtual Stat buildMmmProcEnv( void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmDistPoly(void *spointer, unsigned long sons ) ;
#endif

   virtual Stat buildMmmString(  void *spointer, unsigned long special ) ;

   virtual Stat buildMmmGraphic(  void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmScene3D(  void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmObject3D( void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmDefaults( void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmPointlist(void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmGrlist(   void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmPoint3D(  void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmLine(     void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmTriangle( void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmRgbColor( void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmGraphicCommand( void *spointer, unsigned long sons ) ;
   virtual Stat buildMmmPolygon(  void *spointer, unsigned long sons ) ;
 
#ifndef GRAPHIC
   virtual Stat writeMmmProcEnv( const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmDistPoly(const void *spointer, unsigned long special ) ;
#endif
 
   virtual Stat writeMmmString( const void *spointer, unsigned long special ) ;

   virtual Stat writeMmmGraphic( const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmScene3D( const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmObject3D(const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmDefaults(const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmPointlist(const void *spointer, unsigned long special) ;
   virtual Stat writeMmmGrlist(  const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmPoint3D( const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmLine(    const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmTriangle(const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmRgbColor(const void *spointer, unsigned long special ) ;
   virtual Stat writeMmmGraphicCommand( const void *spointer, 
                                        unsigned long sons ) ;
   virtual Stat writeMmmPolygon( const void *spointer, unsigned long special ) ;
 
} ;

//---------------------------------------------------------------------------
// Implementation

inline MTmcodeParser::Stat  MTmcodeParser::datheader_() 
{                  
   size_t howmuch ;  
   MTscanner::Stat stat ;
   
   scan_->write( &dataheader_.mmmtyp_, 1, howmuch ) ; 
   scan_->write( &dataheader_.size_, 1, howmuch ) ; 
   stat = scan_->write( &dataheader_.sons_, 1, howmuch ) ;
   scan_->flush() ;
   if ( stat == MTscanner::Ok )
      return( Ok ) ;
   else
      return( WriteDataHeader ) ;
}

#endif // __MCO_parser__
