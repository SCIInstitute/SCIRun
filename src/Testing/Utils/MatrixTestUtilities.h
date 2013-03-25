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

#ifndef TESTING_UTIL_MATRIXTESTUTILITIES
#define TESTING_UTIL_MATRIXTESTUTILITIES 1

#include <gtest/gtest.h>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/timer.hpp>
#include <stdexcept>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Utils/StringUtil.h>

#include <Testing/Utils/Share.h>

namespace SCIRun 
{ 

namespace TestUtils
{

//inline long long count_single_value(const Core::Datatypes::Matrix& m, const double val)
//{
//  return static_cast<long long>(std::count(m.begin(), m.end(), val));
//}

inline bool equal_size(const Core::Datatypes::Matrix& m1, const Core::Datatypes::Matrix& m2)
{
  return m1.nrows() == m2.nrows() 
    && m1.ncols() == m2.ncols();
}

//inline bool compare_exactly(const Core::Datatypes::Matrix& m1, const Core::Datatypes::Matrix& m2)
//{
//  return equal_size(m1, m2) &&
//    std::equal(m1.begin(), m1.end(), m2.begin());
//}

#define DEFAULT_MATRIX_PERCENT_TOLERANCE 1e-5

//TODO add tolerance parameter, improve error reporting
//inline bool compare_with_tolerance(const Core::Datatypes::Matrix& m1, const Core::Datatypes::Matrix& m2, double percentTolerance = DEFAULT_MATRIX_PERCENT_TOLERANCE)
//{
//  using namespace boost::test_tools;
//  return equal_size(m1, m2) &&
//    std::equal(m1.begin(), m1.end(), m2.begin(),
//    close_at_tolerance<double>(percent_tolerance(percentTolerance)));
//}
//
//inline ::testing::AssertionResult compare_with_tolerance_readable(const Core::Datatypes::Matrix& m1, const Core::Datatypes::Matrix& m2, double percentTolerance, int printSize = 50)
//{
//  return compare_with_tolerance(m1, m2, percentTolerance) ? 
//    ::testing::AssertionSuccess() :
//  ::testing::AssertionFailure() << "Matrix 1: \n"<< matrix_to_string(m1).substr(0, printSize) << "\nMatrix 2: \n" << matrix_to_string(m2).substr(0, printSize);
//}

//inline double relative_infinity_norm(const ColumnMatrix& x, const ColumnMatrix& xhat)
//{
//  ColumnMatrix diff = xhat - x;
//  double num = diff.infinity_norm();
//  double denom = xhat.infinity_norm();
//
//  return num / denom; 
//}
//
//inline ::testing::AssertionResult compare_with_relative_infinity_norm(const ColumnMatrix& x, const ColumnMatrix& xhat, double relativeError = DEFAULT_MATRIX_PERCENT_TOLERANCE, int printSize = 50)
//{
//  return relative_infinity_norm(x, xhat) < relativeError ? 
//    ::testing::AssertionSuccess() :
//  ::testing::AssertionFailure() << "ColumnMatrix 1: \n"<< matrix_to_string(x).substr(0, printSize) << "ColumnMatrix 2: \n" << matrix_to_string(xhat).substr(0, printSize);
//}
//
inline ::testing::AssertionResult compare_with_two_norm(const Core::Datatypes::DenseColumnMatrix& x, const Core::Datatypes::DenseColumnMatrix& xhat, double error = 1e-15, int printSize = 50)
{
  double delta = (x - xhat).norm();
  return delta < error ? 
    ::testing::AssertionSuccess() :
  ::testing::AssertionFailure() <<
    "Vectors are " << delta << " apart, expect less than " << error << " distance apart.\n" <<
    "ColumnMatrix 1: \n"<< Core::to_string(x).substr(0, printSize) << "ColumnMatrix 2: \n" << Core::to_string(xhat).substr(0, printSize);
}

inline void copyDenseToSparse(const Core::Datatypes::DenseMatrix& from, Core::Datatypes::SparseRowMatrix& to)
{
  to.setZero();
  for (int i = 0; i < from.rows(); ++i)
    for (int j = 0; j < from.cols(); ++j)
      if (fabs(from(i,j)) > 1e-10)
        to.insert(i,j) = from(i,j);
}
 
//TODO improve failure reporting
//#define EXPECT_MATRIX_EQ_TOLERANCE(actual, expected, tolerance) EXPECT_TRUE(compare_with_tolerance_readable((actual), (expected), (tolerance)))
//#define EXPECT_MATRIX_EQ(actual, expected) EXPECT_MATRIX_EQ_TOLERANCE((actual), (expected), DEFAULT_MATRIX_PERCENT_TOLERANCE)
//#define EXPECT_MATRIX_EQ_TO(actual, expected) EXPECT_MATRIX_EQ((actual), MAKE_DENSE_MATRIX(expected))
//
//#define EXPECT_SPARSE_ROW_MATRIX_EQ_TO(actual, expected) EXPECT_MATRIX_EQ((actual), MAKE_SPARSE_ROW_MATRIX(expected))
//
//#define EXPECT_COLUMN_MATRIX_EQ_TO(actual, expected) EXPECT_MATRIX_EQ((actual), MAKE_COLUMN_MATRIX(expected))
//
//#define EXPECT_COLUMN_MATRIX_EQ_BY_RELATIVE_INFINITY_NORM(actual, expected, error) EXPECT_TRUE(compare_with_relative_infinity_norm((actual), (expected), (error)))
#define EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(actual, expected, error) EXPECT_TRUE(compare_with_two_norm((actual), (expected), (error)))
//#define EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM_WITHIN_MACHINE_EPSILON(actual, expected) EXPECT_TRUE(compare_with_two_norm((actual), (expected)))
//
//#define EXPECT_MATRIX_NOT_EQ_TOLERANCE(actual, expected, tolerance) EXPECT_FALSE(compare_with_tolerance_readable((actual), (expected), (tolerance)))
//#define EXPECT_MATRIX_NOT_EQ(actual, expected) EXPECT_MATRIX_NOT_EQ_TOLERANCE((actual), (expected), DEFAULT_MATRIX_PERCENT_TOLERANCE)



struct SCISHARE ScopedTimer
{
  explicit ScopedTimer(const std::string& name) : name_(name) 
  {
    std::cout << "Starting timer " << name_ << std::endl;
    t_.restart();
  }

  ~ScopedTimer()
  {
    double elapsed = t_.elapsed();
    std::cout << "Timer " << name_ << " stopped at " << elapsed << " seconds." << std::endl;
  }

  std::string name_;
  boost::timer t_;
};

}

}

#endif
