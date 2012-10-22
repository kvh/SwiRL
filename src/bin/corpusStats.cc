/**
 * This program extracts patterns from parsed files
 */

#include <iostream>
 
#include "BankTreeProducer.h"
#include "Constants.h"
#include "Lexicon.h"
#include "Parameters.h"
#include "CharUtils.h"
#include "AssertLocal.h"
#include "Wnet.h"
 
using namespace std;
using namespace srl;

static void usage(const char * name)
{
  CERR << "Usage: " << name << " [ <parameters> ] " 
       << " <input parsed file with args> <model directory>" <<  endl;
  CERR << "Valid parameters:" << endl << endl;
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

  string modelPath = argv[idx + 1];

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

  TreePtr tree;
  vector<TreePtr> sentence;
  int count = 0;

  cerr << "Generating corpus statistics...\n";
  cerr << "Reading trees...\n";
  while(tp->gatherStats(treeStream)){
    // increment number of processed sentences
    count ++;
  }

  Lexicon::saveCounts(modelPath);
  return 0;
}



