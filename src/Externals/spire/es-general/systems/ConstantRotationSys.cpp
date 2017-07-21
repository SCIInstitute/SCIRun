#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include "../comp/Transform.hpp"
#include "../comp/StaticGlobalTime.hpp"
#include "../comp/ConstantRotation.hpp"

namespace es      = CPM_ES_NS;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list. If you need to access
// ESCoreBase, you need to create a static component for it.
namespace gen {

class ConstantRotationSys :
    public es::GenericSystem<true,
                             Transform,
                             StaticGlobalTime,
                             ConstantRotation>
{
public:

  static const char* getName() {return "gen:ConstantRotationSys";}

  void groupExecute(
      es::ESCoreBase&, uint64_t /* entityID */,
      const es::ComponentGroup<Transform>& trafo,
      const es::ComponentGroup<StaticGlobalTime>& time,
      const es::ComponentGroup<ConstantRotation>& constRot) override
  {
    /// NOTE: We can combine transformations, so long as this is the only
    ///       other transformation besides modifying the raw transform
    ///       every frame.

    // Keeping the following todo here because it has a relevant discussion
    // of the topics regarding this system.
    /// \todo Since transform is such a common component to modify, we should
    ///       switch this module over to imperative control. Beware of the
    ///       ramifications of multiprocessor execution if we do this.
    /// \note We should *MARK* this system and the affected components so that
    ///       no other system that also modifies the affected components
    ///       will be affected by this behavior.

    // Our delta time is constant, we could pre-calculate this matrix. If it
    // becomes a performance bottleneck, do that.
    float rotationAmount = static_cast<float>(time.front().deltaTime) * constRot.front().radiansPerSec;
    glm::mat4 dataXForm = glm::rotate(rotationAmount,
                                      constRot.front().rotationAxis);
    if (constRot.front().pretransform)
    {
      dataXForm = trafo.front().transform * dataXForm;
    }
    else
    {
      dataXForm = dataXForm * trafo.front().transform;
    }

    const_cast<Transform&>(trafo.front()).transform = dataXForm;

    // A more correct way of performing the modification, but there is currently
    // no way of recombining the data at the end:
    // Transform newTrafo;
    // newTrafo.transform = dataXForm;
    // trafo.modify(newTrafo);
  }
};

void registerSystem_ConstantRotation(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<ConstantRotationSys>();
}

const char* getSystemName_ConstantRotation()
{
  return ConstantRotationSys::getName();
}

} // namespace gen

