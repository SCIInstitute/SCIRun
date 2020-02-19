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
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class CreateLatVolModuleTests : public ModuleTest
{

};

TEST_F(CreateLatVolModuleTests, ThrowsForNullInput)
{
  auto clv = makeModule("CreateLatVol");
  FieldHandle nullField;
  stubPortNWithThisData(clv, 0, nullField);
  stubPortNWithThisData(clv, 1, nullField);

  EXPECT_THROW(clv->execute(), NullHandleOnPortException);
}

TEST_F(CreateLatVolModuleTests, DefaultLatVolCreation)
{
  auto clv = makeModule("CreateLatVol");

  clv->execute();
  auto output = getDataOnThisOutputPort(clv, 0);
  ASSERT_TRUE(output != nullptr);

  auto latvol = boost::dynamic_pointer_cast<SCIRun::Field>(output);
  ASSERT_TRUE(latvol != nullptr);
  FieldInformation info(latvol);
  EXPECT_TRUE(info.is_latvolmesh());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("LatVolMesh<HexTrilinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("LatVolMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point>>,HexTrilinearLgn<double>,FData3d<double,LatVolMesh<HexTrilinearLgn<Point>>>>", info.get_field_type_id());
}
