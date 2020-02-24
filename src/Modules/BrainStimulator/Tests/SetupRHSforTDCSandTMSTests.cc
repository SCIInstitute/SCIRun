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
  FieldHandle LoadColin27ScalpSurfMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/setuptdcs/colin27_skin_surf.fld");
  }
  FieldHandle LoadColin27ElecSurfMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/setuptdcs/colin27_elc_surf.fld");
  }
  FieldHandle LoadColin27TetMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/setuptdcs/elc_tet.fld");
  }

  DenseMatrixHandle ElectrodeSpongeLocationAndThickness()
  {
   DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2,4));
   (*m)(0,0) = 59.5343;
   (*m)(0,1) = 15.5240;
   (*m)(0,2) = 84.5013;
   (*m)(0,3) = 4.0;
   (*m)(1,0) = 122.798;
   (*m)(1,1) = 32.2019;
   (*m)(1,2) = 128.798;
   (*m)(1,3) = 4.0;

   return m;
  }
}

TEST_F(SetupRHSforTDCSandTMSTests, Correct)
{
  auto tdcs = makeModule("SetupTDCS");
  stubPortNWithThisData(tdcs, 0, LoadColin27TetMesh());
  stubPortNWithThisData(tdcs, 1, LoadColin27ScalpSurfMesh());
  stubPortNWithThisData(tdcs, 2, LoadColin27ElecSurfMesh());
  stubPortNWithThisData(tdcs, 3, ElectrodeSpongeLocationAndThickness());
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, ThrowsForWrongType)
{
  auto tdcs = makeModule("SetupTDCS");

  stubPortNWithThisData(tdcs, 0, ElectrodeSpongeLocationAndThickness());
  stubPortNWithThisData(tdcs, 1, ElectrodeSpongeLocationAndThickness());
  stubPortNWithThisData(tdcs, 2, ElectrodeSpongeLocationAndThickness());
  stubPortNWithThisData(tdcs, 3, LoadColin27ElecSurfMesh());

  EXPECT_THROW(tdcs->execute(), WrongDatatypeOnPortException);
}

TEST_F(SetupRHSforTDCSandTMSTests, ThrowsForNullInput)
{
  auto tdcs = makeModule("SetupTDCS");
  FieldHandle nullField;
  stubPortNWithThisData(tdcs, 0, nullField);
  stubPortNWithThisData(tdcs, 1, nullField);
  stubPortNWithThisData(tdcs, 2, nullField);
  stubPortNWithThisData(tdcs, 3, nullField);
  EXPECT_THROW(tdcs->execute(), NullHandleOnPortException);
}
