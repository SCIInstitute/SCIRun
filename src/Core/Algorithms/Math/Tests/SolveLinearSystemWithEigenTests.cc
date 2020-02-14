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


#include <Testing/Utils/SCIRunUnitTests.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#define register
#include <Eigen/Sparse>
#undef register
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Algorithms/DataIO/EigenMatrixFromScirunAsciiFormatConverter.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::DataIO::internal;
using namespace SCIRun::Core;
using namespace SCIRun::TestUtils;
using namespace SCIRun;
using namespace ::testing;

/// @todo: remove these Eigen tests if they are duplicates of existing SLS tests. Other tests below need to live in separate files.

#if 0
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
  DenseColumnMatrixHandle solution = x.get<0>();

  ASSERT_TRUE(solution.get() != nullptr);
  EXPECT_EQ(v, *solution);
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
  DenseColumnMatrixHandle solution = x.get<0>();

  ASSERT_TRUE(solution.get() != nullptr);
  EXPECT_EQ(v, *solution);
}

TEST(SolveLinearSystemWithEigenAlgorithmTests, ThrowsOnNullMatrix)
{
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
#endif
//todo: remove unused code
TEST(SparseMatrixReadTest, DISABLED_RegexOfScirun4Format)
{
  EigenMatrixFromScirunAsciiFormatConverter converter;

  auto file = TestResources::rootDir() / "sp2.mat";

  if (!boost::filesystem::exists(file))
  {
    FAIL() << "TODO: Issue #142 will standardize these file locations other than being on Dan's hard drive." << std::endl
        << "Once that issue is done however, this will be a user setup error." << std::endl;
    return;
  }

  std::string matStr = converter.readFile(file.string());

  //2 3 4 {8 0 2 4 }{8 0 2 0 1 }{1 3.5 -1 2 }}

  std::string contents = converter.getMatrixContentsLine(matStr).get_value_or("");

  std::string newline;
#ifndef WIN32
  newline += "\r";
#endif

  EXPECT_EQ("2 3 4 {8 0 2 4 }{8 0 2 0 1 }{1 3.5 -1 2 }}" + newline, contents);

  auto rawOpt = converter.parseSparseMatrixString(contents);
  ASSERT_TRUE(static_cast<bool>(rawOpt));
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

  auto mat = converter.makeSparse(file.string());
  ASSERT_TRUE(mat.get() != nullptr);
  EXPECT_EQ(2, mat->rows());
  EXPECT_EQ(3, mat->cols());

  DenseMatrix a(2, 3);
  a << 1, 0, 3.5,
    -1, 2, 0;

  EXPECT_EQ(a, *convertMatrix::toDense(mat));
#if !DEBUG
  EXPECT_EQ(to_string(a), to_string(mat->castForPrinting()));
#endif
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
  auto file = TestResources::rootDir() / "CGDarrell" / "A_txt.mat ";
  EigenMatrixFromScirunAsciiFormatConverter converter;
  auto mat = converter.makeSparse(file.string());
  ASSERT_TRUE(mat.get() != nullptr);
  EXPECT_EQ(428931, mat->rows());
  EXPECT_EQ(428931, mat->cols());
}

TEST(SparseMatrixReadTest, DISABLED_CanReadInBigVector)
{
  //428931 1 {0 0.005436646179877679 -0.002975964005526226
  auto file = TestResources::rootDir() / "CGDarrell" / "RHS_text.txt";
  EigenMatrixFromScirunAsciiFormatConverter converter;
  auto mat = converter.makeDense(file.string());
  ASSERT_TRUE(mat.get() != nullptr);
  EXPECT_EQ(428931, mat->rows());
  EXPECT_EQ(1, mat->cols());
}

TEST(EigenSparseSolverTest, DISABLED_CanSolveBigSystem)
{
  auto AFile = TestResources::rootDir() / "CGDarrell" / "A_txt.mat";
  auto rhsFile = TestResources::rootDir() / "CGDarrell" / "RHS_text.txt";
  EigenMatrixFromScirunAsciiFormatConverter converter;
  auto A = converter.make(AFile.string());
  ASSERT_TRUE(A.get() != nullptr);

  std::cout << A->nrows() << " x " << A->ncols() << std::endl;

  auto b = converter.make(rhsFile.string());
  ASSERT_TRUE(b.get() != nullptr);
  std::cout << b->nrows() << " x " << b->ncols() << std::endl;
  auto bCol = convertMatrix::toColumn(b);

  SolveLinearSystemAlgorithm::Outputs x;
  {
    ScopedTimer t("using algorithm object");
    SolveLinearSystemAlgorithm algo;

    x = algo.run(std::make_tuple(A, bCol), std::make_tuple(1e-20, 4000, "cg"));
    MatrixHandle solution = std::get<0>(x);

    ASSERT_TRUE(solution.get() != nullptr);
    std::cout << "error: " << std::get<1>(x) << std::endl;
    std::cout << "iterations: " << std::get<2>(x) << std::endl;
  }

  {
    ScopedTimer t("comparing solutions.");
    auto xFileEigen = TestResources::rootDir() / "CGDarrell" / "xEigenNEW.txt";
    std::ofstream output(xFileEigen.string());
    auto solution = *std::get<0>(x);
    output << std::setprecision(15) << solution << std::endl;

    auto xFileScirun = TestResources::rootDir() / "CGDarrell" / "xScirunColumn.mat";
    auto xExpected = converter.makeColumn(xFileScirun.string());
    ASSERT_TRUE(xExpected.get() != nullptr);
    EXPECT_EQ(428931, xExpected->nrows());
    EXPECT_EQ(1, xExpected->ncols());

    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .1);
    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .01);
    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .001);
    EXPECT_COLUMN_MATRIX_EQ_BY_TWO_NORM(*xExpected, solution , .0001);
  }
}
