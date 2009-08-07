

#include "attriblist.h"
#include "CmpTypes.h"
#include "strutils.h"
#include <cstring>


 // Utility to make an attribute node

ATTRIB_REC::ATTRIB_REC(const char* attr_nom, const char* attr_val)
{
  //ATTRIB_REC *rv = new ATTRIB_REC();

  next = NULL;
  prev = NULL;

  if (attr_nom) {
    //size_t zln = strlen(attr_nom);
    //char *tmp = new char[zln + 1];
    //strcpy(tmp, attr_nom);
    zattr_nom = DuplicateString(attr_nom);
  } else {
    zattr_nom = NULL;
  }
  if (attr_val) {               // we have a STR value for the attribute
    //size_t zln = strlen(attr_val);
    //char *tmp = new char[zln + 1];
    //strcpy(tmp, attr_val);
    zattr_val = DuplicateString(attr_val);
  } else {
    zattr_val = NULL;
  }
}

void DisposeAttribs(ATTRIB_REC * alist)
{
  ATTRIB_REC *arover = alist;
  while (arover) {
    ATTRIB_REC *adel = arover;
    arover = arover->next;
    delete[] adel->zattr_nom;
    delete[] adel->zattr_val;
    delete adel;
  }
}


const char *GetATTRIBvalue(ATTRIB_REC * a_list, const char *targ_name)
{
  ATTRIB_REC *rover = a_list;
  while (rover) {
    if (!strcmp(targ_name, rover->zattr_nom)) {
      return rover->zattr_val;
    }
    rover = rover->next;
  }

  return NULL;
}


const char* GetATTRIBvalue(const AttribList& aList, const char* targ_name)
{
   AttribList::const_iterator it = aList.begin();
   while (it != aList.end()){
     if (!strcmp(targ_name, (*it).zattr_nom))
	   return (*it).zattr_val;
   }
   return NULL;
}


// largeop="true" stretchy="true"  lspace="0em" rspace="0em"

ATTRIB_REC* ExtractAttrs(char *zattrs)
{
  ATTRIB_REC *head = NULL;      // We build an "attrib_list"
  ATTRIB_REC *tail;

  if (zattrs) {
    bool done = false;
    char *anom_ptr = zattrs;
    while (!done) {
      // Span spaces
      while (*anom_ptr && *anom_ptr <= ' ')
        anom_ptr++;
      if (*anom_ptr) {
        char *p_equal = strchr(anom_ptr, '=');
        if (p_equal) {
          char *left_delim = strchr(p_equal + 1, '"');
          if (left_delim) {
            left_delim++;
            char *right_delim = strchr(left_delim, '"');
            if (right_delim) {
              while (p_equal > anom_ptr && *(p_equal - 1) <= ' ')
                p_equal--;
              char znom[80];
              size_t nom_ln = p_equal - anom_ptr;
              if (nom_ln && nom_ln < 80) {
                strncpy(znom, anom_ptr, nom_ln);
                znom[nom_ln] = 0;

                size_t val_ln = right_delim - left_delim;
                if (val_ln && val_ln < 128) {
                  char zval[128];
                  strncpy(zval, left_delim, val_ln);
                  zval[val_ln] = 0;
                  ATTRIB_REC* arp = new ATTRIB_REC(znom, zval);
                  if (!head)
                    head = arp;
                  else
                    tail->next = arp;
                  tail = arp;
                } else {
                  TCI_ASSERT(0);
                }
              } else {
                TCI_ASSERT(!"attrib zname exceeds 80 bytes??");
              }
              anom_ptr = right_delim + 1;
            } else {            // no ending "
              done = true;
            }
          } else {              // no starting "
            done = true;
          }
        } else {                // no = 
          done = true;
        }
      } else {                  // nothing after spaces
        done = true;
      }
    }
  }

  return head;
}

ATTRIB_REC *MergeAttrsLists(ATTRIB_REC * dest_list, ATTRIB_REC * new_attrs)
{
  ATTRIB_REC *rv = NULL;

  if (!dest_list) {
    rv = new_attrs;
  } else if (!new_attrs) {
    rv = dest_list;
  } else {
    rv = dest_list;

    ATTRIB_REC *rover = new_attrs;
    while (rover) {
      ATTRIB_REC *d_rover = dest_list;
      while (d_rover) {
        if (!strcmp(d_rover->zattr_nom, rover->zattr_nom)) {
          delete[] d_rover->zattr_val;
          if (rover->zattr_val) {
            d_rover->zattr_val = rover->zattr_val;
            rover->zattr_val = NULL;
          } else {
            d_rover->zattr_val = NULL;
          }
          ATTRIB_REC *ender = rover;
          rover = rover->next;
          ender->next = NULL;
          ender->prev = NULL;
          if (rover)
            rover->prev = NULL;
          DisposeAttribs(ender);
          break;
        } else {
          if (d_rover->next) {
            d_rover = d_rover->next;
          } else {
            d_rover->next = rover;
            rover->prev = d_rover;
            ATTRIB_REC *ender = rover;
            rover = rover->next;
            ender->next = NULL;
            if (rover)
              rover->prev = NULL;
            break;
          }
        }
      }
    }
  }

  return rv;
}

ATTRIB_REC *RemoveAttr(ATTRIB_REC * a_list, const char *attr_nom)
{
  ATTRIB_REC *rv = NULL;
  ATTRIB_REC *tail;

  if (a_list) {
    ATTRIB_REC *a_rover = a_list;
    while (a_rover) {
      if (!strcmp(a_rover->zattr_nom, attr_nom)) {
        ATTRIB_REC *ender = a_rover;
        a_rover = a_rover->next;
        ender->next = NULL;
        ender->prev = NULL;
        DisposeAttribs(ender);
        if (a_rover)
          a_rover->prev = NULL;
      } else {
        if (!rv) {
          rv = a_rover;
        } else {
          tail->next = a_rover;
          a_rover->prev = tail;
        }
        tail = a_rover;
        a_rover = a_rover->next;
        tail->next = NULL;
        a_rover->prev = NULL;
      }
    }
  }

  return rv;
}
