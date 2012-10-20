/**
 * @file Edge.h
 * Declaration of the Edge class
 * The Edge class is the building block of the parse tree
 */

#include <list>

#include "RCIPtr.h"

#ifndef EDGE_H
#define EDGE_H

namespace srl {

class Edge : public RCObject 
{
 public:

  Edge() { };//std::cout << "Edge::constr\n"; }
  ~Edge() { };//std::cout << "Edge::destr\n"; }
      
 private:

  /** 
   * Type of this edge: 
   */
  char _type;
  
  /**
   * List of children of this edge
   */
  std::list< RCIPtr<Edge> > _children;
};

typedef RCIPtr<Edge> EdgePtr;

} // end namespace srl

#endif // EDGE_H
