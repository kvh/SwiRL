/**
 * Provides a generic interface towards the Charniak parser
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <math.h>

#include "GotIter.h"
#include "Wrd.h"
#include "InputTree.h"
#include "Bchart.h"
#include "ECArgs.h"
#include "MeChart.h"
#include "extraMain.h"
#include "AnsHeap.h"
#include "UnitRules.h"
#include "TimeIt.h"
#include "ewDciTokStrm.h"
#include "Link.h"
#include "ParserApi.h"

using namespace std;
using namespace srl;

int ParserApi::MAX_SENT_LEN = 150;

MeChart* curChart;

/** Initialize the parser */
bool ParserApi::initialize(const char * dataDirectory,
			   bool caseInsensitive)
{
  // don't tokenize input => it comes pretokenized in the API
  Bchart::tokenize = false;

  // do we handle case insensitive test?
  Bchart::caseInsensitive = caseInsensitive;

  // how many solutions to produce
  Bchart::Nth = 1;

  // max sentence length in words
  MAX_SENT_LEN = 150;

  // other options may be set here, see Params::init()
  
  // initialize the data repository and the chart
  cerr << "Initializing parser...\n";
  ECString path(dataDirectory);
  generalInit(path + ECString("/"));

  return true;
}

bool ParserApi::parse(const char * sentence,
		      int howManySolutions,
		      std::vector<srl::TreePtr> & outputTrees,
		      std::vector<double> & probabilities)
{
  // nothing to parse
  if(sentence == NULL) return false;

  // how many solutions should we produce?
  Bchart::Nth = howManySolutions;

  // construct the SGML input stream
  ECString buffer = ECString("<s> ") + ECString(sentence) + ECString(" </s>");
  //cerr << "ParserApi buffer: " << buffer << "\n";
  istringstream is(buffer);
  
  SentRep* srp = new SentRep(is, SentRep::SGML);
  //cerr << "SentRep: " << *srp << endl;

  double log600 = log2(600.0);
  int len = srp->length();
  
  // found a sentence too large
  if(len > MAX_SENT_LEN){
    delete srp;
    cerr << "ParserApi::parse(): sentence too large: " << len << "!\n";
    return false;
  }
  
  // found empty sentence
  if(len == 0){
    delete srp;
    cerr << "ParserApi::parse(): empty sentence!\n";
    return false;
  }

  // construct the chart
  MeChart* chart = new MeChart( *srp );
  curChart = chart;
       
  // parse it!
  //cerr << "Attempting to parse...\n";
  chart->parse( );

  // do we have a sentence?
  Item* topS = chart->topS();
  if(!topS){
    if(!Bchart::silent) {
      cerr << "Parse failed for sentence:" << endl;
      cerr << *srp << endl;
    }
    delete srp;
    delete chart;
    return false;
  }
  
  // compute the outside probabilities on the items so that we can
  // skip doing detailed computations on the really bad ones 
  chart->set_Alphas();

  Bst& bst = chart->findMapParse();
  if( bst.empty()){
    if(!Bchart::silent){
      cerr << "Parse failed for sentence:" << endl;
      cerr << *srp << endl;
    }
    delete chart;
    delete srp;
    return false;
  }
  
  // language modeling junk...
  if(Feature::isLM) {
    //double lgram = log2(bst.sum());
    //lgram -= (srp->length()*log600);
    //double pgram = pow(2,lgram);
    //double iptri =chart->triGram();;
    //double ltri = (log2(iptri)-srp->length()*log600);
    //double ptri = pow(2.0,ltri);
    //double pcomb = (0.667 * pgram)+(0.333 * ptri);
    //double lmix = log2(pcomb);
    //cout << lgram << "\t" << ltri << "\t" << lmix << "\n";
  }

  int numVersions = 0;
  Link diffs(0);
  int numDiff = 0;
  //cerr << "Need num diff: " << Bchart::Nth << endl;
  vector<InputTree*> saveTrees;
  saveTrees.reserve(Bchart::Nth);
  vector<double> saveProbs;
  saveProbs.reserve(Bchart::Nth);
  for(numVersions = 0 ; ; numVersions++) {
    short pos = 0;
    Val* v = bst.next(numVersions);
    if(!v) break;
    InputTree* mapparse=inputTreeFromBsts(v,pos,*srp);
    bool isU;
    diffs.is_unique(mapparse, isU);
    //cerr << "V " << numVersions << " " << isU << " " << v->prob() << "\n" << *mapparse << endl;
    if(isU){
      saveTrees.push_back(mapparse);
      saveProbs.push_back(v->prob());
      numDiff++;
    } else {
      delete mapparse;
    }
    if(numDiff >= Bchart::Nth) break;
  }

  //
  // print all solutions
  //
  for(int i = 0 ; i < numDiff ; i++) {
    InputTree*  mapparse = saveTrees[i];
    double logP =log(saveProbs[i]);
    logP -= (srp->length()*log600);
    probabilities.push_back(logP);
    TreePtr out = mapparse->convertToTree();
    outputTrees.push_back(out);
    delete mapparse;
  }

  delete chart;
  delete srp;
  return true;
}

TreePtr ParserApi::parse(const char * sentence)
{
  vector<TreePtr> trees;
  vector<double> probs;
  parse(sentence, 1, trees, probs);
  if(trees.size() > 0) return trees[0];
  return TreePtr();
}

srl::TreePtr InputTree::convertToTree()
{
  if( word_.length() != 0 ){
    TreePtr tree(new Tree(word_, term_));
    return tree;
  }
  
  TreePtr tree(new Tree(term_)); // XXX: what is the ntInfo_???
  for(ConstInputTreesIter  subTreeIter= subTrees_.begin(); 
      subTreeIter != subTrees_.end() ; subTreeIter ++) {
    InputTree * subTree = *subTreeIter;
    TreePtr child = subTree->convertToTree();
    tree->addChild(child);
    child->setParent(tree);
  }

  return tree;
}

