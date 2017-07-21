#include "Filesystem.hpp"
#include "fscomp/StaticFS.hpp"
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>

namespace es = CPM_ES_NS;

namespace CPM_ES_FS_NS {

class FSSystem : public es::GenericSystem<false, StaticFS>
{
public:
  static const char* getName() {return "es-fs:FSSystem";}

  void execute(es::ESCoreBase& /* core */, uint64_t /* entityID */, const StaticFS* fs) override
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

void Filesystem::registerSystems(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<FSSystem>();
}

} // namespace CPM_ES_FS_NS

