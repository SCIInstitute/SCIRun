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

#include <algorithm>
#include <Core/Math/MiscMath.h>
#include <Core/Utils/Exception.h>
#include <gtest/gtest.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

using namespace ::testing;

const static float epsilon = 0.2e-5f;

TEST(GLMTests, CameraTransform)
{
  glm::mat4 t(1.0);
  float identityList[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1};
  glm::mat4 identity = glm::make_mat4(identityList);
  ASSERT_EQ(t, identity);

  glm::vec3 lookAt = glm::vec3(2, 4.5, 7);
  t = glm::translate(t, -lookAt);
  float translateList[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -2, -4.5, -7, 1};
  glm::mat4 translatedMat = glm::make_mat4(translateList);
  ASSERT_EQ(t, translatedMat);

  const static float theta = static_cast<float>(M_PI) * 0.25f;
  auto rot = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.f, 0.f, 1.f));
  t = rot * t;
  float rotList[16] = {std::cos(theta), std::sin(theta), 0, 0, -std::sin(theta), std::cos(theta), 0,
      0, 0, 0, 1, 0, 1.7677669, -4.5961941, -7, 1};
  glm::mat4 rotMat = glm::make_mat4(rotList);
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      ASSERT_NEAR(t[i][j], rotMat[i][j], epsilon);

  float camDist = 0.7;
  t[3][2] -= camDist;
  float camDistList[16] = {std::cos(theta), std::sin(theta), 0, 0, -std::sin(theta), std::cos(theta), 0,
      0, 0, 0, 1, 0, 1.7677669, -4.5961941, -7.7, 1};
  glm::mat4 camDistMat = glm::make_mat4(camDistList);
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j) ASSERT_NEAR(t[i][j], camDistMat[i][j], epsilon);
}
