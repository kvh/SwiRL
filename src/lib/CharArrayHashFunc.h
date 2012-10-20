
#ifndef CHAR_ARRAY_HASH_FUNC_H
#define CHAR_ARRAY_HASH_FUNC_H

#include <cstdio>

#include "Wide.h"

namespace srl {

typedef struct CharArrayHashFunc
{
  size_t operator()(const Char * s) const;
} CharArrayHashFunc;

} // end namespace srl

#endif
