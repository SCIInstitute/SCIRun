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


#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/GlyphFactory.h>

using namespace SCIRun;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

void BoxPosition::setPosition(const Point& center, const Point& right, const Point& down, const Point& in)
{
  center_ = center;
  right_ = right;
  down_ = down;
  in_ = in;
}

void BoxPosition::getPosition(Point& center, Point& right, Point& down, Point& in) const
{
  center = center_;
  right = right_;
  down = down_;
  in = in_;
}

BasicBoundingBoxWidget::BasicBoundingBoxWidget(const GeneralWidgetParameters& gen,
  BasicBoundingBoxParameters params)
  : WidgetBase({gen.base.idGenerator, "BasicBoundingBox", {{WidgetInteraction::CLICK, WidgetMovement::TRANSLATE}}})
{
  name_ = gen.glyphMaker->basicBox(params, *this);
  setPosition(params.pos.center_);
}
