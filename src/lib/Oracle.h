
#ifndef ORACLE_H
#define ORACLE_H

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "Tree.h"

namespace srl {
  
  class OracleArg {
  public:
    OracleArg(const std::string & name,
	      int left,
	      int right) {
      mName = name;
      mLeftPosition = left;
      mRightPosition = right;
    }

    std::string mName;

    int mLeftPosition;

    int mRightPosition;
  };

  class OracleFrame {
  public:
    OracleFrame(int vp) { mVerbPosition = vp; }

    void readArguments(std::vector< std::vector<std::string> > & matrix,
		       int column);

    int mVerbPosition;

    std::list<OracleArg> mArgs;
  };

  class OracleSentence {
  public:
    void setGoldArguments(Tree * tree);

    static bool readSentences(std::istream & is, 
			      std::vector<OracleSentence *> & sentences);
    
    static OracleSentence * readSentence(std::istream & is);

    std::vector<OracleFrame> mFrames;
  };

} // namespace srl

#endif
