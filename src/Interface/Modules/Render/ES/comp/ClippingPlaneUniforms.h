#ifndef INTERFACE_MODULES_RENDER_ES_COMP_CLIPPING_PLANE_UNIFORMS_H
#define INTERFACE_MODULES_RENDER_ES_COMP_CLIPPING_PLANE_UNIFORMS_H

#include <cstdint>
#include <glm/glm.hpp>
#include <entity-system/GenericSystem.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace SCIRun {
  namespace Render {

    // clipping planes
    struct ClippingPlaneUniforms
    {
      // -- Data --
      std::vector<bool> hasClippingPlaneUniforms;
      std::vector<GLint> locClippingPlaneUniforms;
      std::vector<std::string> strClippingPlaneCodes;
      std::vector<bool> hasClippingPlaneCtrlUniforms;
      std::vector<GLint> locClippingPlaneCtrlUniforms;
      std::vector<std::string> strClippingPlaneCtrlCodes;

      // -- Functions --
      ClippingPlaneUniforms();

      static const char* getName() { return "ClippingPlaneUniforms"; }

      void checkUniformArray(GLuint shaderID);
      void applyUniforms(const glm::mat4 &transform, const std::vector<glm::vec4> &clippingPlanes,
        const std::vector<glm::vec4> &clippingPlaneCtrls) const;

      bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
      {
        return false;
      }
    };

  } // namespace Render
} // namespace SCIRun

#endif 
