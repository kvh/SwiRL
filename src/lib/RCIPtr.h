/**
 * @file RCIPtr.h
 * @brief Reference counting pointer class
 * This file contains the code for the classes and class templates making up
 * the Reference Counting Item of More Effective C++ (source code item 29) by
 * Scott Meyers.
 * This code is optimized as follows: in order to avoid small-memory 
 * allocations, all classes that use smart pointers are required to inherit 
 * from RCObject, such that the reference counter is included in the objects
 * themselves.
 */

#ifndef RCI_PTR_H
#define RCI_PTR_H

namespace srl {

/**
 * Base class for reference-counted objects
 */
class RCObject 
{
 public:
  void addReference();
  void removeReference();

 protected:
  RCObject();
  RCObject(const RCObject& rhs);
  RCObject& operator=(const RCObject& rhs);
  virtual ~RCObject() = 0;

 public:
  unsigned short refCount;
};

inline RCObject::RCObject()
  : refCount(0){} // std::cout << "RCObject constr\n"; }

inline RCObject::RCObject(const RCObject&)
  : refCount(0){} // std::cout << "RCObject copy constr\n"; }

inline RCObject& RCObject::operator=(const RCObject&)
{
  return *this;
}  

inline RCObject::~RCObject() {}

inline void RCObject::addReference() 
{
  ++refCount;
}

inline void RCObject::removeReference()
{
  if (--refCount == 0) delete this;
}

/**
 * Template class for reference counting pointers
 */
template<class T>
class RCIPtr 
{
 public:
  explicit RCIPtr(T* realPtr = 0);
  RCIPtr(const RCIPtr& rhs);
  ~RCIPtr();
  RCIPtr& operator=(const RCIPtr& rhs);

  T* operator->() const;
  T& operator*() const;

  void clear() {
    *this = RCIPtr<T>(0);
  };

private:
  T *pointee;
  void init() {
    if(pointee != 0) pointee->addReference();
  }
};

template<typename T>
inline bool operator== (const RCIPtr<T> & pointer, const RCIPtr<T> & other)
{
  return pointer.operator->() == other.operator->();
}

template<typename T>
inline bool operator!= (const RCIPtr<T> & pointer, const RCIPtr<T> & other)
{
  return pointer.operator->() != other.operator->();
}

template<typename T>
inline bool operator== (const RCIPtr<T> & pointer, const T * other)
{
  return pointer.operator->() == other;
}

template<typename T>
inline bool operator== (const T * other, const RCIPtr<T> & pointer)
{
  return pointer.operator->() == other;
}

template<typename T>
inline bool operator!= (const RCIPtr<T> & pointer, const T * other)
{
  return pointer.operator->() != other;
}

template<typename T>
inline bool operator!= (const T * other, const RCIPtr<T> & pointer)
{
  return pointer.operator->() != other;
}

template<class T>
RCIPtr<T>::RCIPtr(T* realPtr)
  : pointee(realPtr)
{ 
  init();
}

template<class T>
RCIPtr<T>::RCIPtr(const RCIPtr& rhs)
  : pointee(rhs.pointee)
{ 
  init(); 
}

template<class T>
RCIPtr<T>::~RCIPtr()
{ 
  if(pointee != 0) pointee->removeReference(); 
}

template<class T>
RCIPtr<T>& RCIPtr<T>::operator=(const RCIPtr& rhs)
{
  if (pointee != rhs.pointee) {         
    if(pointee != 0) pointee->removeReference();     
    pointee = rhs.pointee;
    init();
  }
  return *this;
}

template<class T>
T* RCIPtr<T>::operator->() const
{ return pointee; }

template<class T>
T& RCIPtr<T>::operator*() const
{ return *(pointee); }

} // end namespace srl

#endif // RCI_PTR_H

