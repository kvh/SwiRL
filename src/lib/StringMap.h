
#include "RCIPtr.h"
#include "HashMap.h"
#include "CharArrayEqualFunc.h"
#include "CharArrayHashFunc.h"
#include "Wide.h"

#ifndef STRING_MAP_H
#define STRING_MAP_H

namespace srl {

  template <class T>
  class StringMapEntry : public RCObject {

  public:
    StringMapEntry(Char * key,
		   const T & value) {
      _key = key;
      _value = value;
    };

    Char * getKey() const { return _key; };

    const T & getValue() const { return _value; };

    void setValue(const T & v) { _value = v; };

    ~StringMapEntry() {
      delete [] _key;
    };

  private:
    Char * _key;
    T _value;
  };

  template <class T>
  class StringMap {

  public:

    typedef typename srl::HashMap<const Char *, RCIPtr< StringMapEntry<T> >,
      srl::CharArrayHashFunc, srl::CharArrayEqualFunc>::iterator 
      iterator;

    typedef typename srl::HashMap<const Char *, RCIPtr< StringMapEntry<T> >,
      srl::CharArrayHashFunc, srl::CharArrayEqualFunc>::const_iterator 
      const_iterator;

    bool set(const Char * key, const T & value) {
      if(contains(key)){
	return false;
      }
      
      int keyLen = 0;
      for(; key[keyLen] != 0; keyLen ++);
      
      Char * newKey = new Char[keyLen + 1];
      for(int i = 0; i < keyLen; i ++) newKey[i] = key[i];
      newKey[keyLen] = '\0';
      
      RCIPtr< StringMapEntry<T> > p(new StringMapEntry<T>(newKey, value));
      _map[p->getKey()] = p;
      
      return true;
    }

    bool overwrite(const Char * key, const T & value) {
      if(contains(key)){
	iterator it = _map.find(key);
	(* it).second->setValue(value);
	return true;
      } else {
	return set(key, value);
      }
    }

    bool get(const Char * key, T & value) const {
      const_iterator it;
      
      if((it = _map.find(key)) != _map.end()){
	value = (* it).second->getValue();
	return true;
      }
      
      return false;
    }

    RCIPtr< StringMapEntry<T> > get(const Char * key) const {
      const_iterator it;
      
      if((it = _map.find(key)) != _map.end()){
	return (* it).second;
      }
      
      return RCIPtr< StringMapEntry<T> >();
    }

    bool contains(const Char * key) const {
      if(_map.find(key) != _map.end()){
	return true;
      }
      
      return false;
    }

    void getKeys(std::vector<std::string> & keys) const {
      for(const_iterator it = begin(); it != end(); it ++){
	keys.push_back((* it).second->getKey());
      }
    }

    size_t size() const { return _map.size(); };

    bool empty() const { return _map.empty(); };

    const_iterator begin() const { return _map.begin(); };

    const_iterator end() const { return _map.end(); };

    iterator mbegin() { return _map.begin(); };

    iterator mend() { return _map.end(); };

  protected:

    srl::HashMap<const Char *, RCIPtr< StringMapEntry<T> >,
      srl::CharArrayHashFunc,
      srl::CharArrayEqualFunc> _map;

  };

} // end namespace srl

#endif
