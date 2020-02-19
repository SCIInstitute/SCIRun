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


#include <Modules/Legacy/Fields/ClipVolumeByIsovalue.h>
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

class ClipVolumeByIsovalueModuleTests : public ModuleTest
{
};

FieldHandle LoadTriangles()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld");
}

FieldHandle LoadTetrahedrals()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld");
}

FieldHandle LoadLatVol()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_latvol.fld");
}


TEST_F(ClipVolumeByIsovalueModuleTests, ClipVolumeByIsovalueTriangleNoThrow_Example1)
{
  auto test = makeModule("ClipVolumeByIsovalue");
  FieldHandle f=LoadTriangles();
  stubPortNWithThisData(test, 0, f);
  EXPECT_NO_THROW(test->execute());
  f=LoadTetrahedrals();
  stubPortNWithThisData(test, 0, f);
  EXPECT_NO_THROW(test->execute());
  f=LoadLatVol();
  stubPortNWithThisData(test, 0, f);
  EXPECT_NO_THROW(test->execute());
}

TEST_F(ClipVolumeByIsovalueModuleTests, ClipVolumeByIsovalueTriangleNoThrow_Example2)
{
  auto test = makeModule("ClipVolumeByIsovalue");
  FieldHandle f=LoadTetrahedrals();
  DenseMatrixHandle iso(new DenseMatrix(1,1));
  *iso << 1.0;
  stubPortNWithThisData(test, 0, f);
  stubPortNWithThisData(test, 1, iso);
  EXPECT_NO_THROW(test->execute());
}

TEST_F(ClipVolumeByIsovalueModuleTests, ThrowForWrongInput)
{
  auto test = makeModule("ClipVolumeByIsovalue");
	DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
	for (int i=0; i<3; i++)
		(*m)(i, 0) = 1;
  stubPortNWithThisData(test, 0, m);
  stubPortNWithThisData(test, 1, FieldHandle());
  EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);
}


TEST_F(ClipVolumeByIsovalueModuleTests, ThrowForNullPointerExeption)
{
  auto test = makeModule("ClipVolumeByIsovalue");
  FieldHandle nullfield;
  stubPortNWithThisData(test, 0, nullfield);
  stubPortNWithThisData(test, 1, nullfield);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);
}
