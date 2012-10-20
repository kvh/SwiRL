
#ifndef SRL_PARAMETERS_H
#define SRL_PARAMETERS_H

#include <string>

#include "Wide.h"
#include "StringMap.h"

namespace srl {

class Parameters {

 public:

  /** 
   * Reads parameters from a file 
   */
  static void read(const std::string & file, bool overwrite = false);

  /** 
   * Reads parameters from the command line, in the form "--name=value", or
   * in the form "--name", which assumes a default value of 1
   * The "config" parameter has a special meaning: if set it points to 
   * a configuration file containing other settings that are automatically
   * read using the above read method with overwrite set to false
   * @return A positive integer representing the first free position in the
   *         argument array. If some error occured, a Boolean exception 
   *         is thrown out of this method
   */
  static int read(int argc, char ** argv);

  static void display(OStream & os);

  static void set(const String & name,
		  const String & value) {
    _parameters.set(name.c_str(), value);
  }

  /**
   * Fetches the value of a parameter
   * If the parameter is not found in the hash, the environment is inspected
   */
  static bool get(const String & name,
		  String & value,
		  bool useEnvironment = true);

#ifdef USE_UNICODE
  /** Fetches a regular string parameter */
  static bool get(const String & name,
		  std::string & value,
		  bool useEnvironment = true);
#endif

  /** Fetches a double parameter */
  static bool get(const String & name,
		  double & value,
		  bool useEnvironment = true);

  /** Fetches a int parameter */
  static bool get(const String & name,
		  int & value,
		  bool useEnvironment = true);

  static bool contains(const String & name) { 
    return _parameters.contains(name.c_str());
  }

 private:

  static bool readNameValue(const String & arg, 
			    String & name,
			    String & value,
			    bool defaultValue = false);

  static bool substitute(const String & raw,
			 String & value);

  /** finds first quote not preceded by backslash */
  static int findQuote(const String & input,
		       int offset);

  static srl::StringMap<String> _parameters;
};

}

#endif
