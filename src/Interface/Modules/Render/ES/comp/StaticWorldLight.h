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


#ifndef INTERFACE_MODULES_RENDER_ES_COMP_STATIC_WORLD_LIGHT_H
#define INTERFACE_MODULES_RENDER_ES_COMP_STATIC_WORLD_LIGHT_H

#include <cstdint>
#include <glm/glm.hpp>
#include <entity-system/GenericSystem.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include "LightingUniforms.h"

namespace SCIRun {
namespace Render {

// Static world light. We may have multiple world lights inside this structure
// eventually.
struct StaticWorldLight
{
  // -- Data --
  glm::vec3 lightDir[LIGHT_NUM];
  glm::vec3 lightColor[LIGHT_NUM];

  // -- Functions --
  StaticWorldLight()
  {
    lightDir[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < LIGHT_NUM; ++i)
      lightColor[i] = glm::vec3(1.0f);
  }

  static const char* getName() {return "StaticWorldLight";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    for (int i = 0; i < LIGHT_NUM; ++i)
      s.serialize("dir", lightDir[i]);
    for (int i = 0; i < LIGHT_NUM; ++i)
      s.serialize("color", lightColor[i]);
    return true;
  }
};

} // namespace Render
} // namespace SCIRun

#endif
