
#include "AdaBoostClassifier.h"
#include "Assert.h"

#include "fvinput.h"

using namespace std;
using namespace srl;

double AdaBoostClassifier::classify(const std::vector<int> & features)
{
  RVASSERT(_classifier != NULL, "AdaBoost classifier not initialized!");
  fvinput * fv = new fvinput();
  RVASSERT(fv != NULL, "Failed to create fvinput!");
  
  for(size_t i = 0; i < features.size(); i ++){
    fv->add_feature(features[i]);
  }
  
  double conf = _classifier->classify(fv);
  delete fv;

  return conf;
}

bool AdaBoostClassifier::initialize(const char * modelFileName)
{
  ifstream tests(modelFileName);
  if(! tests) return false;
  tests.close();

  _classifier = new bAdaBoost();
  RVASSERT(_classifier != NULL, "Failed to create AdaBoost classifier!");

  _classifier->read_from_file((char *) modelFileName);
  return true;
}

