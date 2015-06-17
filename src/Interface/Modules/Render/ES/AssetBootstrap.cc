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

namespace es      = CPM_ES_NS;

namespace SCIRun {
namespace Render {

class AssetBootstrap : public es::EmptySystem
{
public:
  
  static const char* getName() {return "scirun:AssetBootstrap";}

  void execute(es::ESCoreBase& baseCore)
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

    // Will need to replace this with a static entity in the future.
    SRInterface * iface = core.getStaticComponent<StaticSRInterface>()->instance_;
    std::shared_ptr<ren::GeomMan> geomMan = core.getStaticComponent<ren::StaticGeomMan>()->instance_;
    std::shared_ptr<ren::ShaderMan> shaderMan = core.getStaticComponent<ren::StaticShaderMan>()->instance_;

    // A cached entity so that our VBOs and IBOs will not get garbage collected.
    uint64_t cachedEntity = 0;
    cachedEntity = gen::StaticObjRefID::getNewObjectID(core);

    // Load geometry and associated vertex and fragment shaders.
    geomMan->loadGeometry(core, cachedEntity, "Assets/arrow.geom");
    geomMan->loadGeometry(core, cachedEntity, "Assets/sphere.geom");

    // Load shader we will use with the coordinate axes.
    shaderMan->loadVertexAndFragmentShader(core, cachedEntity, "Shaders/DirPhong");

    // Note: We don't need to strictly store the coordinate axes entity.
    // We are really only after its VBO / IBO.

    // Remove our system now that we are done caching assets.
    core.removeUserSystem(getName());
  }
};

void registerSystem_AssetBootstrap(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<AssetBootstrap>();
}

const char* getSystemName_AssetBootstrap() {return AssetBootstrap::getName();}

} // namespace Render
} // namespace SCIRun


