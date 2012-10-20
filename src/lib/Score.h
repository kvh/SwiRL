
#ifndef SCORE_H
#define SCORE_H

namespace srl {
class Score
{
 public:
  int mTotal;
  int mPredicted;
  int mCorrect;

  Score() {
    mTotal = 0;
    mPredicted = 0;
    mCorrect = 0;
  }

  double P() const {
    if(mCorrect == 0 || mPredicted == 0) return 0.0;
    return ((double) mCorrect) / ((double) mPredicted);
  }

  double R() const {
    if(mCorrect == 0 || mTotal == 0) return 0.0;
    return ((double) mCorrect) / ((double) mTotal);
  }

  double F2() const { return F(2.0); }
  double F1() const { return F(1.0); }

  double F(double alpha) const {
    double p = P();
    double r = R();
    if(p == 0.0 || r == 0.0) return 0.0;
    return ((1.0 + alpha) * p * r) / ((alpha * p) + r);
  }

}; // end Score

} // end namespace srl

#endif
