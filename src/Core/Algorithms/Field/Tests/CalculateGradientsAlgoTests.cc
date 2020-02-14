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

#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateGradientsAlgo.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

namespace
{

  /*** TRI SURFs ***/
  FieldHandle CreateTriSurfScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }
  FieldHandle CreateTriSurfVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }

  /*** TET MESHs ***/
  FieldHandle CreateTetMeshVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
  FieldHandle CreateTetMeshTensorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/tensor/tet_tensor_on_node.fld");
  }

  /*** CLOUD POINT ***/
  FieldHandle CreatePointClodeScalar()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/point_cloud/scalar/pts_scalar.fld");
  }
}

TEST(CalculateGradientsAlgoTests, TriSurfScalarOnNodeAsInput)
{
  FieldHandle in = CreateTriSurfScalarOnNode();
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_EQ(algo.run(in, out), true);
  // TODO: compare values from SCIRun 4
}
TEST(CalculateGradientsAlgoTests, TriSurfVectorOnNodeAsInput)
{
  FieldHandle in = CreateTriSurfVectorOnNode();
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}
TEST(CalculateGradientsAlgoTests, TetMeshScalarOnNodeAsInput)
{
//  FAIL() << "TODO"; // SEG FAULT
  FieldHandle in = CreateTetMeshScalarOnNode();
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_EQ(algo.run(in, out), true);
  // TODO: compare values from SCIRun 4
}
TEST(CalculateGradientsAlgoTests, DISABLED_TetMeshTensorOnNodeAsInput)
{
  FieldHandle in = CreateTetMeshTensorOnNode();
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}
TEST(CalculateGradientsAlgoTests, TetMeshVectorOnNodeAsInput)
{
  FieldHandle in = CreateTetMeshVectorOnNode();
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}

TEST(CalculateGradientsAlgoTests, PointCloudScalarOnNodeAsInput)
{
  FieldHandle in = CreatePointClodeScalar();
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}

TEST(CalculateGradientsAlgoTests, NullFieldHandleInput)
{
  FieldHandle in;
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}

//TODO DAN FIX TEST
TEST(CalculateGradientsAlgoTests, DISABLED_NoFieldDataInput)
{
  FieldHandle in = CubeTetVolLinearBasis(DOUBLE_E);
  FieldHandle out;
  CalculateGradientsAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}
