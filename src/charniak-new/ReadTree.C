#include "ReadTree.h"
#include "InputTree.h"
#include "Term.h"
#include "headFinder.h"

ECString ReadTree::tempword[400];
int      ReadTree::tempwordnum;
int      ReadTree::sentenceCount= 0;

istream&
operator >>( istream& is, InputTree& parse )
{
  if(parse.word() != "" || parse.term() != ""
     || parse.subTrees().size() > 0)
    error( "Reading into non-empty parse.",0 );
  ReadTree::readParse(is,parse);
  return is;
}
     
void
ReadTree::
readParse(istream& is, InputTree& ans)
{
  int pos = 0;
  ans.start_ = pos;
  ans.finish_ = pos;
  ans.num_ = -1;

  for(int i = 0 ; i < 128 ; i++)
    {
      tempword[i] = "";
    }

  tempwordnum = 0;
  ECString temp = readNext(is);
  if(!is) return;
  if(temp != "(")
    {
      cerr << "Saw " << temp << endl;
      error("Should have seen an open paren here.",0);
    }
  /* get annotated symbols like NP-OBJ.  term_ = NP ntInfo_ = OBJ */
  temp = readNext(is);
  ans.term_ = "S1";
  if(temp != "(")
    {
      if(temp == "S1" || temp == "TOP" || temp == "ROOT")
	{
	  temp = readNext(is);
	}
      else error("did not see legal topmost type",0);
    }
  if(temp == ")") return;
  if(temp != "(")
    {
      cerr << "Saw " << temp << endl;
      error("Should have seen second open paren here.",0);
    }

  for (;;)
    {
      InputTree* nextTree = newParse(is, pos, &ans);
      ans.subTrees_.push_back(nextTree);
      ans.finish_ = pos;

      ans.headTree_ = nextTree->headTree_;
      temp = readNext(is);
      if (temp==")") break;
      if (temp!="(")
	{
	  cerr << ans << endl;
	  error("Should have open or closed paren here.",0);
	}
    }
}

InputTree*
ReadTree::
newParse(istream& is, int& strt, InputTree* par)
{
  int strt1 = strt;
  ECString wrd;
  ECString trm;
  ECString ntInf;
  InputTrees subTrs;
  int num = -1;

  parseTerm(is, trm, ntInf,num);
  for( ; ; )
    {
      ECString temp = readNext(is);
      if(temp == "(")
	{
	  InputTree* nextTree = newParse(is, strt, NULL);
	  if(nextTree) subTrs.push_back(nextTree);
	}
      else if(temp == ")") break;
      else
	{
	  if(trm != "-NONE-")
	    {
	      wrd = temp;
	      strt++;
	    }
	}
    }

  /* the Chinese treebank has a single pos for all punctuation,
     which is pretty bad for the parser, so make each punc its own pos */
  /* fixes bugs in Chinese Treebank */
  if(Term::Language == "Ch")
    {
      if(trm == "PU") trm = wrd;
      const Term* ctrm = Term::get(trm);
      if(!ctrm)
	{
	  cerr << "No such term " << trm << endl;
	  assert(ctrm);
	}
      if(wrd!="" && !(ctrm->terminal_p()))
	{
	  cout<<trm<<wrd<<" changed to NN"<<endl;
	  trm="NN";
	}
      if(wrd=="" && ctrm->terminal_p() )
	{
	  cout<<trm<<" changed to NP"<<endl;
	  trm="NP";
	}
    }

  InputTree* ans = new InputTree(strt1, strt, wrd, trm, ntInf, subTrs,
				 par, NULL);
  ans->num() = num;
  InputTreesIter iti = subTrs.begin();
  for(; iti != subTrs.end() ; iti++)
    {
      InputTree* st = *iti;
      st->parentSet() = ans;
    }
  
  if(wrd == "" && subTrs.size() == 0) return NULL;
  if(wrd != "")
    {
      ans->headTree() = ans;
    }
  else
    {
      int hpos = headPosFromTree(ans);
      ans->headTree() = ithInputTree(hpos, subTrs)->headTree();
    }
  //cerr << "ANS " << ans->start() << " " << *ans << endl;
  return ans;
}

ECString&
ReadTree::
readNext( istream& is ) 
{
  // if we already have a word, use it, and increment pointer to next word;
  //cerr << "RN1 " << tempwordnum << " " << tempword[tempwordnum] << endl;
  if( tempword[tempwordnum] != "" )
    {
      return tempword[tempwordnum++];
    }
  //cerr << "RN2" << endl;
  // else zero out point and stuff in 
  int tempnum;
  for(tempnum = 0 ; tempword[tempnum] != "" ; tempnum++ )
    tempword[tempnum] = "";
  tempwordnum = 0;
  // then go though next input, separating "()[]";
  int    wordnum  = 0 ;
  ECString  temp;
  is >> temp;
  //cerr << "In readnext " << temp << endl;
  for( tempnum = 0 ; tempnum < temp.length() ; tempnum++ )
    {
      char tempC = temp[tempnum];
      if(tempC == '(' || tempC == ')' ||
	 tempC == '[' || tempC == ']' )
	{
	  if( tempword[wordnum] != "" )
	    wordnum++;
	  tempword[wordnum++] += tempC;
	}
      else tempword[wordnum] += temp[tempnum];
    }
  return tempword[tempwordnum++];
}

/* if we see NP-OBJ make NP a, and -OBJ b */
void
ReadTree::
parseTerm(istream& is, ECString& a, ECString& b, int& num)
{
  ECString temp = readNext(is);
  if(temp == "(" || temp == ")") error("Saw paren rather than term",0);
  int len = temp.length();
  size_t pos;
  pos = temp.find("^");
  if(pos < len && pos > 0)
    {
      ECString na(temp, 0, pos);
      ECString nb(temp, pos+1, len-pos-1);
      a = na;
      // cerr <<"NB " << na << " " << nb << endl;
      num = atoi(nb.c_str());
    }
  else a = temp;
  pos = a.find("-");
  /* things like -RCB- will have a - at position 0 */
  if(pos < len && pos > 0)
    {
      ECString na(a, 0, pos);
      ECString nb(a, pos, len-pos);
      a = na;
      len = pos;
      b = nb;
    }
  else
    {
      b = "";
    }
  pos = a.find("=");
  if(pos < len && pos > 0)
    {
      ECString na(temp, 0, pos);
      ECString nb(temp, pos, len-pos);
      a = na;
      len = pos;
      b += nb;
    }
  pos = a.find("|");
  if(pos < len && pos > 0)
    {
      ECString na(temp, 0, pos);
      ECString nb(temp, pos, len-pos);
      a = na;
      b += nb;
    }
}	   
