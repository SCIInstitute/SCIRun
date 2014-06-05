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
    SRInterface* iface = core.getStaticComponent<StaticSRInterface>()->interface;

    // Load geometry and associated vertex and fragment shaders.
    uint64_t coordAxesEntity = 0;
    ren::GeomMan* geomMan = core.getStaticComponent<ren::StaticGeomMan>()->instance.get();
    coordAxesEntity = gen::StaticObjRefID::getNewObjectID(core);
    geomMan->loadGeometry(core, coordAxesEntity, "assets/arrow");
    iface->mCoordAxesEntity = coordAxesEntity;

    // Note. We don't need to strictly store the coordinate axes entity.
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


