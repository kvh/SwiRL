/******************************************************************************/
/*                                                                            */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                   */
/*  Date      : October 2002                                                  */
/*                                                                            */
/*  example.cc                                                                */
/*                                                                            */
/******************************************************************************/

#include "example.h"
#include <cstdlib>
#include <iostream>



/******************************************************************************/
/*                                                                            */
/*  Class bExample                                                            */
/*                                                                            */
/******************************************************************************/


void bExample::print(ostream& o) const {
  o << "class=" << sign() << "; ";
  fvinput::print(o);
}


/******************************************************************************/
/*                                                                            */
/*  Class bOutput                                                             */
/*                                                                            */
/******************************************************************************/

int bOutput::positive_label = 1;

void bOutput::set_positive_label(int i) {
  bOutput::positive_label = i;
}

bOutput::bOutput() {
  cl = false;
  _weight = 0.0;
}

bOutput::bOutput(int c) {
  cl = (c == bOutput::positive_label);
  _weight = 0.0;
}



/******************************************************************************/
/*                                                                            */
/*  Class mlExample                                                            */
/*                                                                            */
/******************************************************************************/


void mlExample::print(ostream& o) const {
  o << "[ ";
  int l;
  for (l=0; l<_nlabels; l++) {
    o << l << "=" << sign(l) << " ";
  }
  o << " ] ";
  fvinput::print(o);
}



/******************************************************************************/
/*                                                                            */
/*  Class mlOutput                                                             */
/*                                                                            */
/******************************************************************************/

int mlOutput::_nlabels = 1;
char mlOutput::_separator = ':';

void mlOutput::set_separator(char c) {
  mlOutput::_separator = c;
}

void mlOutput::set_nlabels(int n) {
  mlOutput::_nlabels = n;
}


mlOutput::mlOutput() {
  ml = new label[_nlabels];
}

mlOutput::mlOutput(string sl) {
  ml = new label[_nlabels];
  parse_string(sl);
}

mlOutput::~mlOutput() {
  delete [] ml;
}


// labels joined by separator
void mlOutput::parse_string(string in) {
  string lstr;
  string::size_type b, e;
  int l;

  //cout << "mloutput: parsing " << in << endl;
  b = in.find_first_not_of(_separator, 0);
  while (b != string::npos) {
    e = in.find_first_of(_separator, b);
    lstr = in.substr(b,e-b);
    l = atoi(lstr.c_str());
    ml[l].b = true;
    //cout << "Found " << l << endl;
    b = in.find_first_not_of(_separator, e);
  }
  //cout << "mloutput: returning " <<  endl;
}















