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

class RotationTest : public ::testing::Test
{
public:
  RotationTest()
  {
  }
protected:
  BasicRendererObjectProviderStub brop;
};

TEST_F(RotationTest, CanConstruct)
{
  RotateParameters p {};
  ObjectRotationCalculator rotator(&brop, p);
}

TEST_F(RotationTest, CanRotateHorizontalFromOrigin)
{
  RotateParameters p;
  p.initialPosition_ = glm::vec2{1,0};
  p.w_ = 1.0;
  p.originWorld_ = glm::vec3{0,0,0};

  ObjectRotationCalculator rotator(&brop, p);

  auto t = rotator.computeTransform(0, 0);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{0.9294896125793457,	0.05288278311491013,	0.3650376796722412,	0};
  expected[1] = glm::vec4{7.450580596923828e-09,	0.9896687269210815,	-0.1433727443218231,	0};
  expected[2] = glm::vec4{-0.3688483238220215,	0.1332634687423706,	0.9198868274688721,	0};
  expected[3] = glm::vec4{0,	0,	0,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(RotationTest, CanRotateVerticalFromOrigin)
{
  RotateParameters p;
  p.initialPosition_ = glm::vec2{0,1};
  p.w_ = 1.0;
  p.originWorld_ = glm::vec3{0,0,0};

  ObjectRotationCalculator rotator(&brop, p);

  auto t = rotator.computeTransform(0, 0);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{0.981933057308197,	0.02710044384002686,	0.1872780174016953,	0};
  expected[1] = glm::vec4{-0.02661072835326195,	0.999632716178894,	-0.005128937773406506,	0};
  expected[2] = glm::vec4{-0.1873482316732407,	5.266955122351646e-05,	0.9822935461997986,	0};
  expected[3] = glm::vec4{0,	0,	0,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(RotationTest, CanRotateArbitraryFromOrigin)
{
  RotateParameters p;
  p.initialPosition_ = glm::vec2{0,1};
  p.w_ = 1.0;
  p.originWorld_ = glm::vec3{0,0,0};

  ObjectRotationCalculator rotator(&brop, p);

  auto t = rotator.computeTransform(25, -42);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{0.9846640229225159,	0.02713697031140327,	0.1723381131887436,	0};
  expected[1] = glm::vec4{-0.02237801253795624,	0.9993143677711487,	-0.02949748001992702,	0};
  expected[2] = glm::vec4{-0.1730204075574875,	0.02518852241337299,	0.9845961332321167,	0};
  expected[3] = glm::vec4{0,	0,	0,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(RotationTest, CanRotateArbitraryFromArbitrary)
{
  RotateParameters p;
  p.initialPosition_ = glm::vec2{-1,1};
  p.w_ = 1.0;
  p.originWorld_ = glm::vec3{1,2,3};

  ObjectRotationCalculator rotator(&brop, p);

  auto t = rotator.computeTransform(25, -42);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{0.9994457364082336,	-0.003830079222097993,	0.03306873887777328,	0};
  expected[1] = glm::vec4{0.002393543953076005,	0.9990561604499817,	0.04337168484926224,	0};
  expected[2] = glm::vec4{-0.03320364654064178,	-0.04326849430799484,	0.9985115528106689,	0};
  expected[3] = glm::vec4{0.09537804126739502,	0.1355232000350952,	-0.1153466701507568,	1};

  EXPECT_TRUE(t.transform == expected);
}
