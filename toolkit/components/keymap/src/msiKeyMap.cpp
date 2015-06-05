#include "msiKeyMap.h"
#include "nsIDOMElement.h"
#include "nsIDOM3Node.h"
#include "nsIDOMDocument.h"
#include "nsIDOMXMLDocument.h"
#include "nsIXMLHttpRequest.h"
#include "nsIDOMNodeList.h"
#include "nsIFileStreams.h"
#include "nsILocalFile.h"
#include "nsIConverterOutputStream.h"
#include "nsComponentManagerUtils.h"
#include "nsDirectoryServiceUtils.h"
#include "nsIGenericFactory.h"
#include "nsToolkitCompsCID.h"

#define NS_XMLDOCUMENT_CID                        \
{ /* a6cf9063-15b3-11d2-932e-00805f8add32 */      \
 0xa6cf9063, 0x15b3, 0x11d2,                      \
 {0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32}}
 
 
msiKeyMap* msiKeyMap::sInstance = NULL;

void
keyStruct::ExtractInfo ( PRUint32 *virtualKey, PRBool *altKey, PRBool *ctrlKey, PRBool *shiftKey, PRBool *metaKey, PRBool *vk )
{
  *virtualKey = m_encodedInfo>>16;
  *shiftKey = m_encodedInfo & 1;
  *altKey = m_encodedInfo>>1 & 1;
  *ctrlKey = m_encodedInfo>>2 & 1; 
  *metaKey = m_encodedInfo>>3 & 1;
  *vk = m_encodedInfo>>4 & 1;
}  

/* Implementation file */
msiKeyMap*
msiKeyMap::GetInstance()
{
  if (!sInstance) {
    sInstance = new msiKeyMap();
    if (!sInstance)
      return nsnull;

    NS_ADDREF(sInstance);  // addref for sInstance global
  }
  NS_ADDREF(sInstance);   // addref for the getter

  return sInstance;
}

void 
msiKeyMap::ReleaseInstance()
{
  NS_IF_RELEASE(sInstance);
}


NS_IMPL_ISUPPORTS1(msiKeyMap, msiIKeyMap)

msiKeyMap::msiKeyMap() :m_fDirty(PR_FALSE),m_fFileLoaded(PR_FALSE),m_pnhp(nsnull)
{
  keyNameAndCode x[] = 
    {  
    //  BBM TODO: It is probably more efficient to store the string and the value, and remove all
    //  the EmptyStrings. This would change the 
    {NS_LITERAL_STRING("CANCEL"),  0x03},
    {NS_LITERAL_STRING("HELP"),  0x06},
    {NS_LITERAL_STRING("BACK_SPACE"),  0x08},
    {NS_LITERAL_STRING("TAB"),  0x09},
    {NS_LITERAL_STRING("CLEAR"),  0x0C},
    {NS_LITERAL_STRING("RETURN"),  0x0D},
    {NS_LITERAL_STRING("ENTER"),  0x0E},
    {NS_LITERAL_STRING("SHIFT"),  0x10},
    {NS_LITERAL_STRING("CONTROL"),  0x11},
    {NS_LITERAL_STRING("ALT"),  0x12},
    {NS_LITERAL_STRING("PAUSE"),  0x13},
    {NS_LITERAL_STRING("CAPS_LOCK"),  0x14},
    {NS_LITERAL_STRING("ESCAPE"),  0x1B},
    {NS_LITERAL_STRING("SPACE"),  0x20},
    {NS_LITERAL_STRING("PAGE_UP"),  0x21},
    {NS_LITERAL_STRING("PAGE_DOWN"),  0x22},
    {NS_LITERAL_STRING("END"),  0x23},
    {NS_LITERAL_STRING("HOME"),  0x24},
    {NS_LITERAL_STRING("LEFT"),  0x25},
    {NS_LITERAL_STRING("UP"),  0x26},
    {NS_LITERAL_STRING("RIGHT"),  0x27},
    {NS_LITERAL_STRING("DOWN"),  0x28},
    {NS_LITERAL_STRING("PRINTSCREEN"),  0x2C},
    {NS_LITERAL_STRING("INSERT"),  0x2D},
    {NS_LITERAL_STRING("DELETE"),  0x2E},
    {NS_LITERAL_STRING("CONTEXT_MENU"),  0x5D},
    {NS_LITERAL_STRING("NUMPAD0"),  0x60},
    {NS_LITERAL_STRING("NUMPAD1"),  0x61},
    {NS_LITERAL_STRING("NUMPAD2"),  0x62},
    {NS_LITERAL_STRING("NUMPAD3"),  0x63},
    {NS_LITERAL_STRING("NUMPAD4"),  0x64},
    {NS_LITERAL_STRING("NUMPAD5"),  0x65},
    {NS_LITERAL_STRING("NUMPAD6"),  0x66},
    {NS_LITERAL_STRING("NUMPAD7"),  0x67},
    {NS_LITERAL_STRING("NUMPAD8"),  0x68},
    {NS_LITERAL_STRING("NUMPAD9"),  0x69},
    {NS_LITERAL_STRING("MULTIPLY"),  0x6A},
    {NS_LITERAL_STRING("ADD"),  0x6B},
    {NS_LITERAL_STRING("SEPARATOR"),  0x6C},
    {NS_LITERAL_STRING("SUBTRACT"),  0x6D},
    {NS_LITERAL_STRING("DECIMAL"),  0x6E},
    {NS_LITERAL_STRING("DIVIDE"),  0x6F},
    {NS_LITERAL_STRING("F1"),  0x70},
    {NS_LITERAL_STRING("F2"),  0x71},
    {NS_LITERAL_STRING("F3"),  0x72},
    {NS_LITERAL_STRING("F4"),  0x73},
    {NS_LITERAL_STRING("F5"),  0x74},
    {NS_LITERAL_STRING("F6"),  0x75},
    {NS_LITERAL_STRING("F7"),  0x76},
    {NS_LITERAL_STRING("F8"),  0x77},
    {NS_LITERAL_STRING("F9"),  0x78},
    {NS_LITERAL_STRING("F10"),  0x79},
    {NS_LITERAL_STRING("F11"),  0x7A},
    {NS_LITERAL_STRING("F12"),  0x7B},
    {NS_LITERAL_STRING("F13"),  0x7C},
    {NS_LITERAL_STRING("F14"),  0x7D},
    {NS_LITERAL_STRING("F15"),  0x7E},
    {NS_LITERAL_STRING("F16"),  0x7F},
    {NS_LITERAL_STRING("F17"),  0x80},
    {NS_LITERAL_STRING("F18"),  0x81},
    {NS_LITERAL_STRING("F19"),  0x82},
    {NS_LITERAL_STRING("F20"),  0x83},
    {NS_LITERAL_STRING("F21"),  0x84},
    {NS_LITERAL_STRING("F22"),  0x85},
    {NS_LITERAL_STRING("F23"),  0x86},
    {NS_LITERAL_STRING("F24"),  0x87},
    {NS_LITERAL_STRING("NUM_LOCK"),  0x90},
    {NS_LITERAL_STRING("SCROLL_LOCK"),  0x91},
    {NS_LITERAL_STRING("OPEN_BRACKET"),  0xDB},
    {NS_LITERAL_STRING("BACK_SLASH"),  0xDC},
    {NS_LITERAL_STRING("CLOSE_BRACKET"),  0xDD},
    {NS_LITERAL_STRING("QUOTE"),  0xDE},
    {NS_LITERAL_STRING("META"),  0xE0},
    {NS_LITERAL_STRING(""),   0x00}, //sentinel for the end
  };
  for (PRInt32 i = 0; i==0 || x[i-1].keyCode != 0; i++) virtKeyArray[i] = x[i];
}

msiKeyMap::~msiKeyMap()
{
  /* destructor code */
}


PRUint32 msiKeyMap::VKeyStringToIndex( const nsString keyname)
{
  PRUint32 i = 0;
  while (virtKeyArray[i].keyCode != 0)         
  {
    if (keyname.Equals(virtKeyArray[i].keyName)) 
      return virtKeyArray[i].keyCode;
    i++;
  }
  return 0;
}

nsString  msiKeyMap::VKeyIndexToString ( PRUint32 index )
{
  PRUint32 i = 0;
  while (virtKeyArray[i].keyCode != 0)         
  {
    if (index == (virtKeyArray[i].keyCode)) 
      return virtKeyArray[i].keyName;
    i++;
  }
  return EmptyString();
}


static NS_DEFINE_CID( kXMLDocumentCID, NS_XMLDOCUMENT_CID );
static NS_DEFINE_CID( kXMLHttpRequestCID, NS_XMLHTTPREQUEST_CID );

/* boolean loadKeyMapFile (in string fileName); */
NS_IMETHODIMP msiKeyMap::LoadKeyMapFile(PRBool *_retval)
{
// We first look for the file name in the user profile area. If it is not there, we look in the program
// resource area and copy it to the user profile area.
// Parsing and generating the hash tables is pretty straigntforward.
  if (m_fFileLoaded) return NS_OK;
  nsresult rv;
  PRBool fExists = PR_FALSE;
  nsString  fileName;
  fileName = NS_LITERAL_STRING("keytables.xml");
  nsString  finalPath;
  nsString  temp;
  
  nsCOMPtr<nsIFile> mapfile;
  nsCOMPtr<nsIFile> mapfileDirectory;
  rv = NS_GetSpecialDirectory("ProfD", (nsIFile **)&mapfile);
  if (rv == NS_OK)
  {
   mapfile->Clone(getter_AddRefs(mapfileDirectory));
  mapfile->Append(fileName);
  mapfile->Exists(&fExists);
   // if it doesn't exist, copy it from the resource area
   if (!fExists) 
   {
     nsCOMPtr<nsIFile> resFile;

     rv = NS_GetSpecialDirectory("resource:app", getter_AddRefs(resFile));
     resFile->Append(NS_LITERAL_STRING("res"));
     resFile->Append(NS_LITERAL_STRING("xml"));
     resFile->Append(fileName);
     resFile->Exists(&fExists);
  if (!fExists) return NS_ERROR_FAILURE;
     resFile->CopyTo(mapfileDirectory, fileName);
     mapfile->Exists(&fExists);
   }
   if (fExists) rv = mapfile->GetPath(finalPath);
  temp.Assign(NS_LITERAL_STRING("file:///"));
  temp.Append(finalPath);
  finalPath.Assign(temp);
  }
  
  nsAutoString str;
  nsAutoString strData;
  nsAutoString keyname;
  nsAutoString keytype;
  PRBool fScript;
  nsCOMPtr<nsIDOMXMLDocument> docKeyMaps;
  nsCOMPtr<nsIDOMDocument> domdocKeyMaps;
  nsCOMPtr<nsIDOMNodeList> keyTables;
  nsCOMPtr<nsIDOMNode> keyTable;
  nsCOMPtr<nsIDOMElement> keyTableElement;
  PRUint32 keyTableCount = 0;
  nsCOMPtr<nsIDOMNodeList> keys;
  nsCOMPtr<nsIDOMNode> key;
  nsCOMPtr<nsIDOMElement> keyElement;
  nsCOMPtr<nsIDOM3Node> textNode; 
  PRUint32 keyCount = 0;
  PRBool alt, shift, ctrl, meta, vk, reserved;

  // load the XML key map file 
  *_retval = PR_FALSE;
  nsCOMPtr<nsIXMLHttpRequest> req;
  const nsCString GET=NS_LITERAL_CSTRING("GET");
  const nsCString xml=NS_LITERAL_CSTRING("text/xml");
  req = do_CreateInstance(kXMLHttpRequestCID, &rv);
  if (rv) return rv;
  rv = req->OpenRequest(GET, NS_ConvertUTF16toUTF8(finalPath), PR_FALSE, nsString(), nsString());
  if (rv) return rv;
  rv = req->OverrideMimeType(xml);
  if (rv) return rv;
  rv = req->Send(nsnull);
  if (rv) return rv;
  rv = req->GetResponseXML(getter_AddRefs(domdocKeyMaps));
  if (rv) return rv;
  docKeyMaps = do_QueryInterface(domdocKeyMaps);

  rv = docKeyMaps->GetElementsByTagName(NS_LITERAL_STRING("keytable"), getter_AddRefs(keyTables));
  if (keyTables) keyTables->GetLength(&keyTableCount);
  if (keyTableCount > 0)
  {
    for (PRInt32 i = keyTableCount-1; i >= 0; i--)
    {
      keyTables->Item(i, (nsIDOMNode **) getter_AddRefs(keyTable));
      // new keytable. Create new table in the list
      keyTableElement = do_QueryInterface(keyTable);
      nameHashPair * save_pnhp = m_pnhp;
      nsAutoString name;
//      PRUint32 keycount;
      rv = keyTableElement->GetAttribute(NS_LITERAL_STRING("name"),name);
      rv = keyTableElement->GetAttribute(NS_LITERAL_STRING("keytype"),keytype);
      fScript = NS_LITERAL_STRING("script").Equals(keytype);
      m_pnhp = new nameHashPair(name, fScript);
      m_pnhp->m_pNext = save_pnhp;
//      rv = keyTableElement->GetAttribute(NS_LITERAL_STRING("keycount"),name);
//      if (rv==NS_OK)
//      {
//        keycount = 50; //atoi(name.get());
//      } else keycount = 20;
//      m_pnhp->m_table.Init(keycount);   The table in initialized in the nameHashPair constructor
      // now populate the table
//      rv = keyTableElement->GetElementsByTagName(NS_LITERAL_STRING("key"), getter_AddRefs(keys));
//      if (keys) keys->GetLength(&keyCount);
      nsCOMPtr<nsIDOMNode> nodePtr;
      PRUint16 nodeType;
      rv = keyTableElement->GetFirstChild(getter_AddRefs(nodePtr));
      if (!nodePtr) break;
      rv = nodePtr->GetNodeType(&nodeType);
      while (nodePtr != nsnull)
      {
        while ((nodePtr != nsnull) && (nodeType != nsIDOMNode::ELEMENT_NODE))
        {
          rv = nodePtr->GetNextSibling(getter_AddRefs(nodePtr));
          if (nodePtr == nsnull) break;
          rv = nodePtr->GetNodeType(&nodeType);
        }
        if (nodePtr == nsnull) break;
        alt = shift = ctrl = meta = reserved = PR_FALSE;
        keyElement = do_QueryInterface(nodePtr);
        rv = keyElement->GetAttribute(NS_LITERAL_STRING("name"), keyname);
        alt = shift = ctrl = meta = vk = PR_FALSE;
        rv = keyElement->GetAttribute(NS_LITERAL_STRING("alt"), str);
        if (rv == NS_OK && str.Equals(NS_LITERAL_STRING("1"))) alt = PR_TRUE;
        rv = keyElement->GetAttribute(NS_LITERAL_STRING("shift"), str);
        if (rv == NS_OK && str.Equals(NS_LITERAL_STRING("1"))) shift = PR_TRUE;
        rv = keyElement->GetAttribute(NS_LITERAL_STRING("ctrl"), str);
        if (rv == NS_OK && str.Equals(NS_LITERAL_STRING("1"))) ctrl = PR_TRUE;
        rv = keyElement->GetAttribute(NS_LITERAL_STRING("meta"), str);
        if (rv == NS_OK && str.Equals(NS_LITERAL_STRING("1"))) meta = PR_TRUE; 
        rv = keyElement->GetAttribute(NS_LITERAL_STRING("vk"), str);
        if (rv == NS_OK && str.Equals(NS_LITERAL_STRING("1"))) vk = PR_TRUE; 
        rv = keyElement->GetAttribute(NS_LITERAL_STRING("reserved"), str);
        if (rv == NS_OK && str.Equals(NS_LITERAL_STRING("1"))) reserved = PR_TRUE; 
        // have to store the reserved attribute also
        if (reserved)
          strData.Assign(NS_LITERAL_STRING("reserved"));
        else
        {
          textNode = do_QueryInterface(keyElement);
          rv = textNode->GetTextContent(strData);
        }
        nsAutoPtr<nsString> newString(new nsString(strData));
        PRUint32 index;
        if (vk) 
          index = VKeyStringToIndex(keyname);
        else
        {
          nsAString::const_iterator begin;
          keyname.BeginReading(begin);
          index = (PRUint32)(*begin);
          shift = PR_FALSE; // we always ignore shifts for characters because the character is shifted already
        }
//          if (index == 114)
//          {
//            index++; // a little dance so we can stop the debugger
//            index--;
//          }
        if (index)
        {
          m_pnhp->m_table.Put(keyStruct(index, alt, ctrl, shift, meta, vk).m_encodedInfo, newString);
          newString.forget();
        }
        rv = nodePtr->GetNextSibling(getter_AddRefs(nodePtr));
        if (nodePtr == nsnull) break;
        rv = nodePtr->GetNodeType(&nodeType);
      }
    }
  }
  m_fFileLoaded = PR_TRUE;
  return NS_OK;
}

/* boolean mapExists (in string mapname, out unsigned short returnType); */
NS_IMETHODIMP msiKeyMap::MapExists(const nsAString & mapname, PRUint16 *returnType, PRBool *_retval)
{
    *_retval = PR_TRUE;
    nsAutoString str;
    str.Assign(mapname);
    nsAutoString str2;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      str2 = p->m_name;
      if (p->m_name.Equals(mapname))
      {
        *returnType = (p->m_fScript?SCRIPT:CHARACTER);
        return NS_OK;
      }
      p = p->m_pNext;
    }
    *_retval = PR_FALSE;
    return NS_OK;
}

/* unsigned long mapKeyToCharacter (in AString mapname, in unsigned long virtualKey, in boolean altKey, 
  in boolean ctrlKey, in boolean shiftKey, in boolean metaKey, out boolean reserved); */
NS_IMETHODIMP msiKeyMap::MapKeyToCharacter(const nsAString & mapname, PRUint32 virtualKey,
   PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRBool vk, PRBool *reserved, PRUint32 *_retval)
{
    nsString * pstr = new nsString;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name.Equals(mapname))
      {
        if (p->m_table.Get(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey,vk).m_encodedInfo, &pstr))
        {
          if (pstr->Equals(NS_LITERAL_STRING("reserved")))
          {
            *reserved = PR_TRUE;
            return NS_OK;
          }
          else *reserved = PR_FALSE;
          nsAString::const_iterator begin;
          pstr->BeginReading(begin);
          *_retval = (PRUint32)(*begin);
//          delete pstr;
          return NS_OK; 
        } 
      }
      p = p->m_pNext;
    }
    *_retval = 0;
    return NS_OK;
}

/* AString mapKeyToScript (in AString mapname, in unsigned long virtualKey, in boolean altKey, in boolean ctrlKey, in boolean shiftKey, in boolean metaKey, out boolean reserved); */
NS_IMETHODIMP msiKeyMap::MapKeyToScript(const nsAString & mapname, PRUint32 virtualKey, 
  PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRBool vk, PRBool *reserved, nsAString & _retval)
{
    nsString * pstr = nsnull;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name.Equals(mapname))
      {
        if (p->m_table.Get(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey,vk).m_encodedInfo, & pstr))
        {
          if (NS_LITERAL_STRING("reserved").Equals(*pstr))
          {
            *reserved = PR_TRUE;
            return NS_OK;
          }
          if (pstr->Length() > 0)_retval.Assign(*pstr); else _retval.Assign(NS_LITERAL_STRING(""));
          //          delete pstr;
          return NS_OK;
        }  
      }
      p = p->m_pNext;
    }
    _retval = NS_LITERAL_STRING("");
    return NS_OK;
}

/* boolean addCharacterMap (in string mapname, in unsigned long virtualKey, in char destChar); */
NS_IMETHODIMP msiKeyMap::AddCharacterMapping(const nsAString & mapname, PRUint32 virtualKey, 
  PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRBool vk, PRUint32 destChar, PRBool *_retval)
{
    m_fDirty = PR_TRUE;
    return NS_OK;
}

/* boolean addScriptMap (in string mapname, in unsigned long virtualKey, in string theScript); */
NS_IMETHODIMP msiKeyMap::AddScriptMapping(const nsAString & mapname, PRUint32 virtualKey, 
  PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRBool vk, const nsAString & theScript, PRBool *_retval)
{
    nsAutoPtr<nsString> newString(new nsString(theScript));
    m_fDirty = PR_TRUE;
    *_retval = PR_FALSE;
//    nsString str(theScript);
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name.Equals(mapname))
      {
        p->m_table.Put(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey,vk).m_encodedInfo, newString);
        newString.forget();
        *_retval = PR_TRUE;
        return NS_OK;  
      }
      p = p->m_pNext;
    }
    return NS_OK;
}

/* boolean delKeyMap (in string mapname, in unsigned long virtualKey); */
NS_IMETHODIMP msiKeyMap::DelKeyMapping(const nsAString & mapname, PRUint32 virtualKey, 
 PRBool altKey, PRBool ctrlKey, PRBool shiftKey, PRBool metaKey, PRBool vk, PRBool *_retval)
{
    m_fDirty = PR_TRUE;
    *_retval = PR_FALSE;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name.Equals(mapname))
      {
        p->m_table.Remove(keyStruct(virtualKey,altKey,ctrlKey,shiftKey,metaKey,vk).m_encodedInfo);
        *_retval = PR_TRUE;
        return NS_OK;  
      }
      p = p->m_pNext;
    }
    return NS_OK;
}


PLDHashOperator
nsDEnumRead(const PRUint32& aKey, nsString * aData, void* outString);

void msiKeyMap::keyArrayToString( nsTArray<PRUint32>& ka, nsString& sFile, nsClassHashtable<nsUint32HashKey, nsString>& table)
{
  PRUint32 len = ka.Length();
  nsAutoString thisLine;
  nsAutoString s;
  nsAutoString s2;
  nsString * pstr = nsnull;
  PRUint32 keyCode;
  PRBool altKey, ctrlKey, shiftKey, metaKey, vKey;
  for (PRUint32 i = 0; i < len; i++)
  {
    keyStruct ks(ka[i]);
    ks.ExtractInfo(&keyCode, &altKey, &ctrlKey, &shiftKey, &metaKey, &vKey);
    thisLine = NS_LITERAL_STRING("    <key name=\"");
    if (vKey)
    {
      s = msiKeyMap::sInstance->VKeyIndexToString(keyCode); //get the right stuff from the virtKeyArray
      thisLine.Append(s);
    }
    else
    {
      if (keyCode == 0x26) s2 = NS_LITERAL_STRING("&amp;");    
      else if (keyCode == 0x3c) s2 = NS_LITERAL_STRING("&lt;");
      else s2 = nsString((nsString::char_type*)&keyCode,1);
      thisLine.Append(s2);
    }
    thisLine.Append(NS_LITERAL_STRING("\" "));
    if (altKey)
      thisLine.Append(NS_LITERAL_STRING(" alt=\"1\""));
    if (ctrlKey)
      thisLine.Append(NS_LITERAL_STRING(" ctrl=\"1\""));
    if (shiftKey)
      thisLine.Append(NS_LITERAL_STRING(" shift=\"1\""));
    if (metaKey)
      thisLine.Append(NS_LITERAL_STRING(" meta=\"1\""));
    if (vKey)
      thisLine.Append(NS_LITERAL_STRING(" vk=\"1\""));
    if (!table.Get(ka[i], & pstr)) break;
    if (NS_LITERAL_STRING("reserved").Equals(*pstr))
    {
      thisLine.Append(NS_LITERAL_STRING(" reserved=\"1\"/>\n"));
    }
    else
    {
      thisLine.Append(NS_LITERAL_STRING(">"));
      thisLine.Append(*pstr);
      thisLine.Append(NS_LITERAL_STRING("</key>\n"));
    }
    sFile.Append(thisLine);
  }
}

/* boolean saveKeyMaps (); */
NS_IMETHODIMP msiKeyMap::SaveKeyMaps(PRBool *_retval)
{
//  if (!m_fDirty) return NS_OK;
  nsresult rv;

  nsAutoString sFile;
  sFile = NS_LITERAL_STRING("<?xml version=\"1.0\"?>\n");
  sFile +=  NS_LITERAL_STRING("<!DOCTYPE tables PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\" \"http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd\">\n");
  sFile +=  NS_LITERAL_STRING("<tables>\n");

  nsAutoString str;
 
  nameHashPair * p = m_pnhp;
  
  while (p)
  {
    nsTArray<PRUint32> keyArray;
    str = p->m_name;
    sFile += NS_LITERAL_STRING("  <keytable id=\"");
    sFile += str;
    sFile += NS_LITERAL_STRING("\" name=\"");
    sFile += str;
    sFile += NS_LITERAL_STRING("\" keytype=\"");
    sFile += (p->m_fScript ? NS_LITERAL_STRING("script\">\n") : NS_LITERAL_STRING("char\">\n"));
  
    p->m_table.EnumerateRead(nsDEnumRead, &(keyArray));
    keyArray.Sort();
    keyArrayToString(keyArray, sFile, p->m_table);
    sFile += NS_LITERAL_STRING("  </keytable>\n");
    p = p->m_pNext;
  }
  sFile += NS_LITERAL_STRING("</tables>\n");

  PRBool fExists = PR_FALSE;
  nsAutoString fileName;
  fileName = NS_LITERAL_STRING("keytables.xml");
  nsAutoString temp;

  nsCOMPtr<nsIFile> mapfile;
  nsCOMPtr<nsIFile> profdir;
  rv = NS_GetSpecialDirectory("ProfD", (nsIFile **)&profdir);
  if (rv != NS_OK)
  {
    *_retval = PR_FALSE;
    return NS_OK;
  }
  profdir->Clone((nsIFile **)&mapfile);
  profdir->Append(fileName);
  profdir->Exists(&fExists);
  // should we save a backup file?
  if (fExists) rv = profdir->MoveTo(nsnull,NS_LITERAL_STRING("keytables.bak"));
  mapfile->Append(fileName);
  rv = mapfile->Create(0, 0755);
  nsCOMPtr<nsIFileOutputStream> fos = do_CreateInstance("@mozilla.org/network/file-output-stream;1", &rv);
  if (NS_FAILED(rv))
    return rv;
  rv = fos->Init(mapfile, -1, -1, PR_FALSE);
  if (NS_FAILED(rv))
    return rv;
  nsCOMPtr<nsIConverterOutputStream> os = do_CreateInstance("@mozilla.org/intl/converter-output-stream;1", &rv);
  if (NS_FAILED(rv))
    return rv;
  rv = os->Init(fos, "UTF-8", 4096, '?');
  if (NS_FAILED(rv))
    return rv;
  PRBool fSuccess;
  rv = os->WriteString(sFile, &fSuccess);
  if (NS_FAILED(rv))
    return rv;
  rv = os->Close();
  *_retval = PR_TRUE;  
  return rv;    
}
      




PLDHashOperator
nsDEnumRead(const PRUint32& aKey, nsString * aData, void* keyArray)
{  
  keyStruct ks(aKey);
  nsTArray<PRUint32> *pka = (nsTArray<PRUint32> *)keyArray;
  pka->AppendElements(&aKey, 1);
  return PL_DHASH_NEXT;
}

/* AString getTableKeys (in AString mapname); */
NS_IMETHODIMP msiKeyMap::GetTableKeys(const nsAString & mapname, nsAString & _retval)
{
    _retval = EmptyString();
    PRUint32 len;
    nsAutoString strReturn;
    nsTArray<PRUint32> keyArray;
    nameHashPair * p = m_pnhp;
    while (p)
    {
      if (p->m_name.Equals(mapname))
      {
        p->m_table.EnumerateRead(nsDEnumRead, &(keyArray));
        len = keyArray.Length();
        keyArray.Sort();
        nsAutoString s;
        nsAutoString s2;
        nsAutoString strModifiers;
        nsString * pstr = nsnull;
        PRUint32 keyCode;
        PRBool altKey, ctrlKey, shiftKey, metaKey, vKey;
        for (PRUint32 i = 0; i < len; i++)
        {
          keyStruct ks(keyArray[i]);
          strModifiers = EmptyString();
          ks.ExtractInfo(&keyCode, &altKey, &ctrlKey, &shiftKey, &metaKey, &vKey);
          if (vKey)
          {
            s = msiKeyMap::sInstance->VKeyIndexToString(keyCode); //get the right stuff from the virtKeyArray
            strReturn.Append(s);
          }
          else
          {
            if (keyCode == 0x26) s2 = NS_LITERAL_STRING("&amp;");    
            else if (keyCode == 0x3c) s2 = NS_LITERAL_STRING("lt;");
            else s2 = nsString((nsString::char_type*)&keyCode,1);
            strReturn.Append(s2);
          }
          if (altKey)
            strModifiers.Append(NS_LITERAL_STRING("a"));
          if (ctrlKey)
            strModifiers.Append(NS_LITERAL_STRING("c"));
          if (shiftKey)
            strModifiers.Append(NS_LITERAL_STRING("s"));
          if (metaKey)
            strModifiers.Append(NS_LITERAL_STRING("m"));
          if (vKey)
            strModifiers.Append(NS_LITERAL_STRING("v"));
          if (!p->m_table.Get(keyArray[i], & pstr)) break;
          if (NS_LITERAL_STRING("reserved").Equals(*pstr))
          {
            strModifiers.Append(NS_LITERAL_STRING("r"));
          }
          if (!strModifiers.IsEmpty())
          {
            strReturn += NS_LITERAL_STRING("-");
            strReturn += strModifiers;
          }
          strReturn += NS_LITERAL_STRING(" ");
        }
        _retval = strReturn;
        return NS_OK;  
      }
      p = p->m_pNext;
    }
    return NS_OK;
}
