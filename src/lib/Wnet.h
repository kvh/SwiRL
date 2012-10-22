
#ifndef WN_API_H
#define WN_API_H

#include <vector>

#include "Wide.h"
#include "Morpher.h"
#include "RCIPtr.h"
#include "StringMap.h"

namespace srl {

class WordNet : public Morpher {

 public:

  WordNet();
  
  virtual String morph(const String & word,
		       const String & label);

  /**
   * Retrieves all synonyms of this word for this POS tag
   */
  bool synonyms(const String & word,
		const String & label,
		std::vector<String> & synonyms,
		bool onlyForSingleSense = true, 
		bool skipMultiWords = true);

  /**
   * Retrieves all synsets for this word and this POS tag.
   * A synset is represented as a string as follows: word_tag#_sense#
   */
  bool synsets(const String & word,
	       const String & label,
	       std::vector<String> & synsets);

  int getWordNetPos(const String & label);

 private:

  static void cacheLemma(const String & word,
			 int pos,
			 const String & lemma);

  static void cacheSynsets(const String & word,
			   int pos,
			   const std::vector<String> & synsets);

  static bool initialize();

  static bool initialized;

  /** Lemma cache. One hash maintained for each WN POS tag */
  static std::vector< srl::StringMap<String> > _lemmaCache;

  /** Synset cache. One hash maintained for each WN POS tag */
  static std::vector< srl::StringMap< std::vector<String> > > _synsetCache;

};

typedef srl::RCIPtr<WordNet> WordNetPtr;

}

#endif
