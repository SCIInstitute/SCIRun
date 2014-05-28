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

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/BrainStimulator/SetConductivitiesToTetMesh.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class SetConductivitiesToTetMeshTests : public ModuleTest
{

};

namespace
{
  FieldHandle CreateTetMeshVectorOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }
  FieldHandle CreateTetMeshSevenElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh_7elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
}

TEST_F(SetConductivitiesToTetMeshTests, TetMeshScalarSevenElem)
{
  auto test = makeModule("SetConductivitiesToTetMesh");
  stubPortNWithThisData(test, 0, CreateTetMeshSevenElem());
  EXPECT_NO_THROW(test->execute());
}

TEST_F(SetConductivitiesToTetMeshTests, TetMeshScalarOnElem)
{
  auto test = makeModule("SetConductivitiesToTetMesh");
  stubPortNWithThisData(test, 0, CreateTetMeshScalarOnElem());
  EXPECT_NO_THROW(test->execute());
}

TEST_F(SetConductivitiesToTetMeshTests, TetMeshScalarOnNode)
{
  auto test = makeModule("SetConductivitiesToTetMesh");
  stubPortNWithThisData(test, 0, CreateTetMeshScalarOnNode());
  EXPECT_THROW(test->execute(), InvalidArgumentException);
}

TEST_F(SetConductivitiesToTetMeshTests, TetMeshVectorOnElem)
{
  auto test = makeModule("SetConductivitiesToTetMesh");
  stubPortNWithThisData(test, 0, CreateTetMeshVectorOnElem());
  EXPECT_THROW(test->execute(), InvalidArgumentException);
}

TEST_F(SetConductivitiesToTetMeshTests, ThrowsForNullInput)
{
  auto test = makeModule("SetConductivitiesToTetMesh");
  ASSERT_TRUE(test != nullptr);
  FieldHandle nullField;
  stubPortNWithThisData(test, 0, nullField);
  stubPortNWithThisData(test, 1, nullField);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);
}
