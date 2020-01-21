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
#include <Modules/Visualization/RescaleColorMap.h>
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
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core;
using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using ::testing::Values;
using ::testing::Combine;
using ::testing::Range;

#if 0 // rescale might be able to be parameterized, keep this around until next milestone.
class ShowFieldScalingTest : public ParameterizedModuleTest<int>
{
protected:
  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);
    showField = makeModule("ShowField");
    showField->setStateDefaults();
    auto size = GetParam();
    latVol = CreateEmptyLatVol(size, size, size);
    stubPortNWithThisData(showField, 0, latVol);
    Log::get() << INFO << "Setting up ShowField with size " << size << "^3 latvol" << std::endl;
  }

  UseRealModuleStateFactory f;
  ModuleHandle showField;
  FieldHandle latVol;
};

TEST_P(ShowFieldScalingTest, ConstructLatVolGeometry)
{
  Log::get() << INFO << "Start ShowField::execute" << std::endl;
  showField->execute();
  Log::get() << INFO << "End ShowField::execute" << std::endl;
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldScalingTest,
  Values(20, 40, 60, 80
  //, 100, 120, 150//, //200 //to speed up make test
  //, 256 // probably runs out of memory
  )
  );
#endif


class RescaleColorMapModuleTest : public ModuleTest
{
protected:
  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);
    rescaleColorMap = makeModule("RescaleColorMap");
    rescaleColorMap->setStateDefaults();
    colorMap = StandardColorMapFactory::create();
    stubPortNWithThisData(rescaleColorMap, 0, colorMap);
    auto size = 2;
    latVol = CreateEmptyLatVol(size, size, size);
    stubPortNWithThisData(rescaleColorMap, 1, latVol);
  }

  UseRealModuleStateFactory f;
  ModuleHandle rescaleColorMap;
  FieldHandle latVol;
  ColorMapHandle colorMap;
};

TEST_F(RescaleColorMapModuleTest, ModuleRuns)
{
  rescaleColorMap->execute();
}

TEST_F(RescaleColorMapModuleTest, DISABLED_TODO)
{
  FAIL() << "insert more test code here";
}
