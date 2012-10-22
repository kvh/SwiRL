
#include <stdexcept>
#include <sstream>
#include <assert.h>

#ifndef ASSERT_H
#define ASSERT_H

/** Runtime assert */
#define RASSERT(condition, message)\
  EXCEPTION_ASSERT(std::runtime_error, condition, message)

/** Logic assert */
#define LASSERT(condition, message)\
  EXCEPTION_ASSERT(std::logic_error, condition, message)

/** Runtime assert, verbose */
#define RVASSERT(condition, message)\
  VERBOSE_EXCEPTION_ASSERT(std::runtime_error, condition, message)

/** Logic assert, verbose */
#define LVASSERT(condition, message)\
  VERBOSE_EXCEPTION_ASSERT(std::logic_error, condition, message)

/** Throws an exception if condition is false */
#define EXCEPTION_ASSERT(type, condition, message){\
  if((condition) == false){\
    std::ostringstream buffer;\
    buffer << "ERROR in file " << __FILE__ << " line " << __LINE__ << ": " << message;\
    throw type(buffer.str());\
  }\
}\

/** Throws an exception if condition is false */
#define VERBOSE_EXCEPTION_ASSERT(type, condition, message){\
  if((condition) == false){\
    std::ostringstream buffer;\
    buffer << "ERROR in file " << __FILE__ << " line " << __LINE__ << ": " << message;\
    std::cerr << buffer.str() << std::endl;\
    throw type(buffer.str());\
  }\
}\

/** Plain old assert */
#define ASSERT(condition, message) assert(condition)

#endif
