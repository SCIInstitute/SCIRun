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

namespace
{
  WidgetInteraction yetAnotherEnumConversion(MouseButton btn)
  {
    switch (btn)
    {
    case MouseButton::MOUSE_LEFT:
      return WidgetInteraction::CLICK;
    case MouseButton::MOUSE_RIGHT:
      return WidgetInteraction::RIGHT_CLICK;
    default:
      return WidgetInteraction::CLICK;
    }
  }
}

void WidgetUpdateService::setCurrentWidget(Graphics::Datatypes::WidgetHandle w)
{
  currentWidget_ = w;
  movements_ = w->movementType(yetAnotherEnumConversion(buttonPushed_));
}

//gen::Transform WidgetTransformMapping::transformFor(WidgetMovement move) const
//{
//  auto t = transformsCalcs_.find(move);
//  return t != transformsCalcs_.end() ? t->second->computeTransform(x_, y_) : gen::Transform{};
//}

void WidgetUpdateService::doPostSelectSetup(int x, int y, float depth)
{
  logCritical("{} {} {} {}", __FUNCTION__, x, y, depth);
  auto initialW = getInitialW(depth);
  auto initialPosition = screen_.positionFromClick(x, y);

  auto factory = boost::make_shared<ObjectTransformCalculatorFactory>(this, initialPosition, initialW);

  //XYZ xyz(initialPosition, initialW, transformFactory_);

  //for (const auto& movement : movements_)
  //  currentTransformationCalculators_.emplace(movement, xyz.make(movement)(currentWidget_));
}

void WidgetUpdateService::updateWidget(int x, int y)
{
  logCritical("{} {} {}", __FUNCTION__, x, y);
  //if ()
  //if (!currentTransformationCalculators_.empty())
  //{
  //auto event = boost::make_shared<WidgetTransformMapping>(currentTransformationCalculators_, x, y);

  //  auto boundEvent = [&](const std::string& id)
  //  {
  //    transformer_->modifyObject(id, event->transformFor(movements_.front()));
  //  };
  //currentWidget_->mediate(currentWidget_.get(), event);
  //  widgetTransform_ = event->transformFor(movements_.front()).transform;
  //}
}

class WidgetTransformEvent::WidgetTransformEventImpl
{
public:
  WidgetMovement baseMovement_;
  int x_, y_;
  ObjectTransformer* transformer_{ nullptr };
  std::shared_ptr<LazyTransformCalculatorFamily> calcFamily_;
};

LazyTransformCalculatorFamily::LazyTransformCalculatorFamily(ObjectTransformCalculatorFactoryPtr factory) : factory_(factory)
{

}

ObjectTransformCalculatorPtr LazyTransformCalculatorFamily::calcFor(WidgetBase* widget, WidgetMovement movement)
{
  if (calcs_.find(widget) == calcs_.end())
  {
    calcs_[widget] = factory_->create(movement, widget);
  }
  return calcs_[widget];
}

WidgetTransformEvent::WidgetTransformEvent() : impl_(new WidgetTransformEventImpl)
{

}

Graphics::Datatypes::WidgetMovement WidgetTransformEvent::baseMovement() const
{
  return impl_->baseMovement_;
}

void WidgetTransformEvent::move(WidgetBase* widget, WidgetMovement moveType) const
{
  logCritical("{}", __FUNCTION__);
  if (widget)
  {
    auto calc = impl_->calcFamily_->calcFor(widget, moveType);
    impl_->transformer_->modifyObject(widget->uniqueID(), calc->computeTransform(impl_->x_, impl_->y_));
  }
}

void WidgetTransformEvent::transformAt(int x, int y)
{
  impl_->x_ = x;
  impl_->y_ = y;
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
    return boost::make_shared<ObjectTranslationCalculator>(brop_, ObjectTranslationCalculator::Params({ initPos_, initW_, brop_->getStaticCameraViewProjection() }));
  case WidgetMovement::ROTATE:
    return boost::make_shared<ObjectRotationCalculator>(brop_, ObjectRotationCalculator::Params({ initPos_, initW_, toVec3(getRotationOrigin(baseWidget->transformParameters())) }));
  case WidgetMovement::SCALE:
  {
    ObjectScaleCalculator::Params p;
    p.initialPosition_ = initPos_;
    p.w_ = initW_;
    auto widgetTransformParameters = baseWidget->transformParameters();
    p.flipAxisWorld_ = toVec3(getScaleFlipVector(widgetTransformParameters));
    p.originWorld_ = toVec3(getRotationOrigin(widgetTransformParameters));
    return boost::make_shared<ObjectScaleCalculator>(brop_, p);
  }
  default:
    return nullptr;
  }
}

LazyObjectTransformCalculator ObjectTransformCalculatorFactory::create(WidgetMovement movement)
{
  return [movement, this](WidgetBase* widget) { return create(movement, widget); };
}


//void LazyObjectTransformCalculator::provideWidget(WidgetBase* widget)
//{
//  if (widget)
//    lazyImpl_ = factory->create
//}

//gen::Transform LazyObjectTransformCalculator::computeTransform(int x, int y) const override
//{
//  if (lazyImpl_)
//    return lazyImpl_->computeTransform(x, y);
//  throw "no widget provided";
//}
