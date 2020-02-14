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


#include <Modules/Legacy/Fields/CalculateVectorMagnitudes.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class CalculateVectorMagnitudesModuleTests : public ModuleTest
{

};

namespace
{
  FieldHandle CreateTriSurfVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }
  FieldHandle CreateTetMeshVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }
  FieldHandle CreateTriSurfVectorOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_elem/vector/tri_vector_on_elem.fld");
  }
  FieldHandle CreateTetMeshVectorOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }

  // scalar tests are covered at the algorithm level
}

TEST_F(CalculateVectorMagnitudesModuleTests, TriSurfOnElemVectorInput)
{
  auto cvm = makeModule("CalculateVectorMagnitudes");
  connectDummyOutputConnection(cvm, 0);
  stubPortNWithThisData(cvm, 0, CreateTriSurfVectorOnElem());
  EXPECT_NO_THROW(cvm->execute());
}
TEST_F(CalculateVectorMagnitudesModuleTests, TetMeshOnElemVectorInput)
{
  auto cvm = makeModule("CalculateVectorMagnitudes");
  connectDummyOutputConnection(cvm, 0);
  stubPortNWithThisData(cvm, 0, CreateTetMeshVectorOnElem());
  EXPECT_NO_THROW(cvm->execute());
}
TEST_F(CalculateVectorMagnitudesModuleTests, TetMeshOnNodeVectorInput)
{
  auto cvm = makeModule("CalculateVectorMagnitudes");
  connectDummyOutputConnection(cvm, 0);
  stubPortNWithThisData(cvm, 0, CreateTetMeshVectorOnNode());
  EXPECT_NO_THROW(cvm->execute());
}
TEST_F(CalculateVectorMagnitudesModuleTests, TriSurfOnNodeVectorInput)
{
  auto cvm = makeModule("CalculateVectorMagnitudes");
  connectDummyOutputConnection(cvm, 0);
  stubPortNWithThisData(cvm, 0, CreateTriSurfVectorOnNode());
  EXPECT_NO_THROW(cvm->execute());
}
TEST_F(CalculateVectorMagnitudesModuleTests, SparseRowMatrixInput)
{
  auto cvm = makeModule("CalculateVectorMagnitudes");
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
TEST_F(CalculateVectorMagnitudesModuleTests, ThrowsForNullInput)
{
  auto cvm = makeModule("CalculateVectorMagnitudes");
  FieldHandle nullField;
  stubPortNWithThisData(cvm, 0, nullField);
  EXPECT_THROW(cvm->execute(), NullHandleOnPortException);
}
