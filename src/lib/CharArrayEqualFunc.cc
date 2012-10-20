
#include "CharArrayEqualFunc.h"
#include "CharUtils.h"

using namespace srl;

bool CharArrayEqualFunc::operator()(const Char * s1, 
				    const Char * s2) const
{
  return compareWideCharToWideChar(s1, s2);
}
