#ifndef msiKeyMap_h__
#define msiKeyMap_h__


#include "msiIKeyMap.h"
#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsHashKeys.h"
#include "nsClassHashtable.h"


struct keyStruct
{
  PRUint32 m_encodedInfo;
  keyStruct( PRUint32 virtualKey, PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey )
  { m_encodedInfo = virtualKey + (shiftKey?(1<<16):0) + (altKey?(1<<17):0) + (ctrlKey?(1<<18):0) + (metaKey?(1<<19):0);} 
};
  
struct nameHashPair
{
  nsString m_name;
  nsClassHashtable<nsUint32HashKey, nsString> m_table;
  nameHashPair * m_pNext;
  nameHashPair( const nsAString & name ): m_pNext(NULL) { m_name.Assign(name); m_table.Init(30);} 
  ~nameHashPair(){ delete m_pNext; }
};


class msiKeyMap : public msiIKeyMap
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MSIIKEYMAP

  msiKeyMap();

private:
  ~msiKeyMap();

protected:
  PRBool m_fDirty;
  nameHashPair * m_pnhp;
  /* additional members */
  PRUint32 VKeyStringToIndex( const nsString keyname);
  nsString virtKeyArray[225];
};


#endif