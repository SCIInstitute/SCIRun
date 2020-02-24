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


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "LightingUniforms.h"

namespace shaders = spire;

namespace SCIRun {
namespace Render {

LightingUniforms::LightingUniforms()
{
  for (int i = 0; i < LIGHT_NUM; ++i)
  {
    hasLightUniform[i] = false;
    uniformLocation[i] = 0;
    hasLightColorUniform[i] = false;
    colorUnifLocation[i] = 0;
  }
}

void LightingUniforms::checkUniformArray(GLuint shaderID)
{
  // Obtain uniforms from shader and decide which of the uniforms we can
  // provide automatically.
  std::vector<shaders::ShaderUniform> shaderUniforms = shaders::getProgramUniforms(shaderID);
  for (const shaders::ShaderUniform& uniform : shaderUniforms)
  {
    if (uniform.nameInCode == "uLightDirectionView")
    {
      hasLightUniform[0] = true;
      uniformLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView0")
    {
      hasLightUniform[0] = true;
      uniformLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView1")
    {
      hasLightUniform[1] = true;
      uniformLocation[1] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView2")
    {
      hasLightUniform[2] = true;
      uniformLocation[2] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightDirectionView3")
    {
      hasLightUniform[3] = true;
      uniformLocation[3] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor0")
    {
      hasLightColorUniform[0] = true;
      colorUnifLocation[0] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor1")
    {
      hasLightColorUniform[1] = true;
      colorUnifLocation[1] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor2")
    {
      hasLightColorUniform[2] = true;
      colorUnifLocation[2] = uniform.uniformLoc;
    }
    else if (uniform.nameInCode == "uLightColor3")
    {
      hasLightColorUniform[3] = true;
      colorUnifLocation[3] = uniform.uniformLoc;
    }
  }
}

void LightingUniforms::applyUniform(const std::vector<glm::vec3>& lightDirs,
  const std::vector<glm::vec3>& lightColors) const
{
  for (int i = 0; i < lightDirs.size(); ++i)
  {
    if (i >= LIGHT_NUM)
      break;
    if (hasLightUniform[i])
    {
      GL(glUniform3f(uniformLocation[i], lightDirs[i].x, lightDirs[i].y, lightDirs[i].z));
    }
  }
  for (int i = 0; i < lightColors.size(); ++i)
  {
    if (i >= LIGHT_NUM)
      break;
    if (hasLightColorUniform[i])
    {
      GL(glUniform3f(colorUnifLocation[i], lightColors[i].x, lightColors[i].y, lightColors[i].z));
    }
  }
}

} // namespace Render
} // namespace SCIRun
