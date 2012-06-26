/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace boost::numeric::ublas;

typedef matrix<double> MatrixInternal;

namespace
{
  MatrixInternal matrix1()
  {
    MatrixInternal m (3, 3);
    for (unsigned i = 0; i < m.size1 (); ++ i)
      for (unsigned j = 0; j < m.size2 (); ++ j)
        m (i, j) = 3 * i + j;
    return m;
  }
  
  MatrixInternal matrix2()
  {
    MatrixInternal m (2, 2);
    for (unsigned i = 0; i < m.size1 (); ++ i)
      for (unsigned j = 0; j < m.size2 (); ++ j)
        m (i, j) = 2 * i + j;
    return m;
  }

  const MatrixInternal Zero(zero_matrix<double>(3,3));
}

bool operator==(const MatrixInternal& m, const MatrixInternal& n)
{
  bool returnValue = 
    (m.size1() == n.size1()) &&
    (m.size2() == n.size2());

  if (returnValue)
  {
    for (size_t i = 0; returnValue && i < m.size1(); ++i)
    {
      for (size_t j = 0; returnValue && j < m.size2(); ++j)
      {
        returnValue &= m(i,j) == n(i,j);
      }
    }
  }
  return returnValue;
}

std::ostream& operator<<(std::ostream& o, const MatrixInternal& m)
{
  for (size_t i = 0; i < m.size1(); ++i)
  {
    for (size_t j = 0; j < m.size2(); ++j)
    {
      o << m(i,j) << " ";
    }
    o << "\n";
  }
  return o;
}

std::string matrix_to_string(const MatrixInternal& m)
{
  std::ostringstream o;
  o << m;
  return o.str();
}

//#define print
#ifdef print
#define PRINT_MATRIX(x) std::cout << #x << " = \n" << (x) << std::endl
#else
#define PRINT_MATRIX
#endif

TEST(MatrixTest, CanCreateBasicMatrix)
{
  MatrixInternal m(matrix1());
  PRINT_MATRIX(m);
}

TEST(MatrixTest, CanPrintInLegacyFormat)
{
  MatrixInternal m(matrix1());
  std::string legacy = matrix_to_string(0.5 * m);
  std::cout << legacy << std::endl;
  EXPECT_EQ("0 0.5 1 \n1.5 2 2.5 \n3 3.5 4 \n", legacy);
}

TEST(MatrixUnaryOperationTests, CanNegate)
{
  MatrixInternal m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(-m);

  MatrixInternal n = - -m;
  EXPECT_TRUE(m == n);
  EXPECT_TRUE(m + (-m) == Zero);
}

TEST(MatrixUnaryOperationTests, CanScalarMultiply)
{
  MatrixInternal m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(2*m);
  PRINT_MATRIX(m*2);
  EXPECT_TRUE(2*m == m*2);
}

TEST(MatrixUnaryOperationTests, CanTranspose)
{
  MatrixInternal m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(trans(m));

  EXPECT_TRUE(m == trans(trans(m)));
}

TEST(MatrixBinaryOperationTests, CanMultiply)
{
  MatrixInternal m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(prod(m, m));
  EXPECT_TRUE(prod(Zero, m) == Zero);
}

TEST(MatrixBinaryOperationTests, CanAdd)
{
  MatrixInternal m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m + m);
  EXPECT_TRUE(m + m == 2*m);
}

TEST(MatrixBinaryOperationTests, CanSubtract)
{
  MatrixInternal m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m - m);
  EXPECT_TRUE(m - m == Zero);
}

TEST(MatrixBinaryOperationTests, WhatHappensWhenYouAddDifferentSizes)
{
  MatrixInternal sum = matrix1() + matrix2();
  std::cout << sum.size1() << std::endl;
  std::cout << sum.size2() << std::endl;
  PRINT_MATRIX(sum);
}