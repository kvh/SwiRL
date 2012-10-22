#include <list>
#include <string>

#include "RCIPtr.h"
#include "AssertLocal.h"
#include "Wide.h"

#ifndef ENGLISH_HEAD_H
#define ENGLISH_HEAD_H

#define TCI typename std::list< srl::RCIPtr<T> >::const_iterator
#define TCRI typename std::list< srl::RCIPtr<T> >::const_reverse_iterator

namespace srl {

  template <typename T>
    bool 
    isUnnecessaryNode(const srl::RCIPtr<T> & edge) 
    {
      static const Char * unnecessary [] = { 
	W("."), W("''"), W("``"), NULL
      };

      for(int i = 0; unnecessary[i] != NULL; i ++){
	if(edge->getLabel() == unnecessary[i]){
	  return true;
	}
      }

      return false;
    }

  /** Returns the right-most child that is not junk */
  template <typename T>
    srl::RCIPtr<T> 
    getLastChild(const std::list< srl::RCIPtr<T> > & children)
    {
      for(TCRI it = (TCRI) children.rbegin(); 
	  it != (TCRI) children.rend(); it ++)
	if(isUnnecessaryNode(* it) == false) return (* it);
      return children.back();
    }

  /** Returns the left-most child that is not junk */
  template <typename T>
    srl::RCIPtr<T> 
    getFirstChild(const std::list< srl::RCIPtr<T> > & children)
    {
      for(TCI it = (TCI) children.begin(); 
	  it != (TCI) children.end(); it ++)
	if(isUnnecessaryNode(* it) == false) return (* it);
      return children.front();
    }

  template <typename T>
    srl::RCIPtr<T> 
    traverseRightToLeft(const std::list< srl::RCIPtr<T> > & children, 
			const Char * labels [],
			bool traverseLabelsFirst = true)
    {
      if(traverseLabelsFirst == true){
	//
	// Traverse labels than children
	//
	for(int i = 0; labels[i] != NULL; i ++){
	  for(TCRI it = (TCRI) children.rbegin(); 
	      it != (TCRI) children.rend(); it ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      } else{
	//
	// Traverse children than labels
	//
	for(TCRI it = (TCRI) children.rbegin(); 
	    it != (TCRI) children.rend(); it ++){
	  for(int i = 0; labels[i] != NULL; i ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      }

      return srl::RCIPtr<T>();
    }

  template <typename T>
    srl::RCIPtr<T> 
    traverseLeftToRight(const std::list< srl::RCIPtr<T> > & children, 
			const Char * labels [],
			bool traverseLabelsFirst = true)
    {
      if(traverseLabelsFirst == true){
	//
	// Traverse labels than children
	//
	for(int i = 0; labels[i] != NULL; i ++){
	  for(TCI it = (TCI) children.begin(); 
	      it != (TCI) children.end(); it ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      } else{
	//
	// Traverse children than labels
	//
	for(TCI it = (TCI) children.begin(); 
	    it != (TCI) children.end(); it ++){
	  for(int i = 0; labels[i] != NULL; i ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      }

      return srl::RCIPtr<T>();
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishNP(const String & parent,
		      const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] =
	{ W("NN"), W("NNP"), W("NNPS"), W("NNS"), W("NX"), W("POS"), W("JJR"), NULL };
      static const Char * row2 [] = 
	{ W("NP"), W("NP-A"), W("NPB"), NULL };
      static const Char * row3 [] = 
	{ W("$"), W("ADJP"), W("ADJP-A"), W("PRN"), NULL };
      static const Char * row4 [] = 
	{ W("CD"), NULL };
      static const Char * row5 [] = 
	{ W("JJ"), W("JJS"), W("RB"), W("QP"), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseLeftToRight<T>(children, row2, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseRightToLeft<T>(children, row3, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseRightToLeft<T>(children, row4, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseRightToLeft<T>(children, row5, false)) != 
	 (const T *) NULL)
	return head;

      return getLastChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishADJP(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = {  
	W("NNS"), W("QP"), W("NN"), W("$"), W("ADVP"), W("ADVP-A"), W("VBN"), W("JJ"), W("PRT"), 
	W("ADVP|PRT"), W("VBG"), W("ADJP"), W("ADJP-A"), W("JJR"), W("NP"), W("NP-A"), W("NPB"), W("JJS"), W("DT"), 
	W("FW"), W("RBR"), W("RBS"), W("SBAR"), W("SBAR-A"), W("RB"), W("IN"), W("VBD"), W("AUX"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishADVP(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("RB"), W("RBR"), W("RBS"), W("FW"), W("ADVP"), W("ADVP-A"), W("PRT"), W("ADVP|PRT"), W("TO"), 
	W("CD"), W("JJR"), W("JJ"), W("IN"), W("NP"), W("NP-A"), W("NPB"), W("JJS"), W("NN"), W("PP"), W("PP-A"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishCONJP(const String & parent,
			 const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("CC"), W("RB"), W("IN"), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishFRAG(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      return getLastChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishINTJ(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      return getFirstChild(children);
    }
  
  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishLST(const String & parent,
		       const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("LS"), W(":"), W(","), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishNAC(const String & parent,
		       const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("NN"), W("NNS"), W("NNP"), W("NNPS"), W("NP"), W("NP-A"), W("NPB"), W("NAC"), W("EX"), W("$"), 
	W("CD"), W("QP"), W("PRP"), W("VBG"), W("AUX"), W("JJ"), W("JJS"), W("JJR"), W("ADJP"), W("ADJP-A"), W("FW"), 
	NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishPP(const String & parent,
		      const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = 
	{ W("IN"), W("TO"), W("VBG"), W("VBN"), W("AUX"), W("RP"), W("FW"), W("PP"), W("PP-A"), W("ADJP"), W("ADJP-A"), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getLastChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishPRN(const String & parent,
		       const std::list< srl::RCIPtr<T> > & children) 
    {
      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishQP(const String & parent,
		      const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("$"), W("IN"), W("NNS"), W("NN"), W("JJ"), W("RB"), W("DT"), W("CD"), W("QP"), W("JJR"), 
	W("JJS"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishRRC(const String & parent,
		       const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = {
	W("VP"), W("VP-A"), W("NP"), W("NP-A"), W("NPB"), W("ADVP"), W("ADVP-A"), W("PRT"), W("ADVP|PRT"), W("ADJP"), W("ADJP-A"), W("PP"), W("PP-A"), 
	NULL 
      };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishS(const String & parent,
		     const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("TO"), W("IN"), W("VP"), W("VP-A"), W("S"), W("S-A"), W("SBAR"), W("SBAR-A"), W("ADJP"), W("ADJP-A"), W("UCP"), W("UCP-A"), W("NP"), W("NP-A"), W("NPB"), 
	NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishSBAR(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("WHNP"), W("WHPP"), W("WHADVP"), W("WHADJP"), W("IN"), W("DT"), W("S"), W("S-A"), W("SQ"), W("SQ-A"), 
	W("SINV"), W("SBAR"), W("SBAR-A"), W("FRAG"), W("FRAG-A"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishSBARQ(const String & parent,
			 const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("SQ"), W("SQ-A"), W("S"), W("S-A"), W("SINV"), W("SBARQ"), W("SBARQ-A"), W("FRAG"), W("FRAG-A"), W("SBAR"), W("SBAR-A"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishSINV(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("VBZ"), W("VBD"), W("VBP"), W("VB"), W("AUX"), W("MD"), W("VP"), W("VP-A"), W("S"), W("S-A"), W("SINV"), W("ADJP"), W("ADJP-A"), 
	W("NP"), W("NP-A"), W("NPB"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishSQ(const String & parent,
		      const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("VBZ"), W("VBD"), W("VBP"), W("VB"), W("AUX"), W("MD"), W("VP"), W("VP-A"), W("SQ"), W("SQ-A"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishUCP(const String & parent,
		       const std::list< srl::RCIPtr<T> > & children) 
    {
      return getLastChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishVP(const String & parent,
		      const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("TO"), W("VBD"), W("VBN"), W("AUX"), W("MD"), W("VBZ"), W("VB"), W("VBG"), W("VBP"), W("VP"), W("VP-A"), W("VPB"), 
	W("ADJP"), W("ADJP-A"), W("NN"), W("NNS"), W("NP"), W("NP-A"), W("NPB"), NULL
      };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishWHADJP(const String & parent,
			  const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("CC"), W("WRB"), W("JJ"), W("ADJP"), W("ADJP-A"), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishWHADVP(const String & parent,
			  const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("CC"), W("WRB"), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishWHNP(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = 
	{ W("WDT"), W("WP"), W("WP$"), W("WHADJP"), W("ADJP-A"), W("WHPP"), W("WHNP"), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishWHPP(const String & parent,
			const std::list< srl::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("IN"), W("TO"), W("FW"), NULL };

      srl::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishTOP(const String & parent,
		       const std::list< srl::RCIPtr<T> > & children) 
    {
      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishX(const String & parent,
		     const std::list< srl::RCIPtr<T> > & children) 
    {
      return getFirstChild(children);
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglishNoCoord(const String & parent,
			   const std::list< srl::RCIPtr<T> > & children) 
    {
      if(parent == W("NP") || 
	 parent == W("NPB") || 
	 parent == W("NP-A")){
	return findHeadEnglishNP<T>(parent, children);
      } else if(parent == W("ADJP") ||
		parent == W("ADJP-A")){
	return findHeadEnglishADJP<T>(parent, children);
      } else if(parent == W("ADVP") || 
		parent == W("PRT") || 
		parent == W("ADVP-A") || 
		parent == W("ADVP|PRT") ||
		parent == W("PRT|ADVP")){
	return findHeadEnglishADVP<T>(parent, children);
      } else if(parent == W("CONJP")){
	return findHeadEnglishCONJP<T>(parent, children);
      } else if(parent == W("FRAG") ||
		parent == W("FRAG-A")){
	return findHeadEnglishFRAG<T>(parent, children);
      } else if(parent == W("INTJ") ||
		parent == W("INTJ-A")){
	return findHeadEnglishINTJ<T>(parent, children);
      } else if(parent == W("LST")){
	return findHeadEnglishLST<T>(parent, children);
      } else if(parent == W("NAC")){
	return findHeadEnglishNAC<T>(parent, children);
      } else if(parent == W("PP") ||
		parent == W("PP-A")){
	return findHeadEnglishPP<T>(parent, children);
      } else if(parent == W("PRN")){
	return findHeadEnglishPRN<T>(parent, children);
      } else if(parent == W("QP")){
	return findHeadEnglishQP<T>(parent, children);
      } else if(parent == W("RRC")){
	return findHeadEnglishRRC<T>(parent, children);
      } else if(parent == W("S") ||
		parent == W("SG") ||
		parent == W("S-A")){
	return findHeadEnglishS<T>(parent, children);
      } else if(parent == W("SBAR") ||
		parent == W("SBAR-A")){
	return findHeadEnglishSBAR<T>(parent, children);
      } else if(parent == W("SBARQ") ||
		parent == W("SBARQ-A")){
	return findHeadEnglishSBARQ<T>(parent, children);
      } else if(parent == W("SINV")){
	return findHeadEnglishSINV<T>(parent, children);
      } else if(parent == W("SQ") ||
		parent == W("SQ-A")){
	return findHeadEnglishSQ<T>(parent, children);
      } else if(parent == W("UCP") ||
		parent == W("UCP-A")){
	return findHeadEnglishUCP<T>(parent, children);
      } else if(parent == W("VP") ||
		parent == W("VP-A")){
	return findHeadEnglishVP<T>(parent, children);
      } else if(parent == W("WHADJP")){
	return findHeadEnglishWHADJP<T>(parent, children);
      } else if(parent == W("WHADVP")){
	return findHeadEnglishWHADVP<T>(parent, children);
      } else if(parent == W("WHNP")){
	return findHeadEnglishWHNP<T>(parent, children);
      } else if(parent == W("WHPP")){
	return findHeadEnglishWHPP<T>(parent, children);
      } else if(parent == W("TOP") ||
		parent == W("S1")){
	return findHeadEnglishTOP<T>(parent, children);
      } else if(parent == W("X") ||
		parent == W("NX")){
	return findHeadEnglishX<T>(parent, children);
      } else{
	CERR << "Can not find head edge for label " << parent << std::endl;
	CERR << "The children are:\n";
	for(TCI it = (TCI) children.begin(); 
	    it != (TCI) children.end(); it ++){
	  (* it)->display(CERR);
	  CERR << std::endl;
	}
	LVASSERT(false, "unknown edge label");
      }
      
      return getFirstChild(children);
    }

  template <typename T>
    TCRI
    isCoordinatedHead(const std::list< srl::RCIPtr<T> > & children,
		      TCRI current)
    {
      // Advance over head
      current ++;
      if(current == children.rend()) return children.rend();

      // Advance over CC
      if((* current)->getLabel() == W("CC")){
	current ++;
	if(current == children.rend()) return children.rend();
      } else{
	return children.rend();
      }
      
      // Advance over any commas
      for(; current != (TCRI) children.rend() && 
	    (* current)->getLabel() == W(",");
	  current ++);
      if(current == children.rend()) return children.rend();

      return current;
    }

  template <typename T>
    srl::RCIPtr<T>
    findHeadEnglish(const String & parent,
		    const std::list< srl::RCIPtr<T> > & children) 
    {
      LVASSERT(children.empty() == false, "empty children set");
      
      // Find the head edge before the CC condition
      srl::RCIPtr<T> head = 
	findHeadEnglishNoCoord<T>(parent, children);

      // Check for coordinations
      for(TCRI it = (TCRI) children.rbegin(); 
	  it != (TCRI) children.rend(); it ++){

	// Found the head 
	if((* it) == head){
	  TCRI newHead;

	  // If it is a valid CC change the head to the edge before the CC
	  if((parent != W("NPB")) &&
	     (newHead = isCoordinatedHead<T>(children, it)) != children.rend()){
	    return (* newHead);
	  }

	  return head;
	}
      }

      return head;
    }

} // end namespace srl

#endif // ENGLISH_HEAD_H
