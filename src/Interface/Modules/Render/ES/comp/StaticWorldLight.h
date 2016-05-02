#ifndef INTERFACE_MODULES_RENDER_ES_COMP_STATIC_WORLD_LIGHT_H
#define INTERFACE_MODULES_RENDER_ES_COMP_STATIC_WORLD_LIGHT_H

#include <cstdint>
#include <glm/glm.hpp>
#include <entity-system/GenericSystem.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include "LightingUniforms.h"

namespace SCIRun {
namespace Render {

// Static world light. We may have multiple world lights inside this structure
// eventually.
struct StaticWorldLight
{
  // -- Data --
  glm::vec3 lightDir[LIGHT_NUM];

  // -- Functions --
  StaticWorldLight()
  {
    lightDir[0] = glm::vec3(1.0f, 0.0f, 0.0f);
  }

  static const char* getName() {return "StaticWorldLight";}

  std::vector<glm::vec3> getLightDir()
  {
    std::vector<glm::vec3> lights;
    for (int i = 0; i < LIGHT_NUM; ++i)
      lights.push_back(lightDir[i]);
    return lights;
  }

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    for (int i = 0; i < LIGHT_NUM; ++i)
      s.serialize("dir", lightDir[i]);
    return true;
  }
};

} // namespace Render
} // namespace SCIRun

#endif 
