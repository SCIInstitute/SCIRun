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


#ifndef SPIRE_RENDER_COMPONENT_VEC_UNIFORM_HPP
#define SPIRE_RENDER_COMPONENT_VEC_UNIFORM_HPP

#include <es-log/trace-log.h>
#include <cstring>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct SCISHARE VecUniform
{
  // -- Data --
  static const int MaxUniformNameSize = 24;

  enum VecUniformType
  {
    SCALAR,
    VEC2,
    VEC3,
    VEC4,
    NONE
  };

  glm::vec4       uniform;
  VecUniformType  type;
  GLint           uniformLocation;
  char            uniformName[MaxUniformNameSize];

  // -- Functions --
  VecUniform()
  {
    type            = VecUniformType::NONE;
    uniformLocation = -1;
    uniformName[0]  = '\0';
  }

  VecUniform(const char* uniformNameIN, const glm::vec4& vec, VecUniformType unifType);

  // Applies the uniform.
  void applyUniform() const;

  // Checks to see if uniformLocation has been set. If not, then the uniform
  // location in the shader
  void checkUniform(GLuint shaderID);

  // Returns true if this uniform has been setup for rendering with a particular
  // shader.
  bool isSetUp() const;

  static const char* getName() {return "ren:VecUniform";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Write out appropriate vector uniform.
    return true;
  }
};

} // namespace ren

#endif
