/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <Interface/Modules/Render/Tests/ObjectTransformationHelper.h>

using namespace SCIRun;
using namespace Render;
using namespace RenderTesting;

class ScalingTest : public ::testing::Test
{
public:
  ScalingTest()
  {
  }
protected:
  BasicRendererObjectProviderStub brop;
};

TEST_F(ScalingTest, CanConstruct)
{
  ScaleParameters p {};
  ObjectScaleCalculator scaler(&brop, p);
}

TEST_F(ScalingTest, CanScaleHorizontalFromOrigin)
{
  ScaleParameters p;
  p.initialPosition_ = glm::vec2{1,0};
  p.w_ = 1.0;
  p.flipAxisWorld_ = glm::vec3{1,1,1};
  p.originWorld_ = glm::vec3{0,0,0};

  ObjectScaleCalculator scaler(&brop, p);

  auto t = scaler.computeTransform(10, 0);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{-0.3229167461395264,	0.645833432674408,	0.6458333134651184,	0};
  expected[1] = glm::vec4{0.6458333134651184,	-0.3229167461395264,	0.645833432674408,	0};
  expected[2] = glm::vec4{0.645833432674408,	0.6458333134651184,	-0.3229167461395264,	0};
  expected[3] = glm::vec4{0,	0,	0,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(ScalingTest, CanScaleVerticalFromOrigin)
{
  ScaleParameters p;
  p.initialPosition_ = glm::vec2{1,0};
  p.w_ = 1.0;
  p.flipAxisWorld_ = glm::vec3{1,1,1};
  p.originWorld_ = glm::vec3{0,0,0};

  ObjectScaleCalculator scaler(&brop, p);

  auto t = scaler.computeTransform(1, 10);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;

  expected[0] = glm::vec4{-0.3322917520999908,	0.6645834445953369,	0.6645833253860474,	0};
  expected[1] = glm::vec4{0.6645833253860474,	-0.3322917520999908,	0.6645834445953369,	0};
  expected[2] = glm::vec4{0.6645834445953369,	0.6645833253860474,	-0.3322917520999908,	0};
  expected[3] = glm::vec4{0,	0,	0,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(ScalingTest, CanScaleThroughFlipAxis)
{
  ScaleParameters p;
  p.initialPosition_ = glm::vec2{1,0};
  p.w_ = 1.0;
  p.flipAxisWorld_ = glm::vec3{1,1,1};
  p.originWorld_ = glm::vec3{0,0,0};

  ObjectScaleCalculator scaler(&brop, p);

  auto t = scaler.computeTransform(10, 10);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{-0.322916716337204,	0.6458333730697632,	0.6458332538604736,	0};
  expected[1] = glm::vec4{0.6458332538604736,	-0.322916716337204,	0.6458333730697632,	0};
  expected[2] = glm::vec4{0.6458333730697632,	0.6458332538604736,	-0.322916716337204,	0};
  expected[3] = glm::vec4{0,	0,	0,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(ScalingTest, CanScaleArbitraryFromOrigin)
{
  ScaleParameters p;
  p.initialPosition_ = glm::vec2{1,0};
  p.w_ = 1.0;
  p.flipAxisWorld_ = glm::vec3{1,1,1};
  p.originWorld_ = glm::vec3{0,0,0};

  ObjectScaleCalculator scaler(&brop, p);

  auto t = scaler.computeTransform(-12, 42);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{-0.3458333909511566,	0.6916667222976685,	0.6916666030883789,	0};
  expected[1] = glm::vec4{0.6916666030883789,	-0.3458333909511566,	0.6916667222976685,	0};
  expected[2] = glm::vec4{0.6916667222976685,	0.6916666030883789,	-0.3458333909511566,	0};
  expected[3] = glm::vec4{0,	0,	0,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(ScalingTest, CanScaleArbitraryFromArbitrary)
{
  ScaleParameters p;
  p.initialPosition_ = glm::vec2{1,0};
  p.w_ = 1.0;
  p.flipAxisWorld_ = glm::vec3{1,1,1};
  p.originWorld_ = glm::vec3{1,2,3};

  ObjectScaleCalculator scaler(&brop, p);

  auto t = scaler.computeTransform(-30, 10);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  // almost identity matrix...
  glm::mat4 expected;
  expected[0] = glm::vec4{0.9999999403953552,	0,	0,	0};
  expected[1] = glm::vec4{0,	0.9999999403953552,	0,	0};
  expected[2] = glm::vec4{0,	0,	0.9999999403953552,	0};
  expected[3] = glm::vec4{5.960464477539062e-08,	1.192092895507812e-07,	2.384185791015625e-07,	1};

  EXPECT_TRUE(t.transform == expected);
}
