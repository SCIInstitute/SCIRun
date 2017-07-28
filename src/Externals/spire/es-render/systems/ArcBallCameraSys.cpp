#include <glm/glm.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include <es-general/comp/StaticMouseInput.hpp>
#include <es-general/comp/StaticCamera.hpp>
#include <es-general/comp/StaticOrthoCamera.hpp>

#include "../comp/StaticArcBallCam.hpp"

namespace es = CPM_ES_NS;

namespace ren {

class ArcBallCameraSys :
    public es::GenericSystem<true, gen::StaticCamera, gen::StaticOrthoCamera,
                                   StaticArcBallCam, gen::StaticMouseInput>
{
public:
  static const char* getName() {return "ren:ArcBallCameraSys";}

  void groupExecute(es::ESCoreBase&, uint64_t,
      const es::ComponentGroup<gen::StaticCamera>& camera,
      const es::ComponentGroup<gen::StaticOrthoCamera>& orthoCamera,
      const es::ComponentGroup<StaticArcBallCam>& arcBallConst,
      const es::ComponentGroup<gen::StaticMouseInput>& mouseInputIn) override
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

void registerSystem_ArcBallCameraMouse(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<ArcBallCameraSys>();
}

const char* getSystemName_ArcBallCameraMouse()
{
  return ArcBallCameraSys::getName();
}

} // namespace ren
