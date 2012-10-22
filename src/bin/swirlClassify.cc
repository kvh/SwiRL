/**
 * This program detects SR frames in pre-parsed files
 */

#include <iostream>
 
#include "BankTreeProducer.h"
#include "Constants.h"
#include "Parameters.h"
#include "CharUtils.h"
#include "Lexicon.h"
#include "AssertLocal.h"
#include "Exception.h"
#include "Logger.h"
#include "Wnet.h"
 
using namespace std;
using namespace srl;

static void usage(const char * name)
{
  CERR << "Usage: " << name << " [ <parameters> ] " 
       << " <data file> <model directory>" <<  endl;
  CERR << "Valid parameters:" << endl 
       << "\tverbose - Set verbosity level" << endl
       << "\tclassifier = ab | svm" << endl
       << "\tcase-insensitive - use case-insensitive models" << endl;
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

  bool caseSensitive = true; // default: case sensitive
  if(Parameters::contains("case-insensitive")) caseSensitive = false;

  if(idx > argc - 2){
    usage(argv[0]);
    exit(-1);
  }

  string treeFile = argv[idx];
  IFStream treeStream(treeFile.c_str());
  if(! treeStream){
    cerr << "Can not open file: " << treeFile << endl;
    exit(-1);
  }

  string logFile = treeFile + ".log";
  Logger::setLogFile(logFile);

  cerr << "Loading models..." << endl;
  string modelPath = argv[idx + 1];
  Tree::loadClassifierModels(modelPath.c_str());

  RCIPtr<TreeProducer> tp;
  RCIPtr<Morpher> wn;

  try{
    tp = RCIPtr<TreeProducer>(new BankTreeProducer);
    wn = RCIPtr<Morpher>(new WordNet);
  } catch(...){
    CERR << "Some module initialization failed. Exiting..." << endl;
    exit(-1);
  }

  Tree::registerMorpher(wn);

  Lexicon::initialize(modelPath, true);

  TreePtr tree;
  vector<TreePtr> sentence;
  int count = 1;

  try{
    LOGM << "Reading trees..." << endl;
    while((tree = tp->create(treeStream, sentence, caseSensitive)) != 
	  (const Tree *) NULL){
      LOGH << "Tree #" << count << ":" << endl << tree << endl;
      //cout << "Tree #" << count << ":" << endl << tree << endl;
      //cout.flush();
      //cin.get();

      // classify args for all preds
      tree->classify(sentence, count - 1, caseSensitive);

      // dump CoNLL format
      tree->dumpCoNLL(cout, sentence, false);

      /*
      // convert to API input
      cout << "0 ";
      for(size_t i = 0; i < sentence.size(); i ++){
	cout << sentence[i]->getWord() << " ";
	if(! sentence[i]->getNe().empty()) cout << sentence[i]->getNe();
	else cout << "O";
	cout << " ";
	if(sentence[i]->isPredicate()) cout << sentence[i]->getLemma();
	else cout << "-";
	cout << " ";
      }
      cout << "\n";
      */

      /*
      tree->gatherScoringStats();

      list<string> missedArgs, excessArgs;
      tree->gatherListOfMissedArgs(missedArgs, excessArgs);
      if(! missedArgs.empty() || ! excessArgs.empty()){
	if(! missedArgs.empty()){
	  LOGM << "Below, args missed:";
	  for(list<string>::const_iterator it = missedArgs.begin();
	      it != missedArgs.end(); it ++){
	    LOGM << " " << * it;
	  }
	  LOGM << endl;
	}
	if(! excessArgs.empty()){
	  LOGM << "Below, args in excess:";
	  for(list<string>::const_iterator it = excessArgs.begin();
	      it != excessArgs.end(); it ++){
	    LOGM << " " << * it;
	  }
	  LOGM << endl;
	}
	LOGM << tree << endl;
      }
      */

      if(count % 100 == 0){
	CERR << "Processed " << count << " sentences..." << endl;
      }
      
      // increment number of processed sentences
      count ++;
    }
    
    CERR << "Done. Processed " << count - 1 << " sentences." << endl;
    if(verbosity > 0) Tree::dumpScoringStats(Logger::logStream());

  } catch(Exception e){
    CERR << e.getMessage() << endl
	 << "Exception caught in main. Exiting..." << endl;
    exit(-1);
  } catch(...){
    CERR << "Exception caught in main. Exiting..." << endl;
    exit(-1);
  }

  return 0;
}



