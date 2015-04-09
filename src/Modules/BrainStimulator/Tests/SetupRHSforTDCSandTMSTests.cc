/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
  FieldHandle LoadMickeyMouseCleaverMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/mickey_mouse/cleaver_mesh.fld");
  }
  FieldHandle LoadMickeyMouseScalpMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/mickey_mouse/scalp_surf_mesh.fld");
  }
  FieldHandle LoadMickeyMouseElectrodeSpongeMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/mickey_mouse/elc_surf_mesh.fld");
  }
  
  DenseMatrixHandle ElectrodeSpongeLocationAndThickness()
  { 
   DenseMatrixHandle m(boost::make_shared<DenseMatrix>(6,4));
   (*m)(0,0) = 33.5;
   (*m)(0,1) = 33.5;
   (*m)(0,2) = 18.246;
   (*m)(0,3) = 3.0;   
   (*m)(1,0) = 33.5;
   (*m)(1,1) = 18.246;
   (*m)(1,2) = 33.5;
   (*m)(1,3) = 3.0;   
   (*m)(2,0) = 18.246;
   (*m)(2,1) = 33.5;
   (*m)(2,2) = 33.5;
   (*m)(2,3) = 3.0;  
   (*m)(3,0) = 48.754;
   (*m)(3,1) = 33.5;
   (*m)(3,2) = 33.5;
   (*m)(3,3) = 3.0;  
   (*m)(4,0) = 33.5;
   (*m)(4,1) = 48.754;
   (*m)(4,2) = 33.5;
   (*m)(4,3) = 3.0;
   (*m)(5,0) = 33.5;
   (*m)(5,1) = 33.5;
   (*m)(5,2) = 48.754;
   (*m)(5,3) = 3.0;

   return m;
  }
}

TEST_F(SetupRHSforTDCSandTMSTests, Correct)
{
  auto tdcs = makeModule("SetupTDCS");
  stubPortNWithThisData(tdcs, 0, LoadMickeyMouseCleaverMesh());
  stubPortNWithThisData(tdcs, 1, LoadMickeyMouseScalpMesh());
  stubPortNWithThisData(tdcs, 2, LoadMickeyMouseElectrodeSpongeMesh());
  stubPortNWithThisData(tdcs, 3, ElectrodeSpongeLocationAndThickness());
  EXPECT_NO_THROW(tdcs->execute());
}

TEST_F(SetupRHSforTDCSandTMSTests, ThrowsForWrongType)
{
  auto tdcs = makeModule("SetupTDCS");

  stubPortNWithThisData(tdcs, 0, ElectrodeSpongeLocationAndThickness());
  stubPortNWithThisData(tdcs, 1, ElectrodeSpongeLocationAndThickness());
  stubPortNWithThisData(tdcs, 2, ElectrodeSpongeLocationAndThickness());
  stubPortNWithThisData(tdcs, 3, LoadMickeyMouseCleaverMesh());
  
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

