
#include <list>
#include <string>

#include "RCIPtr.h"

#ifdef LANG_ENGLISH
#include "HeadEnglish.h"
#endif

#ifndef EDGE_HEAD_H
#define EDGE_HEAD_H

namespace srl {

  /**
   * This function implements the head finding heuristics
   * The type T can be any edge type that implements the following methods:
   *   const std::wstring & T::getLabel() const
   */
  template <typename T>
    srl::RCIPtr<T> 
    findHead(const String & parent,
	     const std::list< srl::RCIPtr<T> > & children) {
#ifdef LANG_ENGLISH
    return findHeadEnglish<T>(parent, children);
#else
#error Head finding heuristics not implemented for this language!
#endif
  }

} // end namespace srl

#endif // EDGE_HEAD_H
