/**
 * This program trains the SRL system
 */

#include <iostream>
 
#include "BankTreeProducer.h"
#include "Constants.h"
#include "Parameters.h"
#include "CharUtils.h"
#include "Lexicon.h"
#include "Assert.h"
#include "Exception.h"
#include "Wn.h"
#include "Logger.h"
 
using namespace std;
using namespace srl;

static void usage(const char * name)
{
  CERR << "Usage: " << name 
       << " <argument label in focus> <model directory>" 
       <<  endl;
}

/**
 * Adds binary samples to the given argument stream.
 * Each sample is labeled as positive (+1) if its label is equal to the 
 * corresponding argument label, and negative otherwise.
 */
static void addBinarySamples(istream & sampleStream,
			     ofstream & argStream,
			     const String & argLabel,
			     const StringMap<bool> & indeces)
{
  char line[MAX_SAMPLE_LINE];
  while(sampleStream.getline(line, MAX_SAMPLE_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    
    // the corresponding arg label is always token 0
    String label = tokens[0];

    // maintain only the features that appear in the indeces map
    vector<String> features;
    for(size_t i = 1; i < tokens.size(); i ++){
      // first token is the feature index, second is weight
      vector<String> featureTokens;
      simpleTokenize(tokens[i], featureTokens, ":");
      RVASSERT(featureTokens.size() == 2, "Invalid sample line: " << line);
      
      // the feature appears in the indeces map => it has high freq in training
      bool exists;
      if(indeces.get(featureTokens[0].c_str(), exists) == true){
	features.push_back(tokens[i]);
      }
    }

    // found zero common features
    if(features.size() == 0) continue;

    // add this sample to all argument streams
    // this is a positive sample if its label matches the current arg label
    String sampleLabel = "-1";
    if(label == argLabel) sampleLabel = "+1";
    argStream << sampleLabel;
    for(size_t j = 0; j < features.size(); j ++){
      argStream << " " << features[j];
    }
    argStream << "\n";
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

  if(idx > argc - 2){
    usage(argv[0]);
    exit(-1);
  }

  string argLabel = argv[idx];
  string modelPath = argv[idx + 1];

  String posName = modelPath + "/positive.samples";
  String negName = modelPath + "/negative.samples";
  String featFileName = mergeStrings(modelPath, "/feature.lexicon");
  String trainFile = modelPath + "/TRAIN.B-" + argLabel;

  // 
  // load indeces for all features that appear > DISCARD_THRESHOLD
  //
  ifstream featStream(featFileName.c_str());
  StringMap<bool> featIndeces;
  RVASSERT(featStream, "Unable to open feature file: " + featFileName);
  char line[MAX_FEATURE_LINE];
  int featCount = 0;
  while(featStream.getline(line, MAX_FEATURE_LINE)){
    featCount ++;
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    RVASSERT(tokens.size() == 3, "Invalid line in feature file: " << line);
    int freq = strtol(tokens[2].c_str(), NULL, 10);
    RVASSERT(freq > 0, "Invalid feature frequency in line: " << line);
    if(freq > DISCARD_THRESHOLD){
      featIndeces.set(tokens[1].c_str(), true);
    }
  }
  featStream.close();
  cerr << "Inspected " << featCount << " features.\n";
  cerr << "Kept " << featIndeces.size() << " after feature filtering.\n";

  //
  // open the stream for the binary samples
  //
  ofstream os(trainFile.c_str());
  RVASSERT(os, "Failed to create binary train file: " << trainFile);

  //
  // populate the argument stream with examples from positive.samples
  //
  ifstream posStream(posName.c_str());
  RVASSERT(posStream, "Failed to open sample file: " + posName);
  cerr << "Generating samples from file: " << posName << "\n";
  addBinarySamples(posStream, os, "B-" + argLabel, featIndeces);
  posStream.close();

  //
  // populate the argument stream with examples from negative.samples
  //
  ifstream negStream(negName.c_str());
  RVASSERT(negStream, "Failed to open sample file: " + negName);
  cerr << "Generating samples from file: " << negName << "\n";
  addBinarySamples(negStream, os, "B-" + argLabel, featIndeces);
  negStream.close();
  
  //
  // close the argument training stream
  //
  os.close();
  return 0;
}



