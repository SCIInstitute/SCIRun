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


#include <gtest/gtest.h>
#include <Core/Containers/Array2.h>

using namespace SCIRun;

namespace
{
  template <typename Array>
  void print(std::ostream& os, const Array& A)
  {
    os << "[";
    for (auto i = A.begin(); i != A.end(); ++i)
    {
      print(os, *i);
      if (i + 1 != A.end())
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
  EXPECT_EQ(0, a.dim1());
  EXPECT_EQ(0, a.dim2());
  EXPECT_EQ(0, a.size());
  a.resize(2, 3);
  EXPECT_EQ(2, a.dim1());
  EXPECT_EQ(3, a.dim2());
  EXPECT_EQ(6, a.size());
}

TEST(Array2Test, CanAccessUnderlyingStorageWithSingleIndexer)
{
  Array2<double> a;
  auto& aImpl(a.getImpl());
  a.resize(2, 3);
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 3; ++j)
      aImpl[i][j] = 1 + i + j;

  print(std::cout, aImpl);
  std::cout << std::endl;

  auto b(a);
  auto& bImpl(b.getImpl());

  std::for_each(bImpl.origin(), bImpl.origin() + b.size(), [](double& d) {d *= 0.5;});

  print(std::cout, bImpl);
  std::cout << std::endl;

  for (int k = 0; k < a.size(); ++k)
  {
    double q1 = a[k];
    double q2 = b[k];
    double q = q1 / q2;
    EXPECT_EQ(2.0, q);
  }
}
