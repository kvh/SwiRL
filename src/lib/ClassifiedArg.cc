
#include "ClassifiedArg.h"
#include "Tree.h"

using namespace srl;

bool ClassifiedArg::operator == (const ClassifiedArg & arg) const 
{
  if(label == arg.label &&
     phrase->getLeftPosition() == arg.phrase->getLeftPosition() &&
     phrase->getRightPosition() == arg.phrase->getRightPosition())
    return true;
  return false;
}
