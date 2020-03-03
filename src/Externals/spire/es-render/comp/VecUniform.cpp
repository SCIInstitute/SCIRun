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
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr

#include "VecUniform.hpp"

namespace shaders = spire;

namespace ren {

void VecUniform::applyUniform() const
{
  switch (type)
  {
    case SCALAR:
      GL(glUniform1f(uniformLocation, uniform.x));
      break;

    case VEC2:
      GL(glUniform2f(uniformLocation, uniform.x, uniform.y));
      break;

    case VEC3:
      GL(glUniform3f(uniformLocation, uniform.x, uniform.y, uniform.z));
      break;

    case VEC4:
      GL(glUniform4f(uniformLocation, uniform.x, uniform.y, uniform.z, uniform.w));
      break;

    case NONE:
      std::cerr << "VecUniform - applyUniform: attempting to apply NONE." << std::endl;
      break;
  }
}

void VecUniform::checkUniform(GLuint shaderID)
{
  bool boundUniform = false;
  std::vector<shaders::ShaderUniform> uniforms =
      shaders::getProgramUniforms(shaderID);
  for (const shaders::ShaderUniform& unif : uniforms)
  {
    if (unif.nameInCode == uniformName)
    {
      /// \todo Check size and type against what we pulled from OpenGL.
      uniformLocation = unif.uniformLoc;
      boundUniform = true;
      break;
    }
  }

  if (boundUniform == false)
  {
    std::cerr << "Unable to find uniform with name: " << uniformName <<
        " in shader with ID: " << shaderID << std::endl;
  }
}

bool VecUniform::isSetUp() const
{
  return (uniformLocation != -1);
}

VecUniform::VecUniform(const char* uniformNameIn, const glm::vec4& vec, VecUniformType unifType) :
    uniformLocation(-1)
{
  uniform = vec;
  type = unifType;
  if (std::strlen(uniformNameIn) < MaxUniformNameSize - 1)
  {
    std::strncpy(uniformName, uniformNameIn, MaxUniformNameSize - 1);
  }
  else
  {
    std::cerr << "ren::VecUniform Uniform name too long." << std::endl;
  }
}

} // namespace ren
