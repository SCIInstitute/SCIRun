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

#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/Tests/WidgetTestingUtility.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Graphics/Widgets/WidgetBuilders.h>

using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

TEST(SphereWidgetTest, CanCreateSingleSphereReal)
{
  StubGeometryIDGenerator idGen;

  SphereWidget sphere({{idGen, "testSphere1"}, boost::make_shared<RealGlyphFactory>()},
  {
    {10.0, "red", {1,2,3}, {{0,0,0}, {1,1,1}}, 10},
    {-1,1,0}
  });

  EXPECT_EQ(Point(-1,1,0), sphere.position());
  EXPECT_EQ("<dummyGeomId>SphereWidget::testSphere1widget10100", sphere.name());
}

TEST(SphereWidgetTest, CanCreateSingleSphereWithBuilder)
{
  StubGeometryIDGenerator idGen;

  auto sphere = SphereWidgetBuilder(idGen)
    .tag("testSphere1")
    .scale(10.0)
    .defaultColor("red")
    .origin({1,2,3})
    .boundingBox({{0,0,0}, {1,1,1}})
    .resolution(10)
    .centerPoint({-1,1,0})
    .build();

  EXPECT_EQ(Point(-1,1,0), sphere->position());
  EXPECT_EQ("<dummyGeomId>SphereWidget::testSphere1widget10100", sphere->name());
}

TEST(SphereWidgetTest, CanCreateSingleSphereStubGlyphs)
{
  StubGeometryIDGenerator idGen;

  SphereWidget sphere({{idGen, "testSphere1"}, boost::make_shared<StubGlyphFactory>()},
  {
    {10.0, "red", {1,2,3}, {{0,0,0}, {1,1,1}}, 10},
    {-1,1,0}
  });

  EXPECT_EQ(Point(-1,1,0), sphere.position());
  EXPECT_EQ("__sphere__0", sphere.name());
}

TEST(WidgetMovementTreeTest, CanCreate)
{
  StubGeometryIDGenerator idGen;

  Point origin1(0,0,0);
  Point origin2(1,0,0);

  auto sphere1 = WidgetFactory::createSphere(
    {idGen, "testSphere1"},
    {
      {10.0, "red", origin1, {{0,0,0}, {1,1,1}}, 10},
      {-1,1,0}
    }
  );

  auto sphere2 = WidgetFactory::createSphere(
    {idGen, "testSphere2"},
    {
      {10.0, "red", origin2, {{0,0,0}, {1,1,1}}, 10},
      {-1,1,0}
    }
  );

  auto spheres = {sphere1, sphere2};
  auto dualSphere = WidgetFactory::createComposite({idGen, "spheres"},
    std::begin(spheres), std::end(spheres));

#if 0 // next week
  WidgetMovementTreeBuilder(sphere1) >> WidgetMovement::TRANSLATE, sphere2;

  int eventCounter = 0;
  auto eventFunc = [&eventCounter](const std::string& id) { std::cout << "Translating: " << id << std::endl; eventCounter++; };
  SimpleWidgetEvent transEvent{WidgetMovement::TRANSLATE, eventFunc};
  // sphere takes translate events
  sphere1->propagateEvent(transEvent);

  EXPECT_EQ(eventCounter, 2);
#endif
  FAIL() << "todo";
}
