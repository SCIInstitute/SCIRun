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
#include <gmock/gmock.h>

#include <fstream>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Eigen/Sparse>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Algorithms/DataIO/EigenMatrixFromScirunAsciiFormatConverter.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::DataIO::internal;
using namespace SCIRun::TestUtils;
using namespace SCIRun;
using namespace ::testing;

namespace
{
  void copyDenseToSparse(const DenseMatrix& from, SparseRowMatrix& to)
  {
    to.setZero();
    for (int i = 0; i < from.rows(); ++i)
      for (int j = 0; j < from.cols(); ++j)
        if (fabs(from(i,j)) > 1e-10)
          to.insert(i,j) = from(i,j);
  }
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, CanSolveBasicSmallDenseSystemWithEigenClasses)
{ 
  int n = 3;
  DenseMatrix m1(n,n);
  m1 << 2,-1,0,
    -1,2,-1,
    0,-1,2;

  using namespace Eigen;
  VectorXd v(n);
  v << 1,2,3;
  std::cout << "expected solution = \n" << v << std::endl;

  auto rhs = m1*v;
  std::cout << "rhs = \n" << rhs << std::endl;
 
  VectorXd x(n);
  x.setZero();
  //SparseRowMatrix A(n,n);
  //copyDenseToSparse(m1, A);

  std::cout << "matrix to solve: \n" << m1 << std::endl;
  
  ConjugateGradient<DenseMatrix::EigenBase> cg;
  cg.compute(m1);

  ASSERT_TRUE(cg.info() == Success);

  cg.setTolerance(1e-15);
  x = cg.solve(rhs);

  #if 0
cg.setMaxIterations(1);
  int i = 0;
  do {
    x = cg.solveWithGuess(rhs, x);
    
    std::cout << i << " : " << cg.error() << std::endl;
    ++i;
  } while (cg.info() != Success && i < 2000);
#endif

  std::cout << "#iterations:     " << cg.iterations() << std::endl;
  std::cout << "estimated error: " << cg.error()      << std::endl;

  std::cout << x << std::endl;

  EXPECT_EQ(v, x);
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, CanSolveBasicSmallDenseSystem)
{
  int n = 3;
  DenseMatrixHandle A(new DenseMatrix(n,n));
  *A << 2,-1,0,
    -1,2,-1,
    0,-1,2;

  DenseColumnMatrix v(n);
  v << 1,2,3;
  std::cout << "expected solution = \n" << v << std::endl;

  DenseColumnMatrixHandle rhs(new DenseColumnMatrix(*A * v));
  std::cout << "rhs = \n" << *rhs << std::endl;

  SolveLinearSystemAlgorithm algo;

  auto x = algo.run(boost::make_tuple(A, rhs), boost::make_tuple(1e-15, 10));

  ASSERT_TRUE(x.get<0>());
  EXPECT_EQ(v, *x.get<0>());
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, CanSolveBasicSmallSparseSystem)
{
  int n = 3;
  DenseMatrix Adense(n,n);
  Adense << 2,-1,0,
    -1,2,-1,
    0,-1,2;

  SparseRowMatrixHandle A(new SparseRowMatrix(n,n));
  copyDenseToSparse(Adense, *A);

  DenseColumnMatrix v(n);
  v << 1,2,3;
  std::cout << "expected solution = \n" << v << std::endl;

  DenseColumnMatrixHandle rhs(new DenseColumnMatrix(*A * v));
  std::cout << "rhs = \n" << *rhs << std::endl;

  SolveLinearSystemAlgorithm algo;

  auto x = algo.run(boost::make_tuple(A, rhs), boost::make_tuple(1e-15, 10));

  ASSERT_TRUE(x.get<0>());
  EXPECT_EQ(v, *x.get<0>());
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, ThrowsOnNullMatrix)
{
  int n = 3;
  DenseMatrixHandle A;

  DenseColumnMatrixHandle rhs(new DenseColumnMatrix(3));

  SolveLinearSystemAlgorithm algo;

  EXPECT_THROW(algo.run(boost::make_tuple(A, rhs), boost::make_tuple(1e-15, 10)), AlgorithmInputException);
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, ThrowsOnNullRHS)
{
  int n = 3;
  DenseMatrixHandle A(new DenseMatrix(n,n));
  *A << 2,-1,0,
    -1,2,-1,
    0,-1,2;

  DenseColumnMatrixHandle rhs;

  SolveLinearSystemAlgorithm algo;

  EXPECT_THROW(algo.run(boost::make_tuple(A, rhs), boost::make_tuple(1e-15, 10)), AlgorithmInputException);
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, ThrowsOnNegativeTolerance)
{
  int n = 3;
  DenseMatrixHandle A(new DenseMatrix(n,n));
  *A << 2,-1,0,
    -1,2,-1,
    0,-1,2;

  DenseColumnMatrix v(n);
  v << 1,2,3;

  DenseColumnMatrixHandle rhs(new DenseColumnMatrix(*A * v));

  SolveLinearSystemAlgorithm algo;

  EXPECT_THROW(algo.run(boost::make_tuple(A, rhs), boost::make_tuple(-4, 10)), AlgorithmInputException);
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, ThrowsOnNegativeMaxIterations)
{
  int n = 3;
  DenseMatrixHandle A(new DenseMatrix(n,n));
  *A << 2,-1,0,
    -1,2,-1,
    0,-1,2;

  DenseColumnMatrix v(n);
  v << 1,2,3;

  DenseColumnMatrixHandle rhs(new DenseColumnMatrix(*A * v));

  SolveLinearSystemAlgorithm algo;

  EXPECT_THROW(algo.run(boost::make_tuple(A, rhs), boost::make_tuple(1e-15, -1)), AlgorithmInputException);
}

TEST(SparseMatrixReadTest, RegexOfScirun4Format)
{
  EigenMatrixFromScirunAsciiFormatConverter converter;

  const std::string file = "E:\\stuff\\sp2.mat";
  std::string matStr = converter.readFile(file);

  //2 3 4 {8 0 2 4 }{8 0 2 0 1 }{1 3.5 -1 2 }}

  std::string contents = converter.getMatrixContentsLine(matStr).get_value_or("");

  ASSERT_EQ("2 3 4 {8 0 2 4 }{8 0 2 0 1 }{1 3.5 -1 2 }}", contents);

  auto rawOpt = converter.parseSparseMatrixString(contents);
  ASSERT_TRUE(rawOpt);
  auto raw = rawOpt.get();

  EXPECT_EQ("2", raw.get<0>());
  EXPECT_EQ("3", raw.get<1>());
  EXPECT_EQ("4", raw.get<2>());
  EXPECT_EQ("0 2 4 ", raw.get<3>());
  EXPECT_EQ("0 2 0 1 ", raw.get<4>());
  EXPECT_EQ("1 3.5 -1 2 ", raw.get<5>());

  auto data = converter.convertRaw(raw);
  EXPECT_EQ(2, data.get<0>());
  EXPECT_EQ(3, data.get<1>());
  EXPECT_EQ(4, data.get<2>());
  EXPECT_THAT(data.get<3>(), ElementsAre(0, 2, 4));
  EXPECT_THAT(data.get<4>(), ElementsAre(0, 2, 0, 1));
  EXPECT_THAT(data.get<5>(), ElementsAre(1.0, 3.5, -1.0, 2.0));

  auto mat = converter.makeSparse(file);
  ASSERT_TRUE(mat);
  EXPECT_EQ(2, mat->rows());
  EXPECT_EQ(3, mat->cols());

  DenseMatrix a(2, 3);
  a << 1, 0, 3.5,
    -1, 2, 0;

  //TODO: compare dense and sparse
  //EXPECT_EQ(a, *mat);
  EXPECT_EQ(to_string(a), to_string(mat->castForPrinting()));
}

TEST(EigenSparseSolverTest, CanSolveTinySystem)
{
  typedef Eigen::Triplet<double> T;
  std::vector<T> tripletList;
  int estimation_of_entries = 10;
  tripletList.reserve(estimation_of_entries);
  for(int i = 0; i < estimation_of_entries; ++i)
  {
    tripletList.push_back(T(i,i, i *3 + 1));
  }
  int n = 10;
  Eigen::SparseMatrix<double> mat(n,n);
  mat.setFromTriplets(tripletList.begin(), tripletList.end());

  Eigen::VectorXd x(n), b(n);
  // fill A and b
  b.setZero();
  b(1) = 1;

  Eigen::ConjugateGradient<Eigen::SparseMatrix<double> > cg;
  cg.compute(mat);
  x = cg.solve(b);
  std::cout << "#iterations:     " << cg.iterations() << std::endl;
  std::cout << "estimated error: " << cg.error()      << std::endl;
}

TEST(SparseMatrixReadTest, DISABLED_CanReadInBigMatrix)
{
  const std::string file = "C:\\Dev\\Dropbox\\CGDarrell\\A_txt.mat ";
  EigenMatrixFromScirunAsciiFormatConverter converter;
  auto mat = converter.makeSparse(file);
  ASSERT_TRUE(mat);
  EXPECT_EQ(428931, mat->rows());
  EXPECT_EQ(428931, mat->cols());
}

TEST(SparseMatrixReadTest, DISABLED_CanReadInBigVector)
{
  //428931 1 {0 0.005436646179877679 -0.002975964005526226
  const std::string file = "C:\\Dev\\Dropbox\\CGDarrell\\RHS_text.txt";
  EigenMatrixFromScirunAsciiFormatConverter converter;
  auto mat = converter.makeDense(file);
  ASSERT_TRUE(mat);
  EXPECT_EQ(428931, mat->rows());
  EXPECT_EQ(1, mat->cols());
}

TEST(EigenSparseSolverTest, DISABLED_CanSolveBigSystem)
{
  const std::string AFile = "e:\\stuff\\CGDarrell\\A_txt.mat";
  const std::string rhsFile = "e:\\stuff\\CGDarrell\\RHS_text.txt";
  EigenMatrixFromScirunAsciiFormatConverter converter;
  auto A = converter.make(AFile);
  ASSERT_TRUE(A);

  std::cout << A->nrows() << " x " << A->ncols() << std::endl;

  //const int n = A->ncols();
  auto b = converter.make(rhsFile); 
  ASSERT_TRUE(b);
  std::cout << b->nrows() << " x " << b->ncols() << std::endl;
  auto bCol = matrix_convert::to_column(b);
  
  SolveLinearSystemAlgorithm::Outputs x;
  {
    ScopedTimer t("using algorithm object");
    SolveLinearSystemAlgorithm algo;

    x = algo.run(boost::make_tuple(A, bCol), boost::make_tuple(1e-20, 4000));

    ASSERT_TRUE(x.get<0>());
    std::cout << "error: " << x.get<1>() << std::endl;
    std::cout << "iterations: " << x.get<2>() << std::endl;
  }

  {
    ScopedTimer t("comparing solutions.");
    const std::string xFileEigen = "e:\\stuff\\CGDarrell\\xEigenNEW.txt";
    std::ofstream output(xFileEigen);
    auto solution = *x.get<0>();
    output << std::setprecision(15) << solution << std::endl;

    const std::string xFileScirun = "e:\\stuff\\CGDarrell\\xScirunColumn.mat";
    auto xExpected = converter.makeColumn(xFileScirun);
    ASSERT_TRUE(xExpected);
    EXPECT_EQ(428931, xExpected->nrows());
    EXPECT_EQ(1, xExpected->ncols());

    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .1);
    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .01);
    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .001);
    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .0001);
  }
}