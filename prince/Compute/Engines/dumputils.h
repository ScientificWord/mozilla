
#ifndef DUMPUTILS_H
#define DUMPUTILS_H

#ifdef DEBUG
#define ALLOW_DUMPS
#endif


// dump utilities.  output sent to JBMline.out
namespace JBM {
  void JBMLine(const char* line);
  void ClearLog();
}

#endif
