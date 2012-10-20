
#ifndef SRL_WORD_H
#define SRL_WORD_H

#include <iostream>
#include <string>
#include <vector>

#include "RCIPtr.h"
#include "Wide.h"

namespace srl {

class Word {

 public:

  Word(const String & w,
       const String & t) : _word(w), _tag(t) {};

  Word(const srl::Word & word) 
    : _word(word.getWord()), _tag(word.getTag()) {};

  srl::Word & operator = (const srl::Word & word) {
    _word = word.getWord();
    _tag = word.getTag();
    return (* this);
  }

  const String & getWord() const { return _word; };

  const String & getTag() const { return _tag; };

  void setTag(const String & tag) { _tag = tag; };

 private:
  
  String _word;

  String _tag;

};

} // end namespace srl

#endif
