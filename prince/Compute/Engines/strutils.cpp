#include "strutils.h"
#include <cstring>


char* AppendStr2HeapStr(char* zheap_str, 
                        U32& buffer_ln,
                        const char* z_append_str)
{
  char *rv = zheap_str;

  if (z_append_str && *z_append_str) {
    U32 curr_ln = 0;
    if (zheap_str)
      curr_ln += strlen(zheap_str);
    size_t delta_ln = strlen(z_append_str);
    U32 bytes_needed = curr_ln + delta_ln + 1;

    bool OK = true;
    if (bytes_needed > buffer_ln) {
      buffer_ln = bytes_needed + 512;
      char *tmp = new char[buffer_ln];
      if (tmp) {
        rv = tmp;
        if (zheap_str) {
          strcpy(rv, zheap_str);
          delete[] zheap_str;
        } else {
          rv[0] = 0;
        }
      } else {
        TCI_ASSERT(0);
        OK = false;
      }
    }

    if (OK) {
      if (zheap_str)
        strcat(rv, z_append_str);
      else
        strcpy(rv, z_append_str);
    }
  }

  return rv;
}


void StrReplace(char *line, size_t zln, char *tok, const char *sub)
{
  char *ptr = strstr(line, tok);
  if (ptr) {
    char *buffer = new char[zln];

    *ptr = 0;
    strcpy(buffer, line);       // head
    if (sub)
      strcat(buffer, sub);      // substitution
    ptr += strlen(tok);
    strcat(buffer, ptr);        // tail

    strcpy(line, buffer);
    delete[] buffer;
  }
}

// itoa replacement
void StrFromInt(int val, char* buffer)
{
  sprintf(buffer, "%d", val);
}

