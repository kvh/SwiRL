
#include <iostream>
#include <string>

#include "Tree.h"
#include "Assert.h"
#include "CharUtils.h"
#include "Head.h"
#include "Lexicon.h"
#include "Logger.h"

using namespace std;
using namespace srl;

void Tree::displayTreebank(OStream & os) const
{
  os << "(" << _label << " ";

  if(isTerminal()){
    os << _word << " ";

    if(! _lemma.empty()) os << _lemma << " ";
    if(! _ne.empty()) os << _ne;
  }
  else{
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      (* it)->displayTreebank(os);
    }
  }

  for(list<Argument>::const_iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    os << " ";
    (* it).display(os, "", "");
  }

  os << ")";
}

/**
 * Constructs a chain of TreePtrs based on the CoNLL syntactic info
 * @param syntaxLabel The CoNLL syntactic label to be parsed
 * @param upLevels How many levels we have to move up after processing this chain
 */
static TreePtr makeChain(const string & syntaxLabel,
			 int & upLevels)
{
  TreePtr top;
  TreePtr current = top;

  vector<string> tokens;
  simpleTokenize(syntaxLabel, tokens, "(*)");
  for(size_t i = 0; i < tokens.size(); i ++){
    TreePtr tree(new Tree(tokens[i]));
    if(top == (const Tree *) NULL){
      top = tree;
      current = tree;
    } else {
      current->addChild(tree);
      tree->setParent(current);
      current = tree;
    }
  }

  upLevels = 0;
  for(size_t i = 0; i < syntaxLabel.size(); i ++){
    if(syntaxLabel[i] == ')') upLevels ++;
  }

  return top;
}

/**
 * Extracts the actual label from a CoNLL notation
 */
string extractConllLabel(const string & ne)
{
  int end = ne.find('*');
  RVASSERT(end > 0 && end < (int) ne.size(),
	   "Invalid CoNLL annotation: " << ne);
  return ne.substr(1, end - 1);
}

RCIPtr<srl::Tree> Tree::convert(IStream & wordsStream, 
				  IStream & nesStream, 
				  IStream & syntStream, 
				  IStream & propsStream,
				  bool & hasPredicates)
{
  char wordsLine[MAX_CONLL_LINE];
  char nesLine[MAX_CONLL_LINE];
  char syntLine[MAX_CONLL_LINE];
  char propsLine[MAX_CONLL_LINE];

  ///////////////////////////////////////////////////////////////////////
  //
  // read one sentence from the CoNLL streams
  //
  ///////////////////////////////////////////////////////////////////////

  //
  // each sentence line is a word
  // each word contains the following info, 
  // directly extracted from the CoNLL format:
  //   lexem ne pos synt props
  //
  vector< vector<string> > sentence;

  while(wordsStream.getline(wordsLine, MAX_CONLL_LINE)){
    vector<string> tokens;
    simpleTokenize(wordsLine, tokens, " \t\n\r");

    // read the NE info
    RVASSERT(nesStream.getline(nesLine, MAX_CONLL_LINE),
	     "Failed to read from the NE stream");

    // read the syntactic info
    RVASSERT(syntStream.getline(syntLine, MAX_CONLL_LINE),
	     "Failed to read from the syntactic stream");

    // read the props
    RVASSERT(propsStream.getline(propsLine, MAX_CONLL_LINE),
	     "Failed to read from the argument stream");

    // found EOS
    if(tokens.empty()) break;

    simpleTokenize(nesLine, tokens, " \t\n\r");
    simpleTokenize(syntLine, tokens, " \t\n\r");
    simpleTokenize(propsLine, tokens, " \t\n\r");

    // must have at least 5 tokens
    RVASSERT(tokens.size() > 4, "Incorrect number of tokens");
    
    // this is full info about one word
    sentence.push_back(tokens);
  }

  /*
  // display the current sentence
  for(size_t i = 0; i < sentence.size(); i ++){
    for(size_t j = 0; j < sentence[i].size(); j ++){
      cout << sentence[i][j] << " ";
    }
    cout << endl;
  }
  */

  // 
  // do we actually have predicates in this sentence?
  //
  if(sentence.size() > 0 && sentence[0].size() > 5){
    hasPredicates = true;
  } else {
    hasPredicates = false;
  }

  ///////////////////////////////////////////////////////////////////////
  //
  // construct the syntactic tree with POS, NE, and lemma annotations
  //
  ///////////////////////////////////////////////////////////////////////

  // this will store the whole tree
  TreePtr top;
  Tree * parent = NULL;
  if(sentence.size() == 0) return top;

  // the previously seen NE label in the stream
  string previousNeLabel = "";
  
  // traverse all words in sequential order
  for(size_t i = 0; i < sentence.size(); i ++){
    // the terminal corresponding to this word
    string word = sentence[i][0];
    if(word == "(" || word == "[" || word == "{") word = "LRB";
    else if(word == ")" || word == "]" || word == "}") word = "RRB";

    string label = sentence[i][2];
    if(label == "(") label = "LRB";
    else if(label == ")") label = "RRB";

    TreePtr terminal(new Tree(word, label));

    // compute the BIO NE label
    if(startsWith(sentence[i][1], "(")){
      string neLabel = extractConllLabel(sentence[i][1]);
      terminal->setNe("B-" + neLabel);
      if(! endsWith(sentence[i][1], ")")){
	previousNeLabel = neLabel;
      }
    } else {
      if(! previousNeLabel.empty()){
	terminal->setNe("I-" + previousNeLabel);
      }
      if(endsWith(sentence[i][1], ")")){
	previousNeLabel = "";
      }
    }

    // set the predicate lemma if given
    if(sentence[i][4] != "-"){
      terminal->setLemma(sentence[i][4]);
    }

    // construct the phrase chain from the syntactic info
    int upLevels = 0;
    TreePtr chain = makeChain(sentence[i][3], upLevels);
  
    // the syntactic info says we must construct some nodes
    if(chain != (const Tree *) NULL){

      // add this chain as a child of the current parent
      if(top == (const Tree *) NULL){
	top = chain;
      } else{
	parent->addChild(chain);
	chain->setParent(parent);
      }

      // locate the new parent by descending on chain
      parent = chain.operator->();
      while(! parent->getChildren().empty()){
	parent = parent->getMutableChildren().front().operator->();
      }
    }

    // add this terminal node
    parent->addChild(terminal);
    terminal->setParent(parent);

    // move up in the tree if necessary
    for(; upLevels > 0; upLevels --){
      parent = parent->_parent;
    }
  }

  // set terminal positions
  if(top != (const Tree *) NULL){
    top->setPositions();
  }

  /*
  if(top != (const Tree *) NULL){
    top->display(cout);
    cout << endl;
  }
  */

  // this sentence is empty
  if(top == (const Tree *) NULL){
    return top;
  }

  ///////////////////////////////////////////////////////////////////////
  //
  // add argument annotations
  //
  ///////////////////////////////////////////////////////////////////////

  // traverse the arguments for each verb
  for(int verb = 0; verb + 5 < (int) sentence[0].size(); verb ++){
    //
    // find the verb position in the terminal vector
    //
    int verbPosition = 0;
    int count = 0;
    for(int i = 0; i < (int) sentence.size(); i ++){
      if(sentence[i][4] != "-"){
	if(count == verb){
	  verbPosition = i;
	  break;
	}
	count ++;
      }
    }
    //cout << "Looking at verb: " << verbPosition << endl;
    
    //
    // detect all arguments for the current predicate
    //
    int start = 0;
    int end = 0;
    string name = "";
    for(int i = 0; i < (int) sentence.size(); i ++){
      string label = sentence[i][verb + 5];
      if(startsWith(label, "(")){
	start = i;
	name = extractConllLabel(label);
      }
      if(endsWith(label, ")")){
	end = i;
	RVASSERT(! name.empty(), "incorrect arg close");

	// match the argument in the syntactic tree
	// note: we don't do verbs
	if(name != "V"){
	  RVASSERT(top->matchArgument(name, start, end, verbPosition),
		   "Failed to match argument " << name << " (" << start 
		   << ", " << end << ") for verb " << verbPosition << endl);
	}

	start = 0;
	end = 0;
	name = "";
      }
    }
  }

  //top->display(cout); cout << endl;

  return top;
}

bool Tree::matchArgument(const string & name,
			 int start,
			 int end,
			 int verbPosition)
{
  if(exactMatchArgument(name, start, end, verbPosition))
    return true;

  return approxMatchArgument(name, start, end, verbPosition);
}

bool Tree::exactMatchArgument(const string & name,
			      int start,
			      int end,
			      int verbPosition)
{
  // exact match
  if(_leftPosition == start &&
     _rightPosition == end){
    addArgument(Argument("B", name, verbPosition));
    return true;
  }

  // look for exact match in the children list
  for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
    if((* it)->exactMatchArgument(name, start, end, verbPosition)) 
      return true;
  }

  return false;
}

bool Tree::approxMatchArgument(const string & name,
			       int start,
			       int end,
			       int verbPosition)
{
  if(_leftPosition == start &&
     _rightPosition <= end){
    addArgument(Argument("B", name, verbPosition));
    return false;
  }

  if(_leftPosition > start &&
     _rightPosition <= end){
    addArgument(Argument("I", name, verbPosition));
    if(_rightPosition == end) return true;
    return false;
  }

  for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
    if((* it)->approxMatchArgument(name, start, end, verbPosition))
      return true;
  }

  return false;
}
