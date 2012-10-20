
#include <ext/hash_map>

#ifndef HASH_MAP_H
#define HASH_MAP_H

namespace srl {

  template <class K, class V, class F, class E>
    class HashMap : public __gnu_cxx::hash_map<K, V, F, E> 
    {
    };

} // end namespace srl

#endif
