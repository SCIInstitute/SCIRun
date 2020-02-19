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
#include <Modules/Visualization/ShowField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Utils/Exception.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Algorithms::Visualization::Parameters;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core;
using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using ::testing::Values;
using ::testing::Combine;
using ::testing::Range;

class ShowFieldScalingTest : public ParameterizedModuleTest<int>
{
protected:
  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);
    showField = makeModule("ShowField");
    showField->setStateDefaults();
    showField->get_state()->setValue(ShowEdges, false);
    auto size = GetParam();
    latVol = CreateEmptyLatVol(size, size, size);
    stubPortNWithThisData(showField, 0, latVol);
    LOG_DEBUG("Setting up ShowField with size {}^3 latvol", size);
  }

  UseRealModuleStateFactory f;
  ModuleHandle showField;
  FieldHandle latVol;
};

TEST_P(ShowFieldScalingTest, ConstructLatVolGeometry)
{
  LOG_DEBUG("Start ShowField::execute");
  showField->execute();
  LOG_DEBUG("End ShowField::execute");
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldScalingTest,
  Values(20, 40, 60, 80, 100, 120 //,150 //,200 //,256
  )
  );

class ShowFieldStateGeometryNameSynchronizationTest : public ModuleTest
{
protected:
  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);
    showField = makeModule("ShowField");
    showField->setStateDefaults();
    auto size = 2;
    latVol = CreateEmptyLatVol(size, size, size);
    stubPortNWithThisData(showField, 0, latVol);
  }

  UseRealModuleStateFactory f;
  ModuleHandle showField;
  FieldHandle latVol;
};

TEST_F(ShowFieldStateGeometryNameSynchronizationTest, GeometryNameSynchronizesWithShowFieldState)
{
  ModuleLevelUniqueIDGenerator generator(*showField, "EntireField");
  auto hash1 = generator();
  auto hash2NoChange = generator();
  EXPECT_EQ(hash1, hash2NoChange);

  showField->get_state()->setValue(Parameters::CylinderRadius, 2);

  auto stateChangeShouldBeDifferent = generator();
  EXPECT_NE(hash2NoChange, stateChangeShouldBeDifferent);
  EXPECT_EQ(stateChangeShouldBeDifferent, generator());

  auto size = 3;
  latVol = CreateEmptyLatVol(size, size, size);
  stubPortNWithThisData(showField, 0, latVol);

  auto inputChangeShouldBeDifferent = generator();
  EXPECT_NE(stateChangeShouldBeDifferent, inputChangeShouldBeDifferent);

  stubPortNWithThisData(showField, 1, ColorMapHandle());
  auto addInputShouldBeDifferent = generator();
  EXPECT_NE(inputChangeShouldBeDifferent, addInputShouldBeDifferent);

  EXPECT_NE(hash1, addInputShouldBeDifferent);
  EXPECT_NE(inputChangeShouldBeDifferent, hash1);
}

class ShowFieldPreformaceTest : public ModuleTest {};
TEST_F(ShowFieldPreformaceTest, TestFacePreformace)
{
  LogSettings::Instance().setVerbose(false);
  UseRealModuleStateFactory f;
  ModuleHandle showField = makeModule("ShowField");
  showField->setStateDefaults();
  auto state = showField->get_state();
  state->setValue(ShowFaces, true);
  state->setValue(ShowEdges, false);
  state->setValue(ShowNodes, false);
  state->setValue(FacesColoring, 1);

  ColorMapHandle colorMap = StandardColorMapFactory::create();
  stubPortNWithThisData(showField, 1, colorMap);

  std::vector<FieldHandle> vectorOfInputData =
  {
    CreateEmptyLatVol(64, 64, 64)
  };

  std::vector<std::string> vectorOfInputDataNames =
  {
    "64x64x64 LatVol"
  };

  const static int NUM_RUNS = 16;

  for(int i = 0; i < vectorOfInputData.size(); ++i)
  {

    auto start = std::chrono::steady_clock::now();
    stubPortNWithThisData(showField, 0, vectorOfInputData[i]);
    for(int i = 0; i < NUM_RUNS; ++i)
    {
      showField->execute();
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    std::cout << vectorOfInputDataNames[i] << " : " << std::chrono::duration<double, std::milli>(diff).count()/NUM_RUNS << " ms\n";
  }
  std::cout << "\n";
}
