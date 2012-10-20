
#include <string>
#include <vector>

#include "Wide.h"

#ifndef CHAR_UTILS_H
#define CHAR_UTILS_H

namespace srl {

  bool compareWideCharToWideChar(const Char * s1,
				 const Char * s2);

  void simpleTokenize(const String & input,
		      std::vector<String> & output,
		      const String & separators);

  /**
   * String tokenization, considering everything within quotes as 1 token
   * Regular quotes inside tokens MUST be preceded by \
   */
  void tokenizeWithQuotes(const String & input,
			  std::vector<String> & output,
			  const String & separators);

  /**
   * Constructs a valid quote-surrounded token
   * All inside quotes are preceded by \
   */
  String quotify(const String & str);

  int toInteger(const String & s);

  /** Returns true if this is an integer, real, or fraction number */
  bool isNumber(const String & s);

  /** Returns true if this is a number range, e.g. "N-M" */
  bool isNumberRange(const String & s);

  String toUpper(const String & s);

  String toLower(const String & s);

  /**
   * Expands numbers/fractions/ranges to their alphabetical representation
   * E.g. "113" ==> "one hundred thirteen"
   */
  bool expandNumber(const String & number,
		    std::vector<String> & expansion);

  String mergeStrings(const String & s1,
		      const String & s2);

  String mergeStrings(const String & s1,
		      const String & s2,
		      const String & s3);

  String mergeStrings(const String & s1,
		      const String & s2,
		      const String & s3,
		      const String & s4);

  bool startsWith(const String & big,
		  const String & small);

  bool endsWith(const String & big,
		const String & small);

  String stripString(const String & s,
		     int left,
		     int right);
  
} // end namespace srl

#endif
