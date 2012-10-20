/******************************************************************************/
/*                                                                            */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                   */
/*  Date      : October 2002                                                  */
/*                                                                            */
/*                                                                            */
/*  fvinput.cc                                                                */
/*                                                                            */
/******************************************************************************/

#include "fvinput.h"
#include <cstdlib>
#include <iostream>
#include <cmath>

/******************************************************************************/
/*                                                                            */
/*  fvinput:                                                                  */
/*                                                                            */
/******************************************************************************/


/*----------------------------------------------------------------------------*\
 *      Static stuff                                                          * 
\*----------------------------------------------------------------------------*/

bool fvinput::consider_value = true;
char fvinput::label_value_separator = ':';

void fvinput::set_consider_value(bool v) {
  fvinput::consider_value = v;
}

void fvinput::set_label_value_separator(char s) {
  fvinput::label_value_separator = s;
}


/*----------------------------------------------------------------------------*\
 *      Constructors                                                          * 
\*----------------------------------------------------------------------------*/

fvinput::fvinput(string line) 
  : _dimension(0)
{
  parse_string(line);
}


void fvinput::parse_string(string line) {
  string feat, label, value;
  string::size_type b, e, k;
  double v; 

  b = line.find_first_not_of(" ", 0);
  while (b != string::npos) {
    e = line.find_first_of(" ", b);
    feat = line.substr(b,e-b);
    if (fvinput::consider_value) {
      k = feat.find_last_of(fvinput::label_value_separator);
      label = feat.substr(0,k);
      if (k != string::npos) {
	k++;
	value = feat.substr(k);
	v = atof(value.c_str());
      }
      else {
	v = 0.0;
      }
    }
    else {
      label = feat; 
      v = 1.0;
    }
    add_feature(atoi(label.c_str()), v);
    b = line.find_first_not_of(" ", e);
  }
}


fvinput::fvinput(double f1, const fvinput& i1, double f2, const fvinput& i2) {

  _dimension = 0;

  fvinput::const_iterator it1 = i1.begin();
  fvinput::const_iterator it2 = i2.begin();
  
  while ((it1 != i1.end()) && (it2 != i2.end())) {
    if (it1.label() == it2.label()) {
      add_feature(it1.label(), f1*it1.value() + f2*it2.value());
      ++it1;
      ++it2;
    }
    else if (it1.label() < it2.label()) {
      add_feature(it1.label(), f1*it1.value());
      ++it1;
    }
    else {
      add_feature(it2.label(), f2*it2.value());
      ++it2;
    }
  }
  while (it1 != i1.end()) {
    add_feature(it1.label(), f1*it1.value());
    ++it1;
  }
  while (it2 != i2.end()) {
    add_feature(it2.label(), f2*it2.value());
    ++it2;
  }
}

fvinput::~fvinput() {
  //  map<int,iFeature>::iterator i;
  // std::cerr << "fvi destroyed\n";
  //for (i = mf.begin(); i != mf.end(); ++i) {
  //  iFeature f = i->second;
  //  delete f;
  //}
}


/*----------------------------------------------------------------------------*\
 *      Update                                                                * 
\*----------------------------------------------------------------------------*/

void fvinput::add_feature(int l, double v) {
  map<int,double>::value_type vm(l, v);
  mf.insert(vm);
  if (_dimension < l) {
    _dimension = l;
  }
}

void fvinput::increment_feature(int l, double v) {
  map<int,double>::iterator i = mf.find(l);
  if (i != mf.end()) {
    i->second += v;
  }
  else {
    add_feature(l,v);
  }
}



/*----------------------------------------------------------------------------*\
 *      access and calculus                                                   * 
\*----------------------------------------------------------------------------*/

double fvinput::feature_value(int label) const {
  map<int,double>::const_iterator i = mf.find(label);
  if (i != mf.end()) {
    return i->second;
  }
  else {
    return 0.0;
  }
}

double fvinput::inner_product(fvinput* i2) const {
  fvinput::const_iterator it1 = begin();
  fvinput::const_iterator it2 = i2->begin();
  double y = 0.0;

  while ((it1 != end()) && (it2 != i2->end())) {
    if (it1.label() == it2.label()) {
      y += it1.value() * it2.value();
      ++it1;
      ++it2;
    }
    else if (it1.label() < it2.label()) {
      ++it1;
    }
    else {
      ++it2;
    }
  }
  return y;
}


/*----------------------------------------------------------------------------*\
 *      i/o operations                                                        * 
\*----------------------------------------------------------------------------*/

void fvinput::print(ostream& o) const {
  fvinput::const_iterator i;
  o << "[ ";
  for (i = begin(); i != end(); ++i) {
    o << i.label() << "=" << i.value() << " ";
  }
  o << "]";
}


void fvinput::write(ostream& o) const {
  fvinput::const_iterator i;
  for (i = begin(); i != end(); ++i) {
    o << i.label() << fvinput::label_value_separator << i.value() << " ";
  }
}


