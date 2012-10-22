#include "Tree.h"
#include "AssertLocal.h"
#include "CharUtils.h"
#include "Head.h"
#include "Lexicon.h"

using namespace std;
using namespace srl;

void Tree::checkArgumentInclusion()
{
  for(list<Argument>::iterator it = _arguments.begin();
	it != _arguments.end(); it ++){
    int position = (* it).getVerbPosition();
    const Tree * crt = NULL;
    
    for(crt = _parent; crt != NULL; crt = crt->_parent){
      bool found = false;
      for(list<Argument>::const_iterator pit = crt->_arguments.begin();
	  pit != crt->_arguments.end(); pit ++){
	if((* pit).getVerbPosition() == position){
	  found = true;
	  break;
	}
      }
      if(found){
	cout << "FOUND ARG INCLUSION!" << endl << * crt << endl;
      }
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->checkArgumentInclusion();
  }
}

void Tree::checkPredicateInclusion()
{
  for(list<Argument>::iterator it = _arguments.begin();
	it != _arguments.end(); it ++){
    int position = (* it).getVerbPosition();

    if(getLeftPosition() <= position && position <= getRightPosition()){
	cout << "FOUND PREDICATE INCLUSION!" << endl << * this << endl;
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->checkPredicateInclusion();
  }
}

StringMap<ArgCounts *> Tree::_argCounts;

ArgCounts * Tree::getArgCounts(const String & arg)
{
  ArgCounts * s = NULL;
  if(_argCounts.get(arg.c_str(), s) == false){
    s = new ArgCounts();
    _argCounts.set(arg.c_str(), s);
  }
  RVASSERT(s != NULL, "Can not find argCounts for argument: " << arg);
  return s;
}

void Tree::reportArgCounts(std::ostream & os)
{
  for(StringMap<ArgCounts *>::const_iterator it = _argCounts.begin();
      it != _argCounts.end(); it ++){
    ArgCounts * ac = (* it).second->getValue();
    os << (* it).second->getKey() << " " 
       << ac->b << " " << ac->bwi << " " 
       << ac->cori << " " << ac->inci << " " << ac->termi << endl;
  }
}

Tree * Tree::containsBForI(const Tree * iArg, 
			   int verbPosition)
{
  for(list<TreePtr>::const_iterator sit = _children.begin();
      sit != _children.end() && (* sit).operator->() != iArg;
      sit ++){

    for(list<Argument>::iterator ait = (* sit)->_arguments.begin();
	ait != (* sit)->_arguments.end(); ait ++){
      if(verbPosition == (* ait).getVerbPosition() &&
	 (* ait).getType() == "B"){
	return (Tree *) (* sit).operator->();
      }
    }
  }

  return NULL;
}

static bool importantArg(const String & name)
{
  static char * args [] = { "A0", "A1", "A2", NULL };
  for(int i = 0; args[i] != NULL; i ++)
    if(name == args[i]) return true;
  return false;
}

void Tree::countBIs()
{
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if(importantArg((* it).getName())){
      ArgCounts * ac = getArgCounts((* it).getName());

      if((* it).getType() == "I"){ // I
	int position = (* it).getVerbPosition();
	RVASSERT(_parent != NULL, "Found I-A wo parent!" << endl << * this);
	Tree * b;

	if((b = _parent->containsBForI(this, position)) != NULL){ // good I
	  ac->cori ++;
	  if(b->isTerminal() && isTerminal()) ac->termi ++;
	} else { // bad I
	  ac->inci ++;
	}
      } else { // B
	ac->b ++;
      } 
    } 
  }

  for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
    (* it)->countBIs();
  }
}

void Tree::verifyTokenDistances()
{
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    int position = (* it).getVerbPosition();
    int dist = 0;
    if(position < getLeftPosition()) 
      dist = getLeftPosition() - position - 1;
    else if(position > getRightPosition())
      dist = position - getRightPosition() - 1;

    cout << "P-A TOKEN DIST: " << dist << endl;
    if(dist > Lexicon::maxTokenDistance) Lexicon::maxTokenDistance = dist;
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->verifyTokenDistances();
  }
}

bool Tree::checkIAlignment()
{
  bool foundError = false;

  for(list<Argument>::iterator it = _arguments.begin();
	it != _arguments.end(); it ++){
    if((* it).getType() == "I"){
      int position = (* it).getVerbPosition();
      if(_parent == NULL){
	cout << "FOUND I-A WITHOUT PARENT" << endl << * this << endl;
	foundError = true;
      } else{
	bool found = (_parent->containsBForI(this, position) != NULL);
	/*
	for(list<TreePtr>::const_iterator sit = _parent->_children.begin();
	    sit != _parent->_children.end() && (* sit).operator->() != this; 
	    sit ++){

	  for(list<Argument>::iterator ait = (* sit)->_arguments.begin();
	      ait != (* sit)->_arguments.end(); ait ++){
	    if(position == (* ait).getVerbPosition() &&
	       (* ait).getType() == "B"){
	      found = true;
	      break;
	    }
	  }

	  if(found) break;
	}
	*/
	if(! found){
	  // if(getLabel() != ",")
	  cout << "FOUND I-A WITHOUT B-A" << endl << * this 
	       << "IN PARENT PHRASE" << endl << * _parent << endl;
	  foundError = true;
	}
      }
    }
  }

  if(! foundError){
    for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
      if((* it)->checkIAlignment() == true){
	foundError = true;
	break;
      }
    }
  }

  return foundError;
}

int Tree::_commaICount = 0;

void Tree::countCommaIs(const std::vector< RCIPtr<srl::Tree> > & sentence)
{
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if((* it).getType() == "I" && getLabel() == ","){
      _commaICount ++;
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->countCommaIs(sentence);
  }
}

bool Tree::checkLluis(const std::vector< RCIPtr<srl::Tree> > & sentence)
{
  // detect all predicates in this phrase
  vector<int> predPositions;
  detectPredicates(predPositions);

  for(size_t i = 0; i < predPositions.size(); i ++){
    int pos = predPositions[i];
    RVASSERT(pos >= 0 && pos < (int) sentence.size(),
	    "Invalid predicate position " << pos);
    const Tree * pred = sentence[pos].operator->();
    RVASSERT(pred->isPredicate(),
	    "Chosen phrase is not a predicate!");

    const Tree * s = pred->_parent;
    while(s != NULL && s->getLabel().substr(0, 1) != "S")
      s = s->_parent;

    if(s != NULL){
      checkArgumentPositionsForPredicate(sentence, pred, s);
    } else{
      // This should not happen
      cout << "FOUND PREDICATE WITHOUT S: " << * this;
    }
  }

  return true;
}

// This counts how many arguments are matched by Lluis' pruning strategy
int argumentsMatchedByPruningCount = 0;
int argumentsNotMatchedByPruningCount = 0;

void Tree::checkArgumentPositionsForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence, 
					      const Tree * pred,
					      const Tree * s)
{

  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    int position = (* it).getVerbPosition();
    if(position == pred->getRightPosition()){
      bool included = includedIn(s);
      
      // Lluis' heuristic
      if(includesPosition(pred->getRightPosition())){
	argumentsNotMatchedByPruningCount ++;
      } else if(getLeftPosition() > pred->getRightPosition() && 
	 ! included && ! isCollinsSpecial()){
	cout << "FOUND ARG NOT ACCORDING TO LLUIS:\n" << * this;
	argumentsNotMatchedByPruningCount ++;
      } else {
	if(! included && isTerminal() && ! isCollinsSpecial()){
	  cout << "FOUND TERMINAL OUTSIDE S: " << * this;
	  argumentsNotMatchedByPruningCount ++;
	} else {
	  argumentsMatchedByPruningCount ++;
	}
      }
      break;
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->checkArgumentPositionsForPredicate(sentence, pred, s);
  }
}

bool Tree::checkPalmer(const std::vector< RCIPtr<srl::Tree> > & sentence)
{
  // detect all predicates in this phrase
  vector<int> predPositions;
  detectPredicates(predPositions);

  for(size_t i = 0; i < predPositions.size(); i ++){
    int pos = predPositions[i];
    RVASSERT(pos >= 0 && pos < (int) sentence.size(),
	    "Invalid predicate position " << pos);
    const Tree * pred = sentence[pos].operator->();
    RVASSERT(pred->isPredicate(),
	    "Chosen phrase is not a predicate!");

    checkPalmerForPredicate(sentence, pred);
  }

  return true;
}

// This counts how many arguments are matched by Palmer's pruning strategy
int argumentsMatchedByPalmerCount = 0;
int argumentsNotMatchedByPalmerCount = 0;

void Tree::checkPalmerForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence, 
				   const Tree * pred)
{
  for(list<Argument>::iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    int position = (* it).getVerbPosition();

    // found an arg for the current predicate
    if(position == pred->getRightPosition()){
      if(isValidPalmerHeuristic(pred)) argumentsMatchedByPalmerCount ++;
      else argumentsNotMatchedByPalmerCount ++;

      break;
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->checkPalmerForPredicate(sentence, pred);
  }
}
