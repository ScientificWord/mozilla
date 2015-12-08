#ifndef PARSERES_H
#define PARSERES_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#ifndef TCISTRIN_H
  #include "tcistrin.h"
#endif

// --Grammar--
// Sentence = { Expression }
// Expression = text | ( '{' Choices '}' )
// Choices = indexKeyword { '|' Sentence }

class CParseResourceText {
public:
  CParseResourceText();
  ~CParseResourceText();

  void SetDelimiters(TCICHAR symOpenBrace, TCICHAR symCloseBrace,
                     TCICHAR symBar);

  void AddValue(const TCIString &symbol, int value);
  void Parse(TCIString &text);

private:
  int GetValue(const TCIString &symbol);
  void ParseSentence(TCIString &buff, TCI_BOOL writeToBuff);
  void ParseChoices(TCIString &buff, TCI_BOOL writeToBuff);
  void PopToken();
  void PushToken();

// Private Data Types
private:
  // Symbol table
  struct SList;
  struct SList {
    TCIString m_symbol;
    int m_value;
    SList *m_next;
  };

  enum TOKEN_TYPE {
      TOK_END,      // end of token stream
      TOK_STRING,   // characters not symbols
      TOK_SYMBOL,   // symbol character = {}/
  };

// Data
private:
  SList *m_symbolTable;   // Symbol table

  TCICHAR m_symbols[4];    // default is "{}|"
  TCICHAR m_symOpenBrace;
  TCICHAR m_symCloseBrace;
  TCICHAR m_symBar;
    
  TCIString m_buff;       // internal token buffer
  TCIString m_tokenText;  // current token text
  TOKEN_TYPE m_token;     // current token type
};

#endif

