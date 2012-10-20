
#include <iostream>
#include <sstream>
#include <cstdlib>

#include "Argument.h"
#include "Exception.h"

using namespace std;
using namespace srl;

Argument::Argument(const String & s)
{
  String buffer = s;

  // XXX: Jordi bug
  // robustness treatment for malformed arg labels, e.g. "B-A0-15-"
  if(s.size() > 0 && s[s.size() - 1] == '-'){
    cerr << "Found weird arg label: " << s << endl;
    buffer = s.substr(0, s.size() - 1);
  }

  if(buffer[0] == 'B'){
    _type = "B";
  } else if(buffer[0] == 'I'){
    _type = "I";
  } else {
    throw(srl::Exception("Unknown argument: " + buffer));
  }

  // the name starts at position 2
  int end = buffer.find_last_of("-");
  if(end <= 2){
    throw(srl::Exception("Malformed argument: " + buffer));
  }

  _name = buffer.substr(2, end - 2);
  
  String tmp = buffer.substr(end + 1, buffer.size() - end - 1);
  _verbPosition = strtol(tmp.c_str(), NULL, 10);

  _begin = NULL;
  _bType = B_SINGLE;
  _prob = 0;

  // cerr << "Arg: "; display(cerr); cerr << endl;
}

void Argument::display(OStream & os, 
		       const String & prefix,
		       const String & suffix) const
{
  os << prefix << _type << "-" << _name << "-" << _verbPosition << suffix;
}

String Argument::getFullName() const
{
  ostringstream os;
  os << _type << "-" << _name << "-" << _verbPosition;
  return os.str();
}

String Argument::getTypeName() const
{
  ostringstream os;
  os << _type << "-" << _name;
  return os.str();
}

bool Argument::operator == (const Argument & other) const
{
  if(_type == other._type &&
     _name == other._name &&
     _verbPosition == other._verbPosition){
    return true;
  }

  return false;
}

bool Argument::isClean() const
{
  // Previously we trained only for single B args
  if(getBType() == B_SINGLE || 
     getBType() == B_FULL_TERM ||
     getBType() == B_INC_TERM) return true;

  // Now we train for singles and B-I sequences with same parent
  // if(getBType() != B_UGLY) return true;
  return false;
}
