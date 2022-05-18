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


#ifndef SPIRE_GP_REN_UTIL_TYPE_TO_GL_UNIFORM_HPP
#define SPIRE_GP_REN_UTIL_TYPE_TO_GL_UNIFORM_HPP

#include <es-log/trace-log.h>
#include <gl-platform/GLPlatform.hpp>
#include "es-render/comp/VecUniform.hpp"
#include <spire/scishare.h>

namespace ren {

namespace uniform_detail {

template <typename T>
class TypeToGLUniform
{
public:
  typedef int Type;

  static_assert(true, "TypeToGLUniform type specialization not defined.");
  static void addUniform(spire::CerealCore& core, uint64_t entityID,
                         const char* uniformName, const Type& v);
};

template<>
class TypeToGLUniform<float>
{
public:
  typedef float Type;

  static void addUniform(spire::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v, 0.0f, 0.0f, 0.0f), ren::VecUniform::SCALAR);
    core.addComponent(entityID, unif);
  }
};

template<>
class TypeToGLUniform<glm::vec2>
{
public:
  typedef glm::vec2 Type;

  static void addUniform(spire::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v.x, v.y, 0.0f, 0.0f), ren::VecUniform::VEC2);
    core.addComponent(entityID, unif);
  }
};

template<>
class TypeToGLUniform<glm::vec3>
{
public:
  typedef glm::vec3 Type;

  static void addUniform(spire::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v.x, v.y, v.z, 0.0f), ren::VecUniform::VEC3);
    core.addComponent(entityID, unif);
  }
};

template<>
class TypeToGLUniform<glm::vec4>
{
public:
  typedef glm::vec4 Type;

  static void addUniform(spire::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v.x, v.y, v.z, v.w), ren::VecUniform::VEC4);
    core.addComponent(entityID, unif);
  }
};

} // namespace uniform_detail

} // namespace ren

#endif
