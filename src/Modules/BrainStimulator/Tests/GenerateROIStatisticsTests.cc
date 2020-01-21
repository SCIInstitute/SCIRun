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
#include <Core/Datatypes/String.h>
#include <Modules/BrainStimulator/GenerateROIStatistics.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <string>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class GenerateROIStatisticsTests : public ModuleTest
{

};

namespace
{
  FieldHandle CreateGetMeshSolution()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/mesh_sol.fld");
  }

  FieldHandle CreateCoordinatesCubesShiftedBy10()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/coordinates_cubes_shifted_by_10.fld");
  }

  FieldHandle CreateCoordinatesCubes()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/coordinates_cubes.fld");
  }

  FieldHandle CreateAtlas()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/atlas.fld");
  }

  // scalar fields covered by the algorithm tests
}

TEST_F(GenerateROIStatisticsTests, WrongDatatypeOnPortException)
{
  auto test = makeModule("GenerateROIStatistics");

  FieldHandle atlas = CreateAtlas();
  FieldHandle createcoordcubes = CreateCoordinatesCubes();
  FieldHandle createcoordcubesshiftby10 = CreateCoordinatesCubesShiftedBy10();
  FieldHandle createmeshsolution = CreateGetMeshSolution();
  StringHandle str1(new String("Region1;Region2;Region3;Region4;Region5;Region6;Region7;Region8"));
  StringHandle str2(new String("[A/m*m]"));
  StringHandle str3(new String("Talairach"));
  stubPortNWithThisData(test, 0, str2);
  stubPortNWithThisData(test, 1, createmeshsolution);
  stubPortNWithThisData(test, 2, str1);
  stubPortNWithThisData(test, 3, atlas);
  stubPortNWithThisData(test, 4, str3);
  stubPortNWithThisData(test, 5, createcoordcubes);


  EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);

}


TEST_F(GenerateROIStatisticsTests, RunItThroughWithValidInputs)
{
  auto test = makeModule("GenerateROIStatistics");

  FieldHandle atlas = CreateAtlas();
  FieldHandle createcoordcubes = CreateCoordinatesCubes();
  FieldHandle createcoordcubesshiftby10 = CreateCoordinatesCubesShiftedBy10();
  FieldHandle createmeshsolution = CreateGetMeshSolution();
  StringHandle str1(new String("Region1;Region2;Region3;Region4;Region5;Region6;Region7;Region8"));
  StringHandle str2(new String("[A/m*m]"));
  StringHandle str3(new String("Talairach"));
  stubPortNWithThisData(test, 0, createmeshsolution);
  stubPortNWithThisData(test, 1, str2);
  stubPortNWithThisData(test, 2, atlas);
  stubPortNWithThisData(test, 3, str1);
  stubPortNWithThisData(test, 4, createcoordcubes);
  stubPortNWithThisData(test, 5, str3);

  EXPECT_NO_THROW(test->execute());
}

TEST_F(GenerateROIStatisticsTests, ThrowsForNullInput)
{
  auto test = makeModule("GenerateROIStatistics");
  ASSERT_TRUE(test != nullptr);
  FieldHandle nullField;
  StringHandle nullString;
  stubPortNWithThisData(test, 0, nullField);
  stubPortNWithThisData(test, 1, nullString);
  stubPortNWithThisData(test, 2, nullField);
  stubPortNWithThisData(test, 3, nullString);
  stubPortNWithThisData(test, 4, nullField);
  stubPortNWithThisData(test, 5, nullString);
  EXPECT_THROW(test->execute(), NullHandleOnPortException);
}
