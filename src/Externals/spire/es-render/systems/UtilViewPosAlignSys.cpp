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


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include <es-general/comp/Transform.hpp>
#include <es-general/comp/StaticCamera.hpp>

#include "es-render/comp/UtilViewPosAlign.hpp"

namespace es      = spire;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list. If you need to access
// ESCoreBase, you need to create a static component for it.
namespace ren {

class UtilViewPosAlignSys :
    public spire::GenericSystem<true,
                             gen::Transform,
                             gen::StaticCamera,
                             UtilViewPosAlign>
{
public:

  /// Util system classes are in the qren namespacen so they execute
  /// before any rendering code.
  static const char* getName() {return "qren:UtilViewPosAlignSys";}

  void groupExecute(
      spire::ESCoreBase&, uint64_t /* entityID */,
      const spire::ComponentGroup<gen::Transform>& trafo,
      const spire::ComponentGroup<gen::StaticCamera>& cam,
      const spire::ComponentGroup<UtilViewPosAlign>& /* posAlign */) override
  {
    glm::mat4 viewMat = cam.front().data.getInverseView();
    glm::mat4 newTrafo = trafo.front().transform;
    newTrafo[3].x = viewMat[3].x;
    newTrafo[3].y = viewMat[3].y;
    newTrafo[3].z = viewMat[3].z;

    const_cast<gen::Transform&>(trafo.front()).transform = newTrafo;

    // A more correct way of performing the modification, but there is currently
    // no way of recombining the data at the end:
    // gen::Transform newTrafo;
    // newTrafo.transform = dataXForm;
    // trafo.modify(newTrafo);
  }
};

void registerSystem_UtilViewPosAlign(spire::Acorn& core)
{
  core.registerSystem<UtilViewPosAlignSys>();
}

const char* getSystemName_UtilViewPosAlign()
{
  return UtilViewPosAlignSys::getName();
}

} // namespace ren
