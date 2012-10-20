/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                     */
/*  Date      : October 2002                                                    */
/*                                                                              */
/*  example.h : defines classes for examples, including                         */
/*             - bOutput, mlOutput                                              */
/*             - example                                                        */
/*                                                                              */
/********************************************************************************/

#ifndef __example__
#define __example__

#include <string>
#include <map>
#include <list>

#include "fvinput.h"

using namespace std;


/********************************************************************************/
/*                                                                              */
/*  bOutput:  binary (zero (-) / non-zero (+)) output                           */
/*                                                                              */
/********************************************************************************/

class bOutput {
 private:
  bool cl;
  double _weight;
  double _prediction;
  static int positive_label;

 public:
  // default constructor: negative class
  bOutput();
  bOutput(int);
  //  bool get_class() const { return cl; }
  bool positive() const { return cl; }
  int sign() const { return cl ? +1 : -1; }

  double weight() const { return _weight; }
  void set_weight(double w) { _weight = w; }

  double prediction() const { return _prediction; }
  void set_prediction(double p) { _prediction = p; }

  static void set_positive_label(int);
}; 


/********************************************************************************/
/*                                                                              */
/*  bExample:  example with binary class                                        */
/*                                                                              */
/********************************************************************************/

class bExample: public bOutput, public fvinput {
   
public: 
  bExample(int cl, string f)
    : bOutput(cl), fvinput(f)
  {}

  void print(ostream& o) const;

  typedef fvinput::const_iterator feature_iterator;
};


/********************************************************************************/
/*                                                                              */
/*  mlOutput:  a multilabel output                                              */
/*                                                                              */
/********************************************************************************/

class mlOutput {
 private:

  struct label {
    bool b;
    double weight;
    double prediction;

    label():
      b(false), weight(0.0), prediction(0.0)
    { }
  };
  
  // array of labels
  label* ml;
  

  static char _separator;
  void parse_string(string);

 protected: 
  static int _nlabels;

 public:
  static void set_separator(char);
  static void set_nlabels(int);
  static int nlabels();

  mlOutput();
  // labels joined by separator
  mlOutput(string);
  ~mlOutput();
  
  void set_label(int l, bool b) { ml[l].b = b; }
  bool belongs_to(int l) const { return ml[l].b; }
  int sign(int l) const { return ml[l].b ? +1 : -1; }

  void set_weight(int l, double w) { ml[l].weight = w; }
  double weight(int l) const { return ml[l].weight; }
  
  void set_prediction(int l, double pr) { ml[l].prediction = pr; }
  double prediction(int l) { return ml[l].prediction; }
  

};



/********************************************************************************/
/*                                                                              */
/*  mlExample:  multilabelled example                                           */
/*                                                                              */
/********************************************************************************/

class mlExample: public mlOutput, public fvinput {
   
public: 
  mlExample(string ml, string f)
    : mlOutput(ml), fvinput(f)
  {}

  void print(ostream& o) const;

  typedef fvinput::const_iterator feature_iterator;
};



#endif
