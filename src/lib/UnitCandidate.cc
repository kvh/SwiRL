
#include <iomanip>
#include <math.h>
#include <algorithm>

#include "UnitCandidate.h"
#include "Assert.h"
#include "CharUtils.h"
#include "Head.h"
#include "Lexicon.h"
#include "Logger.h"

using namespace std;
using namespace srl;

UnitCandidate::UnitCandidate(const ClassifiedArg & first) 
{
  reset();
  mSequence.push_back(first);
  mProb = first.prob;
  
  if(first.label != "O"){
    mArgProbSum += exp(first.prob);
    mArgCount ++;
  }
}

UnitCandidate * UnitCandidate::makeCand(const ClassifiedArg & last) 
{
  UnitCandidate * newCand = new UnitCandidate();
  for(size_t i = 0; i < mSequence.size(); i ++)
    newCand->mSequence.push_back(mSequence[i]);

  newCand->mSequence.push_back(last);
  newCand->mProb = mProb + last.prob;
  
  newCand->mArgCount = mArgCount;
  newCand->mArgProbSum = mArgProbSum;
  if(last.label != "O"){
    newCand->mArgProbSum += exp(last.prob);
    newCand->mArgCount ++;
  }
  
  return newCand;
}

void UnitCandidate::addArg(const ClassifiedArg & arg)
{
  mSequence.push_back(arg);
  mProb += arg.prob;

  if(arg.label != "O"){
    mArgProbSum += exp(arg.prob);
    mArgCount ++;
  }  
}

/**
 * Verifies if this predicted argument matches a GOLD argument 
 */
static bool matchesGold(const ClassifiedArg & arg,
			const vector<ClassifiedArg> & goldArgs)
{
  for(size_t i = 0; i < goldArgs.size(); i ++){
    if(goldArgs[i] == arg) return true;
  }
  return false;
}

/** Computes the score of this frame compared to the GOLD args */
void UnitCandidate::computeScore(const std::vector<ClassifiedArg> & goldArgs)
{
  mScore.mTotal = goldArgs.size();
  for(size_t i = 0; i < mSequence.size(); i ++){
    if(mSequence[i].label != "O"){
      mScore.mPredicted ++;
      if(matchesGold(mSequence[i], goldArgs)) mScore.mCorrect ++;
    }
  }
}
