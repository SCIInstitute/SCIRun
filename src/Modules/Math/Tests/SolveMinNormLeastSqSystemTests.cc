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
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Modules/Legacy/Math/SolveMinNormLeastSqSystem.h>

using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;

class SolveMinNormLeastSqSystemTests : public ModuleTest
{

};

TEST_F(SolveMinNormLeastSqSystemTests, ThrowsOnNullInput)
{
  auto sls = makeModule("SolveMinNormLeastSqSystem");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(sls, 0, nullMatrix);

  EXPECT_THROW(sls->execute(), NullHandleOnPortException);
}

TEST_F(SolveMinNormLeastSqSystemTests, BasicTest)
{
  /// @todo: when algo is extracted, this test needs the real factory:
  //UseRealAlgorithmFactory f;

  auto sls = makeModule("SolveMinNormLeastSqSystem");
  DenseColumnMatrixHandle v1(new DenseColumnMatrix(3));
  DenseColumnMatrixHandle v2(new DenseColumnMatrix(3));
  DenseColumnMatrixHandle v3(new DenseColumnMatrix(3));
  DenseColumnMatrixHandle target(new DenseColumnMatrix(3));
  stubPortNWithThisData(sls, 0, v1);
  stubPortNWithThisData(sls, 1, v2);
  stubPortNWithThisData(sls, 2, v3);
  stubPortNWithThisData(sls, 3, target);

  sls->execute();

  /// @todo: waiting on getData() to enable
  if (false)
  {
    auto weight = getDataOnThisOutputPort(sls, 0);
    ASSERT_TRUE(weight.get() != nullptr);
    auto result = getDataOnThisOutputPort(sls, 1);
    ASSERT_TRUE(result.get() != nullptr);
  }
}

TEST_F(SolveMinNormLeastSqSystemTests, SizesMustMatch)
{
  auto sls = makeModule("SolveMinNormLeastSqSystem");
  DenseColumnMatrixHandle v1(new DenseColumnMatrix(3));
  DenseColumnMatrixHandle v2(new DenseColumnMatrix(3));
  DenseColumnMatrixHandle v3(new DenseColumnMatrix(2));
  DenseColumnMatrixHandle target(new DenseColumnMatrix(3));
  stubPortNWithThisData(sls, 0, v1);
  stubPortNWithThisData(sls, 1, v2);
  stubPortNWithThisData(sls, 2, v3);
  stubPortNWithThisData(sls, 3, target);

  EXPECT_THROW(sls->execute(), AlgorithmInputException);
}
