#ifndef MUPDATA_H
#define MUPDATA_H


class MupData {
private:
   Grammar* m_pGrammar;

public:
   Grammar* GetGrammar() { return m_pGrammar; }
   void SetGrammar(Grammar* pGrammar) { m_pGrammar = pGrammar; }
  
};
#endif