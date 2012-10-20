/**
 * Converts the CoNLL corpus in the format required for testing
 * See swirl/README for a description of the generated file (look for:
 *   "Parsing a complete file"
 */

#include <iostream>
#include <vector>

#include "Parameters.h"
#include "CharUtils.h"
#include "Assert.h"
#include "Exception.h"
#include "Wn.h"
#include "Logger.h"
 
using namespace std;
using namespace srl;

#define MAX_LINE 32 * 1024

class Token 
{
public:
  string mWord;
  string mNe;
  string mPred;

  Token(const string & w,
	const string & n,
	const string & p) {
    mWord = w;
    mNe = n;
    mPred = p;
  }
};

static void dumpSentence(const vector<Token> & sent,
			 ostream & os)
{
  os << "0 ";
  for(size_t i = 0; i < sent.size(); i ++){
    os << sent[i].mWord << " " << sent[i].mNe << " " << sent[i].mPred << " ";
  }
  os << endl;
}

static void convert(istream & wordStream,
		    istream & neStream,
		    istream & propStream,
		    ostream & os)
{
  char wordLine[MAX_LINE];
  char neLine[MAX_LINE];
  char propLine[MAX_LINE];
  vector<Token> sent;
  string currentNe;

  while(wordStream.getline(wordLine, MAX_LINE)){
    neStream.getline(neLine, MAX_LINE);
    propStream.getline(propLine, MAX_LINE);

    vector<string> wordTokens;
    vector<string> neTokens;
    vector<string> propTokens;
    simpleTokenize(wordLine, wordTokens, " \t\n\r");
    simpleTokenize(neLine, neTokens, " \t\n\r");
    simpleTokenize(propLine, propTokens, " \t\n\r");

    // reached EOS
    if(wordTokens.size() == 0){
      dumpSentence(sent, os);
      sent.clear();
    }

    // continue the current sent
    else {
      string ne = "O";
      if(startsWith(neTokens[0], "(")){
	int star = neTokens[0].find_first_of("*");
	currentNe = neTokens[0].substr(1, star - 1);
	ne = "B-" + currentNe;
      } else {
	if(currentNe.size() > 0) ne = "I-" + currentNe;
      }
      if(endsWith(neTokens[0], ")")) currentNe = "";

      string word = wordTokens[0];
      if(word == "(" || word == "{" || word == "[") word = "LRB";
      else if(word == ")" || word == "}" || word == "]") word = "RRB";
      
      sent.push_back(Token(word, ne, propTokens[0]));
    }
  }

  if(sent.size() > 0) dumpSentence(sent, os);
}

static void usage(const char * name)
{
  CERR << "Usage: " << name 
       << " <conll words> <conll nes> <conll props>" <<  endl;
  CERR << endl;
}

int main(int argc,
	 char ** argv)
{
  int idx = -1;
  
  try{
    idx = Parameters::read(argc, argv);
  } catch(...){
    CERR << "Exiting..." << endl;
    exit(-1);
  }

  if(Parameters::contains(W("help"))){
    usage(argv[0]);
    exit(-1);
  }

  if(idx > argc - 3){
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

  string propsFile = argv[idx + 2];
  IFStream propsStream(propsFile.c_str());
  if(! propsStream){
    cerr << "Can not open file: " << propsFile << endl;
    exit(-1);
  }
  
  convert(wordsStream, nesStream, propsStream, cout);

  wordsStream.close();
  nesStream.close();
  propsStream.close();
}
