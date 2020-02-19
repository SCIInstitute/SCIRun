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


#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;
using ::testing::NotNull;
using ::testing::TestWithParam;
using ::testing::Values;

class ConvertMeshToPointCloudMeshAlgoTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(true);
  }
};

// parameters:
// Location = "node"|"data"

TEST_F(ConvertMeshToPointCloudMeshAlgoTests, CanLogErrorMessage)
{
  ConvertMeshToPointCloudMeshAlgo algo;

  FieldHandle input;
  FieldHandle output;

  EXPECT_FALSE(algo.runImpl(input, output));
}

#if SCIRUN4_CODE_TO_BE_ENABLED_LATER
#if GTEST_HAS_COMBINE

/*Get Parameterized Tests
*/
using ::testing::Bool;
using ::testing::Values;
using ::testing::Combine;
class JoinFieldsAlgoTestsParameterized : public ::testing::TestWithParam < ::std::tr1::tuple<bool, bool, bool, bool, double> >
{
public:
  FieldHandle boundary_;
  JoinFieldsAlgo algo_;
  FieldHandle latVol_;
  GetDomainBoundaryTestsParameterized()
  {
    latVol_ = loadFieldFromFile(TestResources::rootDir() / "latVolWithNormData.fld");
    LogSettings::Instance().setVerbose(true);
  }

protected:
  virtual void SetUp()
  {
    ASSERT_TRUE(latVol_->vmesh()->is_latvolmesh());

    // How to set parameters on an algorithm (that come from the GUI)
    algo_.set(GetDomainBoundaryAlgo::AddOuterBoundary, ::std::tr1::get<0>(GetParam()));

    /// @todo: this logic matches the wacky module behavior
    algo_.set(GetDomainBoundaryAlgo::UseRange, ::std::tr1::get<1>(GetParam()));
    if (!::std::tr1::get<1>(GetParam()))///useRange)
    {
      algo_.set(GetDomainBoundaryAlgo::Domain,   ::std::tr1::get<2>(GetParam()));
      algo_.set(GetDomainBoundaryAlgo::MinRange, ::std::tr1::get<3>(GetParam()));
      algo_.set(GetDomainBoundaryAlgo::MaxRange, ::std::tr1::get<3>(GetParam()));
      algo_.set(GetDomainBoundaryAlgo::UseRange, true);
    }
  }
  virtual void TearDown()
  {  }

};

TEST_P(GetDomainBoundaryTestsParameterized, LatVolBoundry_Parameterized)
{
    //EXPECT_EQ(0, boundary->vmesh()->num_nodes());
  boundary_.reset();
  ASSERT_TRUE(algo_.runImpl(latVol_, unused_, boundary_));
  ASSERT_THAT(boundary_, NotNull());
    //EXPECT_EQ(expectedBoundaryNodes, boundary->vmesh()->num_nodes());
    //EXPECT_EQ(expectedBoundaryElements, boundary->vmesh()->num_elems());
  EXPECT_TRUE(boundary_->vmesh()->is_quadsurfmesh());
}

INSTANTIATE_TEST_CASE_P(
  LatVolBoundry_Parameterized,
  GetDomainBoundaryTestsParameterized,
  Combine(Bool(), Bool(), Values(1,4), Values(1,4), Values(1,4))
  );

#else
TEST(DummyTest, CombineIsNotSupportedOnThisPlatform){}
//
#endif

#endif
