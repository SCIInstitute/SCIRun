/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3, 3));
    for (int i = 0; i < m->rows(); ++ i)
      for (int j = 0; j < m->cols(); ++ j)
        (*m)(i, j) = 3.0 * i + j;
    return m;
  }
  const DenseMatrix Zero(DenseMatrix::Zero(3,3));
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanAdd)
{
  EvaluateLinearAlgebraBinaryAlgorithm algo;

  DenseMatrixHandle m(matrix1());
  DenseMatrixHandle result = algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(m, m), EvaluateLinearAlgebraBinaryAlgorithm::ADD);
  EXPECT_EQ(2 * *m, *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanSubtract)
{
  EvaluateLinearAlgebraBinaryAlgorithm algo;

  DenseMatrixHandle m(matrix1());
  DenseMatrixHandle result = algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(m, m), EvaluateLinearAlgebraBinaryAlgorithm::SUBTRACT);
  EXPECT_EQ(Zero, *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanMultiply)
{
  EvaluateLinearAlgebraBinaryAlgorithm algo;

  DenseMatrixHandle m(matrix1());
  DenseMatrixHandle result = algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(m, m), EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY);
  EXPECT_EQ(*m * *m, *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, NullInputThrowsException)
{
  EvaluateLinearAlgebraBinaryAlgorithm algo;

  EXPECT_THROW(algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(DenseMatrixHandle(), matrix1()), EvaluateLinearAlgebraBinaryAlgorithm::ADD), AlgorithmInputException);
  EXPECT_THROW(algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(matrix1(), DenseMatrixHandle()), EvaluateLinearAlgebraBinaryAlgorithm::ADD), AlgorithmInputException);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanUseFunction)
{
	EvaluateLinearAlgebraBinaryAlgorithm algo; 

	DenseMatrixHandle m(matrix1());
	std::string functionArg = "x+y"; 
	DenseMatrixHandle result  = algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(m, m), EvaluateLinearAlgebraBinaryAlgorithm::Parameters(EvaluateLinearAlgebraBinaryAlgorithm::FUNCTION, functionArg));
	EXPECT_EQ( *m+*m, *result); 
	
}