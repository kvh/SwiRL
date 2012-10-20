
#include "BankTreeProducer.h"
#include "EdgeParser.h"
#include "CharUtils.h"

using namespace std;
using namespace srl;

BankTreeProducer::BankTreeProducer()
{
}

static void gatherVerbParticles(vector<TreePtr> sentence)
{
  for(size_t i = 0; i < sentence.size() - 1; i ++){
    if(sentence[i]->getLabel().substr(0, 2) == "VB" &&
       (// sentence[i + 1]->getLabel().substr(0, 2) == "IN" ||
	sentence[i + 1]->getLabel().substr(0, 2) == "RP")){
      Lexicon::addVerbParticle(sentence[i]->getHeadLemma(),
			       sentence[i + 1]->getHeadLemma());
    }
  }
}

bool BankTreeProducer::gatherStats(IStream & treeStream)
{
  EdgeParser<Tree> parser(treeStream);
  TreePtr edge;

  try{
    if((edge = parser.parseEdge(true)) == (const Tree *) NULL){
      // failed to read a parse tree
      return false;
    }
  
    // remove edges without children, or empty terminals, or -NONE-
    removeEmptyNodes(edge);

    // detect lemmas if not already set and their counts
    edge->detectLemmas(true);

    edge->gatherLabelStats();

    // extract sequence of terminals
    vector<TreePtr> sentence;
    generateSentence(edge, sentence);

    gatherVerbParticles(sentence);

  } catch(...){
    cerr << "Exception caught!" << endl;// << e.getMessage() << endl;
    exit(1);
  }

  return true;
}

TreePtr BankTreeProducer::analyze(IStream & treeStream)
{
  EdgeParser<Tree> parser(treeStream);
  TreePtr edge;

  try{
    if((edge = parser.parseEdge(true)) == (const Tree *) NULL){
      // failed to read a parse tree
      return TreePtr();
    }
  
    // remove edges without children, or empty terminals, or -NONE-
    removeEmptyNodes(edge);

    // sentence terminal positions
    edge->setPositions();

    // detect B-A nodes for all I-As
    edge->detectArgumentBegins(edge.operator->());

    // detect the types of B labels: single, term, incorrect
    edge->detectBTypes();

    edge->verifyTokenDistances();

    // are arguments included in other args of same pred? ==> NO!
    //edge->checkArgumentInclusion();

    // are preds included in args? ==> NO!
    //edge->checkPredicateInclusion(); 

    // are I-A at the same level as B-I?
    //edge->checkIAlignment();

    // count ratios of B vs I args
    //edge->countBIs();

    // extract sequence of terminals
    vector<TreePtr> sentence;
    generateSentence(edge, sentence);

    // Lluis' pruning heuristics
    edge->checkLluis(sentence);
    // Palmer's pruning  heuristic
    edge->checkPalmer(sentence);

    // count how many I-A are commas
    //edge->countCommaIs(sentence);

    cout << edge << endl;
  } catch(...){
    cerr << "Exception caught!" << endl;// << e.getMessage() << endl;
    exit(1);
  }

  return edge;
}

void BankTreeProducer::preprocess(srl::TreePtr edge,
				  vector<TreePtr> & sentence,
				  bool caseSensitive)
{
  // remove edges without children, or empty terminals, or -NONE-
  removeEmptyNodes(edge);
  
  // cout << "Tree initially:" << endl << * edge; cin.get();
  
  // Collins head heuristics
  edge->setHead();
  
  // Surdeanu, ACL2003 content word heuristics
  // head must be set!
  edge->detectContentWords();
  
  // sentence terminal positions
  edge->setPositions();
  
  // detect lemmas if not already set
  edge->detectLemmas(false);
  
  // extract sequence of terminals
  sentence.clear();
  generateSentence(edge, sentence);
  
  // detect voice
  edge->detectPredicateVoices(sentence);
  
  // detect auxiliary verbs (needed for the distance features)
  edge->detectAuxVerbs();
  
  // detect POS contexts
  edge->detectPOSContexts(sentence);
  
  // detect temporal-specific terminals
  edge->detectTemporalTerminals();
  
  // mark the inclusion of several semantic categories
  edge->detectIncludes();
  
  // count commas
  edge->countCommas();
  
  // detect the governing category for NPs: S or VP?
  edge->detectGoverningCategory();
  
  // detect the sequence of children labels
  edge->detectChildrenPath();
  
  // detect sibling contexts
  edge->detectSiblingPaths();
  
  // detect head word suffixes
  edge->detectSuffixes(caseSensitive);
  
  // detect B-A nodes for all I-As
  edge->detectArgumentBegins(edge.operator->());
  
  // detect the types of B labels: single, term, incorrect
  edge->detectBTypes();
  
  // detect left/right siblings for all edges
  edge->detectSiblings();
  
  // generate the feature sets for each phrase
  // here we detect only local arg/pred features
  // path-based features are detected outside, as they depend on the PA tuple
  edge->generateFeatureSets(sentence, caseSensitive);
}

TreePtr BankTreeProducer::create(IStream & treeStream, 
				 vector<TreePtr> & sentence,
				 bool caseSensitive)
{
  EdgeParser<Tree> parser(treeStream);
  TreePtr edge;

  try{
    if((edge = parser.parseEdge(false)) == (const Tree *) NULL){
      // failed to read a parse tree
      return TreePtr();
    }
  
    // preprocess the current tree for SRL
    preprocess(edge, sentence, caseSensitive);

  } catch(...){
    cerr << "Exception caught!" << endl;// << e.getMessage() << endl;
    exit(1);
  }

  return edge;
}

TreePtr BankTreeProducer::read(IStream & treeStream)
{
  EdgeParser<Tree> parser(treeStream);
  TreePtr edge;

  try{
    if((edge = parser.parseEdge(false)) == (const Tree *) NULL){
      // failed to read a parse tree
      return TreePtr();
    }
  } catch(...){
    cerr << "Exception caught!" << endl;// << e.getMessage() << endl;
    exit(1);
  }

  return edge;
}

bool BankTreeProducer::isEmptyNode(const srl::TreePtr & edge) 
{
  static const Char * empty [] = { 
    W("-NONE-"), NULL
  };

  if(edge->getChildren().empty() == true &&
     edge->getWord().empty() == true){
    return true;
  }

  for(int i = 0; empty[i] != NULL; i ++){
    if(edge->getLabel() == empty[i]){
      return true;
    }
  }

  return false;
}

void BankTreeProducer::removeEmptyNodes(srl::TreePtr & edge) 
{
  for(list<TreePtr>::iterator it = edge->getMutableChildren().begin();
      it != edge->getMutableChildren().end(); it ++){
    removeEmptyNodes(* it);
  }

  for(list<TreePtr>::iterator it = edge->getMutableChildren().begin();
      it != edge->getMutableChildren().end(); ){
    if(isEmptyNode(* it)){
      // wcout << "Removing empty child from:" << endl;
      // edge->display(wcout); wcout << endl;
      // wcout << "The edge removed is:" << endl;
      // (* it)->display(wcout); wcout << endl;
      it = edge->getMutableChildren().erase(it);
    } else{
      it ++;
    }
  }
}

void BankTreeProducer::
generateSentence(const srl::TreePtr & edge,
		 std::vector<srl::TreePtr> & sentence) 
{
  if(edge->isTerminal()){

    //
    // This method might be called before empty nodes are removed
    // Skip empty nodes
    //
    if(isEmptyNode(edge) == false){
      sentence.push_back(edge);
    }
  }

  for(list<TreePtr>::const_iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    generateSentence(* it, sentence);
  }
}
