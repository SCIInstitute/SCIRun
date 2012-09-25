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

#include <fstream>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Eigen/Sparse>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

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