
#ifndef BANK_TREE_PRODUCER_H
#define BANK_TREE_PRODUCER_H

#include <vector>

#include "TreeProducer.h"
#include "RCIPtr.h"

namespace srl {

class BankTreeProducer: public srl::TreeProducer {

 public:

  BankTreeProducer();

  virtual srl::TreePtr create(IStream & treeStream, 
				std::vector<srl::TreePtr> & sentence,
				bool caseSensitive);

  virtual srl::TreePtr read(IStream & is);

  virtual bool gatherStats(IStream & treeStream);

  virtual srl::TreePtr analyze(IStream & treeStream);

  static void preprocess(srl::TreePtr tree,
			 std::vector<srl::TreePtr> & sentence,
			 bool caseSensitive);

 private:

  static void removeEmptyNodes(srl::TreePtr & edge);

  static bool isEmptyNode(const srl::TreePtr & edge);

  static void generateSentence(const srl::TreePtr & edge,
			       std::vector<srl::TreePtr> & sentence);
};

}

#endif
