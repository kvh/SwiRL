
#include "Tree.h"
#include "AssertLocal.h"
#include "CharUtils.h"
#include "Head.h"
#include "Lexicon.h"
#include "Logger.h"

#define ADD_ARG_FEAT(n, v) _argFeats.push_back(mergeNameValue((n), v))
#define ADD_PRED_FEAT(n, v) _predFeats.push_back(mergeNameValue((n), v))
#define ADD_PATH_FEAT(n, v) pathFeats.push_back(mergeNameValue((n), v))

using namespace std;
using namespace srl;

int Tree::INCLUDES_SIZE = 8;
// int Tree::SYN_INCLUDES_SIZE = 8;

//
// The morpher object (typically WordNet) is used to generate the lemmas
//
RCIPtr<srl::Morpher> Tree::_morpher;

ofstream Tree::POS;
ofstream Tree::NEG;

bool Tree::_reachedMaxPos = false;
bool Tree::_reachedMaxNeg = false;

std::vector<int> Tree::_pathLengths;

void Tree::initializePathLengths()
{
  _pathLengths.resize(1000, 0);
}

void Tree::reportPathLengths(std::ostream & os)
{
  for(size_t i = 0; i < _pathLengths.size(); i ++){
    os << i << " " << _pathLengths[i] << endl;
  }
}

static String normalizeCase(const String & token,
			    bool caseSensitive)
{
  if(caseSensitive) return token;
  return srl::toLower(token);
}

static String mergeNameValue(const String & name,
			     const String & value)
{
  ostringstream os;
  os << name << "." << value;
  return os.str();
}

static String mergePrefixNameValue(const String & prefix,
				   const String & name,
				   const String & value)
{
  ostringstream os;
  os << prefix << name << "." << value;
  return os.str();
}

static String mergeNameValue(const String & name,
			     int value)
{
  ostringstream os;
  os << name << "." << value;
  return os.str();
}

void Tree::displayArguments(OStream & os) const
{
  for(list<Argument>::const_iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    os << " ";
    it->display(os, "[", "]");
  }

  for(list<Argument>::const_iterator it = _predictedArguments.begin();
      it != _predictedArguments.end(); it ++){
    os << " ";
    it->display(os, "{", "}");
  }
}

void Tree::display(OStream & os,
		   bool isHead, 
		   bool isContent,
		   int offset) const
{
  for(int i = 0; i < offset; i ++) os << W(" ");
  os << getLabel();
  if(isPredicate()) os << "-P";

  if(isTerminal()){
    if(_leftPosition >= 0) os << " " << _leftPosition;
    os << W(" ") << getWord();
    // os << " " << _lemma;
    // if(Lexicon::isUnknownLemma(_lemma.c_str())) os << " U";
    // if(! _temporal.empty()) os << " T";
    if(_ne.empty() == false) os << " " << _ne;
    if(isHead) os << W(" *");
    else if(isContent) os << W(" #");
    displayArguments(os);
  } else{
    if(isHead) os << W(" *");
    else if(isContent) os << W(" #");
    displayArguments(os);
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      os << endl;

      isHead = false;
      if(_head == (* it)) isHead = true;
	  
      isContent = false;
      if(_content == (* it)) isContent = true;

      (* it)->display(os, isHead, isContent, offset + 4);
    }
  }
}

OStream & operator << (OStream & os, const srl::TreePtr & tree)
{
  tree->display(os);
  os << endl;
  return os;
}

OStream & operator << (OStream & os, const srl::Tree & tree)
{
  tree.display(os);
  os << endl;
  return os;
}

void Tree::serialize(OStream & os,
		     int offset) 
{
  for(int i = 0; i < offset; i ++) os << " ";
  os << "( " << getLabel();

  // display a terminal
  if(isTerminal()){
    os << " " << _leftPosition;
    os << " " << getWord();
    os << " " << getLemma();
    if(getNe().empty() == false) os << " " << getNe();
    else os << " O";

    // display arguments
    os << " ";
    serializeArguments(os);
  } 

  // display a non-terminal
  else {
    // display the children position of the head word
    os << " " << getHeadChildrenPosition();
    
    // display arguments
    os << " ";
    serializeArguments(os);

    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      os << endl;
      (* it)->serialize(os, offset + 4);
    }
  }

  os << " )";
}

int Tree::getHeadChildrenPosition() const
{
  int position = 0;
  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++, position ++){
    if(* it == _head) return position;
  }
  return -1;
}

void Tree::serializeArguments(OStream & os) const
{
  if(_predictedArguments.empty() == false){
    os << "{";
    for(list<Argument>::const_iterator it = _predictedArguments.begin();
	it != _predictedArguments.end(); it ++){
      os << " ";
      it->display(os, "", "");
    }
    os << " }";
  }
}

void Tree::displayParens(OStream & os,
			 bool showHead) const
{
  os << W("( ") << getLabel();

  if(isTerminal()){
    os << W(" ") << getWord();
  } else{
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      os << W(" ");
      (* it)->displayParens(os, showHead);
    }

    // display the head position if set
    if(showHead == true && getHead() != (const Tree *) NULL){
      int position = 0;
      for(list<TreePtr>::const_iterator it = _children.begin();
	  it != _children.end(); it ++, position ++){
	if((* it) == getHead()){
	  os << W(" ") << position;
	  break;
	}
      }
    }
  }

  os << W(" )");
}

void Tree::displayPrettyParens(OStream & os,
			       int offset,
			       bool showHead) const
{
  for(int i = 0; i < offset; i ++) os << W(" ");
  os << W("(") << getLabel();
  
  if(isTerminal()){
    os << W(" ") << getWord();
  } else{

    // display the head position if set
    if(showHead == true && getHead() != (const Tree *) NULL){
      int position = 0;
      for(list<TreePtr>::const_iterator it = _children.begin();
	  it != _children.end(); it ++, position ++){
	if((* it) == getHead()){
	  os << "_" << position;
	  break;
	}
      }
    }

    os << endl;
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      (* it)->displayPrettyParens(os, offset + 4, showHead);
      os << endl;
    }

    for(int i = 0; i < offset; i ++) os << W(" ");
  }
  
  os << W(")");
}

const String & Tree::getHeadWord() const
{
  if(isTerminal()){
    return getWord();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getWord();
}

const vector<String> & Tree::getHeadSuffixes() const
{
  if(isTerminal()){
    return _suffixes;
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->_suffixes;
}

const vector<String> & Tree::getContentSuffixes() const
{
  if(isTerminal()){
    return _suffixes;
  }

  TreePtr tmp;
  for(tmp = getContent(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getContent());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->_suffixes;
}

const String & Tree::getContentWord() const
{
  if(isTerminal()){
    return getWord();
  }

  TreePtr tmp;
  for(tmp = getContent(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getContent());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getWord();
}

const String & Tree::getHeadTag() const
{
  if(isTerminal()){
    return getLabel();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLabel();
}

const String & Tree::getContentTag() const
{
  if(isTerminal()){
    return getLabel();
  }

  TreePtr tmp;
  for(tmp = getContent(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getContent());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLabel();
}

short Tree::getHeadPosition() const
{
  if(isTerminal()){
    return getLeftPosition();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLeftPosition();
}

short Tree::getContentPosition() const
{
  if(isTerminal()){
    return getLeftPosition();
  }

  TreePtr tmp;
  for(tmp = getContent(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getContent());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLeftPosition();
}

const String & Tree::getHeadLemma() 
{
  if(isTerminal()){
    return getLemma();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLemma();
}

const String & Tree::getContentLemma() 
{
  if(isTerminal()){
    return getLemma();
  }

  TreePtr tmp;
  for(tmp = getContent(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getContent());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLemma();
}

const String & Tree::getHeadNe() const
{
  if(isTerminal()){
    return getNe();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getNe();
}

const String & Tree::getContentNe() const
{
  if(isTerminal()){
    return getNe();
  }

  TreePtr tmp;
  for(tmp = getContent(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getContent());
  LVASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getNe();
}

const String & Tree::getLemma()
{
  RVASSERT(_morpher != (const Morpher *) NULL, "undefined morpher");
  
  // was not computed before
  if(_lemma.empty() == true){
    if(! _isPredicate) setLemma(_morpher->morph(getWord(), getLabel()));
    else setLemma(_morpher->morph(getWord(), "VB"));
    //CERR << getWord() << "/" << getLabel() << " ==> " << _lemma << endl;
  }

  return _lemma;
}

void Tree::detectLemmas(bool gatherCounts)
{
  if(isTerminal()){
    // call WN here if necessary
    getLemma();
    // increment lemma count
    if(gatherCounts) Lexicon::addLemmaCount(_lemma);
    return;
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectLemmas(gatherCounts);
  }
}

short Tree::setPositions()
{
  short current = 0;
  setPosition(current);
  return (current - 1);
}

void Tree::setPosition(short & current)
{
  if(isTerminal()){
    _leftPosition = current;
    _rightPosition = current;
    current ++;
  } else{
    for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
      (* it)->setPosition(current);
    }

    _leftPosition = _children.front()->getLeftPosition();
    _rightPosition = _children.back()->getRightPosition();
  }
}

void Tree::setHead()
{
  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->setHead();
  }

  if(_children.empty() == false){
    setHead(findHead<Tree>(getLabel(), getChildren()));

    // find the direct iterator
    _headIterator = _children.end();
    for(_headIterator = _children.begin();
	_headIterator != _children.end();
	_headIterator ++){
      if((* _headIterator).operator->() == _head.operator->()){
	break;
      }
    }
    LVASSERT(_headIterator != _children.end(), "missing head iterator");

    // find the reverse iterator
    _headReverseIterator = _children.rend();
    for(_headReverseIterator = _children.rbegin();
	_headReverseIterator != _children.rend();
	_headReverseIterator ++){
      if((* _headReverseIterator).operator->() == _head.operator->()){
	break;
      }
    }
    LVASSERT(_headReverseIterator != _children.rend(), "missing head iterator");

  }
}

void Tree::detectContentWords()
{
  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectContentWords();
  }

  if(_children.empty() == false){
    detectContentWord();
  }
}

bool Tree::isUnnecessaryNode() const
{
  static const Char * unnecessary [] = { 
    W("."), W("''"), W("``"), NULL
  };

  for(int i = 0; unnecessary[i] != NULL; i ++){
    if(getLabel() == unnecessary[i]){
      return true;
    }
  }
  
  return false;
}

const RCIPtr<srl::Tree> & Tree::getLastChild() const
{
  for(list<TreePtr>::const_reverse_iterator it = _children.rbegin(); 
      it != _children.rend(); it ++)
    if((* it)->isUnnecessaryNode() == false) return (* it);
  return _children.back();
}

const RCIPtr<srl::Tree> & Tree::getFirstChild() const
{
  for(list<TreePtr>::const_iterator it = _children.begin(); 
      it != _children.end(); it ++)
    if((* it)->isUnnecessaryNode() == false) return (* it);
  return _children.front();
}

void Tree::detectContentWord()
{    
  // H1
  if(getLabel().substr(0, 2) == "PP"){
    _content = getLastChild();
  } 

  // H2
  else if(getLabel().substr(0, 4) == "SBAR"){
    for(list<TreePtr>::const_iterator it = _children.begin(); 
	it != _children.end(); it ++){
      if((* it)->getLabel().substr(0, 1) == "S"){
	_content = * it;
	break;
      }
    }
  }

  // H3 
  else if(getLabel().substr(0, 2) == "VP"){
    for(list<TreePtr>::const_iterator it = _children.begin(); 
	it != _children.end(); it ++){
      if((* it)->getLabel().substr(0, 2) == "VP"){
	_content = * it;
	break;
      }
    }
  }
  
  // H4
  else if(getLabel().substr(0, 4) == "ADVP"){
    for(list<TreePtr>::reverse_iterator it = _children.rbegin(); 
	it != _children.rend(); it ++){
      if((* it)->getLabel().substr(0, 2) != "TO" &&
	 (* it)->getLabel().substr(0, 2) != "IN"){
	_content = * it;
	break;
      }
    }
  }

  // H5
  else if(getLabel().substr(0, 4) == "ADJP"){
    for(list<TreePtr>::reverse_iterator it = _children.rbegin(); 
	it != _children.rend(); it ++){
      String pre2 = (* it)->getLabel().substr(0, 2);
      if(pre2 == "NN" || pre2 == "NP" || pre2 == "VB" || pre2 == "VP" ||
	 pre2 == "JJ" || (* it)->getLabel().substr(0, 4) == "ADJP"){
	_content = * it;
	break;
      }
    }
  }

  // H6 and robustness to parsing errors
  if(_content.operator->() == NULL) _content = _head;
}

static const Tree * findVerbModifier(short start,
				     short end,
				     const vector<TreePtr> & sentence)
{
  for(short i = end - 1; i >= start; i --){
    String pre = sentence[i]->getLabel().substr(0, 2);
    if(pre == "VB" || pre == "MD" || pre == "TO" || 
       sentence[i]->getLabel().substr(0, 3) == "AUX") 
      return sentence[i].operator->();
  }
  return NULL;
}

void Tree::
detectPredicateVoices(const std::vector< RCIPtr<srl::Tree> > & sentence)
{
  // only care about terminals marked as predicate
  if(isTerminal() && isPredicate()){
    // find its context span
    short left = findVerbContext(sentence);
    //cerr << "VERB " << getWord() << " CONTEXT " << left << endl;
    const Tree * modifier = 
      findVerbModifier(left, getLeftPosition(), sentence);
    //cerr << "VERB " << getWord() << " MODIF ";
    //if(modifier != NULL) cerr << modifier->getWord() << endl;
    //else cerr << "NIL" << endl;
    detectVoice(modifier);
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectPredicateVoices(sentence);
  }
}

static bool isBeVerb(const string & v)
{
  static char * forms [] = { 
    "be", "am", "is", "was", "are", "were", "been", "being", NULL 
  };
  string lower = toLower(v);
  for(int i = 0; forms[i] != NULL; i ++){
    if(lower == forms[i]){
      return true;
    }
  }

  return false;
}

static bool isGetVerb(const string & v)
{
  static char * forms [] = { 
    "get", "got", "gotten", "getting", "geting", "gets", NULL 
  };
  string lower = toLower(v);
  for(int i = 0; forms[i] != NULL; i ++){
    if(lower == forms[i]){
      return true;
    }
  }

  return false;
}

void Tree::detectVoice(const Tree * modifier)
{
  if(getLabel() == "VBG" && modifier == NULL){
    _verbType = VERB_GERUND;
    //cerr << "Verb " << getWord() << " is gerund" << endl;
  } else if(getLabel() == "VB" && 
	    modifier != NULL && modifier->getLabel() == "TO") {
    _verbType = VERB_INFINITIVE;
    //cerr << "Verb " << getWord() << " is infinitive" << endl;
  } else if(isBeVerb(getWord())){
    _verbType = VERB_COPULATIVE;
    //cerr << "Verb " << getWord() << " is copulative" << endl;
  } else if((getLabel() == "VBN" || getLabel() == "VBD") &&
	    modifier != NULL &&
	    (isBeVerb(modifier->getWord()) || isGetVerb(modifier->getWord()))){
    _verbType = VERB_PASSIVE;
    //cerr << "Verb " << getWord() << " is passive" << endl;
  } else{
    _verbType = VERB_ACTIVE;
    //cerr << "Verb " << getWord() << " is active" << endl;
  }

  //
  // we are overgenerous with active verbs
  // if a VBD|VBN active verb has a (PP by ...) attachment, change to passive
  // if (NP (NP ...) (VPB (VBN|VBD ...))), change to passive
  //
  // XXX: improve voice detection

  //
  // inside SQ we might incorrectly label passive verbs as active, e.g.
  // (SBARQ (WHADVP When) (SQ was (NP the telehraph) (VP invented)))
  // in this phrase (VP invented is initially tagged as active due to
  // the limited context available at the time
  //
  // XXX: improve voice detection
}

short Tree::findVerbContext(const std::vector< RCIPtr<srl::Tree> > & sentence) const
{
  const Tree * good = this;
  bool sawSentence = false;
  for(Tree * current = _parent; 
      current != NULL && current->validPredicateParent(good, sawSentence);
      current = current->_parent){
    good = current;
  }
  RVASSERT(good != NULL, "Found predicate without parent!");

  //
  // this context may be too large, as some verbs include CCs:
  //   "producing and promoting"
  // for such situations stop when reaching the CC
  //
  int ctx = getLeftPosition();
  for(; ctx >= good->getLeftPosition() && 
	sentence[ctx]->getLabel() != "CC"; 
      ctx --);
  ctx ++;

  // cerr << "Verb ctx " << ctx << " for verb: " << * this;

  return ctx; // good->getLeftPosition();
}

bool Tree::validPredicateParent(const Tree * child,
				bool & sawSentence) const
{
  // stop if moving out of a clause
  if(sawSentence == true) return false;

  // VP* are Ok
  if(getLabel().substr(0, 2) == "VP") return true;

  // the first S* is Ok if no left children are VP*, NP* or S*
  if(getLabel().substr(0, 1) == "S"){
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end() && (* it).operator->() != child; it ++){
      if((* it)->getLabel().substr(0, 2) == "VP" ||
	 (* it)->getLabel().substr(0, 2) == "NP" ||
	 (* it)->getLabel().substr(0, 1) == "S"){
	return false;
      }
    }
    sawSentence = true;
    return true;
  }

  return false;
}

bool Tree::includedIn(const Tree * p) const
{
  for(const Tree * crt = this; crt != NULL; crt = crt->_parent){
    if(crt == p) return true;
  }
  return false;
}

static void generalizePaths(const Tree * arg,
			    const Tree * pred,
			    const vector<const Tree *> & argParents, 
			    const vector<const Tree *> & verbParents, 
			    vector<String> & paths)
{
  // XXX: do not use for now. feels too generic
  // case 1: arg ^^ top vv pred
  /*
  if(argParents.size() > 0 && verbParents.size() > 0){
    ostringstream os1;
    os1 << arg->getLabel() << "^^"
	<< argParents.back()->getLabel() << "vv"
	<< verbParents.front()->getLabel();
    paths.push_back(os1.str());
  }
  */

  // case 2: arg ^^ top vv nodei vv pred
  if(argParents.size() == 1 && verbParents.size() > 1){
    for(size_t i = 1; i < verbParents.size(); i ++){
      ostringstream os2;
      os2 << arg->getLabel() << "^^"
	  << argParents.back()->getLabel() << "vv"
	  << verbParents[i]->getLabel() << "vv"
	  << verbParents.front()->getLabel();
      paths.push_back(os2.str());
    }
  }

  // case 3: arg ^^ nodei ^^ top vv pred
  if(argParents.size() > 1 && verbParents.size() == 1){
    for(size_t i = 0; i < argParents.size() - 1; i ++){
      ostringstream os3;
      os3 << arg->getLabel() << "^^"
	  << argParents[i]->getLabel() << "^^"
	  << argParents.back()->getLabel() << "vv"
	  << verbParents.front()->getLabel();
      paths.push_back(os3.str());
    }
  }

  // case 4: arg ^^ nodei ^^ top vv nodej vv pred
  if(argParents.size() > 1 && verbParents.size() > 1){
    for(size_t i = 0; i < argParents.size() - 1; i ++){
      for(size_t j = 1; j < verbParents.size(); j ++){
	ostringstream os4;
	os4 << arg->getLabel() << "^^"
	    << argParents[i]->getLabel() << "^^"
	    << argParents.back()->getLabel() << "vv"
	    << verbParents[j]->getLabel() << "vv"
	    << verbParents.front()->getLabel();
	paths.push_back(os4.str());
      }
    }
  }
}

void Tree::constructPath(const Tree * arg, 
			 const Tree * pred,
			 PathFeatures & pf,
			 bool positive)
{
  pf.clauseCount = 0;
  pf.upClauseCount = 0;
  pf.downClauseCount = 0;
  pf.vpCount = 0;
  pf.upVpCount = 0;
  pf.downVpCount = 0;
  pf.length = 0;

  vector<const Tree *> argParents;
  vector<const Tree *> verbParents;

  for(const Tree * crt = arg->_parent; crt != NULL; crt = crt->_parent){
    argParents.push_back(crt);
    if(crt->getLabel().substr(0, 1) == "S"){
      pf.clauseCount ++;
      pf.upClauseCount ++;
    }
    else if(crt->getLabel().substr(0, 2) == "VP"){
      pf.vpCount ++;
      pf.upVpCount ++;
    }

    if(pred->includedIn(crt)) break;
  }
  RVASSERT(! argParents.empty(), "Empty parent set for arg:" << endl << * arg);

  bool foundBigDaddy = false;
  for(const Tree * crt = pred->_parent; crt != NULL; crt = crt->_parent){
    if(crt == argParents.back()){
      foundBigDaddy = true;
      break;
    }
    verbParents.push_back(crt);
    if(crt->getLabel().substr(0, 1) == "S"){
      pf.clauseCount ++;
      pf.downClauseCount ++;
    } else if(crt->getLabel().substr(0, 2) == "VP"){
      pf.vpCount ++;
      pf.downVpCount ++;
    }
  }
  RVASSERT(foundBigDaddy, "Missing big daddy in path detection!");

  pf.length = 1 + argParents.size() + verbParents.size();
  if(pf.length > MAX_PATH_LENGTH) pf.tooLong = true;
  // if(positive) _pathLengths[pf.length] ++;

  pf.subsumptionCount = argParents.size() - verbParents.size() - 1;
  if(pf.subsumptionCount > MAX_PATH_LENGTH - 2) pf.largeSubsumption = true;

  vector<string> fullPath;
  fullPath.push_back(arg->getLabel());
  for(vector<const Tree *>::const_iterator it = argParents.begin();
      it != argParents.end(); it ++){
    fullPath.push_back(string("^") + (* it)->getLabel());
  }
  for(vector<const Tree *>::reverse_iterator it = verbParents.rbegin();
      it != verbParents.rend(); it ++){
    fullPath.push_back(string("v") + (* it)->getLabel());
  }

  ostringstream os;
  for(vector<string>::const_iterator it = fullPath.begin();
      it != fullPath.end(); it ++){
    os << * it;
  }
  if(pf.tooLong == false){
    pf.paths.push_back(os.str());
    // pf.path = os.str();
  }

  generalizePaths(arg, pred, argParents, verbParents, pf.paths);

  /*
  ostringstream ls;
  int leftCount = 0;
  for(vector<string>::const_iterator it = fullPath.begin();
      it != fullPath.end() && leftCount < MAX_PATH_LENGTH; 
      it ++, leftCount ++){
    ls << * it;
  }
  pf.leftPath = ls.str();
  
  ostringstream rs;
  int start = fullPath.size() - MAX_PATH_LENGTH;
  if(start < 0) start = 0;
  for(size_t i = start; i < fullPath.size(); i ++){
    rs << fullPath[i];
  }
  */

  /*
  for(vector<string>::reverse_iterator it = fullPath.rbegin();
      it != fullPath.rend() && rightCount < MAX_PATH_LENGTH; 
      it ++, rightCount ++){
    rs << * it;
  }
  */
  // pf.rightPath = rs.str();
}

void Tree::detectAuxVerbs()
{
  // Auxiliary verbs are VB*|AUX phrases inside VP* that contain other VP*
  if(getLabel().substr(0, 2) == "VP"){
    bool containsVP = false;
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      if((* it)->getLabel().substr(0, 2) == "VP"){
	containsVP = true;
	break;
      }
    }
    if(containsVP == true){
      for(list<TreePtr>::iterator it = _children.begin();
	  it != _children.end(); it ++){
	if((* it)->isTerminal() && 
	   ((* it)->getLabel().substr(0, 2) == "VB" ||
	    (* it)->getLabel().substr(0, 3) == "AUX")){
	  //cerr << "Found aux verb at position: " << (* it)->getLeftPosition() << endl;
	  (* it)->setIsAuxVerb(true);
	}
      }
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectAuxVerbs();
  }
}

static String 
detectContext(const std::vector< RCIPtr<srl::Tree> > & sentence,
	      int center,
	      int leftSpan,
	      int rightSpan)
{
  int start = (center - leftSpan >= 0 ? center - leftSpan : 0);
  int end = (center + rightSpan + 1 < (int) sentence.size() ? 
	     center + rightSpan + 1 : sentence.size());

  ostringstream os;
  for(int i = start; i < end; i ++){
    if(i > start) os << "+";
    os << sentence[i]->getLabel();
  }

  return os.str();
}

void Tree::
detectPOSContexts(const std::vector< RCIPtr<srl::Tree> > & sentence)
{
  // only care about terminals marked as predicate
  if(isTerminal() && isPredicate()){
    _posContexts.push_back(detectContext(sentence, getLeftPosition(), 1, 1));
    _posContexts.push_back(detectContext(sentence, getLeftPosition(), 0, 1));
    _posContexts.push_back(detectContext(sentence, getLeftPosition(), 1, 0));
    _posContexts.push_back(detectContext(sentence, getLeftPosition(), 2, 2));
    _posContexts.push_back(detectContext(sentence, getLeftPosition(), 0, 2));
    _posContexts.push_back(detectContext(sentence, getLeftPosition(), 2, 0));
    //cerr << "Ctx for " << getLeftPosition() << " " << _posContexts.back() << endl;
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectPOSContexts(sentence);
  }
}

void Tree::
detectTemporalTerminals()
{
  // only care about terminals marked as predicate
  if(isTerminal()){
    if(Lexicon::isTmpNN(getWord().c_str())) _temporal = "NN";
    else if(Lexicon::isTmpNNP(getWord().c_str())) _temporal = "NNP";
    else if(Lexicon::isTmpIN(getWord().c_str())) _temporal = "IN";
    else if(Lexicon::isTmpRB(getWord().c_str())) _temporal = "RB";
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectTemporalTerminals();
  }
}

void Tree::detectGoverningCategory()
{
  if(getLabel().substr(0, 2) == "NP"){
    for(const Tree * crt = _parent; crt != NULL; crt = crt->_parent){
      if(crt->getLabel().substr(0, 1) == "S"){
	_gov = "S";
	break;
      } else if(crt->getLabel().substr(0, 2) == "VP"){
	_gov = "VP";
	break;
      }
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectGoverningCategory();
  }
}

void Tree::countCommas()
{
  /*
  if(isTerminal()){
    if(getLabel() == ",") _commaCount = 1;
    else if(getLabel() == "``") _leftQuoteCount = 1;
    else if(getLabel() == "''") _rightQuoteCount = 1;
    else if(getLabel() == "LRB") _leftParenCount = 1;
    else if(getLabel() == "RRB") _rightParenCount = 1;
    else if(getLabel() == "POS") _possesiveCount = 1;
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->countCommas();
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    _commaCount += (* it)->_commaCount;
    _leftQuoteCount += (* it)->_leftQuoteCount;
    _rightQuoteCount += (* it)->_rightQuoteCount;
    _leftParenCount += (* it)->_leftParenCount;
    _rightParenCount += (* it)->_rightParenCount;
    _possesiveCount += (* it)->_possesiveCount;
  }
  */
}

void Tree::detectChildrenPath()
{
  if(! isTerminal()){
    ostringstream os;
    for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
      if(it != _children.begin()) os << "+";
      os << (* it)->getLabel();
    }
    _childrenPath = os.str();
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectChildrenPath();
  }
}

void Tree::detectSiblingPaths()
{
  list<String> leftLabels;
  list<String> rightLabels;
  list<const Tree *> leftNodes;
  list<const Tree *> rightNodes;

  // initialize with 3 singlings NIL
  for(int i = 0; i < SIBLING_CONTEXT_LENGTH; i ++){
    leftLabels.push_back("nil");
    rightLabels.push_back("nil");
    leftNodes.push_back(NULL);
    rightNodes.push_back(NULL);
  }

  // left siblings
  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->_leftSiblingLabels = leftLabels;
    (* it)->_leftSiblingNodes = leftNodes;

    leftLabels.push_front((* it)->getLabel());
    leftLabels.pop_back();

    leftNodes.push_front((* it).operator->());
    leftNodes.pop_back();
  }

  // right siblings
  for(list<TreePtr>::reverse_iterator it = _children.rbegin();
      it != _children.rend(); it ++){
    (* it)->_rightSiblingLabels = rightLabels;
    (* it)->_rightSiblingNodes = rightNodes;

    rightLabels.push_front((* it)->getLabel());
    rightLabels.pop_back();

    rightNodes.push_front((* it).operator->());
    rightNodes.pop_back();
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectSiblingPaths();
  }
}

void Tree::detectSuffixes(bool caseSensitive)
{
  if(isTerminal()){
    for(int i = 2; i <= 4; i ++){
      if((int) _word.size() >= i){
	String suf = normalizeCase(_word.substr(_word.size() - i, i),
				   caseSensitive);
	_suffixes.push_back(suf);
      } else {
	_suffixes.push_back("nil");
      }
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectSuffixes(caseSensitive);
  }
}

const Tree * Tree::findBegin(const String & argName,
			     int verbPosition,
			     int argLeftPosition,
			     int argRightPosition) const
{
  for(list<Argument>::const_iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if((* it).getVerbPosition() == verbPosition &&
       (* it).getName() == argName &&
       (* it).getType() == "B" &&
       getRightPosition() < argLeftPosition){
      return this;
    }
  }

  const Tree * begin = NULL;
  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    // we passed the I-A
    if((* it)->getLeftPosition() > argRightPosition) break;

    const Tree * crt = (* it)->findBegin(argName, verbPosition, 
					 argLeftPosition, argRightPosition);
    if(crt != NULL) begin = crt;
  }
  return begin;
}

void Tree::detectArgumentBegins(const Tree * top)
{
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if((* it).getType() == "I"){
      int position = (* it).getVerbPosition();
      const Tree * begin = 
	top->findBegin((* it).getName(), position, 
		       getLeftPosition(), getRightPosition());
      RVASSERT(begin != NULL, 
	       "Found I-A without begin:" << endl << * this
	       << "In TOP:" << * top << endl);
      (* it).setBegin(begin);
      /*
      cout << "Phrase:" << endl << * this 
	   << "Has BEGIN:" << endl << * begin 
	   << "In TOP:" << endl << * top << endl;
      */
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectArgumentBegins(top);
  }
}

void Tree::incLabelCount(const Char * lab,
			 int increment)
{
  RCIPtr< StringMapEntry<int> > value = _labelCounts.get(lab);
  if(value.operator->() == NULL){
    _labelCounts.set(lab, increment);
  } else{
    value->setValue(value->getValue() + increment);
  }
}

void Tree::detectIncludes()
{
  // PER, ORG, LOC, MISC, TMP_NN, TMP_NNP, TMP_IN, TMP_MISC
  _includes.resize(INCLUDES_SIZE, false);

  if(isTerminal()){
    if(! _ne.empty()){
      if(((int) _ne.find("PER")) >= 0) _includes[INCLUDES_PER] = true;
      else if(((int) _ne.find("ORG")) >= 0) _includes[INCLUDES_ORG] = true;
      else if(((int) _ne.find("LOC")) >= 0) _includes[INCLUDES_LOC] = true;
      else if(((int) _ne.find("MISC")) >= 0) _includes[INCLUDES_MISC] = true;
    }

    if(! _temporal.empty()){
      if(_temporal == "NN") _includes[INCLUDES_NN] = true;
      else if(_temporal == "NNP") _includes[INCLUDES_NNP] = true;
      else if(_temporal == "IN") _includes[INCLUDES_IN] = true;
      else if(_temporal == "RB") _includes[INCLUDES_RB] = true;
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectIncludes();
  }

  for(list<TreePtr>::const_iterator cit = _children.begin();
      cit != _children.end(); cit ++){

    // semantic includes
    for(int i = 0; i < INCLUDES_SIZE; i ++){
      if((* cit)->_includes[i]) _includes[i] = true;
    }

    // syntactic includes: copy from all children
    for(StringMap<int>::const_iterator lit = (* cit)->_labelCounts.begin();
	lit != (* cit)->_labelCounts.end(); lit ++){
      incLabelCount((* lit).second->getKey(),
		    (* lit).second->getValue());
    }
  }

  incLabelCount(getLabel().c_str(), 1);
}

bool Tree::isReferent() const
{
  // (WHNP (WP who))
  if(getLabel() == "WP" &&
     _parent != NULL &&
     _parent->getLabel() == "WHNP")
    return true;

  // (WHNP (WDT which))
  if(getLabel() == "WDT" &&
     _parent != NULL &&
     _parent->getLabel() == "WHNP" &&
     _parent->_children.size() == 1)
    return true;

  // (SBAR (IN that) ...)
  if(getLabel() == "IN" &&
     toLower(getHeadWord()) == "that" &&
     _parent != NULL &&
     _parent->getLabel() == "SBAR" &&
     _parent->_children.front().operator->() == this)
    return true;

  return false;
}

void Tree::
detectSurfaceDistance(const std::vector< RCIPtr<srl::Tree> > & sentence,
		      int left, 
		      int right, 
		      int & vbDist, 
		      int & commaDist, 
		      int & ccDist,
		      int & tokenDist, 
		      bool & isAdjacent,
		      bool & includesReferent)
{
  vbDist = 0;
  commaDist = 0;
  tokenDist = 0;
  ccDist = 0;
  includesReferent = false;

  isAdjacent = false;
  if(left >= right) isAdjacent = true;

  for(int i = left; i < right; i ++){
    tokenDist ++;

    if(sentence[i]->getLabel() == ","){
      commaDist ++;
    } else if(sentence[i]->getLabel() == "CC"){
      ccDist ++;
    } else if(sentence[i]->getLabel().substr(0, 2) == "VB"){
       if(! sentence[i]->isAuxVerb()){
	 vbDist ++;
       }
    } else if(sentence[i]->isReferent()){
      includesReferent = true;
    }
    
  }
}

void Tree::createSampleStreams(const String & posName,
			       const String & negName)
{
  POS.open(posName.c_str());
  RVASSERT(POS, "Failed to create positive example stream!");

  NEG.open(negName.c_str());
  RVASSERT(NEG, "Failed to create negative example stream!");
}
void Tree::closeSampleStreams() {
  POS.close();
  NEG.close();
}

void Tree::
generateExamples(const std::vector< RCIPtr<srl::Tree> > & sentence,
		 bool caseSensitive)
{
  // detect all predicates in this phrase
  vector<int> predPositions;
  detectPredicates(predPositions);

  // generate positive/negative samples for each predicate
  for(int i = 0; i < (int) predPositions.size(); i ++){
    int pos = predPositions[i];
    RVASSERT(pos >= 0 && pos < (int) sentence.size(),
	    "Invalid predicate position " << pos);
    const Tree * pred = sentence[pos].operator->();
    RVASSERT(pred->isPredicate(),
	    "Chosen phrase is not a predicate!");

    const Tree * s = pred->findParentForLluisHeuristic();

    // cout << "TREE:\n" << * this;
    generateExamplesForPredicate(sentence, pred, pos, s, caseSensitive);
  }
}

const Tree * Tree::
findParentForLluisHeuristic() const
{
  const Tree * p = _parent;
  while(p != NULL && p->getLabel().substr(0, 1) != "S")
    p = p->_parent;

  /*
  // found the (S (PP ... (S))) case
  // (S (PP Instead of (S being ...)) (NP 
  if(p->_parent != NULL &&
     p->_parent->getLabel() == "PP" &&
     p->_parent->_children.back().operator->() == p &&
     p->_parent->_parent != NULL &&
     p->_parent->_parent->getLabel().substr(0, 1) == "S")
    p = p->_parent->_parent;
  */

  return p;
}

void Tree::
generateFeatureSets(const std::vector< RCIPtr<srl::Tree> > & sentence,
		    bool caseSensitive)
{
  // a bottom-up implementation to avoid counting 
  // things (e.g. commas) many times
  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->generateFeatureSets(sentence, caseSensitive);
  }

  generateArgumentFeatures(sentence, caseSensitive);
  if(isTerminal() && isPredicate()) 
    generatePredicateFeatures(sentence, caseSensitive);
}

static bool usefulSyntacticLabel(const string & label)
{
  static char * skip [] = {
    "S1", "TOP", "LST", "SYM", "LS", "SBARQ", "RRC", "SQ", "X", "SINV",
    "UH", "INTJ", "WHADJP", "FRAG", "WP$", "FW", "EX", "NX", "UCP", "NAC",
    "AUXG", "WHPP", "PDT", ".", "WHADVP", "WRB", NULL
  };

  for(int i = 0; skip[i] != NULL; i ++){
    if(label == skip[i]) return false;
  }

  return true;
}

void Tree::
generateArgumentFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence,
			 bool caseSensitive)
{
  ADD_ARG_FEAT("label", getLabel());
  
  if(! isTerminal()) ADD_ARG_FEAT("cpath", _childrenPath);
  else ADD_ARG_FEAT("cpath", "nil");

  String hw = normalizeCase(getHeadWord(), caseSensitive);
  if(Lexicon::isUnknownWord(hw)) ADD_ARG_FEAT("hw", "unk");
  else ADD_ARG_FEAT("hw", hw);

  const vector<String> & sufs = getHeadSuffixes();
  RVASSERT(sufs.size() == 3, "Invalid suffix count" << sufs.size());
  for(size_t i = 0; i < sufs.size(); i ++){
    ostringstream os;
    os << "hs" << i + 2;
    ADD_ARG_FEAT(os.str(), sufs[i]);
  }

  String hl = normalizeCase(getHeadLemma(), caseSensitive);
  if(Lexicon::isUnknownLemma(hl)) ADD_ARG_FEAT("hl", "unk");
  else ADD_ARG_FEAT("hl", hl);

  ADD_ARG_FEAT("hpos", getHeadTag());

  String cw = normalizeCase(getContentWord(), caseSensitive);
  if(Lexicon::isUnknownWord(cw)) ADD_ARG_FEAT("cw", "unk");
  else ADD_ARG_FEAT("cw", cw);

  const vector<String> & cwsufs = getContentSuffixes();
  RVASSERT(cwsufs.size() == 3, "Invalid suffix count" << cwsufs.size());
  for(size_t i = 0; i < cwsufs.size(); i ++){
    ostringstream os;
    os << "cws" << i + 2;
    ADD_ARG_FEAT(os.str(), cwsufs[i]);
  }

  String cl = normalizeCase(getContentLemma(), caseSensitive);
  if(Lexicon::isUnknownLemma(cl)) ADD_ARG_FEAT("cl", "unk");
  else ADD_ARG_FEAT("cl", cl);

  ADD_ARG_FEAT("cpos", getContentTag());

  String cne = getContentNe();
  if(cne.empty()) ADD_ARG_FEAT("cne", "nil");
  else ADD_ARG_FEAT("cne", cne);

  /*
  ADD_ARG_FEAT("iper", _includes[INCLUDES_PER]);
  //Lexicon::addGeneralizedValues(_argFeats, "iper", _includes[INCLUDES_PER], 0, 2);
  ADD_ARG_FEAT("iorg", _includes[INCLUDES_ORG]);
  //Lexicon::addGeneralizedValues(_argFeats, "iorg", _includes[INCLUDES_ORG], 0, 2);
  ADD_ARG_FEAT("iloc", _includes[INCLUDES_LOC]);
  //Lexicon::addGeneralizedValues(_argFeats, "iloc", _includes[INCLUDES_LOC], 0, 2);
  ADD_ARG_FEAT("imis", _includes[INCLUDES_MISC]);
  //Lexicon::addGeneralizedValues(_argFeats, "imis", _includes[INCLUDES_MISC], 0, 2);
  ADD_ARG_FEAT("inn", _includes[INCLUDES_NN]);
  //Lexicon::addGeneralizedValues(_argFeats, "inn", _includes[INCLUDES_NN], 0, 2);
  ADD_ARG_FEAT("innp", _includes[INCLUDES_NNP]);
  //Lexicon::addGeneralizedValues(_argFeats, "innp", _includes[INCLUDES_NNP], 0, 2);
  ADD_ARG_FEAT("iin", _includes[INCLUDES_IN]);
  //Lexicon::addGeneralizedValues(_argFeats, "iin", _includes[INCLUDES_IN], 0, 2);
  ADD_ARG_FEAT("irb", _includes[INCLUDES_RB]);
  //Lexicon::addGeneralizedValues(_argFeats, "irb", _includes[INCLUDES_RB], 0, 2);
  */
  Lexicon::addThresholdValue(_argFeats, "iper", _includes[INCLUDES_PER], 3);
  Lexicon::addThresholdValue(_argFeats, "iorg", _includes[INCLUDES_ORG], 3);
  Lexicon::addThresholdValue(_argFeats, "iloc", _includes[INCLUDES_LOC], 3);
  Lexicon::addThresholdValue(_argFeats, "imis", _includes[INCLUDES_MISC], 3);
  Lexicon::addThresholdValue(_argFeats, "inn", _includes[INCLUDES_NN], 3);
  Lexicon::addThresholdValue(_argFeats, "innp", _includes[INCLUDES_NNP], 3);
  Lexicon::addThresholdValue(_argFeats, "iin", _includes[INCLUDES_IN], 3);
  Lexicon::addThresholdValue(_argFeats, "irb", _includes[INCLUDES_RB], 3);

  // XXX: do not use the temporal modifier feature for now
  /*
  if(_includes[INCLUDES_NN] == true){
    bool tm = hasTemporalModifier();
    ADD_ARG_FEAT("itmpmod", tm);
  } else {
    ADD_ARG_FEAT("itmpmod", "nil");
  }
  */

  /*
  ADD_ARG_FEAT("sinpb", _synIncludes[0]);
  ADD_ARG_FEAT("sinp", _synIncludes[1]);
  ADD_ARG_FEAT("sivp", _synIncludes[2]);
  ADD_ARG_FEAT("sipp", _synIncludes[3]);
  ADD_ARG_FEAT("sisb", _synIncludes[4]);
  ADD_ARG_FEAT("sis", _synIncludes[5]);
  ADD_ARG_FEAT("sij", _synIncludes[6]);
  ADD_ARG_FEAT("sir", _synIncludes[7]);

  ADD_ARG_FEAT("tc", getRightPosition() + 1 - getLeftPosition());
  ADD_ARG_FEAT("cc", _commaCount);
  ADD_ARG_FEAT("lqc", _leftQuoteCount);
  ADD_ARG_FEAT("rqc", _rightQuoteCount);
  ADD_ARG_FEAT("lpc", _leftParenCount);
  ADD_ARG_FEAT("rpc", _rightParenCount);
  ADD_ARG_FEAT("posc", _possesiveCount);
  */
  const StringMap<int> & labels = Lexicon::getSyntacticLabelCounts();
  // cerr << "Overall label count: " << labels.size() << endl;
  for(StringMap<int>::const_iterator it = labels.begin();
      it != labels.end(); it ++){
    const Char * label = (* it).second->getKey();

    if(usefulSyntacticLabel(label)){
      // cerr << "Looking at counts for label: " << label << endl;
      int count = 0;
      String lcn = mergeStrings("count", label);
      _labelCounts.get(label, count);

      if(count > 0){
	//ADD_ARG_FEAT(lcn, count);
	//Lexicon::addGeneralizedValues(_argFeats, lcn, count, 0, 2);
	Lexicon::addThresholdValue(_argFeats, lcn, count, 3);
      }
    }
  }

  // XXX: disable lsib and rsib. These are used in the system with F 74.62!!!
  /*
  RVASSERT(_leftSiblingLabels.size() == SIBLING_CONTEXT_LENGTH ||
	   _leftSiblingLabels.size() == 0,
	   "Invalid left sibling count: " << _leftSiblingLabels.size());
  for(int size = 1; size <= SIBLING_CONTEXT_LENGTH; size ++){
    ostringstream os;
    int index = 0;
    for(list<String>::const_iterator it = _leftSiblingLabels.begin();
	it != _leftSiblingLabels.end() && index < size; it ++, index ++){
      if(index > 0) os << "+";
      os << * it;
    }
    ADD_ARG_FEAT("lsib", os.str());
  }

  RVASSERT(_rightSiblingLabels.size() == SIBLING_CONTEXT_LENGTH ||
	   _rightSiblingLabels.size() == 0,
	   "Invalid right sibling count: " << _rightSiblingLabels.size());
  for(int size = 1; size <= SIBLING_CONTEXT_LENGTH; size ++){
    ostringstream os;
    int index = 0;
    for(list<String>::const_iterator it = _rightSiblingLabels.begin();
	it != _rightSiblingLabels.end() && index < size; it ++, index ++){
      if(index > 0) os << "+";
      os << * it;
    }
    ADD_ARG_FEAT("rsib", os.str());
  }
  */

  // new addition: word/pos for first/last terminal in this arg
  TreePtr firstTerm = sentence[getLeftPosition()];
  TreePtr lastTerm = sentence[getRightPosition()];
  ADD_ARG_FEAT("fww", normalizeCase(firstTerm->getWord(), caseSensitive));
  ADD_ARG_FEAT("fwt", firstTerm->getLabel());
  ADD_ARG_FEAT("lww", normalizeCase(lastTerm->getWord(), caseSensitive));
  ADD_ARG_FEAT("lwt", lastTerm->getLabel());
  /*
  cerr << "First word: " << firstTerm
       << "Last word: " << lastTerm
       << "In phrase:\n" << * this;
  */

  // new addition: minimal info from parent/left/right
  if(_parent != NULL)
    _parent->addMinimalFeatures(_argFeats, "prt", true, caseSensitive);
  if(_leftSibling != NULL)
    _leftSibling->addMinimalFeatures(_argFeats, "lft", false, caseSensitive);
  if(_rightSibling != NULL)
    _rightSibling->addMinimalFeatures(_argFeats, "rht", true, caseSensitive);

}

void Tree::addMinimalFeatures(std::vector<String> & features,
			      const String & prefix,
			      bool lexicalized,
			      bool caseSensitive)
{
  features.push_back(mergePrefixNameValue(prefix, "label", getLabel()));
  
  if(lexicalized){
    String hw = normalizeCase(getHeadWord(), caseSensitive);
    if(Lexicon::isUnknownWord(hw)) 
      features.push_back(mergePrefixNameValue(prefix, "hw", "unk"));
    else 
      features.push_back(mergePrefixNameValue(prefix, "hw", hw));
  }

  features.push_back(mergePrefixNameValue(prefix, "hpos", getHeadTag()));
}

int Tree::countDTIgnore() const
{
  String lower = toLower(getHeadWord());
  if(isTerminal() &&
     getLabel() == "DT" &&
     (lower == "the" ||
      lower == "a")){
    return 1;
  }

  int count = 0;
  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    count += (* it)->countDTIgnore();
  }

  return count;
}

bool Tree::hasTemporalModifier() const
{
  static char * labels [] = {
    "CD", "DT", "IN", "JJ", "JJR", "JJS", "NN", "NNS", 
    "NNP", "NNPS", "RB", "RBR", "RBS", NULL
  };

  int dtIgnore = countDTIgnore();

  for(int i = 0; labels[i] != NULL; i ++){
    int count = 0;
    if(_labelCounts.get(labels[i], count) == true){
      if(labels[i] == "DT"){
	if(count > dtIgnore) return true;
      } else if(labels[i] == "NN"){
	if(count > 1) return true;
      } else {
	if(count > 0) return true;
      } 
    }
  }

  return false;
}

void Tree::
generatePredicateFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence,
			  bool caseSensitive)
{
  ADD_PRED_FEAT("voice", getVerbType());

  String pw = normalizeCase(getHeadWord(), caseSensitive);
  if(Lexicon::isUnknownWord(pw)) ADD_PRED_FEAT("pw", "unk");
  else ADD_PRED_FEAT("pw", pw);

  String pl = normalizeCase(getHeadLemma(), caseSensitive);
  if(Lexicon::isUnknownPred(pl)){
    ADD_PRED_FEAT("pl", "unk");
    ADD_PRED_FEAT("pf", 0);
  }
  else{
    ADD_PRED_FEAT("pl", pl);
    ADD_PRED_FEAT("pf", 1);
  }

  // XXX: disable pctx. These were used in the system with F 74.62!!!
  /*
  for(size_t i = 0; i < _posContexts.size(); i ++){
    ADD_PRED_FEAT("pctx", _posContexts[i]);
  }
  */

  // new addition: the subcategorization rule
  String subcat = _parent->detectSubcatRule();
  ADD_PRED_FEAT("psc", subcat);
  //cerr << "Subcat: " << subcat << " for tree:\n" << endl << * _parent;
}

String Tree::detectSubcatRule() const
{
  ostringstream os;

  os << getLabel() << ">";
  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    if(it != _children.begin()) os << "+";
    os << (* it)->getLabel();
  }

  return os.str();
}

void Tree::
generatePathFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence, 
		     const Tree * other, 
		     int position,
		     vector<String> & pathFeats,
		     bool positive) const
{
  bool isPredicate = true;
  if(position < 0) isPredicate = false;

  //
  // Syntactic path-based attributes
  //

  PathFeatures pf;
  constructPath(this, other, pf, positive);

  for(size_t i = 0; i < pf.paths.size(); i ++){
    if(! pf.paths[i].empty()){
      if(i == 0) ADD_PATH_FEAT("path", pf.paths[i]);
      else ADD_PATH_FEAT("pathgen", pf.paths[i]);
    }
  }

  //ADD_PATH_FEAT("clc", pf.clauseCount);
  //Lexicon::addGeneralizedValues(pathFeats, "clc", pf.clauseCount, 0, 8);
  Lexicon::addThresholdValue(pathFeats, "clc", pf.clauseCount, 6);
  //ADD_PATH_FEAT("uclc", pf.upClauseCount);
  //Lexicon::addGeneralizedValues(pathFeats, "uclc", pf.upClauseCount, 0, 4);
  Lexicon::addThresholdValue(pathFeats, "uclc", pf.upClauseCount, 3);
  //ADD_PATH_FEAT("dclc", pf.downClauseCount);
  //Lexicon::addGeneralizedValues(pathFeats, "dclc", pf.downClauseCount, 0, 4);
  Lexicon::addThresholdValue(pathFeats, "dclc", pf.downClauseCount, 3);

  //ADD_PATH_FEAT("plen", pf.length);
  //Lexicon::addGeneralizedValues(pathFeats, "plen", pf.length, 5, 15);
  Lexicon::addThresholdValue(pathFeats, "plen", pf.length, 21);
  Lexicon::addIntervalChecks(pathFeats, "plen", pf.length, 10, 20, 5);
  ADD_PATH_FEAT("ptool", pf.tooLong);

  ADD_PATH_FEAT("subc", pf.subsumptionCount);  
  ADD_PATH_FEAT("plsub", pf.largeSubsumption);

  //ADD_PATH_FEAT("vpc", pf.vpCount);
  //Lexicon::addGeneralizedValues(pathFeats, "vpc", pf.vpCount, 0, 8);
  Lexicon::addThresholdValue(pathFeats, "vpc", pf.vpCount, 8);
  //ADD_PATH_FEAT("uvpc", pf.upVpCount);
  //Lexicon::addGeneralizedValues(pathFeats, "uvpc", pf.upVpCount, 0, 4);
  Lexicon::addThresholdValue(pathFeats, "uvpc", pf.upVpCount, 4);
  //ADD_PATH_FEAT("dvpc", pf.downVpCount);
  //Lexicon::addGeneralizedValues(pathFeats, "dvpc", pf.downVpCount, 0, 4);
  Lexicon::addThresholdValue(pathFeats, "dvpc", pf.downVpCount, 4);

  //
  // Surface-distance attributes
  // 

  int otherRight = other->getRightPosition();
  int otherLeft = other->getLeftPosition();
  if(isPredicate){
    otherRight = position;
    otherLeft = sentence[otherRight]->findVerbContext(sentence);
  }

  int argRight = getRightPosition();
  int argLeft = getLeftPosition();
  /*
    cerr << (* it).getFullName() 
    << "[" << argLeft << ", " << argRight << "] "
    << "[" << verbLeft << ", " << verbRight << "]"
    << endl;
  */
  
  if(isPredicate){
    if(argRight < otherRight) ADD_PATH_FEAT("pos", "b");
    else if(argLeft > otherRight) ADD_PATH_FEAT("pos", "a");
    else ADD_PATH_FEAT("pos", "e");
  }

  int vbDist = -1, commaDist = -1, ccDist = -1, tokenDist = -1; 
  bool isAdjacent = false;
  bool includesReferent = false;

  if(argRight < otherRight) 
    detectSurfaceDistance(sentence, argRight + 1, otherLeft, 
			  vbDist, commaDist, ccDist, tokenDist, 
			  isAdjacent, includesReferent);
  else 
    detectSurfaceDistance(sentence, otherRight + 1, argLeft, 
			  vbDist, commaDist, ccDist, tokenDist, 
			  isAdjacent, includesReferent);

  //ADD_PATH_FEAT("vd", vbDist);
  //Lexicon::addGeneralizedValues(pathFeats, "vd", vbDist, 0, 4);
  Lexicon::addThresholdValue(pathFeats, "vd", vbDist, 4);
  //ADD_PATH_FEAT("cd", commaDist);
  //Lexicon::addGeneralizedValues(pathFeats, "cd", commaDist, 0, 4);
  Lexicon::addThresholdValue(pathFeats, "cd", commaDist, 4);
  //ADD_PATH_FEAT("ccd", ccDist);
  //Lexicon::addGeneralizedValues(pathFeats, "ccd", ccDist, 0, 2);
  Lexicon::addThresholdValue(pathFeats, "ccd", ccDist, 4);
  //ADD_PATH_FEAT("td", tokenDist);
  //Lexicon::addGeneralizedValues(pathFeats, "td", tokenDist, 5, 20);
  Lexicon::addThresholdValue(pathFeats, "td", tokenDist, 41);
  Lexicon::addIntervalChecks(pathFeats, "td", tokenDist, 10, 40, 10);
  ADD_PATH_FEAT("adj", isAdjacent);

  // XXX: do not use the referent feature for now
  // ADD_PATH_FEAT("incref", includesReferent);

  if(isPredicate){
    if(_gov.empty()) ADD_PATH_FEAT("gov", "nil");
    else ADD_PATH_FEAT("gov", _gov);
  }

  //
  // arg starts with predicate particle
  //

  bool startsWithParticle = 
    (argLeft > otherRight &&
     Lexicon::isVerbParticle(((Tree *) other)->getHeadLemma(),
			     sentence[getLeftPosition()]->getHeadLemma()));
  if(startsWithParticle == true)
    ADD_PATH_FEAT("partstart", startsWithParticle);

  //
  // Xue and Palmer's syntactic frame
  //
  list<string> frames;
  if(detectSyntacticFrame(this, other, frames)){
    for(list<string>::const_iterator it = frames.begin();
	it != frames.end(); it ++){
      ADD_PATH_FEAT("synframe", * it);
    }
  }

  //
  // sibling level path if have the same children
  // 
  /*
  if(other->_parent == _parent){
    String sibPath;
    if(argRight < otherLeft) 
      sibPath = _parent->detectSiblingPath(this, other);
    else if(otherRight < argLeft)
      sibPath = _parent->detectSiblingPath(other, this);
    if(sibPath.empty() == false){
      ADD_PATH_FEAT("sibp", sibPath);
    }
  }
  */
}

bool Tree::detectSyntacticFrame(const Tree * arg,
				const Tree * pred,
				std::list<std::string> & frames)
{
  // sequence of phrases that constitute the syntactic frame
  std::list<const Tree *> elements;

  //
  // the syntactic frame makes sense only if the predicate parent is VP
  // e.g. for constructs such as NP(the quitting president) it is useless
  //
  if(pred->getParent() == NULL ||
     pred->getParent()->getLabel() != "VP"){
    return false;
  }

  // the syntactic frame is centered around the predicate
  elements.push_back(pred);
  
  //
  // traverse all the predicate parents and add all NPs or PP-NPs:
  //  to the front if they appear to the left of the parent, or
  //  to the back if they appear to the right of the parent
  //
  const Tree * current = pred;
  for(const Tree * currentParent = pred->getParent(); currentParent != NULL;
      current = currentParent, currentParent = currentParent->getParent()){

    //
    // inspect immediate children of currentParent
    // valid args might be immediate NP childrens of PP children
    //
    bool sawPredicate = false;
    for(list<TreePtr>::const_iterator it = currentParent->_children.begin();
	it != currentParent->_children.end(); it ++){

      // we found the predicate node
      if((* it).operator->() == current){
	sawPredicate = true;
	continue;
      }

      // only consider elements to the right of the predicate here
      if(sawPredicate == true){
	// found the argument
	if((* it)->includes(arg)){
	  elements.push_back((* it).operator->());
	}

	// found an NP
	else if((* it)->getLabel() == "NP"){
	  elements.push_back((* it).operator->());
	}

	// found a PP with NP
	else if((* it)->getLabel() == "PP" &&
		(* it)->_children.empty() == false &&
		(* it)->_children.back()->getLabel() == "NP"){
	  elements.push_back((* it).operator->());
	}
      }
    } // end for to the right

    sawPredicate = false;
    for(list<TreePtr>::const_reverse_iterator it = 
	  currentParent->_children.rbegin();
	it != currentParent->_children.rend(); it ++){

      // we found the predicate node
      if((* it).operator->() == current){
	sawPredicate = true;
	continue;
      }

      // only consider elements to the left of the predicate here
      if(sawPredicate == true){
	// found the argument
	if((* it)->includes(arg)){
	  elements.push_front((* it).operator->());
	}

	// found an NP
	else if((* it)->getLabel() == "NP"){
	  elements.push_front((* it).operator->());
	}

	// found a PP with NP
	else if((* it)->getLabel() == "PP" &&
		(* it)->_children.empty() == false &&
		(* it)->_children.back()->getLabel() == "NP"){
	  elements.push_front((* it).operator->());
	}
      }
    } // end for to the left

  } // end traversal of predicate parents

  //
  // At this point we have the syntactic frame stored in elements
  // We not generate the string representations for the frame
  // 
  ostringstream fullSynFrame; // all the syntactic categories included
  ostringstream anySynFrame; // the syn. cat. of the arg is replaced with ANY
  ostringstream lexSynFrame; // the syn. cat. of the pred replaced with lemma

  for(list<const Tree *>::const_iterator it = elements.begin();
      it != elements.end(); it ++){
    const Tree * crt = * it;

    if(it != elements.begin()){
      fullSynFrame << "_";
      anySynFrame << "_";
      lexSynFrame << "_";
    }

    // found the arg
    if(crt->includes(arg)){
      fullSynFrame << crt->getLabel();
      anySynFrame << "ANY";
      lexSynFrame << crt->getLabel();

      if(crt != arg){
	fullSynFrame << "i";
	anySynFrame << "i";
	lexSynFrame << "i";
      } else {
	fullSynFrame << "x";
	anySynFrame << "x";
	lexSynFrame << "x";
      }
    }

    // found the predicate
    else if(crt == pred){
      fullSynFrame << "v";
      anySynFrame << "v";
      lexSynFrame << ((Tree *) crt)->getLemma();
    }

    // other phrases
    else {
      fullSynFrame << crt->getLabel();
      anySynFrame << crt->getLabel();
      lexSynFrame << crt->getLabel();
    }
  }

  frames.push_back(fullSynFrame.str());
  frames.push_back(anySynFrame.str());
  frames.push_back(lexSynFrame.str());  

  /*
  cout << "In sentence:\n" << * current 
       << "\nConstructing frame for predicate:\n"
       << * pred << " and argument:\n" << * arg 
       << "\nSequence of elements in the frame:\n";
  for(list<const Tree *>::const_iterator it = elements.begin();
      it != elements.end(); it ++){
    cout << * (* it) << "\n";
  }
  cout << "Full frame: " << fullSynFrame.str() << "\n"
       << " Any frame: " << anySynFrame.str() << "\n"
       << " Lex frame: " << lexSynFrame.str() << "\n";
  */
  
  return true;
}

String Tree::
detectSiblingPath(const Tree * start,
		  const Tree * end) const
{
  ostringstream os;
  bool foundStart = false;
  bool foundEnd = false;
  bool firstElem = true;

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    if((* it).operator->() == start) foundStart = true;
    if(foundStart){
      if(! firstElem) os << "+";
      os << (* it)->getLabel();
      firstElem = false;
    }
    if((* it).operator->() == end){
      foundEnd = true;
      break;
    }
  }

  RVASSERT(foundStart == true && foundEnd == true,
	   "Sibling path broken in phrase: " << * this);
  return os.str();
}

static bool goodArgument(const Argument & arg,
			 int position)
{
  if(arg.getVerbPosition() == position &&
     arg.getType() == "B" && // XXX: only handle good B args for now!
     arg.isClean()){
    return true;
  }

  return false;
}

void Tree::
generateExamplesForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			     const Tree * pred, 
			     int position,
			     const Tree * predParent,
			     bool caseSensitive)
{
  bool isValidLluis = isValidArgument(pred, predParent); // Lluis' heuristics
  //bool isValidPalmer = isValidPalmerHeuristic(pred); // Palmer heuristic
  bool includesPred = includesPosition(position); // must not include predicate

  bool isValid = isValidLluis;

  if(_parent != NULL && isValid && includesPred == false){

    bool foundPositive = false;
    for(list<Argument>::iterator it = _arguments.begin();
	it != _arguments.end(); it ++){
      if(goodArgument(* it, position)){
	foundPositive = true;
	generatePositiveExample(sentence,
				pred, position,
				(* it).getType(), 
				(* it).getName(),
				(* it).getBegin(),
				caseSensitive);
      }
    }
    if(foundPositive == false){ // && _reachedMaxNeg == false){
      generateNegativeExample(sentence, pred, position, caseSensitive);
    }
  } 

  bool hasArgForVerb = false;
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if(goodArgument(* it, position)){
      hasArgForVerb = true;
      break;
    }
  }
  
//  if(hasArgForVerb == false){
    for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
      (* it)->generateExamplesForPredicate(sentence, pred, 
					   position, predParent,
					   caseSensitive);
    }
//  }
}

void Tree::detectPredicates(vector<int> & predPositions)
{
  if(isTerminal() && isPredicate()){
    predPositions.push_back(getLeftPosition());
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectPredicates(predPositions);
  }
}

void Tree::
generatePositiveExample(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			const Tree * pred, 
			int position,
			const String & type,
			const String & name,
			const Tree * begin,
			bool caseSensitive)
{
  static int count = 0;

  if(count < POSITIVE_EXAMPLES_MAX_COUNT){

    vector<int> allFeats;
    vector<DebugFeature> debugFeats;
    generateExampleFeatures(sentence, pred, position, begin, true, 
			    allFeats, & debugFeats, true,
			    caseSensitive);

    LOGD << "Positive example using phrase:" << endl << * this;
    LOGD << "The following features are used:";
    for(size_t i = 0; i < debugFeats.size(); i ++){
      LOGD << " " << debugFeats[i]._name << "(" 
	   << debugFeats[i]._index << ")";
    }
    LOGD << endl;

    POS << type << "-" << name; // << "-" << position;
    for(size_t i = 0; i < allFeats.size(); i ++)
      POS << " " << allFeats[i] << ":1";
    POS << endl;

    count ++;

    if(count >= POSITIVE_EXAMPLES_MAX_COUNT){
      _reachedMaxPos = true;
      cerr << "Reached max limit for positive examples: " 
	   << POSITIVE_EXAMPLES_MAX_COUNT << endl;
    }
  }
}

void Tree::
generateNegativeExample(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			const Tree * pred, 
			int position,
			bool caseSensitive)
{
  static int count = 0;

  if(count < NEGATIVE_EXAMPLES_MAX_COUNT){

    vector<int> allFeats;
    generateExampleFeatures(sentence, pred, position, NULL, true, 
			    allFeats, NULL, false,
			    caseSensitive);

    NEG << "B-O";
    for(size_t i = 0; i < allFeats.size(); i ++)
      NEG << " " << allFeats[i] << ":1";
    NEG << endl;

    count ++;

    if(count >= NEGATIVE_EXAMPLES_MAX_COUNT){
      _reachedMaxNeg = true;
      cerr << "Reached max limit for negative examples: " 
	   << NEGATIVE_EXAMPLES_MAX_COUNT << endl;
    }
  }
}

static void addUniqueIndexes(vector<int> & all, 
			     const vector<String> & some,
			     const String & prefix,
			     bool createFeatures,
			     std::vector<DebugFeature> * debugFeats)
{
  for(size_t i = 0; i < some.size(); i ++){
    String name = prefix + some[i];
    int index = Lexicon::getFeatureIndex(name, createFeatures);
    if(index >= 0){
      bool found = false;
      for(size_t j = 0; j < all.size(); j ++){
	if(all[j] == index){
	  found = true;
	  break;
	}
      }
      if(! found){
	all.push_back(index);
	if(debugFeats != NULL){
	  DebugFeature d(name, index);
	  debugFeats->push_back(d);
	}
      }
    }
  }
}

#define USE_ARG_FEATURES 1
#define USE_PRED_FEATURES 1
#define USE_PATH_FEATURES 1

void Tree::
generateExampleFeatures(const std::vector< RCIPtr<srl::Tree> > & sentence, 
			const Tree * pred, 
			int position,
			const Tree * begin,
			bool createFeatures,
			vector<int> & allFeats,
			std::vector<DebugFeature> * debugFeats,
			bool positive, // not really used, set to false
			bool caseSensitive) const 
{
  //
  // generate the path attributes between arg and pred
  // 
  vector<String> predPathFeats;
  generatePathFeatures(sentence, pred, position, predPathFeats, positive);

  //
  // generate the path attributes between arg and begin
  //
  /*
  vector<String> beginPathFeats;
  if(begin != NULL) 
    generatePathFeatures(sentence, begin, -1, beginPathFeats, false); 
  */

  //////////////////////////////////////////////////////////////////////////
  //
  //                 Merge and sort all features
  //
  //////////////////////////////////////////////////////////////////////////

  // this stores all features for this example
  allFeats.clear();
  if(debugFeats != NULL) debugFeats->clear();

  // gather arg feats
  if(USE_ARG_FEATURES){
    addUniqueIndexes(allFeats, _argFeats, "", createFeatures, debugFeats);
  }

  // gather arg feats from left sibling
  /*
  RVASSERT(_leftSiblingNodes.size() == 0 ||
	   _leftSiblingNodes.size() == SIBLING_CONTEXT_LENGTH,
	   "Invalid sibling node count: " << _leftSiblingNodes.size());
  if(_leftSiblingNodes.front() != NULL)
    addUniqueIndexes(allFeats, _leftSiblingNodes.front()->_argFeats, 
		     "l1", createFeatures, debugFeats);
  */
  
  // gather arg feats from right sibling
  /*
  RVASSERT(_rightSiblingNodes.size() == 0 ||
	   _rightSiblingNodes.size() == SIBLING_CONTEXT_LENGTH,
	   "Invalid sibling node count: " << _rightSiblingNodes.size());
  if(_rightSiblingNodes.front() != NULL)
    addUniqueIndexes(allFeats, _rightSiblingNodes.front()->_argFeats, 
		     "r1", createFeatures, debugFeats);
  */

  // gather pred feats
  if(USE_PRED_FEATURES){
    addUniqueIndexes(allFeats, pred->_predFeats, "", 
		     createFeatures, debugFeats);
  }

  // gather path feats
  if(USE_PATH_FEATURES){
    addUniqueIndexes(allFeats, predPathFeats, "", 
		     createFeatures, debugFeats);
  }

  /*
  // path feats to begin
  addUniqueIndexes(allFeats, beginPathFeats, "beg", 
		   createFeatures, debugFeats);

  // arg feats from begin
  if(begin != NULL)
    addUniqueIndexes(allFeats, begin->_argFeats, "beg", 
		     createFeatures, debugFeats);
  */

  // sort
  sort(allFeats.begin(), allFeats.end());
}

bool Tree::isCollinsSpecial() const
{
  if(getLabel() == "," ||
     getLabel() == ":" ||
     getLabel() == "``" ||
     getLabel() == "''")
    return true;
  return false;
}

/**
 * Lluis's heuristic
 */
bool Tree::isValidArgument(const Tree * pred,
			   const Tree * firstSParent) const
{
  bool included = true;
  if(firstSParent != NULL) included = includedIn(firstSParent);

  // must be inside the S or to the left
  if(getLeftPosition() > pred->getRightPosition() && 
     ! included && ! isCollinsSpecial())
    return false;

  // if outside S can't be a terminal
  if(! included && isTerminal() && ! isCollinsSpecial())
    return false;

  return true;
}

bool Tree::isValidPalmerHeuristic(const Tree * pred) const
{
  // this is from Lluis's heuristic:
  //   accept any non-terminals to the left of the predicate
  if(getLeftPosition() < pred->getRightPosition() && 
     (! isTerminal() || isCollinsSpecial()))
    return true;

  for(const Tree * crtParent = pred->_parent; crtParent != NULL; 
      crtParent = crtParent->_parent){
    // is this an immediate children of crtParent?
    // valid args might be immediate childrens of PP children
    for(list<TreePtr>::const_iterator it = crtParent->_children.begin();
	it != crtParent->_children.end(); it ++){

      // check up to depth maxDepth
      int maxDepth = 2; // inspect two layers of children
      if((* it)->hasChild(this, 0, maxDepth)) return true;

      // this is the original Palmer heuristic
      /*
      // immediate children
      if((* it).operator->() == this){
	return true;
      }
      
      // children of a PP child
      if((* it)->getLabel() == "PP"){
	for(list<TreePtr>::const_iterator ppit = (* it)->_children.begin();
	    ppit != (* it)->_children.end(); ppit ++){
	  if((* ppit).operator->() == this){
	    return true;
	  }
	}
      }
      */

      // this is from Lluis' heuristic:
      //   stop looking when reaching the first S* parent
      if(crtParent->_label.substr(0, 1) == "S") break;
    }
  }

  return false;
}

bool Tree::hasChild(const Tree * child,
		    int crtDepth,
		    int maxDepth) const 
{
  // found it
  if(this == child) return true;

  if(crtDepth < maxDepth){
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      // this child matches
      if((* it)->hasChild(child, crtDepth + 1, maxDepth)) return true;
    }
  }

  return false;
}

void Tree::gatherLabelStats()
{
  Lexicon::addSyntacticLabelCount(getLabel());
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    Lexicon::addArgLabelCount((* it).getTypeName());

    if((* it).getType() == "B"){
      Lexicon::addArgValidLabel((* it).getTypeName(), getLabel());
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->gatherLabelStats();
  }
}

void Tree::propagatePredicate(const String & lemma,
			      bool flag)
{
  setLemma(lemma);
  setIsPredicate(flag);

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->propagatePredicate(lemma, flag);
  }
}

void Tree::detectBTypes()
{
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if((* it).getType() == "I"){
      int position = (* it).getVerbPosition();
      Tree * begin = (Tree *) (* it).getBegin();
      RVASSERT(begin != NULL, "Found I without B: " << * this);
      RVASSERT(_parent != NULL, "Found I without parent: " << * this);
      RVASSERT(begin->_parent != NULL, "Found B without parent: " << * this);
      
      bool found = false;
      for(list<Argument>::iterator bit = begin->_arguments.begin();
	  bit != begin->_arguments.end(); bit ++){
	// found ze B
	if((* bit).getType() == "B" &&
	   (* bit).getVerbPosition() == position){

	  if(_parent == begin->_parent &&
	     isTerminal() &&
	     begin->isTerminal()){
	    if((* bit).getBType() == B_SINGLE ||
	       (* bit).getBType() == B_INC_TERM){
	      if(getRightPosition() == position - 1 ||
		 getRightPosition() == _parent->getRightPosition())
		(* bit).setBType(B_FULL_TERM);
	      else
		(* bit).setBType(B_INC_TERM);
	    }

	  } else if(begin->_parent != _parent){
	    (* bit).setBType(B_UGLY);
	  } else {
	    (* bit).setBType(B_SAMEP);
	  }

	  found = true;
	  break;
	}
      }
      RVASSERT(found, "Found I arg without B arg: " << * this);
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectBTypes();
  }
}

void Tree::countBTypes(int & single,
		       int & fullTerm,
		       int & incTerm,
		       int & samep,
		       int & ugly) const
{
  for(list<Argument>::const_iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if((* it).getType() == "B"){
      if((* it).getBType() == B_SINGLE) single ++;
      else if((* it).getBType() == B_FULL_TERM){
	cout << "FOUND B FULL TERM:\n" << * this 
	     << "IN PARENT:\n" << * _parent << endl;
	fullTerm ++;
      }
      else if((* it).getBType() == B_INC_TERM){
	cout << "FOUND B INC TERM:\n" << * this 
	     << "IN PARENT:\n" << * _parent << endl;
	incTerm ++;
      }
      else if((* it).getBType() == B_SAMEP){
	cout << "FOUND B SPLIT WITH SAME PARENT:\n" << * this 
	     << "IN PARENT:\n" << * _parent << endl;
	samep ++;
      }
      else if((* it).getBType() == B_UGLY){
	cout << "FOUND B SPLIT UGLY:\n" << * this 
	     << "IN PARENT:\n" << * _parent << endl;
	ugly ++;
      }

      if((* it).getBType() != B_UGLY &&
	 (* it).getBType() != B_SINGLE)
	Lexicon::addBICount((* it).getName());
      
    }
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->countBTypes(single, fullTerm, incTerm, samep, ugly);
  }
}

bool Tree::ugglyPhrase() const
{
  for(list<Argument>::const_iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if((* it).getType() == "B" &&
       ((* it).getBType() == B_SAMEP ||
	(* it).getBType() == B_UGLY)){
      return true;
    }
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    if((* it)->ugglyPhrase()) return true;
  }

  return false;
}

void Tree::detectSiblings()
{
  Tree * previous = NULL;
  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    list<TreePtr>::iterator next = it; 
    next ++;

    if(previous != NULL) (* it)->setLeftSibling(previous);
    if(next != _children.end()) (* it)->setRightSibling((* next).operator->());

    previous = (* it).operator->();
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->detectSiblings();
  }
}

const Tree * Tree::findMatchingTree(int start, int end)
{
  if(_leftPosition == start &&
     _rightPosition == end)
    return this;

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    const Tree * t = (* it)->findMatchingTree(start, end);
    if(t != NULL) return t;
  }

  return NULL;
}

