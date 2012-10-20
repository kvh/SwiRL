/**
 * Interface to the SRL classifier 
 * Performs full parsing (Charniak) followed by SRL labeling
 */

#include <vector>

#include "Wide.h"
#include "Tree.h"
#include "Oracle.h"

class SwirlToken 
{
 public:
  SwirlToken(String word, String lemma, String pos, String ne, bool pred)
    : mWord(word), mLemma(lemma), mPos(pos), mNe(ne), mPred(pred) {}

  String getWord() const { return mWord; }
  String getLemma() const { return mLemma; }
  String getPos() const { return mPos; }
  String getNe() const { return mNe; }
  bool getPred() const { return mPred; }

 protected:
  String mWord;
  String mLemma; // only used when parsing Banks, can be left unset otherwise
  String mPos; // currently not used, can be left unset
  String mNe;
  bool mPred; // only used when parsing Banks, can be left unset otherwise
};

class Swirl
{
 public:
  /** Initializes the SRL system */
  static bool initialize(const char * srlDataDirectory,
			 const char * parserDataDirectory,
			 bool caseSensitive);
  
  /** 
   * Parses one sentence and returns the best solution 
   * Can run in oracle mode (select candidate frames with highest F1) if
   *   goldFrames is set to a non-NULL value. For regular usage, set
   *   goldFrames to NULL.
   */
  static srl::TreePtr parse(const std::vector<SwirlToken> & sentence,
			    bool detectPredicates,
			    srl::OracleSentence * goldFrames);

  /**
   * Parses one sentence and returns the best solution
   * @param sentence The string containing the input sentence.
   *                 Several formats are accepted:
   *                   0 (word ne pred)+
   *                   1 (word pos ne)+
   *                   2 (word ne)+
   *                   3 (word)+
   */
  static srl::TreePtr parse(const char * sentence);

  /**
   * Reranking oracle system using the given sent of GOLD frames
   * The oracle calculates the distribution of correct candidates and 
   *   the upper limit of the system score
   */
  static srl::TreePtr oracleParse(const char * sentence,
				  srl::OracleSentence * goldFrames);

  /**
   * Displays the args of this tree in CoNLL format
   * @param showProbabilities If true displays an extra column for each 
   *                          predicate with the classifiers' probabilities
   *                          for each generated argument
   */
  static void displayProps(const srl::TreePtr & tree,
			   std::ostream & os,
			   bool showProbabilities);

  /**
   * Displays the tree in extended CoNLL format
   */
  static void serialize(const srl::TreePtr & tree,
			const char * sentence, 
			std::ostream & os);

 private:
  /** If false, do case-insensitive processing */
  static bool mCaseSensitive;

  /** Keeps track if the system was already initialized */
  static bool mIsInitialized;
};
