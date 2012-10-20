
#include "Oracle.h"
#include "Constants.h"
#include "CharUtils.h"

using namespace std;
using namespace srl;

bool OracleSentence::readSentences(std::istream & is,
				   std::vector<OracleSentence *> & sentences)
{
  OracleSentence * sent;
  while((sent = readSentence(is)) != NULL)
    sentences.push_back(sent);
  cerr << "Read " << sentences.size() << " GOLD sentences.\n";
  return true;
}
    
OracleSentence * OracleSentence::readSentence(std::istream & is)
{
  //
  // read the sentence matrix
  //
  vector< vector<string> > matrix;
  char line[MAX_PROP_LINE];
  while(is.getline(line, MAX_PROP_LINE)){
    vector<string> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    if(tokens.size() == 0) break;
    matrix.push_back(tokens);
  }

  if(matrix.size() == 0) return NULL;
  OracleSentence * sent = new OracleSentence();

  //
  // construct the empty frames
  //
  for(size_t i = 0; i < matrix.size(); i ++){
    if(matrix[i][0] != "-"){
      sent->mFrames.push_back(OracleFrame(i));
    }
  }

  //
  // populate all frames
  // 
  for(size_t i = 0; i < sent->mFrames.size(); i ++){
    sent->mFrames[i].readArguments(matrix, i + 1);
  }

  return sent;
}

void OracleFrame::readArguments(std::vector< std::vector<string> > & matrix,
				int column)
{
  for(size_t i = 0; i < matrix.size(); i ++){
    const string & token = matrix[i][column];
    
    // an arg starts here
    if(startsWith(token, "(")){
      int star = token.find_first_of("*");
      string arg = token.substr(1, star - 1);
      //cerr << "argument: " << arg << endl;
      mArgs.push_back(OracleArg(arg, i, -1));
    }

    // an arg ends here
    if(endsWith(token, ")")){
      mArgs.back().mRightPosition = i;
    }
  }
}

void OracleSentence::setGoldArguments(Tree * tree)
{
  // traverse all frames
  for(size_t i = 0; i < mFrames.size(); i ++){
    // traverse all args in the current frame
    for(list<OracleArg>::iterator it = mFrames[i].mArgs.begin();
	it != mFrames[i].mArgs.end(); it ++){
      tree->createGoldArgument(mFrames[i].mVerbPosition,
			       it->mName, 
			       it->mLeftPosition, 
			       it->mRightPosition);
    }
  }
}

bool Tree::createGoldArgument(int verbPosition,
			      const String & name,
			      int leftPosition,
			      int rightPosition)
{
  if(getLeftPosition() == leftPosition &&
     getRightPosition() == rightPosition){
    _arguments.push_back(Argument("B", name, verbPosition));
    return true;
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    if((* it)->createGoldArgument(verbPosition, name, 
				  leftPosition, rightPosition))
      return true;
  }

  return false;
}
