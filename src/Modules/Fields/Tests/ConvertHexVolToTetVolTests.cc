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


#include <Modules/Legacy/Fields/ConvertQuadSurfToTriSurf.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class ConvertHexVolToTetVolModuleTests : public ModuleTest
{
 protected:

  FieldHandle CreateEmptyLatVol(size_type sizex = 3, size_type sizey = 4, size_type sizez = 5, data_info_type type=DOUBLE_E)
  {
    FieldInformation lfi(LATVOLMESH_E, LINEARDATA_E, type);
    Point minb(-1.0, -1.0, -1.0);
    Point maxb(1.0, 1.0, 1.0);
    MeshHandle mesh = CreateMesh(lfi, sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    return ofh;
  }

 DenseMatrixHandle CreateDenseMatrix()
 {
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,3));
  (*m)(0,0) = 1;
  (*m)(1,0) = 2;
  (*m)(2,0) = 3;
  return m;
 }
};

TEST_F(ConvertHexVolToTetVolModuleTests, ThrowsForNullInput)
{
  auto csdf = makeModule("ConvertHexVolToTetVol");
  FieldHandle nullField;
  stubPortNWithThisData(csdf, 0, nullField);

  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}

TEST_F(ConvertHexVolToTetVolModuleTests, WrongDataType)
{
  auto csdf = makeModule("ConvertHexVolToTetVol");
  stubPortNWithThisData(csdf, 0, CreateDenseMatrix());
  EXPECT_THROW(csdf->execute(), DataPortException);
}

TEST_F(ConvertHexVolToTetVolModuleTests, CorrectInput)
{
  auto csdf = makeModule("ConvertHexVolToTetVol");
  auto size=6;
  stubPortNWithThisData(csdf, 0, CreateEmptyLatVol(size, size, size, INT_E));
  EXPECT_NO_THROW(csdf->execute());
}
