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
    SCIRun::Core::Logging::Log::get().setVerbose(true);
  }

  void runTest(bool includeOuterBoundary, bool useRange, int domainValue,
    int expectedBoundaryNodes, int expectedBoundaryElements)
  {
    FieldHandle latVol = loadFieldFromFile(TestResources::rootDir() / "latVolWithNormData.fld");
    ASSERT_TRUE(latVol->vmesh()->is_latvolmesh());

    GetDomainBoundaryAlgo algo;

    // How to set parameters on an algorithm (that come from the GUI)
    algo.set(GetDomainBoundaryAlgo::AddOuterBoundary, includeOuterBoundary);

    // TODO: this logic matches the wacky module behavior
    algo.set(GetDomainBoundaryAlgo::UseRange, useRange);
    if (!useRange)
    {
      algo.set(GetDomainBoundaryAlgo::Domain, domainValue);
      //algo.set(GetDomainBoundaryAlgo::MinRange, domainValue);
	  algo.set(GetDomainBoundaryAlgo::MinRange, 0);
      //algo.set(GetDomainBoundaryAlgo::MaxRange, domainValue);
      algo.set(GetDomainBoundaryAlgo::MaxRange, 255);
	  algo.set(GetDomainBoundaryAlgo::UseRange, true);
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
  runTest(false, true, 1, 9024, 8574);//p
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_False_1)
{
  runTest(false, false, 1, 9024, 8574);//p
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_False_1)
{
  runTest(true, false, 1, -1, -1);//f, crashes GUI 
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_True_1)
{
  runTest(true, true, 1, -1, -1);//f, crashes GUI
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_False_4)
{
  runTest(false, false, 4, 9024, 8574);//p
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_False_4)
{
  runTest(true, false, 4, -1, -1);//f, crashes GUI
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_True_4)
{
  runTest(false, true, 4, 9024, 8574);//p
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_True_4)
{
  runTest(true, true, 4, -1, -1);//f, crashes GUI
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

class GetDomainBoundaryTests1 : public ::testing::TestWithParam < ::std::tr1::tuple<bool, bool, bool, bool, bool, int> >//, int, int> >
{
public:
  FieldHandle boundary;
  SparseRowMatrixHandle unused;
  ~GetDomainBoundaryTests1() {} 

protected:
  virtual void SetUp()
  {
    SCIRun::Core::Logging::Log::get().setVerbose(true);
  //}
    /*void runTest(bool includeOuterBoundary, bool useRange, int domainValue,
    int expectedBoundaryNodes, int expectedBoundaryElements)
  {*/
    FieldHandle latVol = loadFieldFromFile(TestResources::rootDir() / "latVolWithNormData.fld");
    ASSERT_TRUE(latVol->vmesh()->is_latvolmesh());

    GetDomainBoundaryAlgo algo;
    // How to set parameters on an algorithm (that come from the GUI)
    
    algo.set(GetDomainBoundaryAlgo::AddOuterBoundary, ::std::tr1::get<0>(GetParam()));
    
    //// TODO: this logic matches the wacky module behavior
    algo.set(GetDomainBoundaryAlgo::UseRange, ::std::tr1::get<1>(GetParam()));
    if (!::std::tr1::get<1>(GetParam()))//!useRange)
    {
      algo.set(GetDomainBoundaryAlgo::Domain,   ::std::tr1::get<2>(GetParam()));
      algo.set(GetDomainBoundaryAlgo::MinRange, 0); //::std::tr1::get<3>(GetParam()));
      algo.set(GetDomainBoundaryAlgo::MaxRange, 255); //::std::tr1::get<3>(GetParam()));
      algo.set(GetDomainBoundaryAlgo::UseRange, true);
    }  
	algo.set(GetDomainBoundaryAlgo::InnerBoundaryOnly, ::std::tr1::get<3>(GetParam()));
	algo.set(GetDomainBoundaryAlgo::NoInnerBoundary, ::std::tr1::get<4>(GetParam()));
	algo.set(GetDomainBoundaryAlgo::DisconnectBoundaries, ::std::tr1::get<5>(GetParam())); 
    ASSERT_TRUE(algo.runImpl(latVol, unused, boundary));

  }
  virtual void TearDown()
  {  }

};

TEST_P(GetDomainBoundaryTests1, LatVolBoundary_Parameterized)
{
    EXPECT_NO_FATAL_FAILURE(GetDomainBoundaryTests1); 
    //EXPECT_EQ(0, module->boundary->vmesh()->num_nodes()); 
    //ASSERT_THAT(boundary, NotNull());
    //EXPECT_EQ(expectedBoundaryNodes, boundary->vmesh()->num_nodes());
    //EXPECT_EQ(expectedBoundaryElements, boundary->vmesh()->num_elems());
    //EXPECT_TRUE(boundary->vmesh()->is_quadsurfmesh());
}
//TEST_P(GetDomainBoundaryTests1, LatVolBoundary_CheckFailures)
//{
//	GetDomainBoundaryTests1;
//	if(HasFailure())
//		return;
//}

//INSTANTIATE_TEST_CASE_P(
//  LatVolBoundry_Parameterized,
//  GetDomainBoundaryTests1,
//  Combine(Bool(), Bool(), Values(1,4), Values(1,4), Values(1,4)) 
//  );
INSTANTIATE_TEST_CASE_P(
  LatVolBoundary_CheckFailures,
  GetDomainBoundaryTests1,
  Combine(Bool(), Bool(), Bool(), Bool(), Bool(), Values(1,2,3,4)) 
  );
#else
TEST(DummyTest, CombineIsNotSupportedOnThisPlatform){}
//
#endif 