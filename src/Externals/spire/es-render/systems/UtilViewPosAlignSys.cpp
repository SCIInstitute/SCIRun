#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include <es-general/comp/Transform.hpp>
#include <es-general/comp/StaticCamera.hpp>

#include "es-render/comp/UtilViewPosAlign.hpp"

namespace es      = CPM_ES_NS;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list. If you need to access
// ESCoreBase, you need to create a static component for it.
namespace ren {

class UtilViewPosAlignSys :
    public es::GenericSystem<true,
                             gen::Transform,
                             gen::StaticCamera,
                             UtilViewPosAlign>
{
public:

  /// Util system classes are in the qren namespacen so they execute
  /// before any rendering code.
  static const char* getName() {return "qren:UtilViewPosAlignSys";}

  void groupExecute(
      es::ESCoreBase&, uint64_t /* entityID */,
      const es::ComponentGroup<gen::Transform>& trafo,
      const es::ComponentGroup<gen::StaticCamera>& cam,
      const es::ComponentGroup<UtilViewPosAlign>& /* posAlign */) override
  {
    glm::mat4 viewMat = cam.front().data.getView();
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

void registerSystem_UtilViewPosAlign(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<UtilViewPosAlignSys>();
}

const char* getSystemName_UtilViewPosAlign()
{
  return UtilViewPosAlignSys::getName();
}

} // namespace ren

