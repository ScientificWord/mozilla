// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "nsIDOMNSUIEvent.h"
#include "nsCOMPtr.h"
#include "msiEditorLog.h"
#include "msiIMathMLInsertion.h"
#include "msiIMathMLCaret.h"
#include "msiIMathMLCoalesce.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIDocument.h"
#include "nsIDomText.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMNodeList.h"

msiEditorLog::msiEditorLog()
{
}

msiEditorLog::~msiEditorLog()
{
}

//ljh -- There doesn't seem to be NS_IMPL_ISUPPORTS macros to deal with the case when
// a class has inheritance from two non-interface classes. So I listed the nsIEditorLogging 
// interface explicately.
NS_IMPL_ISUPPORTS_INHERITED1(msiEditorLog, msiEditor, nsIEditorLogging) 

 
//Begin msiIMathMLEditor

NS_IMETHODIMP 
msiEditorLog::InsertInlineMath()
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertInlineMath();\n");
    Flush();
  }
  return msiEditor::InsertInlineMath();
}


NS_IMETHODIMP 
msiEditorLog::InsertDisplay()
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertDisplay();\n");
    Flush();
  }
  return msiEditor::InsertDisplay();
}



NS_IMETHODIMP 
msiEditorLog::InsertSuperscript()
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertSuperscript();\n");
    Flush();
  }
  return msiEditor::InsertSuperscript();
}


NS_IMETHODIMP 
msiEditorLog::InsertSubscript()
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertSubscript();\n");
    Flush();
  }
  return msiEditor::InsertSubscript();
}

NS_IMETHODIMP 
msiEditorLog::InsertFraction(const nsAString& lineThickness, PRUint32 attrFlags)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertFraction(\"");
    nsAutoString str(lineThickness);
    PrintUnicode(str);
    Write("\", ");
    WriteInt(attrFlags);
    Write(");\n");
    Flush();
  }
  return msiEditor::InsertFraction(lineThickness, attrFlags);
}

NS_IMETHODIMP 
msiEditorLog::InsertBinomial(const nsAString& opening, const nsAString& closing,
                          const nsAString& lineThickness, PRUint32 attrFlags)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertBinomial(\"");
    nsAutoString str(opening);
    PrintUnicode(str);
    Write("\", \"");
    str = closing;
    PrintUnicode(str);
    Write("\", \"");
    str = lineThickness;
    PrintUnicode(str);
    Write("\", ");
    WriteInt(attrFlags);
    Write(");\n");
    Flush();
  }
  return msiEditor::InsertBinomial(opening, closing, lineThickness, attrFlags);
}

NS_IMETHODIMP 
msiEditorLog::InsertSqRoot()
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertSqRoot();\n");
    Flush();
  }
  return msiEditor::InsertSqRoot();
}

NS_IMETHODIMP 
msiEditorLog::InsertRoot()
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertRoot();\n");
    Flush();
  }
  return msiEditor::InsertRoot();
}


NS_IMETHODIMP
msiEditorLog::InsertSymbol(const nsAString & symbol)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertSymbol(");
    WriteInt(symbol);
    Write(");\n");
    Flush();
  }
  return msiEditor::InsertSymbol(symbol);
}

NS_IMETHODIMP
msiEditorLog::InsertMathname(const nsAString & mathname)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertMathname(\"");
    nsAutoString str(mathname);
    PrintUnicode(str);
    Write("\");\n");
    Flush();
  }
  return msiEditor::InsertMathname(mathname);
}

NS_IMETHODIMP
msiEditorLog::InsertEngineFunction(const nsAString & mathname)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertEngineFunction(\"");
    nsAutoString str(mathname);
    PrintUnicode(str);
    Write("\");\n");
    Flush();
  }
  return msiEditor::InsertEngineFunction(mathname);
}

NS_IMETHODIMP
msiEditorLog::InsertFence(const nsAString & open, const nsAString & close)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertFence(\"");
    nsAutoString str(open);
    PrintUnicode(str);
    Write("\", \"");
    str = close;
    PrintUnicode(str);
    Write("\");\n");
    Flush();
  }
  return msiEditor::InsertFence(open, close);
}

NS_IMETHODIMP
msiEditorLog::InsertMatrix(PRUint32 rows, PRUint32 cols, const nsAString & rowSignature, const nsAString & delim)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertMatrix(");
    WriteInt(rows);
    Write(", ");
    WriteInt(cols);
    Write(", \"");
    nsAutoString str(rowSignature);
    PrintUnicode(str);
    Write("\");\n");
    Flush();
  }
  return msiEditor::InsertMatrix(rows, cols, rowSignature, delim);
}


NS_IMETHODIMP
msiEditorLog::InsertOperator(const nsAString & symbol, PRUint32 attrFlags,
                          const nsAString & leftspace, const nsAString & rightspace,
                          const nsAString & minsize, const nsAString & maxsize)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertOperator(\"");
    nsAutoString str(symbol);
    PrintUnicode(str);
    Write("\", ");
    WriteInt(attrFlags);
    Write(", \"");
    str = leftspace;
    PrintUnicode(str);
    Write("\", \"");
    str = rightspace;
    PrintUnicode(str);
    Write("\", \"");
    str = minsize;
    PrintUnicode(str);
    Write("\", \"");
    str = maxsize;
    PrintUnicode(str);
    Write("\");\n");
    Flush();
  }
  return msiEditor::InsertOperator(symbol, attrFlags, leftspace, rightspace,
                                   minsize, maxsize);
}

NS_IMETHODIMP
msiEditorLog::InsertDecoration(const nsAString & above, const nsAString & below,
                               const nsAString & aroundNotation, const nsAString & aroundType)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).InsertDecoration(\"");
    nsAutoString str(above);
    PrintUnicode(str);
    Write("\", \"");
    str = below;
    PrintUnicode(str);
    Write("\");\n");
    str = aroundNotation;
    PrintUnicode(str);
    Write("\");\n");
    str = aroundType;
    PrintUnicode(str);
    Write("\");\n");
    Flush();
  }
  return msiEditor::InsertDecoration(above, below, aroundNotation, aroundType);
}



//End nsIMathMLEditor

// nsIPlaintextEditor overrides
NS_IMETHODIMP 
msiEditorLog::InsertText(const nsAString &aStringToInsert)
{
  nsAutoHTMLEditorLogLock logLock(this);
  if (!mLocked && mFileStream)
  {
    PrintSelection();
    Write("GetCurrentEditor().QueryInterface(Components.interfaces.msiIMathMLEditor).insertText(\"");
    nsAutoString str(aStringToInsert);
    PrintUnicode(str);
    Write("\");\n");
    Flush();
  }
  return msiEditor::InsertText(aStringToInsert);
}

