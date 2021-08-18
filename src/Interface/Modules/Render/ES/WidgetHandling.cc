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


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <es-log/trace-log.h>
// Needed for OpenGL include files on Travis:
#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/UndefiningX11Cruft.h>
#include <QOpenGLWidget>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/ES/SRCamera.h>
#include <Interface/Modules/Render/ES/comp/StaticClippingPlanes.h>
#include <Core/Application/Preferences/Preferences.h>

#include <Core/Datatypes/Feedback.h>
#include <Core/Logging/Log.h>
#include <Core/Application/Application.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Interface/Modules/Render/ES/RendererCollaborators.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Render;

ObjectTransformCalculatorFactory::ObjectTransformCalculatorFactory(BasicRendererObjectProvider* brop, const glm::vec2& initPos, float initW)
  : brop_(brop), initPos_(initPos), initW_(initW)
{
}

WidgetUpdateService::WidgetUpdateService(ObjectTransformer* transformer, const ScreenParams& screen) :
  transformer_(transformer), screen_(screen)
{
}

glm::mat4 WidgetUpdateService::getStaticCameraViewProjection() { return transformer_->getStaticCameraViewProjection(); }

void WidgetUpdateService::setCurrentWidget(WidgetHandle w)
{
  currentWidget_ = w;
}

void WidgetUpdateService::doPostSelectSetup(int x, int y, float depth)
{
  auto initialW = getInitialW(depth);
  auto initialPosition = screen_.positionFromClick(x, y);

  auto factory = makeShared<ObjectTransformCalculatorFactory>(this, initialPosition, initialW);
  auto tcf = std::make_shared<TransformCalculatorFamily>(currentWidget_->movementType(yetAnotherEnumConversion(buttonPushed_)), factory);
  event_ = makeShared<WidgetTransformEvent>(transformer_, tcf);
}

void WidgetUpdateService::updateWidget(int x, int y)
{
  if (event_)
  {
    event_->transformAt(x, y);
    currentWidget_->mediate(currentWidget_.get(), event_);
    widgetTransform_ = event_->getTransformFor(currentWidget_);
  }
}

class WidgetTransformEvent::WidgetTransformEventImpl
{
public:
  int x_ {0}, y_ {0};
  ObjectTransformer* transformer_{ nullptr };
  std::shared_ptr<TransformCalculatorFamily> calcFamily_;
  std::map<WidgetBase*, glm::mat4> transformsUsed_;
};

TransformCalculatorFamily::TransformCalculatorFamily(const WidgetMovementFamily& movements, ObjectTransformCalculatorFactoryPtr factory) :
  movements_(movements), factory_(factory)
{

}

ObjectTransformCalculatorPtr TransformCalculatorFamily::calcFor(WidgetBase* widget, WidgetMovement movement)
{
  auto shareIter = movements_.propagated.find(movement);
  if (shareIter == movements_.propagated.end())
    THROW_INVALID_ARGUMENT("That movement is not set up for this widget");
  if (calcs_.find(widget) == calcs_.end())
  {
    ObjectTransformCalculatorPtr calc;
    if (shareIter->second == WidgetMovementSharing::SHARED)
    {
      auto reuse = std::find_if(calcs_.begin(), calcs_.end(),
        [movement](const CalcMap::value_type& p) { return p.second->movementType() == movement; });
      if (reuse != calcs_.end())
        calc = reuse->second;
    }
    if (!calc)
      calc = factory_->create(movement, widget);
    calcs_[widget] = calc;
  }
  return calcs_[widget];
}

WidgetTransformEvent::WidgetTransformEvent(ObjectTransformer* transformer,
  std::shared_ptr<TransformCalculatorFamily> calcFamily) : impl_(new WidgetTransformEventImpl)
{
  impl_->transformer_ = transformer;
  impl_->calcFamily_ = calcFamily;
}

WidgetMovement WidgetTransformEvent::baseMovement() const
{
  return impl_->calcFamily_->baseMovement();
}

void WidgetTransformEvent::move(WidgetBase* widget, WidgetMovement moveType) const
{
  if (widget && moveType)
  {
    auto calc = impl_->calcFamily_->calcFor(widget, moveType);
    auto transform = calc->computeTransform(impl_->x_, impl_->y_);
    impl_->transformsUsed_[widget] = transform.transform;
    impl_->transformer_->modifyObject(widget->uniqueID(), transform);
  }
}

void WidgetTransformEvent::transformAt(int x, int y)
{
  impl_->x_ = x;
  impl_->y_ = y;
}

glm::mat4 WidgetTransformEvent::getTransformFor(WidgetHandle w) const
{
  auto tIter = impl_->transformsUsed_.find(w.get());
  return tIter != impl_->transformsUsed_.end() ? tIter->second : glm::mat4{1.0};
}

WidgetTransformEvent::~WidgetTransformEvent() = default;

void WidgetUpdateService::doInitialUpdate(int x, int y, float depth)
{
  doPostSelectSetup(x, y, depth);
  updateWidget(x, y);
}

template <class P>
static glm::vec3 toVec3(const P& p)
{
  return glm::vec3{ p.x(), p.y(), p.z() };
}

float WidgetUpdateService::getInitialW(float depth) const
{
  float zFar = camera_->getZFar();
  float zNear = camera_->getZNear();
  float z = -1.0 / (depth * (1.0 / zFar - 1.0 / zNear) + 1.0 / zNear);
  return -z;
}

//TODO: these need to be delay-built, on a per subwidget basis (I think)

ObjectTransformCalculatorPtr ObjectTransformCalculatorFactory::create(WidgetMovement movement, WidgetBase* baseWidget) const
{
  switch (movement)
  {
  case WidgetMovement::TRANSLATE:
    return makeShared<ObjectTranslationCalculator>(brop_, ObjectTranslationCalculator::Params({ initPos_, initW_, brop_->getStaticCameraViewProjection() }));
  case WidgetMovement::ROTATE:
    return makeShared<ObjectRotationCalculator>(brop_, ObjectRotationCalculator::Params({ initPos_, initW_, toVec3(getRotationOrigin(baseWidget->transformParameters())) }));
  case WidgetMovement::SCALE:
  {
    ObjectScaleCalculator::Params p;
    p.initialPosition_ = initPos_;
    p.w_ = initW_;
    auto widgetTransformParameters = baseWidget->transformParameters();
    p.flipAxisWorld_ = toVec3(getScaleFlipVector(widgetTransformParameters));
    p.originWorld_ = toVec3(getRotationOrigin(widgetTransformParameters));
    return makeShared<ObjectScaleCalculator>(brop_, p);
  }
  case WidgetMovement::SCALE_AXIS:
  {
    ObjectScaleAxisCalculator::Params p;
    p.initialPosition_ = initPos_;
    p.w_ = initW_;
    auto widgetTransformParameters = baseWidget->transformParameters();
    p.flipAxisWorld_ = toVec3(getScaleFlipVector(widgetTransformParameters));
    p.originWorld_ = toVec3(getRotationOrigin(widgetTransformParameters));
    p.axis_ = toVec3(getAxisVector(widgetTransformParameters));
    p.scaleAxisIndex_ = getAxisIndex(widgetTransformParameters);
    return makeShared<ObjectScaleAxisCalculator>(brop_, p);
  }
  case WidgetMovement::TRANSLATE_AXIS:
  {
    ObjectAxisTranslationCalculator::Params p;
    p.initialPosition_ = initPos_;
    p.w_ = initW_;
    auto widgetTransformParameters = baseWidget->transformParameters();
    p.axis_ = toVec3(getAxisVector(widgetTransformParameters));
    return makeShared<ObjectAxisTranslationCalculator>(brop_, p);
  }
  default:
    return nullptr;
  }
}

Graphics::Datatypes::WidgetInteraction SCIRun::Render::yetAnotherEnumConversion(MouseButton btn)
{
  switch (btn)
  {
    case MouseButton::LEFT:
      return Graphics::Datatypes::WidgetInteraction::CLICK;
    case MouseButton::RIGHT:
      return Graphics::Datatypes::WidgetInteraction::RIGHT_CLICK;
    default:
      return Graphics::Datatypes::WidgetInteraction::CLICK;
  }
}
