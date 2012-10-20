
#include <iostream>
#include <string>

#include "Wide.h"

#ifndef EDGE_LEXER_H
#define EDGE_LEXER_H

namespace srl {

class EdgeLexer 
{
 public:

  static const int TOKEN_EOF = 0;
  static const int TOKEN_STRING = 1;
  static const int TOKEN_LP = 2;
  static const int TOKEN_RP = 3;
  
  EdgeLexer(IStream &);

  int lexem(String &);

  int getLineCount() const { return _lineCount; };

 private:
  
  /** The stream */
  IStream & _stream;

  /** Line count */
  int _lineCount;
  
  /** Advance over white spaces */
  void skipWhiteSpaces();

  bool isSpace(Char c) const;
};

} // end namespace srl

#endif
