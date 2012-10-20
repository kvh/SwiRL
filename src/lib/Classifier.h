
#ifndef SRL_CLASSIFIER_H
#define SRL_CLASSIFIER_H

#include <vector>
#include <string>

namespace srl {

class Classifier {
 public:
  Classifier(const std::string & n) : _name(n) {}

  const std::string & getName() const { return _name; }

  virtual double classify(const std::vector<int> & features) { return 0.0; }

  virtual bool initialize(const char * modelFileName) { return false; }

  virtual bool isInitialized() const { return false; }

  virtual ~Classifier() {};

 protected:
  std::string _name;
};

}

#endif
