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
#include <gl-platform/GLPlatform.hpp>
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include <es-general/comp/StaticMouseInput.hpp>
#include <es-general/comp/StaticCamera.hpp>
#include <es-general/comp/StaticOrthoCamera.hpp>

#include "../comp/StaticArcBallCam.hpp"

namespace es = spire;

namespace ren {

class ArcBallCameraSys :
    public spire::GenericSystem<true, gen::StaticCamera, gen::StaticOrthoCamera,
                                   StaticArcBallCam, gen::StaticMouseInput>
{
public:
  static const char* getName() {return "ren:ArcBallCameraSys";}

  void groupExecute(spire::ESCoreBase&, uint64_t,
      const spire::ComponentGroup<gen::StaticCamera>& camera,
      const spire::ComponentGroup<gen::StaticOrthoCamera>& orthoCamera,
      const spire::ComponentGroup<StaticArcBallCam>& arcBallConst,
      const spire::ComponentGroup<gen::StaticMouseInput>& mouseInputIn) override
  {
    // We will modify arcBall in-place. We will be the only system modifying
    // its data, so no ill effects will be felt by bypassing the modification
    // system -- unlike modifying the camera data.
    StaticArcBallCam& arcBall = const_cast<StaticArcBallCam&>(arcBallConst.front());
    const gen::StaticMouseInput& mouseInput = mouseInputIn.front();

    // Loop through all mouse events.
    for (const gen::StaticMouseInput::MouseEvent* it = mouseInput.cbegin();
         it != mouseInput.cend(); ++it)
    {
      glm::vec2 ss = gen::StaticMouseInput::calcScreenSpace(it->mousePos);
      switch (it->event)
      {
        case gen::StaticMouseInput::MOUSE_DOWN:
          arcBall.arcLookAt.doReferenceDown(ss);
          break;

        case gen::StaticMouseInput::MOUSE_MOVE:
          if (it->btnState[gen::StaticMouseInput::LEFT]
              == gen::StaticMouseInput::MOUSE_DOWN)
          {
            arcBall.arcLookAt.doRotation(ss);
          }
          else if (it->btnState[gen::StaticMouseInput::RIGHT]
                   == gen::StaticMouseInput::MOUSE_DOWN)
          {
            arcBall.arcLookAt.doPan(ss);
          }
          break;

        case gen::StaticMouseInput::MOUSE_WHEEL:
          // May need to calculate mouse wheel delta.
          arcBall.arcLookAt.doZoom(static_cast<float>(it->mouseWheel) * 0.05f);
          break;

        case gen::StaticMouseInput::MOUSE_UP:
          break;
      }
    }

    if (mouseInput.size() > 0)
    {
      bool doPersp = false;
      bool doOrtho = false;
      // Update the necessary cameras.
      if (arcBall.cams == StaticArcBallCam::BOTH)
      {
        doPersp = true;
        doOrtho = true;
      }
      else if (arcBall.cams == StaticArcBallCam::PERSPECTIVE)
      {
        doPersp = true;
      }
      else if (arcBall.cams == StaticArcBallCam::ORTHO)
      {
        doOrtho = true;
      }

      if (doPersp)
      {
        gen::StaticCamera newPersp = camera.front();
        newPersp.data.setView(arcBall.arcLookAt.getWorldViewTransform());
        camera.modify(newPersp);
      }

      if (doOrtho)
      {
        gen::StaticOrthoCamera newOrtho = orthoCamera.front();
        newOrtho.data.setView(arcBall.arcLookAt.getWorldViewTransform());
        orthoCamera.modify(newOrtho);
      }
    }
  }

};

void registerSystem_ArcBallCameraMouse(spire::Acorn& core)
{
  core.registerSystem<ArcBallCameraSys>();
}

const char* getSystemName_ArcBallCameraMouse()
{
  return ArcBallCameraSys::getName();
}

} // namespace ren
