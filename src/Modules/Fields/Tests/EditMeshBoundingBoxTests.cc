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


#include <Core/Datatypes/Feedback.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Fields/EditMeshBoundingBox.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Dataflow::Networks;

class EditMeshBoundingBoxModuleTest : public ModuleTest
{
protected:
  ModuleHandle makeExecutedModule()
  {
    auto module = makeModule("EditMeshBoundingBox");
    stubPortNWithThisData(module, 0, CreateEmptyLatVol());
    module->get_state()->setTransientValue(EditMeshBoundingBox::ResetToInput, true);
    module->execute();
    return module;
  }

  void dragWidget(ModuleHandle module, WidgetMovement movement, const Transform& transform)
  {
    ViewSceneFeedback feedback;
    feedback.transform = transform;
    feedback.movementType = movement;
    feedback.selectionName = GeometryObject::delimiter + module->id().id_ + GeometryObject::delimiter;
    module->outputPorts()[1]->sendConnectionFeedback(feedback);
  }

  Vector outputSize(ModuleHandle module)
  {
    auto state = module->get_state();
    return Vector(state->getValue(EditMeshBoundingBox::OutputSizeX).toDouble(),
                  state->getValue(EditMeshBoundingBox::OutputSizeY).toDouble(),
                  state->getValue(EditMeshBoundingBox::OutputSizeZ).toDouble());
  }

  Transform uniformScale(double factor)
  {
    Transform scale;
    scale.pre_scale(Vector(factor, factor, factor));
    return scale;
  }

private:
  UseRealModuleStateFactory useRealState_;
};

TEST_F(EditMeshBoundingBoxModuleTest, ScaleDragUpdatesOutputSize)
{
  auto module = makeExecutedModule();
  const auto initial = outputSize(module);

  dragWidget(module, WidgetMovement::SCALE, uniformScale(2));

  const auto size = outputSize(module);
  EXPECT_NEAR(2 * initial.x(), size.x(), 1e-9);
  EXPECT_NEAR(2 * initial.y(), size.y(), 1e-9);
  EXPECT_NEAR(2 * initial.z(), size.z(), 1e-9);
}

TEST_F(EditMeshBoundingBoxModuleTest, TranslateDragKeepsOutputSize)
{
  auto module = makeExecutedModule();
  dragWidget(module, WidgetMovement::SCALE, uniformScale(2));
  const auto scaled = outputSize(module);

  Transform translate;
  translate.pre_translate(Vector(1, 2, 3));
  dragWidget(module, WidgetMovement::TRANSLATE, translate);

  const auto size = outputSize(module);
  EXPECT_NEAR(scaled.x(), size.x(), 1e-9);
  EXPECT_NEAR(scaled.y(), size.y(), 1e-9);
  EXPECT_NEAR(scaled.z(), size.z(), 1e-9);
}

TEST_F(EditMeshBoundingBoxModuleTest, SavedSizeSurvivesReload)
{
  auto saved = makeExecutedModule();

  auto reloaded = makeModule("EditMeshBoundingBox");
  stubPortNWithThisData(reloaded, 0, CreateEmptyLatVol());
  auto state = reloaded->get_state();
  for (const auto& name : {EditMeshBoundingBox::RotationTransformMatrix, EditMeshBoundingBox::TranslationPoint,
                           EditMeshBoundingBox::InverseFieldTransformMatrix, EditMeshBoundingBox::DataSaved})
    state->setValue(name, saved->get_state()->getValue(name).value());
  Transform savedScale;
  savedScale.pre_scale(Vector(2, 3, 4));
  state->setValue(EditMeshBoundingBox::ScaleTransformMatrix, savedScale.get_string());
  reloaded->execute();

  const auto size = outputSize(reloaded);
  EXPECT_NEAR(4, size.x(), 1e-6);
  EXPECT_NEAR(6, size.y(), 1e-6);
  EXPECT_NEAR(8, size.z(), 1e-6);
}
