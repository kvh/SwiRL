#ifndef EXTPOS_H
#define EXTPOS_H
#include "Term.h"
#include <vector>
#include <iostream>
#include "SentRep.h"
class ExtPos: public vector<vector <const Term*> >
{
 public:
  void read(ifstream* ifs,SentRep& sr);
  bool hasExtPos();
};

#endif
