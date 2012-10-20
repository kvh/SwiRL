
#ifndef SRL_LOGGER_H
#define SRL_LOGGER_H

#include <iostream>
#include <fstream>

namespace srl {
  class Logger {
  public:
    static void setVerbosity(int level) { _level = level; }
    static int getVerbosity() { return _level; }

    static std::ostream & logStream();
    static void setLogFile(const std::string & file);

  private:
    static int _level;

    static std::ofstream _stream;
    static bool _useFileStream;
  };
}

#define LOGD if(Logger::getVerbosity() > 2) Logger::logStream()
#define LOGH if(Logger::getVerbosity() > 1) Logger::logStream()
#define LOGM if(Logger::getVerbosity() > 0) Logger::logStream()

#endif
