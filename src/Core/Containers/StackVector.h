/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/*
 *  StackVector.h: Interface to statically allocated std::vector class.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   July 2004
 *
 */

// This implements a subclass of the std::vector class, except that
// the vector is statically allocated on the stack for performance.

#ifndef SCI_Containers_StackVector_h
#define SCI_Containers_StackVector_h 1

#include <Core/Util/Assert.h>
#include <vector>


namespace SCIRun {

template <class T, int CAPACITY>
class StackVector {
public:
  size_t size_;
  T data_[CAPACITY];

  typedef T        value_type;
  typedef T*       pointer;
  typedef T&       reference;
  typedef const T& const_reference;
  //typedef difference_type;
  typedef T*       iterator;
  typedef const T* const_iterator;
  typedef size_t   size_type;
  //typedef reverse_iterator;
  //typedef const_reverse_iterator;

  iterator begin() { return data_; }
  iterator end() { return data_+size_; }
  const_iterator begin() const { return data_; }
  const_iterator end() const { return data_+size_; }
  size_t size() const { ASSERT(size_ <= CAPACITY); return size_; }
  size_t max_size() const { return CAPACITY; }
  size_t capacity() const { return CAPACITY; }
  bool empty() const { return size_; }
  reference operator[](size_t n) { return data_[n]; }
  const_reference operator[](size_t n) const { return data_[n]; }
  void resize(size_t s) { ASSERT(s <= CAPACITY); size_ = s; }
  void reserve(size_t) {}  // Do nothing: We already have a certain capacity
  void push_back(T t) { ASSERT(size_ < CAPACITY); data_[size_] = t; size_++; }
  void clear() { size_ = 0; }
  StackVector() { size_ = 0; }
  StackVector(size_t s) { ASSERT(s <= CAPACITY); size_ = s; }
  StackVector(size_t s, T val)
  { ASSERT(s <= CAPACITY); size_ = s; for (size_t i=0;i<s;i++) data_[i] = val;}
  ~StackVector() {}
};

} // End namespace SCIRun


#endif /* SCI_Containers_StackVector_h */

