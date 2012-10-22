/**
 * This program detects SR frames in free text.
 * It incorporates Charniak's parsing, followed by my SRL code.
 */

#include <iostream>
#include <cstring>
 
#include "Constants.h"
#include "Parameters.h"
#include "CharUtils.h"
#include "AssertLocal.h"
#include "Exception.h"
#include "Logger.h"
#include "Swirl.h"
#include "Oracle.h"
 
using namespace std;
using namespace srl;

#define MAX_LINE (16 * 1024)

static void usage(const char * name)
{
  CERR << "Usage: " << name << " [ <parameters> ] " 
       << " <srl model directory> <charniak model directory> ";
  CERR << "<data file>?\n"; 
  CERR << "Valid parameters:\n"
       << "\tverbose - Set verbosity level\n"
       << "\tno-prompt - do not display the shell prompt\n"
       << "\tcase-insensitive - use case-insensitive models\n" 
       << "\tgold-props - run in oracle mode using these gold propositions\n";
}

static void processFile(IStream & treeStream,
			const vector<OracleSentence *> & goldProps)
{
  // full parse of the current sentence
  TreePtr tree;
  int count = 1;
  char line[MAX_LINE];
  
  try{
    while(treeStream.getline(line, MAX_LINE) != NULL){
      // classify all predicates in this sentence
      OracleSentence * oracleSent = NULL;
      if(count <= (int) goldProps.size()) oracleSent = goldProps[count - 1];
      if(oracleSent == NULL) tree = Swirl::parse(line);
      else tree = Swirl::oracleParse(line, oracleSent);
      RVASSERT(tree != (const Tree *) NULL,
	       "Failed to parse line: " << line);
      
      // dump CoNLL format
      Swirl::displayProps(tree, cout, false);
      
      if(count % 100 == 0){
	CERR << "Processed " << count << " sentences..." << endl;
      }
      
      // increment number of processed sentences
      count ++;
    }
    
    CERR << "Done. Processed " << count - 1 << " sentences." << endl;
    
  } catch(Exception e){
    CERR << e.getMessage() << endl
	 << "Exception caught in main. Exiting..." << endl;
    exit(-1);
  } catch(...){
    CERR << "Exception caught in main. Exiting..." << endl;
    exit(-1);
  }

  if(goldProps.size() > 0) Tree::printOracleStats(cerr);
}

static void interactiveShell(bool showPrompt)
{
  TreePtr tree;
  char line[MAX_LINE];

  if(showPrompt){
    cout << "SRL> ";
  } else{
    // executed from a Java wrapper => print initialization ack
    cout << "OK\n";
  }
  while(cin.getline(line, MAX_LINE) != NULL){
    if(strlen(line) == 0) break;

    // classify all predicates in this sentence
    tree = Swirl::parse(line);

    // dump extended Treebank format if used in the interactive shell
    if(tree != (const Tree *) NULL && showPrompt){
      tree->serialize(cout);
      cout << "\n\n";
    }

    // dump extended CoNLL format
    Swirl::serialize(tree, line, cout);
    if(showPrompt) cout << "SRL> ";
  }
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
  
  //String classifierType = "ab";
  //Parameters::get("classifier", classifierType);

  bool showPrompt = true;
  if(Parameters::contains("no-prompt")) showPrompt = false;

  bool caseSensitive = true; // default: case sensitive
  if(Parameters::contains("case-insensitive")) caseSensitive = false;

  //
  // if the gold propositions are given we're running in oracle mode, i.e.,
  //   we are gathering various statistics and computing the score upper limits
  //
  vector<OracleSentence *> goldProps;
  String goldPropFile;
  if(Parameters::get("gold-props", goldPropFile)){
    ifstream is(goldPropFile.c_str());
    if(! is){
      cerr << "Can not open file: " << goldPropFile << endl;
      exit(-1);
    }
    OracleSentence::readSentences(is, goldProps); 
  }
  
  if(idx > argc - 2){
    usage(argv[0]);
    exit(-1);
  }

  if(! Swirl::initialize(argv[idx + 0], argv[idx + 1], caseSensitive)){ 
    cerr << "Failed to initialize SRL system!\n";
    exit(1);
  } 

  int offset = 2;
  if(idx <= argc - (offset + 1)){
    string treeFile = argv[idx + offset];
    IFStream treeStream(treeFile.c_str());
    if(! treeStream){
      cerr << "Can not open file: " << treeFile << endl;
      exit(-1);
    }
    processFile(treeStream, goldProps);
  } else {
    interactiveShell(showPrompt);
  }
  
  return 0;
}



