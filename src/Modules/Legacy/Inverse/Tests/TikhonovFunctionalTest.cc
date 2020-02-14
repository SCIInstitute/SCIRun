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
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithStandardTikhonovImpl.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules;
//  using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules::Inverse;
using namespace SCIRun::Core::Algorithms::Inverse;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using ::testing::Values;
using ::testing::Combine;
using ::testing::Range;


class TikhonovFunctionalTest : public ModuleTest
{
protected:
    UseRealModuleStateFactory f;
};

/// -------- INPUTS TESTS ------------ ///
/*
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

// ID squared fwd matrix + RAND measured data
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
TEST_F(TikhonovFunctionalTest, loadIDNonSquareFwdMatrixANDRandData)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(3, 4)));    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(3, 1)));   // measurement data (rand)

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
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - different sizes
//TODO: waiting on text fix from @jcollfont
TEST_F(TikhonovFunctionalTest, DISABLED_loadIDSquareFwdMatrixANDRandDataDiffSizes)
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
TEST_F(TikhonovFunctionalTest, loadIDNonSquareFwdMatrixANDRandDataDiffSizes)
{
  // create inputs
  auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
  MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(3, 4)));    // forward matrix (IDentityt)
  MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(5, 1)));   // measurement data (rand)

  // input data
  stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
  stubPortNWithThisData(tikAlgImp, 2, measuredData);
  // check result
  EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

//// ---------- Source Regularization Matrix Input Tests ----------- //////

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDSquaredSourceReguWithNonSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, TikhonovAlgoAbstractBase::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix non-squared option:"non-squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredSourceReguWithNonSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, TikhonovAlgoAbstractBase::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDSquaredSourceReguWithSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, TikhonovAlgoAbstractBase::AlgorithmSolutionSubcase::solution_constrained_squared);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmSolutionSubcase::solution_constrained_squared);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix non-squared option:"squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredSourceReguWithSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    Core::Algorithms::AlgorithmParameterName TikhonovSolutionSubcase;
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, 1);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, 1);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDSquaredSourceReguWithNonSquareOptionWrongSizeWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, TikhonovAlgoAbstractBase::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredSourceReguWithNonSquareOptionWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, TikhonovAlgoAbstractBase::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmSolutionSubcase::solution_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDSquaredSourceReguWithSquareOptionWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, TikhonovAlgoAbstractBase::AlgorithmSolutionSubcase::solution_constrained_squared);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmSolutionSubcase::solution_constrained_squared);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredSourceReguWithSquareOptionWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovSolutionSubcase, TikhonovAlgoAbstractBase::AlgorithmSolutionSubcase::solution_constrained_squared);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovSolutionSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmSolutionSubcase::solution_constrained_squared);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 1, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}


//// ---------- Measurement (residual) Regularization Matrix Input Tests ----------- //////

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDSquaredMeasurementReguWithNonSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix non-squared option:"non-squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredMeasurementReguWithNonSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDSquaredMeasurementReguWithSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_NO_THROW(tikAlgImp->execute());
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix non-squared option:"squared - ok size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredMeasurementReguWithSquareOption)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 4)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDSquaredMeasurementReguWithNonSquareOptionWrongSizeWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredMeasurementReguWithNonSquareOptionWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDSquaredMeasurementReguWithSquareOptionWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(5, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}

// ID square fwd matrix + RAND measured data  - ok sizes - Regularization matrix squared option:"non-squared - wrong size
TEST_F(TikhonovFunctionalTest, loadIDNonSquaredMeasurementReguWithSquareOptionWrongSize)
{
    // create inputs
    auto tikAlgImp = makeModule("SolveInverseProblemWithTikhonov");
    MatrixHandle fwdMatrix(new DenseMatrix(DenseMatrix::Identity(4, 4)));    // forward matrix (IDentityt)
    MatrixHandle measuredData(new DenseMatrix(DenseMatrix::Random(4, 1)));   // measurement data (rand)
    MatrixHandle sourceRegularizationMatrix(new DenseMatrix(DenseMatrix::Identity(4, 5)));    // forward matrix (Identity)

    // change params
    tikAlgImp->setStateDefaults();                                                  // set default params
<<<<<<< HEAD
    tikAlgImp->get_state()->setValue(parameter_::TikhonovResidualSubcase, TikhonovAlgoAbstractBase::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
=======
    tikAlgImp->get_state()->setValue(Parameters::TikhonovResidualSubcase, BioPSE::TikhonovAlgorithmImpl::AlgorithmResidualSubcase::residual_constrained_squared);  // select single lambda
>>>>>>> 0276d4595f32390b8b41accc8c207379dd81a7d5

    // input data
    stubPortNWithThisData(tikAlgImp, 0, fwdMatrix);
    stubPortNWithThisData(tikAlgImp, 2, measuredData);
    stubPortNWithThisData(tikAlgImp, 3, sourceRegularizationMatrix);
    // check result
    EXPECT_THROW(tikAlgImp->execute(), SCIRun::Core::DimensionMismatch);
}
*/
