#include "Registration.hpp"

#include "comp/StaticObjRefID.hpp"
#include "comp/Transform.hpp"
#include "comp/ConstantRotation.hpp"
#include "comp/CameraSelect.hpp"
#include "comp/ClickBox2D.hpp"
#include "comp/StaticCamera.hpp"
#include "comp/StaticOrthoCamera.hpp"
#include "comp/StaticMouseInput.hpp"
#include "comp/StaticKeyboardInput.hpp"
#include "comp/StaticScreenDims.hpp"
#include "comp/StaticGlobalTime.hpp"
#include "comp/StaticRandom.hpp"

#include "systems/ConstantRotationSys.hpp"
#include "systems/ClickBox2DSys.hpp"

namespace gen {

void registerAll(CPM_ES_ACORN_NS::Acorn& core)
{
  // Register systems
  registerSystem_ConstantRotation(core);
  registerSystem_ClickBox2D(core);

  // Register components
  core.registerComponent<ConstantRotation>();
  core.registerComponent<Transform>();
  core.registerComponent<CameraSelect>();
  core.registerComponent<ClickBox2D>();
  core.registerComponent<StaticMouseInput>();
  core.registerComponent<StaticKeyboardInput>();
  core.registerComponent<StaticScreenDims>();
  core.registerComponent<StaticGlobalTime>();
  core.registerComponent<StaticCamera>();
  core.registerComponent<StaticOrthoCamera>();
  core.registerComponent<StaticObjRefID>();
  core.registerComponent<StaticRandom>();
}

} // namespace gen

