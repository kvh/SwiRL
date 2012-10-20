
#ifndef SCORET_H
#define SCORET_H

#include "InputTree.h"

class ScoreTree
{
 public:
  int equivInt(int x);
  int puncEquiv(int i, vector<ECString>& sr);
  void setEquivInts(vector<ECString>& sr);
  bool lexact2(InputTree* tree);
  void recordGold(InputTree* tree, ParseStats& parseStats);
  void precisionRecall( InputTree* tree, ParseStats& parseStats );
  bool scorePunctuation( const ECString trmString );
  multiset<int> trips;
  int equivPos[401];

};

#endif /* ! SCORET_H */


