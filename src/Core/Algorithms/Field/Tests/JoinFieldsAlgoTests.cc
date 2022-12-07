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
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace Core::Datatypes;
using namespace Core::Geometry;
using namespace Core::Logging;
using namespace Core::Algorithms::Fields;
using namespace Core::Algorithms;
using namespace TestUtils;
using testing::NotNull;
using testing::TestWithParam;
using testing::Values;

// TODO: surface and volume mesh types (TriSurf, TetVol), other basis types

class JoinFieldsAlgoTests : public testing::Test
{
protected:
  void SetUp() override
  {
    LogSettings::Instance().setVerbose(true);
  }

  FieldHandle CreateEmptyLatVol(size_type sizex = 3, size_type sizey = 4, size_type sizez = 5, data_info_type type=data_info_type::DOUBLE_E)
  {
    FieldInformation lfi(mesh_info_type::LATVOLMESH_E, databasis_info_type::LINEARDATA_E, type);
    Point minb(-1.0, -1.0, -1.0);
    Point maxb(1.0, 1.0, 1.0);
    MeshHandle mesh = CreateMesh(lfi, sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    return ofh;
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

TEST_F(JoinFieldsAlgoTests, CanJoinMultipleLatVols)
{
  const JoinFieldsAlgo algo;

  FieldList input;
  input.push_back(CreateEmptyLatVol(2,3,4, data_info_type::INT_E));
  EXPECT_EQ(2*3*4, input[0]->vmesh()->num_nodes());
  input.push_back(CreateEmptyLatVol(5,6,7, data_info_type::INT_E));
  EXPECT_EQ(5*6*7, input[1]->vmesh()->num_nodes());
  input.push_back(CreateEmptyLatVol(8,9,10, data_info_type::INT_E));
  EXPECT_EQ(8*9*10, input[2]->vmesh()->num_nodes());

  FieldHandle output;
  EXPECT_TRUE(algo.runImpl(input, output));
  EXPECT_EQ(914, output->vmesh()->num_nodes());

  input.push_back(CreateEmptyLatVol(11,12,13, data_info_type::INT_E));
  EXPECT_EQ(11*12*13, input[3]->vmesh()->num_nodes());
  EXPECT_TRUE(algo.runImpl(input, output));
  EXPECT_EQ(2588, output->vmesh()->num_nodes());
}

TEST_F(JoinFieldsAlgoTests, CanJoinMultipleLatVolsGeneric)
{
  JoinFieldsAlgo algo;

  FieldList input;
  input.push_back(CreateEmptyLatVol(2,3,4));
  input.push_back(CreateEmptyLatVol(5,6,7));
  input.push_back(CreateEmptyLatVol(8,9,10));

  auto outputObj = algo.run(withInputData((Variables::InputFields, input)));

  FieldHandle output = outputObj.get<SCIRun::Field>(Variables::OutputField);
  EXPECT_EQ(914, output->vmesh()->num_nodes());
}

#if GTEST_HAS_COMBINE

// Get Parameterized Tests

using testing::Bool;
using testing::Values;
using testing::Combine;

class JoinFieldsAlgoTestsParameterized : public TestWithParam<std::tuple<bool, bool, bool, bool,
      double>>
{
public:
  JoinFieldsAlgo algo_;
  FieldList input;
  FieldHandle output;
protected:
  void SetUp() override
  {
    LogSettings::Instance().setVerbose(true);
    // How to set parameters on an algorithm (that come from the GUI)
    algo_.set(Parameters::merge_nodes,      std::get<0>(GetParam()));
    algo_.set(Parameters::merge_elems,      std::get<1>(GetParam()));
    algo_.set(Parameters::match_node_values, std::get<2>(GetParam()));
    algo_.set(Parameters::make_no_data,      std::get<3>(GetParam()));
    algo_.set(Parameters::tolerance,       std::get<4>(GetParam()));
  }

  void TearDown() override { }
};

TEST_P(JoinFieldsAlgoTestsParameterized, JoinFieldsAlgo_Parameterized)
{
  input.push_back(CreateEmptyLatVol(2, 3, 4, data_info_type::INT_E));
  input.push_back(CreateEmptyLatVol(5, 6, 7, data_info_type::INT_E));
  input.push_back(CreateEmptyLatVol(8, 9, 10, data_info_type::INT_E));
  EXPECT_TRUE(algo_.runImpl(input, output));
}

TEST_P(JoinFieldsAlgoTestsParameterized, JoinFieldsAlgo_Parameterized_data_types)
{
  // should just print message about converting data
  input.push_back(CreateEmptyLatVol(2, 3, 4, data_info_type::INT_E));
  input.push_back(CreateEmptyLatVol(5, 6, 7, data_info_type::INT_E));
  input.push_back(CreateEmptyLatVol(8, 9, 10, data_info_type::INT_E));
  input.push_back(CreateEmptyLatVol(11, 12, 13, data_info_type::FLOAT_E));
  EXPECT_TRUE(algo_.runImpl(input, output));
}

TEST_P(JoinFieldsAlgoTestsParameterized, JoinFieldsAlgo_Parameterized_missing_input)
{
  algo_.runImpl(input, output);
  EXPECT_FALSE(algo_.runImpl(input, output));
}

TEST_P(JoinFieldsAlgoTestsParameterized, JoinFieldsAlgo_Parameterized_single_input_structured)
{
  input.push_back(CreateEmptyLatVol(2, 3, 4, data_info_type::INT_E));
  EXPECT_TRUE(algo_.runImpl(input, output));
}

//TODO DAN FIX TEST
TEST_P(JoinFieldsAlgoTestsParameterized, DISABLED_JoinFieldsAlgo_Parameterized_single_input_unstructured_match_nodes)
{
  // sample field has no data
  input.push_back(TriangleTriSurfConstantBasis(data_info_type::INT_E));
  algo_.set(Parameters::match_node_values, true);
  EXPECT_FALSE(algo_.runImpl(input, output));
}

//TODO DAN FIX TEST
TEST_P(JoinFieldsAlgoTestsParameterized, DISABLED_JoinFieldsAlgo_Parameterized_single_input_unstructured_no_node_match)
{
  // sample field has no data
  input.push_back(TriangleTriSurfConstantBasis(data_info_type::INT_E));
  algo_.set(Parameters::match_node_values, false);
  EXPECT_TRUE(algo_.runImpl(input, output));
}

TEST_P(JoinFieldsAlgoTestsParameterized, JoinFieldsAlgo_Parameterized_mismatched_field_geom)
{
  input.push_back(CubeTriSurfConstantBasis(data_info_type::INT_E));
  input.push_back(CubeTetVolConstantBasis(data_info_type::INT_E));
  EXPECT_FALSE(algo_.runImpl(input, output));
}

TEST_P(JoinFieldsAlgoTestsParameterized, JoinFieldsAlgo_Parameterized_generic)
{
  input.push_back(CreateEmptyLatVol(2,3,4));
  input.push_back(CreateEmptyLatVol(5,6,7));
  input.push_back(CreateEmptyLatVol(8,9,10));

  auto outputObj = algo_.run(withInputData((Variables::InputFields, input)));
  FieldHandle output = outputObj.get<SCIRun::Field>(Variables::OutputField);
  EXPECT_EQ(output->vmesh()->num_nodes(),output->vmesh()->num_nodes());
}

INSTANTIATE_TEST_CASE_P(
                        JoinFieldsAlgo_Parameterized,
                        JoinFieldsAlgoTestsParameterized,
                        Combine(Bool(), Bool(), Bool(), Bool(), Values(1e-1,1e-3))
                        );

#else

TEST(DummyTest, CombineIsNotSupportedOnThisPlatform){}

#endif
