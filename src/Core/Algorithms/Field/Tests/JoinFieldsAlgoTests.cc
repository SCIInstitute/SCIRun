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
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
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

class JoinFieldsAlgoTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    SCIRun::Core::Logging::Log::get().setVerbose(true);
  }
};

// parameters:
// MergeNodes = Bool()
// MergeElems = Bool()
// MatchNodeValues = Bool()
// MakeNoData = Bool()
// Tolerance = Values(1e-1, 1e-3, ...)

TEST_F(JoinFieldsAlgoTests, CanLogErrorMessage)
{
  JoinFieldsAlgo algo;
  FieldList input;
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(input, output));
}

//#if SCIRUN4_CODE_TO_BE_ENABLED_LATER
#if GTEST_HAS_COMBINE

/*Get Parameterized Tests
*/
using ::testing::Bool;
using ::testing::Values;
using ::testing::Combine;

class JoinFieldsAlgoTestsParameterized : public ::testing::TestWithParam < ::std::tr1::tuple<bool, bool, bool, bool, bool, double> >
{
public:
  FieldHandle latVol1_;
  FieldHandle latVol2_;

  JoinFieldsAlgo algo_;
  FieldList fieldList_; 

  JoinFieldsAlgoTestsParameterized() 
  {
    latVol1_ = loadFieldFromFile(TestResources::rootDir() / "latVolWithNormData.fld");
    SCIRun::Core::Logging::Log::get().setVerbose(true);
  } 

protected:
  virtual void SetUp()
  {
	FieldList fl;
	//create 2 latVol's
	//SCIRun::Modules::Fields lv1;
	//CreateLatVol() lv2; 

	//lv1.set(lv1.XSize(2)); 
	
    ASSERT_TRUE(latVol1_->vmesh()->is_latvolmesh());
    // How to set parameters on an algorithm (that come from the GUI)
    algo_.set(algo_.MergeNodes,      ::std::tr1::get<0>(GetParam()));
	algo_.set(algo_.MergeElems,      ::std::tr1::get<1>(GetParam()));
    algo_.set(algo_.InputFields,     ::std::tr1::get<2>(GetParam()));
    algo_.set(algo_.MatchNodeValues, ::std::tr1::get<3>(GetParam()));
    algo_.set(algo_.MakeNoData,      ::std::tr1::get<4>(GetParam()));
    algo_.set(algo_.Tolerance,       ::std::tr1::get<5>(GetParam()));
  }
  virtual void TearDown()
  {  }

};

TEST_P(JoinFieldsAlgoTestsParameterized, JoinFieldsAlgo_Parameterized)
{
  //boundary_.reset();
  //ASSERT_TRUE(algo_.runImpl(latVol1_));
  //ASSERT_THAT(boundary_, NotNull());
  //EXPECT_TRUE(boundary_->vmesh()->is_quadsurfmesh());
}

INSTANTIATE_TEST_CASE_P(
  JoinFieldsAlgo_Parameterized,
  JoinFieldsAlgoTestsParameterized,
  Combine(Bool(), Bool(), Bool(), Bool(), Bool(), Values(1e-1,1e-3)) 
  );

#else
TEST(DummyTest, CombineIsNotSupportedOnThisPlatform){}
//
#endif 

//#endif