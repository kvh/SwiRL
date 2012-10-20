/*
 * Copyright 1997, Brown University, Providence, RI.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef UTILS_H 
#define UTILS_H

#include "ECString.h"
#include <vector>

#define WARN( msg ) warn( __FILE__, __LINE__, msg )
#define ERROR( msg ) error( __FILE__, __LINE__, msg )

void warn( const char *filename, int filelinenum, const char *msg );
void error( const char *filename, int filelinenum, const char *msg );
void error(const char *s); // backwards compatibility

char* langAwareToLower(const char* str, char* temp);
char* toLower(const char* str, char* temp);
ECString intToString(int i);

typedef vector<ECString> ECStrings;
typedef ECStrings::iterator ECStringsIter;
bool vECfind(const ECString& st, ECStrings& sts);

void find_and_replace(std::string & tInput, std::string tFind, std::string tReplace);
void escape_parens(ECString& word);

#endif /* ! UTILS_H */

