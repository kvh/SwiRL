
#include "Logger.h"
#include "Assert.h"

using namespace std;
using namespace srl;

int Logger::_level = 0;
std::ofstream Logger::_stream;
bool Logger::_useFileStream = false;

std::ostream & Logger::logStream()
{
  if(_useFileStream) return _stream;
  return cerr;
}

void Logger::setLogFile(const std::string & file)
{
  _stream.open(file.c_str());
  RVASSERT(_stream, "Can not open log file: " << file);
  _useFileStream = true;
}
