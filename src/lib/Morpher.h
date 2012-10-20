
#ifndef MORPHER_H
#define MORPHER_H

#include "Wide.h"
#include "RCIPtr.h"

namespace srl {

class Morpher : public RCObject {

 public:

  virtual String morph(const String & word,
		       const String & label) = 0;

};

}

#endif
