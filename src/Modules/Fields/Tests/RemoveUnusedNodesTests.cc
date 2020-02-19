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


#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Legacy/Fields/RemoveUnusedNodes.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Geometry;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class RemoveUnusedNodesModuleTests : public ModuleTest
{

};


TEST_F(RemoveUnusedNodesModuleTests, ThrowsForNullInput)
{
  auto cg = makeModule("RemoveUnusedNodes");
  FieldHandle nullField;
  stubPortNWithThisData(cg, 0, nullField);
  EXPECT_THROW(cg->execute(), NullHandleOnPortException);
}

TEST_F(RemoveUnusedNodesModuleTests, WrongInput)
{
  auto cvm = makeModule("RemoveUnusedNodes");
	SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
	m->insert(0,0) = 1;
	m->insert(0,1) = 7;
	m->insert(0,2) = 3;
	m->insert(1,0) = 7;
	m->insert(1,1) = 4;
	m->insert(1,2) = -5;
	m->insert(2,0) = 3;
	m->insert(2,1) = -5;
	m->insert(2,2) = 6;
	m->makeCompressed();
  stubPortNWithThisData(cvm, 0, m);
  EXPECT_THROW(cvm->execute(), WrongDatatypeOnPortException);
}

TEST_F(RemoveUnusedNodesModuleTests, ShouldWork)
{
  auto cg = makeModule("RemoveUnusedNodes");
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
  stubPortNWithThisData(cg, 0, basicTriangleField_);
  EXPECT_NO_THROW(cg->execute());
}
