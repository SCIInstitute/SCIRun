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
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Eigen/Sparse>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;
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
  //x = VectorXd::Random(n);
  int i = 0;
  do {
    //std::cout << "i = " << i << std::endl;
    //std::cout << "guess\n" << x << std::endl;
    x = cg.solveWithGuess(rhs, x);
    //std::cout << "new guess\n" << x << std::endl;
    //std::cout << "-------------" << std::endl;
    
    std::cout << i << " : " << cg.error() << std::endl;
    ++i;
  } while (cg.info() != Success && i < 2000);
#endif

  std::cout << "#iterations:     " << cg.iterations() << std::endl;
  std::cout << "estimated error: " << cg.error()      << std::endl;

  std::cout << x << std::endl;

  EXPECT_EQ(v, x);

  //EXPECT_TRUE(false);
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

  ASSERT_TRUE(x);
  EXPECT_EQ(v, *x);
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

  ASSERT_TRUE(x);
  EXPECT_EQ(v, *x);
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

  const std::string file = "E:\\sp2.mat";
  std::string matStr = converter.readFile(file);

  //std::cout << matStr << std::endl;
  //2 3 4 {8 0 2 4 }{8 0 2 0 1 }{1 3.5 -1 2 }}

  std::string contents = converter.getMatrixContentsLine(matStr).get_value_or("");
  //std::cout << contents << std::endl;

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

  //std::cout << *mat << std::endl;

  DenseMatrix a(2, 3);
  a << 1, 0, 3.5,
    -1, 2, 0;

  //std::cout << a << std::endl;

  //TODO: compare dense and sparse
  //EXPECT_EQ(a, *mat);

  //EXPECT_TRUE(false);
}

TEST(EigenSparseSolverTest, CanSolveTinySystem)
{
  //Eigen::MatrixXd a(4, 3);
  //a << 1.0,2.,3.,
  //  1.,4,5,
  //  5.,6,7,
  //  4,0.1,0.1;

  //std::cout << a << std::endl;

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

  //std::cout << mat << std::endl;

  Eigen::VectorXd x(n), b(n);
  // fill A and b
  b.setZero();
  b(1) = 1;
  //std::cout << b << std::endl;

  Eigen::ConjugateGradient<Eigen::SparseMatrix<double> > cg;
  cg.compute(mat);
  x = cg.solve(b);
  //std::cout << x << std::endl;
  std::cout << "#iterations:     " << cg.iterations() << std::endl;
  std::cout << "estimated error: " << cg.error()      << std::endl;


  //EXPECT_TRUE(false);
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

TEST(EigenSparseSolverTest, CanSolveBigSystem)
{
  const std::string AFile = "C:\\Dev\\Dropbox\\CGDarrell\\A_txt.mat";
  const std::string rhsFile = "C:\\Dev\\Dropbox\\CGDarrell\\RHS_text.txt";
  EigenMatrixFromScirunAsciiFormatConverter converter;
  auto A = converter.makeSparse(AFile);

  const int n = A->cols();
  Eigen::VectorXd x(n);
  x.setZero();
  auto b = converter.makeDense(rhsFile);

  {
    ScopedTimer t("Solving block.");
    Eigen::ConjugateGradient<Eigen::SparseMatrix<double> > cg;
    cg.compute(*A);
    std::cout << "done precomputing." << std::endl;
    cg.setTolerance(0.05);
    //cg.setMaxIterations(2000);
    //x = cg.solve(*b);
    //std::cout << x << std::endl;

    cg.setMaxIterations(1);
    const int iters = 2000;
    int i = 0;
    do {
      x = cg.solveWithGuess(*b, x);
      std::cout << i << " : " << cg.error() << std::endl;
      ++i;
    } while (cg.info() != Eigen::Success && i < iters);

    std::cout << "#iterations:     " << cg.iterations() << std::endl;
    std::cout << "estimated error: " << cg.error()      << std::endl;
  }

  {
    ScopedTimer t("comparing solutions.");
    const std::string xFileEigen = "C:\\Dev\\Dropbox\\CGDarrell\\xEigen.txt";
    std::ofstream output(xFileEigen);
    output << x << std::endl;

    const std::string xFileScirun = "C:\\Dev\\Dropbox\\CGDarrell\\xScirun.mat";
    auto xExpected = converter.makeDense(xFileScirun);
    ASSERT_TRUE(xExpected);
    EXPECT_EQ(428931, xExpected->rows());
    EXPECT_EQ(1, xExpected->cols());

    EXPECT_EQ(*xExpected, x /*, .1)*/);
    EXPECT_EQ(*xExpected, x /*, .01)*/);
    EXPECT_EQ(*xExpected, x /*, .001)*/);
    EXPECT_EQ(*xExpected, x /*, .0001)*/);
    //EXPECT_TRUE(eigenMatricesEqual(*xExpected, x, .00001));
  }

  //EXPECT_TRUE(false);
}