/**
 * Interface to the SRL classifier 
 */

#include "Swirl.h"
#include "ParserApi.h"
#include "Constants.h"
#include "Parameters.h"
#include "CharUtils.h"
#include "Lexicon.h"
#include "AssertLocal.h"
#include "Exception.h"
#include "Logger.h"
#include "Wnet.h"
#include "BankTreeProducer.h"

using namespace std;
using namespace srl;

bool Swirl::mCaseSensitive = true;
bool Swirl::mIsInitialized = false;

bool Swirl::initialize(const char * srlDataDirectory,
		       const char * parserDataDirectory,
		       bool caseSensitive)
{
  if(mIsInitialized){
    cerr << "Already initialized.\n";
    return false;
  }

  // load SRL models
  cerr << "Loading models..." << endl;
  Tree::loadClassifierModels(srlDataDirectory); 
  mCaseSensitive = caseSensitive;

  // create morpher object
  try {
    RCIPtr<Morpher> wn = RCIPtr<Morpher>(new WordNet);
    Tree::registerMorpher(wn);
  } catch(...){
    cerr << "Morpher initialization failed. Exiting...\n";
    return false;
  }

  // initialize SRL lexicon
  Lexicon::initialize(srlDataDirectory, true);

  // initialize Charniak's parser
  if(! ParserApi::initialize(parserDataDirectory, ! caseSensitive)){
    cerr << "Parser initialization failed. Exiting...\n";
    return false;
  }

  // reset oracle stats, if necessary
  Tree::resetOracleStats();

  mIsInitialized = true;
  return true;
}

/**
 * Propagates the NE labels from SwirlTokens to Tree terminals
 */
static void setNeLabels(TreePtr tree,
			const std::vector<SwirlToken> & sentence,
			size_t & position)
{
  if(tree->isTerminal()){
    RVASSERT(position < sentence.size(),
	     "Missmatch between tree and sentence size!");
    tree->setNe(sentence[position].getNe());
    position ++;
    return;
  }

  for(list<TreePtr>::iterator it = tree->getMutableChildren().begin();
      it != tree->getMutableChildren().end(); it ++){
    setNeLabels(* it, sentence, position);
  }
}

/**
 * Propagates the predicate flags from SwirlTokens to Tree terminals
 */
static void setPredicateFlags(TreePtr tree,
			      const std::vector<SwirlToken> & sentence,
			      size_t & position)
{
  if(tree->isTerminal()){
    RVASSERT(position < sentence.size(),
	     "Missmatch between tree and sentence size!");
    tree->setIsPredicate(sentence[position].getPred());
    position ++;
    return;
  }

  for(list<TreePtr>::iterator it = tree->getMutableChildren().begin();
      it != tree->getMutableChildren().end(); it ++){
    setPredicateFlags(* it, sentence, position);
  }
}

static bool includesChildWithLabel(TreePtr tree,
				   const string & label)
{
  for(list<TreePtr>::const_iterator it = tree->getChildren().begin();
      it != tree->getChildren().end(); it ++){
    if((* it)->getLabel() == label) return true;
  }
  return false;
}

/**
 * Detects possible predicates using simple heuristics
 */
static void detectPredicatesOnline(TreePtr tree)
{
  if(tree->isTerminal()){
    // VB* that are head words of VPs, which do not include other VPs
    if(tree->getLabel().substr(0, 2) == "VB" &&
       tree->getParent()->getLabel() == "VP" &&
       includesChildWithLabel(tree, "VP") == false){
      //cerr << "Found predicate: " << tree << "\n";
      tree->setIsPredicate(true);
    }

    // VB* included in an NP, which has a different head
    else if(tree->getLabel().substr(0, 2) == "VB" &&
	    tree->getParent()->getLabel() == "NP" &&
	    tree->getParent()->getHead() != tree){
      tree->setIsPredicate(true);
    }

    /*
    // JJ* included in an NP, which has a different head
    else if(tree->getLabel().substr(0, 2) == "JJ" &&
	    tree->getParent()->getLabel() == "NP" &&
	    tree->getParent()->getHead() != tree){
      tree->setIsPredicate(true);
    }
    */
  }

  for(list<TreePtr>::iterator it = tree->getMutableChildren().begin();
      it != tree->getMutableChildren().end(); it ++){
    detectPredicatesOnline(* it);
  }
}

/**
 * Replaces all white spaces with "_"
 * This is needed to avoid that Charniak tokenizes out tokens again
 */
static String normalizeSpaces(const String & str)
{
  ostringstream os;
  for(size_t i = 0; i < str.length(); i ++){
    if(str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
      os << "_";
    else
      os << str[i];
  }
  return os.str();
}

srl::TreePtr Swirl::parse(const std::vector<SwirlToken> & sentence,
			  bool detectPredicates,
			  OracleSentence * goldFrames)
{
  std::vector<srl::TreePtr> sentenceTerminals;

  try {
    //
    // construct the parser input as a sequence of words
    //
    ostringstream parserInput;
    for(size_t i = 0; i < sentence.size(); i ++){
      parserInput << normalizeSpaces(sentence[i].getWord()) << " ";
    }
    //cerr << "Parsing sentence: *" << parserInput.str() << "*\n";
    
    //
    // actual parsing
    //
    TreePtr tree = ParserApi::parse(parserInput.str().c_str());
    if(tree == (const Tree *) NULL) return tree;
    //cerr << "After parsing...\n" << tree << "\n";
    
    //
    // set the NE labels to all tree terminals
    //
    //cerr << "Setting NEs...\n";
    size_t position = 0;
    setNeLabels(tree, sentence, position);
    RVASSERT(position == sentence.size(), 
	     "Missmatch between tree and sentence size!");

    //
    // propagate/detect the predicate flags
    //
    //cerr << "Setting predicate flags...\n";
    if(detectPredicates == false){
      position = 0;
      // propagate the predicate flags from SwirlTokens to Tree
      setPredicateFlags(tree, sentence, position);
      RVASSERT(position == sentence.size(), 
	       "Missmatch between tree and sentence size!");
    } else {
      // detect possible predicates online
      detectPredicatesOnline(tree);
    }

    //
    // preprocess the tree for SRL
    //
    //cerr << "Starting preprocess...\n";
    BankTreeProducer::preprocess(tree, sentenceTerminals, mCaseSensitive);
    RVASSERT(sentenceTerminals.size() == sentence.size(), 
	     "Invalid sentence terminal count after preprocessing!");
    //cerr << "Parsed sentence:\n" << tree << "\n";

    //
    // if running in oracle mode set the GOLD arguments 
    //   => needed later in the frame selection process
    //
    if(goldFrames != NULL){
      goldFrames->setGoldArguments(tree.operator->());
      tree->setOracleMode(true);
      //cerr << "TREE WITH GOLD ARGS:\n";
      //tree->display(cerr);
      //cerr << "\n";
    }

    //
    // classify args for all preds
    //
    tree->classify(sentenceTerminals, 0, mCaseSensitive);
    //cerr << "Parsed tree completed:\n" << tree << "\n";
    return tree;
  } catch(Exception e){
    cerr << "SRL system failed with the exception: " << e.getMessage() << "\n";
    return TreePtr();
  }

  return TreePtr();
}

static bool extractTokens(const char * sentence,
			  vector<SwirlToken> & srlTokens,
			  bool & detectPredicates)
{
  vector<string> tokens;
  tokenizeWithQuotes(sentence, tokens, " \t\n\r");
  //cerr << "Parsing sentence: " << sentence << "\n";
  //cerr << "Tokens:";
  //for(size_t i = 0; i < tokens.size(); i ++) cerr << " *" << tokens[i] << "*";
  //cerr << "\n";

  //
  // do not detect predicates in this format. 
  // predicates come preset with their lemmas
  //
  if(tokens[0] == "0"){
    // incorrect format
    if(tokens.size() < 1 || (tokens.size() - 1) % 3 != 0) return false;

    for(size_t i = 1; i < tokens.size(); i += 3){
      string word = tokens[i];
      string ne = tokens[i + 1];
      if(ne == "O") ne = "";
      bool pred = false;
      string lemma;
      if(tokens[i + 2] != "-"){
	pred = true;
	lemma = tokens[i + 2];
      }

      srlTokens.push_back(SwirlToken(word, lemma, "", ne, pred));
    }

    detectPredicates = false;
  }

  //
  // detect predicates in this format
  // this format includes NEs and POS tags
  //
  else if(tokens[0] == "1"){
    // incorrect format
    if(tokens.size() < 1 || (tokens.size() - 1) % 3 != 0) return false;

    for(size_t i = 1; i < tokens.size(); i += 3){
      string word = tokens[i];
      string pos = tokens[i + 1];
      string ne = tokens[i + 2];
      if(ne == "O") ne = "";

      srlTokens.push_back(SwirlToken(word, "", pos, ne, false));
    }

    detectPredicates = true;
  }

  //
  // detect predicates in this format
  // this format includes NEs
  //
  else if(tokens[0] == "2"){
    // incorrect format
    if(tokens.size() < 1 || (tokens.size() - 1) % 2 != 0) return false;

    for(size_t i = 1; i < tokens.size(); i += 2){
      string word = tokens[i];
      string ne = tokens[i + 1];
      if(ne == "O") ne = "";

      srlTokens.push_back(SwirlToken(word, "", "", ne, false));
    }

    detectPredicates = true;
  }

  //
  // detect predicates in this format
  // this format does NOT include NEs nor POS tags
  //   ==> may function worse if SRL system trained with NEs
  //
  else if(tokens[0] == "3"){
    // incorrect format
    if(tokens.size() < 1) return false;

    for(size_t i = 1; i < tokens.size(); i ++){
      string word = tokens[i];
      srlTokens.push_back(SwirlToken(word, "", "", "", false));
    }

    detectPredicates = true;
  }

  // invalid format
  else{
    return false;
  }

  return true;
}

srl::TreePtr Swirl::parse(const char * sentence)
{
  vector<SwirlToken> srlTokens;
  bool detectPredicates = false;

  if(extractTokens(sentence, srlTokens, detectPredicates) == false){
    return TreePtr();
  }

  /*
  cerr << "Parsing sentence:";
  for(size_t i = 0; i < srlTokens.size(); i ++){
    cerr << " " << srlTokens[i].getWord() << "/" 
	 << srlTokens[i].getLemma() << "/"
	 << srlTokens[i].getPred();
  }
  cerr << "\n";
  */

  TreePtr tree = parse(srlTokens, detectPredicates, NULL);
  
  // XXX: build backup parse if parse failed here!

  return tree;
}

srl::TreePtr Swirl::oracleParse(const char * sentence,
				OracleSentence * goldFrames)
{
  vector<SwirlToken> srlTokens;
  bool detectPredicates = false;

  if(extractTokens(sentence, srlTokens, detectPredicates) == false){
    return TreePtr();
  }

  TreePtr tree = parse(srlTokens, detectPredicates, goldFrames);
  return tree;
}

static void extractTerminals(TreePtr tree,
			     vector<TreePtr> & terminals)
{
  if(tree->isTerminal()){
    terminals.push_back(tree);
    return;
  }

  for(list<TreePtr>::iterator it = tree->getMutableChildren().begin();
      it != tree->getMutableChildren().end(); it ++){
    extractTerminals(* it, terminals);
  }
}

void Swirl::displayProps(const srl::TreePtr & tree,
			 std::ostream & os,
			 bool showProbabilities)
{
  vector<TreePtr> terminals;
  extractTerminals(tree, terminals);
  tree->dumpCoNLL(os, terminals, showProbabilities);
}

void Swirl::serialize(const srl::TreePtr & tree,
		      const char * sentence,
		      std::ostream & os)
{
  if(tree != (const Tree *) NULL){
    vector<TreePtr> terminals;
    extractTerminals(tree, terminals);
    tree->dumpExtendedCoNLL(os, terminals);
  } else {
    vector<SwirlToken> srlTokens;
    bool detectPredicates = false;
    RVASSERT(extractTokens(sentence, srlTokens, detectPredicates),
	     "Failed to tokenize SRL input!");

    for(size_t i = 0; i < srlTokens.size(); i ++){
      // pos tree lemma pred
      os << "* * ";
      String lemma;
      if(srlTokens[i].getPos().empty()){ // not enough info to get lemma
	lemma = srlTokens[i].getWord();
      } else{
	RVASSERT(Tree::getMorpher() != (const Morpher *) NULL, 
		 "Undefined morpher!");
	lemma = Tree::getMorpher()->morph(srlTokens[i].getWord(),
					  srlTokens[i].getPos());
      }
      os << quotify(lemma) << "\t\t0" << endl;
    }
    os << endl;
  }
}
