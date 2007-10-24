#ifndef msiKeyMap_h__
#define msiKeyMap_h__


#include "msiIKeyMap.h"
#include "nsStringAPI.h"
#include "nsCOMPtr.h"
#include "nsHashKeys.h"
#include "nsClassHashtable.h"
#include "nsTArray.h"


struct keyStruct
{
  PRUint32 m_encodedInfo;
  keyStruct( PRUint32 virtualKey, PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRBool vk )
  { m_encodedInfo = (virtualKey<<16) +(shiftKey?1:0)+(altKey?(1<<1):0)+(ctrlKey?(1<<2):0)+(metaKey?(1<<3):0)+(vk?(1<<4):0);}
  keyStruct( PRUint32 encodedInfo ){ m_encodedInfo=encodedInfo;}
  void ExtractInfo ( PRUint32 *virtualKey, PRBool *altKey, PRBool *ctrlKey, PRBool *shiftKey, PRBool *metaKey, PRBool *vk );
};
  
struct nameHashPair
{
  nsString m_name;
  PRBool m_fScript; // true if keys map to JavaScript; false if they map to keys
  nsClassHashtable<nsUint32HashKey, nsString> m_table;
  nameHashPair * m_pNext;
  nameHashPair( const nsAString & name, PRBool fScript ): m_pNext(NULL), m_fScript(fScript) { m_name.Assign(name); m_table.Init(30);} 
  ~nameHashPair(){ delete m_pNext; }
};

struct keyNameAndCode
{
  nsString keyName;
  PRUint32 keyCode;
};


// {0176db9d-bae7-4b42-82af-6da0269bdd2f}
#define MSI_KEYMAP_CID \
{ 0x0176db9d, 0xbae7, 0x4b42, { 0x82, 0xaf, 0x6d, 0xa0, 0x26, 0x9b, 0xdd, 0x2f }}

#define MSI_KEYMAP_CONTRACTID \
 "@mackichan.com/keymap/keymap_service;1"


class msiKeyMap : public msiIKeyMap
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MSIIKEYMAP

  msiKeyMap();
static  msiKeyMap* GetInstance();
static  void       ReleaseInstance();
static msiKeyMap   *sInstance;
  
PRUint32 VKeyStringToIndex( const nsString keyname);
nsString VKeyIndexToString ( PRUint32 index );
void keyArrayToString( nsTArray<PRUint32>& ka, nsString& sFile, nsClassHashtable<nsUint32HashKey, nsString>& table);
keyNameAndCode virtKeyArray[80];

private:
  ~msiKeyMap();

protected:
  PRBool m_fDirty;
  PRBool m_fFileLoaded;
  nameHashPair * m_pnhp;
  /* additional members */
};


#endif
