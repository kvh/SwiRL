
#ifndef CHAR_ARRAY_EQUAL_FUNC_H
#define CHAR_ARRAY_EQUAL_FUNC_H

#include "Wide.h"

namespace srl {

typedef struct CharArrayEqualFunc
{
  bool operator()(const Char * s1, const Char * s2) const;
} CharArrayEqualFunc;

} // end namespace srl

#endif
