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


#ifndef SPIRE_ES_GENERAL_CONSTANT_ROTATION_HPP
#define SPIRE_ES_GENERAL_CONSTANT_ROTATION_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <spire/scishare.h>

namespace gen {

// This component will *not* be present when there is no mouse input available
// for the system. You can check other input sources such as touch input
// or keyboard input.
struct ConstantRotation
{
  // -- Data --
  bool      pretransform;     ///< If true, then if this matrix is X, and the
                              ///< transformation is Y, then the resulting
                              ///< transform is Y * X, otherwise it is X * Y.
  float     radiansPerSec;    ///< Radians per second to rotate around an axis.
  glm::vec3 rotationAxis;     ///< Axis of rotation.

  // -- Functions --
  ConstantRotation()
  {
    pretransform = true;
    radiansPerSec = 0.0f;
    rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
  }

  static const char* getName() {return "gen:ConstantRotation";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("prexform", pretransform);
    s.serialize("axis", rotationAxis);
    s.serialize("rPerSec", radiansPerSec);
    return true;
  }
};

} // namespace gen

#endif
