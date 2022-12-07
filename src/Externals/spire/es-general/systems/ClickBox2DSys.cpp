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

#include "../comp/Transform.hpp"
#include "../comp/StaticMouseInput.hpp"
#include "../comp/StaticOrthoCamera.hpp"
#include "../comp/StaticCamera.hpp"
#include "../comp/CameraSelect.hpp"
#include "../comp/ClickBox2D.hpp"

namespace es      = spire;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list. If you need to access
// ESCoreBase, you need to create a static component for it.
namespace gen {

class ClickBox2DSys :
    public spire::GenericSystem<true,
                             Transform,
                             ClickBox2D,
                             CameraSelect,
                             StaticCamera,
                             StaticOrthoCamera,
                             StaticMouseInput>
{
public:

  static const char* getName() {return "gen:ClickBox2DSys";}

  bool isComponentOptional(uint64_t type) override
  {
    return spire::OptionalComponents<Transform, CameraSelect>(type);
  }

  void groupExecute(
      spire::ESCoreBase&, uint64_t /* entityID */,
      const spire::ComponentGroup<Transform>& trafo,
      const spire::ComponentGroup<ClickBox2D>& clickBox,
      const spire::ComponentGroup<CameraSelect>& camSel,
      const spire::ComponentGroup<StaticCamera>& cam,
      const spire::ComponentGroup<StaticOrthoCamera>& orthoCam,
      const spire::ComponentGroup<StaticMouseInput>& mouseInput) override
  {
    if (mouseInput.front().size() == 0)
      return;

    // Check to see if the user clicked in our AABB.
    bool perspCam = false;
    if (camSel.size() > 0)
    {
      if (camSel.front().cam == CameraSelect::PERSPECTIVE_CAMERA)
        perspCam = true;
    }

    glm::vec2 camLoc;
    glm::mat4 view;
    if (perspCam)
    {
      view = cam.front().data.getInverseView();
    }
    else
    {
      view = orthoCam.front().data.getInverseView();
    }

    camLoc.x = view[3][0];
    camLoc.y = view[3][1];

    glm::vec2 trafoOffset(0.0f, 0.0f);
    if (trafo.size() != 0)
    {
      trafoOffset.x = trafo.front().transform[3][0];
      trafoOffset.y = trafo.front().transform[3][1];
    }

    for (const gen::StaticMouseInput::MouseEvent* it = mouseInput.front().cbegin();
         it != mouseInput.front().cend(); ++it)
    {
      glm::vec2 ss = gen::StaticMouseInput::calcScreenSpace(it->mousePos);
      glm::vec2 cs = ss + camLoc;

      switch (it->event)
      {
        case gen::StaticMouseInput::MOUSE_DOWN:
          if (it->handled == false)
          {
            // Check to see if we clicked
            int index = 0;
            for (const ClickBox2D& box : clickBox)
            {
              AABB2D tbox; // translated box
              tbox.lowerBound = box.hitBox.lowerBound + trafoOffset;
              tbox.upperBound = box.hitBox.upperBound + trafoOffset;

              if (tbox.contains(cs))
              {
                ClickBox2D newBox = box;
                newBox.hit = true;
                clickBox.modify(newBox, index);

                const_cast<gen::StaticMouseInput::MouseEvent*>(it)->handled = true;

                break;  // We break to ensure this click doesn't get handled multiple times.
              }
              ++index;
            }
          }
          break;

        case gen::StaticMouseInput::MOUSE_MOVE: break;
        case gen::StaticMouseInput::MOUSE_WHEEL: break;
        case gen::StaticMouseInput::MOUSE_UP: break;
      }
    }
  }
};

void registerSystem_ClickBox2D(spire::Acorn& core)
{
  core.registerSystem<ClickBox2DSys>();
}

const char* getSystemName_ClickBox2D()
{
  return ClickBox2DSys::getName();
}

} // namespace gen
