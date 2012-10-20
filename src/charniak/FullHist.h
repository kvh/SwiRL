
#ifndef FULLHIST_H
#define FULLHIST_H

#include "Edge.h"
#include <list>
#include "Wrd.h"
#include <iostream>
#include <fstream>
#include "ECString.h"

class FullHist;
typedef list<FullHist*>::iterator FullHistIter;

class
FullHist
{
public:
  FullHist() : e(NULL), hd(NULL), back(NULL), cpos(0) {}
  FullHist(Edge* e1) : e(e1), back(NULL), cpos(0) {}
  FullHist(int tint, FullHist* fh)
    : pos(-1), term(tint), hd(NULL), back(fh), cpos(0) {}
  FullHist(int tint, FullHist* fh, Item* i)
    : pos(-1), term(tint), itm(i), hd(NULL), back(fh), cpos(0) {}
  FullHist* extendByEdge(Edge* e1);
  FullHist* extendBySubConstit();
  FullHist* retractByEdge();
  FullHist* nth(int n)
    {
      if(n < 0 || n >= size) return NULL;
      else return fharray[n];
    }
  friend ostream& operator<<(ostream& os, const FullHist& fh);
 int hpos;
  int pos;
  int term;
  int preTerm;
  Edge* e;
  Item* itm;
  const Wrd* hd;
  FullHist* back;
  FullHist* fharray[64];
  int cpos;
  int size;
};

#endif
