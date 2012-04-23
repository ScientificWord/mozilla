#include "msiUtil.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsComponentManagerUtils.h"
#include "nsIIOService.h"
#include "nsIFile.h"
#include "nsILocalFile.h"
#include "nsIFileURL.h"
#include "nsIFileStreams.h"
#include "nsIConverterInputStream.h"
#include "nsContentCID.h"
#include "nsIBinaryInputStream.h"
#include "nsIBufferedStreams.h"
#include "nsIGenericFactory.h"
#include "../../build/nsToolkitCompsCID.h"
#include "nsQuickSort.h"


/* Implementation file */
NS_IMPL_ISUPPORTS1(msiUtil, msiIUtil)

  
msiUtil::~msiUtil() 
{
  4+3;
}

msiUtil::msiUtil() 
{
  3+4;
}

/*   boolean SynchronousFileCopy(in inFileURI, in outFileURI);
 */
NS_IMETHODIMP msiUtil::SynchronousFileCopy(nsIFile * inFile, nsIFile * outFile, PRBool * _retval)
  
{
  nsresult res = 0;
  PRUint32 avail = 0;
  PRUint32 written = 0;
  nsCOMPtr<nsIFileInputStream> fstream = do_CreateInstance("@mozilla.org/network/file-input-stream;1",&res);
  if (fstream) res = fstream->Init(inFile, -1, -1, PR_FALSE);
  nsCOMPtr<nsIBinaryInputStream> bstream = do_CreateInstance("@mozilla.org/binaryinputstream;1",&res);
  res = bstream->SetInputStream(fstream);
  res = bstream->Available(&avail);
  nsCOMPtr<nsIFileOutputStream> fostream = do_CreateInstance("@mozilla.org/network/file-output-stream;1", &res);  
  fostream->Init(outFile, 0x04 | 0x08 | 0x20, 0600, 0); // readwrite, create, truncate  
  nsCOMPtr<nsIBufferedOutputStream> buffostream = 
  do_CreateInstance("@mozilla.org/network/buffered-output-stream;1", &res);
  buffostream->Init(fostream, 4000);  //write, create, truncate  
  PRBool nonblocking = PR_FALSE;
  buffostream->IsNonBlocking(&nonblocking);
  buffostream->WriteFrom(bstream, avail, &written);
  buffostream->Close();
  printf("out of synchronouscopy");
  return NS_OK;
}

msiUtil* smsiUtil;

/* static */ msiUtil*
msiUtil::GetInstance()
{
  if (!smsiUtil) {
    smsiUtil = new msiUtil();
    if (!smsiUtil)
      return nsnull;

    NS_ADDREF(smsiUtil);   // addref the global
  }

  NS_ADDREF(smsiUtil);   // addref the return result
  return smsiUtil;
}
