
#ifndef CLASSRULE_H
#define CLASSRULE_H

#include "FullHist.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "AnswerTree.h"
#include "Bst.h"

#define MCALCRULES 10

class ClassRule
{
 public:
  ClassRule(int dd, int mm, int rr, int tt)
    : d_(dd), m_(mm), t_(tt), rel_(rr)  {}
  ClassRule(const ClassRule& cr)
    : d_(cr.d()), m_(cr.m()), t_(cr.t()), rel_(cr.rel()) {}
  Val* apply(FullHist* treeh);
  static void readCRules(ECString str);
  static vector<ClassRule>& getCRules(FullHist* treeh, int wh);
  friend ostream& operator<<(ostream& os, const ClassRule& cr)
    {
      os << "{"<< cr.d() << "," << cr.m() << "," << cr.rel() << "," << cr.t() << "}";
      return os;
    }
  int d() const { return d_; }
  int m() const { return m_; }
  int t() const { return t_; }
  int rel() const { return rel_; }
 private:
  int d_;
  int m_;
  int t_;
  int rel_;
  static vector<ClassRule>  rBundles2_[MAXNUMNTTS][MAXNUMNTS];
  static vector<ClassRule>  rBundles3_[MAXNUMNTTS][MAXNUMNTS];
  static vector<ClassRule>  rBundlesm_[MAXNUMNTTS][MAXNUMNTS];
};
    
typedef vector<ClassRule> CRuleBundle;

#endif /* ! CLASSRULE_H */
