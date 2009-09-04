#include "strutils.h"
#include <cctype>
#include <cstring>
#include <stdio.h>


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

U32 ASCII2U32(const char* ptr, int place_val)
{
  U32 unicode = 0;

  if (place_val == 10) {
    while (*ptr) {
      if (isdigit(*ptr))
        unicode = place_val * unicode + *ptr - '0';
      else
        break;
      ptr++;
    }
  } else if (place_val == 16) {
    while (*ptr) {
      if (isdigit(*ptr))
        unicode = place_val * unicode + *ptr - '0';
      else if (*ptr >= 'A' && *ptr <= 'F')
        unicode = place_val * unicode + *ptr - 'A' + 10;
      else if (*ptr >= 'a' && *ptr <= 'f')
        unicode = place_val * unicode + *ptr - 'a' + 10;
      else
        break;
      ptr++;
    }
  }

  return unicode;
}


char* DuplicateString(const char* str)
{
   if (str == NULL)
     return (char*) NULL;

   size_t ln = strlen(str);
   char* tmp = new char[ln + 1];
   strcpy(tmp, str);
   return tmp;
   
}


bool StringEqual(const char* str1, const char* str2)
{
  if (str1 == NULL && str2 != NULL)
    return false;

  if (str1 != NULL && str2 == NULL)
    return false;

  if (str1 == NULL && str2 == NULL)
    return true;

  return ! strcmp(str1, str2);
}


bool StringNonEmpty(const char* str)
{
  return (str != NULL) && (strlen(str) > 0);
}





