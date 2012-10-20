#include <sstream>

#include "Exception.h"

using namespace std;
using namespace srl;

Exception::Exception(const std::string & msg, int line)
{
  ostringstream buffer;
  buffer << "[Line " << line << "] " << msg << ends;
  _message = buffer.str();
}
