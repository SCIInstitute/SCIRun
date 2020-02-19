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
#include <Modules/Legacy/Fields/CleanupTetMesh.h>
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

class CleanupTetMeshModuleTests : public ModuleTest
{

};


TEST_F(CleanupTetMeshModuleTests, ThrowsForNullInput)
{
  auto cg = makeModule("CleanupTetMesh");
  FieldHandle nullField;
  stubPortNWithThisData(cg, 0, nullField);
  EXPECT_THROW(cg->execute(), NullHandleOnPortException);
}

TEST_F(CleanupTetMeshModuleTests, WrongInput)
{
  auto cvm = makeModule("CleanupTetMesh");
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

TEST_F(CleanupTetMeshModuleTests, ShouldWork)
{
  auto cg = makeModule("CleanupTetMesh");
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
  stubPortNWithThisData(cg, 0, field);
  EXPECT_NO_THROW(cg->execute());
}
