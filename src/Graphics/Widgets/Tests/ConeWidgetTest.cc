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

#include <Graphics/Widgets/ConeWidget.h>
#include <Graphics/Widgets/Tests/WidgetTestingUtility.h>

using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

TEST(ConeWidgetTest, CanCreateSingleConeReal)
{
  StubGeometryIDGenerator idGen;

  ConeWidget cone({{idGen, "testCone1"}, boost::make_shared<RealGlyphFactory>()},
  {
    {{10.0, "red", {1,2,3}, {{0,0,0}, {1,1,1}}, 10},
    {1,1,0}, {2,2,0}}, true
  });

  EXPECT_EQ(Point(1.5,1.5,0), cone.position());
  EXPECT_EQ("<dummyGeomId>ConeWidget::testCone1widget10100", cone.name());
}

TEST(ConeWidgetTest, CanCreateSingleConeStubbed)
{
  StubGeometryIDGenerator idGen;

  ConeWidget cone({{idGen, "testCone1"}, boost::make_shared<StubGlyphFactory>()},
  {
    {{10.0, "red", {1,2,3}, {{0,0,0}, {1,1,1}}, 10},
    {1,1,0}, {2,2,0}}, true
  });

  EXPECT_EQ(Point(1.5,1.5,0), cone.position());
  EXPECT_EQ("__cone__0", cone.name());
}

TEST(ConeWidgetTest, CanSetupConeForRotation)
{
  StubGeometryIDGenerator idGen;

  ConeParameters params {
    {{10.0, "red", {1,2,3}, {{0,0,0}, {1,1,1}}, 10},
    {1,1,0}, {2,2,0}}, true };

  ConeWidget cone({{idGen, "testCone1"}, boost::make_shared<RealGlyphFactory>()}, params);

  cone.setTransformParameters<Rotation>(params.cylinder.common.origin);

  auto rotationOrigin = getRotationOrigin(cone.transformParameters());

  EXPECT_EQ(params.cylinder.common.origin, rotationOrigin);
}
