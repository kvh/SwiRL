
#ifndef SRL_ARG_H
#define SRL_ARG_H

#include "RCIPtr.h"
#include "Wide.h"

namespace srl {
  class Tree;

  typedef enum BType { 
    B_SINGLE, // has no Is
    B_FULL_TERM, // B,I terms w same parent, full span
    B_INC_TERM, // B,I terms w same parent, incomplete span
    B_SAMEP, // !term, but same parent
    B_UGLY // !term, !same parent
  };

  class Argument {
  public:
    Argument() : _verbPosition(-1), _begin(NULL), _bType(B_SINGLE), _prob(0) {};

    Argument(const String & s);

    Argument(const String & type, 
	     const String & name,
	     int position,
	     const Tree * begin = NULL) :
      _type(type), _name(name), 
      _verbPosition(position), _begin(begin), _bType(B_SINGLE), _prob(0) {}

    void display(OStream & os,
		 const String & prefix,
		 const String & suffix) const;

    const String & getType() const { return _type; }

    const String & getName() const { return _name; }

    String getTypeName() const;

    String getFullName() const;

    String getLabel() const { return getTypeName(); }

    const int getVerbPosition() const { return _verbPosition; }

    const Tree * getBegin() const { return _begin; }
    void setBegin(const Tree * t) { _begin = t; }

    void setBType(BType bt) { _bType = bt; }
    BType getBType() const { return _bType; }

    double getProb() const { return _prob; }
    void setProb(double p) { _prob = p; }

    bool isClean() const;

    bool operator == (const Argument & other) const;

  private:
    
    /** Type, e.g. B or I */
    String _type; 

    /** Name e.g. A0, A1 */
    String _name; 

    /** Predicate position in the terminal vector */
    int _verbPosition;

    /** B-A if this argument is an I */
    const Tree * _begin;

    /** 
     * Subtype of argument, if type is B. 
     * Currently used only by the corpus analysis code.
     */ 
    BType _bType;

    /** 
     * The probability with which this argument was generated (prediction only)
     * This is computed with the softmax function from the classifier confidence
     */
    double _prob;
  };
}

#endif

