#include "ExtPos.h"
#include "utils.h"
#include <sstream>

/*
  external pos file is of form

  tag1 tag2 //tages for first word 
  tag3
  ---
*/

void
ExtPos::
read(ifstream* isp,SentRep& sr)
{
  clear();
  assert(isp);
  istream& is=*isp;
  if(!is)return;
  int i;
  for(i=0;;i++){
    string buf;
    getline(is,buf); 
    if(buf=="---"){
      break;
    }
    if(!is){
      break;
    }
    vector<const Term*> vt;
    stringstream ss(buf);
    ECString wrd;
    ss>>wrd;  //First entry on each line is the word, not a tag.
    escape_parens(wrd);
    if (wrd != sr[i].lexeme()) {
      cerr << "wrd:            '" << wrd << "'" << endl;
      cerr << "sr[i].lexeme(): '" << sr[i].lexeme() << "'" << endl;
    }
    assert(wrd==sr[i].lexeme());
    while(ss){
      ss>>wrd;
      if (!ss) break;
      const Term* trm=Term::get(wrd);
      if (trm) {
        vt.push_back(trm);
      } else {
        cerr << "Warning: Haven't seen term '" << wrd << "' in terms.txt" << endl;
      }
    }
    push_back(vt);
  }
}

bool ExtPos::hasExtPos() {
    for (size_t i = 0; i < size(); i ++) {
        vector<const Term*> terms = operator[](i);
        if (terms.size() > 0) {
            return true;
        }
    }

    return false;
}
