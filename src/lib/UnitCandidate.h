
#ifndef UNIT_CANDIDATE_H
#define UNIT_CANDIDATE_H

#include "Tree.h"
#include "ClassifiedArg.h"
#include "Score.h"

namespace srl {
class UnitCandidate
{
 public:
  /** Label assignments for all candidates */
  std::vector<ClassifiedArg> mSequence; 

  /** log_prob of the whole sequence */ 
  double mProb; 

  /** Sum of probs for all non-O arguments */
  double mArgProbSum;
  int mArgCount;

  /** The oracle score of a frame */
  Score mScore;

  UnitCandidate() {
    reset();
  }

  void reset(){
    mProb = 0.0;
    
    mArgProbSum = 0;
    mArgCount = 0;
  }

  UnitCandidate(const ClassifiedArg & first);

  void addArg(const ClassifiedArg & arg);

  UnitCandidate * makeCand(const ClassifiedArg & last);

  size_t size() { return mSequence.size(); }
  ClassifiedArg & get(int i) { return mSequence[i]; }

  void computeScore(const std::vector<ClassifiedArg> & goldArgs);

}; // UnitCandidate

} // namespace srl

#endif
