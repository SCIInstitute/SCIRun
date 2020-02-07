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
}
