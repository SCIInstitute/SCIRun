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

#include <gtest/gtest.h>

#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/ConeWidget.h>
#include <Graphics/Widgets/CylinderWidget.h>
#include <Graphics/Widgets/DiskWidget.h>
#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Graphics/Widgets/Tests/WidgetTestingUtility.h>

using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

TEST(ArrowWidgetTest, CanCreateSingleArrowReal)
{
  StubGeometryIDGenerator idGen;
  auto rgf = boost::make_shared<RealGlyphFactory>();
  WidgetFactory::setGlyphFactory(rgf);

  ArrowWidget arrow({{idGen, "testArrow1"}, rgf},
  {
    {10.0, "", {0,1,2}, {{0,0,0}, {1,1,1}}, 10},
    {1,1,0}, {2,2,0}, true, 2, 4
  });

  EXPECT_EQ(Point(16,16,0), arrow.position());
  EXPECT_EQ("<dummyGeomId>testArrow1widget[1 1 0][2 2 0]0", arrow.name());
}

TEST(ArrowWidgetTest, CanCreateSingleArrowStubbed)
{
  StubGeometryIDGenerator idGen;
  auto sgf = boost::make_shared<StubGlyphFactory>();
  WidgetFactory::setGlyphFactory(sgf);

  ArrowWidget arrow({{idGen, "testArrow1"}, sgf},
  {
    {10.0, "", {0,1,2}, {{0,0,0}, {1,1,1}}, 10},
    {1,1,0}, {2,2,0}, true, 2, 4
  });

  EXPECT_EQ(Point(16,16,0), arrow.position());
  //arrow does not get name from factory--generates it in constructor.
  EXPECT_EQ("<dummyGeomId>testArrow1widget[1 1 0][2 2 0]0", arrow.name());
}

TEST(ArrowWidgetTest, ArrowComponentsObserveEachOther)
{
  StubGeometryIDGenerator idGen;

  Point origin(0,0,0);
  CommonWidgetParameters common
  {
    1.0, "red", origin,
    {origin, Point(origin + Point{1,1,1})},
    10
  };
  ArrowParameters arrowParams
  {
    common,
    origin, Vector{1,1,1}, true, 2, 4
  };
  auto arrow = WidgetFactory::createArrowWidget(
    {idGen, "testArrow1"},
    arrowParams
  );

  ASSERT_TRUE(arrow != nullptr);

  auto compositeArrow = boost::dynamic_pointer_cast<CompositeWidget>(arrow);
  ASSERT_TRUE(compositeArrow != nullptr);

  WidgetList internals(compositeArrow->subwidgetBegin(), compositeArrow->subwidgetEnd());
  ASSERT_EQ(internals.size(), 4);

  auto sphere = boost::dynamic_pointer_cast<SphereWidget>(internals[0]);
  ASSERT_TRUE(sphere != nullptr);
  EXPECT_EQ("__sphere__4", sphere->name());
  EXPECT_EQ("<dummyGeomId>SphereWidget::ArrowWidget(0)(2)(4)", sphere->uniqueID());
  auto shaft = boost::dynamic_pointer_cast<CylinderWidget>(internals[1]);
  ASSERT_TRUE(shaft != nullptr);
  EXPECT_EQ("__cylinder__5", shaft->name());
  EXPECT_EQ("<dummyGeomId>CylinderWidget::ArrowWidget(1)(2)(4)", shaft->uniqueID());
  auto cone = boost::dynamic_pointer_cast<ConeWidget>(internals[2]);
  ASSERT_TRUE(cone != nullptr);
  EXPECT_EQ("__cone__6", cone->name());
  EXPECT_EQ("<dummyGeomId>ConeWidget::ArrowWidget(2)(2)(4)", cone->uniqueID());
  auto disk = boost::dynamic_pointer_cast<DiskWidget>(internals[3]);
  ASSERT_TRUE(disk != nullptr);
  EXPECT_EQ("__disk__7", disk->name());
  EXPECT_EQ("<dummyGeomId>DiskWidget::ArrowWidget(3)(2)(4)", disk->uniqueID());

  int eventCounter = 0;
  auto eventFunc = [&eventCounter](const std::string& id) { std::cout << "Translating: " << id << std::endl; eventCounter++; };
  SimpleWidgetEvent transEvent{WidgetMovement::TRANSLATE, eventFunc};
  // sphere takes translate events
  sphere->propagateEvent(transEvent);

  EXPECT_EQ(eventCounter, internals.size());

  eventCounter = 0;
  // shaft takes translate events too
  shaft->propagateEvent(transEvent);
  EXPECT_EQ(eventCounter, internals.size());

  eventCounter = 0;
  // cone takes rotate events
  cone->propagateEvent({WidgetMovement::ROTATE, [&eventCounter](const std::string& id) { std::cout << "Rotating: " << id << std::endl; eventCounter++; }});
  EXPECT_EQ(eventCounter, internals.size());

  eventCounter = 0;
  // disk takes scale events
  disk->propagateEvent({WidgetMovement::SCALE, [&eventCounter](const std::string& id) { std::cout << "Scaling: " << id << std::endl; eventCounter++; }});
  EXPECT_EQ(eventCounter, internals.size());
}
