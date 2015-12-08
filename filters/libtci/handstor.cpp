
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "chmtypes.h"
#include "TCI_new.h"
#include "handstor.h"

/* 
  HandleStore - a container object which associates handles and data pointers.
  
  You can think of the store as an array of pointers to data,
  and the handles as indexes into this array.             

  Objects in a HandleStore are used frequently, and many calls
  are typically made for new handles.  Fast access is required.
  Thus we assume that every object associated with a handle
  should be locked.

  Typical uses of HandleStores are by paragraphs, which need
  handles to span and atom records.
*/

/* This implementation uses a dynamic array implemented by allocating
   additional groups of slots as needed. BASESIZE slots are kept directly
   in the object.  If more slots are needed they are allocated from the
   heap in groups of GRPSIZE.  These additional groups of slots are linked
   into a list.  Hopefully we can tune BASESIZE and GRPSIZE to achieve
   efficiency.
   There is no provision for locking or compacting in the present scheme.
   Also, long pointers are stored and pasted out to clients so the data
   pointed to MUST BE STORED IN FIXED MEMORY.

   Containers should have their own allocators
*/


class ptrgroup {
public:
  ptrgroup()    {}
  void* pg[GRPSIZE+1];
};


HandleStore::HandleStore() {

  for ( int i=0; i<=BASESIZE; i++ )
    handles[i]  =  (void*)0;

  hndsptr       =  (void**)0;
  highwatermark =  0;
  numhandlesout =  0;
}


HandleStore::~HandleStore() {

  for ( int i=1; i <= BASESIZE; i++ )
    if ( handles[i] != (void*)0 ) UserDelete( handles[i] );

  void** groupptr =  hndsptr;
  while ( groupptr != (void**)0 ) {
    for ( int i=0; i<GRPSIZE; i++ )
      if ( groupptr[i] != (void*)0 ) UserDelete( groupptr[i] );
    hndsptr     =  groupptr;
    groupptr    =  (void**)groupptr[GRPSIZE];
    delete hndsptr;
  }   // while loop thru ptr blocks

}


int HandleStore::New(int oldhandle) {

// First we allocate the memory to store the new object

  if (oldhandle) {
    void* temp =  this->P(oldhandle);
    if ( temp != (void*)0 )
      UserDup(temp);
    else
      UserNew();
  } else
    UserNew();          // Does a new for *data

// How do we know UserNew or UserDup worked?  Check Glockenspiel containers.

  numhandlesout++;

// Find a handle( ie. slot ), then stick the pointer data into it.

  for ( int i=1; i<=BASESIZE; i++)
    if ( handles[i] == (void*)0 ) {
      handles[i]        =  data;
      if ( i > highwatermark ) {
        highwatermark =  i;
      }
      return i;
    }

  void** groupptr =  hndsptr;
  int starti =  BASESIZE + 1;
  while ( groupptr != (void*)0 ) {
    for ( int i=0; i<GRPSIZE; i++ )
      if ( groupptr[i] == (void*)0 ) {
        groupptr[i]     =  data;
        if ( i+starti > highwatermark ) {
          highwatermark =  i+starti;
        }
        return  i+starti;
      }
    groupptr    =  (void**) groupptr[GRPSIZE];
    starti      +=  GRPSIZE;
  }   // while loop thru ptr blocks

// If we get to here, there are no empty slots

  ptrgroup* newptr;
  newptr =  TCI_NEW(ptrgroup);

  if (newptr != (ptrgroup*)0) {
    (newptr->pg)[0]     =  data;
    for ( int i=1; i<=GRPSIZE; i++ )
      (newptr->pg)[i] =  (void*)0;

    if ( hndsptr == (void**)0 )
      hndsptr =  (void**) newptr;
    else {
      void** tempptr =  hndsptr;
      while ( tempptr != (void**)0 )
        if ( tempptr[GRPSIZE] == (void*)0 ) {
          tempptr[GRPSIZE] =  (void*)newptr;
          break;
        } else
          tempptr =  (void**)tempptr[GRPSIZE];
    }
    highwatermark = starti;
    return starti;

  } else {
    UserDelete( data ); 
    numhandlesout--;
    return 0;
  }
}


void HandleStore::Delete( int handle ) {

  if ( handle <= BASESIZE ) {
    if ( handles[handle] != (void*)0 ) {
      UserDelete( handles[handle] );    //How do we know UserDelete worked?
      handles[handle]   =  (void*)0;
      numhandlesout--;
      return;
    }
  } else {
    void** groupptr =  hndsptr;
    int limit   =  BASESIZE + GRPSIZE;
    while ( groupptr != (void**)0 ) {
      if ( handle <= limit ) {
        int i   =  handle + GRPSIZE - limit -1;
        if ( groupptr[i] != (void*)0 ) {
          UserDelete( groupptr[i] );
          groupptr[i]   =  (void*)0;
          numhandlesout--;
          return;
        } else
          break;
      } else {
        groupptr        =  (void**)groupptr[GRPSIZE];
        limit   +=  GRPSIZE;
      }
    }   // while loop thru ptr blocks
  }

}


void* HandleStore::P(int handle) const {

  if ( handle <= BASESIZE ) {
    if ( handles[handle] != (void*)0 )
      return( handles[handle] );

  } else {
    void** groupptr =  hndsptr;
    int limit   =  BASESIZE + GRPSIZE;
    while ( groupptr != (void**)0 ) {
      if ( handle <= limit ) {
        int i   =  handle + GRPSIZE - limit -1;
        if ( groupptr[i] != (void*)0 )  return groupptr[i];
        else  break;
      } else {
        groupptr        =  (void**)groupptr[GRPSIZE];
        limit   +=  GRPSIZE;
      }
    }   // while loop thru ptr blocks

  }

  return (void*)0;  //How do we avoid this?  Should there be a
                       //class  static for each derived class ?
}


int HandleStore::Count() {

  return numhandlesout;
}


int HandleStore::GetNext( int currhandle ) {

  int mark  =  currhandle+1;
  for ( int i=mark; i<=BASESIZE; i++ )
    if ( handles[i] != (void*)0 )
      return i;

  if (mark < BASESIZE+1) mark = BASESIZE+1;

  void** groupptr = hndsptr;
  int starti  =  BASESIZE+1;
  while ( groupptr != (void*)0 ) {
    for ( int j=mark; j<starti+GRPSIZE; j++ ) {
      if ( groupptr[j-starti] != (void*)0 )
        return j;
    }
    groupptr    =  (void**) groupptr[GRPSIZE];
    starti  +=  GRPSIZE;
    if (mark < starti) mark = starti; 
  }   // while loop thru ptr blocks

  return 0;
}



void HandleStore::UserNew(){
  //Leave this function empty
}


void HandleStore::UserDup( void* ){
  //Leave this function empty
}


void HandleStore::UserDelete( void* ){
  //Leave this function empty
}

