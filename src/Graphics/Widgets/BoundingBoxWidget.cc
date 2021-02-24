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
  : WidgetBase({gen.base.idGenerator, "BasicBoundingBox",
      {
        { WidgetInteraction::CLICK,
            singleMovementWidget(WidgetMovement::TRANSLATE) }
      }
    })
{
  name_ = gen.glyphMaker->basicBox(params, *this);
  setPosition(params.pos.center_);
}

// Pseudocode
#if 0
BoundingBoxWidget::BoundingBoxWidget(const GeneralWidgetParameters& gen,
  BasicBoundingBoxParameters params) : CompositeWidget(gen.base)
{
  // vertices--sphere widgets
  // left-click: scale
  // right-click: axis scale
  WidgetHandle v0,v1,v2,v3,v4,v5,v6,v7;

  // edges--cylinders
  // left-click: translate
  // right-click: axis translate
  WidgetHandle e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11;
  // build step:
  ...
  .transformMapping({
    {WidgetInteraction::CLICK, WidgetMovement::TRANSLATE},
    {WidgetInteraction::RIGHT_CLICK, WidgetMovement::AXIS_TRANSLATE}
  })
  ...
  auto edges = std::vector<WidgetHandle>{e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11};

  // faces part 1--disks
  // left-click: translate/bidirectional scale
  // right-click: translate/unidirectional scale
  WidgetHandle fd0, fd1, fd2, fd3, fd4, fd5;

  //TODO:
  // face spheres
  WidgetHandle fs0, fs1, fs2, fs3, fs4, fs5;

  for (auto& e : edges)
  {
    registerAllSiblingWidgetsForEvent(e, WidgetMovement::TRANSLATE);
  }

  // face 0 = {v0,v1,v2,v3}
  // axis translate
  fd0 translates along an axis--T0
  all v from face 0, {v0,v1,v2,v3}, also translate--T0
  all e from face 0, {e0,e1,e2,e3} also translate--T0
  opposite face {v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, is fixed--Id
  parallel edges--{e8,e9,e10,e11}--uni-scale/translate along axis--T1
  anything else on parallel faces {fs/fd...} --T0/2

  void wireTheAboveMovementScheme(clicked, translated, fixed?, uniscaled+translated, halfTranslated)
  {
    clicked << propagatesEvent<WidgetMovement::TRANSLATE>::to << TheseWidgets {translated};
    clicked << propagatesEvent<WidgetMovement::UNISCALED_TRANSLATED>::to << uniscaled+translated;
    clicked << propagatesEvent<WidgetMovement::MODIFIED_TRANSLATE>::to << halfTranslated;
  }

  wireTheAboveMovementScheme(fd0, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  //can we use mesh geometry functions to get all the relative components functionally?

  wireTheAboveMovementScheme(fd1, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
}
#endif
