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

#include <Interface/Modules/Render/Vtk/VtkRendererCollaborators.h>
#include <Interface/Modules/Render/Vtk/VtkRenderer.h>
#include <Graphics/Widgets/Widget.h>
#include <Core/Datatypes/feedback.h>

using namespace SCIRun;
using namespace SCIRun::Render;
using namespace SCIRun::Core::Datatypes;

template <class P>
static glm::vec3 toVec3(const P& p)
{
  return glm::vec3{p.x(), p.y(), p.z()};
}

std::unique_ptr<VtkTransformCalculator> VtkTransformCalculatorFactory::Create(
    WidgetMovement movement,
    Graphics::Datatypes::WidgetHandle widget,
    const VtkInteractionContext& context,
    const glm::vec3& initialPickWorld)
{
  switch (movement)
  {
  case WidgetMovement::TRANSLATE:
  {
    return std::make_unique<VtkTranslationCalculator>(initialPickWorld);
  }

  case WidgetMovement::TRANSLATE_AXIS:
  {
    auto params = widget->transformParameters();

    return std::make_unique<VtkAxisTranslationCalculator>(initialPickWorld, toVec3(getAxisVector(params)));
  }

  case WidgetMovement::SCALE:
  {
    auto params = widget->transformParameters();

    return std::make_unique<VtkScaleCalculator>(toVec3(getRotationOrigin(params)), initialPickWorld);
  }

  case WidgetMovement::SCALE_AXIS:
  {
    auto params = widget->transformParameters();

    return std::make_unique<VtkScaleAxisCalculator>(toVec3(getRotationOrigin(params)), initialPickWorld, toVec3(getAxisVector(params)));
  }

  case WidgetMovement::ROTATE:
  {
    auto params = widget->transformParameters();

    return std::make_unique<VtkRotationCalculator>(toVec3(getRotationOrigin(params)), toVec3(getAxisVector(params)), initialPickWorld);
  }
  }
}

VtkWidgetUpdateService::VtkWidgetUpdateService(VtkRenderer* renderer)
    : renderer_(renderer),
    buttonPushed_(MouseButton::NONE)
{
}

void VtkWidgetUpdateService::setCurrentWidget(Graphics::Datatypes::WidgetHandle widget)
{
  currentWidget_ = widget;
}

Graphics::Datatypes::WidgetHandle VtkWidgetUpdateService::currentWidget() const
{
  return currentWidget_;
}

void VtkWidgetUpdateService::setButtonPushed(MouseButton button)
{
  buttonPushed_ = button;
}

glm::mat4 VtkWidgetUpdateService::widgetTransform() const
{
  return widgetTransform_;
}

void VtkWidgetUpdateService::reset()
{
  calculator_.reset();

  currentWidget_.reset();

  widgetTransform_ = glm::mat4(1.0f);

  initialPickWorld_ = glm::vec3(0.0f);

  lastPickWorld_ = glm::vec3(0.0f);

  buttonPushed_ = MouseButton::NONE;
}

static Graphics::Datatypes::WidgetInteraction ToWidgetInteraction(MouseButton button)
{
  switch (button)
  {
  case MouseButton::LEFT: return Graphics::Datatypes::WidgetInteraction::CLICK;

  case MouseButton::RIGHT: return Graphics::Datatypes::WidgetInteraction::RIGHT_CLICK;

  default: return Graphics::Datatypes::WidgetInteraction::CLICK;
  }
}

void VtkWidgetUpdateService::beginInteraction(int x, int y, vtkCellPicker* picker)
{
  if (!currentWidget_ || !picker || !renderer_) return;

  double pos[3];
  picker->GetPickPosition(pos);

  initialPickWorld_ =
    {static_cast<float>(pos[0]),
    static_cast<float>(pos[1]),
    static_cast<float>(pos[2])};

  lastPickWorld_ = initialPickWorld_;

  context_ = renderer_->getInteractionContext();
  context_.startMouse = glm::vec2(x, y);
  context_.startWorld = lastPickWorld_;
  context_.dragPlanePoint = lastPickWorld_;
  const auto viewDir = context_.focalPoint -
      context_.cameraPosition;
  if (glm::length(viewDir) > 1e-6f)
  {
    context_.dragPlaneNormal = glm::normalize(viewDir);
  }
  else
  {
    context_.dragPlaneNormal = glm::vec3(0.f, 0.f, -1.f);
  }

  const auto interaction =
      ToWidgetInteraction(buttonPushed_);

  const auto movement =
      currentWidget_->movementType(interaction).base;

  calculator_ =
      VtkTransformCalculatorFactory::Create(
          movement,
          currentWidget_,
          context_,
          initialPickWorld_);
}

void VtkWidgetUpdateService::updateWidget(int x, int y)
{
  if (!currentWidget_)
      return;

  if (!calculator_)
      return;

  glm::vec3 worldPos = renderer_->computeWorldPosition(x, y, context_.dragPlanePoint, context_.dragPlaneNormal);

  widgetTransform_ = calculator_->computeTransform(worldPos);
}

