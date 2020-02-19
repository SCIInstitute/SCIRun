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
///@file   Array2.h
///@brief  Interface to dynamic 2D array class
///
///@author Steven G. Parker
/// 	   Department of Computer Science
/// 	   University of Utah
///@date   March 1994
///

#ifndef CORE_CONAINTERS_ARRAY2_H
#define CORE_CONAINTERS_ARRAY2_H 1

#include <boost/multi_array.hpp>

#include <Core/Persistent/Persistent.h>

namespace SCIRun {

template<class T>
class Array2
{
public:
  typedef boost::multi_array<T, 2> impl_type;
  typedef T value_type;

  Array2() {}

  Array2(size_t size1, size_t size2)
  {
    resize(size1, size2);
  }

  void resize(size_t size1, size_t size2)
  {
    typename impl_type::extent_gen extents;
    impl_.resize(extents[size1][size2]);
  }

  size_t size() const
  {
    return dim1() * dim2();
  }

  T& operator[](size_t idx)
  {
    return impl_.origin()[idx];
  }

  const T& operator[](size_t idx) const
  {
    return impl_.origin()[idx];
  }

  inline const T& operator()(index_type d1, index_type d2) const
  {
    return impl_[d1][d2];
  }

  inline T& operator()(index_type d1, index_type d2)
  {
    return impl_[d1][d2];
  }

  //////////
  ///Returns number of rows
  inline size_t dim1() const {return impl_.shape()[0];}

  //////////
  ///Returns number of cols
  inline size_t dim2() const {return impl_.shape()[1];}

  impl_type& getImpl() { return impl_; }
private:
  impl_type impl_;
};

template<class T> void Pio(Piostream& stream, Array2<T>& data);
template<class T> void Pio(Piostream& stream, Array2<T>*& data);

#define ARRAY2_VERSION 2

template<class T>
void Pio(Piostream& stream, Array2<T>& data)
{
  int version = stream.begin_class("Array2", ARRAY2_VERSION);
  if(stream.reading())
  {
    // Allocate the array...
    if (version < 2)
    {
      int d1, d2;
      Pio(stream, d1);
      Pio(stream, d2);
      data.resize(static_cast<size_type>(d1), static_cast<size_type>(d2));
    }
    else
    {
      long long d1, d2;
      Pio(stream, d1);
      Pio(stream, d2);
      data.resize(static_cast<size_type>(d1), static_cast<size_type>(d2));
    }
  }
  else
  {
    long long d1 = static_cast<long long>(data.dim1());
    long long d2 = static_cast<long long>(data.dim2());
    Pio(stream, d1);
    Pio(stream, d2);
  }
  if (stream.supports_block_io())
  {
    stream.block_io(&data[0],sizeof(T),data.size());
  }
  else
  {
    for(index_type i=0;i<data.dim1();i++)
    {
      for(index_type j=0;j<data.dim2();j++)
      {
        Pio(stream, data.getImpl()[i][j]);
      }
    }
  }
  stream.end_class();
}

template<class T>
void Pio(Piostream& stream, Array2<T>*& data)
{
  if (stream.reading())
  {
    data=new Array2<T>;
  }
  Pio(stream, *data);
}

} // End namespace SCIRun

#endif
