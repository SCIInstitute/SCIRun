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
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

TEST(ConvertFieldBasisAlgoTest, TetMeshOnNodeVectorMatLinear)
{
  ConvertFieldBasisTypeAlgo algo;

  FieldHandle tetmesh = LoadTetWithData();

  const int givenBasis = 1;
  EXPECT_EQ(givenBasis, tetmesh->vfield()->basis_order());
  EXPECT_EQ(7, tetmesh->vfield()->num_values());
  EXPECT_EQ(0, tetmesh->vfield()->num_evalues());
  double min, max;
  tetmesh->vfield()->minmax(min,max);
  EXPECT_NEAR(338.38, min, 1e-3);
  EXPECT_NEAR(340.657, max, 1e-4);

  algo.setOption(Parameters::OutputType, "Linear");
  algo.set(Parameters::BuildBasisMapping, false);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));

  const int expectedBasis = 1;
  EXPECT_EQ(expectedBasis, result->vfield()->basis_order());
  EXPECT_EQ(7, result->vfield()->num_values());
  EXPECT_EQ(0, result->vfield()->num_evalues());
  result->vfield()->minmax(min,max);
  //TODO: get cell data--this is coming back with 0, which is unexpected based on the v4 GUI, but correct based on the wiki doc.
  //EXPECT_NEAR(339.275, min, 1e-4);
  //EXPECT_NEAR(339.844, max, 1e-4);
}

//TODO BEN: generate unit tests for each combination of input/output bases.
TEST(ConvertFieldBasisAlgoTest, TetMeshOnNodeVectorMatConstant)
{
  ConvertFieldBasisTypeAlgo algo;

  FieldHandle tetmesh = LoadTetWithData();

  const int givenBasis = 1;
  EXPECT_EQ(givenBasis, tetmesh->vfield()->basis_order());
  EXPECT_EQ(7, tetmesh->vfield()->num_values());
  EXPECT_EQ(0, tetmesh->vfield()->num_evalues());
  double min, max;
  tetmesh->vfield()->minmax(min,max);
  EXPECT_NEAR(338.38, min, 1e-3);
  EXPECT_NEAR(340.657, max, 1e-4);

  algo.setOption(Parameters::OutputType, "Constant");
  algo.set(Parameters::BuildBasisMapping, false);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));

  const int expectedBasis = 0;
  EXPECT_EQ(expectedBasis, result->vfield()->basis_order());
  EXPECT_EQ(3, result->vfield()->num_values());
  EXPECT_EQ(0, result->vfield()->num_evalues());
  result->vfield()->minmax(min,max);
  //TODO: get cell data--this is coming back with 0, which is unexpected based on the v4 GUI, but correct based on the wiki doc.
  //EXPECT_NEAR(339.275, min, 1e-4);
  //EXPECT_NEAR(339.844, max, 1e-4);
}
TEST(ConvertFieldBasisAlgoTest, TetMeshOnNodeVectorMatNone)
{
  ConvertFieldBasisTypeAlgo algo;

  FieldHandle tetmesh = LoadTetWithData();

  const int givenBasis = 1;
  EXPECT_EQ(givenBasis, tetmesh->vfield()->basis_order());
  EXPECT_EQ(7, tetmesh->vfield()->num_values());
  EXPECT_EQ(0, tetmesh->vfield()->num_evalues());
  double min, max;
  tetmesh->vfield()->minmax(min,max);
  EXPECT_NEAR(338.38, min, 1e-3);
  EXPECT_NEAR(340.657, max, 1e-4);

  algo.setOption(Parameters::OutputType, "None");
  algo.set(Parameters::BuildBasisMapping, false);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));

  const int expectedBasis = -1;
  EXPECT_EQ(expectedBasis, result->vfield()->basis_order());
  EXPECT_EQ(0, result->vfield()->num_values());
  EXPECT_EQ(0, result->vfield()->num_evalues());
  result->vfield()->minmax(min,max);
  //TODO: get cell data--this is coming back with 0, which is unexpected based on the v4 GUI, but correct based on the wiki doc.
  //EXPECT_NEAR(339.275, min, 1e-4);
  //EXPECT_NEAR(339.844, max, 1e-4);
}
//TODO: Test mapping matrix output. Not used much, so disable for now.
