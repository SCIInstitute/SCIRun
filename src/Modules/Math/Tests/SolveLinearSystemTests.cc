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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Math/SolveLinearSystem.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

using namespace SCIRun::Testing;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class SolveLinearSystemModuleTest : public ModuleTest
{
};

TEST_F(SolveLinearSystemModuleTest, ThrowsForNullMatrices)
{
  auto sls = makeModule("SolveLinearSystem");
  MatrixHandle nullMatrix, nullColumnMatrix;
  stubPortNWithThisData(sls, 0, nullMatrix);
  stubPortNWithThisData(sls, 1, nullColumnMatrix);

  EXPECT_THROW(sls->execute(), NullHandleOnPortException);
}

TEST_F(SolveLinearSystemModuleTest, ThrowsForNonsparseLHS)
{
  auto sls = makeModule("SolveLinearSystem");
  MatrixHandle lhs(new DenseMatrix(DenseMatrix::Identity(3,3)));
  MatrixHandle rhs(new DenseColumnMatrix(3));

  stubPortNWithThisData(sls, 0, lhs);
  stubPortNWithThisData(sls, 1, rhs);

  EXPECT_THROW(sls->execute(), AlgorithmInputException);
}

TEST_F(SolveLinearSystemModuleTest, CanSolveSimple)
{
  UseRealAlgorithmFactory f;

  auto sls = makeModule("SolveLinearSystem");
  SparseRowMatrixHandle lhs(new SparseRowMatrix(3,3));
  lhs->insert(0,0) = lhs->insert(1,1) = lhs->insert(2,2) = 1;
  lhs->makeCompressed();
  DenseColumnMatrixHandle rhs(new DenseColumnMatrix(3));
  (*rhs)[0] = 1;

  stubPortNWithThisData(sls, 0, lhs);
  stubPortNWithThisData(sls, 1, rhs);

  sls->execute();
}
