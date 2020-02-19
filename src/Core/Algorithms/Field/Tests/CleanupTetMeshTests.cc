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
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/Cleanup/CleanupTetMesh.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

class CleanupTetMeshTests : public ::testing::Test
{

};

TEST_F(CleanupTetMeshTests, CleanupTetMeshTests_RemoveDegenerateTet)
{
  FieldInformation fi("TetVolMesh", CONSTANTDATA_E, "double");
  FieldHandle field = CreateField(fi);
  auto vmesh = field->vmesh();
  VMesh::Node::array_type vdata;
  vdata.resize(4);
  vmesh->node_reserve(4);
  vmesh->elem_reserve(1);
  vmesh->add_point( Point(98.2337, 179.3770, 72.7303));
  vmesh->add_point( Point(98.2337, 179.3770, 72.7303));
  vmesh->add_point( Point(97.0604, 179.6824, 72.3515));
  vmesh->add_point( Point(97.9706, 180.4140, 71.7113));
  vdata[0] = 0;
  vdata[1] = 0;
  vdata[2] = 2;
  vdata[3] = 3;
  vmesh->add_elem(vdata);
  field->vfield()->resize_values();
  field->vfield()->set_all_values(0.0);
  CleanupTetMeshAlgo algo;
  FieldHandle output;
  algo.set(Parameters::RemoveDegenerateCheckBox, true);
  algo.set(Parameters::FixOrientationCheckBox, false);
  if(!algo.run(field, output))
  {
   std::cout << "Unexpected error: algorithm failed!" << std::endl;
  }
  EXPECT_EQ(output->vmesh()->num_nodes(),4);
  EXPECT_EQ(output->vmesh()->num_elems(),0);
  EXPECT_EQ(output->vfield()->num_values(),0);
}

TEST_F(CleanupTetMeshTests, CleanupTetMeshTests_fixorientations)
{
  FieldInformation fi("TetVolMesh", CONSTANTDATA_E, "double");
  FieldHandle field = CreateField(fi);
  auto vmesh = field->vmesh();
  VMesh::Node::array_type vdata;
  vdata.resize(4);
  vmesh->node_reserve(4);
  vmesh->elem_reserve(1);
  vmesh->add_point( Point(-98.2337, -179.3770, -72.7303));
  vmesh->add_point( Point(97.0604, 179.6824, 72.3515));
  vmesh->add_point( Point(97.9706, 180.4140, 71.7113));
  vmesh->add_point( Point(-97.5724, -179.0278, -71.8274));
  for (size_type i = 0; i < 4; ++i)
    vdata[i] = i;
  vmesh->add_elem(vdata);
  field->vfield()->resize_values();
  field->vfield()->set_all_values(0.0);
  CleanupTetMeshAlgo algo;
  FieldHandle output;
  algo.set(Parameters::RemoveDegenerateCheckBox, false);
  algo.set(Parameters::FixOrientationCheckBox, true);
  if(!algo.run(field, output))
  {
   std::cout << "Unexpected error: algorithm failed!" << std::endl;
  }
  VMesh*  imesh  = output->vmesh();
  VMesh::Node::array_type nodes;
  imesh->get_nodes(nodes,(VMesh::Elem::index_type)0);
  EXPECT_EQ(nodes[0],1);
  EXPECT_EQ(nodes[1],0);
  EXPECT_EQ(nodes[2],2);
  EXPECT_EQ(nodes[3],3);
}
