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
#include <glm/gtc/matrix_transform.hpp>
#include <es-acorn/Acorn.hpp>

#include <es-general/comp/StaticObjRefID.hpp>
#include <es-render/comp/StaticShaderMan.hpp>
#include <es-render/comp/StaticVBOMan.hpp>
#include <es-render/comp/StaticIBOMan.hpp>
#include <es-render/comp/StaticTextureMan.hpp>
#include <es-render/comp/StaticGeomMan.hpp>
#include <es-render/comp/StaticFontMan.hpp>
#include <es-render/comp/StaticGLState.hpp>

#include "AssetBootstrap.h"
#include "Core.h"

namespace SCIRun {
namespace Render {

class AssetBootstrap : public spire::EmptySystem
{
public:

  static const char* getName() {return "scirun:AssetBootstrap";}

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

    // We are guaranteed that CoreBootstrap ran one frame prior. So we will
    // have access to all of the rendering subsystems.

    std::weak_ptr<ren::GeomMan> gm =
            core.getStaticComponent<ren::StaticGeomMan>()->instance_;
    std::weak_ptr<ren::ShaderMan> sm =
            core.getStaticComponent<ren::StaticShaderMan>()->instance_;

    // A cached entity so that our VBOs and IBOs will not get garbage collected.
    uint64_t cachedEntity = 0;
    cachedEntity = gen::StaticObjRefID::getNewObjectID(core);
    if (std::shared_ptr<ren::GeomMan> geomMan = gm.lock())
    {
      // Load geometry and associated vertex and fragment shaders.
      geomMan->loadGeometry(core, cachedEntity, "Assets/arrow.geom");
    }
    if (std::shared_ptr<ren::ShaderMan> shaderMan = sm.lock())
    {
        // Load shader we will use with the coordinate axes.
      shaderMan->loadVertexAndFragmentShader(core, cachedEntity, "Shaders/OrientationGlyph");
    }

    // Note: We don't need to strictly store the coordinate axes entity.
    // We are really only after its VBO / IBO.

    // Remove our system now that we are done caching assets.
    core.removeUserSystem(getName());
  }
};

void registerSystem_AssetBootstrap(spire::Acorn& core)
{
  core.registerSystem<AssetBootstrap>();
}

const char* getSystemName_AssetBootstrap() {return AssetBootstrap::getName();}

} // namespace Render
} // namespace SCIRun
