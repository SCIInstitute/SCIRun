#ifndef IAUNS_RENDER_COMPONENTS_STATIC_ORTHO_CAMERA_HPP
#define IAUNS_RENDER_COMPONENTS_STATIC_ORTHO_CAMERA_HPP

#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include "StaticCamera.hpp"

namespace gen {

// This is the same structure as StaticCamera, but we need a different type
// in order to differentiate. We could possibly use simplistic inherintence
// to share functionality.
struct StaticOrthoCamera
{
  // -- Data --
  StaticCameraData data;

  // -- Functions --
  static const char* getName() {return "gen:StaticOrthoCamera";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize camera settings.
    return true;
  }
};

} // namespace gen

#endif 
