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
#include <Modules/BrainStimulator/SetupRHSforTDCSandTMS.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using ::testing::Mock;

class SetupRHSforTDCSandTMSTests : public ModuleTest
{

};

namespace
{
  FieldHandle CreateTetMeshVectorOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }
  FieldHandle CreateTetMeshVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }
  FieldHandle CreateTetMeshScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
  FieldHandle CreateTriSurfVectorOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_elem/vector/tri_vector_on_elem.fld");
  }
  FieldHandle CreateTriSurfVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }
  FieldHandle CreateTriSurfScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_elem/scalar/tri_scalar_on_elem.fld");
  }
  FieldHandle CreateTriSurfScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }
}

TEST_F(SetupRHSforTDCSandTMSTests, TetMeshVectorOnElem)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTetMeshVectorOnElem());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, TetMeshVectorOnNode)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTetMeshVectorOnNode());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, TetMeshScalarOnElem)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTetMeshScalarOnElem());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, TetMeshScalarOnNode)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTetMeshScalarOnNode());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, TriSurfVectorOnElem)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTriSurfVectorOnElem());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, TriSurfVectorOnNode)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTriSurfVectorOnNode());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, TriSurfScalarOnElem)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTriSurfScalarOnElem());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, TriSurfScalarOnNode)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  stubPortNWithThisData(tdcs, 0, CreateTriSurfScalarOnNode());
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, ThrowsForMatrixInput)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 40;
  stubPortNWithThisData(tdcs, 0, m);
  stubPortNWithThisData(tdcs, 1, m);
  EXPECT_THROW(tdcs->execute(), WrongDatatypeOnPortException);
}

TEST_F(SetupRHSforTDCSandTMSTests, ThrowsForNullInput)
{
  auto tdcs = makeModule("SetupRHSforTDCSandTMS");
  FieldHandle nullField;
  stubPortNWithThisData(tdcs, 0, nullField);
  stubPortNWithThisData(tdcs, 1, nullField);
  EXPECT_THROW(tdcs->execute(), NullHandleOnPortException);
}