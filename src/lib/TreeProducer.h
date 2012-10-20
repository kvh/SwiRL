
#ifndef SRL_TREE_PRODUCER_H
#define SRL_TREE_PRODUCER_H

#include <iostream>

#include "Tree.h"
#include "RCIPtr.h"

namespace srl {

class TreeProducer : public RCObject {

 public:

  virtual srl::TreePtr create(IStream & is,
				std::vector<srl::TreePtr> & sentence,
				bool caseSensitive) = 0;

  /**
   * Reads a phrase from the stream WITHOUT any additional operations 
   * Note: create is similar but performs a number of additional ops
   */
  virtual srl::TreePtr read(IStream & is) = 0;

  virtual bool gatherStats(IStream & treeStream)= 0;

  virtual srl::TreePtr analyze(IStream & treeStream) = 0;
};

}

#endif
