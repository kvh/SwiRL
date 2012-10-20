/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                     */
/*  Date      : October 2002                                                    */
/*                                                                              */
/*  classifier.h : interface to classifiers                                     */
/*                                                                              */
/********************************************************************************/


#ifndef __classifier__
#define __classifier__

#include "input.h"

class classifier {
 public:

  virtual double classify(input*) const = 0;
  virtual ~classifier() {}

};

class ensemble: public classifier {
 public:
  virtual double classify(input*) const = 0;

  virtual int size() const  = 0;

  virtual void begin_ensemble_classification() = 0;
  virtual bool end_ensemble_classification() = 0;
  virtual double classify(input* example, double current_pr, int steps) = 0; 
  virtual ~ensemble() {}  
};

#endif

