
#ifndef SRL_TREE_H
#define SRL_TREE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <values.h>

#include "RCIPtr.h"
#include "Wide.h"
#include "Morpher.h"
#include "Argument.h"
#include "Classifier.h"
#include "StringMap.h"
#include "ClassifiedArg.h"
#include "PathFeatures.h"

namespace srl {

/**
 * Voice types
 */
typedef enum VerbType { 
  VERB_ACTIVE,     // 0
  VERB_PASSIVE,    // 1
  VERB_COPULATIVE, // 2
  VERB_INFINITIVE, // 3
  VERB_GERUND      // 4
};

/**
 * Stores the value of a feature and its index in the dictionary.
 * Needed only for debug purposes, ignore in the API.
 */
class DebugFeature {
 public:
  DebugFeature(const String & f, int i) : _name(f), _index(i) {}

  String _name;
  int _index;
};

/**
 * Scoring statistics.
 * Only used when parsing TreeBanks. Ignore in the API.
 */
class Stats {
 public:
  Stats() : correct(0), excess(0), missed(0) {}

  int correct;
  int excess;
  int missed;
};

/**
 * Statistics for argument types
 * Only used by the corpus analysis code (Analysis.cc). Ignore in the API.
 */
class ArgCounts {
 public:
  ArgCounts() : b(0), bwi(0), inci(0), cori(0) {}

  int b;
  int bwi; // B that has at least an I
  int inci;
  int cori;
  int termi; // B-I chain in a pre-terminal
};

/**
 * Stores the complete syntactic tree of a sentence.
 * Each node has associated a list of semantic roles. 
 *   Fetch the argument list of a given node with getPredictedArguments().
 */
class Tree : public RCObject {

 public:

  /** C'tor */
  Tree() : _head(NULL), _content(NULL), _parent(NULL), 
    _leftSibling(NULL), _rightSibling(NULL), 
    _verbType(VERB_ACTIVE), 
    _leftPosition(-1), _rightPosition(-1),
    _isPredicate(false), _isAuxVerb(false), _oracleMode(false) {}

  /** C'tor */
  Tree(const String & word,
       const String & label): 
    _word(word), _label(label), _head(NULL), _content(NULL), _parent(NULL),
    _leftSibling(NULL), _rightSibling(NULL), 
    _verbType(VERB_ACTIVE), _leftPosition(-1), _rightPosition(-1),
    _isPredicate(false), _isAuxVerb(false), _oracleMode(false) {} 
  
  /** C'tor */
  Tree(const String & label):
    _label(label), _head(NULL), _content(NULL), _parent(NULL),
    _leftSibling(NULL), _rightSibling(NULL), 
    _verbType(VERB_ACTIVE), _leftPosition(-1), _rightPosition(-1),
    _isPredicate(false), _isAuxVerb(false), _oracleMode(false) {}

  /** Fetches the token word, for terminal nodes */
  const String & getWord() const { return _word; };

  /** Sets the token word, for terminal nodes */
  void setWord(const String & w) { _word = w; };

  /** Fetches the node syntactic label */
  const String & getLabel() const { return _label; };  

  /** Sets the node syntactic label */
  void setLabel(const String & label) { _label = label; };

  /** Fetches the head node pointer */
  const RCIPtr<srl::Tree> & getHead() const { return _head; };

  /** 
   * Fetches the head word of this phrase. 
   * This is a recursive method that descends the chain of head pointers.
   */
  const String & getHeadWord() const;
  /** Fetches the suffixes of the head word of this phrase */
  const std::vector<String> & getHeadSuffixes() const;
  
  /** Returns the position of the terminal in the sentence */
  short getHeadPosition() const;

  /** Returns the position of the immediate head phrase in the children list */
  int getHeadChildrenPosition() const;

  /** 
   * Fetches the head lemma for this phrase. 
   * This is a recursive method that descends the chain of head pointers.
   * Not const because lemmas are detected on the fly.
   */
  const String & getHeadLemma();

  /** Fetches the NE label of the head terminal for this phrase */
  const String & getHeadNe() const;

  /** Returns the POS tag associated with the head word */
  const String & getHeadTag() const;

  /** Sets the head node of this phrase */
  void setHead(const RCIPtr<srl::Tree> & h) { _head = h; };

  /** Sets the head word for all nodes in this phrase using Collins' heuristics */
  void setHead();

  /** Detects content words, see Surdeanu et al, ACL 2003 */
  void detectContentWords();

  /** Fetches the pointer to the content word for this phrase */
  const RCIPtr<srl::Tree> & getContent() const { return _content; };

  /** Fetches the content word for this phrase */
  const String & getContentWord() const;
  /** Fetches the position of the content word for this phrase */
  short getContentPosition() const;
  /** Fetches the lemma of the content word for this phrase */
  const String & getContentLemma();
  /** Fetches the NE label of the content word for this phrase */
  const String & getContentNe() const;
  /** Fetches the POS tag of the content word for this phrase */
  const String & getContentTag() const;
  /** Fetches the suffixes of the content word for this phrase */
  const std::vector<String> & getContentSuffixes() const;

  /** Sets the parent of this phrase */
  void setParent(const RCIPtr<srl::Tree> & p) { _parent = p.operator->(); };
  /** Sets the parent of this phrase */
  void setParent(srl::Tree * p) { _parent = p; };
  /** Fetches the parent of this phrase */
  srl::Tree * getParent() const { return _parent; };

  /** Sets the left sibling of this phrase */
  void setLeftSibling(Tree * t) 
    { _leftSibling = t; }

  /** Sets the right sibling of this phrase */
  void setRightSibling(Tree * t)
    { _rightSibling = t; }

  /** Sets the lemma of this phrase, for terminal nodes */
  void setLemma(const String & lemma) { _lemma = lemma; };

  /** 
   * Fetches the lemma for this phrase, for terminal nodes.
   * Not const because lemmas are detected on the fly.
   */
  const String & getLemma();

  /** Detect lemmas if not already set */
  void detectLemmas(bool gatherCounts);

  /** Returns true if this phrase is a terminal node */
  bool isTerminal() const { 
    return (_children.empty() == true && _word.empty() == false);
  };

  /** Returns true if this phrase is a predicate */
  bool isPredicate() const { return _isPredicate; }
  /** Sets the predicate flag */
  void setIsPredicate(bool b) { _isPredicate = b; }
  /** Propagates the predicate flag and lemma down to children */
  void propagatePredicate(const String & lemma, bool flag);

  /** Returns true if this phrase is an auxiliary verb */
  bool isAuxVerb() const { return _isAuxVerb; }
  /** Sets the auxiliary verb flag */
  void setIsAuxVerb(bool b) { _isAuxVerb = b; }

  /** Adds a child to this phrase */
  void addChild(const RCIPtr<srl::Tree> & c,
		bool atEnd = true) { 
    if(atEnd == true)_children.push_back(c); 
    else _children.push_front(c);
  };

  /** Fetches the complete list of immediate children (read-only) */
  const std::list< RCIPtr<srl::Tree> > & getChildren() const 
    { return _children; };

  /** 
   * Fetches the complete list of immediate children (read-write) 
   * Note: Use with care! Update _head if children are changed!
   */
  std::list< RCIPtr<srl::Tree> > & getMutableChildren() 
    { return _children; };

  /** 
   * Fetches the complete list of immediate children (read-write)
   * Note: Use with care! Update _head if children are changed!
   */
  std::list< RCIPtr<srl::Tree> > & children() 
    { return _children; };

  /** Returns the voice of this verb */
  const srl::VerbType & getVerbType() const { return _verbType; };
  /** Sets the voice for this verb */
  void setVerbType(const srl::VerbType & vt) { _verbType = vt; };

  /** Sets the NE (BIO CoNLL style) of this phrase */
  void setNe(const String & ne) { _ne = ne; };
  /** Fetches the BIO NE label of this phrase */
  const String & getNe() const { return _ne; };

  /** Resets the info stored in this node */
  void clear() {
    _word = W("");
    _lemma = W("");
    _label = W("");
    _children.clear();
    _head = RCIPtr<srl::Tree>();
    _content = RCIPtr<srl::Tree>();
  };

  /** Is this sentence position included in this phrase? */
  bool includesPosition(int pos) const
    { return (pos >= getLeftPosition() && pos <= getRightPosition()); }

  /**
   * Verifies if this includes the other Tree
   */
  bool includes(const Tree * other) const {
    if(includesPosition(other->getLeftPosition()) &&
       includesPosition(other->getRightPosition())) return true;
    return false;
  }
  
  /**
   * Pretty display with gold arguments, useful when training
   */
  void display(OStream & os,
	       bool isHead = false, 
	       bool isContent = false,
	       int offset = 0) const;

  /**
   * Displays the tree with all the relevant information included.
   * This method produces the output for the Java API
   */
  void serialize(OStream & os,
		 int offset = 0);

  /**
   * Treebank-style display 
   */
  void displayParens(OStream & os,
		     bool showHead = true) const;

  /**
   * Treebank-style display, with children indented for easier reading
   */
  void displayPrettyParens(OStream & os,
			   int offset = 0,
			   bool showHead = false) const;

  /**
   * Displays the fully-annotated tree in Treebank format.
   * This format is the required input to swirl_train and swirl_classify
   */
  void displayTreebank(OStream & os) const;

  /**
   * Converts the CoNLL annotations into a Treebank-style parse tree.
   * This format is the required input to swirl_train and swirl_classify
   */
  static RCIPtr<srl::Tree> convert(IStream & wordsStream, 
				   IStream & nesStream, 
				   IStream & syntStream, 
				   IStream & propsStream,
				   bool & hasPredicates);

  /** Detects if this phrase contains any argument that maps incorrectly */
  bool ugglyPhrase() const;

  /** 
   * Sets the _position fields for all terminal edges in this phrase 
   * The first token gets position 1
   * Returns the number of terminals in this sentence
   */
  short setPositions();

  /** Returns the left-most sentence position for this phrase */
  short getLeftPosition() const { return _leftPosition; };
  /** Returns the right-most sentence position for this phrase */
  short getRightPosition() const { return _rightPosition; };

  /** Adds a new gold argument for this phrase (used in training) */
  void addArgument(const String & a) { _arguments.push_back(Argument(a)); };
  /** Adds a new gold argument for this phrase (used in training) */
  void addArgument(const Argument & a) { _arguments.push_back(a); }
  /** Adds several gold arguments to this phrase (used in training) */
  void setArguments(const std::list<Argument> & l) { _arguments = l; };
  const std::list<Argument> & getGoldArguments() { return _arguments; };

  /** Adds a new predicated argument for this phrase (used when testing) */
  void addArgPrediction(const Argument & arg) 
    { _predictedArguments.push_back(arg); }

  /**
   * Returns the list of predicted arguments for this phrase 
   */
  const std::list<srl::Argument> & getPredictedArguments() const 
    { return _predictedArguments; }

  /** Generates svm_light examples for SRL training */
  void generateExamples(const std::vector< RCIPtr<srl::Tree> > & sentence,
			bool caseSensitive);

  /** Detects auxiliary verbs */
  void detectAuxVerbs();

  /** Detects POS contexts */
  void detectPOSContexts(const std::vector< RCIPtr<srl::Tree> > & sentence);

  /** Detects temporal terminals */
  void detectTemporalTerminals();

  /** Detects inclusion of several semantic categories */
  void detectIncludes();
  void incLabelCount(const Char * lab, int increment);
  bool hasTemporalModifier() const;
  int countDTIgnore() const;

  /** Detects verb voices for terminals marked as predicates */
  void detectPredicateVoices(const std::vector< RCIPtr<srl::Tree> > & sent);

  /** Counts commas included in this phrase */
  void countCommas();

  /** Detects the governing category for NPs: S or VP? */
  void detectGoverningCategory();

  /** Detects the sequence of children labels */
  void detectChildrenPath();

  /** Detects sibling contexts, of length 1, 2, 3 */
  void detectSiblingPaths();

  /** Detect head word suffixes */
  void detectSuffixes(bool caseSensitive);

  /** Detects B-A nodes for all I-As */
  void detectArgumentBegins(const Tree * top);

  /** Are arguments included in other args of same pred? */
  void checkArgumentInclusion();

  /** Detect the types of B labels: single, term, incorrect */
  void detectBTypes();
  void countBTypes(int & single,
		   int & fullTerm,
		   int & incTerm,
		   int & samep,
		   int & ugly) const;

  /** Are preds included in args? */
  void checkPredicateInclusion();

  void verifyTokenDistances();

  /** Are I-A at the same level as B-I? */
  bool checkIAlignment();

  /** Count ratios of B vs I args */
  void countBIs();
  static void reportArgCounts(std::ostream & os);
  Tree * containsBForI(const Tree * iArg, 
		       int verbPosition);

  /** Detects if this phrase is a special character, i.e. , : `` '' */
  bool isCollinsSpecial() const;

  /** 
   * Can this be an argument for this verb? 
   * This is the Marquez et al argument pruning heuristic.
   */
  bool isValidArgument(const Tree * pred,
		       const Tree * firstSParent) const;

  /** Required by the Marquez et al heuristic */
  const Tree * findParentForLluisHeuristic() const;

  /** Verify Lluis' pruning heuristics */
  bool checkLluis(const std::vector< RCIPtr<srl::Tree> > & sentence);
  void checkArgumentPositionsForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence, 
					  const Tree * pred,
					  const Tree * s);

  /**
   * Can this tree be an argument for this predicate?
   * This is the Hue & Palmer's heuristic:
   *   Acceptable arguments must be immediate children of the predicate parents
   */
  bool isValidPalmerHeuristic(const Tree * pred) const;
  bool hasChild(const Tree * child, 
		int currentDepth,
		int maxDepth) const;

  /** Verify Palmer's pruning heuristic */
  bool checkPalmer(const std::vector< RCIPtr<srl::Tree> > & sentence);
  void checkPalmerForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			       const Tree * pred);

  /** Counts how many I-A are commas */
  void countCommaIs(const std::vector< RCIPtr<srl::Tree> > & sentence);

  /** Gathers label statistics */
  void gatherLabelStats();
  
  /** Generates the phrase-specific feature sets */
  void generateFeatureSets(const std::vector< RCIPtr<srl::Tree> > & sentence,
			   bool caseSensitive);

  /** Classifies arguments for all predicates in this sentence */
  void classify(const std::vector< RCIPtr<srl::Tree> > & sentence,
		int sentenceIndex,
		bool caseSensitive);

  /** Dump tree in the CoNLL standard format */
  void dumpCoNLL(OStream & os,
		 const std::vector< RCIPtr<srl::Tree> > & sentence,
		 bool showProbabilities);

  /** 
   * Dump tree in the extended CoNLL format.
   * This format includes:
   *   lemmas for all words;
   *   the Charniak syntax.
   */
  void dumpExtendedCoNLL(OStream & os,
			 const std::vector< RCIPtr<srl::Tree> > & sentence);

  /**
   * Generates the full-syntax label for one terminal,
   *   e.g. "(S1(S(PP*)"
   */
  String generateTreeLabel() const;

  /**
   * Verifies if this phrase includes arguments for the verb at given position
   */
  bool includesArgsForVerb(int position, 
			   Argument & arg,
			   bool recursive) const;

  /**
   * Used for scoring predicted vs gold args
   */
  void gatherListOfMissedArgs(std::list<std::string> & missedArgs,
			      std::list<std::string> & excessArgs) const;

  /** Detects left/right siblings */
  void detectSiblings();

  /**
   * Finds the node that matches the given boundaries 
   */
  const Tree * findMatchingTree(int start, int end);

  /** Register a morpher, currently only WordNet supported */
  static void registerMorpher(const RCIPtr<srl::Morpher> & m)
    { _morpher = m; };
  static const RCIPtr<srl::Morpher> & getMorpher() { 
    return _morpher;
  }

  /** Creates streams for positive/negative examples (used in training) */
  static void createSampleStreams(const String & posName,
				  const String & negName);
  /** Closes the streams for positive/negative examples (used in training) */
  static void closeSampleStreams();

  /** Loads the model for the argument classifiers */
  static bool loadClassifierModels(const char * path);
  /** Fetches the classifier for the given argument label */
  static Classifier * getClassifier(const String & label);

  /** 
   * Returns true if we generated the maximum number of training examples
   */
  static bool reachedMaxExamples() 
    { return (_reachedMaxPos && _reachedMaxNeg); }

  typedef std::list< RCIPtr<srl::Tree> >::iterator iterator;

  typedef std::list< RCIPtr<srl::Tree> >::const_iterator const_iterator;

  /** Counts the number of I-* arguments that are commas */
  static int _commaICount;

  /** Fetches the scoring stats */
  static Stats * getStats(const String & arg);
  /** Generates the scoring stats */
  void gatherScoringStats() const;
  /** Reports the scoring stats */
  static void dumpScoringStats(std::ostream & os);

  /** Resets the map of path lenghts (used in corpus analysis) */
  static void initializePathLengths();
  /** Reports the map of path lenghts (used in corpus analysis) */
  static void reportPathLengths(std::ostream & os);

  /** 
   * Generates the COMPLETE FEATURE SET for one example 
   * If you want add/modify the feature set, this method and 
   *   generateFeatureSets are the entry points!
   */
  void generateExampleFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			       const Tree * pred, 
			       int position,
			       const Tree * begin,
			       bool createFeatures,
			       std::vector<int> & allFeats,
			       std::vector<DebugFeature> * debugFeats,
			       bool positive, // not used
			       bool caseSensitive) const; 

  ///////////////////////////////////////////////////////////////////////////
  //
  // The methods below are used when running in ORACLE mode
  //
  ///////////////////////////////////////////////////////////////////////////

  /**
   * Creates a GOLD arguments with the given parameters.
   * Required when parsing in ORACLE reranking mode, when we select 
   *   the candidate frames with the highest F1 scores. 
   *   Other than that not used.
   */
  bool createGoldArgument(int verbPosition,
			  const String & name,
			  int leftPosition,
			  int rightPosition);

  /** 
   * Sets the oracle mode; if true we run in oracle mode. 
   * Default value is false.
   */
  void setOracleMode(bool mode) { _oracleMode = mode; }
  bool getOracleMode() { return _oracleMode; }

  /** Resets oracle stats */
  static void resetOracleStats();

  /** Display oracle stats */
  static void printOracleStats(std::ostream & is);

 private:
  /** Pretty display of the list of arguments for this phrase */
  void displayArguments(OStream & os) const;

  /** Serialization of the list of arguments for this phrase */
  void serializeArguments(OStream & os) const;

  /** Used by setPositions() */
  void setPosition(short & current);

  /** Detects reference phrases */
  bool isReferent() const;

  /** Used in voice detection */
  short findVerbContext(const std::vector< RCIPtr<srl::Tree> > & sentence) const;
  bool validPredicateParent(const Tree * child, bool & sawSentence) const;
  void detectVoice(const Tree * modifier);

  /** Detects the content word for this phrase */
  void detectContentWord();
  const RCIPtr<srl::Tree> & getLastChild() const;
  const RCIPtr<srl::Tree> & getFirstChild() const;  
  bool isUnnecessaryNode() const;

  /** Used for path construction */
  static void constructPath(const Tree * arg, 
			    const Tree * pred,
			    PathFeatures & pf,
			    bool positive);
  bool includedIn(const Tree * p) const;

  /** Generates the phrase-specific feature sets for arguments */
  void generateArgumentFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence,
				bool caseSensitive);

  /** Generates the phrase-specific feature sets for predicates */
  void generatePredicateFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence,
				 bool caseSensitive);

  /** 
   * Generates path features between this and other 
   * other could be the predicates (most common) or 
   * the phrase labeled with B-A if we're looking at an I-A
   */
  void generatePathFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			    const Tree * other, 
			    int position,
			    std::vector<String> & pathFeats,
			    bool positive) const;

  /** Detects the predicates in this sentence */
  void detectPredicates(std::vector<int> & predPositions);
  void generateExamplesForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence, 
				    const Tree * pred, 
				    int position,
				    const Tree * predParent,
				    bool caseSensitive);

  /** Generates one POSITIVE example */
  void generatePositiveExample(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			       const Tree * pred, 
			       int position,
			       const String & type,
			       const String & name,
			       const Tree * begin,
			       bool caseSensitive);

  /** Generate one NEGATIVE example */
  void generateNegativeExample(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			       const Tree * pred, 
			       int position,
			       bool caseSensitive);

  /** Finds the beginning phrase for this argument span */
  const Tree * findBegin(const String & argName,
			 int verbPosition,
			 int argLeftPosition,
			 int argRightPosition) const;

  /** Detects the surface distance between two points */
  static void 
    detectSurfaceDistance(const std::vector< RCIPtr<srl::Tree> > & sentence,
			  int left, 
			  int right, 
			  int & vbDist, 
			  int & commaDist, 
			  int & ccDist,
			  int & tokenDist, 
			  bool & isAdjacent,
			  bool & includesReferent);

  /** Detects the sibling path between two nodes */
  String detectSiblingPath(const Tree * start,
			   const Tree * end) const;

  /**
   * Generates argument labels for all candidate phrases for a predicate
   * Used by both greedyClassification and dynamicClassification
   */
  void generateArgsForCandidates(const std::vector< RCIPtr<srl::Tree> > & sentence,
				 const Tree * predicate,
				 const std::list<String> & possibleArgLabels, 
				 const std::vector<Tree *> & candidates,
				 bool caseSensitive,
				 int countBeam,
				 double confBeam,
				 std::vector<std::vector<ClassifiedArg> *> & allArgs);

  /**
   * Fetches the list of gold args for this predicate 
   * This works only when running in ORACLE mode.
   * This method is needed to compute the oracle scores.
   */
  void fetchGoldArgsForPredicate(const Tree * predicate,
				 std::vector<ClassifiedArg> & goldArgs);

  /**
   * Greedy classification for all arguments of a given predicate
   */
  void greedyClassification(const Tree * predicate,
			    std::vector<std::vector<ClassifiedArg> *> & allArgs,
			    const std::vector<ClassifiedArg> & goldArgs);

  /**
   * Classification with global search for all arguments of a given predicate
   */
  void rerankingClassification(const Tree * predicate,
			       std::vector<std::vector<ClassifiedArg> *> & allArgs,
			       int beam,
			       const std::vector<ClassifiedArg> & goldArgs);

  /**
   * Classifies this phrase for a given predicate
   */
  void classifyForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence,
			    const Tree * predicate,
			    const std::list<String> & possibleArgLabels, 
			    bool caseSensitive,
			    int countBeam,
			    double confBeam,
			    std::vector<ClassifiedArg> & output,
			    std::vector<int> * savedFeatures);

  /**
   * Finds all possible candidates for this predicate
   */
  void findCandidatesForPredicate(const Tree * predicate,
				  const Tree * predParent,
				  std::vector<Tree *> & candidates);

  /** Simple heuristics for argument expansion */
  void expandArguments(const std::vector< RCIPtr<srl::Tree> > & sentence,
		       const Tree * predicate,
		       int sentenceIndex, 
		       int predicateIndex);
    
  /** Loads all the argument labels we handle */
  static void loadAcceptableArgumentLabels(const String & lemma,
					   std::list<String> & args);

  /** Dumps args for one predicate in IOB2 format */
  void dumpCoNLLColumnIOB(const std::vector< RCIPtr<srl::Tree> > & sentence,
			  int predicatePosition,
			  std::vector< std::vector<String> > & matrix,
			  std::vector< std::vector<double> > & probMatrix,
			  int column) const;

  /** Converts the internal IOB2 format to CoNLL's paren format */
  void convertIOB2ToParens(std::vector< std::vector<String> > & matrix,
			   int column) const;

  /** 
   * Corrects some argument boundaries using heuristics 
   * 1) Include `` in the argument if '' already included in the same arg
   */
  void correctionHeuristics(const std::vector< RCIPtr<srl::Tree> > & sentence,
			    std::vector< std::vector<String> > & matrix,
			    std::vector< std::vector<double> > & probMatrix,
			    int column) const;

  /** Detects the subcategorization rule for this phrase */
  String detectSubcatRule() const;

  /** 
   * Appends a minimal set of features extracted from this phrase.
   * Used when constructing the context of a phrase.
   */
  void addMinimalFeatures(std::vector<String> & features,
			  const String & prefix,
			  bool lexicalized,
			  bool caseSensitive);

  /**
   * Matches one argument inside the tree based on the argument boundaries
   * Used to convert CoNLL annotations to our Treebank format
   */
  bool matchArgument(const std::string & name,
		     int start,
		     int end,
		     int verbPosition);
  bool exactMatchArgument(const std::string & name,
			  int start,
			  int end,
			  int verbPosition);
  bool approxMatchArgument(const std::string & name,
			   int start,
			   int end,
			   int verbPosition);

  /**
   * Constructs the syntactic frame (see Xue and Palmer)
   * This is supported only if the predicate parent is VP
   * @return true if frame constructed, false otherwise
   */
  static bool detectSyntacticFrame(const Tree * arg,
				   const Tree * pred,
				   std::list<std::string> & frames);

 private:

  /** Copy c-tor */
  //Tree(const srl::Tree &);
  
  /** The phrase lexem, for terminal nodes */
  String _word;

  /** The phrase lemma, for terminal nodes */
  String _lemma;

  /** The phrase label, POS for terminals, TreeBank for non-terminals */
  String _label;

  /** The BIO NE label attached to this terminal */
  String _ne;

  /** Pointer to the head phrase */
  RCIPtr<srl::Tree> _head;
  /** Left-to-right iterator starting at the head */
  std::list< RCIPtr<srl::Tree> >::iterator _headIterator;
  /** Right-to-left iterator starting at the head */
  std::list< RCIPtr<srl::Tree> >::reverse_iterator _headReverseIterator;

  /** Pointer to the content phrase, see Surdeanu et al, ACL 2003 */
  RCIPtr<srl::Tree> _content;

  /** Set of childrens */
  std::list< RCIPtr<srl::Tree> > _children;

  /** Dumb parent pointer */
  Tree * _parent;

  /** Dumb pointer to left sibling */
  Tree * _leftSibling;

  /** Dumb pointer to right sibling */
  Tree * _rightSibling;

  /** Verb voice, if this phrase is a verb */
  srl::VerbType _verbType;

  /** Set of gold arguments assigned to this node (training only) */
  std::list<srl::Argument> _arguments;

  /** Set of predicted arguments assigned to this node */
  std::list<srl::Argument> _predictedArguments;

  /**
   * Position in the input token list
   * By default, this value is not set. Call setPositions() to set it
   */
  short _leftPosition;
  short _rightPosition;

  /** Predicate in the SRL task */
  bool _isPredicate;

  /** Is it an auxiliary verb? */
  bool _isAuxVerb;

  /** The POS contexts of length 3 and 5 of this predicate */
  std::vector<String> _posContexts;

  /** Set to its POS if temporal entity */
  String _temporal;

  /** Governing category */
  String _gov;

  /** Sequence of children labels */
  String _childrenPath;

  /** Left/right sibling paths, lengths 1, 2, 3 */
  std::list<String> _leftSiblingLabels;
  std::list<String> _rightSiblingLabels;

  /** List of left sibling nodes */
  std::list<const Tree *> _leftSiblingNodes;
  /** List of right sibling nodes */
  std::list<const Tree *> _rightSiblingNodes;

  /** Head word suffixes */
  std::vector<String> _suffixes;

  /** 
   * Boolean flags to mark inclusion of several semantic categories
   * PER, ORG, LOC, MISC, TMP_NN, TMP_NNP, TMP_IN, TMP_RB
   */
  std::vector<bool> _includes;
  static int INCLUDES_SIZE;

  /** Label counts for this phrase. ALL TB labels are used! */
  StringMap<int> _labelCounts;

  /** Feature set if phrase is used as argument */
  std::vector<String> _argFeats;

  /** Feature set if phrase is used as predicate */
  std::vector<String> _predFeats;

  /** Are we running in oracle mode or not? */
  bool _oracleMode;

  /** Morpher used for lemma detection */
  static RCIPtr<srl::Morpher> _morpher;

  /** Stream of positive samples (training only) */
  static std::ofstream POS;
  /** Stream of negative samples (training only) */
  static std::ofstream NEG;

  /** Have we reached the max number of positive examples? (training only) */
  static bool _reachedMaxPos;
  /** Have we reached the max number of negative examples? (training only) */
  static bool _reachedMaxNeg;

  /** The set of argument classifiers (testing only) */
  static StringMap<Classifier *> _classifiers;

  /** Scoring stats (PB testing only) */
  static StringMap<Stats *> _stats;

  /** Counts for each argument label (analysis only) */
  static StringMap<ArgCounts *> _argCounts;
  /** Fetch the counts for a given argument label */
  static ArgCounts * getArgCounts(const String & arg);

  /** Counts for every path length (analysis only) */
  static std::vector<int> _pathLengths;
};

typedef RCIPtr<srl::Tree> TreePtr;

}

OStream & operator << (OStream & os, const srl::TreePtr & tree);

OStream & operator << (OStream & os, const srl::Tree & tree);

#endif
