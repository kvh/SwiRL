
#ifndef ADA_BOOST_SRL_CLASSIFIER_H
#define ADA_BOOST_SRL_CLASSIFIER_H

#include "Classifier.h"
#include "bAdaBoost.h"

namespace srl {

class AdaBoostClassifier: public Classifier {
 public:
  AdaBoostClassifier(const std::string & n) : 
    Classifier(n), _classifier(NULL) {}

  virtual double classify(const std::vector<int> & features);

  virtual bool initialize(const char * modelFileName);

  virtual bool isInitialized() const { return (_classifier != NULL); }

  virtual ~AdaBoostClassifier() {};

 private:
  bAdaBoost * _classifier;
};

}

#endif

