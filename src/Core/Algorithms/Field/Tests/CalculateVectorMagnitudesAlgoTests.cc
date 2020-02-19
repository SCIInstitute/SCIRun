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
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateVectorMagnitudesAlgo.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/SCIRunFieldSamples.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::TestUtils;

namespace {

  /*** TRI SURFs ***/
  FieldHandle CreateTriSurfVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }
  FieldHandle CreateTriSurfVectorOnNodeSCIRun4Output()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/CalculatedVectorMagnitudesForTriSurfVectorOnNodeFromSCIRun4.fld");
  }
  FieldHandle CreateTriSurfScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }

  /*** TET MESHs ***/
  FieldHandle CreateTetMeshVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }
  FieldHandle CreateTetMeshVectorOnNodeSCIRun4Output()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/CalculatedVectorMagnitudesForTetMeshVectorOnNodeFromSCIRun4.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
  FieldHandle CreateTetMeshTensorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/tensor/tet_tensor_on_node.fld");
  }
}

TEST(CalculateVectorMagnitudesAlgoTests, CompareTriSurfVectorOnNodeToSCIRun4)
{
  FieldHandle in = CreateTriSurfVectorOnNode();
  FieldHandle out;
  CalculateVectorMagnitudesAlgo algo;
  algo.run(in, out);

  FieldHandle SCIRun4Output = CreateTriSurfVectorOnNodeSCIRun4Output();
  VField* expected_vals = SCIRun4Output->vfield(); // what is to be expected
  VField* outputed_vals = out->vfield(); // the output
  double* expected_mag = reinterpret_cast<double*>(expected_vals->get_values_pointer());
  double* outputed_mag = reinterpret_cast<double*>(outputed_vals->get_values_pointer());

  // getting the number of things to compare
  VMesh*  imesh  = in->vmesh();
  VField::size_type num_elems = imesh->num_elems();

  for (VMesh::Elem::index_type idx = 0; idx < num_elems; idx++)
    EXPECT_EQ(expected_mag[idx],outputed_mag[idx]);
}

TEST(CalculateVectorMagnitudesAlgoTests, TriSurfScalarOnNodeAsInput)
{
  FieldHandle in = CreateTriSurfScalarOnNode();
  FieldHandle out;
  CalculateVectorMagnitudesAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}

TEST(CalculateVectorMagnitudesAlgoTests, TetMeshVectorOnNodeAsInput)
{
  FieldHandle in = CreateTetMeshVectorOnNode();
  FieldHandle out;
  CalculateVectorMagnitudesAlgo algo;
  algo.run(in, out);

  FieldHandle SCIRun4Output = CreateTetMeshVectorOnNodeSCIRun4Output();
  VField* expected_vals = SCIRun4Output->vfield(); // what is to be expected
  VField* outputed_vals = out->vfield(); // the output
  double* expected_mag = reinterpret_cast<double*>(expected_vals->get_values_pointer());
  double* outputed_mag = reinterpret_cast<double*>(outputed_vals->get_values_pointer());

  // getting the number of things to compare
  VMesh*  imesh  = in->vmesh();
  VField::size_type num_elems = imesh->num_elems();

  // comparing every value
  for (VMesh::Elem::index_type idx = 0; idx < num_elems; idx++)
    EXPECT_EQ(expected_mag[idx],outputed_mag[idx]);
}

TEST(CalculateVectorMagnitudesAlgoTests, TetMeshScalarOnNodeAsInput)
{
  FieldHandle in = CreateTetMeshScalarOnNode();
  FieldHandle out;
  CalculateVectorMagnitudesAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}

TEST(CalculateVectorMagnitudesAlgoTests, DISABLED_TetMeshTensorOnNodeAsInput)
{
  FAIL(); // TODO: Enable test when tensors can be read
  FieldHandle in = CreateTetMeshTensorOnNode();
  FieldHandle out;
  CalculateVectorMagnitudesAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}

TEST(CalculateVectorMagnitudesAlgoTests, NullFieldHandleInput)
{
  FieldHandle in;
  FieldHandle out;
  CalculateVectorMagnitudesAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}

TEST(CalculateVectorMagnitudesAlgoTests, NoFieldDataInput)
{
  FieldHandle in = CubeTetVolLinearBasis(DOUBLE_E);
  FieldHandle out;
  CalculateVectorMagnitudesAlgo algo;
  EXPECT_THROW(algo.run(in, out), AlgorithmInputException);
}
