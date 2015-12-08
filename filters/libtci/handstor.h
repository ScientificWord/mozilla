#ifndef HANDSTOR_H
#define HANDSTOR_H

/* 
  HandleStore - Instances of Classes derived from this class  are containers
		for "heap" objects.
  
  You can think of the store as an array of pointers to data,
  and handles as indexes into this array.             

  Objects in a HandleStore are used frequently, and many calls
  are typically made for new handles.  Fast access is required.
  With this in mind, we assume for the nonce that every object 
  associated with a handle is locked throughout its existence.

  Typical uses of HandleStores are by paragraphs, which need
  handles to span and atom records.

  The constructor of this class  is private - all class es which
  are actually used must be derived from this one.
*/


class HandleStoreTest;

class HandleStore {
public:
  virtual ~HandleStore();

  virtual int     New(int = 0);
  virtual void    Delete( int );
  
          int     Count();
          int     GetNext(int);

protected:
  HandleStore();
  virtual void    UserNew();
  virtual void    UserDup( void* );
  virtual void    UserDelete( void* );
          void*   P(int) const;

  void* data;

private:
#  define BASESIZE 10
  void* handles[BASESIZE+1];           //Handle 0 is invalid

#  define GRPSIZE 40
  void** hndsptr;

private:
  int highwatermark;
  int numhandlesout;
  friend class  HandleStoreTest;
};
#endif
