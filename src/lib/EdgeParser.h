
#include <iostream>
#include <list>
#include <string>

#include "Pair.h"
#include "EdgeLexer.h"
#include "Lexicon.h"
#include "CharUtils.h"
#include "Exception.h"
#include "AssertLocal.h"

#ifndef EDGE_PARSER_H
#define EDGE_PARSER_H

#define EXTRACT_TOKEN(type, msg){ \
  if(lexem(text) != type){ \
    throw srl::Exception(msg, _lexer.getLineCount()); \
  } \
}

namespace srl {

/**
 * Parses an edge of type T. 
 * The type T must have these methods: setLabel, setWord, addChild
 */
template <class T>
class EdgeParser 
{
 public:

  EdgeParser(IStream & stream) : _lexer(stream) {};

  srl::RCIPtr<T> parseEdge(bool gatherCounts) {
    String text;
    int lex;

    lex = lexem(text);

    // end of file reached
    if(lex == EdgeLexer::TOKEN_EOF){
      return srl::RCIPtr<T>();
    }
    
    RVASSERT(lex == srl::EdgeLexer::TOKEN_LP,
	     "Syntax error: Left parenthesis expected");
    /*
    if(lex != srl::EdgeLexer::TOKEN_LP){
      throw(Exception("Syntax error: Left parenthesis expected", 
		      _lexer.getLineCount()));
    }
    */

    srl::RCIPtr<T> edge(new T);

    lex = lexem(text);

    // Phrase label
    if(lex == srl::EdgeLexer::TOKEN_STRING){
      edge->setLabel(text);
      lex = lexem(text);
    } else if(lex != srl::EdgeLexer::TOKEN_RP){ // we might have () phrases
      edge->setLabel("TOP");
    }

    // This is a non-terminal phrase
    if(lex == srl::EdgeLexer::TOKEN_LP){

      // The head position might be specified immediately after the children
      // int headPosition = -1;

      // Parse all children
      while(lex == srl::EdgeLexer::TOKEN_LP){
	unget(text, lex);
	srl::RCIPtr<T> child = parseEdge(gatherCounts);
	edge->addChild(child);
	child->setParent(edge);
	lex = lexem(text);
      }

      /*
      // This token might be the head position
      if(lex == srl::EdgeLexer::TOKEN_STRING &&
	 (headPosition = toInteger(text)) >= 0 &&
	 headPosition < (int) edge->getChildren().size()){

	// Set the head
	for(typename std::list< srl::RCIPtr<T> >::const_iterator it = 
	      edge->getChildren().begin();
	    it != edge->getChildren().end(); it ++, headPosition --){
	  // Found the head child
	  if(headPosition == 0){
	    edge->setHead(* it);
	    break;
	  }
	}
      } else{
      */
      unget(text, lex);
    } 

    // Phrase word for terminal phrases
    else if(lex == srl::EdgeLexer::TOKEN_STRING){
      edge->setWord(text);
      if(gatherCounts) Lexicon::addWordCount(text);
    }

    // Empty phrase: ()
    else if(lex == srl::EdgeLexer::TOKEN_RP){
      unget(text, lex);
    }

    else{
      RVASSERT(false, "Syntax error: Left parenthesis or string expected");
      /*
      throw(srl::Exception("Syntax error: Left parenthesis or string expected", 
			     _lexer.getLineCount()));
      */
    }

    while((lex = lexem(text)) != srl::EdgeLexer::TOKEN_RP){
      if(lex == srl::EdgeLexer::TOKEN_EOF){
	RVASSERT(false, "EOF before phrase end");
	/*
	throw(srl::Exception("EOF before phrase end", 
			       _lexer.getLineCount()));
	*/
      } else if(lex == srl::EdgeLexer::TOKEN_STRING){

	// NE label
	if(isNeLabel(text)){
	  //cerr << "FOUND NE: " << text << std::endl;
	  edge->setNe(text);
	}
	// Arg
	else if(isArg(text)){
	  edge->addArgument(text);
	} 
	// must be the lemma of a predicate
	else{
	  // XXX: Jordi bug
	  // Set the lemma and predicate flag only for VB*
	  /*
	  String pre2 = edge->getLabel().substr(0, 2);
	  if(pre2 == "VB" || pre2 == "VP"){
	    // cerr << "LEMMA: " << text << std::endl;
	    edge->propagatePredicate(text, true);
	  } else {
	    // cerr << "Found non-verb predicate: " << * edge;
	  }
	  */
	  edge->propagatePredicate(text, true);
	}

      } else {
	RVASSERT(false, "Unknown token before phrase end");
	/*
	throw(srl::Exception("Unknown token before phrase end", 
			       _lexer.getLineCount()));
	*/
      }
    }

    return edge;
  }

 private:
  
  /** The lexer */
  srl::EdgeLexer _lexer;

  bool isArg(const String & text){
    if(text.size() < 3) return false;
    String pref = text.substr(0, 2);
    if(pref != "B-" && pref != "I-") return false;
    return true;
  }

  /** We accept only CoNLL NEs */
  bool isNeLabel(const String & text){
    if(text.size() < 3) return false;
    String pref = text.substr(0, 2);
    if(pref != "B-" && pref != "I-") return false;
    String tmp = text.substr(2, text.size() - 2);
    if(tmp != "PER" && tmp != "LOC" && tmp != "ORG" && tmp != "MISC")
      return false;
    return true;
  }

  int lexem(String & text) {
    if(_buffer.empty() == true){
      return _lexer.lexem(text);
    }

    srl::Pair<String, int> last = _buffer.back();
    _buffer.pop_back();

    text = last.getFirst();
    return last.getSecond();
  }

  void unget(String & text, int lex){
    _buffer.push_back(srl::Pair<String, int>(text, lex));
  }

  /** The unget stack */
  std::list< srl::Pair<String, int> > _buffer;

};

} // end namespace srl

#endif
