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


#ifndef SPIRE_GP_REN_UTIL_UNIFORM_HPP
#define SPIRE_GP_REN_UTIL_UNIFORM_HPP

#include <es-log/trace-log.h>
#include <es-cereal/CerealCore.hpp>
#include "helper/TypeToGLUniform.hpp"

namespace ren {

// This function *only* adds a uniform. It does not check for pre-existing
// uniforms. Faster and simpler if we know the uniform doesn't already
// exist (such as when the object is first being created).
template <typename T>
void addGLUniform(spire::CerealCore& core, uint64_t entityID,
                  const char* uniformName, const T& val)
{
  uniform_detail::TypeToGLUniform<T>::addUniform(core, entityID, uniformName, val);
}

// Detects appropriate uniform to add to the entity. Will attempt to find
// a pre-existing uniform in its component list and modify it instead of
// adding a new one. Not implemented yet, check the other ren-utils that
// check components. For this implementation, iteration over the components
// will be necessary. May need to add functionality for this.
//template <typename T>
//void setGLUniform(spire::CerealCore& core, uint64_t entityID, const T& val) ...

} // namespace ren

#endif
