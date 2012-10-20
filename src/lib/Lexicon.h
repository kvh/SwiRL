/**
 * @file Lexicon.h
 * Lexicon learned from training data
 */

#include <list>
#include <vector>
#include <string>

#include "RCIPtr.h"
#include "HashMap.h"
#include "CharArrayEqualFunc.h"
#include "CharArrayHashFunc.h"
#include "Constants.h"
#include "Pair.h"
#include "StringMap.h"
#include "Wide.h"

#ifndef LEXICON_H
#define LEXICON_H

namespace srl {

class FeatureData {
 public:
  FeatureData(int i, int f) : _index(i), _freq(f) {}

 public:
  int _index;
  int _freq;
};

class VerbParticle {
 public:
  VerbParticle(const String & w, int c) : _word(w), _count(c) {}

  String _word;
  int _count;
};

class Lexicon : public RCObject
{
 public:
  Lexicon();

  /** Initializes the lexicon for training/testing */
  static void initialize(const String & modelPath,
			 bool testing);

  static void addWordCount(const String & word);
  static int getWordCount(const Char * word);
  static bool isUnknownWord(const String & word) { 
    return (getWordCount(word.c_str()) < UNKNOWN_WORD_THRESHOLD);
  }

  static void addLemmaCount(const String & word);
  static int getLemmaCount(const Char * word);
  static bool isUnknownLemma(const String & word) { 
    return (getLemmaCount(word.c_str()) < UNKNOWN_LEMMA_THRESHOLD);
  }
  static bool isUnknownPred(const String & word) { 
    return (getLemmaCount(word.c_str()) < UNKNOWN_PREDICATE_THRESHOLD);
  }

  static void addArgLabelCount(const String & label);
  static void addSyntacticLabelCount(const String & label);

  static void addBICount(const String & label);
  static void saveBICounts(std::ostream & os);

  static bool isTmpNN(const Char * word);
  static bool isTmpNNP(const Char * word);
  static bool isTmpIN(const Char * word);
  static bool isTmpRB(const Char * word);

  static void saveCounts(const String & path);
  static void loadCounts(const String & path);
  static void loadTmps(const String & path);

  static int getFeatureIndex(const String & feature,
			     bool create);

  static void saveFeatureLexicon(std::ostream & os);
  static void loadFeatureLexicon(const String & path);

  static void loadArgFrames(const String & path);
  static bool * getFrame(const String & lemma);

  static const StringMap<int> & getSyntacticLabelCounts() 
    { return _syntacticLabelCounts; }

  static const StringMap<int> & getBICounts() 
    { return _biCounts; }

  static void addArgValidLabel(const String & arg,
			       const String & label);

  static void addVerbParticle(const String & verb,
			      const String & particle,
			      int count = 1);
  static bool isVerbParticle(const String & verb,
			     const String & particle);

  static void addIntervalChecks(std::vector<String> & feats, 
				const String & prefix,
				int value, 
				int min, 
				int max,
				int increment);

  static void addThresholdValue(std::vector<String> & feats, 
				const String & prefix,
				int value,
				int threshold);

  /** not used */
  static void addGeneralizedValues(std::vector<String> & feats, 
				   const String & prefix,
				   int value, 
				   int min, 
				   int max);

  static int maxTokenDistance;

 private:

  /** Word counts */
  static StringMap<int> _wordCounts;
  static StringMap<int> _lemmaCounts;

  /** Argument label counts */
  static StringMap<int> _labelCounts;

  /** Syntactic label counts */
  static StringMap<int> _syntacticLabelCounts;

  /** Temporal phrases */
  static StringMap<int> _tmpNN, _tmpIN, _tmpRB, _tmpNNP;

  /** Feature lexicon */
  static StringMap<FeatureData *> _features;
  static int _featureIndex;

  /** Verb frames */
  static StringMap<bool *> _frames;

  /** Valid syntactic labels for each attribute */
  static StringMap<StringMap<int> *> _validLabels;

  /** Set of particles for each verb */
  static StringMap<std::vector<VerbParticle> *> _verbParticles;

  /** Counts for B args that have I conts */
  static StringMap<int> _biCounts;
};

typedef RCIPtr<srl::Lexicon> LexiconPtr;

} // end namespace srl

#endif
