/**
 * This program generate the Treebank-formatted file with all the info
 * required to train the SRL system: parse trees with NE and arg annotations.
 */

#include <iostream>
 
#include "BankTreeProducer.h"
#include "Constants.h"
#include "Parameters.h"
#include "CharUtils.h"
#include "Lexicon.h"
#include "AssertLocal.h"
#include "Exception.h"
#include "Wnet.h"
#include "Logger.h"
 
using namespace std;
using namespace srl;

static void usage(const char * name)
{
  CERR << "Usage: " << name << " [ <parameters> ] " 
       << " <conll words> <conll nes> <conll charniak> <conll props>" <<  endl;
  CERR << "Valid parameters: " << endl
       << "  --skip-empty - skips sentences without predicates (for training)" << endl;
  
  CERR << endl;
}

int main(int argc,
	 char ** argv)
{
  int idx = -1;

  try{
    idx = Parameters::read(argc, argv);
    // Parameters::display(CERR);
  } catch(...){
    CERR << "Exiting..." << endl;
    exit(-1);
  }

  if(Parameters::contains(W("help"))){
    usage(argv[0]);
    exit(-1);
  }

  int verbosity = 0;
  Parameters::get("verbose", verbosity);
  Logger::setVerbosity(verbosity);

  bool skipEmptySentences = false;
  if(Parameters::contains(W("skip-empty"))){
    skipEmptySentences = true;
  }

  if(idx > argc - 4){
    usage(argv[0]);
    exit(-1);
  }

  string wordsFile = argv[idx];
  IFStream wordsStream(wordsFile.c_str());
  if(! wordsStream){
    cerr << "Can not open file: " << wordsFile << endl;
    exit(-1);
  }

  string nesFile = argv[idx + 1];
  IFStream nesStream(nesFile.c_str());
  if(! nesStream){
    cerr << "Can not open file: " << nesFile << endl;
    exit(-1);
  }

  string syntFile = argv[idx + 2];
  IFStream syntStream(syntFile.c_str());
  if(! syntStream){
    cerr << "Can not open file: " << syntFile << endl;
    exit(-1);
  }

  string propsFile = argv[idx + 3];
  IFStream propsStream(propsFile.c_str());
  if(! propsStream){
    cerr << "Can not open file: " << propsFile << endl;
    exit(-1);
  }
  
  cerr << "Converting CoNLL data to internal format...\n";
  TreePtr tree;
  bool hasPredicates = false;
  while((tree = Tree::convert(wordsStream, 
			      nesStream, 
			      syntStream, 
			      propsStream,
			      hasPredicates)) != (const Tree *) NULL){
    if(hasPredicates == true || skipEmptySentences == false){
      tree->displayTreebank(cout);
      cout << endl;
    } 
  }
}
