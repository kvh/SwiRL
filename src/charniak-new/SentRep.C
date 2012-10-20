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

#include "Bchart.h"
#include "SentRep.h"
#include "Params.h"
#include "utils.h"
#include "ewDciTokStrm.h"
#include <assert.h>

//------------------------------

SentRep::
SentRep()
{
  sent_.reserve(Params::DEFAULT_SENT_LEN);
}

//------------------------------

SentRep::
SentRep(int size)
{
  sent_.reserve(size);
}

//------------------------------

SentRep::
SentRep(list<ECString> wtList) 
  : sent_( wtList.size() )
{
  int i = 0;
  for (list<ECString>::iterator wi = wtList.begin(); wi != wtList.end(); ++wi){
      sent_[i] = Wrd(*wi,i);
      i++;
  }
}

//------------------------------
// 05/30/06 ML
// This really belongs in ewDciTokStrm.h/.C, but as it appears it
// isn't needed elsewhere, I'll just avoid modifying another file.

static ewDciTokStrm& operator>>( ewDciTokStrm& istr, ECString& w)
{
  w = istr.read();
  return istr;
}

//------------------------------
// Avoid scaring anyone by exposing use of template in header file --
// instead just map operator>> calls to this file local function.

template<class T>
static T& readSentence( T& istr, vector<Wrd>& sent, ECString& name)
{
  sent.clear();
  ECString w;

  while (!(!istr))
    {
      istr >> w;
      if( w == "<s>" ) 
	break;
      if(w == "<s")
	{
	  istr >> name;
	  if ( name[name.length()-1] == '>' ) 
	    {	    
	      name = name.substr(0,name.length()-1); // discard trailing '>'
	    } 
	  else // "<s LABEL >"
	    {
	      istr >> w;
	      if ( w != ">" ) 
		WARN("No closing '>' delimiter found to match opening \"<s\"");
	    } 
	  break;
	}
    }
  while (!(!istr))
    {
      istr >> w;

      if (w == "</s>" )
	break;

      // XXX previously:
      // Wrd::substBracket(w);
      escape_parens(w);
      int pos = sent.size();
      sent.push_back(Wrd(w,pos));
    }

  return istr;
}


//------------------------------

istream& operator>> (istream& is, SentRep& sr) 
{
  return readSentence(is, sr.sent_, sr.name_);
}

//------------------------------

ewDciTokStrm& operator>> (ewDciTokStrm& is, SentRep& sr)
{
  return readSentence(is, sr.sent_, sr.name_);
}

//------------------------------

ostream& operator<< (ostream& os, const SentRep& sr) 
{
  for( int i = 0; i < sr.length(); i++ )
    os << sr[ i ] << " ";
  return os;
}

////////////////////////////////
// End of File
