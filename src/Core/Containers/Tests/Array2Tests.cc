/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <gtest/gtest.h>
#include <Core/Containers/Array2.h>

using namespace SCIRun;

namespace
{
  template <typename Array>
  void print(std::ostream& os, const Array& A)
  {
    typename Array::const_iterator i;
    os << "[";
    for (i = A.begin(); i != A.end(); ++i) 
    {
      print(os, *i);
      if (boost::next(i) != A.end())
        os << ',';
    }
    os << "]";
  }

  template<>
  void print<double>(std::ostream& os, const double& x)
  {
    os << x;
  }
}

TEST(Array2Test, CanResize)
{
  Array2<double> a;
  EXPECT_EQ(0, a.shape()[0]);
  EXPECT_EQ(0, a.shape()[1]);
  a.resize(2, 3);
  EXPECT_EQ(2, a.shape()[0]);
  EXPECT_EQ(3, a.shape()[1]);
}

TEST(Array2Test, CanAccessUnderlyingStorageWithSingleIndexer)
{
  Array2<double> a;
  a.resize(2, 3);
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 3; ++j)
      (static_cast<Array2<double>::base_type&>(a))[i][j] = 1 + i + j;
  

  // [[1,2,3],[2,3,4]]
  print(std::cout, a);
  std::cout << std::endl;

  std::for_each(a.origin(), a.origin() + a.totalSize(), [](double& d) {d *= 0.5;});

  // [[1,2,3],[2,3,4]]
  print(std::cout, a);
  std::cout << std::endl;
}