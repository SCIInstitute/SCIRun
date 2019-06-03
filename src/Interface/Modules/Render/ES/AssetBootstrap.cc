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

#include "SRInterface.h"
#include "comp/StaticSRInterface.h"

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
    if (std::shared_ptr<ren::GeomMan> geomMan = gm.lock()) {
        // Load geometry and associated vertex and fragment shaders.
        geomMan->loadGeometry(core, cachedEntity, "Assets/arrow.geom");
        geomMan->loadGeometry(core, cachedEntity, "Assets/sphere.geom");
    }
    if (std::shared_ptr<ren::ShaderMan> shaderMan = sm.lock()) {
        // Load shader we will use with the coordinate axes.
      shaderMan->loadVertexAndFragmentShader(core, cachedEntity, "Shaders/DirPhong");
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
