/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


// Testing libraries
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunUnitTests.h>

// General Libraries
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

// DataType libraries
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>

// Tikhonov specific
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovImpl.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules;
//  using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::TestUtils;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;


class TikhonovFunctionalTest : public ModuleTest
{
};


// NULL fwd matrix + NULL measure data
TEST_F(TikhonovFunctionalTest, loadNullFwdMatrixANDNullData)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle nullMatrix, nullColumnMatrix;
  // input data
  stubPortNWithThisData(tikAlgImp, 0, nullMatrix);
  stubPortNWithThisData(tikAlgImp, 2, nullColumnMatrix);
  // check result
  EXPECT_THROW(tikAlgImp->execute(), NullHandleOnPortException);

}

// ID fwd matrix + null measured data
TEST_F(TikhonovFunctionalTest, loadIDFwdMatrixANDNullData)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(3, 3)));    // forward matrix (IDentityt)
  MatrixHandle nullColumnMatrix;              // measurement data (null)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, nullColumnMatrix);
  // check result
  EXPECT_THROW(tikAlgImp->execute(), NullHandleOnPortException);

}

// NULL fwd matrix + RANF measured data
TEST_F(TikhonovFunctionalTest, loadNullFwdMatrixANDRandData)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix;    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(3, 1)));    // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_THROW(tikAlgImp->execute(), NullHandleOnPortException);

}

// ID fwd matrix + RAND measured data
TEST_F(TikhonovFunctionalTest, loadIDFwdMatrixANDRandData)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(3, 3)));    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(3, 1)));   // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_NO_THROW(tikAlgImp->execute());

}

// ID non-square fwd matrix + RAND measured data  (underdetermined)
// TODO: FAILS TEST: fails test when it shouldn't. The sizes of forward matrix and data are the same
TEST_F(TikhonovFunctionalTest, loadIDNonSquareFwdMatrixANDRandData)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(3, 4)));    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID non-square fwd matrix + RAND measured data  (overdetermined)
TEST_F(TikhonovFunctionalTest, loadIDNonSquareFwdMatrixANDRandData2)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 3)));    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(3, 1)));   // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - different sizes
TEST_F(TikhonovFunctionalTest, loadIDSquareFwdMatrixANDRandDataDiffSizes)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(3, 3)));    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID non-square fwd matrix + RAND measured data  - different sizes
// TODO: FAILS TEST: does not fail test when it shouldn't. The sizes of forward matrix and data are the different (note that this is only for size(fwd,2) < size(data,1) )!
TEST_F(TikhonovFunctionalTest, loadIDNonSquareFwdMatrixANDRandDataDiffSizes)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(3, 4)));    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(3, 1)));   // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}