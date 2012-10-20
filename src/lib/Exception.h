
#include <string>

#ifndef EXCEPTION_H
#define EXCEPTION_H

namespace srl {

class Exception
{
 public:

  Exception(const std::string & msg) : _message(msg) {};

  Exception(const std::string & msg, int line);

  const std::string & getMessage() const { return _message; };

 private:
  
  std::string _message;
};

} // end namespace srl

#endif
