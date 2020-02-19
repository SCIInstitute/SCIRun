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
#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;
using ::testing::NotNull;
using ::testing::TestWithParam;
using ::testing::Values;

class GetDomainBoundaryTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(true);
  }

  void runTest(bool includeOuterBoundary, bool useRange, int domainValue,
    int expectedBoundaryNodes, int expectedBoundaryElements)
  {
    FieldHandle latVol = loadFieldFromFile(TestResources::rootDir() / "Fields" / "latVolWithNormData.fld");
    ASSERT_TRUE(latVol->vmesh()->is_latvolmesh());

    GetDomainBoundaryAlgo algo;

    // How to set parameters on an algorithm (that come from the GUI)
    algo.set(Parameters::AddOuterBoundary, includeOuterBoundary);

    /// @todo: this logic matches the wacky module behavior

    algo.set(Parameters::UseRange, useRange);
    if (!useRange)
    {
      algo.set(Parameters::Domain, domainValue);
      algo.set(Parameters::MinRange, domainValue);
      algo.set(Parameters::MaxRange, domainValue);
	  algo.set(Parameters::UseRange, true);
    }

    FieldHandle boundary;
    SparseRowMatrixHandle unused;
    algo.runImpl(latVol, unused, boundary);

    ASSERT_THAT(boundary, NotNull());

    EXPECT_EQ(expectedBoundaryNodes, boundary->vmesh()->num_nodes());
    EXPECT_EQ(expectedBoundaryElements, boundary->vmesh()->num_elems());
    EXPECT_TRUE(boundary->vmesh()->is_quadsurfmesh());
  }
};

// manual, undesirable way to test many different value combinations.
// includeOuterBoundary = Bool()
// useRange = Bool()
// domainValue = Values(0,1,2,3,4)

// expected values of -1 need to be figured out from the v4 GUI.
TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_True_1)
{
  runTest(false, true, 1, 9024, 8574);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_False_1)
{
  runTest(false, false, 1, 8680, 8334);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_False_1)
{
  runTest(true, false, 1, 12328, 12324);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_True_1)
{
  runTest(true, true, 1, 17264, 17700);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_False_4)
{
  runTest(false, false, 4, 0, 0);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_False_4)
{
  runTest(true, false, 4, 0, 0);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_True_4)
{
  runTest(false, true, 4, 9024, 8574);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_True_4)
{
  runTest(true, true, 4, 17264, 17700);
}

TEST_F(GetDomainBoundaryTests, CanLogErrorMessage)
{
  GetDomainBoundaryAlgo algo;

  FieldHandle input, output;
  SparseRowMatrixHandle elemLink;

  EXPECT_FALSE(algo.runImpl(input, elemLink, output));
}

#if GTEST_HAS_COMBINE

/*Get Parameterized Tests
*/
using ::testing::Bool;
using ::testing::Values;
using ::testing::Combine;
class GetDomainBoundaryTestsParameterized : public ::testing::TestWithParam < std::tuple<bool, bool, int, int, int> >

{
public:
  FieldHandle boundary_;
  SparseRowMatrixHandle unused_;
  GetDomainBoundaryAlgo algo_;
  FieldHandle latVol_;
  GetDomainBoundaryTestsParameterized()
  {
    latVol_ = loadFieldFromFile(TestResources::rootDir() / "Fields" / "latVolWithNormData.fld");
    LogSettings::Instance().setVerbose(true);
  }

protected:
  virtual void SetUp()
  {
    ASSERT_TRUE(latVol_->vmesh()->is_latvolmesh());

    // How to set parameters on an algorithm (that come from the GUI)
    algo_.set(Parameters::AddOuterBoundary, std::get<0>(GetParam()));

    /// @todo: this logic matches the wacky module behavior
    algo_.set(Parameters::UseRange, std::get<1>(GetParam()));
    if (!std::get<1>(GetParam()))///useRange)
    {
      algo_.set(Parameters::Domain,   std::get<2>(GetParam()));
      algo_.set(Parameters::MinRange, std::get<3>(GetParam()));
      algo_.set(Parameters::MaxRange, std::get<3>(GetParam()));
      algo_.set(Parameters::UseRange, true);
    }
	//algo_.set(Parameters::InnerBoundaryOnly, ::std::tr1::get<3>(GetParam()));
	//algo_.set(Parameters::NoInnerBoundary, ::std::tr1::get<4>(GetParam()));
	//algo_.set(Parameters::DisconnectBoundaries, ::std::tr1::get<5>(GetParam()));
    //ASSERT_TRUE(algo_.runImpl(latVol, unused, boundary));
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

#endif
