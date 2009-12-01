#ifndef ATTRIBLIST_H
#define ATTRIBLIST_H

#include <list>

class ATTRIB_REC {
public: 
  ATTRIB_REC(const char *attrib_name, const char *attrib_value);
                     // Struct to define an attribute node.
  ATTRIB_REC* next;          //  The MNODE struct defined below
  ATTRIB_REC* prev;          //  can carry a list of attributes.
  char* zattr_nom;
  char* zattr_val;
};

typedef std::list<ATTRIB_REC> AttribList;


//ATTRIB_REC* MakeATTRIBNode(const char *attrib_name, const char *attrib_value);
const char* GetATTRIBvalue(ATTRIB_REC* a_list, const char *attrib_name);

const char* GetATTRIBvalue(const AttribList& aList, const char *attrib_name);

void DisposeAttribs(ATTRIB_REC * alist);
ATTRIB_REC* ExtractAttrs(char *zattrsAndvals);
ATTRIB_REC* MergeAttrsLists(ATTRIB_REC * dest_list, ATTRIB_REC * new_attrs);
ATTRIB_REC* RemoveAttr(ATTRIB_REC * a_list, const char *attr_nom);

#endif