
#include <cstring>
#include <stdlib.h>

#include "Parameters.h"

using namespace std;
using namespace srl;

srl::StringMap<String> Parameters::_parameters;

#define MAX_PAR_LINE (16 * 1024)

/** Returns the position of the first non-space character */
static int skipSpaces(const Char * line)
{
  int i = 0;
  for(; line[i] != '\0'; i ++){
    if(! isspace(line[i])){
      break;
    }
  }
  return i;
}

void Parameters::read(const std::string & file,
		      bool overwrite)
{
  IFStream is(file.c_str());

  if(! is){
    CERR << "Failed to open file: " << file << endl;
    throw(false);
  }

  Char line[MAX_PAR_LINE];
  while(is.getline(line, MAX_PAR_LINE)){
    int start = skipSpaces(line);

    //
    // this is a blank line
    //
    if(line[start] == W('\0')){
      continue;
    }

    //
    // this is a comment line
    //
    if(line[start] == W('#')){
      continue;
    }

    //
    // read the current param
    //
    String name, value;
    if(readNameValue(line + start, name, value, false) == false){
      CERR << "Failed to parse argument \"" << line + start << "\"" << endl;
      throw(false);
    }

    //
    // add the param to hash
    //
    if(overwrite == true || contains(name) == false){
      set(name, value);
    }
  }
}

int Parameters::read(int argc, 
		      char ** argv)
{
  int current = 1;

  for(; current < argc; current ++){
    int length = strlen(argv[current]);

    //
    // found something that looks like a parameter:
    // --name=value
    //
    if(length > 2 && strncmp(argv[current], "--", 2) == 0){
      String arg;
      for(int i = 2; i < length; i ++){
	arg.append(1, (Char) argv[current][i]);
      }

      String name, value;
      if(readNameValue(arg, name, value, true) == false){
	CERR << "Failed to parse argument \"" << arg << "\"" << endl;
	throw(false);
      }

      set(name, value);
    } else{
      // found something that is not a param
      break;
    }
  }

  //
  // if a parameter with the name "config" exists, read additional parameters
  // from the file indicated by the parameter value
  //
  String configValue;
  if(get(W("config"), configValue, false) == false){
    return current;
  }

  // convert the name to regular chars
  string file;
  for(unsigned int i = 0; i < configValue.size(); i ++){
    file.append(1, (char) configValue[i]);
  }

  // read from file. throw exception if anything fails
  read(file, false);

  return current;
}

bool Parameters::readNameValue(const String & arg, 
			       String & name,
			       String & value,
			       bool defaultValue)
{
  static String sep = " \t=";

  int nameStart = arg.find_first_not_of(sep, 0);
  if(nameStart < 0) return false;

  int nameEnd = arg.find_first_of(sep, nameStart);
  if(nameEnd < 0) nameEnd = arg.size();

  name = arg.substr(nameStart, nameEnd - nameStart);

  if(nameEnd < (int) arg.size()){
    int valueStart = arg.find_first_not_of(sep, nameEnd);
    if(valueStart < 0) return false;

    String raw;

    if(arg[valueStart] == '\"'){
      int valueEnd = findQuote(arg, valueStart + 1);
      raw = arg.substr(valueStart + 1, valueEnd - valueStart - 1);
    } else{
      int valueEnd = arg.find_first_of(sep, valueStart);
      if(valueEnd < 0) valueEnd = arg.size();
      raw = arg.substr(valueStart, valueEnd - valueStart);
    }

    if(substitute(raw, value) == false){
      return false;
    }
  } else{
    if(defaultValue == true){
      // Assume a default value of "1"
      value = W("1");
    } else{
      return false;
    }
  }

  return true;
}

int Parameters::findQuote(const String & input,
			  int offset)
{
  for(int i = offset; i < (int) input.size(); i ++){
    if(input[i] == '\"' &&
       (i == 0 || input[i - 1] != '\\')){
      return i;
    }
  }
  return input.size();
}

bool Parameters::substitute(const String & raw,
			    String & value)
{
  OStringStream out;
  int end = -1;

  for(unsigned int i = 0; i < raw.size(); i ++){
    
    //
    // found a complete variable: ${...}
    //
    if(i < raw.size() - 3 && raw[i] == W('$') && raw[i + 1] == W('{') &&
       (end = raw.find_first_of(W('}'), i + 2)) > 0){
      
      String varName = raw.substr(i + 2, end - i - 2);
      String varValue;

      // fetch the variable value
      if(get(varName, varValue, true) == false){
	CERR << "Undefined parameter: " << varName << endl;
	return false;
      }

      // add this value to stream
      out << varValue;

      i = end;
    } 
    // found a special character preceded by backslash
    else if(raw[i] == '\\' && i < raw.size() - 1){
      out << raw[i + 1];
      i ++;
    } else{
      out << raw[i];
    }
  }

  value = out.str();
  return true;
}

bool Parameters::get(const String & name,
		     String & value,
		     bool useEnvironment)
{
  if(_parameters.get(name.c_str(), value) == true){
    return true;
  }

  if(useEnvironment == false){
    return false;
  }

  // convert the name to regular chars
  string n;
  for(unsigned int i = 0; i < name.size(); i ++){
    n.append(1, (char) name[i]);
  }

  char * env = getenv(n.c_str());

  if(env == NULL){
    return false;
  }

  // convert the value to String
  value.clear();
  int length = strlen(env);
  for(int i = 0; i < length; i ++){
    value.append(1, (Char) env[i]);
  }

  return true;
}

#ifdef USE_UNICODE

static Parameters::get(const String & name,
		       std::string & value,
		       bool useEnvironment)
{
#error Convert from String to string here!
}

#endif

bool Parameters::get(const String & name,
		     double & value,
		     bool useEnvironment)
{
  std::string str;
  if(get(name, str, useEnvironment) == false){
    return false;
  }

  value = strtod(str.c_str(), NULL);
  return true;
}

bool Parameters::get(const String & name,
		     int & value,
		     bool useEnvironment)
{
  std::string str;
  if(get(name, str, useEnvironment) == false){
    return false;
  }

  value = strtol(str.c_str(), NULL, 10);
  return true;
}

void Parameters::display(OStream & os)
{
  for(StringMap<String>::const_iterator it = _parameters.begin();
      it != _parameters.end(); it ++){
    os << (* it).second->getKey() << " = " 
       << (* it).second->getValue() << endl;
  }
}

/*
int main()
{
  Parameters::read("PARAMS");
  Parameters::display(cerr);
}
*/
