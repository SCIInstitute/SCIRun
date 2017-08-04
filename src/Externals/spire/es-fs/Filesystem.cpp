#include "Filesystem.hpp"
#include "fscomp/StaticFS.hpp"
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>

namespace es = spire;

namespace spire {

class FSSystem : public spire::GenericSystem<false, StaticFS>
{
public:
  static const char* getName() {return "es-fs:FSSystem";}

  void execute(spire::ESCoreBase& /* core */, uint64_t /* entityID */, const StaticFS* fs) override
  {
    // Update the static filesystem by casting off const.
    StaticFS* mutableFS = const_cast<StaticFS*>(fs);
    mutableFS->instance->update();
  }
};

const char* Filesystem::getFSSystemName()
{
  return FSSystem::getName();
}

void Filesystem::registerSystems(spire::Acorn& core)
{
  core.registerSystem<FSSystem>();
}

} // namespace spire

