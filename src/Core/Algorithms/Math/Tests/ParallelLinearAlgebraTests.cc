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
#include <Core/Algorithms/Math/ParallelAlgebra/ParallelLinearAlgebra.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;
using namespace SCIRun;
using namespace ::testing;

namespace
{
  SparseRowMatrix matrix1() 
  {
    SparseRowMatrix m(3,3);
    m.insert(0,0) = 1;
    m.insert(1,2) = -1;
    return m;
  }

  SparseRowMatrix matrix2() 
  {
    SparseRowMatrix m(3,3);
    m.insert(0,1) = -2;
    m.insert(1,1) = 0.5;
    return m;
  }

  SparseRowMatrix matrix3() 
  {
    SparseRowMatrix m(3,3);
    m.insert(0,0) = -1;
    m.insert(1,2) = 1;
    return m;
  }

  DenseColumnMatrix vector1()
  {
    DenseColumnMatrix v(3);
    v << 1, 2, 3;
    return v;
  }

  DenseColumnMatrix vector2()
  {
    DenseColumnMatrix v(3);
    v << -1, -2, -3;
    return v;
  }

  DenseColumnMatrix vector3()
  {
    DenseColumnMatrix v(3);
    v << 0, 1, 0;
    return v;
  }
}

TEST(ParallelLinearAlgebraTests, CanCreateEmptyParallelVector)
{
  EXPECT_TRUE(false);
}

TEST(ParallelLinearAlgebraTests, CanCopyParallelVector)
{
  EXPECT_TRUE(false);
}

TEST(ParallelLinearAlgebraTests, CanCopyParallelSparseMatrix)
{
  EXPECT_TRUE(false);
}

TEST(ParallelLinearAlgebraTests, CanCopyContentsOfVector)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanTakeAbsoluteValueOfDiagonal)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanComputeMaxOfVector)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanInvertElementsOfVectorWithAbsoluteValueThreshold)
{
  EXPECT_TRUE(false);
}

TEST(ParallelLinearAlgebraTests, CanFillVectorWithOnes)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanMultiplyMatrixByVector)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanSubtractVectors)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanCompute2Norm)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanMultiplyVectorsComponentWise)
{
  EXPECT_TRUE(false);
}

TEST(ParallelArithmeticTests, CanComputeDotProduct)
{
  EXPECT_TRUE(false);
}
