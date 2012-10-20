/**
 * Provides a generic interface towards the Charniak parser
 */

#include <vector>
#include "Tree.h"

class ParserApi 
{
public:
  /** Initialize the parser */
  static bool initialize(const char * dataDirectory,
			 bool caseInsensitive);
  
  /** 
   * Rank parses one sentence 
   * The sentence is a sequence of tokens separated by space
   */
  static bool parse(const char * sentence,
		    int howManySolutions,
		    std::vector<srl::TreePtr> & outputTrees,
		    std::vector<double> & probabilities);

  /** Parses one sentence and returns the best solution */
  static srl::TreePtr parse(const char * sentence);

 private:
  static int MAX_SENT_LEN;
};
