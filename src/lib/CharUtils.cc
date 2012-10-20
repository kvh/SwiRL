
#include <list>
#include <stdlib.h>
#include <ctype.h>

#include "CharUtils.h"
#include "Assert.h"

using namespace std;
using namespace srl;

template <typename T1, typename T2>
static bool compareStrings(T1 s1,
			   T2 s2)
{
  int i = 0;

  for(; s1[i] != 0; i ++){
    if(s1[i] != s2[i]){
      return false;
    }
  }

  if(s2[i] != 0){
    return false;
  }

  return true;
}

bool srl::compareWideCharToWideChar(const Char * s1,
				      const Char * s2)
{
  return compareStrings<const Char *, const Char *>(s1, s2);
}

bool srl::startsWith(const String & big,
		       const String & small)
{
  if(small.size() > big.size()) return false;

  for(size_t i = 0; i < small.size(); i ++){
    if(small[i] != big[i]){
      return false;
    }
  }

  return true;
}

bool srl::endsWith(const String & big,
		     const String & small)
{
  size_t bigSize = big.size();
  size_t smallSize = small.size();

  if(smallSize > bigSize) return false;

  for(size_t i = 0; i < smallSize; i ++){
    if(small[i] != big[bigSize - smallSize + i]){
      return false;
    }
  }

  return true;
}

int srl::toInteger(const String & s)
{
#ifdef USE_UNICODE
#error "toInteger not implemented for Unicode"
#else
  return strtol(s.c_str(), NULL, 10);
#endif
}


void srl::simpleTokenize(const String & input,
			   std::vector<String> & output,
			   const String & separators)
{
  for(int start = input.find_first_not_of(separators);
      start < (int) input.size() && start >= 0;
      start = input.find_first_not_of(separators, start)){
    int end = input.find_first_of(separators, start);
    if(end < 0) end = input.size();
    output.push_back(input.substr(start, end - start));
    start = end;
  }
}

/**
 * Replaces all occurences of \" with "
 */
static String normalizeQuotes(const String & str) {
  ostringstream os;
  for(size_t i = 0; i < str.size(); i ++){
    // do not include \ if followed by "
    if(str[i] == '\\' && i < str.size() - 1 && str[i + 1] == '\"'){
      continue;
    } else {
      os << str[i];
    }
  }
  return os.str();
}

void srl::tokenizeWithQuotes(const String & input,
			       std::vector<String> & tokens,
			       const String & separators)
{
  int position = 0;
	
  while((position = input.find_first_not_of(separators, position)) != -1){
    int end = -1;

      // found quoted token (not preceded by \)
      if(input[position] == '\"' &&
	 (position == 0 || input[position - 1] != '\\')){

	// find the first quote not preceded by slash
	int current = position;
	for(;;){
	  // found end of string first
	  if((end = input.find_first_of('\"', current + 1)) == -1){
	    end = input.size();
	    break;
	  } else { // found a quote
	    if(input[end - 1] != '\\'){ // valid quote
	      end ++;
	      break;
	    } else { // quote preceded by slash
	      current = end;
	    }
	  }
	}

	// do not include the quotes in the token
	tokens.push_back(normalizeQuotes(input.substr(position + 1, 
						      end - position - 2)));
      }

      // regular token
      else {
	if((end = input.find_first_of(separators, position + 1)) == -1)
	  end = input.size();

	tokens.push_back(input.substr(position, end - position));
      }
	    
      position = end;
    }
}

/**
 * Constructs a valid quote-surrounded token
 * All inside quotes are preceded by \
 */
String srl::quotify(const String & str) {
  ostringstream buffer;
  buffer << '\"';
  for(size_t i = 0; i < str.size(); i ++){
    if(str[i] == '\"') buffer << '\\';
    buffer << str[i];
  }
  buffer << '\"';
  return buffer.str();
}

bool srl::isNumberRange(const String & s)
{
  if(s.empty() == true){
    return false;
  }

  bool seenDash = false;

  for(unsigned int i = 0; i < s.size(); i ++){
    // digits are ok
    if(isdigit(s[i])){
      continue;
    }

    //
    // dashes are ok if:
    // no dashes have been seen, and
    // this is not the beginning of the stream
    // this is not the end of the stream
    //
    if(s[i] == '-' &&
       seenDash == false &&
       i > 0 && i < s.size() - 1){
      seenDash = true;
      continue;
    }

    // nothing else is fine
    return false;
  }

  return true;
}

bool srl::isNumber(const String & s)
{
  if(s.empty() == true){
    return false;
  }

  bool seenDot = false;
  bool seenDigit = false;
  bool seenSlash = false;

  // a number must start with a digit, +, -, or .
  if(s[0] == '.'){
    seenDot = true;
  }
  else if(! isdigit(s[0]) && 
	  s[0] != '+' && 
	  s[0] != '-'){
    return false;
  } else if(isdigit(s[0])){
    seenDigit = true;
  }

  for(unsigned int i = 1; i < s.size(); i ++){
    // digits are ok
    if(isdigit(s[i])){
      seenDigit = true;
      continue;
    }

    //
    // dots are ok if:
    //   no dots have been previously seen,
    //   previous char is a digit, and
    //   next char is a digit
    //
    if(s[i] == '.' &&
       seenDot == false &&
       isdigit(s[i - 1]) &&
       i < s.size() - 1 &&
       isdigit(s[i + 1])){
      seenDot = true;
      continue;
    }

    //
    // commas are ok if:
    //   no dots have been previously seen,
    //   previous char is a digit, and
    //   next char is a digit
    //
    if(s[i] == ',' &&
       seenDot == false &&
       isdigit(s[i - 1]) &&
       i < s.size() - 1 &&
       isdigit(s[i + 1])){
      continue;
    }

    //
    // "\/" are ok if:
    //   previous char is a digit, and
    //   next char is a digit
    //
    if(s[i] == '\\' && i < s.size() - 1 && s[i + 1] == '/' &&
       isdigit(s[i - 1]) &&
       i < s.size() - 2 &&
       isdigit(s[i + 2])){
      i ++;
      continue;
    }

    //
    // "/" are ok if:
    //   previous char is a digit, and
    //   next char is a digit, and
    //   we have not seen slashes before (otherwise it's a date)
    //
    if(s[i] == '/' && 
       isdigit(s[i - 1]) &&
       i < s.size() - 1 &&
       isdigit(s[i + 1]) &&
       seenSlash == false){
      seenSlash = true;
      continue;
    }

    // nothing else is fine
    return false;
  }

  if(seenDigit == false){
    return false;
  }

  return true;
}

String srl::toUpper(const String & s)
{
  String o = s;

  for(int i = 0; i < (int) o.size(); i ++){
    o[i] = toupper(o[i]);
  }

  return o;
}

String srl::toLower(const String & s)
{
  String o = s;

  for(int i = 0; i < (int) o.size(); i ++){
    o[i] = tolower(o[i]);
  }

  return o;
}

/**
 * Breaks a number into its constituents: sign, left, separator, right
 * right will only be set for reals, fractions, ranges
 */
static bool breakNumber(const String & number,
			String & sign,
			String & left,
			String & separator,
			String & right)
{
  int position = 0;

  if(number[position] == '-' ||
     number[position] == '+'){
    sign = number.substr(0, 1);
    position ++;
  } else{
    sign = "";
  }

  separator = "";
  left = "";
  right = "";

  for(; position < (int) number.size(); position ++){

    // found the separator
    if(separator.empty() == true &&
       (number[position] == '/' ||
	number[position] == '.' ||
	number[position] == '-')){
      separator = number.substr(position, 1);
      continue;
    }

    if(separator.empty() == true &&
       number.substr(position, 2) == "\\/"){
      separator = String("/");
      position ++;
      continue;
    }

    // skip commas
    if(number[position] == ','){
      continue;
    }

    // found a digit
    if(isdigit(number[position])){

      if(separator.empty()){
	left.push_back(number[position]);
      } else{
	right.push_back(number[position]);
      }

      continue;
    }

    // nothing else is Ok
    return false;
  }

  // empty left, not empty right => left = 0
  if(left.empty() && ! right.empty()){
    left = "0";
  }

  return true;
}

/**
 * Expands a 3-digit integer number
 * This function was highly influenced by Edgar Gonzalez i Pellicer's Perl code
 * E.g. "113" => "one hundred thirteen"
 */
static bool expand3DigitNumber(int number,
			       std::vector<String> & expansion)
{
  static Char * digits [] = {
    "",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    NULL
  };

  static Char * tens [] = {
    "",
    "ten",
    "twenty",
    "thirty",
    "fourty",
    "fifty",
    "sixty",
    "seventy",
    "eighty",
    "ninety",
    NULL
  };

  static Char * specials [] = {
    "",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven",
    "twelve",
    "thirteen",
    "fourteen",
    "fifteen",
    "sixteen",
    "seventeen",
    "eighteen",
    "nineteen",
    NULL
  };

  if(number > 99){
    expansion.push_back(digits[number / 100]);
    expansion.push_back("hundred");
    number %= 100;
    if(number != 0){
      expansion.push_back("and");
    }
  }

  if(number > 0){
    if(number < 20){
      expansion.push_back(specials[number]);
    } else{
      expansion.push_back(tens[number / 10]);
      number %= 10;
      if(number != 0){
	expansion.push_back(digits[number]);
      }
    }
  }

  return true;
}

/**
 * Expands an integer number
 * This function was highly influenced by Edgar Gonzalez i Pellicer's Perl code
 * E.g. "113" => "one hundred thirteen"
 */
static bool expandInteger(const String & number,
			  std::vector<String> & expansion)
{
  Char * multipliers [] = {
    NULL, 
    "thousand",
    "million",
    "billion",
    NULL
  };

  long int value = strtol(number.c_str(), NULL, 10);
  LASSERT(value >= 0, "invalid integer value");

  // zero
  if(value == 0){ 
    expansion.push_back("zero");
    return true;
  }

  // break the number into groups of 3 digits
  list<int> groups;
  int groupCount = 0;
  while(value > 0){
    groups.push_front(value % 1000);
    groupCount ++;
    value /= 1000;
  }

  RASSERT(groupCount < 5, "Number too large");
  groupCount --;

  // process each 3-digit group
  for(list<int>::const_iterator it = groups.begin(); 
      it != groups.end(); it ++, groupCount --){

    int g = (* it);
    
    if(g != 0){
      expand3DigitNumber(g, expansion);
    }

    // add the corresponding multiplier
    if(multipliers[groupCount] != NULL){
      expansion.push_back(multipliers[groupCount]);
    }
  }

  return true;
}

/**
 * Expands an integer number as a sequence of digits
 * E.g. "113" => "one one three"
 */
static bool expandDigits(const String & number,
			 std::vector<String> & expansion)
{
  static Char * digits [] = {
    "zero",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    NULL
  };

  for(int i = 0; i < (int) number.size(); i ++){
    int value = (int) number[i] - '0';
    LASSERT(value < 10 && value >= 0, "invalid digit: " << value);
    expansion.push_back(digits[value]);
  }

  return true;
}

bool srl::expandNumber(const String & number,
			 std::vector<String> & expansion)
{
  if(! isNumber(number) &&
     ! isNumberRange(number)){
    return false;
  }

  String sign, left, sep, right;
  if(breakNumber(number, sign, left, sep, right) == false){
    return false;
  }

  if(sign == "-"){
    expansion.push_back("minus");
  }

  expandInteger(left, expansion);

  if(sep == "."){
    expansion.push_back("point");
  } else if(sep == "/"){
    expansion.push_back("over");
  } else if(sep == "-"){
    expansion.push_back("to");
  }

  if(right.empty() == false){
    if(sep == "."){
      if(right.size() < 3 && right[0] != '0'){
	expandInteger(right, expansion);
      } else{
	expandDigits(right, expansion);
      }
    } else{
      expandInteger(right, expansion);
    }
  }

  return true;
}

String srl::mergeStrings(const String & s1,
			   const String & s2)
{
  ostringstream os;
  os << s1 << s2;
  return os.str();
}

String srl::mergeStrings(const String & s1,
			   const String & s2,
			   const String & s3)
{
  ostringstream os;
  os << s1 << s2 << s3;
  return os.str();
}

String srl::mergeStrings(const String & s1,
			   const String & s2,
			   const String & s3,
			   const String & s4)
{
  ostringstream os;
  os << s1 << s2 << s3 << s4;
  return os.str();
}

String srl::stripString(const String & s,
			  int left,
			  int right)
{
  return s.substr(left, s.size() - left - right);
}

/*
int main(int argc, char ** argv)
{
  String p = argv[1];
  vector<String> exp;
  expandNumber(p, exp);
  for(int i = 0; (int) i < exp.size(); i ++){
    cout << "\"" << exp[i] << "\"" << " ";
  }
  cout << endl;
}
*/
