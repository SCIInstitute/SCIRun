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
#include <boost/thread/thread.hpp>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;
using namespace SCIRun;
using namespace ::testing;

namespace
{
  const int size = 1000;
  SparseRowMatrixHandle matrix1() 
  {
    SparseRowMatrixHandle m(new SparseRowMatrix(size,size));
    m->insert(0,0) = 1;
    m->insert(1,2) = -1;
    m->insert(size-1,size-1) = 2;
    return m;
  }

  SparseRowMatrix matrix2() 
  {
    SparseRowMatrix m(size,size);
    m.insert(0,1) = -2;
    m.insert(1,1) = 0.5;
    m.insert(size-1,size-1) = 4;
    return m;
  }

  SparseRowMatrix matrix3() 
  {
    SparseRowMatrix m(size,size);
    m.insert(0,0) = -1;
    m.insert(1,2) = 1;
    m.insert(size-1,size-1) = -2;
    return m;
  }

  DenseColumnMatrixHandle vector1()
  {
    DenseColumnMatrixHandle v(new DenseColumnMatrix(size));
    v->setZero();
    *v << 1, 2, 4;
    (*v)[size-1] = -1;
    return v;
  }

  DenseColumnMatrixHandle vector2()
  {
    DenseColumnMatrixHandle v(new DenseColumnMatrix(size));
    v->setZero();
    *v << -1, -2, -4;
    (*v)[size-1] = 1;
    return v;
  }

  DenseColumnMatrixHandle vector3()
  {
    DenseColumnMatrixHandle v(new DenseColumnMatrix(size));
    v->setZero();
    *v << 0, 1, 0;
    (*v)[size-1] = -7;
    return v;
  }

  unsigned int numProcs()
  {
    return boost::thread::hardware_concurrency();
  }

  SolverInputs getDummySystem()
  {
    SolverInputs system;
    system.A = matrix1();
    system.b = vector1();
    system.x = vector2();
    system.x0 = vector3();
    return system;
  }
}

TEST(ParallelLinearAlgebraTests, CanCreateEmptyParallelVector)
{
  ParallelLinearAlgebraSharedData data(getDummySystem());
  ParallelLinearAlgebra pla(data, 0, 1);

  ParallelLinearAlgebra::ParallelVector v;
  EXPECT_TRUE(pla.new_vector(v));

  EXPECT_EQ(size, v.size_);
}

TEST(ParallelLinearAlgebraTests, CanCreateParallelVectorFromVectorAsShallowReference)
{
  ParallelLinearAlgebraSharedData data(getDummySystem());
  ParallelLinearAlgebra pla(data, 0, 1);

  ParallelLinearAlgebra::ParallelVector v;
  auto v1 = vector1();
  EXPECT_TRUE(pla.add_vector(v1, v));

  EXPECT_EQ(v1->nrows(), v.size_);
  for (size_t i = 0; i < size; ++i)
    EXPECT_EQ((*v1)[i], v.data_[i]);

  EXPECT_EQ(0, (*v1)[100]);
  v.data_[100]++;
  EXPECT_EQ(1, (*v1)[100]);
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
  ParallelLinearAlgebraSharedData data(getDummySystem());
  ParallelLinearAlgebra pla(data, 1, 1);

  //pla.max()


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
