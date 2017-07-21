#include "Registration.hpp"

#include "Filesystem.hpp"
#include "fscomp/StaticFS.hpp"

namespace CPM_ES_FS_NS {

void registerAll(CPM_ES_ACORN_NS::Acorn& core)
{
  // Systems
  Filesystem::registerSystems(core);

  // Components
  core.registerComponent<StaticFS>();   // Non-serializable
}

} // namespace CPM_ES_FS_NS

