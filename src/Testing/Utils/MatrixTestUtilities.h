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


#ifndef TESTING_UTIL_MATRIXTESTUTILITIES
#define TESTING_UTIL_MATRIXTESTUTILITIES 1

#include <gtest/gtest.h>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/timer.hpp>
#include <boost/assign.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Utils/StringUtil.h>

#include <Testing/Utils/share.h>

namespace SCIRun
{

namespace TestUtils
{

inline bool equal_size(const Core::Datatypes::Matrix& m1, const Core::Datatypes::Matrix& m2)
{
  return m1.nrows() == m2.nrows()
    && m1.ncols() == m2.ncols();
}

#if BOOST_VERSION >= 106700
namespace btt = boost::math::fpc;
#else
namespace btt = boost::test_tools;
#endif

#define DEFAULT_MATRIX_PERCENT_TOLERANCE 1e-5

//TODO improve error reporting
inline bool compare_with_tolerance(const Core::Datatypes::DenseMatrix& m1, const Core::Datatypes::DenseMatrix& m2, double percentTolerance = DEFAULT_MATRIX_PERCENT_TOLERANCE)
{
  return equal_size(m1, m2) &&
    std::equal(m1.data(), m1.data() + m1.size(), m2.data(),
		btt::close_at_tolerance<double>(btt::percent_tolerance(percentTolerance)));
}

inline bool compare_with_tolerance(const Core::Datatypes::SparseRowMatrix& m1, const Core::Datatypes::SparseRowMatrix& m2, double percentTolerance = DEFAULT_MATRIX_PERCENT_TOLERANCE)
{
  return equal_size(m1, m2) &&
    std::equal(m1.valuePtr(), m1.valuePtr() + m1.nonZeros(), m2.valuePtr(),
		btt::close_at_tolerance<double>(btt::percent_tolerance(percentTolerance)));
}

inline ::testing::AssertionResult compare_with_tolerance_readable(const Core::Datatypes::DenseMatrix& m1, const Core::Datatypes::DenseMatrix& m2, double percentTolerance, int printSize = 50)
{
  return compare_with_tolerance(m1, m2, percentTolerance) ?
    ::testing::AssertionSuccess() :
  ::testing::AssertionFailure() << "Matrix 1: \n"<< matrix_to_string(m1).substr(0, printSize) << "\nMatrix 2: \n" << matrix_to_string(m2).substr(0, printSize);
}


inline ::testing::AssertionResult compare_with_tolerance_readable(const Core::Datatypes::SparseRowMatrix& m1, const Core::Datatypes::SparseRowMatrix& m2, double percentTolerance, int printSize = 50)
{
  return compare_with_tolerance(m1, m2, percentTolerance) ?
    ::testing::AssertionSuccess() :
    ::testing::AssertionFailure() << "Matrix 1: \n" << sparse_matrix_to_string(m1).substr(0, printSize) << "\nMatrix 2: \n" << sparse_matrix_to_string(m2).substr(0, printSize);
}

inline double relative_infinity_norm(const Core::Datatypes::DenseColumnMatrix& x, const Core::Datatypes::DenseColumnMatrix& xhat)
{
  Core::Datatypes::DenseColumnMatrix diff = xhat - x;
  double num = diff.lpNorm<Eigen::Infinity>();
  double denom = xhat.lpNorm<Eigen::Infinity>();

  return num / denom;
}

inline ::testing::AssertionResult compare_with_relative_infinity_norm(const Core::Datatypes::DenseColumnMatrix& x, const Core::Datatypes::DenseColumnMatrix& xhat, double relativeError = DEFAULT_MATRIX_PERCENT_TOLERANCE, int printSize = 50)
{
  return relative_infinity_norm(x, xhat) < relativeError ?
    ::testing::AssertionSuccess() :
  ::testing::AssertionFailure() << "ColumnMatrix 1: \n"<< matrix_to_string(x).substr(0, printSize) << "ColumnMatrix 2: \n" << matrix_to_string(xhat).substr(0, printSize);
}


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

inline Core::Datatypes::SparseRowMatrixHandle toSparseHandle(const Core::Datatypes::DenseMatrix& dense)
{
  Core::Datatypes::SparseRowMatrixHandle sp(boost::make_shared<Core::Datatypes::SparseRowMatrix>(static_cast<int>(dense.rows()), static_cast<int>(dense.cols())));
  copyDenseToSparse(dense, *sp);
  sp->makeCompressed();
  return sp;
}

inline Core::Datatypes::DenseMatrixHandle makeDense(const Core::Datatypes::SparseRowMatrix& sparse)
{
  Core::Datatypes::DenseMatrixHandle dense(new Core::Datatypes::DenseMatrix(sparse.rows(), sparse.cols(), 0));
  for (int k=0; k < sparse.outerSize(); ++k)
  {
    for (Core::Datatypes::SparseRowMatrix::InnerIterator it(sparse,k); it; ++it)
    {
      (*dense)(it.row(), it.col()) = it.value();
    }
  }
  return dense;
}

//TODO improve failure reporting

#define EXPECT_MATRIX_EQ_TOLERANCE(actual, expected, tolerance) EXPECT_TRUE(compare_with_tolerance_readable((actual), (expected), (tolerance)))
#define EXPECT_MATRIX_EQ(actual, expected) EXPECT_MATRIX_EQ_TOLERANCE((actual), (expected), DEFAULT_MATRIX_PERCENT_TOLERANCE)
#define EXPECT_MATRIX_EQ_TO(actual, expected) EXPECT_MATRIX_EQ((actual), MAKE_DENSE_MATRIX(expected))
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


// Note: this approach is limited to matrices with <= 5 columns, due to the implementation of boost::assign::tuple_list_of.
template <class Tuple>
Core::Datatypes::DenseMatrix convertDataToMatrix(const std::vector<Tuple>& matrixData)
{
  typedef typename boost::tuples::element<0,Tuple>::type element0type;
  typedef boost::is_convertible<element0type, double> CanConvertToDouble;
  BOOST_STATIC_ASSERT(CanConvertToDouble::value);

  const int columns = boost::tuples::length<Tuple>::value;
  const bool tupleShouldBeAllOneType = columns * sizeof(element0type) == sizeof(Tuple);
  BOOST_STATIC_ASSERT(tupleShouldBeAllOneType);

  const size_t rows = matrixData.size();
  Core::Datatypes::DenseMatrix matrix(rows, columns);

  for (size_t i = 0; i < rows; ++i)
  {
    const element0type* tupleData = reinterpret_cast<const element0type*>(&matrixData[i]);
    std::copy(tupleData, tupleData + columns, matrix.data() + i*columns);
  }

  return matrix;
}

template <class Cont>
inline std::vector<typename Cont::value_type> to_vector(const Cont& cont)
{
  if (cont.empty())
    throw std::invalid_argument("Matrix<double> construction will not work without any data");

  std::vector<typename Cont::value_type> v;
  return cont.to_container(v);
}

#define MAKE_DENSE_MATRIX(x) (convertDataToMatrix(to_vector(boost::assign::tuple_list_of x)))

#define MAKE_DENSE_MATRIX_HANDLE(x) (boost::make_shared<SCIRun::Core::Datatypes::DenseMatrix>(MAKE_DENSE_MATRIX(x)))

#define MAKE_SPARSE_MATRIX_HANDLE(x) (toSparseHandle(MAKE_DENSE_MATRIX(x)))

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

// TODO: move to Field utils file
SCISHARE FieldHandle loadFieldFromFile(const boost::filesystem::path& filename);
SCISHARE bool compareNodes(FieldHandle expected, FieldHandle actual);

}

}

#endif
