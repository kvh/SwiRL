/**
 * This stores info about one classified argument
 * Used only during classification (not training)
 */

#ifndef CLASSIFIED_ARG_H
#define CLASSIFIED_ARG_H

#include <limits.h>
#include <float.h>
#include "Wide.h"

namespace srl {

class Tree;

class ClassifiedArg
{
 public:
  Tree * phrase;
  String label;
  double conf;
  double prob;

  ClassifiedArg() {
    phrase = NULL;
    conf = DBL_MIN;
    prob = 0.0;
  }

  ClassifiedArg(Tree * p,
		const String & l,
		double c) {
    phrase = p;
    label = l;
    conf = c;
    prob = 0.0;
  }

  ClassifiedArg(const ClassifiedArg & other) {
    construct(other);
  }

  ClassifiedArg & operator = (const ClassifiedArg & other) {
    if(& other != this) construct(other);
    return * this;
  }

  bool operator == (const ClassifiedArg & arg) const;

 private:
  void construct(const ClassifiedArg & other) {
    phrase = other.phrase;
    label = other.label;
    conf = other.conf;
    prob = other.prob;
  }
};

}

#endif
