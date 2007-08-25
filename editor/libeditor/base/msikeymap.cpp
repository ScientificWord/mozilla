#include "msiKeyMap.h"
#include "nsIDOMElement.h"
#include "nsIDOM3Node.h"
#include "nsIDOMDocument.h"
#include "nsIDOMXMLDocument.h"
#include "nsIDOMNodeList.h"
#include "nsContentCID.h"
#include "nsComponentManagerUtils.h"

/* Implementation file */
NS_IMPL_ISUPPORTS1(msiKeyMap, msiIKeyMap)

msiKeyMap::msiKeyMap()
{
  m_fDirty = PR_FALSE;
  m_pnhp = NULL;
  nsString x[] = 
    {  EmptyString(),  // 0x00
    EmptyString(),  // 0x01
    EmptyString(),  // 0x02
    NS_LITERAL_STRING("CANCEL"),  // 0x03;
    EmptyString(),  // 0x04
    EmptyString(),  // 0x05
    NS_LITERAL_STRING("HELP"),  // 0x06;
    EmptyString(),  // 0x07
    NS_LITERAL_STRING("BACK_SPACE"),  // 0x08;
    NS_LITERAL_STRING("TAB"),  // 0x09;
    EmptyString(),  // 0x0A
    EmptyString(),  // 0x0B
    NS_LITERAL_STRING("CLEAR"),  // 0x0C;
    NS_LITERAL_STRING("RETURN"),  // 0x0D;
    NS_LITERAL_STRING("ENTER"),  // 0x0E;
    EmptyString(),  // 0x0F
    NS_LITERAL_STRING("SHIFT"),  // 0x10;
    NS_LITERAL_STRING("CONTROL"),  // 0x11;
    NS_LITERAL_STRING("ALT"),  // 0x12;
    NS_LITERAL_STRING("PAUSE"),  // 0x13;
    NS_LITERAL_STRING("CAPS_LOCK"),  // 0x14;
    EmptyString(),  // 0x15
    EmptyString(),  // 0x16
    EmptyString(),  // 0x17
    EmptyString(),  // 0x18
    EmptyString(),  // 0x19
    EmptyString(),  // 0x1A
    NS_LITERAL_STRING("ESCAPE"),  // 0x1B;
    EmptyString(),  // 0x1C
    EmptyString(),  // 0x1D
    EmptyString(),  // 0x1E
    EmptyString(),  // 0x1F
    NS_LITERAL_STRING("SPACE"),  // 0x20;
    NS_LITERAL_STRING("PAGE_UP"),  // 0x21;
    NS_LITERAL_STRING("PAGE_DOWN"),  // 0x22;
    NS_LITERAL_STRING("END"),  // 0x23;
    NS_LITERAL_STRING("HOME"),  // 0x24;
    NS_LITERAL_STRING("LEFT"),  // 0x25;
    NS_LITERAL_STRING("UP"),  // 0x26;
    NS_LITERAL_STRING("RIGHT"),  // 0x27;
    NS_LITERAL_STRING("DOWN"),  // 0x28;
    EmptyString(),  // 0x29
    EmptyString(),  // 0x2A
    EmptyString(),  // 0x2B
    NS_LITERAL_STRING("PRINTSCREEN"),  // 0x2C;
    NS_LITERAL_STRING("INSERT"),  // 0x2D;
    NS_LITERAL_STRING("DELETE"),  // 0x2E;
    EmptyString(),  // 0x2F
    NS_LITERAL_STRING("0"),  // 0x30;
    NS_LITERAL_STRING("1"),  // 0x31;
    NS_LITERAL_STRING("2"),  // 0x32;
    NS_LITERAL_STRING("3"),  // 0x33;
    NS_LITERAL_STRING("4"),  // 0x34;
    NS_LITERAL_STRING("5"),  // 0x35;
    NS_LITERAL_STRING("6"),  // 0x36;
    NS_LITERAL_STRING("7"),  // 0x37;
    NS_LITERAL_STRING("8"),  // 0x38;
    NS_LITERAL_STRING("9"),  // 0x39;
    EmptyString(),  // 0x3A
    NS_LITERAL_STRING("SEMICOLON"),  // 0x3B;
    EmptyString(),  // 0x3C
    NS_LITERAL_STRING("EQUALS"),  // 0x3D;
    EmptyString(),  // 0x3E
    EmptyString(),  // 0x3F
    EmptyString(),  // 0x40
    NS_LITERAL_STRING("A"),  // 0x41;
    NS_LITERAL_STRING("B"),  // 0x42;
    NS_LITERAL_STRING("C"),  // 0x43;
    NS_LITERAL_STRING("D"),  // 0x44;
    NS_LITERAL_STRING("E"),  // 0x45;
    NS_LITERAL_STRING("F"),  // 0x46;
    NS_LITERAL_STRING("G"),  // 0x47;
    NS_LITERAL_STRING("H"),  // 0x48;
    NS_LITERAL_STRING("I"),  // 0x49;
    NS_LITERAL_STRING("J"),  // 0x4A;
    NS_LITERAL_STRING("K"),  // 0x4B;
    NS_LITERAL_STRING("L"),  // 0x4C;
    NS_LITERAL_STRING("M"),  // 0x4D;
    NS_LITERAL_STRING("N"),  // 0x4E;
    NS_LITERAL_STRING("O"),  // 0x4F;
    NS_LITERAL_STRING("P"),  // 0x50;
    NS_LITERAL_STRING("Q"),  // 0x51;
    NS_LITERAL_STRING("R"),  // 0x52;
    NS_LITERAL_STRING("S"),  // 0x53;
    NS_LITERAL_STRING("T"),  // 0x54;
    NS_LITERAL_STRING("U"),  // 0x55;
    NS_LITERAL_STRING("V"),  // 0x56;
    NS_LITERAL_STRING("W"),  // 0x57;
    NS_LITERAL_STRING("X"),  // 0x58;
    NS_LITERAL_STRING("Y"),  // 0x59;
    NS_LITERAL_STRING("Z"),  // 0x5A;
    EmptyString(),  // 0x5B
    EmptyString(),  // 0x5C
    NS_LITERAL_STRING("CONTEXT_MENU"),  // 0x5D;
    EmptyString(),  // 0x5E
    EmptyString(),  // 0x5F
    NS_LITERAL_STRING("NUMPAD0"),  // 0x60;
    NS_LITERAL_STRING("NUMPAD1"),  // 0x61;
    NS_LITERAL_STRING("NUMPAD2"),  // 0x62;
    NS_LITERAL_STRING("NUMPAD3"),  // 0x63;
    NS_LITERAL_STRING("NUMPAD4"),  // 0x64;
    NS_LITERAL_STRING("NUMPAD5"),  // 0x65;
    NS_LITERAL_STRING("NUMPAD6"),  // 0x66;
    NS_LITERAL_STRING("NUMPAD7"),  // 0x67;
    NS_LITERAL_STRING("NUMPAD8"),  // 0x68;
    NS_LITERAL_STRING("NUMPAD9"),  // 0x69;
    NS_LITERAL_STRING("MULTIPLY"),  // 0x6A;
    NS_LITERAL_STRING("ADD"),  // 0x6B;
    NS_LITERAL_STRING("SEPARATOR"),  // 0x6C;
    NS_LITERAL_STRING("SUBTRACT"),  // 0x6D;
    NS_LITERAL_STRING("DECIMAL"),  // 0x6E;
    NS_LITERAL_STRING("DIVIDE"),  // 0x6F;
    NS_LITERAL_STRING("F1"),  // 0x70;
    NS_LITERAL_STRING("F2"),  // 0x71;
    NS_LITERAL_STRING("F3"),  // 0x72;
    NS_LITERAL_STRING("F4"),  // 0x73;
    NS_LITERAL_STRING("F5"),  // 0x74;
    NS_LITERAL_STRING("F6"),  // 0x75;
    NS_LITERAL_STRING("F7"),  // 0x76;
    NS_LITERAL_STRING("F8"),  // 0x77;
    NS_LITERAL_STRING("F9"),  // 0x78;
    NS_LITERAL_STRING("F10"),  // 0x79;
    NS_LITERAL_STRING("F11"),  // 0x7A;
    NS_LITERAL_STRING("F12"),  // 0x7B;
    NS_LITERAL_STRING("F13"),  // 0x7C;
    NS_LITERAL_STRING("F14"),  // 0x7D;
    NS_LITERAL_STRING("F15"),  // 0x7E;
    NS_LITERAL_STRING("F16"),  // 0x7F;
    NS_LITERAL_STRING("F17"),  // 0x80;
    NS_LITERAL_STRING("F18"),  // 0x81;
    NS_LITERAL_STRING("F19"),  // 0x82;
    NS_LITERAL_STRING("F20"),  // 0x83;
    NS_LITERAL_STRING("F21"),  // 0x84;
    NS_LITERAL_STRING("F22"),  // 0x85;
    NS_LITERAL_STRING("F23"),  // 0x86;
    NS_LITERAL_STRING("F24"),  // 0x87;
    EmptyString(),  // 0x88
    EmptyString(),  // 0x89
    EmptyString(),  // 0x8A
    EmptyString(),  // 0x8B
    EmptyString(),  // 0x8C
    EmptyString(),  // 0x8D
    EmptyString(),  // 0x8E
    EmptyString(),  // 0x8F
    NS_LITERAL_STRING("NUM_LOCK"),  // 0x90;
    NS_LITERAL_STRING("SCROLL_LOCK"),  // 0x91;
    EmptyString(),  // 0x92
    EmptyString(),  // 0x93
    EmptyString(),  // 0x94
    EmptyString(),  // 0x95
    EmptyString(),  // 0x96
    EmptyString(),  // 0x97
    EmptyString(),  // 0x98
    EmptyString(),  // 0x99
    EmptyString(),  // 0x9A
    EmptyString(),  // 0x9B
    EmptyString(),  // 0x9C
    EmptyString(),  // 0x9D
    EmptyString(),  // 0x9E
    EmptyString(),  // 0x9F
    EmptyString(),  // 0xA0
    EmptyString(),  // 0xA1
    EmptyString(),  // 0xA2
    EmptyString(),  // 0xA3
    EmptyString(),  // 0xA4
    EmptyString(),  // 0xA5
    EmptyString(),  // 0xA6
    EmptyString(),  // 0xA7
    EmptyString(),  // 0xA8
    EmptyString(),  // 0xA9
    EmptyString(),  // 0xAA
    EmptyString(),  // 0xAB
    EmptyString(),  // 0xAC
    EmptyString(),  // 0xAD
    EmptyString(),  // 0xAE
    EmptyString(),  // 0xAF
    EmptyString(),  // 0xB0
    EmptyString(),  // 0xB1
    EmptyString(),  // 0xB2
    EmptyString(),  // 0xB3
    EmptyString(),  // 0xB4
    EmptyString(),  // 0xB5
    EmptyString(),  // 0xB6
    EmptyString(),  // 0xB7
    EmptyString(),  // 0xB8
    EmptyString(),  // 0xB9
    EmptyString(),  // 0xBA
    EmptyString(),  // 0xBB
    NS_LITERAL_STRING("COMMA"),  // 0xBC;
    EmptyString(),  // 0xBD
    NS_LITERAL_STRING("PERIOD"),  // 0xBE;
    NS_LITERAL_STRING("SLASH"),  // 0xBF;
    NS_LITERAL_STRING("BACK_QUOTE"),  // 0xC0;
    EmptyString(),  // 0xC1
    EmptyString(),  // 0xC2
    EmptyString(),  // 0xC3
    EmptyString(),  // 0xC4
    EmptyString(),  // 0xC5
    EmptyString(),  // 0xC6
    EmptyString(),  // 0xC7
    EmptyString(),  // 0xC8
    EmptyString(),  // 0xC9
    EmptyString(),  // 0xCA
    EmptyString(),  // 0xCB
    EmptyString(),  // 0xCC
    EmptyString(),  // 0xCD
    EmptyString(),  // 0xCE
    EmptyString(),  // 0xCF
    EmptyString(),  // 0xD0
    EmptyString(),  // 0xD1
    EmptyString(),  // 0xD2
    EmptyString(),  // 0xD3
    EmptyString(),  // 0xD4
    EmptyString(),  // 0xD5
    EmptyString(),  // 0xD6
    EmptyString(),  // 0xD7
    EmptyString(),  // 0xD8
    EmptyString(),  // 0xD9
    EmptyString(),  // 0xDA
    NS_LITERAL_STRING("OPEN_BRACKET"),  // 0xDB;
    NS_LITERAL_STRING("BACK_SLASH"),  // 0xDC;
    NS_LITERAL_STRING("CLOSE_BRACKET"),  // 0xDD;
    NS_LITERAL_STRING("QUOTE"),  // 0xDE;
    EmptyString(),  // 0xDF
    NS_LITERAL_STRING("META")  // 0xE0;
  };
  for (PRUint32 i = 0; i < 225; i++) virtKeyArray[i] = x[i];
}

msiKeyMap::~msiKeyMap()
{
  /* destructor code */
}


PRUint32 msiKeyMap::VKeyStringToIndex( const nsString keyname)
{
  for (PRUint32 i = 0; i < 225; i++)          // 225 is the number of items in the array
    if (keyname.Equals(virtKeyArray[i])) return i;
  return 0;
}

static NS_DEFINE_CID( kXMLDocumentCID, NS_XMLDOCUMENT_CID );

/* boolean loadKeyMapFile (in string fileName); */
NS_IMETHODIMP msiKeyMap::LoadKeyMapFile(const nsAString & filePath, PRBool *_retval)
{
// We first look for the file name in the user profile area. If it is not there, we look in the program resource area.
// Parsing and generating the hash tables is pretty straigntforward.
  nsresult rv;
  nsAutoString str;
  nsAutoString strData;
  nsAutoString keyname;
  nsCOMPtr<nsIDOMXMLDocument> docKeyMaps;
  nsCOMPtr<nsIDOMNodeList> keyTables;
  nsCOMPtr<nsIDOMNode> keyTable;
  nsCOMPtr<nsIDOMElement> keyTableElement;
  PRUint32 keyTableCount = 0;
  nsCOMPtr<nsIDOMNodeList> keys;
  nsCOMPtr<nsIDOMNode> key;
  nsCOMPtr<nsIDOMElement> keyElement;
  nsCOMPtr<nsIDOM3Node> textNode; 
  PRUint32 keyCount = 0;
  PRBool alt, shift, ctrl, meta;

  // load the XML key map file 
  *_retval = PR_FALSE;
  docKeyMaps = do_CreateInstance(kXMLDocumentCID, &rv);
  if (rv) return rv;
  rv = docKeyMaps->SetAsync(PR_FALSE);
  if (rv) return rv;
  rv = docKeyMaps->Load( filePath, _retval);
  if (rv) return rv;
  rv = docKeyMaps->GetElementsByTagName(NS_LITERAL_STRING("keytable"), getter_AddRefs(keyTables));
  if (keyTables) keyTables->GetLength(&keyTableCount);
  if (keyTableCount > 0)
  {
    for (PRUint32 i = 0; i < keyTableCount; i++)
    {
      keyTables->Item(i, (nsIDOMNode **) getter_AddRefs(keyTable));
      // new keytable. Create new table in the list
      keyTableElement = do_QueryInterface(keyTable);
      nameHashPair * save_pnhp = m_pnhp;
      nsAutoString name;
      PRUint32 keycount;
      rv = keyTableElement->GetAttribute(NS_LITERAL_STRING("name"),name);
      m_pnhp = new nameHashPair(name);
      m_pnhp->m_pNext = save_pnhp;
      rv = keyTableElement->GetAttribute(NS_LITERAL_STRING("keycount"),name);
      if (rv==NS_OK)
      {
        keycount = 50; //atoi(name.get());
      } else keycount = 20;
      m_pnhp->m_table.Init(keycount);
      // now populate the table
      rv = keyTableElement->GetElementsByTagName(NS_LITERAL_STRING("key"), getter_AddRefs(keys));
      if (keys) keys->GetLength(&keyCount);
      if (keyCount > 0)
      {
        for (PRUint32 j = 0; i < keyCount; j++)
        {
          keys->Item(j, (nsIDOMNode **) getter_AddRefs(key));
          keyElement = do_QueryInterface(key);
          rv = keyElement->GetAttribute(NS_LITERAL_STRING("name"), keyname);
          alt = shift = ctrl = meta = PR_FALSE;
          rv = keyElement->GetAttribute(NS_LITERAL_STRING("alt"), str);
          if (rv == NS_OK && str.EqualsLiteral("1")) alt = PR_TRUE;
          rv = keyElement->GetAttribute(NS_LITERAL_STRING("shift"), str);
          if (rv == NS_OK && str.EqualsLiteral("1")) shift = PR_TRUE;
          rv = keyElement->GetAttribute(NS_LITERAL_STRING("ctrl"), str);
          if (rv == NS_OK && str.EqualsLiteral("1")) ctrl = PR_TRUE;
          rv = keyElement->GetAttribute(NS_LITERAL_STRING("meta"), str);
          if (rv == NS_OK && str.EqualsLiteral("1")) meta = PR_TRUE; 
          // have to store the reserved attribute also
          textNode = do_QueryInterface(keyElement);
          rv = textNode->GetTextContent(strData);
          PRUint32 index = VKeyStringToIndex(name);
          if (index)
            m_pnhp->m_table.Put(keyStruct(index, alt, ctrl, shift, meta).m_encodedInfo, &strData);
        }
      }
    }
  }
  return NS_OK;
}

/* boolean mapExists (in string mapname, out unsigned short returnType); */
NS_IMETHODIMP msiKeyMap::MapExists(const nsAString & mapname, PRUint16 *returnType, PRBool *_retval)
{
    *_retval = PR_TRUE;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name == mapname)
        return NS_OK;
      p = p->m_pNext;
    }
    *_retval = PR_FALSE;
    return NS_OK;
}

/* char mapKeyToCharacter (in string mapname, in unsigned long virtualKey); */
NS_IMETHODIMP msiKeyMap::MapKeyToCharacter(const nsAString & mapname, PRUint32 virtualKey, 
  PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRUint32 *_retval)
{
    nsString * pstr = new nsString;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name == mapname)
      {
        p->m_table.Get(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey).m_encodedInfo, &pstr);
        *_retval = (PRUint32)(*pstr)[0];
        delete pstr;
        return NS_OK;  
      }
      p = p->m_pNext;
    }
    *_retval = 0;
    return NS_OK;
}

/* string mapKeyToScript (in string mapname, in unsigned long virtualKey); */
NS_IMETHODIMP msiKeyMap::MapKeyToScript(const nsAString & mapname, PRUint32 virtualKey, 
  PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, nsAString & _retval)
{
    nsString * pstr = new nsString;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name == mapname)
      {
        p->m_table.Get(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey).m_encodedInfo, &pstr);
        _retval.Assign(*pstr);
        delete pstr;
        return NS_OK;  
      }
      p = p->m_pNext;
    }
    _retval = NS_LITERAL_STRING("");
    return NS_OK;
}

/* boolean addCharacterMap (in string mapname, in unsigned long virtualKey, in char destChar); */
NS_IMETHODIMP msiKeyMap::AddCharacterMapping(const nsAString & mapname, PRUint32 virtualKey, 
  PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRUint32 destChar, PRBool *_retval)
{
    m_fDirty = PR_TRUE;
    return NS_OK;
}

/* boolean addScriptMap (in string mapname, in unsigned long virtualKey, in string theScript); */
NS_IMETHODIMP msiKeyMap::AddScriptMapping(const nsAString & mapname, PRUint32 virtualKey, 
  PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, const nsAString & theScript, PRBool *_retval)
{
    m_fDirty = PR_TRUE;
    *_retval = PR_FALSE;
    nsString str(theScript);
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name == mapname)
      {
        p->m_table.Put(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey).m_encodedInfo, &str);
        *_retval = PR_TRUE;
        return NS_OK;  
      }
      p = p->m_pNext;
    }
    return NS_OK;
}

/* boolean delKeyMap (in string mapname, in unsigned long virtualKey); */
NS_IMETHODIMP msiKeyMap::DelKeyMapping(const nsAString & mapname, PRUint32 virtualKey, 
 PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRBool *_retval)
{
    m_fDirty = PR_TRUE;
    *_retval = PR_FALSE;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name == mapname)
      {
        p->m_table.Remove(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey).m_encodedInfo);
        *_retval = PR_TRUE;
        return NS_OK;  
      }
      p = p->m_pNext;
    }
    return NS_OK;
}

/* boolean saveKeyMaps (); */
NS_IMETHODIMP msiKeyMap::SaveKeyMaps(PRBool *_retval)
{
    m_fDirty = PR_FALSE;
    return NS_OK;
}

