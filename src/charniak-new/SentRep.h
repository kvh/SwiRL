/*
 * Copyright 1999, 2005 Brown University, Providence, RI.
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

#ifndef SENTREP_H
#define SENTREP_H

#include "Wrd.h"
#include "ewDciTokStrm.h"
#include <istream>
#include <list>
#include <ostream>
#include <vector>

class SentRep
{
public:

    SentRep();
    SentRep(int size); // initial size for vector to grow from
    SentRep(list<ECString> wtList); // used by wwBCTest

    // SGML layout introduces sentence with <s> and ends it with </s>.
    // <s name> ... </s> also allowed and returned as "name" parameter. 
    friend istream& operator>> (istream& is, SentRep& sr);
    friend ewDciTokStrm& operator>> (ewDciTokStrm& is, SentRep& sr);

    int length() const { return sent_.size(); }

    const Wrd& operator[] ( int index ) const { return sent_[ index ]; }
    Wrd&       operator[] ( int index )       { return sent_[ index ]; }

    const ECString& getName() const { return name_; }

  private:

    vector<Wrd> sent_;
    ECString name_;

};

ostream& operator<< (ostream& os, const SentRep& sr);

#endif /* ! SENTREP_H */
