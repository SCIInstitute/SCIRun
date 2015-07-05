#ifndef INTERFACE_MODULES_RENDER_ES_COMP_STATIC_WORLD_LIGHT_H
#define INTERFACE_MODULES_RENDER_ES_COMP_STATIC_WORLD_LIGHT_H

#include <cstdint>
#include <glm/glm.hpp>
#include <entity-system/GenericSystem.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace SCIRun {
namespace Render {

// Static world light. We may have multiple world lights inside this structure
// eventually.
struct StaticWorldLight
{
  // -- Data --
  glm::vec3 lightDir;

  // -- Functions --
  StaticWorldLight() { }

  static const char* getName() {return "StaticWorldLight";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("dir", lightDir);
    return true;
  }
};

} // namespace Render
} // namespace SCIRun

#endif 
