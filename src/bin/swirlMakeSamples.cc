/**
 * This program trains the SRL system
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
       << " <input parsed file with args> <model directory>" <<  endl;
  CERR << "Valid parameters:" << endl
       << "\tcase-insensitive - generate case-insensitive models" << endl;
  CERR << "Note: use the convertToTreebank program to generate the parse/arg file" << endl;
  CERR << endl;
}

#define GENERATE_SAMPLE_FILE true

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

  Lexicon::initialize(modelPath, false);

  TreePtr tree;
  vector<TreePtr> sentence;
  int count = 0;
  //int ugglyCount = 0;

  Tree::initializePathLengths();

  try{
    String posName = modelPath + "/positive.samples";
    String negName = modelPath + "/negative.samples";
    string fname = mergeStrings(modelPath, "/feature.lexicon");

    if(GENERATE_SAMPLE_FILE){
      Tree::createSampleStreams(posName, negName);
      
      cerr << "Generating all training samples...\n";
      cerr << "Reading trees...\n";
      while((tree = tp->create(treeStream, sentence, caseSensitive)) != (const Tree *) NULL){
	LOGD << "Tree #" << count << ":" << endl << tree << endl;

	if(Tree::reachedMaxExamples()){
	  cerr << "Reached the desired number of examples." << endl;
	  break;
	}
      
	/*
	if(tree->ugglyPhrase()){
	  LOGH << "Found ugly phrase: " << endl << * tree << endl;
	  ugglyCount ++;
	} else {
	*/
	tree->generateExamples(sentence, caseSensitive);
	//}
      
	// increment number of processed sentences
	count ++;
      }
    
      cerr << "Processed " << count << " sentences." << endl;
      //cerr << "Eliminated " << ugglyCount << " ugly sentences." << endl;

      //
      // Save all features
      //
      ofstream featStream(fname.c_str());
      RASSERT(featStream, "Failed to create feature lexicon stream!");
      Lexicon::saveFeatureLexicon(featStream);
      featStream.close();
    } // GENERATE_SAMPLE_FILE

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



