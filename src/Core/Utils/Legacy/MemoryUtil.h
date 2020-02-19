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


/// @todo Documentation Core/Utils/Legacy/MemoryUtil.h

#ifndef CORE_UTIL_MEMORYUTIL_H
#define CORE_UTIL_MEMORYUTIL_H

#include <vector>
#include <boost/shared_array.hpp>
#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

template <class Cont>
void delete_all_items(Cont& cont)
{
  for (const auto& t : cont)
    delete t;
}

template <class MapType>
void delete_all_values(MapType& cont)
{
  for (const auto& t : cont)
    delete t.second;
}

template <typename T>
boost::shared_array<T> make_deep_copy(const boost::shared_array<T>& arr, size_t length)
{
  boost::shared_array<T> copy(new T[length]);
  std::copy(arr.get(), arr.get() + length, copy.get());
  return copy;
}

template <typename T>
boost::shared_array<T> make_deep_copy(const std::vector<T>& v)
{
  boost::shared_array<T> copy(new T[v.size()]);
  std::copy(v.begin(), v.end(), copy.get());
  return copy;
}

}

#endif
