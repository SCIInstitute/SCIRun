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


#ifndef SPIRE_RENDER_UTIL_VIEW_POS_ALIGN_HPP
#define SPIRE_RENDER_UTIL_VIEW_POS_ALIGN_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <spire/scishare.h>

namespace ren {

// Careful with this component. It requires that the transform be modified
// using const_cast and not using the default modification system.
struct UtilViewPosAlign
{
  // -- Data --
  // !!NOTE!! Neither of the following variables are implemented. See
  // ViewPosAlignSys. That is where it should be implemented.
  bool offsetInViewCoords;  // True if the offset is relative to the view coordinate system.
  glm::vec3 offset;         // Offset from the view position.

  // -- Functions --
  UtilViewPosAlign()
  {
    offsetInViewCoords = false;
    offset = glm::vec3(0.0f, 0.0f, 0.0f);
  }

  static const char* getName() {return "ren:UtilViewPosAlign";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("offvc", offsetInViewCoords);
    s.serialize("offset", offset);
    return true;
  }
};

} // namespace ren

#endif
