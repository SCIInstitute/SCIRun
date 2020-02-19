/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


///
///@file   StackVector.h
///@brief  Interface to statically allocated std::vector class.
///
///@author Michael Callahan
///	   Department of Computer Science
/// 	   University of Utah
///@date   July 2004
///

/// This implements a subclass of the std::vector class, except that
/// the vector is statically allocated on the stack for performance.

#ifndef SCI_Containers_StackVector_h
#define SCI_Containers_StackVector_h 1

#include <boost/array.hpp>

namespace SCIRun {

template <class T, int CAPACITY>
class StackVector : public boost::array<T, CAPACITY>
{
public:
  typedef boost::array<T, CAPACITY> base_type;
  typedef typename base_type::value_type value_type;

  StackVector() : size_(0) {}
  StackVector(size_t size, const value_type& v = value_type()) : size_(size)
  {
    this->fill(v);
  }
  void resize(size_t size, const value_type& val = value_type())
  {
    size_ = size;
    //not sure what to do here. semantics is different, but SCIRun 4 probably overruns buffers all the time anyway...
  }
  void clear() {}
  size_t size() const { return size_; }
private:
  size_t size_;
};

} // End namespace SCIRun


#endif /* SCI_Containers_StackVector_h */
