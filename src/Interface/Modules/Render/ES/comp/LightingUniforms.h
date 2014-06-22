#ifndef INTERFACE_MODULES_RENDER_ES_COMP_LIGHTING_UNIFORMS_H
#define INTERFACE_MODULES_RENDER_ES_COMP_LIGHTING_UNIFORMS_H

#include <cstdint>
#include <glm/glm.hpp>
#include <entity-system/GenericSystem.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace SCIRun {
namespace Render {

// Static world light. We may have multiple world lights inside this structure
// eventually.
struct LightingUniforms
{
  // -- Data --
  bool hasLightUniform;
  GLint uniformLocation;

  // -- Functions --
  LightingUniforms() : hasLightUniform(false), uniformLocation(0) { }

  static const char* getName() {return "LightingUniforms";}

  void checkUniformArray(GLuint shaderID);
  void applyUniform(const glm::vec3& lightDir) const;

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    return false;
  }
};

} // namespace Render
} // namespace SCIRun

#endif 
