/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#include <Modules/Visualization/ShowField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Utils/Exception.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
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
    Log::get().setVerbose(false);
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

class ModuleLevelUniqueIDGenerator
{
public:
  ModuleLevelUniqueIDGenerator(const ModuleInterface& module, const std::string& name) :
    module_(module), name_(name)
  {}
  std::string operator()() const { return generateModuleLevelUniqueID(module_, name_); }
private:
  const ModuleInterface& module_;
  std::string name_;
  static std::hash<std::string> hash_;
  std::string generateModuleLevelUniqueID(const ModuleInterface& module, const std::string& name) const;
};

std::hash<std::string> ModuleLevelUniqueIDGenerator::hash_;

std::string ModuleLevelUniqueIDGenerator::generateModuleLevelUniqueID(const ModuleInterface& module, const std::string& name) const
{
  std::ostringstream ostr;
  ostr << name << "_" << module.get_id() << "__";

  std::ostringstream toHash;
  toHash << "Data{";
  for (const auto& input : module.inputPorts())
  {
    auto data = input->getData();
    auto dataID = data ? (*data ? (*data)->id() : -1) : -2;
    toHash << "[" << input->get_portname() << "]:" << dataID << "_";
  }

  toHash << "}__State{";
  auto state = module.get_state();
  for (const auto& key : state->getKeys())
  {
    toHash << key << "->" << state->getValue(key).value() << "_";
  }
  toHash << "}";

  //std::cout << "trying to hash: " << toHash.str() << std::endl;

  ostr << hash_(toHash.str());

  return ostr.str();
}

class GeometryIDGenerator
{
public:
  virtual ~GeometryIDGenerator() {}
  virtual std::string generateGeometryID(const std::string& description) const = 0;
};

class ModuleWithGeometryOutput : public GeometryIDGenerator // : public Module
{
public:
  virtual std::string generateGeometryID(const std::string& description) const override;
};

class ShowFieldStateGeometryNameSynchronizationTest : public ModuleTest
{
protected:
  virtual void SetUp()
  {
    Log::get().setVerbose(false);
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

  showField->get_state()->setValue(ShowFieldModule::CylinderRadius, 2);

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
