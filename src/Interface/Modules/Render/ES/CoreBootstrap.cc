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


#include <cstdint>
#include <entity-system/EmptySystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <es-fs/Filesystem.hpp>
#include <es-fs/fscomp/StaticFS.hpp>
#include <es-acorn/Acorn.hpp>

#include <es-general/comp/StaticObjRefID.hpp>
#include <es-general/comp/StaticCamera.hpp>
#include <es-general/comp/StaticOrthoCamera.hpp>
#include <es-general/comp/StaticKeyboardInput.hpp>
#include <es-general/comp/StaticMouseInput.hpp>
#include <es-general/comp/StaticScreenDims.hpp>
#include <es-general/comp/StaticGlobalTime.hpp>
#include <es-general/comp/StaticRandom.hpp>

#include <es-render/ShaderMan.hpp>
#include <es-render/comp/StaticShaderMan.hpp>
#include <es-render/comp/StaticVBOMan.hpp>
#include <es-render/comp/StaticIBOMan.hpp>
#include <es-render/comp/StaticTextureMan.hpp>
#include <es-render/comp/StaticFBOMan.hpp>
#include <es-render/comp/StaticGeomMan.hpp>
#include <es-render/comp/StaticFontMan.hpp>
#include <es-render/comp/StaticGLState.hpp>

#include "comp/RenderBasicGeom.h"
#include "comp/StaticWorldLight.h"
#include "comp/StaticClippingPlanes.h"
#include "systems/RenderBasicSys.h"
#include "systems/RenderTransBasicSys.h"
#include "systems/RenderTransText.h"
#include "CoreBootstrap.h"
#include "AssetBootstrap.h"
#include "Core.h"

namespace SCIRun {
namespace Render {

class CoreBootstrap : public spire::EmptySystem
{
public:
  static const char* getName() {return "scirun:CoreBootstrap";}

  void execute(spire::ESCoreBase& baseCore)
  {
    // Dynamic cast core into our core so that we have access to the systems
    // factory.
    ESCore* corePtr = dynamic_cast<ESCore*>(&baseCore);
    if (corePtr == nullptr)
    {
      std::cerr << "ap bad cast." << std::endl;
      return;
    }
    ESCore& core = *corePtr;

    // Perform any caching or mandatory initialization. This bootstrap will
    // only be called once.

    // Add StaticObjRefID
    gen::StaticObjRefID newObjRefID;
    core.addStaticComponent(newObjRefID);
    core.addExemptComponent<gen::StaticObjRefID>();

    // Kernel file system.
    core.addKernelSystem(spire::Filesystem::getFSSystemName());

    // --== Base Rendering ==--

    // Static shader man and associated systems. Only run GC every 5 minutes.
    // The systems themselves will run GC at their own convenience.
    core.addKernelSystem(ren::ShaderMan::getGCName(), 1000 * 60 * 5);
    core.addKernelSystem(ren::TextureMan::getGCName(), 1000 * 60 * 5);

    // -- Promise Fulfillment --
    // Run shader promise fulfillment 5 times a second.
    core.addKernelSystem(ren::ShaderMan::getPromiseVFFulfillmentName(), 200);
    core.addKernelSystem(ren::TextureMan::getPromiseSystemName(), 200, 0, 50);
    core.addKernelSystem(ren::GeomMan::getPromiseSystemName(), 200, 0, 100);
    core.addKernelSystem(ren::FontMan::getPromiseSystemName(), 200, 0, 150);

    // -- Garbage Collection --
    core.addGarbageCollectorSystem(ren::ShaderMan::getGCName());
    core.addGarbageCollectorSystem(ren::TextureMan::getGCName());
    core.addGarbageCollectorSystem(ren::GeomMan::getGCName());
    core.addGarbageCollectorSystem(ren::FontMan::getGCName());
    core.addGarbageCollectorSystem(ren::IBOMan::getGCName());
    core.addGarbageCollectorSystem(ren::VBOMan::getGCName());

    // -- Static Rendering Components --
    core.addStaticComponent(ren::StaticShaderMan());
    core.addExemptComponent<ren::StaticShaderMan>();

    core.addStaticComponent(ren::StaticVBOMan());
    core.addExemptComponent<ren::StaticVBOMan>();

    core.addStaticComponent(ren::StaticIBOMan());
    core.addExemptComponent<ren::StaticIBOMan>();

	  core.addStaticComponent(ren::StaticFBOMan());
	  core.addExemptComponent<ren::StaticFBOMan>();

    core.addStaticComponent(ren::StaticTextureMan());
    core.addExemptComponent<ren::StaticTextureMan>();

    core.addStaticComponent(ren::StaticGeomMan());
    core.addExemptComponent<ren::StaticGeomMan>();

    core.addStaticComponent(ren::StaticFontMan());
    core.addExemptComponent<ren::StaticFontMan>();

    // --== SCIRun5 Rendering ==--

    core.addUserSystem(getSystemName_RenderBasicGeom());
    core.addUserSystem(getSystemName_RenderBasicTransGeom());
    core.addUserSystem(getSystemName_RenderTransTextGeom());

    // --== General ==--

    core.addExemptComponent<gen::StaticRandom>();
    core.addStaticComponent(gen::StaticRandom());

    // -- Static General Components --
    // Misc components that were added by the os specific layer that need
    // to be exempt.
    core.addExemptComponent<gen::StaticMouseInput>();
    core.addExemptComponent<gen::StaticKeyboardInput>();
    core.addExemptComponent<gen::StaticScreenDims>();
    core.addExemptComponent<gen::StaticGlobalTime>();
    core.addExemptComponent<ren::StaticGLState>();
    core.addExemptComponent<spire::StaticFS>();

    // Setup default camera projection.
    gen::StaticCamera cam;
    float aspect = static_cast<float>(800) / static_cast<float>(600);

    float perspFOVY = 0.59f;
    float perspZNear = 1.0f;
    float perspZFar = 2000.0f;
    glm::mat4 proj = glm::perspective(perspFOVY, aspect, perspZNear, perspZFar);
    cam.data.setProjection(proj, perspFOVY, aspect, perspZNear, perspZFar);
    cam.data.setView(glm::mat4());

    // float desZ = std::get<0>(cam.data.getPPDesWidth(6.0f));
    // std::cout << "Desired Z: " << desZ << std::endl;
    float desZ = -7.40457f;
    cam.data.setZPlaneOfPlay(desZ);

    core.addStaticComponent(cam);
    core.addExemptComponent<gen::StaticCamera>();

    // Add static world light.
    StaticWorldLight worldLight;
    //worldLight.lightDir = glm::vec3(1.0f, 0.0f, 0.0f);
    core.addStaticComponent(worldLight);
    core.addExemptComponent<StaticWorldLight>();

    // Add static clipping planes.
    StaticClippingPlanes clippingPlanes;
    for (int i = 0; i < 6; ++i)
    {
      clippingPlanes.clippingPlanes.push_back(glm::vec4());
      clippingPlanes.clippingPlaneCtrls.push_back(glm::vec4());
    }
    core.addStaticComponent(clippingPlanes);
    core.addExemptComponent<StaticClippingPlanes>();

    // Setup default ortho camera projection
    gen::StaticOrthoCamera orthoCam;
    float orthoZNear  = -1000.0f;
    float orthoZFar   =  1000.0f;
    glm::mat4 orthoProj =
        glm::ortho(/*left*/   -1.0f,      /*right*/ 1.0f,
                   /*bottom*/ -1.0f,      /*top*/   1.0f,
                   /*znear*/  orthoZNear, /*zfar*/  orthoZFar);
    orthoCam.data.setOrthoProjection(orthoProj, aspect, 2.0f, 2.0f, orthoZNear, orthoZFar);
    orthoCam.data.setView(glm::mat4());
    core.addStaticComponent(orthoCam);
    core.addExemptComponent<gen::StaticOrthoCamera>();

    // Add asset bootstrap.
    core.addUserSystem(getSystemName_AssetBootstrap());

    // Now remove ourselves. We should only execute for one frame.
    core.removeUserSystem(getName());
  }
};

void registerSystem_CoreBootstrap(spire::Acorn& core)
{
  core.registerSystem<CoreBootstrap>();
}

const char* getSystemName_CoreBootstrap() {return CoreBootstrap::getName();}

} // namespace Render
} // namespace SCIRun
