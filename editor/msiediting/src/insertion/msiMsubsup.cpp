// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsubsup.h"
#include "nsIDOMNode.h"

msiMsubsup::msiMsubsup(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiScript(mathmlNode, offset, MATHML_MSUBSUP)
{
}
  

PRUint32 msiMsubsup::DetermineInsertPosition(PRUint32 flags)
{
  PRUint32 position(IP_BaseLeft);
  if ((m_offset == 0 && (flags & FLAGS_NODE_LEFT_RIGHT) == FLAGS_NONE) || 
      (m_offset == 0 && (flags & FLAGS_NODE_LEFT) == FLAGS_NODE_LEFT))
    position = IP_BaseLeft;
  else if ((m_offset == 0 && (flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT) ||
           (m_offset == 1 && (flags & FLAGS_NODE_LEFT_RIGHT) == FLAGS_NONE))
    position = IP_BaseRight;
  else if (m_offset == 1 && ((flags & FLAGS_NODE_LEFT) == FLAGS_NODE_LEFT))
    position = IP_Script1Left;
  else if ((m_offset == 1 && (flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT) ||
           (m_offset == 2 && (flags & FLAGS_NODE_LEFT_RIGHT) == FLAGS_NONE))
    position = IP_Script1Right;
  else if (m_offset == 2 && ((flags & FLAGS_NODE_LEFT) == FLAGS_NODE_LEFT))
    position = IP_Script2Left;
  else // if ((m_offset == 2 && (flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT) || m_offset == 3)
    position = IP_Script2Right;
  return position;
}
