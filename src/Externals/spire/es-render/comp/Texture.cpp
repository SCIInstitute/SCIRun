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
#include "Texture.hpp"

namespace shaders = spire;

namespace ren {

void Texture::checkUniform(GLuint shaderID)
{
  if (isSetUp() == false)
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
}

void Texture::applyUniform() const
{
  if (isSetUp())
  {
    // When rendering an object with this program.
    GL(glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(textureUnit)));
    GL(glBindTexture(textureType, glid));
    GL(glUniform1i(uniformLocation, textureUnit));
  }
}

bool Texture::isSetUp() const
{
  return (uniformLocation != -1);
}

} // namespace ren
