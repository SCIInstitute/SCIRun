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


#include <Graphics/Widgets/Widget.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

WidgetBase::WidgetBase(const WidgetBaseParameters& params)
  : GeometryObjectSpire(params.idGenerator, params.tag, true),
  InputTransformMapper(params.mapping)
{
}

Point WidgetBase::position() const
{
  return position_;
}

void WidgetBase::setPosition(const Point& p)
{
  position_ = p;
}

void WidgetBase::propagateEvent(const SimpleWidgetEvent& e)
{
  e.func(uniqueID());
  notify(e);
}

void CompositeWidget::registerAllSiblingWidgetsForEvent(WidgetHandle selected, WidgetMovement movement)
{
  for (auto& subwidget : widgets_)
  {
    if (selected.get() != subwidget.get())
      selected->registerObserver(movement, subwidget.get());
  }
}

void CompositeWidget::addToList(GeometryBaseHandle handle, GeomList& list)
{
  if (handle.get() == this)
  {
    list.insert(widgets_.begin(), widgets_.end());
  }
}

void CompositeWidget::propagateEvent(const SimpleWidgetEvent& e)
{
  for (auto& w : widgets_)
    w->propagateEvent(e);
}

InputTransformMapper::InputTransformMapper(TransformMappingParams pairs)
  : interactionMap_(pairs)
{
}

WidgetMovement InputTransformMapper::movementType(WidgetInteraction interaction) const
{
  auto i = interactionMap_.find(interaction);
  return i != interactionMap_.cend() ? i->second : WidgetMovement::NONE;
}

Core::Geometry::Point SCIRun::Graphics::Datatypes::getRotationOrigin(const MultiTransformParameters& ts)
{
  auto rotIter = std::find_if(ts.begin(), ts.end(), [](TransformParametersPtr t) { return std::dynamic_pointer_cast<Rotation>(t) != nullptr; });
  if (rotIter != ts.end())
  {
    auto rot = std::dynamic_pointer_cast<Rotation>(*rotIter);
    return rot->origin;
  }
  return {};
}

Core::Geometry::Vector SCIRun::Graphics::Datatypes::getScaleFlipVector(const MultiTransformParameters& ts)
{
  auto scIter = std::find_if(ts.begin(), ts.end(), [](TransformParametersPtr t) { return std::dynamic_pointer_cast<Scaling>(t) != nullptr; });
  if (scIter != ts.end())
  {
    auto sc = std::dynamic_pointer_cast<Scaling>(*scIter);
    return sc->flip;
  }
  return {};
}

TransformPropagationProxy SCIRun::Graphics::Datatypes::operator<<(WidgetHandle widget, WidgetMovement movement)
{
  return { [=](WidgetHandle other) { widget->registerObserver(movement, other.get()); } };
}

TransformPropagationProxy SCIRun::Graphics::Datatypes::operator<<(const TransformPropagationProxy& proxy, WidgetHandle widget)
{
  proxy.registrationApplier(widget);
  return proxy;
}

TransformPropagationProxy SCIRun::Graphics::Datatypes::operator<<(const TransformPropagationProxy& proxy, const std::vector<WidgetHandle>& widgets)
{
  for (auto& widget : widgets)
    proxy.registrationApplier(widget);
  return proxy;
}
