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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/BrainStimulator/SetConductivitiesToTetMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
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
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }
  FieldHandle CreateTetMeshSevenElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/tet_mesh_7elem.fld");
  }
  // scalar fields covered by the algorithm tests
}

TEST_F(SetConductivitiesToTetMeshTests, TetMeshScalarSevenElem)
{
  UseRealAlgorithmFactory f;

  auto test = makeModule("SetConductivitiesToMesh");
  stubPortNWithThisData(test, 0, CreateTetMeshSevenElem());

  EXPECT_NO_THROW(test->execute());
}

TEST_F(SetConductivitiesToTetMeshTests, TetMeshVectorOnElem)
{
  UseRealAlgorithmFactory f;

  auto test = makeModule("SetConductivitiesToMesh");
  stubPortNWithThisData(test, 0, CreateTetMeshVectorOnElem());

  EXPECT_THROW(test->execute(), AlgorithmInputException);
}

TEST_F(SetConductivitiesToTetMeshTests, CheckInputParameters)
{
  UseRealAlgorithmFactory f;

  auto test = makeModule("SetConductivitiesToMesh");
  stubPortNWithThisData(test, 0, CreateTetMeshVectorOnElem());

  EXPECT_THROW(test->execute(), AlgorithmInputException);
}

TEST_F(SetConductivitiesToTetMeshTests, SparseRowMatrixInput)
{
  auto test = makeModule("SetConductivitiesToMesh");
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
  stubPortNWithThisData(test, 0, m);
  EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);
}

TEST_F(SetConductivitiesToTetMeshTests, ThrowsForNullInput)
{
  auto test = makeModule("SetConductivitiesToMesh");
  ASSERT_TRUE(test != nullptr);
  FieldHandle nullField;
  stubPortNWithThisData(test, 0, nullField);
  stubPortNWithThisData(test, 1, nullField);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);
}
