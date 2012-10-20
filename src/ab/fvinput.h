/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                     */
/*  Date      : October 2002                                                    */
/*                                                                              */
/*  fvinput.h   : defines classes for feature-value vectors                         */
/*             - fvinput                                                            */
/*                                                                              */
/********************************************************************************/

#ifndef __fvinput__
#define __fvinput__

#include <string>
#include <map>
#include <set>
#include <list>

using namespace std;



/********************************************************************************/
/*                                                                              */
/*  fvinput:  feature-value input                                                   */
/*                                                                              */
/********************************************************************************/

class fvinput {
 private:
  map<int,double> mf;
  int _dimension;

  static bool  consider_value;
  static char label_value_separator;

 public:

  // static functions
  static void set_consider_value (bool);
  static void set_label_value_separator (char);
  
  // constructors & destructor
  fvinput()
    : _dimension(0)
    {}

  // string format: <l1>:<v1> <l2>:<v2> <ldim>:<vdim>
  fvinput(string);

  // new fvinput is f1*i1 + f2*i2
  fvinput(double f1, const fvinput& i1, double f2, const fvinput& i2);

  ~fvinput();

  // update functions
  void add_feature(int l, double v = 1.0);
  void increment_feature(int l, double v = 1.0);

  // string format: <l1>:<v1> <l2>:<v2> <ldim>:<vdim>
  void parse_string(string);

  // consultors
  double feature_value(int label) const;
  int    size() const { return mf.size(); }
  int    dimension() const { return _dimension; }

  // calcul
  double inner_product(fvinput*) const;

  // recorregut de features
  class const_iterator {
    private :
      map<int,double>::const_iterator i;

    public:
    const_iterator() 
      {};
    const_iterator(map<int,double>::const_iterator i0) 
      : i(i0)
      {}

    //    const iFeature* operator->() const { return &(i->second); }
    const int label() const { return i->first; }
    const double value() const { return i->second; }
    
    const_iterator& operator++() { ++i; return *this; }
    
    bool operator==(const const_iterator& rhs) { return i == rhs.i; }
    bool operator!=(const const_iterator& rhs) { return i != rhs.i; }
    
  };

  const_iterator begin() const { const_iterator i(mf.begin()); return i; }
  const_iterator end() const { const_iterator i(mf.end()); return i; }
  

  void print(ostream& o) const;
  void write(ostream& o) const;
};





#endif














