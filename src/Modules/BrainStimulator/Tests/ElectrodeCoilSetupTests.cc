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


//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
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

class ElectrodeCoilSetupTests : public ModuleTest
{

};

namespace
{
  FieldHandle LoadMickeyScalpSurfMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/BrainStim/mickey_mesh.fld");
  }
  FieldHandle LoadMickeyTMSCoil()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/BrainStim/mickey_tms_coil.fld");
  }

  DenseMatrixHandle TMSCoilLocation()
  {
   DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,3));
   (*m)(0,0) = 33.6654;
   (*m)(0,1) = 34.625;
   (*m)(0,2) = 10.6;

   return m;
  }
}


TEST_F(ElectrodeCoilSetupTests, Correct)
{
  auto tms = makeModule("ElectrodeCoilSetup");
  stubPortNWithThisData(tms, 0, LoadMickeyScalpSurfMesh());
  stubPortNWithThisData(tms, 1, TMSCoilLocation());
  stubPortNWithThisData(tms, 2, LoadMickeyTMSCoil());
  EXPECT_NO_THROW(tms->execute());
}

TEST_F(ElectrodeCoilSetupTests, ThrowsForWrongType)
{
  auto tms = makeModule("ElectrodeCoilSetup");
  stubPortNWithThisData(tms, 0, TMSCoilLocation());
  stubPortNWithThisData(tms, 1, LoadMickeyScalpSurfMesh());
  stubPortNWithThisData(tms, 2, TMSCoilLocation());
  EXPECT_THROW(tms->execute(), WrongDatatypeOnPortException);
}

TEST_F(ElectrodeCoilSetupTests, ThrowsForNullInput)
{
  auto tms = makeModule("ElectrodeCoilSetup");
  FieldHandle nullField;
  stubPortNWithThisData(tms, 0, nullField);
  stubPortNWithThisData(tms, 1, nullField);
  stubPortNWithThisData(tms, 2, nullField);
  EXPECT_THROW(tms->execute(), NullHandleOnPortException);
}
