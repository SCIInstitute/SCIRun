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


#ifndef SPIRE_RENDER_COMPONENT_VBO_HPP
#define SPIRE_RENDER_COMPONENT_VBO_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct VBO
{
  // -- Data --
  VBO() : glid(0) {}

  // -- Functions --
  static const char* getName() {return "ren:VBO";}

  GLuint glid;

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Serialize out the asset name (if any), and the attributes associated
    // with this glid. We don't need to store the attributes alongside
    // the glid, just in a central repository. This is very similar to
    // how we handle the shader.

    // Maybe write out the name of the file the VBO is associated with?
    // Maybe write out the VBO itself?
    return true;
  }
};

} // namespace ren

#endif
