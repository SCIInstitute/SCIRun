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
#include <Graphics/Widgets/share.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable)
  : GeometryObjectSpire(idGenerator, tag, isClippable)
{
}

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Point& origin)
  : GeometryObjectSpire(idGenerator, tag, isClippable),
    origin_(glm::vec3(origin.x(), origin.y(), origin.z()))
{
}

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Point& pos, const Point& origin)
  : GeometryObjectSpire(idGenerator, tag, isClippable),
    origin_(glm::vec3(origin.x(), origin.y(), origin.z())),
    position_(pos)
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

void WidgetBase::setToScale(const Vector& flipAxis)
{
  movementType_ = WidgetMovement::SCALE;
  flipAxis_ = glm::vec3(flipAxis.x(), flipAxis.y(), flipAxis.z());
}

void WidgetBase::setToRotate()
{
  movementType_ = WidgetMovement::ROTATE;
}

void WidgetBase::setToTranslate()
{
  movementType_ = WidgetMovement::TRANSLATE;
}

glm::vec3 WidgetBase::getFlipVector()
{
  return flipAxis_;
}

WidgetMovement WidgetBase::getMovementType()
{
  return movementType_;
}

void WidgetBase::addInitialId() {
  auto ids = std::vector<std::string>(1);
  ids.push_back(uniqueID());
  connectedIds_ = ids;
}

void CompositeWidget::addToList(GeometryBaseHandle handle, GeomList& list)
{
  if (handle.get() == this)
  {
    list.insert(widgets_.begin(), widgets_.end());
  }
}

void CompositeWidget::addToList(WidgetHandle widget)
{
  widgets_.push_back(widget);
}

std::vector<std::string> CompositeWidget::getListOfConnectedIds()
{
  std::vector<std::string> ids(widgets_.size());
  for(int i = 0; i < ids.size(); i++)
  {
    ids[i] = widgets_[i]->uniqueID();
  }
  return ids;
}

CompositeWidget::~CompositeWidget()
{
}
