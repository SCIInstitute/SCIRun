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
  const int LIGHT_NUM = 4;

struct LightingUniforms
{
  // -- Data --
  bool hasLightUniform[LIGHT_NUM];
  GLint uniformLocation[LIGHT_NUM];
  bool hasLightColorUniform[LIGHT_NUM];
  GLint colorUnifLocation[LIGHT_NUM];

  // -- Functions --
  LightingUniforms();

  static const char* getName() {return "LightingUniforms";}

  void checkUniformArray(GLuint shaderID);
  void applyUniform(const std::vector<glm::vec3>& lightDirs,
    const std::vector<glm::vec3>& lightColors) const;

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    return false;
  }
};

} // namespace Render
} // namespace SCIRun

#endif 
