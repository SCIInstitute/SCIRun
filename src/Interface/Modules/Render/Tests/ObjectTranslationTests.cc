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

static const ScreenParams screen_;

size_t ScreenParametersTest::getScreenWidthPixels() const { return screen_.width; }
size_t ScreenParametersTest::getScreenHeightPixels() const { return screen_.height; }
void ScreenParametersTest::calculateScreenSpaceCoords(int x_in, int y_in, float& x_out, float& y_out) { throw "not needed"; }
MouseMode ScreenParametersTest::getMouseMode() const { throw "not needed"; }

static const ScreenParametersTest spt;
static SRCamera camera_(&spt);

SRCamera& BasicRendererObjectProviderStub::camera() const { return camera_; }
const ScreenParams& BasicRendererObjectProviderStub::screen() const { return screen_; }

class TranslationTest : public ::testing::Test
{
public:
  TranslationTest()
  {
    // taken from a real viewscene value
    viewProj_[0] = glm::vec4{-2.51635,	-0.486686,	-0.332241,	-0.195739};
    viewProj_[1] = glm::vec4{-0.557466,	3.0272,	0.759605,	0.447519};
    viewProj_[2] = glm::vec4{-0.278562,	-1.66171,	1.48111,	0.872592};
    viewProj_[3] = glm::vec4{0.161495,	-0.0474197,	4.01229,	8.50435};
  }
protected:
  BasicRendererObjectProviderStub brop;
  glm::mat4 viewProj_;
};

TEST_F(TranslationTest, CanConstruct)
{
  TranslateParameters p {};
  ObjectTranslationCalculator translator(&brop, p);
}

bool SCIRun::RenderTesting::operator==(const glm::mat4& lhs, const glm::mat4& rhs)
{
  return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

TEST_F(TranslationTest, CanTranslateHorizontalFromOrigin)
{
  TranslateParameters p;
  p.initialPosition_ = glm::vec2{0,0};
  p.w_ = 1.0;
  p.viewProj = viewProj_;

  ObjectTranslationCalculator translator(&brop, p);

  auto t = translator.computeTransform(10, 0);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{1,	      0,	      0,	        0};
  expected[1] = glm::vec4{0,	      1,	      0,	        0};
  expected[2] = glm::vec4{0,	      0,	      1,	        0};
  expected[3] = glm::vec4{0.3227177858352661,	0.3292640745639801,	-0.09647537022829056,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(TranslationTest, CanTranslateVerticalFromOrigin)
{
  TranslateParameters p;
  p.initialPosition_ = glm::vec2{0,0};
  p.w_ = 1.0;
  p.viewProj = viewProj_;

  ObjectTranslationCalculator translator(&brop, p);

  auto t = translator.computeTransform(0, 10);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{1,	      0,	      0,	        0};
  expected[1] = glm::vec4{0,	      1,	      0,	        0};
  expected[2] = glm::vec4{0,	      0,	      1,	        0};
  expected[3] = glm::vec4{0.336086243391037,	0.3214852511882782,	-0.08948712050914764,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(TranslationTest, CanTranslateArbitraryFromOrigin)
{
  TranslateParameters p;
  p.initialPosition_ = glm::vec2{0,0};
  p.w_ = 1.0;
  p.viewProj = viewProj_;

  ObjectTranslationCalculator translator(&brop, p);

  auto t = translator.computeTransform(15, 10);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{1,	      0,	      0,	        0};
  expected[1] = glm::vec4{0,	      1,	      0,	        0};
  expected[2] = glm::vec4{0,	      0,	      1,	        0};
  expected[3] = glm::vec4{0.3185345828533173,	0.3175969123840332,	-0.09143009781837463,	1};

  EXPECT_TRUE(t.transform == expected);
}

TEST_F(TranslationTest, CanTranslateArbitraryFromArbitrary)
{
  TranslateParameters p;
  p.initialPosition_ = glm::vec2{15,30};
  p.w_ = 1.0;
  p.viewProj = viewProj_;

  ObjectTranslationCalculator translator(&brop, p);

  auto t = translator.computeTransform(42, -72);

  //std::cout << std::setprecision(16) << t.transform << std::endl;

  glm::mat4 expected;
  expected[0] = glm::vec4{1,	      0,	      0,	        0};
  expected[1] = glm::vec4{0,	      1,	      0,	        0};
  expected[2] = glm::vec4{0,	      0,	      1,	        0};
  expected[3] = glm::vec4{7.090302467346191,	-5.827230453491211,	4.57905387878418,	1};

  EXPECT_TRUE(t.transform == expected);
}
