
#include "dumputils.h"

#include "stdio.h"

#ifdef ALLOW_DUMPS


#define JBM_FILENAME "/tmp/JBMLine.out"

#ifdef DEBUG
void JBM::JBMLine(const char *line)
{
  FILE *jbmfile = fopen(JBM_FILENAME, "a");
  if (jbmfile) {
    if (line) {
      fputs(line, jbmfile);
    }
    fclose(jbmfile);
  }
}


#else
void JBM::JBMLine(const char *line) {}
#endif

void JBM::ClearLog()
{
  FILE *jbmfile = fopen(JBM_FILENAME, "w");
  if (jbmfile)
    fclose(jbmfile);
}

#endif
