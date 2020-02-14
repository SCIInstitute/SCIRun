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


#include <Modules/BrainStimulator/SimulateForwardMagneticField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::TestUtils;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class SimulateForwardMagneticFieldModuleTests : public ModuleTest
{

};

namespace
{

 FieldHandle LoadFieldFirstModuleInput()
 {
  return loadFieldFromFile(TestResources::rootDir() / "Fields" / "simulateforwardmagneticfield/first.fld");
 }

 FieldHandle LoadFieldSecondModuleInput()
 {
  return loadFieldFromFile(TestResources::rootDir() / "Fields" / "simulateforwardmagneticfield/second.fld");
 }

 FieldHandle LoadFieldThirdModuleInput()
 {
  return loadFieldFromFile(TestResources::rootDir() / "Fields" / "simulateforwardmagneticfield/third.fld");
 }

 FieldHandle LoadFieldFourthModuleInput()
 {
  return loadFieldFromFile(TestResources::rootDir() / "Fields" / "simulateforwardmagneticfield/fourth.fld");
 }

}

TEST_F(SimulateForwardMagneticFieldModuleTests, ThrowsForNullInput)
{
  auto test = makeModule("SimulateForwardMagneticField");
  FieldHandle nullField;
  stubPortNWithThisData(test, 0, nullField);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);

  stubPortNWithThisData(test, 1, nullField);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);

  stubPortNWithThisData(test, 2, nullField);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);

  stubPortNWithThisData(test, 3, nullField);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);
}

TEST_F(SimulateForwardMagneticFieldModuleTests, WrongDataType)
{
  auto test = makeModule("SimulateForwardMagneticField");
  DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
	for (int i=0; i<3; i++)
		(*m)(i, 0) = 1;
  stubPortNWithThisData(test, 0, m);
  EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);

  stubPortNWithThisData(test, 1, m);
  EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);

  stubPortNWithThisData(test, 2, m);
  EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);

  stubPortNWithThisData(test, 3, m);
  EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);
}

TEST_F(SimulateForwardMagneticFieldModuleTests, NothingOnPortZeroOneTwo)
{
  auto sls = makeModule("SimulateForwardMagneticField");
  EXPECT_THROW(sls->execute(), DataPortException);
}

TEST_F(SimulateForwardMagneticFieldModuleTests, NormalExecution)
{
  auto test = makeModule("SimulateForwardMagneticField");
  FieldHandle f0 = LoadFieldFirstModuleInput();
  stubPortNWithThisData(test, 0, f0);

  FieldHandle f1 = LoadFieldSecondModuleInput();
  stubPortNWithThisData(test, 1, f1);

  FieldHandle f2 = LoadFieldThirdModuleInput();
  stubPortNWithThisData(test, 2, f2);

  FieldHandle f3 = LoadFieldFourthModuleInput();
  stubPortNWithThisData(test, 3, f3);

  EXPECT_NO_THROW(test->execute());
}
