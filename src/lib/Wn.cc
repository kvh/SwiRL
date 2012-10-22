
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>

#include <wn.h>

#include "Wnet.h"
#include "StringMap.h"
#include "AssertLocal.h"

using namespace std;
using namespace srl;

bool WordNet::initialized = false;

// How many POS tags are in WordNet: 4 + 1 for unknown
#define WN_POS_COUNT 5

/** Cache for lemmas */
std::vector< srl::StringMap<String> > WordNet::_lemmaCache;

/** Cache for synsets */
std::vector< srl::StringMap< std::vector<String> > > WordNet::_synsetCache;

bool WordNet::initialize()
{
  if(initialized == true){
    return true;
  }

  int wncode = wninit();
  // CERR << "WordNet init: " << wncode << endl;
  if(wncode != 0){
    return false;
  }

  int morphcode = morphinit();
  // CERR << "WordNet morph init: " << morphcode << endl;
  if(morphcode != 0){
    return false;
  }

  // initialize the lemma and synset caches
  for(int i = 0; i < WN_POS_COUNT; i ++){
    srl::StringMap<String> h1;
    _lemmaCache.push_back(h1);

    srl::StringMap< vector<String> > h2;
    _synsetCache.push_back(h2);
  }

  initialized = true;
  return true;
}

WordNet::WordNet()
{
  if(initialize() == false){
    throw bool(false);
  }
}

int WordNet::getWordNetPos(const String & label)
{
  if(label.substr(0, 2) == "NN"){
    return 1;
  } else if(label.substr(0, 2) == "VB"){
    return 2;
  } else if(label == "AUX"){ // Charniak's parser
    return 2;
  } else if(label.substr(0, 2) == "JJ"){
    return 3;
  } else if(label.substr(0, 2) == "RB"){
    return 4;
  } 

  return 0;
}

void WordNet::cacheLemma(const String & word,
			 int pos,
			 const String & lemma)
{
  LASSERT(pos < WN_POS_COUNT, "invalid pos index");
  _lemmaCache[pos].set(word.c_str(), lemma);
}

void WordNet::cacheSynsets(const String & word,
			   int pos,
			   const std::vector<String> & synsets)
{
  LASSERT(pos < WN_POS_COUNT, "invalid pos index");
  _synsetCache[pos].set(word.c_str(), synsets);
}

String WordNet::morph(const String & word,
		      const String & label)
{

#ifdef USE_UNICODE
#error "WordNet::morph not implemented for Unicode"
#endif

  if(word.empty()){
    return word;
  }

  // This will eventually store the lemma
  String lemma;

  // What is the WN POS code?
  int wnPos = getWordNetPos(label);
  LASSERT(wnPos < WN_POS_COUNT, "invalid pos index");

  // Has this word been cached?
  if(_lemmaCache[wnPos].get(word.c_str(), lemma) == true){
    return lemma;
  }

  /*
  // Copy word in a work buffer
  char * buffer = new char[word.size() + 1];
  strcpy(buffer, word.c_str());
  buffer[word.size()] = '\0';

  // Morph the word
  char * ret = morphstr(buffer, wnPos);

  if(ret != NULL){
    lemma = ret;
  }

  // Note: ret should not be freed. it is a static WN variable
  delete [] buffer;

  // If lemma is empty return the original word
  if(lemma.empty()){
    cacheLemma(word, wnPos, word);
    return word;
  }

  cacheLemma(word, wnPos, lemma);
  return lemma;
  */
  // Copy word in a work buffer
  char * buffer = new char[word.size() + 1];
  for(size_t i = 0; i < word.size(); i ++)
    buffer[i] = tolower(word[i]);
  buffer[word.size()] = '\0';

  // Morph the word
  char * ret = morphstr(buffer, wnPos);

  if(ret != NULL){
    lemma = ret;
  } else {
    // if not found, set to the lower-case word
    lemma = buffer;
  }

  // Note: ret should not be freed. it is a static WN variable
  delete [] buffer;
  
  cacheLemma(word, wnPos, lemma);
  return lemma;
}

bool WordNet::synonyms(const String & word,
		       const String & label,
		       std::vector<String> & synonyms,
		       bool onlyForSingleSense, 
		       bool skipMultiWords)
{

#ifdef USE_UNICODE
#error "WordNet::morph not implemented for Unicode"
#endif

  if(word.empty()){
    return false;
  }

  // What is the WN POS code?
  int wnPos = getWordNetPos(label);
  LASSERT(wnPos < WN_POS_COUNT, "invalid pos index");

  // Copy word in a work buffer
  char * buffer = new char[word.size() + 1];
  strcpy(buffer, word.c_str());
  buffer[word.size()] = '\0';

  SynsetPtr syns = findtheinfo_ds(buffer, wnPos, SIMPTR, ALLSENSES);

  // not in WN
  if(syns == NULL){
    delete [] buffer;
    return false;
  }

  if(onlyForSingleSense){
    // make sure there is a single synset 
    int synsetCount = 0;
    for(SynsetPtr s = syns; s != NULL; s = s->nextss){
      synsetCount ++;
    }
    if(synsetCount <= 1){
      delete [] buffer;
      return false;
    }
  }

  int senseCount = 0;
  for(SynsetPtr s = syns; s != NULL; s = s->nextss, senseCount ++){
    for(int i = 0; i < s->wcount; i ++){
      if((skipMultiWords == false || // skip multiwords
	  strstr(s->words[i], "_") == NULL) && 
	 strstr(s->words[i], "(") == NULL && // skip words with parens
	 find(synonyms.begin(), // skip repetitions
	      synonyms.end(), 
	      s->words[i]) == synonyms.end() &&
	 word != s->words[i]){ // skip the word itself
	synonyms.push_back(s->words[i]);
      }
    }
  }

  // cerr << "Found " << senseCount << " synsets" << endl;

  // cleanup
  delete [] buffer;
  return true;
}

bool WordNet::synsets(const String & word,
		      const String & label,
		      std::vector<String> & synsets)
{
  #ifdef USE_UNICODE
#error "WordNet::morph not implemented for Unicode"
#endif

  if(word.empty()){
    return false;
  }

  // What is the WN POS code?
  int wnPos = getWordNetPos(label);
  LASSERT(wnPos < WN_POS_COUNT, "invalid pos index");

  // Has this word been cached?
  if(_synsetCache[wnPos].get(word.c_str(), synsets) == true){
    // cerr << "Found in cache!" << endl;
    return true;
  }

  // Copy word in a work buffer
  char * buffer = new char[word.size() + 1];
  strcpy(buffer, word.c_str());
  buffer[word.size()] = '\0';

  SynsetPtr syns = findtheinfo_ds(buffer, wnPos, SIMPTR, ALLSENSES);

  // not in WN
  if(syns == NULL){
    delete [] buffer;
    return false;
  }

  int senseCount = 1;
  for(SynsetPtr s = syns; s != NULL; s = s->nextss, senseCount ++){
    OStringStream os;
    os << s->words[0] << "_" << wnPos << "_" << senseCount;
    synsets.push_back(os.str());
  }

  // cache
  cacheSynsets(word, wnPos, synsets);

  // cleanup
  delete [] buffer;
  return true;
}
