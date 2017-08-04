#include "Registration.hpp"

#include "Filesystem.hpp"
#include "fscomp/StaticFS.hpp"

namespace spire {

void registerAll(spire::Acorn& core)
{
  // Systems
  Filesystem::registerSystems(core);

  // Components
  core.registerComponent<StaticFS>();   // Non-serializable
}

} // namespace spire

