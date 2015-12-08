
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "parseres.h"

CParseResourceText::CParseResourceText()
{
  m_symbolTable = NULL;
  m_symOpenBrace = m_symbols[0] = '{';
  m_symCloseBrace = m_symbols[1] = '}';
  m_symBar = m_symbols[2] = '|';
  m_symbols[3] = '\0';
}  


CParseResourceText::~CParseResourceText()
{
  SList* node = m_symbolTable;
  while (node) {
    SList* delNode = node;
    node = node->m_next;
    delete delNode;
  }
}



void CParseResourceText::SetDelimiters(TCICHAR symOpenBrace,
                                       TCICHAR symCloseBrace, TCICHAR symBar)
{
  m_symOpenBrace = m_symbols[0] = symOpenBrace;
  m_symCloseBrace = m_symbols[1] = symCloseBrace;
  m_symBar = m_symbols[2] = symBar;
  m_symbols[3] = '\0';
}

    
// Symbol Table Procedures

void CParseResourceText::AddValue(const TCIString &symbol, int value)
{
  SList* node = TCI_NEW(SList);
  node->m_symbol = symbol;
  node->m_value = value;

  node->m_next = m_symbolTable;
  m_symbolTable = node;
}

int CParseResourceText::GetValue(const TCIString &symbol)
{
  SList* node = m_symbolTable;
  while (node) {
    if (node->m_symbol == symbol)
      return node->m_value;

    node = node->m_next;
  }
  return 0;
}


// Parsing Procedures

// --EBNF Grammar--
// Sentence = { Expression }
// Expression = text | ( '{' Choices '}' )
// Choices = indexKeyword { '|' Sentence }


void CParseResourceText::Parse(TCIString &buff)
{
  m_buff = buff;    // initialize token buffer

  buff = "";
  ParseSentence(buff, TRUE);
}


void CParseResourceText::ParseSentence(TCIString &buff, TCI_BOOL writeToBuff)
{
  PopToken();
  while (m_token != TOK_END) {
    if (m_token == TOK_STRING) {
      if (writeToBuff)
        buff += m_tokenText;
    }
    else if (m_tokenText[0] == m_symOpenBrace) {
      ParseChoices(buff, writeToBuff);
      PopToken();
      if (m_tokenText[0] != m_symCloseBrace)
        return;     // error
    }
    else {
      PushToken();
      return;
    }
    
    PopToken();
  }     
}


void CParseResourceText::ParseChoices(TCIString &buff, TCI_BOOL writeToBuff)
{
  PopToken();
  if (m_token != TOK_STRING)
    return;     // error
  
  int idx = GetValue(m_tokenText);
  
  int i = 0;
  PopToken();
  while (m_token != TOK_END) {
    if (m_tokenText[0] == m_symBar) {
      ParseSentence(buff, writeToBuff && (idx == i));
      ++i;
    }
    else if (m_tokenText[0] == m_symCloseBrace) {
      PushToken();
      break;
    }
    else
      return;     // error
    
    PopToken();
  }        
}



//
// Token functions
//



//  Parses the internal buffer (m_buff) for the next token.
void CParseResourceText::PopToken()
{
  if (m_buff.IsEmpty()) {
    m_tokenText = "";
    m_token = TOK_END;
    return;
  }

  int idx = 0;
  TCICHAR ch = m_buff[idx++];
  TCICHAR *symbols = m_symbols; // optimization

  if (strchr(symbols, ch))
    m_token = TOK_SYMBOL;
  else {
    m_token = TOK_STRING;
    const TCICHAR *buff = m_buff;   // optimization
    while (ch && !strchr(symbols, ch))
      ch = buff[idx++];
    --idx;      // backup one character
  }

  m_tokenText = m_buff.Left(idx);
  m_buff = m_buff.Mid(idx);
}


void CParseResourceText::PushToken()
{
  m_buff = m_tokenText + m_buff;
}

