#ifndef PAIR_H
#define PAIR_H

namespace srl {

  /**
   * Wrapper for the non-standard pair
   */
  template <class T1, class T2>
    class Pair
    {
     public:

      Pair(const T1 & t1, const T2 & t2) : _first(t1), _second(t2) {};

      const T1 & getFirst() const { return _first; };

      const T2 & getSecond() const { return _second; };

      T2 & getSecond() { return _second; };

      void setFirst(const T1 & t1) { _first = t1; }

      void setSecond(const T2 & t2) { _second = t2; }

     private:
      
      T1 _first;

      T2 _second;
    };

} // end namespace srl

#endif
