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
#include <Core/Algorithms/Legacy/Fields/Cleanup/RemoveUnusedNodes.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

class RemoveUnusedNodesTests : public ::testing::Test
{

};

TEST_F(RemoveUnusedNodesTests, RemoveUnusedNodesTests_trianglemeshexample)
{
  MeshHandle basicTriangleMesh_;
  FieldInformation fi("TriSurfMesh", CONSTANTDATA_E, "double");
  basicTriangleMesh_ = CreateMesh(fi);
  auto triangleVMesh = basicTriangleMesh_->vmesh();
  FieldHandle basicTriangleField_ = CreateField(fi, basicTriangleMesh_);
  triangleVMesh->add_point(Point(0.0, 0.0, 0.0));
  triangleVMesh->add_point(Point(1.0, 0.0, 0.0));
  triangleVMesh->add_point(Point(0.5, 1.0, 0.0));
  triangleVMesh->add_point(Point(2.0, 2.0, 2.0)); // add extra point which need to be removed
  VMesh::Node::array_type vdata(3);
  vdata[0]=0;
  vdata[1]=1;
  vdata[2]=2;
  triangleVMesh->add_elem(vdata);
  auto triangleVField = basicTriangleField_->vfield();
  triangleVField->resize_values();
  triangleVField->set_all_values(0.0);
  RemoveUnusedNodesAlgo algo;
  FieldHandle output;
  algo.run(basicTriangleField_, output);
  EXPECT_EQ(output->vmesh()->num_nodes(),3);
  EXPECT_EQ(output->vmesh()->num_elems(),1);
  EXPECT_EQ(output->vfield()->num_values(),1);
}

TEST_F(RemoveUnusedNodesTests, RemoveUnusedNodesTests_tetrahedralmeshexample)
{
  FieldInformation fi("TetVolMesh", CONSTANTDATA_E, "double");
  FieldHandle field = CreateField(fi);
  auto vmesh = field->vmesh();
  VMesh::Node::array_type vdata;
  vdata.resize(4);

  vmesh->node_reserve(5);
  vmesh->elem_reserve(1);
  vmesh->add_point( Point(0.0, 0.0, 0.0) );
  vmesh->add_point( Point(0.5, 1.0, 0.0) );
  vmesh->add_point( Point(1.0, 0.0, 0.0) );
  vmesh->add_point( Point(0.5, 0.5, 1.0) );
  vmesh->add_point( Point(2.0, 2.0, 2.0) ); // add extra point which need to be removed

  for (size_type i = 0; i < 4; ++i)
  {
    vdata[i] = i;
  }
  vmesh->add_elem(vdata);
  field->vfield()->resize_values();
  field->vfield()->set_all_values(0.0);

  RemoveUnusedNodesAlgo algo;
  FieldHandle output;
  algo.run(field, output);
  EXPECT_EQ(output->vmesh()->num_nodes(),4);
  EXPECT_EQ(output->vmesh()->num_elems(),1);
  EXPECT_EQ(output->vfield()->num_values(),1);
}
