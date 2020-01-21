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


#include <Modules/Legacy/Fields/SetFieldDataToConstantValue.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::TestUtils;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class SetFieldDataToConstantValueModuleTests : public ModuleTest
{

};

namespace
{
  // FieldHandle CreateTriSurfScalarOnNode()
  // {
  //   return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  // }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
  // FieldHandle CreatePointCloudeScalar()
  // {
  //   return loadFieldFromFile(TestResources::rootDir() / "Fields/point_cloud/scalar/pts_scalar.fld");
  // }
}

TEST_F(SetFieldDataToConstantValueModuleTests, TetMeshOnPortZero)
{
  auto test = makeModule("SetFieldDataToConstantValue");
  FieldHandle f = CreateTetMeshScalarOnNode();
  stubPortNWithThisData(test, 0, f);
  EXPECT_NO_THROW(test->execute());
}
