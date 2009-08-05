#ifndef STR_UTILS_H
#define STR_UTILS_H

#include "cmptypes.h"

char* AppendStr2HeapStr(char* zheap_str, 
                        U32& buffer_ln,
                        const char* z_append_str);

void StrReplace(char* line, size_t zln, char* tok, const char* sub);
void StrFromInt(int val, char* buffer);
U32 ASCII2U32(const char *p_digits, int place_val);



#endif